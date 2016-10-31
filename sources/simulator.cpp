#ifdef _WIN32
#pragma comment(lib, "../../lib/x86/pthreadVSE2.lib")
#elif defined __unix__
#pragma comment(lib, "../../lib/x86/libpthreadGCE2.a")
#elif defined __APPLE__
//do something for mac
#endif
#include "simulator.h"
#include "netlist.h"
#include "simulation_data.h"
#include "datawriter.h"
#include <iostream>
#include <algorithm>
#include <list>
#define HAVE_STRUCT_TIMESPEC
#include <thread>
#include "stack.h"
#include "numcores.h"

#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD   -12
#define BAD_MESSAGE         -13
#define SUCCESS               0


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
};

enum ThreadStatus {
  ts_wait_for_data = 0,   // поток должен ждать следующую порцию данных
  ts_work_on_data,        // поток может приступать к обработке порции данных
  ts_can_exit,            // поток может завершиться
};

std::vector<ThreadStatus>  statuses;    // 2 потока - 2 статуса (для каждого)
std::vector<gate*> simulating;
std::vector<OperationType> OpType;

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
  
  return SUCCESS;
}

void routine(int id) {
  std::cout << "[ " << id << " ] : Entering thread" << std::endl;
THREAD_BEGIN:
  std::cout << "[ " << id << " ] : Waiting..." << std::endl;
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
  std::cout << "[ " << id << " ] : Working on data" << std::endl;
  if(OpType[id] == t_minus)
    simulating[id]->t_minus();  
  if(OpType[id] == t_zero)
    simulating[id]->operate();  

  // Закончили обработку данных - сами себе ставим статус ожидания новых данных и переходим в цикл ожидания while
  simulating[id] = NULL;
  statuses[id] = ts_wait_for_data;
  goto THREAD_BEGIN;

THREAD_END:
  std::cout << "[ " << id << " ] : Exiting thread!" << std::endl;
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
            } else {
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
            if (stackSim->gatesChain[stackSim->busy]->repeat < 500) 
              stackSim->gatesChain[index % stackSize]->t_minus();
          }

          for (int index = stackSim->busy; index < temp_free; index++) {                                          // момент времени t0 в сети Петри
            if (stackSim->gatesChain[stackSim->busy]->repeat < 500) 
              stackSim->gatesChain[index % stackSize]->operate();
          }

          for (int index = stackSim->busy; index < temp_free; index++) {                                          // момент времени t+ в сети Петри
            if (stackSim->gatesChain[stackSim->busy]->repeat < 500) {
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


void simulator::simulation(netlist* netl, sim_data* simData, std::string filename, int stackSize) {
  int initialTime = 0;                                                                                // время начала моделирования
  int stopTime = simData->eventChain[simData->eventChain.size() - 1].time + 10;                       // время окончания моделирования
  int time = 0;                                                                                       // непосредственно момент моделирования

  int cores = getNumCores();                                                                          // получем количество ядер ЦП в системе
  printf("__inf__ : Available cores: %d\n", cores);                                                   // выводим на экран
  std::vector<someArgs_t> args;                                                                       // создаём вектор аргументов
  args.resize(cores);                                                                                 // по количеству ядер
  simulating.resize(cores);
  statuses.resize(cores);
  for (int i = 0; i < cores; i++)
    statuses[i] = ts_wait_for_data;
  OpType.resize(cores);
  std::vector<std::thread> threads;
  threads.reserve(cores);

  for (int i=0; i < cores; i++) {
    std::thread th(routine, i);
    threads.push_back(move(th));
  }
  
  std::vector<std::vector<gate*>*> returned;                                                            // создаём вектор возвращённых вентилей
  returned.resize(cores);                                                                             // по количеству ядер
  for (int y = 0; y < returned.size(); y++) {
    returned[y] = new std::vector<gate*>;
  }
                                                                                         // размер стека
  int temp_free = 0;

  std::string gateName;                                                                               // имя вентиля
  datawriter wr(simData->getVCDname().c_str());                                                                    // контейнер выходных данных
  stack *stackSim = new stack(stackSize);                                                             // стек моделирования
  for (size_t i = 0; i < netl->nets.size(); i++)
    wr.AddDumpVar(netl->nets[i]);                                            // указываем контейнеру, значения каких узлов отслеживать

  wr.PrintHeader();                                                                                   // пишем в выходной файл шапку

  for (time = initialTime; time < stopTime; time++) {																							    // временная ось

    for (size_t i = 0; i < simData->eventChain.size(); i++) {                                         // пробежка по всем событиям
      if (simData->eventChain[i].time == time) {                                                      // если достигли времени данного события
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

          int index = stackSim->busy;
          while(index < temp_free) {                                          // момент времени t- в сети Петри
            if (stackSim->gatesChain[index % stackSize]->repeat < 500) {
              for(int y = 0; y < cores; y++) {
                
                if (statuses[y] == ts_wait_for_data) {
                  simulating[y] = stackSim->gatesChain[index % stackSize];
                  OpType[y] = t_minus;
                  statuses[y] = ts_work_on_data;
                  index++;
                }
                if (index == temp_free) {
                  break;
                }
              }
            }

          }

          bool can_continue = false;
          while (!can_continue) {
            for (int ix = 0; ix < cores; ix++) {
              can_continue = true;
              if (statuses[ix] == ts_work_on_data) {
                can_continue = false;
                break;
              }
            }
          }


          for (int index = stackSim->busy; index < temp_free; index++) {                                          // момент времени t0 в сети Петри
            if (stackSim->gatesChain[stackSim->busy]->repeat < 500)
              stackSim->gatesChain[index % stackSize]->operate();
          }




          for (int index = stackSim->busy; index < temp_free; index++) {                                          // момент времени t+ в сети Петри
            if (stackSim->gatesChain[stackSim->busy]->repeat < 500) {
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

  for (int i = 0; i < cores; i++) {
    threads[i].join();
  }


}


/*
void simulator::simulation(netlist* netl, sim_data* simData, std::string filename, int stackSize) {
  // две переменные ниже задал с заделом на будущее, ибо в симуляторе можно будет выбирать, какой временной участок промоделировать (типа как в Active HDL)
  int initialTime = 0;                                                                                // время начала моделирования
  int stopTime = simData->eventChain[simData->eventChain.size() - 1].time + 10;                       // время окончания моделирования
  int time = 0;                                                                                       // непосредственно момент моделирования

  int cores = getNumCores();                                                                          // получем количество ядер ЦП в системе
  printf("__inf__ : Available cores: %d\n", cores);                                                   // выводим на экран
  std::vector<someArgs_t> args;                                                                       // создаём вектор аргументов
  args.resize(cores);                                                                                 // по количеству ядер

  std::vector<pthread_t> threads;                                                                     // создаём вектор потоков
  threads.resize(cores);                                                                              // по количеству ядер

  std::vector<std::vector<gate*>*> returned;                                                            // создаём вектор возвращённых вентилей
  returned.resize(cores);                                                                             // по количеству ядер
  for (int y = 0; y < returned.size(); y++) {
    returned[y] = new std::vector<gate*>;
  }

  int status;
  int status_addr;

  int num_threads = 0;

  valueChanged = false;                                                                               // флаг изменения состояния на выходе вентиля
  std::string gateName;                                                                               // имя вентиля
  //datawriter wr(filename.c_str());                                                                    // контейнер выходных данных
  datawriter wr(simData->getVCDname().c_str());                                                                    // контейнер выходных данных
  stack *stackSim = new stack(stackSize);                                                             // стек моделирования
  for (size_t i = 0; i < netl->nets.size(); i++)
    wr.AddDumpVar(netl->nets[i]);                                            // указываем контейнеру, значения каких узлов отслеживать

  wr.PrintHeader();                                                                                   // пишем в выходной файл шапку

  for (time = initialTime; time < stopTime; time++) {																							    // временная ось
    for (size_t i = 0; i < simData->eventChain.size(); i++) {                                         // пробежка по всем событиям
      if (simData->eventChain[i].time == time) {                                                      // если достигли времени данного события
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

            } else {
              stackSim->push_back(simData->eventChain[i].netsChain[l]->gates[m]);
            }
          }
        }

        int temp_free = 0;
        while (stackSim->busy != stackSim->free) {
          num_threads = 0;
          for (int core = 0; core < cores; core++) {
            returned[core] = new std::vector<gate*>;
            args[core].busy = stackSim->busy + core;
            args[core].simData = simData;
            args[core].netl = netl;
            args[core].stackSim = stackSim;
            args[core].returned = returned[core];
            args[core].returned->resize(100,NULL);
            if (stackSim->busy > stackSim->free)
              temp_free = stackSim->free + stackSize;
            else
              temp_free = stackSim->free;
            if (stackSim->busy + core < temp_free) {
              status = pthread_create(&threads[core], NULL, routine, (void*)&args[core]);
              num_threads++;
              if (status != 0) {
                printf("main error: can't create thread, status = %d\n", status);
                exit(ERROR_CREATE_THREAD);
              }
            }
          }
          for (int core = 0; core < num_threads; core++) {
            if (stackSim->busy > stackSim->free)
              temp_free = stackSim->free + stackSize;
            else
              temp_free = stackSim->free;
            status = pthread_join(threads[core], (void**)&status_addr);
            if (status != SUCCESS) {
              printf("main error: can't join thread, status = %d\n", status);
              exit(ERROR_JOIN_THREAD);
            }
            for (int yy = 0; yy < returned[core]->size(); yy++) {
              if ((*returned[core])[yy] == NULL)
                break;
              if ((*returned[core])[yy] != NULL) 
                stackSim->push_back((*returned[core])[yy]);
            }
            delete returned[core];
            stackSim->eject();
          }
        }




      }
    }

    wr.DumpVars(time);
  }
}
*/