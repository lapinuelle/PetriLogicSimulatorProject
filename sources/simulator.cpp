#include "simulator.h"
#include "netlist.h"
#include "simulation_data.h"
#include "datawriter.h"
#include "interpreter.h"
#include "stack.h"
#include <cstring>
#include <thread>

//#define DEBUG_PRINT_THREAD
//#define DEBUG_PRINT_GATES
//#define DEBUG_PRINT_CYCLE_TIME

simulator::simulator() {
}

void simulator::simulation(netlist* netl, sim_data* simData, int stackSize) {

  printf("          Single thread will be used.\n");
                                                                                                      // размер стека
  int temp_free = 0;

  valueChanged = false;                                                                               // флаг изменения состояния на выходе вентиля
  //int time = 0;
  datawriter wr(simData->getVCDname().c_str());                                                                    // контейнер выходных данных
  stack *stackSim = new stack(stackSize);                                                             // стек моделирования

  std::map<std::string, net*>::iterator it;
  for (it = netl->netsMap.begin(); it != netl->netsMap.end(); it++) {
    wr.AddDumpVar(it->second);
  }
  wr.SetTimescale(simData->timescale.str_timescale);

  /*
  for (size_t i = 0; i < netl->nets.size(); i++)
    wr.AddDumpVar(netl->nets[i]);                                            // указываем контейнеру, значения каких узлов отслеживать
    */

  wr.PrintHeader();                                                                                   // пишем в выходной файл шапку

  //for (size_t time = 0; time < simData->eventChain.size(); ++time) {
  for (int time = 0; time < simData->endTime; time++) {
    if (simData->newEventChain[time]) {
      printf("Current time: %d\n", time);
   

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
          if (!stateChanged) {
            if (((simData->newEventChain[time]->netsChain[i]->value == level_0) || (simData->newEventChain[time]->netsChain[i]->value == level_u)) && (simData->newEventChain[time]->statesChain[i] == level_1))
              simData->newEventChain[time]->netsChain[i]->stability = '\\';
            if (((simData->newEventChain[time]->netsChain[i]->value == level_1) || (simData->newEventChain[time]->netsChain[i]->value == level_u)) && (simData->newEventChain[time]->statesChain[i] == level_0))
              simData->newEventChain[time]->netsChain[i]->stability = '/';
            simData->newEventChain[time]->netsChain[i]->value = simData->newEventChain[time]->statesChain[i];

          }
        } else {
          if (((simData->newEventChain[time]->netsChain[i]->value == level_0) || (simData->newEventChain[time]->netsChain[i]->value == level_u)) && (simData->newEventChain[time]->statesChain[i] == level_1))
            simData->newEventChain[time]->netsChain[i]->stability = '\\';
          if (((simData->newEventChain[time]->netsChain[i]->value == level_1) || (simData->newEventChain[time]->netsChain[i]->value == level_u)) && (simData->newEventChain[time]->statesChain[i] == level_0))
            simData->newEventChain[time]->netsChain[i]->stability = '/';
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
          if (*stackSim->gatesChain[index % stackSize]->repeat < 500) {
            stackSim->gatesChain[index % stackSize]->t_minus();
          }
        }

        for (int index = stackSim->busy; index < temp_free; index++) {                                          // момент времени t0 в сети Петри
          if (*stackSim->gatesChain[index % stackSize]->repeat < 500) {
            gate *p_gate = stackSim->gatesChain[index % stackSize];
            if (p_gate->tokens.empty()) {
              p_gate->operate();
            } else {
              interpreter *interp = new interpreter();
              interp->operate(stackSim->gatesChain[index % stackSize], netl);
            }
          }
        }

        for (int index = stackSim->busy; index < temp_free; index++) {                                          // момент времени t+ в сети Петри
          if (*stackSim->gatesChain[index % stackSize]->repeat < 500) {
            if (stackSim->gatesChain[index % stackSize]->delay == 0) {
              bool valueChanged = stackSim->gatesChain[index % stackSize]->t_plus();
              if (valueChanged) {                                                                                 
                for (int y = 0, gatchsize = stackSim->gatesChain[index % stackSize]->outs.size(); y < gatchsize; ++y) {
                  std::vector <gate*> &returned = netl->returnGate(stackSim->gatesChain[index % stackSize]->outs[y]);
                  if (!returned.empty())
                    for (int indexx = 0, retsize = returned.size(); indexx < retsize; ++indexx) {
                      stackSim->push_back(returned[indexx]);
                    }
                }
              }
            } else {
              for (size_t outst = 0; outst < stackSim->gatesChain[index % stackSize]->outs.size(); outst++) {
                if (stackSim->gatesChain[index % stackSize]->outs[outst]->value != stackSim->gatesChain[index % stackSize]->outs_temp[outst]) {
                  Event *ev = (simData->addMapEvent(time + stackSim->gatesChain[index % stackSize]->delay, stackSim->gatesChain[index % stackSize]->outs[outst], stackSim->gatesChain[index % stackSize]->outs_temp[outst], true));
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
