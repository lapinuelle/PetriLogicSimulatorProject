#if !defined(S_HEADER)
#define S_HEADER
#include "netlist.h"
#include "simulation_data.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

class simulator {
public:
  simulator();
	void simulation(netlist* netl, sim_data* simul_data, int stackSize);
  void simulation_stack(netlist* netl, sim_data* simul_data, int stackSize);
	bool valueChanged;
  
};
#endif
