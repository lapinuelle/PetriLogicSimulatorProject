// -i adder_02.v -r tb_FADD -s 20 -multicore

module FADDER(A, B, Cin, SUM, Cout);
  input   A, B, Cin;
  output  SUM, Cout;
  
  xor i1(n1, A, B);
  xor i2(SUM, n1, Cin);
  
  and i3(n2, n1, Cin);
  and i4(n3, A, B);
  or  i5(Cout, n2, n3);
endmodule

module tb_FADD();

  reg A, B, Cin;
  wire SUM, Cout;
	
  FADDER i(A, B, Cin, SUM, Cout);

  initial 
  begin
    $dumpfile("FADDER.vcd");
	  $dumpvars;
    #0    Cin=0;
          A=0;
          B=0;
    #10   A=1;
    #10   A=0;
          B=1;
    #10   A=1;
    #10   Cin=1;
          A=0;
          B=0;
    #10   A=1;
    #10   A=0;
          B=1;
    #10   A=1;
    $finish;
  end
  
endmodule

  