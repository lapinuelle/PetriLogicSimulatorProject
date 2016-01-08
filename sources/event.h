#if !defined(EVENT_HEADER)
#define EVENT_HEADER

#include <string>
#include <vector>
#include <iostream>
#include "nets.h"
#include "gates.h"
#include "logiclevel.h"
#include <list>

class Event {
public:
  Event();
public:
  int time;
  std::vector<net*> netsChain;
  std::vector<LogicLevel> statesChain;
  std::list<gate*> gatesChain;
  //std::vector <gate*> gatesChain;
};

#endif
