#include <string>
#include <vector>
#include <iostream>
#include "nets.h"
#include "gates.h"
#include "event.h"

Event::Event() : time(0) {
  
}
float Event::getTime()
{
  return this->time;
}
void Event::setTime(float time)
{
  this->time = time;
}
net * Event::getNet(int index)
{
  return this->netsChain[index];
}
std::vector<net*> Event::getNetsChain()
{
  return this->netsChain;
}
void Event::addNet(net* nnet)
{
  this->netsChain.push_back(nnet);
}
int Event::getNetsChainSize()
{
  return this->netsChain.size();
}
;
