// tech_bram_with_init.v
//
// Init-capable variant of 'tech_bram.v', selected by
// 'synth_fpga -bram_with_init'.  It differs from 'tech_bram.v' in exactly two
// ways: each of the three techmap modules declares an 'INIT' parameter, and
// each '_TECHMAP_REPLACE_' instance forwards it to the macro.
//
// 'INIT' is attached to the mapped cell by 'memory_libmap' when the memory
// library declares 'init no_undef' (see 'bram_memory_map_with_init.txt', which
// also records the bit layout).  It must be forwarded VERBATIM: the layout is a
// flat bit vector over the macro's whole capacity, so the same value is correct
// for every geometry selected below, and re-ordering it here would silently
// scramble the memory contents.
//
// Dropping the forward is the failure this file exists to prevent.  It does not
// error anywhere downstream -- the design places, routes and programs, and the
// BRAM simply comes up full of zeros.

module \$__tech_spram__
  # (
     parameter INIT = 0,
     parameter PORT_A_WIDTH = 64,
     parameter PORT_A_RD_USED = 1,
     parameter PORT_A_WR_USED = 1,
     parameter PORT_A_OPTION_WRITE_MODE = "NO_CHANGE",
     parameter PORT_A_ABITS = 14,
     parameter PORT_A_WR_EN_WIDTH = 1,
     parameter PORT_A_WR_BE_WIDTH = 2,
     parameter PORT_A_OPTION_RDWR = "OLD"
     ) 
   (
    input                            PORT_A_CLK,
    input                            PORT_A_CLK_EN,
    input [(PORT_A_WR_EN_WIDTH-1):0] PORT_A_WR_EN,
    input [(PORT_A_WR_BE_WIDTH-1):0] PORT_A_WR_BE,
    input                            PORT_A_RD_EN,
    input [(PORT_A_ABITS-1):0]       PORT_A_ADDR,
    input [PORT_A_WIDTH-1:0]         PORT_A_WR_DATA,
    output [PORT_A_WIDTH-1:0]        PORT_A_RD_DATA
    );

   generate

      if (PORT_A_WIDTH > 32) begin

	 wire _TECHMAP_FAIL_ = 1'b1;

      end


      else if (PORT_A_WIDTH > 16) begin

	 spram_512x32 #(.INIT(INIT)) _TECHMAP_REPLACE_ (
					  .clk(PORT_A_CLK),
					  .ce(PORT_A_CLK_EN),
					  .we(PORT_A_WR_EN),
					  .be(PORT_A_WR_BE),
					  .re(PORT_A_RD_EN),
					  .addr(PORT_A_ADDR[(PORT_A_ABITS - 1)-:9]),
					  .din(PORT_A_WR_DATA),
					  .dout(PORT_A_RD_DATA)
					  );	 

      end


      else if (PORT_A_WIDTH > 8) begin

	 spram_1024x16 #(.INIT(INIT)) _TECHMAP_REPLACE_ (
					  .clk(PORT_A_CLK),
					  .ce(PORT_A_CLK_EN),
					  .we(PORT_A_WR_EN),
					  .be(PORT_A_WR_BE),
					  .re(PORT_A_RD_EN),
					  .addr(PORT_A_ADDR[(PORT_A_ABITS - 1)-:10]),
					  .din(PORT_A_WR_DATA),
					  .dout(PORT_A_RD_DATA)
					  );	 

      end


      else if (PORT_A_WIDTH > 4) begin

	 spram_2048x8 #(.INIT(INIT)) _TECHMAP_REPLACE_ (
					  .clk(PORT_A_CLK),
					  .ce(PORT_A_CLK_EN),
					  .we(PORT_A_WR_EN),
					  .be(PORT_A_WR_BE),
					  .re(PORT_A_RD_EN),
					  .addr(PORT_A_ADDR[(PORT_A_ABITS - 1)-:11]),
					  .din(PORT_A_WR_DATA),
					  .dout(PORT_A_RD_DATA)
					  );	 

      end


      else if (PORT_A_WIDTH > 2) begin

	 spram_4096x4 #(.INIT(INIT)) _TECHMAP_REPLACE_ (
					  .clk(PORT_A_CLK),
					  .ce(PORT_A_CLK_EN),
					  .we(PORT_A_WR_EN),
					  .be(PORT_A_WR_BE),
					  .re(PORT_A_RD_EN),
					  .addr(PORT_A_ADDR[(PORT_A_ABITS - 1)-:12]),
					  .din(PORT_A_WR_DATA),
					  .dout(PORT_A_RD_DATA)
					  );	 

      end


      else if (PORT_A_WIDTH > 1) begin

	 spram_8192x2 #(.INIT(INIT)) _TECHMAP_REPLACE_ (
					  .clk(PORT_A_CLK),
					  .ce(PORT_A_CLK_EN),
					  .we(PORT_A_WR_EN),
					  .be(PORT_A_WR_BE),
					  .re(PORT_A_RD_EN),
					  .addr(PORT_A_ADDR[(PORT_A_ABITS - 1)-:13]),
					  .din(PORT_A_WR_DATA),
					  .dout(PORT_A_RD_DATA)
					  );	 

      end


      else begin

	 spram_16384x1 #(.INIT(INIT)) _TECHMAP_REPLACE_ (
					  .clk(PORT_A_CLK),
					  .ce(PORT_A_CLK_EN),
					  .we(PORT_A_WR_EN),
					  .be(PORT_A_WR_BE),
					  .re(PORT_A_RD_EN),
					  .addr(PORT_A_ADDR[(PORT_A_ABITS - 1)-:14]),
					  .din(PORT_A_WR_DATA),
					  .dout(PORT_A_RD_DATA)
					  );	 

      end


   endgenerate
   
   
endmodule 

module \$__tech_sdpram__
  # (
     parameter INIT = 0,
     parameter PORT_A_OPTION_WRITE_MODE = "NO_CHANGE",
     parameter PORT_A_WIDTH = 32,
     parameter PORT_A_ABITS = 14,
     parameter PORT_A_WR_EN_WIDTH = 1,
     parameter PORT_A_WR_BE_WIDTH = 2,
     parameter PORT_B_OPTION_WRITE_MODE = "NO_CHANGE",
     parameter PORT_B_WIDTH = 32,
     parameter PORT_B_ABITS = 14,
     parameter PORT_B_RD_EN_WIDTH = 1,
     parameter OPTION_SYNC_MODE = 0
     ) 
   (
    input 			     CLK_C,
    input 			     PORT_A_CLK,
    input 			     PORT_A_CLK_EN,
    input [(PORT_A_WR_EN_WIDTH-1):0] PORT_A_WR_EN,
    input [(PORT_A_WR_BE_WIDTH-1):0] PORT_A_WR_BE,  // TODO validate
    input [(PORT_A_ABITS-1):0] 	     PORT_A_ADDR,
    input [PORT_A_WIDTH-1:0] 	     PORT_A_WR_DATA,
   
    input 			     PORT_B_CLK,
    input 			     PORT_B_CLK_EN,
    input [(PORT_B_RD_EN_WIDTH-1):0] PORT_B_RD_EN,
    input [(PORT_B_ABITS-1):0] 	     PORT_B_ADDR,
    output [PORT_B_WIDTH-1:0] 	     PORT_B_RD_DATA
    );

   wire                              CLK_A_ASSIGNED;
   wire                              CLK_B_ASSIGNED;
   
   
   generate

      if (OPTION_SYNC_MODE == 1) begin
         assign CLK_A_ASSIGNED = CLK_C;
         assign CLK_B_ASSIGNED = CLK_C;
      end
      else begin
         assign CLK_A_ASSIGNED = PORT_A_CLK;
         assign CLK_B_ASSIGNED = PORT_B_CLK;
      end
      
      if (PORT_A_WIDTH > 16) begin

	 wire _TECHMAP_FAIL_ = 1'b1;

      end


      else if (PORT_A_WIDTH > 8) begin

	 sdpram_1024x16 #(.INIT(INIT)) _TECHMAP_REPLACE_ (
					  .clk_a(CLK_A_ASSIGNED),
					  .clk_b(CLK_B_ASSIGNED),
					  .ce_a(PORT_A_CLK_EN),
					  .we_a(PORT_A_WR_EN),
					  .be_a(PORT_A_WR_BE),
					  .addr_a(PORT_A_ADDR[(PORT_A_ABITS - 1)-:10]),
					  .din_a(PORT_A_WR_DATA),
					  .ce_b(PORT_B_CLK_EN),
					  .re_b(PORT_B_RD_EN),
					  .addr_b(PORT_B_ADDR[(PORT_B_ABITS - 1)-:10]),
					  .dout_b(PORT_B_RD_DATA)
					  );	 

      end


      else if (PORT_A_WIDTH > 4) begin

	 sdpram_2048x8 #(.INIT(INIT)) _TECHMAP_REPLACE_ (
					  .clk_a(CLK_A_ASSIGNED),
					  .clk_b(CLK_B_ASSIGNED),
					  .ce_a(PORT_A_CLK_EN),
					  .we_a(PORT_A_WR_EN),
					  .be_a(PORT_A_WR_BE),
					  .addr_a(PORT_A_ADDR[(PORT_A_ABITS - 1)-:11]),
					  .din_a(PORT_A_WR_DATA),
					  .ce_b(PORT_B_CLK_EN),
					  .re_b(PORT_B_RD_EN),
					  .addr_b(PORT_B_ADDR[(PORT_B_ABITS - 1)-:11]),
					  .dout_b(PORT_B_RD_DATA)
					  );	 

      end


      else if (PORT_A_WIDTH > 2) begin

	 sdpram_4096x4 #(.INIT(INIT)) _TECHMAP_REPLACE_ (
					  .clk_a(CLK_A_ASSIGNED),
					  .clk_b(CLK_B_ASSIGNED),
					  .ce_a(PORT_A_CLK_EN),
					  .we_a(PORT_A_WR_EN),
					  .be_a(PORT_A_WR_BE),
					  .addr_a(PORT_A_ADDR[(PORT_A_ABITS - 1)-:12]),
					  .din_a(PORT_A_WR_DATA),
					  .ce_b(PORT_B_CLK_EN),
					  .re_b(PORT_B_RD_EN),
					  .addr_b(PORT_B_ADDR[(PORT_B_ABITS - 1)-:12]),
					  .dout_b(PORT_B_RD_DATA)
					  );	 

      end


      else if (PORT_A_WIDTH > 1) begin

	 sdpram_8192x2 #(.INIT(INIT)) _TECHMAP_REPLACE_ (
					  .clk_a(CLK_A_ASSIGNED),
					  .clk_b(CLK_B_ASSIGNED),
					  .ce_a(PORT_A_CLK_EN),
					  .we_a(PORT_A_WR_EN),
					  .be_a(PORT_A_WR_BE),
					  .addr_a(PORT_A_ADDR[(PORT_A_ABITS - 1)-:13]),
					  .din_a(PORT_A_WR_DATA),
					  .ce_b(PORT_B_CLK_EN),
					  .re_b(PORT_B_RD_EN),
					  .addr_b(PORT_B_ADDR[(PORT_B_ABITS - 1)-:13]),
					  .dout_b(PORT_B_RD_DATA)
					  );	 

      end


      else begin

	 sdpram_16384x1 #(.INIT(INIT)) _TECHMAP_REPLACE_ (
					  .clk_a(CLK_A_ASSIGNED),
					  .clk_b(CLK_B_ASSIGNED),
					  .ce_a(PORT_A_CLK_EN),
					  .we_a(PORT_A_WR_EN),
					  .be_a(PORT_A_WR_BE),
					  .addr_a(PORT_A_ADDR[(PORT_A_ABITS - 1)-:14]),
					  .din_a(PORT_A_WR_DATA),
					  .ce_b(PORT_B_CLK_EN),
					  .re_b(PORT_B_RD_EN),
					  .addr_b(PORT_B_ADDR[(PORT_B_ABITS - 1)-:14]),
					  .dout_b(PORT_B_RD_DATA)
					  );	 

      end

      
   endgenerate

endmodule


module \$__tech_tdpram__
  # (
     parameter INIT = 0,
     parameter PORT_A_RD_USED = 1,
     parameter PORT_A_WR_USED = 1,
     parameter PORT_A_OPTION_WRITE_MODE = "NO_CHANGE",
     parameter PORT_A_WIDTH = 32,
     parameter PORT_A_ABITS = 14,
     parameter PORT_A_WR_EN_WIDTH = 1,
     parameter PORT_A_WR_BE_WIDTH = 2,
     parameter PORT_B_RD_USED = 1,
     parameter PORT_B_WR_USED = 1,
     parameter PORT_B_OPTION_WRITE_MODE = "NO_CHANGE",
     parameter PORT_B_WIDTH = 32,
     parameter PORT_B_ABITS = 14,
     parameter PORT_B_WR_EN_WIDTH = 1,
     parameter PORT_B_WR_BE_WIDTH = 2,
     parameter OPTION_SYNC_MODE = 0
     ) 
   (
    input                            CLK_C,
    input                            PORT_A_CLK,
    input                            PORT_A_CLK_EN,
    input [(PORT_A_WR_EN_WIDTH-1):0] PORT_A_WR_EN,
    input [(PORT_A_WR_BE_WIDTH-1):0] PORT_A_WR_BE,
    input                            PORT_A_RD_EN,
    input [(PORT_A_ABITS-1):0]       PORT_A_ADDR,
    input [PORT_A_WIDTH-1:0]         PORT_A_WR_DATA,
    output [PORT_A_WIDTH-1:0]        PORT_A_RD_DATA,
   
    input                            PORT_B_CLK,
    input                            PORT_B_CLK_EN,
    input [(PORT_B_WR_EN_WIDTH-1):0] PORT_B_WR_EN,
    input [(PORT_B_WR_BE_WIDTH-1):0] PORT_B_WR_BE,
    input                            PORT_B_RD_EN,
    input [(PORT_B_ABITS-1):0]       PORT_B_ADDR,
    input [PORT_B_WIDTH-1:0]         PORT_B_WR_DATA,
    output [PORT_B_WIDTH-1:0]        PORT_B_RD_DATA
    );

   wire                              CLK_A_ASSIGNED;
   wire                              CLK_B_ASSIGNED;
   
   
   generate

      if (OPTION_SYNC_MODE == 1) begin
         assign CLK_A_ASSIGNED = CLK_C;
         assign CLK_B_ASSIGNED = CLK_C;
      end
      else begin
         assign CLK_A_ASSIGNED = PORT_A_CLK;
         assign CLK_B_ASSIGNED = PORT_B_CLK;
      end
      
      if (PORT_A_WIDTH > 16) begin

	 wire _TECHMAP_FAIL_ = 1'b1;

      end


      else if (PORT_A_WIDTH > 8) begin

	 tdpram_1024x16 #(.INIT(INIT)) _TECHMAP_REPLACE_ (
					  .clk_a(CLK_A_ASSIGNED),
					  .clk_b(CLK_B_ASSIGNED),
					  .ce_a(PORT_A_CLK_EN),
					  .we_a(PORT_A_WR_EN),
					  .be_a(PORT_A_WR_BE),
					  .re_a(PORT_A_RD_EN),
					  .addr_a(PORT_A_ADDR[(PORT_A_ABITS - 1)-:10]),
					  .din_a(PORT_A_WR_DATA),
					  .dout_a(PORT_A_RD_DATA),
					  .ce_b(PORT_B_CLK_EN),
					  .re_b(PORT_B_RD_EN),
					  .we_b(PORT_B_WR_EN),
					  .be_b(PORT_B_WR_BE),
					  .addr_b(PORT_B_ADDR[(PORT_B_ABITS - 1)-:10]),
					  .din_b(PORT_B_WR_DATA),
					  .dout_b(PORT_B_RD_DATA)
					  );	 

      end


      else if (PORT_A_WIDTH > 4) begin

	 tdpram_2048x8 #(.INIT(INIT)) _TECHMAP_REPLACE_ (
					  .clk_a(CLK_A_ASSIGNED),
					  .clk_b(CLK_B_ASSIGNED),
					  .ce_a(PORT_A_CLK_EN),
					  .we_a(PORT_A_WR_EN),
					  .be_a(PORT_A_WR_BE),
					  .re_a(PORT_A_RD_EN),
					  .addr_a(PORT_A_ADDR[(PORT_A_ABITS - 1)-:11]),
					  .din_a(PORT_A_WR_DATA),
					  .dout_a(PORT_A_RD_DATA),
					  .ce_b(PORT_B_CLK_EN),
					  .re_b(PORT_B_RD_EN),
					  .we_b(PORT_B_WR_EN),
					  .be_b(PORT_B_WR_BE),
					  .addr_b(PORT_B_ADDR[(PORT_B_ABITS - 1)-:11]),
					  .din_b(PORT_B_WR_DATA),
					  .dout_b(PORT_B_RD_DATA)
					  );	 

      end


      else if (PORT_A_WIDTH > 2) begin

	 tdpram_4096x4 #(.INIT(INIT)) _TECHMAP_REPLACE_ (
					  .clk_a(CLK_A_ASSIGNED),
					  .clk_b(CLK_B_ASSIGNED),
					  .ce_a(PORT_A_CLK_EN),
					  .we_a(PORT_A_WR_EN),
					  .be_a(PORT_A_WR_BE),
					  .re_a(PORT_A_RD_EN),
					  .addr_a(PORT_A_ADDR[(PORT_A_ABITS - 1)-:12]),
					  .din_a(PORT_A_WR_DATA),
					  .dout_a(PORT_A_RD_DATA),
					  .ce_b(PORT_B_CLK_EN),
					  .re_b(PORT_B_RD_EN),
					  .we_b(PORT_B_WR_EN),
					  .be_b(PORT_B_WR_BE),
					  .addr_b(PORT_B_ADDR[(PORT_B_ABITS - 1)-:12]),
					  .din_b(PORT_B_WR_DATA),
					  .dout_b(PORT_B_RD_DATA)
					  );	 

      end


      else if (PORT_A_WIDTH > 1) begin

	 tdpram_8192x2 #(.INIT(INIT)) _TECHMAP_REPLACE_ (
					  .clk_a(CLK_A_ASSIGNED),
					  .clk_b(CLK_B_ASSIGNED),
					  .ce_a(PORT_A_CLK_EN),
					  .we_a(PORT_A_WR_EN),
					  .be_a(PORT_A_WR_BE),
					  .re_a(PORT_A_RD_EN),
					  .addr_a(PORT_A_ADDR[(PORT_A_ABITS - 1)-:13]),
					  .din_a(PORT_A_WR_DATA),
					  .dout_a(PORT_A_RD_DATA),
					  .ce_b(PORT_B_CLK_EN),
					  .re_b(PORT_B_RD_EN),
					  .we_b(PORT_B_WR_EN),
					  .be_b(PORT_B_WR_BE),
					  .addr_b(PORT_B_ADDR[(PORT_B_ABITS - 1)-:13]),
					  .din_b(PORT_B_WR_DATA),
					  .dout_b(PORT_B_RD_DATA)
					  );	 

      end


      else begin

	 tdpram_16384x1 #(.INIT(INIT)) _TECHMAP_REPLACE_ (
					  .clk_a(CLK_A_ASSIGNED),
					  .clk_b(CLK_B_ASSIGNED),
					  .ce_a(PORT_A_CLK_EN),
					  .we_a(PORT_A_WR_EN),
					  .be_a(PORT_A_WR_BE),
					  .re_a(PORT_A_RD_EN),
					  .addr_a(PORT_A_ADDR[(PORT_A_ABITS - 1)-:14]),
					  .din_a(PORT_A_WR_DATA),
					  .dout_a(PORT_A_RD_DATA),
					  .ce_b(PORT_B_CLK_EN),
					  .re_b(PORT_B_RD_EN),
					  .we_b(PORT_B_WR_EN),
					  .be_b(PORT_B_WR_BE),
					  .addr_b(PORT_B_ADDR[(PORT_B_ABITS - 1)-:14]),
					  .din_b(PORT_B_WR_DATA),
					  .dout_b(PORT_B_RD_DATA)
					  );	 

      end

      
   endgenerate
      
endmodule
