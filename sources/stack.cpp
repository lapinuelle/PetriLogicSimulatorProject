#include "stack.h"

stack::stack(int size) {
  gatesChain.resize(size);
  this->size = size;
  busy = -1;
  free = 0;

}
stack::~stack() {
  gatesChain.erase(gatesChain.begin(), gatesChain.end());
}

void stack::push_back(gate* gate) {
  if ((free != busy)) {
    if (busy == -1) {
      busy = 0;
    }
    gatesChain[free] = gate;
    if (free < (int)gatesChain.size()) {
      free++;
    }
    if (free == (int)gatesChain.size()) {
      free = 0;
    }
    
  }
  else {
    printf("__err__ : Stack overload.\n\n");
  }
}

void stack::eject() {
  if (busy < (int)gatesChain.size()) {
    busy++;
  }
  if (busy == (int)gatesChain.size()) {
    busy = 0;
  }

}

void stack::reset() {
  for (int i = 0; i < this->size; i++)
    this->gatesChain[i] = NULL;
  busy = -1;
  free = 0;
}
