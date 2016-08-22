#include "gates.h"
#include "nets.h"
#include "netlist.h"
#include "netlistreader.h"
#include "simulator.h"
#include "simulation_data.h"
#include "VerilogHDL_Flattener.h"
#include <stdlib.h>

/******* Info about version naming history ******
0.0.1 - ������, ���� ���-�� ���������� ������ ����������, ���� ������ gate level
0.0.2 - ������� �� �����c�� � ������� Verilog HDL
0.0.3 - ����� ���� �������������, ����������� �������� ��������������
0.0.4 - ������ ����� � ������ �������� ������ �������� ����������� ��������� ������
	      -s <������_�����>
	      -r <��������_������>
	      -i <�������_����>
0.0.5 - ������ �������� � �������������� � ������� �������
*************************************************/

int main(int argc, char *argv[]) {
  printf("PetriLogicSimulator v0.0.5\n\n");
  std::string filename;
  inetlistreader *p_reader = NULL;
  netlist* netl = new netlist;
  sim_data* simul_data = new sim_data;
  simulator* sim = new simulator;
  int stackSize = 20;																							        // ������ �����
  std::string rootModule = "root";																				// ��� ������ �������� ������
  VerilogHDL_Flattener vnf;

																												                  // ������ ���������� ��������� ������
  if (argc > 2) {
    for (size_t i = 0; i < (size_t)argc; i++) {
      if (std::string(argv[i]) == "-i")
        if (i < (size_t)argc - 1)
          filename = argv[i + 1];
      if ((std::string(argv[i]) == "-s") && (i < (size_t)argc - 1))
        stackSize = atoi(argv[i + 1]);
      if ((std::string(argv[i]) == "-r") && (i < (size_t)argc - 1))
        rootModule = argv[i + 1];
    }
  }
  else {
    printf("__err__ : You must specify input file.\n\n");
    goto EXIT_POINT;
  }

  //*
  if (!vnf.Read(filename, rootModule)) {
    goto EXIT_POINT;
  }
  //*/
                                                                          // ������ ��������
  p_reader = get_appropriate_reader(vnf.GetFlatFileName());
  if(!p_reader) 
    goto EXIT_POINT;
  if(!p_reader->read(netl, simul_data))
    goto EXIT_POINT;
  
 
  //sim->simulation_stack(netl, simul_data, vnf.GetFlatFileName(), stackSize);           // �������� ���������
    sim->simulation(netl, simul_data, vnf.GetFlatFileName(), stackSize);           // �������� ���������
  delete sim;                                                             // ������� ������

EXIT_POINT:                                                               // ����� ������
  free_reader(p_reader);
  delete simul_data;
  delete netl;

  return 0;
}
