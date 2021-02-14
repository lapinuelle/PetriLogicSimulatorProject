#if !defined(S_HEADER)
#define S_HEADER
#include "netlist.h"
#include "simulation_data.h"
#include "datawriter.h"
#include "GateDumper.h"
#include "sdf.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

class simulator {
  datawriter *p_wr;
  GateDumper *gdumper;
public:
  simulator();
  void simulation_thread(netlist* netl, sim_data* simul_data, int stackSize);
  void simulation(netlist* netl, sim_data* simul_data, int stackSize, SDF *sdf);
	bool valueChanged;
public:
  void begin_multistep_mode(netlist* netl, sim_data* simul_data, int stackSize, SDF *sdf);
  float make_one_step(netlist* netl, sim_data* simul_data, int stackSize, SDF *sdf, std::string signal, std::string edge);
  void end_multistep_mode();
  
};
#endif
