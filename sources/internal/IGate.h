#ifndef IGATE_H
#define IGATE_H

#include "../logiclevel.h"
#include <vector>
#include <string>
#include <map>

class net;

class IGate {

public:
	virtual void t_minus() = 0;                       ///< Момент сети Петри t_minus
	virtual void operate() = 0;             ///< Момент сети Петри t_0
	virtual bool t_plus() = 0;
	virtual bool postprocess() = 0;

	virtual void setDelay(float delay) = 0;             ///< Функция, задающая задержку вентиля
	virtual float getDelay() = 0;

	virtual void setName(std::string name) = 0;
	virtual std::string getName() = 0;

	virtual void addInput(net*) = 0;
	virtual std::vector<net*> getInputs() = 0;

	virtual void addOutput(net*) = 0;
	virtual std::vector<net*> getOutputs() = 0;

	virtual void setInternalInputValue(int, LogicLevel) = 0;
	virtual LogicLevel getInternalInputValue(int) = 0;

	virtual void setInternalOutputValue(int, LogicLevel) = 0;
	virtual LogicLevel getInternalOutputValue(int) = 0;
public:
	virtual ~IGate();
};

#endif // GATE_H
