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
#include <map>

class netlist {
public:
  // —тара€ структура дл€ хранени€ указателей на неты
  std::vector <net*> nets;
  // Ќова€ структура, так получать их будет проще, особенно дл€ assign
  std::map<std::string, net*> netsMap;
  //
  std::vector <gate*> gates;
  //
  std::map<std::string, gate*> gatesMap;
  int *repeats;
  //std::vector <>
  net *addNet(const std::string &net_name, gate *gate);
  net* returnNet(std::string netName);
  std::vector<gate*> &returnGate(net* net);
  /*
    addGate - считали вентиль, добавили в вектор
  */
  void addGate(gate* gate);
  /*
    addGateMap - новый вариант. —читали вентиль, добавили в структуру типа std::map<std::string, gate*>.  лючом в такой структуре выступает им€ вентил€
  */
  void addGateMap(gate* gate);
	void connectNG();
  int neededSteps;
	netlist();
	~netlist();
};

#endif
