#ifndef INET_H
#define INET_H

#include <string>
#include <vector>
#include <iostream>
#include "../logiclevel.h"

class gate;

class inet {
private:
	std::string name;
	std::string realName;
	LogicLevel value;
	char type;
	std::string stability;
	int delay;
	std::vector<gate *>  gates;


public:
	inet(const std::string &net_name);

	virtual void setName(std::string) = 0;
	virtual void setRealName(std::string) = 0;

	virtual void setValue(LogicLevel) = 0;
	virtual LogicLevel getValue() = 0;

	virtual void setType(char) = 0;
	virtual char getType() = 0;

	virtual void setStability(std::string) = 0;
	virtual std::string getStability() = 0;

	virtual void setDelay(int) = 0;
	virtual int getDelay() = 0;

	virtual void addConnectedGate(gate*) = 0;
	virtual void addConnectedGates(std::vector<gate*>) = 0;
	virtual std::vector<gate*> getConnectedGates() = 0;




};


#endif // NET_H
