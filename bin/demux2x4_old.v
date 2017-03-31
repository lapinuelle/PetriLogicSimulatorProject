// -i demux2x4.v -r demux2x4 -s 20 -multicore

module demux2x4 (x1, x2, y0, y1, y2, y3);
	not not1(nx1, x1);
	not not2(nx2, x2);
	
	and and1(y0, nx1, nx2);
	and and2(y1, nx1, x2);
	and and3(y2, x1, nx2);
	and and4(y3, x1, x2);
initial 
begin
  $dumpfile("demux2x4.vcd");
  $dumpvars;
  #0	x1 = 0;
	#0	x2 = 0;
	#5	x1 = 1;
	#0	x2 = 1;
	#10	x1 = 0;
	#10 x2 = 0;
	#0	x1 = 1;
	#10	x1 = 0;
	#10	x1 = 1;
	#0  x2 = 1;
  $finish;
end

endmodule