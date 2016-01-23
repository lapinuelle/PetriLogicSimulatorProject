module shifter();
  input D , C ;
  output Q0 , Q1 ;
  initial begin # 0 D = 0 ; # 0 C = 0 ; # 10 C = 1 ; # 2 C = 0 ; # 3 D = 1 ; # 5 C = 1 ; # 2 C = 0 ; # 8 C = 1 ; # 2 C = 0 ; # 8 D = 0 ; # 0 C = 1 ; # 2 C = 0 ; # 8 C = 1 ; # 2 C = 0 ; # 8 C = 1 ; # 2 C = 0 ; # 8 C = 1 ; # 2 C = 0 ; end

  not not_00_1(net000, D);
  or or_00_1(net001, D, C);
  or or_00_2(net002, C, net000);
  nand nand_00_1(net003, net001, net004);
  nand nand_00_2(net004, net002, net003);
  and and_00_1(net005, C, net003);
  and and_00_2(net006, C, net004);
  nor nor_00_1(Q0, nQ0, net005);
  nor nor_00_2(nQ0, Q0, net006);
  not not_01_1(net010, Q0);
  or or_01_1(net011, Q0, C);
  or or_01_2(net012, C, net010);
  nand nand_01_1(net013, net011, net014);
  nand nand_01_2(net014, net012, net013);
  and and_01_1(net015, C, net013);
  and and_01_2(net016, C, net014);
  nor nor_01_1(Q1, nQ1, net015);
  nor nor_01_2(nQ1, Q1, net016);
endmodule
