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

void gate::setDelay(float delay)
{
	this->delay = delay;
}

bool gate::t_plus()
{
	return false;
}
