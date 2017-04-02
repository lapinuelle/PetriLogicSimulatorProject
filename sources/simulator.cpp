#include "simulator.h"
#include "netlist.h"
#include "simulation_data.h"
#include "datawriter.h"
#include "stack.h"
#include "numcores.h"
#include <thread>

//#define DEBUG_PRINT_THREAD
//#define DEBUG_PRINT_GATES
//#define DEBUG_PRINT_CYCLE_TIME

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

  //printf("");

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
      if (*stackSim->gatesChain[stackSim->busy + i]->repeat < 500) {
        bool valueChanged = stackSim->gatesChain[stackSim->busy + i]->t_plus();
        // если флаг назначен, то добавляем вентили, висящие на выходе узла, в стек
        if (valueChanged) {                                                                                 
          for (int y = 0, gatchsize = stackSim->gatesChain[stackSim->busy + i]->outs.size(); y < gatchsize; ++y) {
            std::vector <gate*> &returned = netli->returnGate(stackSim->gatesChain[stackSim->busy + i]->outs[y]);
            if (!returned.empty())
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

void simulator::simulation(netlist* netl, sim_data* simData, int stackSize) {
  int cores = getNumCores();
  //int cores = 4;
  printf("          Number of threads will be used: %d\n", cores);

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

  //for each(net* nett in netl->nets)
  //  wr.AddDumpVar(nett);
  for (size_t i = 0; i < netl->nets.size(); ++i)
    wr.AddDumpVar(netl->nets[i]);                                                                    

  wr.PrintHeader();

  for (size_t time = 0; time < simData->eventChain.size(); ++time) {
#if defined DEBUG_PRINT_CYCLE_TIME
      clock_t time_start = clock();
#endif
    //printf("=== Time = [%d] ===\n", time);
    memset(netl->repeats, 0, sizeof(int)*netl->gates.size());

    // присваиваем новые входные воздействия из тестбенча портам
    for(size_t i = 0; i < simData->eventChain[time].netsChain.size(); ++i)
      simData->eventChain[time].netsChain[i]->value = simData->eventChain[time].statesChain[i];

    stackSim->reset();

    // инициируем начальную цепочку вентилей на основе изменения состояний на входах

    for (int l = 0, ecncsize = simData->eventChain[time].netsChain.size(); l < ecncsize; l++)
      for (int m = 0, ecncgsize = simData->eventChain[time].netsChain[l]->gates.size(); m < ecncgsize; m++)
          stackSim->push_back(simData->eventChain[time].netsChain[l]->gates[m]);

    //
    // Вот тут-то и начинается самый сок
    //

    // Работают сети Петри
    while (stackSim->busy != stackSim->free) {                                                                
      bool allEqual = true;
      for (int core = 0; core < cores; ++core) {
        allEqual = ts_wait_for_data == statuses[core];
        __asm nop;
        if(!allEqual)
          break;
      }
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
#if defined(DEBUG_PRINT_THREAD)
        printf("t_minus : ");
#endif
        std::fill(statuses.begin(), statuses.end(), ts_work_on_data);
#if defined(DEBUG_PRINT_THREAD)
        printf("wait->work\n");
#endif
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
#if defined(DEBUG_PRINT_THREAD)
        printf("t_plus : ");
#endif
        std::fill(statuses.begin(), statuses.end(), ts_work_on_data);
#if defined(DEBUG_PRINT_THREAD)
        printf("wait->work\n");
#endif
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
#if defined DEBUG_PRINT_CYCLE_TIME
    clock_t time_end = clock();
    clock_t dump_start = clock();
#endif
    wr.DumpVars(time);
#if defined DEBUG_PRINT_CYCLE_TIME
    clock_t dump_end = clock();
    printf("Interval time: %5ld, Dump time: %3ld\n", time_end - time_start, dump_end - dump_start);
#endif
  }

  // Присоединяем потоки
  std::fill(statuses.begin(), statuses.end(), ts_can_exit);
  for (int i = 0; i < cores; ++i)
    threads[i].join();

  delete stackSim;
}

void simulator::simulation_stack(netlist* netl, sim_data* simData, int stackSize) {
  // две переменные ниже задал с заделом на будущее, ибо в симуляторе можно будет выбирать, какой временной участок промоделировать (типа как в Active HDL)
  printf("          Single thread will be used.\n");
                                                                                                      // размер стека
  int temp_free = 0;

  valueChanged = false;                                                                               // флаг изменения состояния на выходе вентиля
  //int time = 0;
  datawriter wr(simData->getVCDname().c_str());                                                                    // контейнер выходных данных
  stack *stackSim = new stack(stackSize);                                                             // стек моделирования
  for (size_t i = 0; i < netl->nets.size(); i++)
    wr.AddDumpVar(netl->nets[i]);                                            // указываем контейнеру, значения каких узлов отслеживать

  wr.PrintHeader();                                                                                   // пишем в выходной файл шапку

  //for (size_t time = 0; time < simData->eventChain.size(); ++time) {
  for (int time = 0; time < simData->endTime; time++) {
    if (simData->newEventChain[time]) {
   

  #if defined DEBUG_PRINT_CYCLE_TIME
      clock_t time_start = clock();
  #endif
      //printf("=== Time = [%d] ===\n", time);
      memset(netl->repeats, 0, sizeof(int)*netl->gates.size());

      // присваиваем новые входные воздействия из тестбенча портам
      
      for (size_t i = 0; i < simData->newEventChain[time]->netsChain.size(); ++i) {
        if (simData->newEventChain[time]->delayed[i]) {
          bool stateChanged = false;
          Event* eventCh = simData->newEventChain.find(time)->second;
          std::string assignNet = eventCh->netsChain[i]->name;
          std::vector<net*> evInputs = eventCh->inputStates.find(assignNet)->second;
          std::vector<LogicLevel> evInputsValues = eventCh->inputStatesValues.find(assignNet)->second;
          for (size_t inp = 0; inp < evInputs.size(); inp++) {
            if (evInputs[inp]->value != evInputsValues[inp]) {
              stateChanged = true;
              break;
            }
          }
          if (!stateChanged)
            simData->newEventChain[time]->netsChain[i]->value = simData->newEventChain[time]->statesChain[i];
        } else {
          simData->newEventChain[time]->netsChain[i]->value = simData->newEventChain[time]->statesChain[i];
        }
      }

      stackSim->reset();

          // инициируем начальную цепочку вентилей на основе изменения состояний на входах

      for (int l = 0, ecncsize = simData->newEventChain[time]->netsChain.size(); l < ecncsize; l++)
        for (int m = 0, ecncgsize = simData->newEventChain[time]->netsChain[l]->gates.size(); m < ecncgsize; m++)
          stackSim->push_back(simData->newEventChain[time]->netsChain[l]->gates[m]);

      //
      // Вот тут-то и начинается самый сок
      //

      while (stackSim->busy != stackSim->free) {                                                                // операции сетей Петри
        if (stackSim->busy > stackSim->free)                                                                    // назначаем временный указатель free, на случай, если free < busy
          temp_free = stackSim->free + stackSize;
        else
          temp_free = stackSim->free;

        for (int index = stackSim->busy; index < temp_free; index++) {                                          // момент времени t- в сети Петри
          if (*stackSim->gatesChain[index % stackSize]->repeat < 500)
            stackSim->gatesChain[index % stackSize]->t_minus();
        }

        for (int index = stackSim->busy; index < temp_free; index++) {                                          // момент времени t0 в сети Петри
          if (*stackSim->gatesChain[index % stackSize]->repeat < 500)
            stackSim->gatesChain[index % stackSize]->operate();
        }

        for (int index = stackSim->busy; index < temp_free; index++) {                                          // момент времени t+ в сети Петри
          if (*stackSim->gatesChain[index % stackSize]->repeat < 500) {
            if (stackSim->gatesChain[index % stackSize]->delay == 0) {
              bool valueChanged = stackSim->gatesChain[index % stackSize]->t_plus();
              if (valueChanged) {                                                                                 
                for (int y = 0, gatchsize = stackSim->gatesChain[index % stackSize]->outs.size(); y < gatchsize; ++y) {
                  std::vector <gate*> &returned = netl->returnGate(stackSim->gatesChain[index % stackSize]->outs[y]);
                  if (!returned.empty())
                    for (int index = 0, retsize = returned.size(); index < retsize; ++index) 
                      stackSim->push_back(returned[index]);
                }
              }
            } else {
              for (size_t outst = 0; outst < stackSim->gatesChain[index % stackSize]->outs.size(); outst++) {
                if (stackSim->gatesChain[index % stackSize]->outs[outst]->value != stackSim->gatesChain[index % stackSize]->outs_temp[outst]) {
                  Event *ev = (simData->addMapEvent(time + stackSim->gatesChain[index % stackSize]->delay, stackSim->gatesChain[index % stackSize]->outs[outst], stackSim->gatesChain[index % stackSize]->outs_temp[outst], true));
                  //ev->delayed.push_back(true);
                  for (size_t ins = 0; ins < stackSim->gatesChain[index % stackSize]->ins.size(); ins++) {
                    (ev->inputStates[stackSim->gatesChain[index % stackSize]->outs[outst]->name]).push_back(stackSim->gatesChain[index % stackSize]->ins[ins]);
                    (ev->inputStatesValues[stackSim->gatesChain[index % stackSize]->outs[outst]->name]).push_back(stackSim->gatesChain[index % stackSize]->ins[ins]->value);
                  }
                  

                }
              }
            }
            stackSim->eject();
          }
        }
      }

  #if defined DEBUG_PRINT_CYCLE_TIME
      clock_t time_end = clock();
      clock_t dump_start = clock();
  #endif
      wr.DumpVars(time);
  #if defined DEBUG_PRINT_CYCLE_TIME
      clock_t dump_end = clock();
      printf("Interval time: %5ld, Dump time: %3ld\n", time_end - time_start, dump_end - dump_start);
  #endif
    }
  }
}
