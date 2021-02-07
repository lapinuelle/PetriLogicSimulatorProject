// -i count4_dc2.v -r tb_Johnson_count -s 20  -sdf count4_dc2.sdf
// -verify count4_dc2.v count4_dc2.v -r tb_Johnson_count -s 20  -sdf count4_dc2.sdf -signal negedge clk

#include "gates.h"
#include "nets.h"
#include "netlist.h"
#include "netlistreader.h"
#include "simulator.h"
#include "simulation_data.h"
#include "sdf.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DEBUG_MEASURE_TIME

/******* Info about version naming history ******
0.0.1 - Первая, хоть как-то работающая версия симулятора, пока только gate level
0.0.2 - Перешли на нетлиcты в формате Verilog HDL
0.0.3 - Введён стек моделирования, значительно улучшено быстродействие
0.0.4 - Размер стека и модуль верхнего уровня задаются параметрами командной строки
	      -s <размер_стека>
	      -r <название_модуля>
	      -i <входной_файл>
0.0.5 - Чтение иерархии и преобразование в плоский нетлист
0.0.6 - Распараллеливание на ядрах CPU с использованием C++11 Threads
        Включается параметром -multicore
        Отключено до лучших времён
0.0.7 - Нормальная читалка иерархии, читается уровень вложенности > 1
*************************************************/

int main(int argc, char *argv[]) {
  printf("PetriLogicSimulator v0.0.8\nBehavioral description reading and parsing implementation\n\n");
#if defined DEBUG_MEASURE_TIME
  clock_t A, B, C, D;
#endif
#if defined DEBUG_MEASURE_TIME
  A = clock();
#endif
  
  std::string filename;
  inetlistreader *p_reader1 = NULL, *p_reader2 = NULL;
  netlist* netl1 = new netlist;
  netlist* netl2 = new netlist;
  sim_data* simul_data1 = new sim_data;
  sim_data* simul_data2 = new sim_data;
  simulator* sim1 = new simulator;
  simulator* sim2 = new simulator;
  int stackSize = 20;																							        // размер стека
  std::string rootModule = "root";																				// имя модуля верхнего уровня
  std::string sdfFile;																				// имя модуля верхнего уровня
  //bool multiCore = false;
  std::string cmp_file1, 
              cmp_file2;
  std::string signal, edge;

  SDF *sdf = NULL;
																												                  // Чтение аргументов командной строки
  if (argc > 2) {
    for (size_t i = 0; i < (size_t)argc; i++) {
      if (std::string(argv[i]) == "-i")
        if (i < (size_t)argc - 1)
          filename = argv[i + 1];
      if ((std::string(argv[i]) == "-s") && (i < (size_t)argc - 1))
        stackSize = atoi(argv[i + 1]);
      if ((std::string(argv[i]) == "-r") && (i < (size_t)argc - 1))
        rootModule = argv[i + 1];
      if ((std::string(argv[i]) == "-sdf") && (i < (size_t)argc - 1))
        sdfFile = argv[i + 1];
      if ((std::string(argv[i]) == "-verify") && (i < (size_t)argc - 2)) {
        cmp_file1 = argv[i + 1];
        cmp_file2 = argv[i + 2];
      }
      if ((std::string(argv[i]) == "-signal") && (i < (size_t)argc - 2)) {
        edge = argv[i + 1];  // "posedge", "negedge"
        signal = argv[i + 2];
      }
      //if ((std::string(argv[i]) == "-multicore"))
      //  multiCore = true;
    }
  }
  else {
    printf("__err__ : You must specify input file.\n\n");
    goto EXIT_POINT;
  }
#if defined DEBUG_MEASURE_TIME
  B = clock();
#endif

  if (!filename.empty()) {
    p_reader1 = get_appropriate_reader(filename);
    if(!p_reader1) 
      goto EXIT_POINT;
    if(!p_reader1->read(netl1, simul_data1, rootModule))
      goto EXIT_POINT;
    free_reader(&p_reader1);
    if (!sdfFile.empty()) {
      sdf = new SDF;
      if (!sdf->read(sdfFile))
        goto EXIT_POINT;
    }


#if defined DEBUG_MEASURE_TIME
  C = clock();
#endif

  
  /*if (multiCore) {
    printf("__inf__ : Simulation started using multi CPU cores.\n");
    sim->simulation_thread(netl, simul_data, stackSize);           // проводим симуляцию
  } else {*/
    printf("__inf__ : Simulation started using single CPU core.\n");
    sim1->simulation(netl1, simul_data1, stackSize, sdf);           // проводим симуляцию

    delete sim1;                                                             // удаляем объект
  }
  //}

  if (!cmp_file1.empty() && !cmp_file2.empty()) {

    p_reader1 = get_appropriate_reader(cmp_file1);
    if (!p_reader1)
      goto EXIT_POINT;
    if (!p_reader1->read(netl1, simul_data1, rootModule))
      goto EXIT_POINT;
    free_reader(&p_reader1);

    p_reader2 = get_appropriate_reader(cmp_file2);
    if(!p_reader2) 
      goto EXIT_POINT;
    if(!p_reader2->read(netl2, simul_data2, rootModule))
      goto EXIT_POINT;
    free_reader(&p_reader2);

   if (!sdfFile.empty()) {
      sdf = new SDF;
      if (!sdf->read(sdfFile))
        goto EXIT_POINT;
    }


#if defined DEBUG_MEASURE_TIME
    C = clock();
#endif


    printf("__inf__ : Simulation started using step-by-step mode.\n");
    //sim1->simulation(netl, simul_data, stackSize, sdf);           // проводим симуляцию
    sim1->begin_multistep_mode(netl1, simul_data1, stackSize, NULL);
    sim2->begin_multistep_mode(netl2, simul_data2, stackSize, sdf);

    while (!simul_data1->newEventChain.empty() ) {
      float t1 = sim1->make_one_step(netl1, simul_data1, stackSize, NULL, signal, edge);
      float t2 = sim2->make_one_step(netl2, simul_data2, stackSize, sdf, signal, edge);

      for (auto i = netl1->netsMap.begin(); i != netl1->netsMap.end(); ++i) {
        
        if (!(*i).second) {
          //printf("__wrn__ : (*i).second == NULL with name = '%s' appeared at time %f\n", (*i).first.c_str(), t1);
          //printf("          t2 = %f\n", t2);
          continue;
        }

        if((*i).second->getName().find('.') != std::string::npos)
          continue;
        if ((*i).second->value != netl2->netsMap[(*i).second->getName()]->value) {
          printf("__inf__ : [1:%f] [2:%f] : Pin values are not equal for pin '%s'\n", t1, t2, (*i).first.c_str());
          printf("          [1] :  %d, [2] : %d\n", (*i).second->value, netl2->netsMap[(*i).second->getName()]->value);
        }
      }

    }

    delete sim1;                                                             // удаляем объект
    delete sim2;                                                             // удаляем объект
  }

#if defined DEBUG_MEASURE_TIME
  D = clock();
  printf("\nSimulation statistics:\n");
  printf("\tcommand line args parsing time: %ldms\n", B - A);
  printf("\tnetlist reading time: %ldms\n", C - B);
  printf("\tsimulation time: %ldms\n", D - C);
  printf("Total time spent on simulation: %ldms\n\n", D - A);
#endif

EXIT_POINT:                                                               // точка выхода
  if(simul_data1)
    delete simul_data1;
  if(simul_data2)
    delete simul_data2;
  if(netl1)
    delete netl1;
  if(netl2)
    delete netl2;
  
  if(sdf)
    delete sdf;

  return 0;
}
