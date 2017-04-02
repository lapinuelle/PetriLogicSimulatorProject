// -i trig_DFF.v -r dff -s 20 -multicore

`timescale 1ns/1ps

module dff(D, C, Q, nQ);
  input D, C;
  output Q, nQ;
  
  not   not1(net0, D);
  
  or    or1(net1, D, C);
  or    or2(net2, C, net0);
  
  nand  nand1(net3, net1, net4);
  nand  nand2(net4, net2, net3);

  and   and1(net5, C, net3);
  and   and2(net6, C, net4);
  
  nor  #1 nor1(Q, nQ, net5);
  nor  #1 nor2(nQ, Q, net6);
endmodule 


	

module tb_DFF();
  reg D, C, I;
  wire Q, nQ, O;
  
//  beh i2(I, O);
  dff i1(D, C, Q, nQ);
  
  initial 
  begin
  $dumpfile("trig_DFF.vcd");
  $dumpvars;
  #0   D=0;
  #0   C=0;
  #10  C=1;
  #2   C=0;
  #3   D=1;
  #5   C=1;
  #2   C=0;
  #18  D=0;
  #10  C=1;
  #5   D=1;
  #5   C=0;
  #25  C=1;
  #10  C=0;
  #0   D=0;
  $finish;
  end
  
endmodule
