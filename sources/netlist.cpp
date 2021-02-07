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
  /*for (size_t i = 0; i < this->netsMap.size(); ++i)
    delete this->netsMap[i];
  this->netsMap.erase(this->netsMap.begin(), this->netsMap.end());*/

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
		if(nets[i]->getName() == net_name)
			break;
	}
	if(i < nets.size()) {
		if(gate != NULL) {
			for(j = 0; j < nets[i]->getGatesCount(); ++j)
				if(nets[i]->getConnectedGate(j) == gate)
					break;
			if(j >= nets[i]->getGatesCount())
				nets[i]->connectToGate(gate);
		}
		return nets[i];
	} else {


		netPointer = new net(net_name);
		if (gate != NULL)
		netPointer->connectToGate(gate);
		nets.push_back(netPointer);
		return netPointer;
	}
}

net* netlist::returnNet(std::string netName) {
  for (size_t i=0; i<nets.size(); i++) {
	  if (nets[i]->getName() == netName) {
		  return nets[i];
	  } 
  }
  return NULL;
}

////////////////////////////////////////
////////////////////////////////////////
net* netlist::addNetMap(const std::string &net_name, gate *gate) {
	net *netPointer = NULL;
	size_t i = 0, j = 0;

	if(netsMap[net_name]) {
		if(gate != NULL) {
			for(j = 0; j < netsMap[net_name]->getGatesCount(); ++j)
				if(netsMap[net_name]->getConnectedGate(j) == gate)
					break;
			if(j >= netsMap[net_name]->getGatesCount())
				netsMap[net_name]->connectToGate(gate);
		}
		return netsMap[net_name];
	} else {


		netPointer = new net(net_name);
		if (gate != NULL)
		  netPointer->connectToGate(gate);
		netsMap[net_name] = netPointer;
		return netPointer;
	}
}

net* netlist::returnNetMap(std::string netName) {
	if (netsMap[netName]) 
	 return netsMap[netName];
  return NULL;
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////


std::vector<gate*> netlist::returnGate(net* net) {									// возвращает gate, у которого данный net на входе
	/*
  std::vector <gate*> returned;
	for (size_t i = 0; i < getGates.size(); i++) {
    for (size_t j = 0; j < gates[i]->ins.size(); j++) {
      if (net->name == this->gates[i]->ins[j]->name) {
        returned.push_back(gates[i]);
      }
    }
  }
  return returned;
  //*/
  return net->getGates();
}

gate* netlist::returnMapGate(std::string name) {
  return this->gatesMap[name];
}


void netlist::addGate(gate* gate) {
  this->gates.push_back(gate);
}

void netlist::addGateMap(gate* gate) {
  this->gatesMap[gate->getName()] = gate;
}
