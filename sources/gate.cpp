#include "gate.h"

void gate::t_minus()
{
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
}

std::string gate::getName()
{
	return this->name;
}

void gate::addInput(net *)
{
}

std::vector<net*> gate::getInputs()
{
	return this->ins;
}

void gate::addOutput(net *)
{
}

std::vector<net*> gate::getOutputs()
{
	return this->outs;
}

void gate::setDelay(float delay)
{
}

bool gate::t_plus()
{
	return false;
}
