// -i 3inv.v -r 3invertors -s 20 -multicore
module 3invertors (in, out);

	input in;
	output out;
	wire inv1, inv2;

	not not1(inv1, in);
	not not2(inv2, inv1);
	not not3(out, inv2);
endmodule

module tb_3inv();
	reg in;
	wire out;
	
	3invertors invert(in, out);
	
	initial begin
    $dumpfile("3inv.vcd");
    $dumpvars;
		#0 in = 0;
		#5 in = 1;
		#7 in = 0;
		#20 in = 1;
    $finish;
	end 

endmodule
