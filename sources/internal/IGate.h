#ifndef IGATE_H
#define IGATE_H

#include "../logiclevel.h"
#include <vector>
#include <string>
#include <map>

class net;

class igate {
private:
	std::vector <net*> ins;               ///< Входы вентиля
	std::vector<LogicLevel> ins_temp;     ///< Временные (внутренние) входы для переноса меток
	std::vector <net*> outs;              ///< Выходы вентиля
	std::vector<LogicLevel> outs_temp;    ///< Временные (внутренние) выходы для переноса меток
	std::string realName;                     ///< Имя вентиля
	std::string name;                     ///< Имя вентиля
	int repeat;                          ///< Количество повторений моделирования вентиля в конкретный момент времени
	float delay;                            ///< Задержка вентиля
	std::vector<std::string> tokens;
	std::map<std::string, int> jumps;

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
public:
	virtual ~igate();
};

#endif // GATE_H
