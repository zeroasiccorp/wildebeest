/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2012  Claire Xenia Wolf <claire@yosyshq.com>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
// Picked up from Xilinx plugin

module DSP48 (
    input signed [17:0] A,
    input signed [17:0] B,
    input signed [47:0] C,
    input signed [17:0] BCIN,
    input signed [47:0] PCIN,
    input CARRYIN,
    input [6:0] OPMODE,
    input SUBTRACT,
    input [1:0] CARRYINSEL,
    output signed [47:0] P,
    output signed [17:0] BCOUT,
    output signed [47:0] PCOUT,
    (* clkbuf_sink *)
    input CLK,
    input CEA,
    input CEB,
    input CEC,
    input CEM,
    input CECARRYIN,
    input CECINSUB,
    input CECTRL,
    input CEP,
    input RSTA,
    input RSTB,
    input RSTC,
    input RSTM,
    input RSTCARRYIN,
    input RSTCTRL,
    input RSTP
);

parameter integer AREG = 1;
parameter integer BREG = 1;
parameter integer CREG = 1;
parameter integer MREG = 1;
parameter integer PREG = 1;
parameter integer CARRYINREG = 1;
parameter integer CARRYINSELREG = 1;
parameter integer OPMODEREG = 1;
parameter integer SUBTRACTREG = 1;
parameter B_INPUT = "DIRECT";
parameter LEGACY_MODE = "MULT18X18S";

wire signed [17:0] A_OUT;
wire signed [17:0] B_OUT;
wire signed [47:0] C_OUT;
wire signed [35:0] M_MULT;
wire signed [35:0] M_OUT;
wire signed [47:0] P_IN;
wire [6:0] OPMODE_OUT;
wire [1:0] CARRYINSEL_OUT;
wire CARRYIN_OUT;
wire SUBTRACT_OUT;
reg INT_CARRYIN_XY;
reg INT_CARRYIN_Z;
reg signed [47:0] XMUX;
reg signed [47:0] YMUX;
wire signed [47:0] XYMUX;
reg signed [47:0] ZMUX;
reg CIN;

// The B input multiplexer.
wire signed [17:0] B_MUX;
assign B_MUX = (B_INPUT == "DIRECT") ? B : BCIN;

// The cascade output.
assign BCOUT = B_OUT;
assign PCOUT = P;

// The registers.
reg signed [17:0] A0_REG;
reg signed [17:0] A1_REG;
reg signed [17:0] B0_REG;
reg signed [17:0] B1_REG;
reg signed [47:0] C_REG;
reg signed [35:0] M_REG;
reg signed [47:0] P_REG;
reg [6:0] OPMODE_REG;
reg [1:0] CARRYINSEL_REG;
reg SUBTRACT_REG;
reg CARRYIN_REG;
reg INT_CARRYIN_XY_REG;

initial begin
	A0_REG = 0;
	A1_REG = 0;
	B0_REG = 0;
	B1_REG = 0;
	C_REG = 0;
	M_REG = 0;
	P_REG = 0;
	OPMODE_REG = 0;
	CARRYINSEL_REG = 0;
	SUBTRACT_REG = 0;
	CARRYIN_REG = 0;
	INT_CARRYIN_XY_REG = 0;
end

always @(posedge CLK) begin
	if (RSTA) begin
		A0_REG <= 0;
		A1_REG <= 0;
	end else if (CEA) begin
		A0_REG <= A;
		A1_REG <= A0_REG;
	end
	if (RSTB) begin
		B0_REG <= 0;
		B1_REG <= 0;
	end else if (CEB) begin
		B0_REG <= B_MUX;
		B1_REG <= B0_REG;
	end
	if (RSTC) begin
		C_REG <= 0;
	end else if (CEC) begin
		C_REG <= C;
	end
	if (RSTM) begin
		M_REG <= 0;
	end else if (CEM) begin
		M_REG <= M_MULT;
	end
	if (RSTP) begin
		P_REG <= 0;
	end else if (CEP) begin
		P_REG <= P_IN;
	end
	if (RSTCTRL) begin
		OPMODE_REG <= 0;
		CARRYINSEL_REG <= 0;
		SUBTRACT_REG <= 0;
	end else begin
		if (CECTRL) begin
			OPMODE_REG <= OPMODE;
			CARRYINSEL_REG <= CARRYINSEL;
		end
		if (CECINSUB)
			SUBTRACT_REG <= SUBTRACT;
	end
	if (RSTCARRYIN) begin
		CARRYIN_REG <= 0;
		INT_CARRYIN_XY_REG <= 0;
	end else begin
		if (CECINSUB)
			CARRYIN_REG <= CARRYIN;
		if (CECARRYIN)
			INT_CARRYIN_XY_REG <= INT_CARRYIN_XY;
	end
end

// The register enables.
assign A_OUT = (AREG == 2) ? A1_REG : (AREG == 1) ? A0_REG : A;
assign B_OUT = (BREG == 2) ? B1_REG : (BREG == 1) ? B0_REG : B_MUX;
assign C_OUT = (CREG == 1) ? C_REG : C;
assign M_OUT = (MREG == 1) ? M_REG : M_MULT;
assign P = (PREG == 1) ? P_REG : P_IN;
assign OPMODE_OUT = (OPMODEREG == 1) ? OPMODE_REG : OPMODE;
assign SUBTRACT_OUT = (SUBTRACTREG == 1) ? SUBTRACT_REG : SUBTRACT;
assign CARRYINSEL_OUT = (CARRYINSELREG == 1) ? CARRYINSEL_REG : CARRYINSEL;
assign CARRYIN_OUT = (CARRYINREG == 1) ? CARRYIN_REG : CARRYIN;

// The multiplier.
assign M_MULT = A_OUT * B_OUT;

// The post-adder inputs.
always @* begin
	case (OPMODE_OUT[1:0])
		2'b00: XMUX <= 0;
		2'b10: XMUX <= P;
		2'b11: XMUX <= {{12{A_OUT[17]}}, A_OUT, B_OUT};
		default: XMUX <= 48'hxxxxxxxxxxxx;
	endcase
	case (OPMODE_OUT[1:0])
		2'b01: INT_CARRYIN_XY <= A_OUT[17] ~^ B_OUT[17];
		2'b11: INT_CARRYIN_XY <= ~A_OUT[17];
		// TODO: not tested in hardware.
		default: INT_CARRYIN_XY <= A_OUT[17] ~^ B_OUT[17];
	endcase
end

always @* begin
	case (OPMODE_OUT[3:2])
		2'b00: YMUX <= 0;
		2'b11: YMUX <= C_OUT;
		default: YMUX <= 48'hxxxxxxxxxxxx;
	endcase
end

assign XYMUX = (OPMODE_OUT[3:0] == 4'b0101) ? M_OUT : (XMUX + YMUX);

always @* begin
	case (OPMODE_OUT[6:4])
		3'b000: ZMUX <= 0;
		3'b001: ZMUX <= PCIN;
		3'b010: ZMUX <= P;
		3'b011: ZMUX <= C_OUT;
		3'b101: ZMUX <= {{17{PCIN[47]}}, PCIN[47:17]};
		3'b110: ZMUX <= {{17{P[47]}}, P[47:17]};
		default: ZMUX <= 48'hxxxxxxxxxxxx;
	endcase
	// TODO: check how all this works on actual hw.
	if (OPMODE_OUT[1:0] == 2'b10)
		INT_CARRYIN_Z <= ~P[47];
	else
		case (OPMODE_OUT[6:4])
			3'b001: INT_CARRYIN_Z <= ~PCIN[47];
			3'b010: INT_CARRYIN_Z <= ~P[47];
			3'b101: INT_CARRYIN_Z <= ~PCIN[47];
			3'b110: INT_CARRYIN_Z <= ~P[47];
			default: INT_CARRYIN_Z <= 1'bx;
		endcase
end

always @* begin
	case (CARRYINSEL_OUT)
		2'b00: CIN <= CARRYIN_OUT;
		2'b01: CIN <= INT_CARRYIN_Z;
		2'b10: CIN <= INT_CARRYIN_XY;
		2'b11: CIN <= INT_CARRYIN_XY_REG;
		default: CIN <= 1'bx;
	endcase
end

// The post-adder.
assign P_IN = SUBTRACT_OUT ? (ZMUX - (XYMUX + CIN)) : (ZMUX + XYMUX + CIN);

endmodule

