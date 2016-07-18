#include "simulator.h"
#include "netlist.h"
#include "simulation_data.h"
#include "datawriter.h"
#include <iostream>
#include <algorithm>
#include <list>
#include "stack.h"

simulator::simulator() {
}


void simulator::simulation_stack(netlist* netl, sim_data* simData, std::string filename, int stackSize) {
  // две переменные ниже задал с заделом на будущее, ибо в симуляторе можно будет выбирать, какой временной участок промоделировать (типа как в Active HDL)
  int initialTime = 0;                                                                                // время начала моделирования
  int stopTime = simData->eventChain[simData->eventChain.size() - 1].time + 10;                       // время окончания моделирования
  int time = 0;                                                                                       // непосредственно момент моделирования
                                                                                                      // размер стека

  valueChanged = false;                                                                               // флаг изменения состояния на выходе вентиля
  std::string gateName;                                                                               // имя вентиля
  datawriter wr(filename.c_str());                                                                    // контейнер выходных данных
  stack *stackSim = new stack(stackSize);                                                             // стек моделирования
//  for (size_t i = 0; i < simData->dumpNames.size(); i++)
//    wr.AddDumpVar(netl->returnNet(simData->dumpNames[i]));                                            // указываем контейнеру, значения каких узлов отслеживать
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
        
        while (stackSim->busy != stackSim->free) {
          valueChanged = false;
          simData->outs_temp.resize(stackSim->gatesChain[stackSim->busy]->outs.size());									        // этот блок отвечает за создание временного хранилища 
          for (size_t it = 0; it < simData->outs_temp.size(); it++) {																						// текущего состояния на выходе вентиля
            simData->outs_temp[it] = stackSim->gatesChain[stackSim->busy]->outs[it]->value;
          }

          if (stackSim->gatesChain[stackSim->busy]->repeat < 500) {                                             // стандартные операции алгоритма сетей Петри
            stackSim->gatesChain[stackSim->busy]->t_minus();
            stackSim->gatesChain[stackSim->busy]->operate();
            stackSim->gatesChain[stackSim->busy]->t_plus();
            stackSim->gatesChain[stackSim->busy]->repeat++;                                                     // инкремент
          }

          for (size_t it = 0; it < simData->outs_temp.size(); it++) {																						// если хоть один выход поменялся, то назначаем флаг
            if (stackSim->gatesChain[stackSim->busy]->outs[it]->value != simData->outs_temp[it])
              valueChanged = true;
          }

          if (valueChanged) {                                                                                   // если флаг назначен, то добавляем вентили, висящие на выходе узла в стек
            for (size_t y = 0; y < stackSim->gatesChain[stackSim->busy]->outs.size(); y++) {
              std::vector <gate*> returned = netl->returnGate(stackSim->gatesChain[stackSim->busy]->outs[y]);
              if ((!returned.empty()) && (stackSim->gatesChain[stackSim->busy]->repeat < 500))
              for (size_t index = 0; index < returned.size(); index++)
                stackSim->push_back(returned[index]);
            }
          }

          stackSim->eject();
          /*if (stackSim->busy == stackSim->free && stackSim->busy > 0 && !valueChanged) {
            stackSim->busy --;
            
          }*/


        }
        



      }
    }
    
    wr.DumpVars(time);
  }
}


void simulator::simulation(netlist* netl, sim_data* simData, std::string filename) {
	/*
  valueChanged = false;
	std::string gateName;
	datawriter wr(filename.c_str());

	for (size_t i = 0; i < simData->dumpNames.size(); i++)
		wr.AddDumpVar(netl->returnNet(simData->dumpNames[i]));

	wr.PrintHeader();

	// две переменные ниже задал с заделом на будущее, ибо в симуляторе можно будет выбирать, какой временной участок промоделировать (типа как в Active HDL)
	int initialTime = 0;                                                                                // время начала моделирования
	int stopTime = simData->eventChain[simData->eventChain.size() - 1].time + 10;                 // время окончания моделирования
  
	for (int time = initialTime; time < stopTime; time++) {																							// временная ось
		for (size_t i=0; i< simData->eventChain.size(); i++) {                                         // пробежка по всем событиям
			if (simData->eventChain[i].time==time) {                                                     // если достигли времени данного события
				for (size_t k=0; k < netl->gates.size(); k++)																									// обнуляем счётчик повторений для всех вентилей
					netl->gates[k]->repeat = 0;
				for (size_t k=0; k<netl->nets.size(); k++) {                                                  // присваиваем значения портам
					for (size_t j=0; j < simData->eventChain[i].netsChain.size(); j++) {
						if (netl->nets[k]->name==simData->eventChain[i].netsChain[j]->name)
							netl->nets[k]->value = simData->eventChain[i].statesChain[j];
						
					}
				}
				
				// инициируем начальную цепочку вентилей на основе изменения состояний на входах
				for (size_t l=0; l<simData->eventChain[i].netsChain.size(); l++) {
					for (size_t m = 0; m < simData->eventChain[i].netsChain[l]->gates.size(); m++) {
						if (std::find(simData->eventChain[i].gatesChain.begin(), simData->eventChain[i].gatesChain.end(), simData->eventChain[i].netsChain[l]->gates[m]) == simData->eventChain[i].gatesChain.end())
							simData->eventChain[i].gatesChain.push_back(simData->eventChain[i].netsChain[l]->gates[m]);
					}
				}


				// новый цикл
				while (!simData->eventChain[i].gatesChain.empty()) {
					simData->outs_temp.resize(simData->eventChain[i].gatesChain.front()->outs.size());									// этот блок отвечает за создание временного хранилища 
					for (size_t it = 0; it < simData->outs_temp.size(); it++) {																						// текущено состояния на выходе вентиля
						simData->outs_temp[it] = simData->eventChain[i].gatesChain.front()->outs[it]->value;
					}

						for(
							std::list<gate*>::iterator currentGate = simData->eventChain[i].gatesChain.begin();
								currentGate != simData->eventChain[i].gatesChain.end();
								currentGate ++)
									if ((*currentGate)->repeat < 500)
										(*currentGate)->t_minus();
						for(
								std::list<gate*>::iterator currentGate = simData->eventChain[i].gatesChain.begin();
								currentGate != simData->eventChain[i].gatesChain.end();
								currentGate ++)
									if ((*currentGate)->repeat < 500)
										(*currentGate)->operate();
						for(
								std::list<gate*>::iterator currentGate = simData->eventChain[i].gatesChain.begin();
								currentGate != simData->eventChain[i].gatesChain.end();
								currentGate ++)
									if ((*currentGate)->repeat < 500)
										(*currentGate)->t_plus();
						for(
								std::list<gate*>::iterator currentGate = simData->eventChain[i].gatesChain.begin();
								currentGate != simData->eventChain[i].gatesChain.end();
								currentGate ++)
							(*currentGate)->repeat++;
		
					for (size_t it = 0; it < simData->outs_temp.size(); it++) {																						// если хоть один выход поменялся, то назначаем флаг
						if (simData->eventChain[i].gatesChain.front()->outs[it]->value != simData->outs_temp[it])
							valueChanged = true;
					}

					if (valueChanged) {
						for (size_t y = 0; y<simData->eventChain[i].gatesChain.front()->outs.size(); y++) {
							if (!(netl->returnGate(simData->eventChain[i].gatesChain.front()->outs[y]).empty()) && (simData->eventChain[i].gatesChain.front()->repeat < 500)) 
								simData->eventChain[i].gatesChain.splice(simData->eventChain[i].gatesChain.end(), netl->returnGate(simData->eventChain[i].gatesChain.front()->outs[y]));
						}
					}
					
					simData->eventChain[i].gatesChain.pop_front();																													// удаляем промоделированный вентиль
				}	
			}
		}

		wr.DumpVars(time);
	}
  */
}
