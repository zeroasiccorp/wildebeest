//An similar additional type of support is to allow either
//edge of the set/reset/enable signals to be active.  Support
//of these types is where the number of flop permutations really
//explodes, and where having a naming convention that is easy
//to parse is critical.  Above, we have sacrificed a bit of
//clarity in exchange for brevity of flop names, so that when
//they are the only types used the flop types are compact while
//still easy to understand.  When permuting the set/reset/enable
//polarities, this breaks down.  We'll deal with this by simply
//mimicing Yosys's convention as a suffix stacked on top of our
//Core 8/Negative Core 8 naming conventions.  Do this as follows:

//Define a three-letter suffix in which the meaning of the suffix
//is <reset polarity><set polarity><enable polarity>
//and each polarity is denoted as follows:
//
// p - positive edge / active high polarity
// n - negative edge / active low polarity
// x - signal not present
//
//In this convention, our Core 8 would have the following
//expanded names:
//
// dff    | dff_xxx
// dffr   | dffr_nxx
// dffs   | dffs_xnx
// dffrs  | dffrs_nnx
// dffe   | dffe_xxp
// dffer  | dffer_nxp
// dffes  | dffes_xnp
// dffers | dffers_nnp

//As you can see, this convention introduces a bit of field redundancy,
//in exchange for the preservation of the original Core 8 names

//Like the core 8, the negative edge clock versions simply insert an 'n'
//in the apprpropriate location prior to the underscore.

//First, cover the positive edge versions of flops with just a set OR a
//reset (one new version edge)

(* techmap_celltype = "$_DFF_PP0_" *)
module dffr_pxx
  (
   clk,
   D,
   R,
   Q
   );

   input clk;
   input R;
   input D;
   output Q;


endmodule // tech_dffr_pxx
      
(* techmap_celltype = "$_DFF_PP1_" *)
module dffs_xpx
  (
   clk,
   D,
   S,
   Q
   );

   input clk;
   input S;
   input D;
   output Q;

      
endmodule // tech_dffs_xpx

//Next, cover the positive edge versions of flops with a set AND a
//reset (three versions)

(* techmap_celltype = "$_DFFSR_PPN_" *)
module dffrs_pnx
  (
   clk,
   D,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   output Q;

      
endmodule // tech_dffrs_pnx

(* techmap_celltype = "$_DFFSR_PNP_" *)
module dffrs_npx
  (
   clk,
   D,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   output Q;

endmodule // tech_dffrs_npx

(* techmap_celltype = "$_DFFSR_PPP_" *)
module dffrs_ppx
  (
   clk,
   D,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   output Q;

      
endmodule // tech_dffrs_ppx

//Do this all over again for flops with active high enables

(* techmap_celltype = "$_DFFE_PP0P_" *)
module dffer_pxp
  (
   clk,
   D,
   E,
   R,
   Q
   );

   input clk;
   input R;
   input D;
   input E;
   output Q;

endmodule // tech_dffer_pxp
      
(* techmap_celltype = "$_DFFE_PP1P_" *)
module dffes_xpp
  (
   clk,
   D,
   E,
   S,
   Q
   );

   input clk;
   input S;
   input D;
   input E;
   output Q;

      
endmodule // tech_dffes_xpp

(* techmap_celltype = "$_DFFSRE_PPNP_" *)
module dffers_pnp
  (
   clk,
   D,
   E,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffers_pnp

(* techmap_celltype = "$_DFFSRE_PNPP_" *)
module dffers_npp
  (
   clk,
   D,
   E,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffers_npp

(* techmap_celltype = "$_DFFSRE_PPPP_" *)
module dffers_ppp
  (
   clk,
   D,
   E,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffers_ppp

//Make versions of Core 8 flops with active low enables

(* techmap_celltype = "$_DFFE_PN_" *)
module dffe_xxn
  (
   clk,
   D,
   E,
   Q
   );

   input clk;
   input D;
   input E;
   output Q;

endmodule // tech_dffe
			
(* techmap_celltype = "$_DFFE_PN0N_" *)
module dffer_nxn
  (
   clk,
   D,
   E,
   R,
   Q
   );

   input clk;
   input R;
   input D;
   input E;
   output Q;

endmodule // tech_dffer

(* techmap_celltype = "$_DFFE_PN1N_" *)
module dffes_xnn
  (
   clk,
   D,
   E,
   S,
   Q
   );

   input clk;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffes

(* techmap_celltype = "$_DFFSRE_PNNN_" *)
module dffers_nnn
  (
   clk,
   D,
   E,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffers

//Now do all the non-core flops with active low enables

(* techmap_celltype = "$_DFFE_PP0N_" *)
module dffer_pxn
  (
   clk,
   D,
   E,
   R,
   Q
   );

   input clk;
   input R;
   input D;
   input E;
   output Q;

endmodule // tech_dffer_pxn
      
(* techmap_celltype = "$_DFFE_PP1N_" *)
module dffes_xpn
  (
   clk,
   D,
   E,
   S,
   Q
   );

   input clk;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffes_xpn

(* techmap_celltype = "$_DFFSRE_PPNN_" *)
module dffers_pnn
  (
   clk,
   D,
   E,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffers_pnn

(* techmap_celltype = "$_DFFSRE_PNPN_" *)
module dffers_npn
  (
   clk,
   D,
   E,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffers_npn

(* techmap_celltype = "$_DFFSRE_PPPN_" *)
module dffers_ppn
  (
   clk,
   D,
   E,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffers_ppn

//Now repeat ALL the non-core flops we just made with negative-edge
//triggered clocks

//First, cover the positive edge versions of flops with just a set OR a
//reset (one new version edge)

(* techmap_celltype = "$_DFF_NP0_" *)
module dffnr_pxx
  (
   clk,
   D,
   R,
   Q
   );

   input clk;
   input R;
   input D;
   output Q;

endmodule // tech_dffnr_pxx
      
(* techmap_celltype = "$_DFF_NP1_" *)
module dffns_xpx
  (
   clk,
   D,
   S,
   Q
   );

   input clk;
   input S;
   input D;
   output Q;

endmodule // tech_dffns_xpx

//Next, cover the positive edge versions of flops with a set AND a
//reset (three versions)

(* techmap_celltype = "$_DFFSR_NPN_" *)
module dffnrs_pnx
  (
   clk,
   D,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   output Q;

endmodule // tech_dffnrs_pnx

(* techmap_celltype = "$_DFFSR_NNP_" *)
module dffnrs_npx
  (
   clk,
   D,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   output Q;

endmodule // tech_dffnrs_npx

(* techmap_celltype = "$_DFFSR_NPP_" *)
module dffnrs_ppx
  (
   clk,
   D,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   output Q;

endmodule // tech_dffnrs_ppx

//Do this all over again for flops with active high enables

(* techmap_celltype = "$_DFFE_NP0P_" *)
module dffenr_pxp
  (
   clk,
   D,
   E,
   R,
   Q
   );

   input clk;
   input R;
   input D;
   input E;
   output Q;

endmodule // tech_dffenr_pxp
      
(* techmap_celltype = "$_DFFE_NP1P_" *)
module dffens_xpp
  (
   clk,
   D,
   E,
   S,
   Q
   );

   input clk;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffens_xpp

(* techmap_celltype = "$_DFFSRE_NPNP_" *)
module dffenrs_pnp
  (
   clk,
   D,
   E,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffenrs_pnp

(* techmap_celltype = "$_DFFSRE_NNPP_" *)
module dffenrs_npp
  (
   clk,
   D,
   E,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffenrs_npp

(* techmap_celltype = "$_DFFSRE_NPPP_" *)
module dffenrs_ppp
  (
   clk,
   D,
   E,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffenrs_ppp

//Make versions of Core 8 flops with active low enables

(* techmap_celltype = "$_DFFE_NN_" *)
module dffen_xxn
  (
   clk,
   D,
   E,
   Q
   );

   input clk;
   input D;
   input E;
   output Q;

endmodule // tech_dffen
			
(* techmap_celltype = "$_DFFE_NN0N_" *)
module dffenr_nxn
  (
   clk,
   D,
   E,
   R,
   Q
   );

   input clk;
   input R;
   input D;
   input E;
   output Q;

endmodule // tech_dffenr

(* techmap_celltype = "$_DFFE_NN1N_" *)
module dffens_xnn
  (
   clk,
   D,
   E,
   S,
   Q
   );

   input clk;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffens

(* techmap_celltype = "$_DFFSRE_NNNN_" *)
module dffenrs_nnn
  (
   clk,
   D,
   E,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffenrs

//Now do all the non-core flops with active low enables

(* techmap_celltype = "$_DFFE_NP0N_" *)
module dffenr_pxn
  (
   clk,
   D,
   E,
   R,
   Q
   );

   input clk;
   input R;
   input D;
   input E;
   output Q;

endmodule // tech_dffenr_pxn
      
(* techmap_celltype = "$_DFFE_NP1N_" *)
module dffens_xpn
  (
   clk,
   D,
   E,
   S,
   Q
   );

   input clk;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffens_xpn

(* techmap_celltype = "$_DFFSRE_NPNN_" *)
module dffenrs_pnn
  (
   clk,
   D,
   E,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffenrs_pnn

(* techmap_celltype = "$_DFFSRE_NNPN_" *)
module dffenrs_npn
  (
   clk,
   D,
   E,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffenrs_npn

(* techmap_celltype = "$_DFFSRE_NPPN_" *)
module dffenrs_ppn
  (
   clk,
   D,
   E,
   R,
   S,
   Q
   );

   input clk;
   input R;
   input S;
   input D;
   input E;
   output Q;

endmodule // tech_dffenrs_ppn
