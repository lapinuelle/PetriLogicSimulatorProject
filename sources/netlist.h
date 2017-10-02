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
  // Старая структура для хранения указателей на неты
  std::vector <net*> nets;
  // Новая структура, так получать их будет проще, особенно для assign
  std::map<std::string, net*> netsMap;
  // Старая структура для хранения указателей на вентили
  std::vector <gate*> gates;
  // Новая структура, так получать их будет проще, особенно для поведенческого
  std::map<std::string, gate*> gatesMap;
  int *repeats;
  //std::vector <>
  net *addNet(const std::string &net_name, gate *gate);
  net* returnNet(std::string netName);

  net *addNetMap(const std::string &net_name, gate *gate);
  net* returnNetMap(std::string netName);
  gate* returnMapGate(std::string);
  std::vector<gate*> &returnGate(net* net);
  /*
    addGate - считали вентиль, добавили в вектор
  */
  void addGate(gate* gate);
  /*
    addGateMap - новый вариант. Считали вентиль, добавили в структуру типа std::map<std::string, gate*>. Ключом в такой структуре выступает имя вентиля
  */
  void addGateMap(gate* gate);
	void connectNG();
  int neededSteps;
	netlist();
	~netlist();
};

#endif
