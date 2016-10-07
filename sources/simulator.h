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
	void simulation(netlist* netl, sim_data* simul_data, std::string filename, int stackSize);
  void simulation_stack(netlist* netl, sim_data* simul_data, std::string filename, int stackSize);
	bool valueChanged;
  
};
#endif
