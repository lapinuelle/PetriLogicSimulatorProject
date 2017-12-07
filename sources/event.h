#if !defined(EVENT_HEADER)
#define EVENT_HEADER

#include <string>
#include <vector>
#include <iostream>
#include "nets.h"
#include "gates.h"
#include "logiclevel.h"
#include <list>
#include <map>

class Event {
public:
  Event();
public:
  int time;
  std::vector<net*> netsChain;
  std::vector<bool> delayed;
  std::vector<LogicLevel> statesChain;
  std::list<gate*> gatesChain;
  std::map<std::string, std::vector<net*> > inputStates;
  std::map < std::string, std::vector < LogicLevel > > inputStatesValues;
  //std::vector <gate*> gatesChain;
};

#endif
