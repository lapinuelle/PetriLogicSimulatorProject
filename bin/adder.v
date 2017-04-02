// -i adder.v -r tb_FADD -s 20 -multicore

//                   _____
//  A   o------+----| XOR |--        
//             |    |     | |          _____
//  B   o--------+--|_____| ---+------| XOR |-------------o SUM
//             | |             |      |     |
//  Cin o------------------------+----|_____|
//             | |             | |     ___
//             | |             | -----| & |---
//             | |             |      |   |  |   ___
//             | |             -------|___|  ---|   |-----o Cout
//             | |                     ___      | 1 |
//             | ---------------------| & |-----|___|
//             |                      |   |
//             -----------------------|___|
//

module FADDER(A, B, Cin, SUM, Cout);
  input   A, B, Cin;
  output  SUM, Cout;
  
  xor #1 i1(n1, A, B);
  xor #1 i2(SUM, n1, Cin);
  
  and i3(n2, n1, Cin);
  and i4(n3, A, B);
  or  i5(Cout, n2, n3);
endmodule

module tb_FADD();

  reg A, B, Cin;
  wire SUM1, Cout1;
  wire SUM2, Cout2;
  wire SUM3, Cout3;
  wire SUM4, Cout4;
  wire SUM5, Cout5;
	
  FADDER i1(A, B, Cin, SUM1, Cout1);
  FADDER i2(A, B, Cin, SUM2, Cout2);
  FADDER i3(A, B, Cin, SUM3, Cout3);
  FADDER i4(A, B, Cin, SUM4, Cout4);
  FADDER i5(A, B, Cin, SUM5, Cout5);

  initial 
  begin
    $dumpfile("FADDER1.vcd");
	  $dumpvars;
    #0    Cin=0;
    #0    A=0;
    #0    B=0;
    #10   A=1;
    #10   A=0;
    #0    B=1;
    #10   A=1;
    #10   Cin=1;
    #0    A=0;
    #0    B=0;
    #10   A=1;
    #10   A=0;
    #0    B=1;
    #10   A=1;
    $finish;
  end
  
endmodule

  