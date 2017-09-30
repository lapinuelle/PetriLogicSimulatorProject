#include "netlistreader.h"
#include <cstring>

// Information about module's internals
#define MODULE_HAS_SMTH     0x00
#define MODULE_HAS_RTL      0x01
#define MODULE_HAS_ALWAYS   0x02
#define MODULE_HAS_STR      0x04
#define MODULE_HAS_INITIAL  0x08

#define MAX_ERRORS_ALLOWED  0x0A

struct VerilogModuleInfo {
  unsigned char             type;
  size_t                    token_start,
                            token_end;
  std::vector<std::string>  pins;
  std::string               name;
};

struct behGateInfo {
  unsigned char             type;
  std::string               name;
  std::vector<std::string>  inputs;
  std::vector<std::string>  outputs;
};

struct SuperDuperModule {
  std::string               name;

  std::vector < std::vector<std::string> >  assigns;
  std::vector < std::vector<std::string> >  initials;
  std::vector < std::vector<std::string> >  alwayses;
  std::vector < std::vector<std::string> >  gates;
  std::map <int, std::vector<std::string> > alwaysGates;
  std::map <int, behGateInfo> alwsGates;

} root;

netlistreader_verilog::netlistreader_verilog(std::string fileName) : inetlistreader(fileName) {
}

netlistreader_verilog::~netlistreader_verilog() {
}

bool netlistreader_verilog::is_my_format() {
  size_t pos = fileName.find_last_of(".");
  if(pos == std::string::npos)
    return false;
  if(fileName.substr(pos + 1) == "v")
    return true;
  return false;
}

gate * netlistreader_verilog::CreateGate(const std::string &gate_name, const std::string &gate_type) {
  if (gate_type == "not") {
    return new gate_not(gate_name);
  };
  if (gate_type == "and") {
    return new gate_and(gate_name);
  };
  if (gate_type == "nand") {
    return new gate_nand(gate_name);
  };
  if (gate_type == "or") {
    return new gate_or(gate_name);
  };
  if (gate_type == "nor") {
    return new gate_nor(gate_name);
  };
  if (gate_type == "xor") {
    return new gate_xor(gate_name);
  };
  if (gate_type == "xnor") {
    return new gate_xnor(gate_name);
  };
  if (gate_type == "buf") {
    return new gate_buf(gate_name);
  };
  if (gate_type == "beh") {
    return new gate_beh(gate_name);
  };
  return NULL;
}


bool netlistreader_verilog::read_macro(size_t &i) {
  size_t line_orig = tokens[i].line;
  // Just skip this line
  while(line_orig == tokens[i].line)
    ++i;
  --i;
  return true;
}

bool netlistreader_verilog::read_moduleInfo(size_t &i, VMIs &vminfos) {
  VerilogModuleInfo vmi;
  vmi.type = MODULE_HAS_SMTH;
  vmi.token_start = i;
  vmi.name = tokens[++i].item;

  printf("__inf__ : Reading module '%s'...", vmi.name.c_str());

  if ("(" != tokens[++i].item) {
    printf("\n__err__ [%d,%d]: '(' expected.\n", tokens[i].line, tokens[i].pos);
    return false;
  }

  ++i;
  while (")" != tokens[i].item) {
    vmi.pins.push_back(tokens[i].item);
    ++i;
    if ("," != tokens[i].item && ")" != tokens[i].item) {
      printf("\n__err__ [%d,%d]: ',' expected.\n", tokens[i].line, tokens[i].pos);
      return false;
    }
    if(")" == tokens[i].item)
      continue;
    ++i;
  }

  if (";" != tokens[++i].item) {
    printf("\n__err__ [%d,%d]: ';' expected.\n", tokens[i].line, tokens[i].pos);
    return false;
  }

  while ("endmodule" != tokens[i].item && i < tokens.size()) {
    ++i;
    if("endmodule" == tokens[i].item)
      break;
    if ("input" == tokens[i].item || "output" == tokens[i].item || "wire" == tokens[i].item || "reg" == tokens[i].item) {
      while (i < tokens.size() && ";" != tokens[i].item)
        ++i;
      continue;
    }
    if ("not" == tokens[i].item || "or" == tokens[i].item || "and" == tokens[i].item || "nor" == tokens[i].item || "nand" == tokens[i].item || "xor" == tokens[i].item || "xnor" == tokens[i].item) {
      vmi.type |= MODULE_HAS_RTL;
      while (i < tokens.size() && ";" != tokens[i].item)
        ++i;
      continue;
    }
    if ("always" == tokens[i].item) {
      vmi.type |= MODULE_HAS_ALWAYS;
      if("@" == tokens[i + 1].item)
        while (i < tokens.size() && "end" != tokens[i].item)
          ++i;
      else
        while (i < tokens.size() && ";" != tokens[i].item)
          ++i;
      continue;
    }
    if ("initial" == tokens[i].item) {
      vmi.type |= MODULE_HAS_INITIAL;
      while (i < tokens.size() && "end" != tokens[i].item)
        ++i;
      continue;
    }
    if ("(" == tokens[i + 2].item) {
      vmi.type |= MODULE_HAS_STR;
      while (i < tokens.size() && ";" != tokens[i].item)
        ++i;
      continue;
    }
    if ("#" == tokens[i + 1].item && "(" == tokens[i + 4].item) {
      vmi.type |= MODULE_HAS_STR;
      while (i < tokens.size() && ";" != tokens[i].item)
        ++i;
      continue;
    }
  }
  vmi.token_end = i;

  printf(" done\n");

  vminfos.push_back(vmi);
  return true;
}

bool netlistreader_verilog::unwrap_module(size_t &i_gate, std::string &real_name, std::vector<std::string> &real_pins) {
  std::vector<std::string>  pins;
  std::string               instance_name;
  std::string               module_name;

  module_name = root.gates[i_gate][0];
  instance_name = root.gates[i_gate][1];

  size_t i = 2;

  while (")" != root.gates[i_gate][i]) {
    ++i;
    if (")" == root.gates[i_gate][i])
      continue;
    if ("," == root.gates[i_gate][i] || "input" == root.gates[i_gate][i] || "out" == root.gates[i_gate][i])
      continue;
    pins.push_back(root.gates[i_gate][i]);
  };

  // now we have to search our file for this module and instantiate it to root module
  size_t i_inst = 0;
  for (; i_inst < vminfos.size(); ++i_inst) {
    if(vminfos[i_inst].name == module_name)
      break;
  }

  if (i_inst == vminfos.size()) {
    printf("__err__ : Can't find instance for module '%s'.\n", module_name.c_str());
    return false;
  }

  std::vector<std::string>  items;

  for (size_t i = vminfos[i_inst].token_start; i < vminfos[i_inst].token_end; i++) {
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;

    if ("module" == tokens[i].item) {
      while(tokens[i].item != ";")
        ++i;
      continue;
    }
    if ("input" == tokens[i].item) {
      ++i;
      while (";" != tokens[i].item) {
        if ("," != tokens[i].item) {
          std::string pin = tokens[i].item;
          size_t k = 0;
          for (; k < vminfos[i_inst].pins.size(); ++k)
            if (pin == vminfos[i_inst].pins[k]) {
              //if (items[j] == real_pins[k]) {
              pin = real_pins[k];
              break;
            }
          if (k == vminfos[i_inst].pins.size())
            pin = real_name + std::string(".") + pin;
          inputs.push_back(pin);

        }
        ++i;
      }
      ++i;
      //continue;
    }
    if ("output" == tokens[i].item) {
      ++i;
      while (";" != tokens[i].item) {
        if ("," != tokens[i].item) {
          std::string pin = tokens[i].item;
          size_t k = 0;
          for (; k < vminfos[i_inst].pins.size(); ++k)
            if (pin == vminfos[i_inst].pins[k]) {
              //if (items[j] == real_pins[k]) {
              pin = real_pins[k];
              break;
            }
          if (k == vminfos[i_inst].pins.size())
            pin = real_name + std::string(".") + pin;
          outputs.push_back(pin);
        }
        ++i;
      }
      ++i;
      //continue;
    }
    if ("wire" == tokens[i].item || "reg" == tokens[i].item || "input" == tokens[i].item || "output" == tokens[i].item) {
      while (";" != tokens[i].item) {
        ++i;
      }
      continue;
    }
    if ("initial" == tokens[i].item) {
      items.clear();
      while ("end" != tokens[i].item) {
        items.push_back(tokens[i].item);
        ++i;
      }
      items.push_back(tokens[i].item);
      if (";" == tokens[i + 1].item) {
        ++i;
        items.push_back(tokens[i].item);
      }
      size_t j = 1;
      if(items[j] == "begin")
        ++j;
      for (; j < items.size(); ++j) {
        if(items[j] == "#" || items[j] == ";" || items[j] == "=")
          continue;
        if(isdigit(items[j][0]))
          continue;
        size_t k = 0;
        for(; k < vminfos[i_inst].pins.size(); ++k)
          if (items[j] == vminfos[i_inst].pins[k]) {
          //if (items[j] == real_pins[k]) {
            items[j] = real_pins[k];
            break;
          }
        if(k == vminfos[i_inst].pins.size())
          items[j] = real_name + std::string(".") + items[j];
      }
      root.initials.push_back(items);
      ++i;
      continue;
    }

    int tempi = 0;

    if ("always" == tokens[i].item) {
      tempi = i;
      items.clear();
      while ("end" != tokens[i].item) {
        items.push_back(tokens[i].item);
        ++i;
      }
      items.push_back(tokens[i].item);
      if (";" == tokens[i + 1].item) {
        ++i;
        items.push_back(tokens[i].item);
      }
      size_t j = 1;
      if(items[j] == "begin")
        ++j;
      for (; j < items.size(); ++j) {

        if(items[j] == "@" || items[j] == ";" || items[j] == "=" || items[j] == "~" || items[j] == "begin" || items[j] == "end")
          continue;
        if(isdigit(items[j][0]))
          continue;
        size_t k = 0;
        for(; k < vminfos[i_inst].pins.size(); ++k)
          if (items[j] == vminfos[i_inst].pins[k]) {
          //if (items[j] == real_pins[k]) {
            items[j] = real_pins[k];
            break;
          }
        if(k == vminfos[i_inst].pins.size())
          items[j] = real_name + std::string(".") + items[j];
      }
      root.alwayses.push_back(items);
      if(items[1] == "@") {
        root.alwsGates[root.alwayses.size() - 1].name = real_name;
        root.alwsGates[root.alwayses.size() - 1].inputs = inputs;
        root.alwsGates[root.alwayses.size() - 1].outputs = outputs;
      }


      ++i;

      
        continue;
    }
    if ("assign" == tokens[i].item) {
      items.clear();
      while (";" != tokens[i].item) {
        items.push_back(tokens[i].item);
        ++i;
      }
      items.push_back(tokens[i].item);
      size_t j = 1;
      for (; j < items.size(); ++j) {
        if (items[j] == "#" || items[j] == ";" || items[j] == "=" || items[j] == "~" || items[j] == "&" || items[j] == "|" || items[j] == "(" || items[j] == ")" || items[j] == "?" || items[j] == ":" || items[j] == "||" || items[j] == "&&")
          continue;
        if (isdigit(items[j][0]))
          continue;
        size_t k = 0;
        for (; k < vminfos[i_inst].pins.size(); ++k)
          if (items[j] == vminfos[i_inst].pins[k]) {
          //if (items[j] == real_pins[k]) {
            items[j] = real_pins[k];
            break;
          }
        if (k == vminfos[i_inst].pins.size())
          items[j] = real_name + std::string(".") + items[j];
      }
      root.assigns.push_back(items);
      //++i;
      continue;
    }

    // If we are here, we are reading STR description
    items.clear();
    while (";" != tokens[i].item) {
      items.push_back(tokens[i].item);
      ++i;
    }
    items.push_back(tokens[i].item);
    size_t j = 1;
    
    if("#"  == items[j])
      j += 2;

    items[j] = real_name + std::string(".") + items[j];
    j += 2;

    for (; j < items.size() - 2; ++j) {
      if(items[j] == ",")
        continue;
      size_t k = 0;
      for (; k < vminfos[i_inst].pins.size(); ++k)
        if (items[j] == vminfos[i_inst].pins[k]) {
        //if (items[j] == real_pins[k]) {
          items[j] = real_pins[k];
          break;
        }
      if (k == vminfos[i_inst].pins.size())
        items[j] = real_name + std::string(".") + items[j];
    }
    root.gates.push_back(items);

  }

  return true;
}

bool netlistreader_verilog::unwrap_from_root(std::string rootname) {
  size_t i_root = 0;
  printf("\n");
  for(i_root = 0; i_root < vminfos.size(); ++i_root)
    if(vminfos[i_root].name == rootname)
      break;

  if (i_root == vminfos.size()) {
    printf("__err__ : Root module '%s' was not found.\n          Next %d modules are available:\n", rootname.c_str(), vminfos.size());
    for(size_t i_root = 0; i_root < vminfos.size(); ++i_root)
      printf("            [%d, 1] -> %s\n", tokens[vminfos[i_root].token_start].line, vminfos[i_root].name.c_str());
    printf("          Abort.\n");
    return false;
  }

  printf("\n__inf__ : Root module found.\n\n");

  root.name = vminfos[i_root].name;
  size_t i = vminfos[i_root].token_start;

  while(";" != tokens[i].item)
    ++i;
  ++i;

  std::vector<std::string>  items;

  while (i < vminfos[i_root].token_end) {
    if ("wire" == tokens[i].item || "reg" == tokens[i].item) {
      while(";" != tokens[i].item)
        ++i;
      ++i;
      continue;
    }

    if ("initial" == tokens[i].item) {
      items.clear();
      while ("end" != tokens[i].item) {
        items.push_back(tokens[i].item);
        ++i;
      }
      items.push_back(tokens[i].item);
      if (";" == tokens[i + 1].item) {
        ++i;
        items.push_back(tokens[i].item);
      }
      root.initials.push_back(items);
      ++i;
      continue;
    }

    if ("assign" == tokens[i].item) {
      items.clear();
      while (";" != tokens[i].item) {
        items.push_back(tokens[i].item);
        ++i;
      }
      items.push_back(tokens[i].item);
      root.assigns.push_back(items);
      ++i;
      continue;
    }

    if ("always" == tokens[i].item) {
      items.clear();
      if ("@" != tokens[i + 1].item) {
        while (";" != tokens[i].item) {
          items.push_back(tokens[i].item);
          ++i;
        }
      }
      else {
        int op_braces = 0;
        while ("begin" != tokens[i].item) {
          items.push_back(tokens[i].item);
          ++i;
        }
        do {
          items.push_back(tokens[i].item);
          if("begin" == tokens[i].item)
            ++op_braces;
          if("end" == tokens[i].item)
            --op_braces;
          ++i;
        } while(op_braces);
        --i;
      }
      if(";" == tokens[i].item)
        items.push_back(tokens[i].item);
      root.alwayses.push_back(items);
      ++i;
      continue;
    }

    // If we are here, we are reading STR description
    items.clear();
    while (";" != tokens[i].item) {
      items.push_back(tokens[i].item);
      ++i;
    }
    items.push_back(tokens[i].item);
    root.gates.push_back(items);
    ++i;
  }

  for (size_t i = 0; i < root.gates.size(); ++i) {
    // Skip built-in gates
    if("not" == root.gates[i][0] || "or" == root.gates[i][0] || "nor" == root.gates[i][0] || "and" == root.gates[i][0] || "nand" == root.gates[i][0] || "xor" == root.gates[i][0] || "xnor" == root.gates[i][0])
      continue;

    std::vector<std::string>  pins;

    std::string name = root.gates[i][1];

    size_t j = 2;
    while (")" != root.gates[i][j]) {
      ++j;
      if (")" == root.gates[i][j])
        continue;
      if ("," == root.gates[i][j] || "input" == root.gates[i][j] || "out" == root.gates[i][j])
        continue;
      pins.push_back(root.gates[i][j]);
    };

    unwrap_module(i, name, pins);
    root.gates.erase(root.gates.begin() + i);
    --i;
  }

  return true;
}

bool netlistreader_verilog::read(netlist *netl, sim_data *simul_data, std::string rootname) {
  if(!tokenize())
    return false;

  size_t  errCounter = 0;

  for (size_t i = 0; i < tokens.size(); ++i) {
    // Check if we've got too much errors for this moment
    if (errCounter > MAX_ERRORS_ALLOWED) {
      printf("__inf__ : Too much errors (%d). Abort reading.\n", MAX_ERRORS_ALLOWED);
      return false;
    }
    // Check if somewhy we have a token that is not at the beginning of the line
    // It means that we haven't readed something to the end, so it is the error
    if (1 != tokens[i].pos) {
      printf("__err__ [%d,%d]: Unexpected token '%s'.\n", tokens[i].line, tokens[i].pos, tokens[i].item.c_str());
      ++errCounter;
      continue;
    }

    // Reading some kind of macro
    if ("`" == tokens[i].item) {
      if(!this->read_macro(i))
        ++errCounter;
      continue;
    }

    if ("module" == tokens[i].item) {
      if (!this->read_moduleInfo(i, vminfos))
        ++errCounter;
      continue;
    }

    printf("__err__ [%d,%d]: Unsupported token '%s'.\n", tokens[i].line, tokens[i].pos, tokens[i].item.c_str());
    ++errCounter;
  }

  if (errCounter) {
    printf("__inf__ : %d errors found while reading netlist. Abort flattening.\n", errCounter);
    return false;
  }

  if(!unwrap_from_root(rootname))
    return false;

  // At this place we have flat netlist in root. Let's parse it!
  return parse_flat_netlist(netl, simul_data);
}

bool netlistreader_verilog::parse_flat_gates(netlist *netl, sim_data *simul_data) {
  gate *p_gate = NULL;

  for (size_t i = 0; i < root.gates.size(); ++i) {

    size_t j_pins = 0;

    if (root.gates[i][1] == "#") {
      p_gate = CreateGate(root.gates[i][3], root.gates[i][0]);
      if (!p_gate)
        return false;
      p_gate->setDelay(atoi(root.gates[i][2].c_str()));
      j_pins = 5;
    }
    else {
      p_gate = CreateGate(root.gates[i][1], root.gates[i][0]);
      if (!p_gate)
        return false;
      p_gate->setDelay(0);
      j_pins = 3;
    }

    //p_gate->outs.push_back(netl->addNet(root.gates[i][j_pins], NULL));
    p_gate->outs.push_back(netl->addNetMap(root.gates[i][j_pins], NULL));
    j_pins += 2;
    for (; j_pins < root.gates[i].size() - 1; j_pins += 2)
      p_gate->ins.push_back(netl->addNetMap(root.gates[i][j_pins], p_gate));
      //p_gate->ins.push_back(netl->addNet(root.gates[i][j_pins], p_gate));

    p_gate->repeat = 0;
    netl->addGate(p_gate);
  }
  return true;
}

bool netlistreader_verilog::parse_flat_initials(netlist *netl, sim_data *simul_data) {
  Event ev_map;
  int currentTime = 0;
  int time = 0;
  
  for (size_t i = 0; i < root.initials.size(); ++i) {
    for (size_t j = 0; j < root.initials[i].size(); ++j) {
      // $dumpfile
      if (root.initials[i][j] == "$dumpfile") {
        std::string vcdname;
        j = j + 2;
        while (root.initials[i][j] != ")") {
          if (root.initials[i][j] != "\"") 
            vcdname = vcdname + root.initials[i][j];
          ++j;
        }
        simul_data->setVCDname(vcdname);
        ++j;
        continue;
      } // $dumpfile

      // read delay #
      if (root.initials[i][j] == "#") {
        currentTime = atoi(root.initials[i][++j].c_str());
        time += currentTime;
        if(simul_data->endTime < time)
          simul_data->endTime = time;
        ++j;
        //ev_map = *(simul_data->addMapEvent(time, netl->addNet(root.initials[i][j], NULL), LogicLevel(atoi(root.initials[i][j + 2].c_str())), false));
        ev_map = *(simul_data->addMapEvent(time, netl->addNetMap(root.initials[i][j], NULL), LogicLevel(atoi(root.initials[i][j + 2].c_str())), false));
        j += 3;
        continue;
      } // read delay #

    }
  }
  return true;
}

bool netlistreader_verilog::parse_flat_assigns(netlist *netl, sim_data *simul_data) {
  if (root.assigns.size()) {
    for(size_t i = 0; i < root.assigns.size(); ++i) {
      // logic functions
      int delay = 0;
      int del = 0;
      // Setting delay
      if (root.assigns[i][1] == "#") {
        del = 2;
        if((root.assigns[i][2].find_first_not_of( "0123456789" ) != std::string::npos)) {
          printf("__err__ : Unknown type of token: delay should be specified as integer.\n");
          return false;
        } else {
          delay = atoi(root.assigns[i][2].c_str());  
        }
      }
      //
      // not
      //
      if (root.assigns[i][3 + del] == "~" ) {
        gate *p_gate = NULL;
        p_gate = CreateGate("not_"+root.assigns[i][1 + del], "not");
        if (!p_gate)
          return false;
        p_gate->outs.push_back(netl->addNetMap(root.assigns[i][1 + del], NULL));
        p_gate->ins.push_back(netl->addNetMap(root.assigns[i][4 + del], p_gate));
        p_gate->setDelay(delay);
        netl->addGate(p_gate);
      }
      //
      // or, and, xor
      //
      if (root.assigns[i][4 + del] == "|" || root.assigns[i][4 + del] == "&" || root.assigns[i][4 + del] == "^") {
        gate *p_gate = NULL;
        std::string type;
        if (root.assigns[i][4 + del] == "|")
          type = "or";
        if (root.assigns[i][4 + del] == "&")
          type = "and";
        if (root.assigns[i][4 + del] == "^")
          type = "xor";
        p_gate = CreateGate(type + root.assigns[i][1 + del], type);
        if (!p_gate)
          return false;
        p_gate->outs.push_back(netl->addNetMap(root.assigns[i][1 + del], NULL));
        int step = 0;
        while(";" != root.assigns[i][3 + del + step]) {
          if(root.assigns[i][3 + del + step] == "|" || root.assigns[i][3 + del + step] == "&" || root.assigns[i][3 + del + step] == "^") {
            step++;
            continue;
          }
          p_gate->ins.push_back(netl->addNetMap(root.assigns[i][3 + del + step], p_gate));
          step++;
        }
        p_gate->setDelay(delay);
        netl->addGate(p_gate);
      }
    }
  }
  return true;
}

bool netlistreader_verilog::parse_flat_alwayses(netlist *netl, sim_data *simul_data) {
  
  Event ev_map;
  LogicLevel level = level_0;
  int cycleTime = 0;

  for (size_t i = 0; i < root.alwayses.size(); ++i) {
    gate* p_gate = NULL;
    bool behGate = false;
    if (root.alwayses[i][1] == "@") {


      std::vector< std::string > behPins;
      std::string gateName = root.alwsGates[i].name;
      for (int kk = 1; kk < root.alwaysGates[i].size(); kk++)
        behPins.push_back(root.alwaysGates[i][kk]);
      p_gate = CreateGate(gateName, "beh");
      for (size_t iin = 0; iin < root.alwsGates[i].inputs.size(); iin++)
        p_gate->ins.push_back(netl->addNetMap(root.alwsGates[i].inputs[iin], p_gate));
      for (size_t iout = 0; iout < root.alwsGates[i].outputs.size(); iout++)
        p_gate->outs.push_back(netl->addNetMap(root.alwsGates[i].outputs[iout], NULL));
      behGate = true;
    }
    
    if (behGate) {
      bool ended = false;
      size_t ii = 1;
      while (!ended) {
        if ((root.alwayses[i][ii] == "begin")) {
          ii++;
          continue;
        }
        if ((root.alwayses[i][ii] == "@") && (root.alwayses[i][ii+1] != "posedge") && (root.alwayses[i][ii+1] != "negedge")) {
          p_gate->tokens.push_back("cmp");
          p_gate->tokens.push_back(root.alwayses[i][ii+1]);
          p_gate->tokens.push_back("*");
          ii = ii + 2;
          continue;
        }
        if (root.alwayses[i][ii] == "posedge") {
          p_gate->tokens.push_back("cmp");
          p_gate->tokens.push_back(root.alwayses[i][ii + 1]);
          p_gate->tokens.push_back("/");
          ii = ii + 3;
          continue;
        }
        if (root.alwayses[i][ii] == "negedge") {
          p_gate->tokens.push_back("cmp");
          p_gate->tokens.push_back(root.alwayses[i][ii + 1]);
          p_gate->tokens.push_back("/");
          ii = ii + 3;
          continue;
        }
        if (root.alwayses[i][ii] == "end") {
          p_gate->tokens.push_back("@alwsend");
          p_gate->jumps["@alwsend"] = p_gate->tokens.size() - 1;
          ended = true;
          continue;
        }
        ++ii;
      }
      if (ended) {
        p_gate->tokens.push_back("exit");
        return true;
      }
    }
    

      

      // From here we're starting to parse behavioral description into assembler code

   //   printf("__err__ : Sorry, behavior description is not supported yet.\n");
   //   return false;
   // }
    if (root.alwayses[i][3] != root.alwayses[i][6] || root.alwayses[i][5] != "~") {
      printf("__err__ : Sorry, at the moment only clock generators are available.\n");
      return false;
    }
    cycleTime = atoi(root.alwayses[i][2].c_str());

    for (int time = 0; time < simul_data->endTime; time += cycleTime) {
      ev_map = *(simul_data->addMapEvent(time, netl->addNetMap(root.alwayses[i][3], NULL), level, false));
      switch (level) {
      case level_0: 
        level = level_1;
        break;
      case level_1: 
        level = level_0;
        break;
      }
    }
  }

  return true;
}

bool netlistreader_verilog::parse_flat_netlist(netlist *netl, sim_data *simul_data) {
  if (!parse_flat_gates(netl, simul_data))
    return false;
  if (!parse_flat_initials(netl, simul_data))
    return false;
  if (!parse_flat_assigns(netl, simul_data))
    return false;
  if (!parse_flat_alwayses(netl, simul_data))
    return false;

  netl->repeats = new int[netl->gates.size()];

  for (size_t i = 0; i < netl->gates.size(); ++i) {
    netl->gates[i]->repeat = &netl->repeats[i];
    if (!netl->gates[i]->postprocess())
      return false;
  }

  memset(netl->repeats, 0, sizeof(int)*netl->gates.size());

  return true;
}


/*
bool netlistreader_verilog::read(netlist *netl, sim_data *simul_data) {
  gate* p_gate = NULL;

  //size_t i = 0;
  netl->neededSteps = 0;
  bool gateInputs = false;
  bool readStates = false;
  bool readNames = false;
  bool readModule = false;
  Event ev;
  Event ev_map;
  ev_map.time = 0;
  ev.time=0;
  int currentTime = 0;
  int time = 0;


  if(!tokenize())
    return false;

  for (size_t i = 0; i < tokens.size(); i++) {
    if (tokens[i].pos == 1) {
      gateInputs = false;
      readStates = false;
      readNames = false;
    }

    // ���������� $dumpfile, ���� ��� $dumpvars

    if(tokens[i].item == "$dumpfile") {
        std::string vcdname;
        i=i+2;
        while(tokens[i].item != ")") {
            if(tokens[i].item != "\"") {
                vcdname = vcdname + tokens[i].item;
            }
            ++i;
        }
        simul_data->setVCDname(vcdname);
    }

    //New module for Verilog (basic)
    if (readModule) {
      if ((tokens[i].item == "input") || ((tokens[i].item == "output") && (tokens[i].pos == 1))) {

      }
      if ((tokens[i].item == "nor") || (tokens[i].item == "nand") || (tokens[i].item == "or") || (tokens[i].item == "and") || (tokens[i].item == "not") || (tokens[i].item == "xor") || (tokens[i].item == "xnor") || (tokens[i].item == "buf")) {
        int gotDelay = 0;
        int gotName = 1;
        int gateDelay = 0;
        if (tokens[i+1].item == "#") {
          gateDelay = atoi(tokens[i+2].item.c_str());
          gotDelay = 2;
        }
        if (tokens[i+gotDelay+1].item != "(") {
          gotName = 1;
        }
        p_gate = CreateGate(tokens[i + gotDelay + gotName].item, tokens[i].item);
        p_gate->delay = 0;
        if (gateDelay != 0) {
          p_gate->delay = gateDelay;
        }

        if (!p_gate)
          continue;
        p_gate->repeat = 0;
        netl->addGate(p_gate);


        p_gate->outs.push_back(netl->addNet(tokens[i + gotDelay + gotName + 2].item, NULL));
        gateInputs = true;
        i += gotDelay + gotName + 3;
      }
      if (gateInputs) {
        while (tokens[i].item != ")") {
          if (tokens[i].item != ",") {
            p_gate->ins.push_back(netl->addNet(tokens[i].item, p_gate));
          }
          i++;
        }
        gateInputs = false;
      }
    }

    if ((tokens[i].item == "module") && (tokens[i].pos == 1)) {
      i += 3;
      readModule = true;
    }

    if ((tokens[i].item == "endmodule")) {
      readModule = false;
    }


    if ((tokens[i].item == "#")) { // && (tokens[i].pos == 1)) {
      currentTime = atoi(tokens[i+1].item.c_str());
      i++;
      time += currentTime;
      i++;
      //ev = *(simul_data->addEvent(time, netl->returnNet(tokens[i].item), LogicLevel(atoi(tokens[i + 2].item.c_str()))));
      while((tokens[i].item != "#") || (tokens[i].item != "$finish")) {
          if((tokens[i].item == "#") || (tokens[i].item == "$finish")) {
          simul_data->endTime = time;
          i--;
                break;
            }
            //ev = *(simul_data->addEvent(time, netl->addNet(tokens[i].item, NULL), LogicLevel(atoi(tokens[i + 2].item.c_str()))));
        ev_map = *(simul_data->addMapEvent(time, netl->addNet(tokens[i].item, NULL), LogicLevel(atoi(tokens[i + 2].item.c_str())), false));
        //ev_map.delayed.push_back(false);
            i += 4;
        }

    }

  }

  netl->repeats = new int [netl->gates.size()];

  for (size_t i = 0; i < netl->gates.size(); ++i) {
    netl->gates[i]->repeat = &netl->repeats[i];
    if (!netl->gates[i]->postprocess())
      return false;
  }

  memset(netl->repeats, 0, sizeof(int)*netl->gates.size());

  return true;
}
*/

