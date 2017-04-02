#include "simulator.h"
#include "netlist.h"
#include "simulation_data.h"
#include "datawriter.h"
#include <iostream>
#include <algorithm>
#include <list>
#include <thread>
#include <mutex>
#include "stack.h"
#include "numcores.h"


//#define DEBUG_PRINT_THREAD
//#define DEBUG_PRINT_GATES

enum OperationType {
  t_minus = 0,
  t_zero,
  t_plus,
  t_none,
  eject,
};

enum ThreadStatus {
  ts_wait_for_data = 0,   // поток должен ждать следующую порцию данных
  ts_work_on_data,        // поток может приступать к обработке порции данных
  ts_can_exit,            // поток может завершиться
};

std::vector<ThreadStatus>  statuses;    
std::mutex mut;
OperationType OpType;

std::vector<int> ranges;
std::vector<int> starts;
int deal = 0;

simulator::simulator() {
}

void routine(int id, stack *stackSim, netlist *netli) {
#if defined(DEBUG_PRINT_THREAD)
  printf("[ %d ] : Entering thread\n", id);
#endif
THREAD_BEGIN:
#if defined(DEBUG_PRINT_THREAD)
  printf("[ %d ] : Waiting\n", id);
#endif
  // Ждём, пока из функции main нам не установят статус либо продолжения работы, либо выхода 
  bool can_continue = false;
  while (!can_continue) {
    __asm nop;
    can_continue = (statuses[id] != ts_wait_for_data);
  }

  printf("");

  // Если мы вышли из while, значит либо работем, либо выходим
  // Просят выйти - выходим
  if (statuses[id] == ts_can_exit)
    goto THREAD_END;
  
  // Просят работать - работаем
#if defined(DEBUG_PRINT_THREAD)
  printf("[ %d ] : Working on data\n", id);
#endif

  if (OpType == t_minus) {
    for (int i = starts[id]; i < ranges[id]; ++i) {
#if defined(DEBUG_PRINT_GATES)
      printf("Thread [%d] -> t_minus : %s\n", id, stackSim->gatesChain[stackSim->busy + i]->name.c_str());
#endif
      stackSim->gatesChain[stackSim->busy + i]->t_minus();
#if defined(DEBUG_PRINT_GATES)
      printf("Thread [%d] -> operate : %s\n", id, stackSim->gatesChain[stackSim->busy + i]->name.c_str());
#endif
      stackSim->gatesChain[stackSim->busy + i]->operate();
        deal+=2;
    }
  }

  if (OpType == t_plus) {
    for (int i = starts[id]; i < ranges[id]; ++i) {
#if defined(DEBUG_PRINT_GATES)
      printf("Thread [%d] -> t_plus  : %s\n", id, stackSim->gatesChain[stackSim->busy + i]->name.c_str());
#endif
      if (stackSim->gatesChain[stackSim->busy + i]->repeat < 500) {
        bool valueChanged = false;
        if (stackSim->gatesChain[stackSim->busy + i]->t_plus())
          valueChanged = true;
        ++(stackSim->gatesChain[stackSim->busy + i]->repeat);                                                  
        // если флаг назначен, то добавляем вентили, висящие на выходе узла в стек
        if (valueChanged) {                                                                                 
          for (int y = 0, gatchsize = stackSim->gatesChain[stackSim->busy + i]->outs.size(); y < gatchsize; ++y) {
            std::vector <gate*> returned = netli->returnGate(stackSim->gatesChain[stackSim->busy + i]->outs[y]);
            if ((!returned.empty()) && (stackSim->gatesChain[stackSim->busy + i]->repeat < 500))
              for (int index = 0, retsize = returned.size(); index < retsize; ++index) 
                stackSim->push_back(returned[index]);
          }
        }
      }
      ++deal;
    }
  }
  // Закончили обработку данных - сами себе ставим статус ожидания новых данных и переходим в цикл ожидания while
  statuses[id] = ts_wait_for_data;
  
  goto THREAD_BEGIN;

THREAD_END:
#if defined(DEBUG_PRINT_THREAD)
  printf("[ %d ] : Exiting thread\n", id);
#endif
  ;
}

void simulator::simulation(netlist* netl, sim_data* simData, std::string filename, int stackSize) {
  // время начала моделирования
  int initialTime = 0;                                                                                
  // время окончания моделирования
  int stopTime = simData->eventChain[simData->eventChain.size() - 1].time + 10;
  // непосредственно момент моделирования
  int time = 0;                                                                                       

  // получем количество ядер ЦП в системе
  int cores = getNumCores();                                                                          
  //int cores = 4;
  //printf("__inf__ : Available cores: %d\n", cores);
  int tempSize = 0;
  statuses.resize(cores);
  std::fill(statuses.begin(), statuses.end(), ts_wait_for_data);
  OpType = t_none;

  std::vector<std::thread> threads;
  threads.reserve(cores);

  ranges.resize(cores);
  starts.resize(cores);

  // стек моделирования
  stack *stackSim = new stack(stackSize);                                                             
  for (int core = 0; core < cores; ++core)
    threads.push_back(std::thread(routine, core, stackSim, netl));

  // размер стека
  int temp_free = 0;

  datawriter wr(simData->getVCDname().c_str());

  for each(net* nett in netl->nets)
    wr.AddDumpVar(nett);
  //for (size_t i = 0; i < netl->nets.size(); ++i)
  //  wr.AddDumpVar(netl->nets[i]);                                                                    

  wr.PrintHeader();

  int i = 0;
  for (time = initialTime; time < stopTime; ++time) {

    if(simData->eventChain[i].time != time)
      continue;

    //printf("=== Time = [%d] ===\n", time);
    // обнуляем счётчик повторений для всех вентилей
//    for each(gate* valve in netl->gates)
//      valve->repeat = 0;
    for (int k = 0, gsize = netl->gates.size(); k < gsize; ++k)
      netl->gates[k]->repeat = 0;

    // присваиваем значения портам
    
    for (int k = 0, nsize = netl->nets.size(); k < nsize; k++) {
      for (int j = 0, echnsize = simData->eventChain[i].netsChain.size(); j < echnsize; j++) {
        if (netl->nets[k]->name == simData->eventChain[i].netsChain[j]->name)
          netl->nets[k]->value = simData->eventChain[i].statesChain[j];
      }
    }
    /*
    for each (net* wire in netl->nets) {
      int st = 0;
      for each (net* chainWire in simData->eventChain[i].netsChain) {
        if(wire->name == chainWire->name) 
          wire->value = simData->eventChain[i].statesChain[st];
        st++;
      }
    }
    */

    stackSim->reset();

    // инициируем начальную цепочку вентилей на основе изменения состояний на входах

    /*
    for each(net* wire in simData->eventChain[i].netsChain) {
      for each(gate* netgate in wire->gates) {
        std::vector<gate*>::iterator it = std::find(stackSim->gatesChain.begin(), stackSim->gatesChain.end(), netgate);
        int iit = std::distance(stackSim->gatesChain.begin(), it);
        if (it != stackSim->gatesChain.end()) {
          if (iit > stackSim->free && iit < stackSim->busy)
            stackSim->push_back(netgate);
        }
        else 
          stackSim->push_back(netgate);
      }
    }
    */
    
    for (int l = 0, ecncsize = simData->eventChain[i].netsChain.size(); l < ecncsize; l++) {
      for (int m = 0, ecncgsize = simData->eventChain[i].netsChain[l]->gates.size(); m < ecncgsize; m++) {
        std::vector<gate*>::iterator it = std::find(stackSim->gatesChain.begin(), stackSim->gatesChain.end(), simData->eventChain[i].netsChain[l]->gates[m]);
        int iit = std::distance(stackSim->gatesChain.begin(), it);
        if (it != stackSim->gatesChain.end()) {
          if (iit > stackSim->free && iit < stackSim->busy)
            stackSim->push_back(simData->eventChain[i].netsChain[l]->gates[m]);
        }
        else 
          stackSim->push_back(simData->eventChain[i].netsChain[l]->gates[m]);
      }
    }
    

    //
    // Вот тут-то и начинается самый сок
    //

    // Работают сети Петри
    while (stackSim->busy != stackSim->free) {                                                                
      //printf(".");
      bool allEqual = true;
      for (int core = 0; core < cores; ++core) {
        if(allEqual && ts_wait_for_data != statuses[core]) {
          __asm nop;
          allEqual = false;
        }
        //printf("%d:%d", core, cores);
        switch (statuses[core]) {
        case ts_wait_for_data: 
          //printf("? ");
          break;
        case ts_work_on_data:
          //printf("! ");
          break;
        case ts_can_exit:
          //printf("@ ");
          break;
        }
      }
      //printf("\n");
      if (!allEqual)
        continue;

      // назначаем временный указатель free, на случай, если free < busy
      if (stackSim->busy > stackSim->free)                                                                    
        temp_free = stackSim->free + stackSize;
      else
        temp_free = stackSim->free;

      int stackFilled = temp_free - stackSim->busy;

      std::fill(starts.begin(), starts.end(), 0);
      std::fill(ranges.begin(), ranges.end(), 0);

      // раскидаем весь стек по потокам. Сами указатели не двигаем, передаём лишь интервалы

      // Если элементов меньше, чем ядер
      if (stackFilled < cores) {
        ranges[0] = 1;
        for (int si = 1; si < stackFilled; ++si) {
          starts[si] = starts[si - 1] + 1;
          ranges[si] = ranges[si - 1] + 1;
        }
      }
      // Если элементов больше, чем ядер
      else {
        int basicSize = (stackFilled - (stackFilled % cores)) / cores;
        ranges[0] = basicSize;
        for (int si = 1; si < cores; ++si) {
          starts[si] = starts[si - 1] + basicSize;
          ranges[si] = ranges[si - 1] + basicSize;
        }
        ranges[cores - 1] += stackFilled % cores;
      }

      if (deal == tempSize * 4) {
        deal = 0;
        tempSize = stackFilled;
        OpType = t_minus;
        //printf("t_minus : ");
        std::fill(statuses.begin(), statuses.end(), ts_work_on_data);
        //printf("wait->work\n");
        continue;
      }

      if (deal == tempSize) {
        OpType = t_zero;
        #if defined(DEBUG_PRINT_THREAD)
          printf("t_zero : ");
        #endif
        std::fill(statuses.begin(), statuses.end(), ts_work_on_data);
        #if defined(DEBUG_PRINT_THREAD)
          printf("wait->work\n");
        #endif
        continue;
      }
      if (deal == tempSize * 2) {
        OpType = t_plus;
        //printf("t_plus : ");
        std::fill(statuses.begin(), statuses.end(), ts_work_on_data);
        //printf("wait->work\n");
        continue;
      }

      if (deal == tempSize * 3) {
        for (int ind = 0; ind < tempSize; ++ind) {
#if defined(DEBUG_PRINT_GATES)
          printf("Thread [%d] -> eject   : %s\n", ind, stackSim->gatesChain[stackSim->busy]->name.c_str());
#endif
          stackSim->eject();
          ++deal;
        }
        std::fill(starts.begin(), starts.end(), 0);
        std::fill(ranges.begin(), ranges.end(), 0);
        continue;
      }
    }
    ++i;
    wr.DumpVars(time);
  }

  // Присоединяем потоки
  std::fill(statuses.begin(), statuses.end(), ts_can_exit);
  for (int i = 0; i < cores; ++i)
    threads[i].join();

  delete stackSim;
}

void simulator::simulation_stack(netlist* netl, sim_data* simData, std::string filename, int stackSize) {
  // две переменные ниже задал с заделом на будущее, ибо в симуляторе можно будет выбирать, какой временной участок промоделировать (типа как в Active HDL)
  int initialTime = 0;                                                                                // время начала моделирования
  int stopTime = simData->eventChain[simData->eventChain.size() - 1].time + 10;                       // время окончания моделирования
  int time = 0;                                                                                       // непосредственно момент моделирования
                                                                                                      // размер стека
  int temp_free = 0;

  valueChanged = false;                                                                               // флаг изменения состояния на выходе вентиля
  datawriter wr(simData->getVCDname().c_str());                                                                    // контейнер выходных данных
  stack *stackSim = new stack(stackSize);                                                             // стек моделирования
  for (size_t i = 0; i < netl->nets.size(); i++)
    wr.AddDumpVar(netl->nets[i]);                                            // указываем контейнеру, значения каких узлов отслеживать

  wr.PrintHeader();                                                                                   // пишем в выходной файл шапку

  for (time = initialTime; time < stopTime; time++) {																							    // временная ось

    for (size_t i = 0; i < simData->eventChain.size(); i++) {                                         // пробежка по всем событиям
      if (simData->eventChain[i].time == time) {                                                      // если достигли времени данного события
        #if defined(DEBUG_PRINT_THREAD)
          printf("=== Time = [%d] ===\n", time);
        #endif
        for (size_t k = 0; k < netl->gates.size(); k++)																								// обнуляем счётчик повторений для всех вентилей
          netl->gates[k]->repeat = 0;
        for (size_t k = 0; k < netl->nets.size(); k++) {                                              // присваиваем значения портам
          for (size_t j = 0; j < simData->eventChain[i].netsChain.size(); j++) {
            if (netl->nets[k]->name == simData->eventChain[i].netsChain[j]->name)
              netl->nets[k]->value = simData->eventChain[i].statesChain[j];

          }
        }
        stackSim->reset();

        // инициируем начальную цепочку вентилей на основе изменения состояний на входах

        for (size_t l = 0; l < simData->eventChain[i].netsChain.size(); l++) {
          for (size_t m = 0; m < simData->eventChain[i].netsChain[l]->gates.size(); m++) {
            std::vector<gate*>::iterator it = std::find(stackSim->gatesChain.begin(), stackSim->gatesChain.end(), simData->eventChain[i].netsChain[l]->gates[m]);
            int iit = std::distance(stackSim->gatesChain.begin(), it);
            if (it != stackSim->gatesChain.end()) {
              if (iit > stackSim->free && iit < stackSim->busy)
                stackSim->push_back(simData->eventChain[i].netsChain[l]->gates[m]);
            }
            else {
              stackSim->push_back(simData->eventChain[i].netsChain[l]->gates[m]);
            }
          }
        }

        //
        // Вот тут-то и начинается самый сок
        //

        while (stackSim->busy != stackSim->free) {                                                                // операции сетей Петри
          if (stackSim->busy > stackSim->free)                                                                    // назначаем временный указатель free, на случай, если free < busy
            temp_free = stackSim->free + stackSize;
          else
            temp_free = stackSim->free;

          for (int index = stackSim->busy; index < temp_free; index++) {                                          // момент времени t- в сети Петри
            if (stackSim->gatesChain[index % stackSize]->repeat < 500)
              stackSim->gatesChain[index % stackSize]->t_minus();
          }

          for (int index = stackSim->busy; index < temp_free; index++) {                                          // момент времени t0 в сети Петри
            if (stackSim->gatesChain[index % stackSize]->repeat < 500)
              stackSim->gatesChain[index % stackSize]->operate();
          }

          for (int index = stackSim->busy; index < temp_free; index++) {                                          // момент времени t+ в сети Петри
            if (stackSim->gatesChain[index % stackSize]->repeat < 500) {
              bool valueChanged = false;
              if (stackSim->gatesChain[index % stackSize]->t_plus())
                valueChanged = true;
              stackSim->gatesChain[index % stackSize]->repeat++;                                                  // инкремент
              if (valueChanged) {                                                                                 // если флаг назначен, то добавляем вентили, висящие на выходе узла в стек
                for (size_t y = 0; y < stackSim->gatesChain[index % stackSize]->outs.size(); y++) {
                  std::vector <gate*> returned = netl->returnGate(stackSim->gatesChain[index % stackSize]->outs[y]);
                  if ((!returned.empty()) && (stackSim->gatesChain[index % stackSize]->repeat < 500))
                    for (size_t index = 0; index < returned.size(); index++)
                      stackSim->push_back(returned[index]);
                }
              }
              stackSim->eject();
            }
          }
        }
      }
    }

    wr.DumpVars(time);
  }
}