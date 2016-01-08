module trigger (S, R, Q, NQ);

	input S, R;
	output Q, NQ;

	nor nor1(NQ, S, Q);
	nor nor2(Q, R, NQ);
	
	initial begin
		#0 S = 0;
		#0 R = 0;
		#5 S = 1;
		#2 S = 0;
		#13 R = 1;
		#2 R = 0;
	end 

endmodule
