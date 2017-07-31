#if !defined(INTERPRET_HEADER)
#define INTERPRET_HEADER

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include "logiclevel.h"
#include "netlist.h"

class gate;
class net;

class interpreter {
public:
  void cmp(net*, LogicLevel);
  void cmp(net*, std::string);
  void mov(LogicLevel, net*);
  void add(LogicLevel, LogicLevel);
  
  void add(int, int);
  void sub(int, int);

  void and(LogicLevel, LogicLevel);
  void or(LogicLevel, LogicLevel);
  void xor(LogicLevel, LogicLevel);

  void reset();
  void operate(std::vector<std::string>, std::map<std::string, int>, netlist*);

  std::map <std::string, int> flags;
  std::map <std::string, int> intRegisters;
  std::map <std::string, LogicLevel> logRegisters;

  interpreter();
};

#endif