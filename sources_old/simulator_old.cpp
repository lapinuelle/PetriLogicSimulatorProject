#include "simulator.h"
#include "netlist.h"
#include "simulation_data.h"
#include "datawriter.h"
#include <iostream>
#include <algorithm>
#include <list>
#define HAVE_STRUCT_TIMESPEC
#include <thread>
#include <mutex>
#include "stack.h"
#include "numcores.h"


typedef struct someArgs_tag {
  int busy;
  sim_data* simData;
  netlist* netl;
  stack* stackSim;
  std::vector<gate*>* returned;
} someArgs_t;

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
netlist* netli;
int deal = 0;

simulator::simulator() {
}



void* routine_old(void *args) {
  someArgs_t *arg = (someArgs_t*)args;
  bool valueChanged = false;
  if (arg->stackSim->gatesChain[arg->busy]->repeat < 500) {                                             // стандартные операции алгоритма сетей Петри
    arg->stackSim->gatesChain[arg->busy]->t_minus();
    arg->stackSim->gatesChain[arg->busy]->operate();
    if (arg->stackSim->gatesChain[arg->busy]->t_plus())
      valueChanged = true;
    arg->stackSim->gatesChain[arg->busy]->repeat++;                                                     // инкремент
  }
  if (valueChanged) {                                                                                   // если флаг назначен, то добавляем вентили, висящие на выходе узла в стек
    for (size_t y = 0; y < arg->stackSim->gatesChain[arg->busy]->outs.size(); y++) {
      std::vector <gate*> returned = arg->netl->returnGate(arg->stackSim->gatesChain[arg->busy]->outs[y]);
      if ((!returned.empty()) && (arg->stackSim->gatesChain[arg->busy]->repeat < 500))
        for (size_t index = 0; index < returned.size(); index++)
          (*arg->returned)[index] = returned[index];
    }
  }
  
  return 0;
}

void routine(int id, stack *stackSim) {
  //std::cout << "[ " << id << " ] : Entering thread" << std::endl;
THREAD_BEGIN:
  //std::cout << "[ " << id << " ] : Waiting..." << std::endl;
  // Ждём, пока из функции main нам не установят статус либо продолжения работы, либо выхода 
  bool can_continue = false;
  while (!can_continue)
    can_continue = (statuses[id] != ts_wait_for_data);

  // Если мы вышли из while, значит либо работем, либо выходим
  // Просят выйти - выходим
  if (statuses[id] == ts_can_exit)
    goto THREAD_END;
  
  // Просят работать - работаем
  // Тип выполняемой работы будет зависеть от момента времени в сети Петри
  //std::cout << "[ " << id << " ] : Working on data" << std::endl;

  if (OpType == t_minus) {
    for (int i = starts[id]; i < ranges[id]; i++) {
      //printf("Thread [%d] -> t_minus : %s\n", id, stackSim->gatesChain[stackSim->busy + i]->name.c_str());
      stackSim->gatesChain[stackSim->busy + i]->t_minus();
      mut.lock();
        deal++;
      mut.unlock();
    }
  }
  if (OpType == t_zero) {
    for (int i = starts[id]; i < ranges[id]; i++) {
      //printf("Thread [%d] -> operate : %s\n", id, stackSim->gatesChain[stackSim->busy + i]->name.c_str());
      stackSim->gatesChain[stackSim->busy + i]->operate();
      mut.lock();
        deal++;
      mut.unlock();
    }
  }
  if (OpType == t_plus) {
    for (int i = starts[id]; i < ranges[id]; i++) {
      //printf("Thread [%d] -> t_plus  : %s\n", id, stackSim->gatesChain[stackSim->busy + i]->name.c_str());
      if (stackSim->gatesChain[stackSim->busy + i]->repeat < 500) {
        bool valueChanged = false;
        if (stackSim->gatesChain[stackSim->busy + i]->t_plus())
          valueChanged = true;
        stackSim->gatesChain[stackSim->busy + i]->repeat++;                                                  // инкремент
        if (valueChanged) {                                                                                 // если флаг назначен, то добавляем вентили, висящие на выходе узла в стек
          for (size_t y = 0; y < stackSim->gatesChain[stackSim->busy + i]->outs.size(); y++) {
            std::vector <gate*> returned = netli->returnGate(stackSim->gatesChain[stackSim->busy + i]->outs[y]);
            if ((!returned.empty()) && (stackSim->gatesChain[stackSim->busy + i]->repeat < 500))
              for (size_t index = 0; index < returned.size(); index++)
                stackSim->push_back(returned[index]);
          }
        }
      }
      mut.lock();
      deal++;
      mut.unlock();
    }
  }
  // Закончили обработку данных - сами себе ставим статус ожидания новых данных и переходим в цикл ожидания while
  statuses[id] = ts_wait_for_data;
  
  goto THREAD_BEGIN;

THREAD_END:;
  //std::cout << "[ " << id << " ] : Exiting thread!" << std::endl;
}

void simulator::simulation(netlist* netl, sim_data* simData, std::string filename, int stackSize) {
  int initialTime = 0;                                                                                // время начала моделирования
  int stopTime = simData->eventChain[simData->eventChain.size() - 1].time + 10;                       // время окончания моделирования
  int time = 0;                                                                                       // непосредственно момент моделирования

  int cores = getNumCores();                                                                          // получем количество ядер ЦП в системе
  //int cores = 4;
  printf("__inf__ : Available cores: %d\n", cores);                                                   // выводим на экран
  std::vector<someArgs_t> args;                                                                       // создаём вектор аргументов
  args.resize(cores);                                                                                 // по количеству ядер
  int tempSize = 0;
  statuses.resize(cores);
  for (int i = 0; i < cores; i++)
    statuses[i] = ts_wait_for_data;
  OpType = t_none;
  
  std::vector<std::thread> threads;
  threads.reserve(cores);

  ranges.resize(cores);
  starts.resize(cores);

  netli = netl;
  stack *stackSim = new stack(stackSize);                                                             // стек моделирования
  for (int i=0; i < cores; i++) {
    std::thread th(routine, i, stackSim);
    threads.push_back(move(th));
  }
  
  //std::vector<std::vector<gate*>*> returned;                                                            // создаём вектор возвращённых вентилей
  //returned.resize(cores);                                                                             // по количеству ядер
  //for (size_t y = 0; y < returned.size(); y++) {
  //  returned[y] = new std::vector<gate*>;
  //}
                                                                                         // размер стека
  int temp_free = 0;

  std::string gateName;                                                                               // имя вентиля
  datawriter wr(simData->getVCDname().c_str());                                                                    // контейнер выходных данных
  //stack *stackSim = new stack(stackSize);                                                             // стек моделирования
  for (size_t i = 0; i < netl->nets.size(); i++)
    wr.AddDumpVar(netl->nets[i]);                                            // указываем контейнеру, значения каких узлов отслеживать

  wr.PrintHeader();                                                                                   // пишем в выходной файл шапку

  for (time = initialTime; time < stopTime; time++) {																							    // временная ось

    for (size_t i = 0; i < simData->eventChain.size(); i++) {                                         // пробежка по всем событиям
      if (simData->eventChain[i].time == time) {  
        //printf("=== Time = [%d] ===\n", time);// если достигли времени данного события
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
          bool allEqual = false;
          for (int core = 0; core < cores; core++) {
            allEqual = ts_wait_for_data == statuses[core];
            //BUG : Потенциальный косяк! А что, если [0] == false, а [1] == true?
            //SOLUTION:
            if(!allEqual)
              break;
          }
          if (!allEqual)
            continue;
          if (stackSim->busy > stackSim->free)                                                                    // назначаем временный указатель free, на случай, если free < busy
            temp_free = stackSim->free + stackSize;
          else
            temp_free = stackSim->free;

          int stackFilled = temp_free - stackSim->busy;

         // if (allEqual) {

            for (int core = 0; core < cores; core++) {
              starts[core] = 0;
              ranges[core] = 0;
            }

            // раскидаем весь стек по потокам. Сами указатели не двигаем, передаём лишь интервалы
            //starts[0] = 0;

            if (stackFilled < cores) {
              for (int si = 1; si < stackFilled; si++) {
                starts[si] = starts[si-1] + 1;
              }
              ranges[0] = 1;
              for (int si = 1; si < stackFilled; si++) {
                ranges[si] = ranges[si - 1] + 1;
              }
              
            } else {
              int basicSize = (stackFilled-(stackFilled % cores)) / cores;
              starts[0] = 0;
              for(int si = 1; si < cores; si++)
                starts[si] = starts[si-1] + basicSize;
              ranges[0] = basicSize;
              for(int si = 1; si < cores; si++)
                ranges[si] = ranges[si-1] + basicSize;
              ranges[cores-1] += stackFilled % cores;
            }
          //}
          

          if (allEqual && (deal == tempSize * 4)) {
            deal = 0;
            tempSize = stackFilled;
            OpType = t_minus;
            for (int core = 0; core < cores; core++)
              statuses[core] = ts_work_on_data;
            // ???
            continue;
          }

          if (allEqual && (deal == tempSize)) {
            OpType = t_zero;
            for (int core = 0; core < cores; core++)
              statuses[core] = ts_work_on_data;
            // ???
            continue;
          }
          if (allEqual && (deal == tempSize * 2)) {
            OpType = t_plus;
            for (int core = 0; core < cores; core++)
              statuses[core] = ts_work_on_data;
            // ???
            continue;
          }

          if (allEqual && (deal == tempSize * 3)) {
            for (int ind = 0; ind < tempSize; ind++) {
             //printf("Thread [%d] -> eject   : %s\n", ind, stackSim->gatesChain[stackSim->busy]->name.c_str());
              stackSim->eject();

              deal++;
            }
            for (int core = 0; core < cores; core++) {
              starts[core] = 0;
              ranges[core] = 0;
            }
            // ???
            continue;
          }
        }
      }
    }

    wr.DumpVars(time);
  }
  for (int i = 0; i < cores; i++) {
    statuses[i] = ts_can_exit;
  }
  for (int i = 0; i < cores; i++) {
    threads[i].join();
  }


}

void simulator::simulation_stack(netlist* netl, sim_data* simData, std::string filename, int stackSize) {
  // две переменные ниже задал с заделом на будущее, ибо в симуляторе можно будет выбирать, какой временной участок промоделировать (типа как в Active HDL)
  int initialTime = 0;                                                                                // время начала моделирования
  int stopTime = simData->eventChain[simData->eventChain.size() - 1].time + 10;                       // время окончания моделирования
  int time = 0;                                                                                       // непосредственно момент моделирования
                                                                                                      // размер стека
  int temp_free = 0;

  valueChanged = false;                                                                               // флаг изменения состояния на выходе вентиля
  std::string gateName;                                                                               // имя вентиля
  datawriter wr(simData->getVCDname().c_str());                                                                    // контейнер выходных данных
  stack *stackSim = new stack(stackSize);                                                             // стек моделирования
  for (size_t i = 0; i < netl->nets.size(); i++)
    wr.AddDumpVar(netl->nets[i]);                                            // указываем контейнеру, значения каких узлов отслеживать

  wr.PrintHeader();                                                                                   // пишем в выходной файл шапку

  for (time = initialTime; time < stopTime; time++) {																							    // временная ось

    for (size_t i = 0; i < simData->eventChain.size(); i++) {                                         // пробежка по всем событиям
      if (simData->eventChain[i].time == time) {                                                      // если достигли времени данного события
        printf("=== Time = [%d] ===\n", time);
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