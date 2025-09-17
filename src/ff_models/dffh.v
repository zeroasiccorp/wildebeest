module dffh(D,clk,H,Q,QN);
input D;      
input clk;   
input H;      
output reg Q;     
output reg QN; 

always @(posedge clk) 

begin
	if (H == 1'b0) begin // sync. set active low
            Q <= 1'b1;
	end
	else begin
            Q <= D; 
        end
end 

assign QN = !Q;

endmodule 
