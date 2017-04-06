#include "nets.h"
#include "gates.h"
#include "netlist.h"

#include <string>
#include <vector>
#include <algorithm>
#include <list>


netlist::netlist() : repeats(NULL) {
}

netlist::~netlist() {
  for (size_t i = 0; i < this->nets.size(); ++i)
    delete this->nets[i];
  this->nets.erase(this->nets.begin(), this->nets.end());

  for (size_t i = 0; i < this->gates.size(); ++i)
    delete this->gates[i];
	this->gates.erase(this->gates.begin(), this->gates.end());

  if (repeats) {
    delete [] repeats;
    repeats = NULL;
  }
}

net* netlist::addNet(const std::string &net_name, gate *gate) {
	net *netPointer = NULL;
	size_t i = 0, j = 0;

	for(i = 0; i < nets.size(); ++i) {
		if(nets[i]->name == net_name)
			break;
	}
	if(i < nets.size()) {
		if(gate != NULL) {
			for(j = 0; j < nets[i]->gates.size(); ++j)
				if(nets[i]->gates[j] == gate)
					break;
			if(j >= nets[i]->gates.size())
				nets[i]->gates.push_back(gate);
		}
		return nets[i];
	} else {


		netPointer = new net(net_name);
		if (gate != NULL)
		netPointer->gates.push_back(gate);
		nets.push_back(netPointer);
		return netPointer;
	}
}

net* netlist::returnNet(std::string netName) {
  for (size_t i=0; i<nets.size(); i++) {
	  if (nets[i]->name == netName) {
		  return nets[i];
	  } 
  }
  return NULL;
}


std::vector<gate*> &netlist::returnGate(net* net) {									// возвращает gate, у которого данный net на входе
	/*
  std::vector <gate*> returned;
	for (size_t i = 0; i < gates.size(); i++) {
    for (size_t j = 0; j < gates[i]->ins.size(); j++) {
      if (net->name == this->gates[i]->ins[j]->name) {
        returned.push_back(gates[i]);
      }
    }
  }
  return returned;
  //*/
  return net->gates;
}


void netlist::addGate(gate* gate) {
  this->gates.push_back(gate);
}

void netlist::addGateMap(gate* gate) {
  this->gatesMap[gate->name] = gate;
}
