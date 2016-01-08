module dlatch(D, E, Q, nQ);
  input S, R;
  output Q, nQ;
  
  and and1(net1, D, E);
  not not1(net0, D);
  and and2(net2, E, net0);
  nor nor1(nQ, net1, Q);
  nor nor2(Q, net2, nQ);
  
  initial begin
  #0    D=0;
  #0    E=0;
  #10   E=1;
  #2   E=0;
  #3   D=1;
  #5   E=1;
  #2   E=0;
  #18   D=0;
  #10   E=1;
  #5   D=1;
  #5   E=0;
  #30   D=0;
  
  end
  
endmodule
