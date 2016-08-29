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
#include <pthread.h>
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

simulator::simulator() {
}



void* routine(void *args) {
  someArgs_t *arg = (someArgs_t*)args;
  printf("__sys__ : Thread number %d launched\n", arg->busy);
  bool valueChanged = false;
  arg->simData->outs_temp.resize(arg->stackSim->gatesChain[arg->busy]->outs.size());									        // ���� ���� �������� �� �������� ���������� ��������� 
  for (size_t it = 0; it < arg->simData->outs_temp.size(); it++) {																						// �������� ��������� �� ������ �������
    arg->simData->outs_temp[it] = arg->stackSim->gatesChain[arg->busy]->outs[it]->value;
  }
  if (arg->stackSim->gatesChain[arg->busy]->repeat < 500) {                                             // ����������� �������� ��������� ����� �����
    arg->stackSim->gatesChain[arg->busy]->t_minus();
    arg->stackSim->gatesChain[arg->busy]->operate();
    arg->stackSim->gatesChain[arg->busy]->t_plus();
    arg->stackSim->gatesChain[arg->busy]->repeat++;                                                     // ���������
  }
  for (size_t it = 0; it < arg->simData->outs_temp.size(); it++) {																						// ���� ���� ���� ����� ���������, �� ��������� ����
    if (arg->stackSim->gatesChain[arg->busy]->outs[it]->value != arg->simData->outs_temp[it])
      valueChanged = true;
  }
  if (valueChanged) {                                                                                   // ���� ���� ��������, �� ��������� �������, ������� �� ������ ���� � ����
    for (size_t y = 0; y < arg->stackSim->gatesChain[arg->busy]->outs.size(); y++) {
      std::vector <gate*> returned = arg->netl->returnGate(arg->stackSim->gatesChain[arg->busy]->outs[y]);
      if ((!returned.empty()) && (arg->stackSim->gatesChain[arg->busy]->repeat < 500))
        for (size_t index = 0; index < returned.size(); index++)
          arg->stackSim->push_back(returned[index]);
    }
  }
  
  return SUCCESS;
}

void simulator::simulation_stack(netlist* netl, sim_data* simData, std::string filename, int stackSize) {
  // ��� ���������� ���� ����� � ������� �� �������, ��� � ���������� ����� ����� ��������, ����� ��������� ������� ��������������� (���� ��� � Active HDL)
  int initialTime = 0;                                                                                // ����� ������ �������������
  int stopTime = simData->eventChain[simData->eventChain.size() - 1].time + 10;                       // ����� ��������� �������������
  int time = 0;                                                                                       // ��������������� ������ �������������
                                                                                                      // ������ �����

  valueChanged = false;                                                                               // ���� ��������� ��������� �� ������ �������
  std::string gateName;                                                                               // ��� �������
  datawriter wr(filename.c_str());                                                                    // ��������� �������� ������
  stack *stackSim = new stack(stackSize);                                                             // ���� �������������
//  for (size_t i = 0; i < simData->dumpNames.size(); i++)
//    wr.AddDumpVar(netl->returnNet(simData->dumpNames[i]));                                            // ��������� ����������, �������� ����� ����� �����������
  for (size_t i = 0; i < netl->nets.size(); i++)
    wr.AddDumpVar(netl->nets[i]);                                            // ��������� ����������, �������� ����� ����� �����������

  wr.PrintHeader();                                                                                   // ����� � �������� ���� �����
  
  for (time = initialTime; time < stopTime; time++) {																							    // ��������� ���

    for (size_t i = 0; i < simData->eventChain.size(); i++) {                                         // �������� �� ���� ��������
      if (simData->eventChain[i].time == time) {                                                      // ���� �������� ������� ������� �������
        for (size_t k = 0; k < netl->gates.size(); k++)																								// �������� ������� ���������� ��� ���� ��������
          netl->gates[k]->repeat = 0;
        for (size_t k = 0; k < netl->nets.size(); k++) {                                              // ����������� �������� ������
          for (size_t j = 0; j < simData->eventChain[i].netsChain.size(); j++) {
            if (netl->nets[k]->name == simData->eventChain[i].netsChain[j]->name)
              netl->nets[k]->value = simData->eventChain[i].statesChain[j];

          }
        }
        stackSim->reset();

        // ���������� ��������� ������� �������� �� ������ ��������� ��������� �� ������
        
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
          simData->outs_temp.resize(stackSim->gatesChain[stackSim->busy]->outs.size());									        // ���� ���� �������� �� �������� ���������� ��������� 
          for (size_t it = 0; it < simData->outs_temp.size(); it++) {																						// �������� ��������� �� ������ �������
            simData->outs_temp[it] = stackSim->gatesChain[stackSim->busy]->outs[it]->value;
          }

          if (stackSim->gatesChain[stackSim->busy]->repeat < 500) {                                             // ����������� �������� ��������� ����� �����
            stackSim->gatesChain[stackSim->busy]->t_minus();
            stackSim->gatesChain[stackSim->busy]->operate();
            stackSim->gatesChain[stackSim->busy]->t_plus();
            stackSim->gatesChain[stackSim->busy]->repeat++;                                                     // ���������
          }

          for (size_t it = 0; it < simData->outs_temp.size(); it++) {																						// ���� ���� ���� ����� ���������, �� ��������� ����
            if (stackSim->gatesChain[stackSim->busy]->outs[it]->value != simData->outs_temp[it])
              valueChanged = true;
          }

          if (valueChanged) {                                                                                   // ���� ���� ��������, �� ��������� �������, ������� �� ������ ���� � ����
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


void simulator::simulation(netlist* netl, sim_data* simData, std::string filename, int stackSize) {
  // ��� ���������� ���� ����� � ������� �� �������, ��� � ���������� ����� ����� ��������, ����� ��������� ������� ��������������� (���� ��� � Active HDL)
  int initialTime = 0;                                                                                // ����� ������ �������������
  int stopTime = simData->eventChain[simData->eventChain.size() - 1].time + 10;                       // ����� ��������� �������������
  int time = 0;                                                                                       // ��������������� ������ �������������
  
  int cores = getNumCores();                                                                          // ������� ���������� ���� �� � �������
  printf("__inf__ : Available cores: %d\n", cores);                                                   // ������� �� �����
  std::vector<someArgs_t> args;                                                                       // ������ ������ ����������
  args.resize(cores);                                                                                 // �� ���������� ����

  std::vector<pthread_t> threads;                                                                     // ������ ������ �������
  threads.resize(cores);                                                                              // �� ���������� ����

  std::vector<std::vector<gate*>*> returned;                                                            // ������ ������ ������������ ��������
  returned.resize(cores);                                                                             // �� ���������� ����

  int status;
  int status_addr;

  valueChanged = false;                                                                               // ���� ��������� ��������� �� ������ �������
  std::string gateName;                                                                               // ��� �������
  datawriter wr(filename.c_str());                                                                    // ��������� �������� ������
  stack *stackSim = new stack(stackSize);                                                             // ���� �������������
//  for (size_t i = 0; i < simData->dumpNames.size(); i++)
//    wr.AddDumpVar(netl->returnNet(simData->dumpNames[i]));                                            // ��������� ����������, �������� ����� ����� �����������
  for (size_t i = 0; i < netl->nets.size(); i++)
    wr.AddDumpVar(netl->nets[i]);                                            // ��������� ����������, �������� ����� ����� �����������

  wr.PrintHeader();                                                                                   // ����� � �������� ���� �����
  
  for (time = initialTime; time < stopTime; time++) {																							    // ��������� ���

    for (size_t i = 0; i < simData->eventChain.size(); i++) {                                         // �������� �� ���� ��������
      if (simData->eventChain[i].time == time) {                                                      // ���� �������� ������� ������� �������
        for (size_t k = 0; k < netl->gates.size(); k++)																								// �������� ������� ���������� ��� ���� ��������
          netl->gates[k]->repeat = 0;
        for (size_t k = 0; k < netl->nets.size(); k++) {                                              // ����������� �������� ������
          for (size_t j = 0; j < simData->eventChain[i].netsChain.size(); j++) {
            if (netl->nets[k]->name == simData->eventChain[i].netsChain[j]->name)
              netl->nets[k]->value = simData->eventChain[i].statesChain[j];

          }
        }
        stackSim->reset();

        // ���������� ��������� ������� �������� �� ������ ��������� ��������� �� ������
        
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
          for (int core = 0; core < cores; cores++) {
            args[core].busy = stackSim->busy + core;
            args[core].simData = simData;
            args[core].netl = netl;
            args[core].stackSim = stackSim;
            args[core].returned = returned[core];
            if (stackSim->busy + core < stackSim->free) {
              status = pthread_create(&threads[core], NULL, routine, (void*)&args[core]);
              if (status != 0) {
                printf("main error: can't create thread, status = %d\n", status);
                exit(ERROR_CREATE_THREAD);
              }
            }
          }

          for (int core = 0; core < cores; core++) {
            if (stackSim->busy + core < stackSim->free) {
/*
              for (size_t kk = 0; kk < args[core].returned->size(); kk++) {
                stackSim->push_back(args[core].returned[kk]);
              }
              */
              status = pthread_join(threads[core], (void**)&status_addr);
              if (status != SUCCESS) {
                printf("main error: can't join thread, status = %d\n", status);
                exit(ERROR_JOIN_THREAD);
              }
              printf("joined with address %d\n", status_addr);
            }
            stackSim->eject();
          }

          //routine((void*) &args[core]);          
        }
        



      }
    }
    
    wr.DumpVars(time);
  }
}
