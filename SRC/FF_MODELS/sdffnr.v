module sdffnr(D,clk,R,Q);
input D;      // Data input 
input clk;    // clock input 
input R;      // reset input 
output reg Q;     // output Q 

always @(negedge clk) 

begin
	if (R == 1'b0) begin // sync. reset active low
            Q <= 1'b0;
	end 
	else begin
            Q <= D; 
        end
end 
endmodule 
