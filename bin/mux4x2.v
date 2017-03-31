// -i mux4x2.v -r mux4x2 -s 20 -multicore

module mux4x2 (x1, x2, x3, x4, y0, y1);
	not not1(nx1, x1);
	not not2(nx2, x2);
	not not3(nx3, x3);
	not not4(nx4, x4);
	and and1(net1, nx1, nx2, x3, nx4);
	and and2(net2, nx1, nx2, nx3, x4);
	and and3(net3, nx1, x2, nx3, nx4);
	or or1(y1, net1, net2);
	or or2(y0, net2, net3);
initial 
begin
  $dumpfile("mux4x2.vcd");
  $dumpvars;
    #0	x1 = 0;
	#0	x2 = 0;
	#0	x3 = 0;
	#0	x4 = 0;
	#5	x1 = 1;
	#5	x1 = 0;
	#0	x2 = 1;
	#5  x1 = 1;
	#5  x1 = 0;
	#0	x2 = 0;
	#0	x3 = 1;
	#5	x1 = 1;
	#5	x1 = 0;
	#0	x2 = 1;
	#5	x1 = 1;
	#5	x1 = 0;
	#0	x2 = 0;
	#0	x3 = 0;
	#0	x4 = 1;
	#5	x1 = 1;
	#5	x1 = 0;
	#0	x2 = 1;
	#5	x1 = 1;
	#5	x1 = 0;
	#0	x2 = 0;
	#0	x3 = 1;
	#5	x1 = 1;
	#5	x1 = 0;
	#0	x2 = 1;
	#5	x1 = 1;
	#5	x1 = 0;
	#0	x2 = 0;
	#0	x3 = 0;
	#0	x4 = 0;
  $finish;
end

endmodule
