#if !defined(NLR_HEADER)
#define NLR_HEADER

#include <string>
#include <vector>

#include "netlist.h"
#include "simulation_data.h"

struct token {
  std::string item;
  size_t      line,
              line_orig,
              pos,
              pos_orig;
};

struct Macro {
  std::string               name;
  std::vector<std::string>  tokens;
};

void clear_at_left(std::string &line);
void clear_at_right(std::string &line);

// Interface for netlistreader
class inetlistreader {
protected:
  std::vector<token>  tokens;
  std::string         fileName;
public:
  inetlistreader(std::string fileName);
  virtual ~inetlistreader();
public:
  virtual bool is_my_format() = 0;
  virtual bool read(netlist *netl, sim_data *simul_data, std::string rootname) = 0;
protected:
  virtual bool tokenize() = 0;
};


struct VerilogModuleInfo;
typedef std::vector <VerilogModuleInfo> VMIs;

// netlistreader implementation for Verilog file format
class netlistreader_verilog : public inetlistreader {
  VMIs                vminfos;
  TimescaleInfo       timescale;
  std::vector<Macro>  macros;
public:
  netlistreader_verilog(std::string fileName);
 ~netlistreader_verilog();
public:
  std::map < std::string, std::string > realPins;
  bool is_my_format();
  bool read(netlist *netl, sim_data *simul_data, std::string rootname);
private:
  bool read_macro(size_t &i);
  bool preprocess_file();
  bool read_moduleInfo(size_t &i, VMIs &vminfos);
  bool unwrap_from_root(std::string rootname);
  bool unwrap_module(size_t &i_gate, std::string &real_name, std::vector<std::string> &real_pins);
  gate *CreateGate(const std::string &gate_name, const std::string &gate_type);
  bool tokenize();
  bool parse_flat_netlist(netlist *netl, sim_data *simul_data);
  bool parse_flat_gates(netlist *netl, sim_data *simul_data);
  bool parse_flat_initials(netlist *netl, sim_data *simul_data);
  bool parse_flat_assigns(netlist *netl, sim_data *simul_data);
  bool parse_flat_alwayses(netlist *netl, sim_data *simul_data);
};

// netlistreader implementation for VHDL file format

inetlistreader *get_appropriate_reader(std::string fileName);
void free_reader(inetlistreader *p_reader);

#endif
