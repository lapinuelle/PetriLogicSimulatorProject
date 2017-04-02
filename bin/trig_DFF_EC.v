// -i trig_DFF_EC.v -r tb_dff -s 20 -multicore

module DFF(D, C, EC, Q, nQ);
  input D, C, EC;
  output Q, nQ;
  
  or    or0(net1, D, Cdff);

  not   not1(net0, D);

  or    or1(Cdff, C, EC);
  or    or2(net2, Cdff, net0);
  
  nand  nand1(net3, net1, net4);
  nand  nand2(net4, net2, net3);

  nor   nor1(Q, nQ, net5);
  nor   nor2(nQ, Q, net6);

  and   and1(net5, Cdff, net3);
  and   and2(net6, Cdff, net4);
  
endmodule

module tb_DFF();
  reg D, C, EC;
  wire Q, nQ;
  
  DFF dff1(D, C, EC, Q, nQ);
  
  initial 
  begin
  $dumpfile("trig_DFF_ec.vcd");
  $dumpvars;

  #0	C = 0;
		EC = 0;
		D = 0;
		
  // 1
  #10	C = 1;
  
  #5	EC = 1;

  #5	C = 0;

  // 2
  #10	C = 1;
  #10	C = 0;
  
  // 3
  #10	
		D = 1;
		C = 1;
		//D = 1;
		//EC = 0;
  #10	C = 0;
  
  // 4
  #10	
		EC = 0;
  // Вот тут я обманываю iverilog! (Поставил #0 отдельно!)
  // Просто говорю, что в этот же момент времени происходит событие, но происходит следом за предыдущим!
  // Если это убрать - будет вариант 1 с картинки
  // С этим кодом - вариант 2
  //#0
		C = 1;
		//D = 0;
  #10	C = 0;

  // 5
  #10	C = 1;
  #5	D = 1;
  #5	C = 0;

  // 6
  #10	C = 1;
  #10	C = 0;

  #5	D = 0;
  
  // 7
  #10	C = 1;
  #10	C = 0;

  $finish;
  end
  
endmodule
