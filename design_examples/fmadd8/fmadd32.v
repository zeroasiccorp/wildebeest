//#############################################################################
// Copyright: Zero ASIC. All rights Reserved.
// Author: Andreas Olofsson
// License:  MIT (see LICENSE file in LogikBench repository)
//#############################################################################
//
// fmadd32: single-precision (fp32) fused multiply-add top wrapper. It is a
// thin, parametrized shell around the common 'fmadd' datapath, hard-wired to
// the fp32 field layout by its default parameters (EXP=8, MANT=23). The
// reduced-precision tops (fmadd16, fmadd8) instantiate this module with
// overridden parameters, so the arithmetic lives in one place.
//
//#############################################################################
module fmadd32 #(parameter EXP = 8,  // exponent field width  (fp32)
                 parameter MANT = 23 // mantissa field width   (fp32)
                 )
   (
    input [EXP+MANT:0]	a, // multiplicand
    input [EXP+MANT:0]	b, // multiplier
    input [EXP+MANT:0]	c, // addend
    output [EXP+MANT:0]	r  // round(a*b + c), fused
    );

   fmadd #(.EXP(EXP), .MANT(MANT)) u_fmadd (.a(a), .b(b), .c(c), .r(r));

endmodule

//#############################################################################
// Common fused multiply-add datapath (parametrized by EXP/MANT).
//
// Computes r = round(a*b + c) as a single fused operation: the a*b product is
// kept full width and added to c before a single round-to-nearest-even. The
// format is IEEE-754-like with round-to-nearest-even, but simplified for a
// synthesis benchmark: subnormals are flushed to zero (FTZ) on input and
// output, and the sticky handling in far-apart subtraction is approximate
// (accurate to ~1 ulp), so it is not a fully IEEE-754-compliant unit. The
// datapath is fully combinational (continuous assigns; the leading-one
// normalizer is a generated log-shifter).
//#############################################################################
module fmadd #(parameter EXP = 8, // exponent field width
               parameter MANT = 7 // mantissa (fraction) field width
               )
   (
    input [EXP+MANT:0]	a, // multiplicand   (format: 1|EXP|MANT)
    input [EXP+MANT:0]	b, // multiplier
    input [EXP+MANT:0]	c, // addend
    output [EXP+MANT:0]	r  // round(a*b + c), fused
    );

   // Local parameters
   localparam W     = EXP + MANT + 1;    // total width
   localparam MW    = MANT + 1;          // significand width (implicit 1)
   localparam PW    = 2*MW;              // product significand width
   localparam EXTRA = PW + 2;            // low guard bits kept on alignment
   localparam ACCW  = PW + EXTRA + 2;    // accumulator width (~2*PW)
   localparam BIAS  = (1 << (EXP-1)) - 1;
   localparam EW    = 16;                // signed exponent working width
   localparam NST   = $clog2(ACCW);      // normalizer stages

   // Field decode. Subnormals are flushed to zero (FTZ): exp==0 means zero.
   wire	      sa = a[W-1];
   wire	      sb = b[W-1];
   wire	      sc = c[W-1];
   wire [EXP-1:0] ea = a[W-2 -: EXP];
   wire [EXP-1:0] eb = b[W-2 -: EXP];
   wire [EXP-1:0] ec = c[W-2 -: EXP];
   wire [MANT-1:0] fa = a[MANT-1:0];
   wire [MANT-1:0] fb = b[MANT-1:0];
   wire [MANT-1:0] fc = c[MANT-1:0];

   wire		   a_zero = (ea == {EXP{1'b0}});
   wire		   b_zero = (eb == {EXP{1'b0}});
   wire		   c_zero = (ec == {EXP{1'b0}});
   wire		   a_emax = (ea == {EXP{1'b1}});
   wire		   b_emax = (eb == {EXP{1'b1}});
   wire		   c_emax = (ec == {EXP{1'b1}});
   wire		   a_inf = a_emax & (fa == {MANT{1'b0}});
   wire		   b_inf = b_emax & (fb == {MANT{1'b0}});
   wire		   c_inf = c_emax & (fc == {MANT{1'b0}});
   wire		   a_nan = a_emax & (fa != {MANT{1'b0}});
   wire		   b_nan = b_emax & (fb != {MANT{1'b0}});
   wire		   c_nan = c_emax & (fc != {MANT{1'b0}});

   // significands (implicit 1 for normals, 0 for zero/FTZ operands)
   wire [MW-1:0]   siga = a_zero ? {MW{1'b0}} : {1'b1, fa};
   wire [MW-1:0]   sigb = b_zero ? {MW{1'b0}} : {1'b1, fb};
   wire [MW-1:0]   sigc = c_zero ? {MW{1'b0}} : {1'b1, fc};

   wire		   sp = sa ^ sb;               // product sign
   wire		   prod_inf = a_inf | b_inf;

   // Special-value results (override the arithmetic datapath below).
   wire		   inf_x_zero = (a_inf & b_zero) | (b_inf & a_zero);
   wire		   p_is_inf = prod_inf & ~inf_x_zero;
   wire		   inf_m_inf = p_is_inf & c_inf & (sp != sc);
   wire		   res_nan = a_nan | b_nan | c_nan | inf_x_zero | inf_m_inf;
   wire		   res_inf = ~res_nan & (p_is_inf | c_inf);
   wire		   res_isign = p_is_inf ? sp : sc;

   // product significand
   wire [PW-1:0]   prod = siga * sigb;

   // unbiased LSB (unit) exponents, and alignment to the larger of the two
   wire signed [EW-1:0]	eaS = $signed({{(EW-EXP){1'b0}}, ea});
   wire signed [EW-1:0]	ebS = $signed({{(EW-EXP){1'b0}}, eb});
   wire signed [EW-1:0]	ecS = $signed({{(EW-EXP){1'b0}}, ec});
   wire signed [EW-1:0]	p_lsb  = eaS + ebS - (2*BIAS) - (2*MANT);
   wire signed [EW-1:0]	c_lsb  = ecS - BIAS - MANT;
   wire signed [EW-1:0]	maxlsb = (p_lsb >= c_lsb) ? p_lsb : c_lsb;
   wire signed [EW-1:0]	common = maxlsb - EXTRA;
   wire signed [EW-1:0]	ps     = EXTRA - (maxlsb - p_lsb);
   wire signed [EW-1:0]	cs     = EXTRA - (maxlsb - c_lsb);

   // align product term (left if ps>=0, else right with sticky)
   wire			ps_left     = (ps >= 0);
   wire [EW-1:0]	p_rs        = -ps;
   wire [EW-1:0]	ps_amt      = ps_left ? ps[EW-1:0] : p_rs;
   wire			p_allsticky = ~ps_left & (p_rs >= PW);
   wire [ACCW-1:0]	prod_ext    = {{(ACCW-PW){1'b0}}, prod};
   wire [ACCW-1:0]	op_p        = ps_left     ? (prod_ext << ps_amt) :
                        p_allsticky ? {ACCW{1'b0}} :
                        (prod_ext >> ps_amt);
   wire			pst         = ps_left     ? 1'b0 :
                        p_allsticky ? (|prod) :
                        (|(prod & ~({PW{1'b1}} << ps_amt)));

   // align addend term
   wire			cs_left     = (cs >= 0);
   wire [EW-1:0]	c_rs        = -cs;
   wire [EW-1:0]	cs_amt      = cs_left ? cs[EW-1:0] : c_rs;
   wire			c_allsticky = ~cs_left & (c_rs >= MW);
   wire [ACCW-1:0]	sigc_ext    = {{(ACCW-MW){1'b0}}, sigc};
   wire [ACCW-1:0]	op_c        = cs_left     ? (sigc_ext << cs_amt) :
                        c_allsticky ? {ACCW{1'b0}} :
                        (sigc_ext >> cs_amt);
   wire			cst         = cs_left     ? 1'b0 :
                        c_allsticky ? (|sigc) :
                        (|(sigc & ~({MW{1'b1}} << cs_amt)));

   // effective add/subtract (magnitude), pick result sign
   wire			eff_sub = (sp != sc);
   wire			p_ge_c  = (op_p >= op_c);
   wire [ACCW-1:0]	ssum    = eff_sub ? (p_ge_c ? (op_p - op_c) : (op_c - op_p))
                        : (op_p + op_c);
   wire			sbit    = eff_sub ? (p_ge_c ? sp : sc) : sp;
   wire			salign  = pst | cst;

   // leading-one normalizer: log-shifter left-justifies the leading 1 to the
   // MSB and accumulates the shift count (= leading zeros).
   wire [ACCW-1:0]	nstage [0:NST];
   wire [NST:0]		shamt  [0:NST];
   assign nstage[0] = ssum;
   assign shamt[0]  = {(NST+1){1'b0}};

   genvar s;
   generate
      for (s = 0; s < NST; s = s + 1) begin : g_norm
         localparam integer STEP = (1 << (NST-1-s));
         wire		    top_zero = ~(|nstage[s][ACCW-1 -: STEP]);
         assign nstage[s+1] = top_zero ? (nstage[s] << STEP) : nstage[s];
         assign shamt[s+1]  = top_zero ? (shamt[s] + STEP)   : shamt[s];
      end
   endgenerate

   wire [ACCW-1:0] norm = nstage[NST];
   wire [NST:0]	   lz   = shamt[NST];

   // extract significand + guard/round/sticky, round to nearest even
   wire [MW-1:0]   mant_full = norm[ACCW-1 -: MW];
   wire		   roundb    = norm[ACCW-1-MW];
   wire		   stickyb   = (|norm[ACCW-1-MW-1 : 0]) | salign;
   wire [MW:0]	   mrnd      = {1'b0, mant_full} +
                   (roundb & (stickyb | mant_full[0]));
   wire		   mant_of   = mrnd[MW];             // significand -> 2.0
   wire [MANT-1:0] fracout  = mant_of ? mrnd[MANT:1] : mrnd[MANT-1:0];

   // result exponent (unbiased): msb = (ACCW-1) - lz, plus round carry
   wire signed [EW-1:0]	eres  = common + (ACCW-1)
                        - $signed({1'b0, lz}) + $signed({1'b0, mant_of});
   wire signed [EW-1:0]	ebias = eres + BIAS;

   wire			zero_res = (ssum == {ACCW{1'b0}});
   wire			ovf      = (ebias >= ((1 << EXP) - 1));
   wire			udf      = (ebias <= 0);
   wire [EXP-1:0]	ebfield  = ebias[EXP-1:0];

   wire [W-1:0]		tmpres = zero_res ? {W{1'b0}} :
                        ovf      ? {sbit, {EXP{1'b1}}, {MANT{1'b0}}} :
                        udf      ? {sbit, {(W-1){1'b0}}} :
                        {sbit, ebfield, fracout};

   wire [W-1:0]		nanval = {1'b0, {EXP{1'b1}}, 1'b1, {(MANT-1){1'b0}}};
   wire [W-1:0]		infval = {res_isign, {EXP{1'b1}}, {MANT{1'b0}}};

   assign r = res_nan ? nanval : res_inf ? infval : tmpres;

endmodule
