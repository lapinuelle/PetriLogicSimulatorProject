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



void simulator::routine(sim_data* simData, netlist* netl, stack* stackSim) {
  bool valueChanged = false;
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
          routine(simData, netl, stackSim);          
        }
        



      }
    }
    
    wr.DumpVars(time);
  }
}
