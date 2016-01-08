#include "netlistreader.h"

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
  return NULL;
}

bool netlistreader_verilog::read(netlist *netl, sim_data *simul_data) {
  gate* p_gate = NULL;
  
  //size_t i = 0;
  netl->neededSteps = 0;
  bool gateInputs = false;
  bool readStates = false;
  bool readNames = false;
  bool readModule = false;
  Event ev;
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
    
    //New module for Verilog (basic)
    if (readModule) {
      if ((tokens[i].item == "input") || ((tokens[i].item == "output") && (tokens[i].pos == 1))) {
        
      }
      if ((tokens[i].item == "nor") || (tokens[i].item == "nand") || (tokens[i].item == "or") || (tokens[i].item == "and") || (tokens[i].item == "not")) {
        p_gate = CreateGate(tokens[i + 1].item, tokens[i].item);
        
        if (!p_gate)
          continue;
        p_gate->repeat = 0;
        netl->addGate(p_gate);
        p_gate->outs.push_back(netl->addNet(tokens[i + 3].item, NULL));
        gateInputs = true;
        i += 4;
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
      while (tokens[i].item != ")") {
        if (tokens[i].item != ",") {
          simul_data->dumpNames.push_back(tokens[i].item);
        }
        i++;
      }
      
      readModule = true;
    }
    
    if ((tokens[i].item == "endmodule")) {
      readModule = false;
    }
    
    
    
    if ((tokens[i].item == "#") && (tokens[i].pos == 1)) {
      i++;
      currentTime = atoi(tokens[i].item.c_str());
      time += currentTime;
      i++;
      ev = *(simul_data->addEvent(time, netl->returnNet(tokens[i].item), LogicLevel(atoi(tokens[i + 2].item.c_str()))));
      i += 3;
      /*
      if (atoi(tokens[i + 1].item.c_str()) != currentTime) {
        simul_data->eventChain.push_back(ev);
        ev.netsChain.clear();
        ev.statesChain.clear();
        ev.time = 0;
        ev.time = atoi(tokens[i + 1].item.c_str());
      }
      currentTime = ev.time;
      ev.netsChain.push_back(netl->returnNet(tokens[i + 2].item));
      if (tokens[i + 4].item == "0")
        ev.statesChain.push_back(level_0);
      if (tokens[i + 4].item == "1")
        ev.statesChain.push_back(level_1);
      if (tokens[i + 4].item == "U")
        ev.statesChain.push_back(level_u);
        
      i = i + 5;
      */
      
    }
    
  }
  
  for(size_t i = 0; i < netl->gates.size(); ++i)
    if(!netl->gates[i]->postprocess())
      return false;
  return true;
}
