#if !defined(GATE_HEADER)
#define GATE_HEADER

#include <string>
#include <vector>
#include <map>
#include "nets.h"

class gate {
protected:
	std::vector <net*> ins;               ///< Входы вентиля
	std::vector<LogicLevel> ins_temp;     ///< Временные (внутренние) входы для переноса меток

public:
  
  std::vector <net*> outs;              ///< Выходы вентиля
  std::vector<LogicLevel> outs_temp;    ///< Временные (внутренние) выходы для переноса меток
  std::string realName;                     ///< Имя вентиля
  std::string name;                     ///< Имя вентиля
  int repeat;                          ///< Количество повторений моделирования вентиля в конкретный момент времени
  float delay;                            ///< Задержка вентиля
  // methods
  void t_minus();                       ///< Момент сети Петри t_minus
  void setDelay(float delay);             ///< Функция, задающая задержку вентиля
  virtual void operate()=0;             ///< Момент сети Петри t_0
  bool t_plus();                        
  virtual bool postprocess() = 0;
  std::vector<std::string> tokens;
  std::map<std::string, int> jumps;

  void addInput(net*);
  void setInternalInput(int,LogicLevel);
  net* getInput(int);
  LogicLevel getInternalInput(int);
  int getInputsCount();

public:
  virtual ~gate() {};
};

class gate_beh: public gate {
public:
  gate_beh(std::string nameFile);
  void operate();
  bool postprocess();
  
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