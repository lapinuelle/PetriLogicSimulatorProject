// -i 2bit_shift_REG_hier.v -r DFF_test -s 20 -multicore
module inv(in, out);
  input in;
  output out;
  always @in
    begin
      if(in == 0) {
        out = 1;
      }
      if(in == 1) {
        out = 0;
      }
    end
endmodule

module dff(D, C, Q, nQ);
  input D, C;
  output Q, nQ;
  
  //assign net0 = ~D;
  //not   i1(net0, D);
  inv i1(D, net0);
  
  //assign net1 = D | C;
  or    or1(net1, D, C);
  or    or2(net2, C, net0);
  //assign net2 = C | net0;
  
  nand  nand1(net3, net1, net4);
  nand  nand2(net4, net2, net3);

  //assign net5 = C & net3;
  and   and1(net5, C, net3);
  and   and2(net6, C, net4);
  
  nor  nor1(Q, nQ, net5);
  nor  nor2(nQ, Q, net6);
  
endmodule



module shift_REG_2bits(D, C, Q1, Q2);
  input D, C;
  output Q1, Q2, nQ1, nQ2;
 
  dff d1(D,  C, Q1, nQ1);
  dff d2(Q1, C, Q2, nQ2);
endmodule

module DFF_test ();

  shift_REG_2bits(D, C, Q1, Q2);

  initial 
  begin
    $dumpfile("2bit_shift_REG_hier.vcd");
    $dumpvars;
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
    $finish;
  end
  
endmodule
