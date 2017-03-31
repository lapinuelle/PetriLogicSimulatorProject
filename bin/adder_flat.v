module tb_FADD();
  reg A , B , Cin ;
  wire SUM1 , Cout1 ;
  wire SUM2 , Cout2 ;
  wire SUM3 , Cout3 ;
  wire SUM4 , Cout4 ;
  wire SUM5 , Cout5 ;
  initial begin $dumpfile ( "FADDER1 . vcd" ) ; $dumpvars ; # 0 Cin = 0 ; # 0 A = 0 ; # 0 B = 0 ; # 10 A = 1 ; # 10 A = 0 ; # 0 B = 1 ; # 10 A = 1 ; # 10 Cin = 1 ; # 0 A = 0 ; # 0 B = 0 ; # 10 A = 1 ; # 10 A = 0 ; # 0 B = 1 ; # 10 A = 1 ; $finish ; end

  xor #1 i1_i1(i1_n1, A, B);
  xor #1 i1_i2(SUM1, i1_n1, Cin);
  and i1_i3(i1_n2, i1_n1, Cin);
  and i1_i4(i1_n3, A, B);
  or i1_i5(Cout1, i1_n2, i1_n3);
  xor #1 i2_i1(i2_n1, A, B);
  xor #1 i2_i2(SUM2, i2_n1, Cin);
  and i2_i3(i2_n2, i2_n1, Cin);
  and i2_i4(i2_n3, A, B);
  or i2_i5(Cout2, i2_n2, i2_n3);
  xor #1 i3_i1(i3_n1, A, B);
  xor #1 i3_i2(SUM3, i3_n1, Cin);
  and i3_i3(i3_n2, i3_n1, Cin);
  and i3_i4(i3_n3, A, B);
  or i3_i5(Cout3, i3_n2, i3_n3);
  xor #1 i4_i1(i4_n1, A, B);
  xor #1 i4_i2(SUM4, i4_n1, Cin);
  and i4_i3(i4_n2, i4_n1, Cin);
  and i4_i4(i4_n3, A, B);
  or i4_i5(Cout4, i4_n2, i4_n3);
  xor #1 i5_i1(i5_n1, A, B);
  xor #1 i5_i2(SUM5, i5_n1, Cin);
  and i5_i3(i5_n2, i5_n1, Cin);
  and i5_i4(i5_n3, A, B);
  or i5_i5(Cout5, i5_n2, i5_n3);
endmodule
