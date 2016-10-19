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
    $finish
  end
  
endmodule

  