#include "gate.h"
#include "nets.h"

void gate::t_minus()
{
	for (size_t i = 0; i < this->ins.size(); ++i) {

		if ((this->ins[i]->value == level_0) && (this->ins_temp[i] == level_u))
			this->ins[i]->stability = '\\';
		if ((this->ins[i]->value == level_1) && (this->ins_temp[i] == level_u))
			this->ins[i]->stability = '/';

		if (((this->ins[i]->value == level_0) || (this->ins[i]->value == level_u)) && (this->ins_temp[i] == level_1))// && (ins[i]->stability == "_"))
			this->ins[i]->stability = '\\';
		if (((this->ins[i]->value == level_1) || (this->ins[i]->value == level_u)) && (this->ins_temp[i] == level_0))// && (ins[i]->stability == "_"))
			this->ins[i]->stability = '/';
		this->ins_temp[i] = this->ins[i]->value;
	}
}

void gate::operate()
{
}

bool gate::postprocess()
{
	return false;
}

float gate::getDelay()
{
	return this->delay;
}

void gate::setName(std::string name)
{
	this->name = name;
}

std::string gate::getName()
{
	return this->name;
}

void gate::addInput(net* in)
{
	this->ins.push_back(in);
}

std::vector<net*> gate::getInputs()
{
	return this->ins;
}

void gate::addOutput(net* out)
{
	this->outs.push_back(out);
}

std::vector<net*> gate::getOutputs()
{
	return this->outs;
}

void gate::setInternalInputValue(int index, LogicLevel value)
{
	this->ins_temp[index] = value;
}

LogicLevel gate::getInternalInputValue(int index)
{
	return this->ins_temp[index];
}

void gate::setInternalOutputValue(int index, LogicLevel value)
{
	this->outs_temp[index] = value;
}

LogicLevel gate::getInternalOutputValue(int index)
{
	return this->outs_temp[index];
}

void gate::setRealName(std::string name)
{
	this->realName = name;
}

std::string gate::getRealName()
{
	return this->realName;
}

int gate::getRepeatCount()
{
	return this->repeat;
}

void gate::setRepeatCount(int count)
{
	this->repeat = count;
}

std::vector<std::string> gate::getTokens()
{
	return this->tokens;
}

void gate::addToken(std::string token)
{
	this->tokens.push_back(token);
}

void gate::addJump(std::string tag, int position)
{
	this->jumps[tag] = position;
}

int gate::getTokensCount()
{
	return 0;
}

std::map<std::string, int> gate::getJumps()
{
	return this->jumps;
}

void gate::setDelay(float delay)
{
	this->delay = delay;
}

bool gate::t_plus()
{
	return false;
}
