module dffens(D,clk,E,S,Q);
input D;      // Data input 
input clk;    // clock input 
input E;      // enable 
input S;      // set input 
output reg Q;     // output Q 

always @(negedge clk or negedge S) 

begin
	if (S == 1'b0) begin // set active low
            Q <= 1'b1;
	end 
	else if (E == 1'b1) begin
            Q <= D; 
        end
end 
endmodule 
