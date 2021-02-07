#if !defined(NETS_HEADER)
#define NETS_HEADER

#include <string>
#include <vector>
#include <iostream>
#include "logiclevel.h"

class gate;

class net {
protected:
	std::string name;
  std::string realName;
  LogicLevel value;
  char type;
  std::string stability;
  int delay;
  std::vector<gate *>  gates;

public:
	net(const std::string &net_name);

	std::string getName();
	void setName(std::string);

  std::string getRealName();
  void setRealName(std::string);

	LogicLevel getValue();
	void setValue(LogicLevel);

	char getType();
	void setType(char);

	std::string getStability();
	void setStability(std::string);

	int getDelay();
	void setDelay(int);

	void connectToGate(gate*);
	gate* getConnectedGate(int);
	std::vector<gate*> getGates();
  int getGatesCount();


};

#endif