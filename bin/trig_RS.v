// -i trig_RS.v -r trigger -s 20 -multicore

module trigger (S, R, Q, NQ);

	input S, R;
	output Q, NQ;

	nor nor1(NQ, S, Q);
	nor nor2(Q, R, NQ);
	
	initial begin
    $dumpfile("trig_RS.vcd");
    $dumpvars;
		#0 S = 0;
		#0 R = 0;
		#5 S = 1;
		#2 S = 0;
		#13 R = 1;
		#2 R = 0;
    $finish;
	end 

endmodule
