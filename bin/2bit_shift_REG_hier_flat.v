module DFF_test();
  initial begin # 0 D = 0 ; # 0 C = 0 ; # 10 C = 1 ; # 2 C = 0 ; # 3 D = 1 ; # 5 C = 1 ; # 2 C = 0 ; # 18 D = 0 ; # 10 C = 1 ; # 5 D = 1 ; # 5 C = 0 ; # 25 C = 1 ; # 10 C = 0 ; # 0 D = 0 ; end

  not _d1_not1(_d1_net0, D);
  or _d1_or1(_d1_net1, D, C);
  or _d1_or2(_d1_net2, C, _d1_net0);
  nand _d1_nand1(_d1_net3, _d1_net1, _d1_net4);
  nand _d1_nand2(_d1_net4, _d1_net2, _d1_net3);
  and _d1_and1(_d1_net5, C, _d1_net3);
  and _d1_and2(_d1_net6, C, _d1_net4);
  nor _d1_nor1(Q1, _nQ1, _d1_net5);
  nor _d1_nor2(_nQ1, Q1, _d1_net6);
  not _d2_not1(_d2_net0, Q1);
  or _d2_or1(_d2_net1, Q1, C);
  or _d2_or2(_d2_net2, C, _d2_net0);
  nand _d2_nand1(_d2_net3, _d2_net1, _d2_net4);
  nand _d2_nand2(_d2_net4, _d2_net2, _d2_net3);
  and _d2_and1(_d2_net5, C, _d2_net3);
  and _d2_and2(_d2_net6, C, _d2_net4);
  nor _d2_nor1(Q2, _nQ2, _d2_net5);
  nor _d2_nor2(_nQ2, Q2, _d2_net6);
endmodule
