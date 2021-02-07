#include "nets.h"

net::net(const std::string &net_name) : name(net_name), value(level_u) {
  
}

std::string net::getName()
{
	return this->name;
}

void net::setName(std::string name)
{
	this->name = name;
}

std::string net::getRealName()
{
  return this->realName;
}

void net::setRealName(std::string name)
{
  this->realName = name;
}

LogicLevel net::getValue()
{
	return this->value;
}

void net::setValue(LogicLevel value)
{
	this->value = value;
}

char net::getType()
{
	return this->type;
}

void net::setType(char type)
{
	this->type = type;
}

std::string net::getStability()
{
	return this->stability;
}

void net::setStability(std::string stability)
{
	this->stability = stability;
}

int net::getDelay()
{
	return this->delay;
}

void net::setDelay(int delay)
{
	this->delay = delay;
}

void net::connectToGate(gate* gat)
{
	this->gates.push_back(gat);
}

gate * net::getConnectedGate(int index)
{
	return this->gates[index];
}

std::vector<gate*> net::getGates()
{
	return this->gates;
}
