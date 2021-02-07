#if !defined(GATE_HEADER)
#define GATE_HEADER

#include <string>
#include <vector>
#include <map>
#include "nets.h"

class gate {
protected:
	std::vector <net*> ins;               ///< ����� �������
	std::vector<LogicLevel> ins_temp;     ///< ��������� (����������) ����� ��� �������� �����
	std::vector <net*> outs;              ///< ������ �������
	std::vector<LogicLevel> outs_temp;    ///< ��������� (����������) ������ ��� �������� �����
	std::string realName;                     ///< ��� �������
	std::string name;                     ///< ��� �������
	int repeat;                          ///< ���������� ���������� ������������� ������� � ���������� ������ �������
	float delay;                            ///< �������� �������
	std::vector<std::string> tokens;
	std::map<std::string, int> jumps;

public:
  void t_minus();                       ///< ������ ���� ����� t_minus
  virtual void operate()=0;             ///< ������ ���� ����� t_0
  bool t_plus();                        
  virtual bool postprocess() = 0;
  
  

  std::string getToken(int);
  std::vector<std::string> getTokens();
  void addToken(std::string);
  int getTokensCount();

  void addJump(std::string, int);
  int getJump(std::string);
  std::map<std::string, int> getJumps();

  void setName(std::string);
  std::string getName();

  void setRealName(std::string);
  std::string getRealName();

  void addInput(net*);
  void setInternalInput(int,LogicLevel);
  net* getInput(int);
  LogicLevel getInternalInput(int);
  int getInputsCount();

  void addOutput(net*);
  void setInternalOutput(int, LogicLevel);
  net* getOutput(int);
  LogicLevel getInternalOutput(int);
  int getOutputsCount();

  int getRepeatCount();
  void setRepeatCount(int);

  float getDelay();
  void setDelay(float);

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