#if !defined(INTERPRET_HEADER)
#define INTERPRET_HEADER

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include "logiclevel.h"
#include "netlist.h"
#include "gates.h"

class gate;
class net;

class interpreter {
public:
  LogicLevel interpreter::not(LogicLevel);

  LogicLevel interpreter::not(net* net);
  
  void cmp(net*, LogicLevel);
  void cmp(net*, std::string);
  void mov(LogicLevel, net*);
  void add(LogicLevel, LogicLevel);
  
  void add(int, int);
  void sub(int, int);

  void pnand(LogicLevel, LogicLevel);
  void pnor(LogicLevel, LogicLevel);
  void pnxor(LogicLevel, LogicLevel);

  void reset();
  void operate(gate*, netlist*);
  gate* modeGate;

  std::map <std::string, int> flags;
  std::map <std::string, int> intRegisters;
  std::map <std::string, LogicLevel> logRegisters;

  interpreter();
};

#endif
