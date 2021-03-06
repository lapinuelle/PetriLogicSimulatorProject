// -i 3inv_beh.v -r tb_3inv -s 20 -multicore

module beh_inv(x, y);
  input x;
  output y;
  
  always @x
  begin
	if(x == 1)
	  begin
	    y = 0;
      end
	if(x == 0)
	  begin
	    y = 1;
	  end
  end
endmodule

module 3invertors (in, out);

	input in;
	output out;
	wire inv1, inv2;

	beh_inv #1 not1(in, inv1);
	beh_inv #2 not2(inv1, inv2);
	beh_inv #3 not3(inv2, out);
endmodule

module tb_3inv();
	reg in;
	wire out;
	
	3invertors invert(in, out);
	
	always #5 in = ~in;
	
	initial begin
    $dumpfile("3inv_beh.vcd");
    $dumpvars;
	#100 in = 0;
    $finish;
	end 

endmodule
