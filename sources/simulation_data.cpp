#include "simulation_data.h"

sim_data::sim_data() {
  timescale.str_timescale = "1ns";
  timescale.str_precision = "1ps";
  timescale.timescale = 1e-9;
  timescale.precision = 1e-12;
};

void sim_data::setVCDname(std::string name) {
	vcdName = name;
}

std::string sim_data::getVCDname() {
	return vcdName;
}


Event* sim_data::addEvent(float time, net* net, LogicLevel state) {
  
  Event* eventPointer = NULL;
  size_t i = 0, j = 0;

  
  for (i = 0; i < eventChain.size(); ++i) {
    if (eventChain[i].getTime() == time)
      break;
  }
  if (i < eventChain.size()) {
    if (net != NULL) {
      for (j = 0; j < eventChain[i].getNetsChainSize(); ++j)
        if (eventChain[i].getNet(j) == net)
          break;
      if (j >= eventChain[i].getNetsChainSize()) {
        eventChain[i].addNet(net);
        eventChain[i].statesChain.push_back(state);
      }
    }
    return &eventChain[i];
  } else {
    eventPointer = new Event();
    eventPointer->setTime(time);
    if (net != NULL) {
      eventPointer->addNet(net);
      eventPointer->statesChain.push_back(state);
    }
    eventChain.push_back(*eventPointer);
    eventChain[i] = *eventPointer;
    return eventPointer;
  }
}

Event* sim_data::addMapEvent(float time, net* net, LogicLevel state) {
  
  Event* eventPointer = NULL;
  size_t i = 0, j = 0;

  if (newEventChain[time]) {
    if (net != NULL) {
      for (j = 0; j < newEventChain[time]->getNetsChainSize(); ++j)
        if (newEventChain[time]->getNet(j) == net)
          break;
      if (j >= newEventChain[time]->getNetsChainSize()) {
        newEventChain[time]->addNet(net);
        newEventChain[time]->statesChain.push_back(state);
      }
    }
    return newEventChain[time];
  } else {
    eventPointer = new Event();
    eventPointer->setTime(time);
    if (net != NULL) {
      eventPointer->addNet(net);
      eventPointer->statesChain.push_back(state);
    }
    eventChain.push_back(*eventPointer);
    newEventChain[time] = eventPointer;
    return eventPointer;
  }
  
}

Event* sim_data::addMapEvent(float time, net* net, LogicLevel state, bool delayedState) {

  Event* fEvent = NULL;
  size_t i = 0, j = 0;

  if (newEventChain[time]) {
    if (net != NULL) {
      for (j = 0; j < newEventChain[time]->getNetsChainSize(); ++j)
        if (newEventChain[time]->getNet(j) == net)
          break;
      if (j >= newEventChain[time]->getNetsChainSize()) {
        newEventChain[time]->addNet(net);
        newEventChain[time]->statesChain.push_back(state);
      }
    }
    newEventChain[time]->setDelayed(delayedState);
    return newEventChain[time];
  }
  else {
    fEvent = new Event();
    fEvent->setTime(time);
    if (net != NULL) {
      fEvent->addNet(net);
      fEvent->statesChain.push_back(state);
    }
    eventChain.push_back(*fEvent);
    newEventChain[time] = fEvent;
    newEventChain[time]->setDelayed(delayedState);
    return fEvent;
  }

}


