module 3invertors (in, out);

	input in;
	output out;
	wire inv1, inv2;

	not not1(inv1, in);
	not not2(inv2, inv1);
	not not3(out, inv2);
	
	initial begin
		#0 in = 0;
		#5 in = 1;
		#7 in = 0;
		#20 in = 1;
	end 

endmodule
