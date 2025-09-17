module dffehl(D,clk,E,H,L,Q,QN);
input D;      
input clk;   
input E;     
input H;      
input L;      
output reg Q;     
output reg QN; 

always @(posedge clk) 

begin
	if (L == 1'b0) begin // sync. reset active low
            Q <= 1'b0;
	end 
	else if (H == 1'b0) begin // sync. set active low
            Q <= 1'b1;
	end
	else if (E == 1'b1) begin
            Q <= D; 
        end
end 

assign QN = !Q;

endmodule 
