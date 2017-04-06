#include "gates.h"
#include "nets.h"
#include "netlist.h"
#include "netlistreader.h"
#include "simulator.h"
#include "simulation_data.h"
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
  clock_t A = clock();
#endif
  
  std::string filename;
  inetlistreader *p_reader = NULL;
  netlist* netl = new netlist;
  sim_data* simul_data = new sim_data;
  simulator* sim = new simulator;
  int stackSize = 20;																							        // размер стека
  std::string rootModule = "root";																				// имя модуля верхнего уровня
  bool multiCore = false;

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
      if ((std::string(argv[i]) == "-multicore"))
        multiCore = true;
    }
  }
  else {
    printf("__err__ : You must specify input file.\n\n");
    goto EXIT_POINT;
  }
#if defined DEBUG_MEASURE_TIME
  clock_t B = clock();
#endif

  p_reader = get_appropriate_reader(filename);
  if(!p_reader) 
    goto EXIT_POINT;
  if(!p_reader->read(netl, simul_data, rootModule))
    goto EXIT_POINT;

#if defined DEBUG_MEASURE_TIME
  clock_t C = clock();
#endif

  
  /*if (multiCore) {
    printf("__inf__ : Simulation started using multi CPU cores.\n");
    sim->simulation(netl, simul_data, stackSize);           // проводим симуляцию
  } else {*/
    printf("__inf__ : Simulation started using single CPU core.\n");
    sim->simulation_stack(netl, simul_data, stackSize);           // проводим симуляцию
  //}
  
  delete sim;                                                             // удаляем объект
#if defined DEBUG_MEASURE_TIME
  clock_t D = clock();
  printf("\nSimulation statistics:\n");
  printf("\tcommand line args parsing time: %ldms\n", B - A);
  printf("\tnetlist reading time: %ldms\n", C - B);
  printf("\tsimulation time: %ldms\n", D - C);
  printf("Total time spent on simulation: %ldms\n\n", D - A);
#endif

EXIT_POINT:                                                               // точка выхода
  free_reader(p_reader);
  delete simul_data;
  delete netl;

  return 0;
}
