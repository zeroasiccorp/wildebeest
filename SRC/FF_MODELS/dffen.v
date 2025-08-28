module dffen(D,clk,E,Q);
input D;     // Data input 
input clk;   // clock input 
input E;     //  enable
output reg Q;    // output Q 

always @(negedge clk) 

begin
	if (E == 1'b1) begin
          Q <= D; 
        end
end 
endmodule 
