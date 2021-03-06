#if !defined(S_HEADER)
#define S_HEADER
#include "netlist.h"
#include "simulation_data.h"
#include "datawriter.h"
#include "sdf.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include "nlohmann/json.hpp"


class simulator {
  datawriter *p_wr;
public:
  simulator();
  void simulation_thread(netlist* netl, sim_data* simul_data, int stackSize);
  void simulation(netlist* netl, sim_data* simul_data, int stackSize, SDF *sdf);
	bool valueChanged;
public:
  void begin_multistep_mode(netlist* netl, sim_data* simul_data, int stackSize, SDF *sdf);
  float make_one_step(netlist* netl, sim_data* simul_data, int stackSize, SDF *sdf, std::string signal, std::string edge, nlohmann::json* dump);
  void end_multistep_mode();
  
};
#endif
