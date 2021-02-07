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
private:
  float time;
  std::vector<net*> netsChain;
  std::vector<bool> delayed;
public:
  Event();
public:
  
  
  std::vector<LogicLevel> statesChain;
  std::list<gate*> gatesChain;
  std::map<std::string, std::vector<net*> > inputStates;
  std::map < std::string, std::vector < LogicLevel > > inputStatesValues;
  //std::vector <gate*> gatesChain;

public:
  float getTime();
  void setTime(float);

  net* getNet(int);
  std::vector<net*> getNetsChain();
  void addNet(net*);
  int getNetsChainSize();

  bool isDelayed(int);
  void setDelayed(bool);
};

#endif
