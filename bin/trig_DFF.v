// -i trig_DFF.v -r dff -s 20 -multicore

//`timescale 1ns/1ps

module deeper_inv(x, y);
  input x;
  output y;
  
  not i1(y, x);
endmodule

module inv(x, y);
  input x;
  output y;
  
  //deeper_inv a1(x, y);
  beh_inv a1(x,y);
endmodule

module beh_inv(x, y);
  input x;
  output y;
  
  always @x
  begin
    y = ~x;
  end
endmodule

module dff(D, C, Q, nQ);
  input D, C;
  output Q, nQ;
  
  //assign net0 = ~D;
  //not   not1(net0, D);
  inv i1(D, net0);
  
  assign net1 = D | C;
  //or    or1(net1, D, C);
  //or    or2(net2, C, net0);
  assign net2 = C | net0;
  
  nand  nand1(net3, net1, net4);
  nand  nand2(net4, net2, net3);

  assign net5 = C & net3;
  //and   and1(net5, C, net3);
  and   and2(net6, C, net4);
  
  nor  #1 nor1(Q, nQ, net5);
  nor  #1 nor2(nQ, Q, net6);
  
endmodule 

module tb_DFF();
  reg in_D, in_C;
  wire out_Q, out_nQ;
  
  dff i1(in_D, in_C, out_Q, out_nQ);
  
  always #5 in_C = ~in_C;
  always #13 in_D = ~in_D;
  
  initial 
  begin
  $dumpfile("trig_DFF.vcd");
  $dumpvars;
  #100 in_D = 0;
  $finish;
  end
  
endmodule
