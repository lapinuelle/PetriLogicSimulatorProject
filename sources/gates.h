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
  int *repeat;
  int delay;
  // methods
  virtual void t_minus();
  void setDelay(int delay);
  virtual void operate()=0;
  virtual bool t_plus();
  virtual bool postprocess() = 0;
public:
  virtual ~gate() {};
};

class gate_beh: public gate {
public:
  gate_beh(std::string nameFile);
  void t_minus();
  void operate();
  bool t_plus();
};

class gate_not: public gate{
public:
  gate_not(std::string nameFile);
  void operate();
  bool postprocess();
};

class gate_buf: public gate{
public:
  gate_buf(std::string nameFile);
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

class gate_xor: public gate{
public:
  gate_xor(std::string nameFile);
  void operate();
  bool postprocess();
};

class gate_xnor: public gate{
public:
  gate_xnor(std::string nameFile);
  void operate();
  bool postprocess();
};



#endif
