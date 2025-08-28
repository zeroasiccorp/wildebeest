module sdffns(D,clk,S,Q);
input D;      // Data input 
input clk;    // clock input 
input S;      // set input 
output reg Q;     // output Q 

always @(negedge clk) 

begin
	if (S == 1'b0) begin // sync. set active low
            Q <= 1'b1;
	end 
	else begin
            Q <= D; 
        end
end 
endmodule 
