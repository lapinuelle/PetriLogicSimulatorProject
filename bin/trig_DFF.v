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
  
  nor   nor1(Q, nQ, net5);
  nor   nor2(nQ, Q, net6);
  
  initial begin
  #0    D=0;
  #0    C=0;
  #10   C=1;
  #2   C=0;
  #3   D=1;
  #5   C=1;
  #2   C=0;
  #18   D=0;
  #10   C=1;
  #5   D=1;
  #5   C=0;
  #25   C=1;
  #10   C=0;
  #0   D=0;
  
  end
  
endmodule
