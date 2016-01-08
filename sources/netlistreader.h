#if !defined(NLR_HEADER)
#define NLR_HEADER

#include <string>
#include <vector>

#include "netlist.h"
#include "simulation_data.h"


struct token {
  std::string item;
  size_t      line,
              pos;
};


// Interface for netlistreader
class inetlistreader {
protected:
  std::string         fileName;
public:
  inetlistreader(std::string fileName);
  virtual ~inetlistreader();
public:
  virtual bool is_my_format() = 0;
  virtual bool read(netlist *netl, sim_data *simul_data) = 0;
protected:
  bool tokenize();
};


// netlistreader implementation for Verilog file format
class netlistreader_verilog : public inetlistreader {
  std::vector<token>  tokens;
public:
  netlistreader_verilog(std::string fileName);
 ~netlistreader_verilog();
public:
  bool is_my_format();
  bool read(netlist *netl, sim_data *simul_data);
private:
  gate *CreateGate(const std::string &gate_name, const std::string &gate_type);
  bool tokenize();
};

// netlistreader implementation for VHDL file format


inetlistreader *get_appropriate_reader(std::string fileName);
void free_reader(inetlistreader *p_reader);

#endif
