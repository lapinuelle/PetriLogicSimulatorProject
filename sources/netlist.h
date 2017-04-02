#if !defined(NL_HEADER)
#define NL_HEADER
#include "logiclevel.h"
#include "nets.h"
#include "gates.h"

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <list>

class netlist {
public:
  std::vector <net*> nets;
  std::vector <gate*> gates;
  int *repeats;
  //std::vector <>
  net *addNet(const std::string &net_name, gate *gate);
  net* returnNet(std::string netName);
  std::vector<gate*> &returnGate(net* net);
  void addGate(gate* gate);
	void connectNG();
  int neededSteps;
	netlist();
	~netlist();
};

#endif
