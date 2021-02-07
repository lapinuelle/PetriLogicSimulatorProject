#include "simulator.h"
#include "netlist.h"
#include "simulation_data.h"
#include "interpreter.h"
#include "stack.h"
#include <cstring>
#include <thread>

//#define DEBUG_PRINT_THREAD
//#define DEBUG_PRINT_GATES
//#define DEBUG_PRINT_CYCLE_TIME

simulator::simulator() : p_wr(NULL) {
}

void simulator::simulation(netlist* netl, sim_data* simData, int stackSize, SDF *sdf) {

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
  //wr.SetTimescale(simData->timescale.str_timescale);
  wr.SetTimescale(simData->timescale.str_precision);

  if (sdf) {
    for (std::map<std::string, gate*>::iterator it = netl->gatesMap.begin(); it != netl->gatesMap.end(); ++it) {
      if (sdf->delays[(*it).second->getRealName()]) {
        (*it).second->setDelay(sdf->delays[(*it).second->getRealName()]);
      }
    }
  }

  /*
  for (size_t i = 0; i < netl->nets.size(); i++)
    wr.AddDumpVar(netl->nets[i]);                                            // указываем контейнеру, значения каких узлов отслеживать
    */

  wr.PrintHeader();                                                                                   // пишем в выходной файл шапку

  //for (size_t time = 0; time < simData->eventChain.size(); ++time) {
  //for (float time = 0; time < simData->endTime; time++) {
  for(std::map<float, Event*>::iterator itt = simData->newEventChain.begin(); itt != simData->newEventChain.end(); itt++ ) {
    float time = itt->first;
    printf("Current time: %f\n", time);
    if (simData->newEventChain[time]) {

  #if defined DEBUG_PRINT_CYCLE_TIME
      clock_t time_start = clock();
  #endif
      //printf("=== Time = [%d] ===\n", time);
      //memset(netl->repeats, 0, sizeof(int)*netl->gatesMap.size());

      // присваиваем новые входные воздействия из тестбенча портам
      
      for (size_t i = 0; i < simData->newEventChain[time]->netsChain.size(); ++i) {
        if (simData->newEventChain[time]->delayed[i]) {
          bool stateChanged = false;
          Event* eventCh = simData->newEventChain.find(time)->second;
          std::string assignNet = eventCh->netsChain[i]->getName();
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
              simData->newEventChain[time]->netsChain[i]->stability = '/';
            if (((simData->newEventChain[time]->netsChain[i]->value == level_1) || (simData->newEventChain[time]->netsChain[i]->value == level_u)) && (simData->newEventChain[time]->statesChain[i] == level_0))
              simData->newEventChain[time]->netsChain[i]->stability = '\\';
            simData->newEventChain[time]->netsChain[i]->value = simData->newEventChain[time]->statesChain[i];

          }
        } else {
          if (((simData->newEventChain[time]->netsChain[i]->value == level_0) || (simData->newEventChain[time]->netsChain[i]->value == level_u)) && (simData->newEventChain[time]->statesChain[i] == level_1))
            simData->newEventChain[time]->netsChain[i]->stability = '/';
          if (((simData->newEventChain[time]->netsChain[i]->value == level_1) || (simData->newEventChain[time]->netsChain[i]->value == level_u)) && (simData->newEventChain[time]->statesChain[i] == level_0))
            simData->newEventChain[time]->netsChain[i]->stability = '\\';
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
          if (stackSim->gatesChain[index % stackSize]->getRepeatCount() < 500) {
            stackSim->gatesChain[index % stackSize]->t_minus();
          } else {
            printf("T- > 500\n");
          }
        }

        for (int index = stackSim->busy; index < temp_free; index++) {                                          // момент времени t0 в сети Петри
          if (stackSim->gatesChain[index % stackSize]->getRepeatCount() < 500) {
            gate *p_gate = stackSim->gatesChain[index % stackSize];
            if (p_gate->getTokens().empty()) {
              p_gate->operate();
            } else {
              interpreter *interp = new interpreter();
              interp->operate(stackSim->gatesChain[index % stackSize], netl);
            }
          } else {
            printf("T0 > 500\n");
          }
        }

        for (int index = stackSim->busy; index < temp_free; index++) {                                          // момент времени t+ в сети Петри
          if (stackSim->gatesChain[index % stackSize]->getRepeatCount() < 500) {
            for (int hj = 0; hj < stackSim->gatesChain[index % stackSize]->getInputsCount(); hj++)
              stackSim->gatesChain[index % stackSize]->getInput(hj)->stability = "_";
            if (stackSim->gatesChain[index % stackSize]->getDelay() == 0) {
              bool valueChanged = stackSim->gatesChain[index % stackSize]->t_plus();
              if (valueChanged) {                                                                                 
                for (int y = 0, gatchsize = stackSim->gatesChain[index % stackSize]->getOutputsCount(); y < gatchsize; ++y) {
                  std::vector <gate*> &returned = netl->returnGate(stackSim->gatesChain[index % stackSize]->getOutput(y));
                  if (!returned.empty())
                    for (int indexx = 0, retsize = returned.size(); indexx < retsize; ++indexx) {
                      stackSim->push_back(returned[indexx]);
                    }
                }
              }
            } else {
              for (size_t outst = 0; outst < stackSim->gatesChain[index % stackSize]->getOutputsCount(); outst++) {
                if (stackSim->gatesChain[index % stackSize]->getOutput(outst)->value != stackSim->gatesChain[index % stackSize]->getInternalOutput(outst)) {
                  Event *ev = (simData->addMapEvent(time + stackSim->gatesChain[index % stackSize]->getDelay(), stackSim->gatesChain[index % stackSize]->getOutput(outst), stackSim->gatesChain[index % stackSize]->getInternalOutput(outst), true));
                  for (size_t ins = 0; ins < stackSim->gatesChain[index % stackSize]->getInputsCount(); ins++) {
                    (ev->inputStates[stackSim->gatesChain[index % stackSize]->getOutput(outst)->getName()]).push_back(stackSim->gatesChain[index % stackSize]->getInput(ins));
                    (ev->inputStatesValues[stackSim->gatesChain[index % stackSize]->getOutput(outst)->getName()]).push_back(stackSim->gatesChain[index % stackSize]->getInput(ins)->value);
                  }
                }
              }
            }
            stackSim->eject();
          } else {
            printf("T+ > 500\n");
          }
        }
      }

  #if defined DEBUG_PRINT_CYCLE_TIME
      clock_t time_end = clock();
      clock_t dump_start = clock();
  #endif
      wr.DumpVars((int)(time*1000.0));
  #if defined DEBUG_PRINT_CYCLE_TIME
      clock_t dump_end = clock();
      printf("Interval time: %5ld, Dump time: %3ld\n", time_end - time_start, dump_end - dump_start);
  #endif
    }
  }
}























void simulator::begin_multistep_mode(netlist* netl, sim_data* simData, int stackSize, SDF *sdf) {

  printf("          Single thread will be used.\n");
  // размер стека

  if(!sdf)
    p_wr = new datawriter(("1_" + simData->getVCDname()).c_str());                                                                    // контейнер выходных данных
  else
    p_wr = new datawriter(("2_" + simData->getVCDname()).c_str());                                                                    // контейнер выходных данных

  std::map<std::string, net*>::iterator it;
  for (it = netl->netsMap.begin(); it != netl->netsMap.end(); it++) {
    p_wr->AddDumpVar(it->second);
  }
  //wr.SetTimescale(simData->timescale.str_timescale);
  p_wr->SetTimescale(simData->timescale.str_precision);

  if (sdf) {
    for (std::map<std::string, gate*>::iterator it = netl->gatesMap.begin(); it != netl->gatesMap.end(); ++it) {
      if (sdf->delays[(*it).second->getRealName()]) {
        (*it).second->setDelay(sdf->delays[(*it).second->getRealName()]);
      }
    }
  }

  /*
  for (size_t i = 0; i < netl->nets.size(); i++)
  wr.AddDumpVar(netl->nets[i]);                                            // указываем контейнеру, значения каких узлов отслеживать
  */

  p_wr->PrintHeader();                                                                                   // пишем в выходной файл шапку
}

float simulator::make_one_step(netlist* netl, sim_data* simData, int stackSize, SDF *sdf, std::string signal, std::string edge) {
  int temp_free = 0;
  valueChanged = false;                                                                               // флаг изменения состояния на выходе вентиля
  stack *stackSim = new stack(stackSize);                                                             // стек моделирования
                                                                                                     //int time = 0;

  bool flagStopSim = false;
  float stopTime = 0.0f;

  float time = 0.0f;

  std::map<float, Event*>::iterator itt;
  for(itt = simData->newEventChain.begin(); itt != simData->newEventChain.end(); itt++) {
    if(sdf)
      if(flagStopSim && time >= stopTime)
        break;
    if(!sdf)
      if(flagStopSim)
        break;

    time = itt->first;
    if(!sdf)
      printf("Current time [1]: %f\n", time);
    else
      printf("Current time [2]: %f\n", time);
    if (simData->newEventChain[time]) {

#if defined DEBUG_PRINT_CYCLE_TIME
      clock_t time_start = clock();
#endif
      //printf("=== Time = [%d] ===\n", time);
      //memset(netl->repeats, 0, sizeof(int)*netl->gatesMap.size());

      // присваиваем новые входные воздействия из тестбенча портам
      for (size_t i = 0; i < simData->newEventChain[time]->netsChain.size(); ++i) {
        if (simData->newEventChain[time]->delayed[i]) {
          bool stateChanged = false;
          Event* eventCh = simData->newEventChain.find(time)->second;
          std::string assignNet = eventCh->netsChain[i]->getName();
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
              simData->newEventChain[time]->netsChain[i]->stability = '/';
            if (((simData->newEventChain[time]->netsChain[i]->value == level_1) || (simData->newEventChain[time]->netsChain[i]->value == level_u)) && (simData->newEventChain[time]->statesChain[i] == level_0))
              simData->newEventChain[time]->netsChain[i]->stability = '\\';
            simData->newEventChain[time]->netsChain[i]->value = simData->newEventChain[time]->statesChain[i];
            
            if(signal == simData->newEventChain[time]->netsChain[i]->realName)
              if ((simData->newEventChain[time]->netsChain[i]->stability == "/" && edge == "posedge") || (simData->newEventChain[time]->netsChain[i]->stability == "\\" && edge == "negedge")) {
                flagStopSim = true;
                stopTime = time + 0.175f;
              }
          }
        } else {
          if (((simData->newEventChain[time]->netsChain[i]->value == level_0) || (simData->newEventChain[time]->netsChain[i]->value == level_u)) && (simData->newEventChain[time]->statesChain[i] == level_1))
            simData->newEventChain[time]->netsChain[i]->stability = '/';
          if (((simData->newEventChain[time]->netsChain[i]->value == level_1) || (simData->newEventChain[time]->netsChain[i]->value == level_u)) && (simData->newEventChain[time]->statesChain[i] == level_0))
            simData->newEventChain[time]->netsChain[i]->stability = '\\';
          simData->newEventChain[time]->netsChain[i]->value = simData->newEventChain[time]->statesChain[i];

          if(signal == simData->newEventChain[time]->netsChain[i]->realName)
            if ((simData->newEventChain[time]->netsChain[i]->stability == "/" && edge == "posedge") || (simData->newEventChain[time]->netsChain[i]->stability == "\\" && edge == "negedge")) {
              flagStopSim = true;
              stopTime = time + 0.175f;
            }
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
          if (stackSim->gatesChain[index % stackSize]->getRepeatCount() < 500) {
            stackSim->gatesChain[index % stackSize]->t_minus();
          } else {
            printf("T- > 500\n");
          }
        }

        for (int index = stackSim->busy; index < temp_free; index++) {                                          // момент времени t0 в сети Петри
          if (stackSim->gatesChain[index % stackSize]->getRepeatCount() < 500) {
            gate *p_gate = stackSim->gatesChain[index % stackSize];
            if (p_gate->getTokens().empty()) {
              p_gate->operate();
            } else {
              interpreter *interp = new interpreter();
              interp->operate(stackSim->gatesChain[index % stackSize], netl);
            }
          } else {
            printf("T0 > 500\n");
          }
        }

        for (int index = stackSim->busy; index < temp_free; index++) {                                          // момент времени t+ в сети Петри
          if (stackSim->gatesChain[index % stackSize]->getRepeatCount() < 500) {
            for (int hj = 0; hj < stackSim->gatesChain[index % stackSize]->getInputsCount(); hj++)
              stackSim->gatesChain[index % stackSize]->getInput(hj)->stability = "_";
            if (stackSim->gatesChain[index % stackSize]->getDelay() == 0) {
              bool valueChanged = stackSim->gatesChain[index % stackSize]->t_plus();
              if (valueChanged) {                                                                                 
                for (int y = 0, gatchsize = stackSim->gatesChain[index % stackSize]->getOutputsCount(); y < gatchsize; ++y) {
                  std::vector <gate*> &returned = netl->returnGate(stackSim->gatesChain[index % stackSize]->getOutput(y));
                  if (!returned.empty())
                    for (int indexx = 0, retsize = returned.size(); indexx < retsize; ++indexx) {
                      stackSim->push_back(returned[indexx]);
                    }
                }
              }
            } else {
              for (size_t outst = 0; outst < stackSim->gatesChain[index % stackSize]->getOutputsCount(); outst++) {
                if (stackSim->gatesChain[index % stackSize]->getOutput(outst)->value != stackSim->gatesChain[index % stackSize]->getInternalOutput(outst)) {
                  Event *ev = (simData->addMapEvent(time + stackSim->gatesChain[index % stackSize]->getDelay(), stackSim->gatesChain[index % stackSize]->getOutput(outst), stackSim->gatesChain[index % stackSize]->getInternalOutput(outst), true));
                  for (size_t ins = 0; ins < stackSim->gatesChain[index % stackSize]->getInputsCount(); ins++) {
                    (ev->inputStates[stackSim->gatesChain[index % stackSize]->getOutput(outst)->getName()]).push_back(stackSim->gatesChain[index % stackSize]->getInput(ins));
                    (ev->inputStatesValues[stackSim->gatesChain[index % stackSize]->getOutput(outst)->getName()]).push_back(stackSim->gatesChain[index % stackSize]->getInput(ins)->value);
                  }
                }
              }
            }
            stackSim->eject();
          } else {
            printf("T+ > 500\n");
          }
        }
      }

#if defined DEBUG_PRINT_CYCLE_TIME
      clock_t time_end = clock();
      clock_t dump_start = clock();
#endif
      p_wr->DumpVars((int)(time*1000.0));
#if defined DEBUG_PRINT_CYCLE_TIME
      clock_t dump_end = clock();
      printf("Interval time: %5ld, Dump time: %3ld\n", time_end - time_start, dump_end - dump_start);
#endif
    }
    //break;
  }
  if(itt == simData->newEventChain.begin())
    simData->newEventChain.erase(simData->newEventChain.begin());
  else
    simData->newEventChain.erase(simData->newEventChain.begin(), itt);

  return time;
}

void simulator::end_multistep_mode() {

}