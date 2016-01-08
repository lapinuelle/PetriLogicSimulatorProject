#include "simulation_data.h"

sim_data::sim_data() {};

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
    return eventPointer;
  }
}
