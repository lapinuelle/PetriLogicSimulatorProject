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


Event* sim_data::addEvent(int time, net* net, LogicLevel state) {
  
  Event* eventPointer = NULL;
  size_t i = 0, j = 0;

  
  for (i = 0; i < eventChain.size(); ++i) {
    if (eventChain[i].time == time)
      break;
  }
  if (i < eventChain.size()) {
    if (net != NULL) {
      for (j = 0; j < eventChain[i].netsChain.size(); ++j)
        if (eventChain[i].netsChain[j] == net)
          break;
      if (j >= eventChain[i].netsChain.size()) {
        eventChain[i].netsChain.push_back(net);
        eventChain[i].statesChain.push_back(state);
      }
    }
    return &eventChain[i];
  } else {
    eventPointer = new Event();
    eventPointer->time = time;
    if (net != NULL) {
      eventPointer->netsChain.push_back(net);
      eventPointer->statesChain.push_back(state);
    }
    eventChain.push_back(*eventPointer);
    eventChain[i] = *eventPointer;
    return eventPointer;
  }
}

Event* sim_data::addMapEvent(int time, net* net, LogicLevel state) {
  
  Event* eventPointer = NULL;
  size_t i = 0, j = 0;

  if (newEventChain[time]) {
    if (net != NULL) {
      for (j = 0; j < newEventChain[time]->netsChain.size(); ++j)
        if (newEventChain[time]->netsChain[j] == net)
          break;
      if (j >= newEventChain[time]->netsChain.size()) {
        newEventChain[time]->netsChain.push_back(net);
        newEventChain[time]->statesChain.push_back(state);
      }
    }
    return newEventChain[time];
  } else {
    eventPointer = new Event();
    eventPointer->time = time;
    if (net != NULL) {
      eventPointer->netsChain.push_back(net);
      eventPointer->statesChain.push_back(state);
    }
    eventChain.push_back(*eventPointer);
    newEventChain[time] = eventPointer;
    return eventPointer;
  }
  
}

Event* sim_data::addMapEvent(int time, net* net, LogicLevel state, bool delayedState) {

  Event* eventPointer = NULL;
  size_t i = 0, j = 0;

  if (newEventChain[time]) {
    if (net != NULL) {
      for (j = 0; j < newEventChain[time]->netsChain.size(); ++j)
        if (newEventChain[time]->netsChain[j] == net)
          break;
      if (j >= newEventChain[time]->netsChain.size()) {
        newEventChain[time]->netsChain.push_back(net);
        newEventChain[time]->statesChain.push_back(state);
      }
    }
    newEventChain[time]->delayed.push_back(delayedState);
    return newEventChain[time];
  }
  else {
    eventPointer = new Event();
    eventPointer->time = time;
    if (net != NULL) {
      eventPointer->netsChain.push_back(net);
      eventPointer->statesChain.push_back(state);
    }
    eventChain.push_back(*eventPointer);
    newEventChain[time] = eventPointer;
    newEventChain[time]->delayed.push_back(delayedState);
    return eventPointer;
  }

}


