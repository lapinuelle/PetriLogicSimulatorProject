module tb_DFF();
  reg D , C ;
  wire Q , nQ ;
  initial begin $dumpfile ( "trig_DFF . vcd" ) ; $dumpvars ; # 0 D = 0 ; C = 0 ; # 10 C = 1 ; # 2 C = 0 ; # 3 D = 1 ; # 5 C = 1 ; # 2 C = 0 ; # 18 D = 0 ; # 10 C = 1 ; # 5 D = 1 ; # 5 C = 0 ; # 25 C = 1 ; # 10 C = 0 ; D = 0 ; $finish ; end

  not i1_not1(i1_net0, D);
  or i1_or1(i1_net1, D, C);
  or i1_or2(i1_net2, C, i1_net0);
  nand i1_nand1(i1_net3, i1_net1, i1_net4);
  nand i1_nand2(i1_net4, i1_net2, i1_net3);
  and i1_and1(i1_net5, C, i1_net3);
  and i1_and2(i1_net6, C, i1_net4);
  nor i1_nor1(Q, nQ, i1_net5);
  nor i1_nor2(nQ, Q, i1_net6);
endmodule
