#if !defined(GATE_HEADER)
#define GATE_HEADER

#include <string>
#include <vector>
#include "nets.h"

class gate {
public:
  std::vector <net*> ins;
  std::vector<LogicLevel> ins_temp;
  std::vector <net*> outs;
  std::vector<LogicLevel> outs_temp;
  std::string name;
  int repeat;
  // methods
  void t_minus();
  virtual void operate()=0;
  void t_plus();
  virtual bool postprocess() = 0;
public:
  virtual ~gate() {};
};

class gate_not: public gate{
public:
  gate_not(std::string nameFile);
  void operate();
  bool postprocess();
};

class gate_and: public gate{
public:
  gate_and(std::string nameFile);
  void operate();
  bool postprocess();
};

class gate_or: public gate{
public:
  gate_or(std::string nameFile);
  void operate();
  bool postprocess();
};

class gate_nor: public gate{
public:
  gate_nor(std::string nameFile);
  void operate();
  bool postprocess();
};

class gate_nand: public gate{
public:
  gate_nand(std::string nameFile);
  void operate();
  bool postprocess();
};



#endif
