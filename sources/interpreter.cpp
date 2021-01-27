#include "interpreter.h"
#include <map>
#include "gates.h"
#include "logiclevel.h"
#include "netlist.h"
#include "nets.h"

LogicLevel interpreter::not(LogicLevel value) {
  if (value == level_0)
    return level_1;
  if (value == level_1)
    return level_0;
  return level_u;
}

LogicLevel interpreter::not(net* net) {
  for (size_t i = 0; i < this->modeGate->getInputs().size(); i++) {
    if (this->modeGate->getInputs[i]->name == net->name) {
      if (this->modeGate->getInternalInputValue(i) == level_0)
        return level_1;
      if (this->modeGate->getInternalInputValue(i) == level_1)
        return level_0;
      return level_u;
    }
  }
  for (size_t i = 0; i < this->modeGate->getOutputs().size(); i++) {
    if (this->modeGate->getOutputs()[i]->name == net->name) {
      if (this->modeGate->getInternalOutputValue(i) == level_0)
        return level_1;
      if (this->modeGate->getInternalOutputValue(i) == level_1)
        return level_0;
      return level_u;
    }
  }
}

void interpreter::cmp(net* net, LogicLevel value) {
  flags["ZF"] = 0;
  flags["GF"] = 0;
  flags["LF"] = 0;
  for (size_t i = 0; i < this->modeGate->getInputs().size(); i++) {
    if (this->modeGate->getInputs()[i]->name == net->name) {
      if (this->modeGate->getInternalInputValue(i) == value)
        flags["ZF"] = 1;
      if (this->modeGate->getInternalInputValue(i) > value) {
        flags["ZF"] = 0;
        flags["GF"] = 1;
      }
      if (this->modeGate->getInternalInputValue(i) < value) {
        flags["ZF"] = 0;
        flags["LF"] = 1;
      }
    }
  }
  
}

void interpreter::cmp(net* net, std::string value) {
  if (net->stability == value)
    flags["ZF"] = 1;
  if (value == "*")
    if (net->stability != "_")
      flags["ZF"] = 1;
}

void interpreter::mov(LogicLevel value, net* net) {
  for (size_t i = 0; i < this->modeGate->getOutputs().size(); i++) {
    if (this->modeGate->getOutputs()[i]->name == net->name) {
      this->modeGate->setInternalOutputValue(i, value);
    }
  }
}

void interpreter::add(LogicLevel value1, LogicLevel value2) {
  logRegisters["ADD"] = value1 + value2;
}

void interpreter::add(int value1, int value2) {
  intRegisters["ADD"] = value1 + value2;
}

void interpreter::sub(int value1, int value2) {
  intRegisters["SUB"] = value1 - value2;
}

void interpreter::pnand(LogicLevel value1, LogicLevel value2) {
  logRegisters["AND"] = value1 + value2;
}

void interpreter::pnor (LogicLevel value1, LogicLevel value2) {
  logRegisters["OR"] = value1 * value2;
}

void interpreter::pnxor (LogicLevel value1, LogicLevel value2) {
  logRegisters["XOR"] = value1 ^ value2;
}

void interpreter::reset() {
  intRegisters.clear();
  logRegisters.clear();
  flags.clear();
}

interpreter::interpreter() {
  this->reset();
}

void interpreter::operate(gate* currentGate, netlist* netl) {
  this->modeGate = currentGate;
  std::vector<std::string> commands = this->modeGate->tokens;
  std::map<std::string, int> jumps = this->modeGate->jumps;
  int commandsSize = commands.size();
  int i = 0;
  while (i < commandsSize) {
    if (i >= commandsSize)
      break;
    if ((commands[i] == "jmp"))
      i = jumps[commands[i + 1]];
    if ((commands[i] == "jz") || (commands[i] == "je") || (commands[i] == "JZ") || (commands[i] == "JE")) {
      if(flags["ZF"] == 1)
        i = jumps[commands[i + 1]];
    }
    if ((commands[i] == "jnz") || (commands[i] == "jne") || (commands[i] == "JNZ") || (commands[i] == "JNE")) {
      if (flags["ZF"] == 0)
        i = jumps[commands[i + 1]];
    }
    if ((commands[i] == "jg") || (commands[i] == "jlne") || (commands[i] == "JG") || (commands[i] == "JLNE")) {
      if ((flags["ZF"] == 0) && (flags["GF"] == 1))
        i = jumps[commands[i + 1]];
    }
    if ((commands[i] == "jl") || (commands[i] == "jgne") || (commands[i] == "JL") || (commands[i] == "JGNE")) {
      if ((flags["ZF"] == 0) && (flags["LF"] == 1))
        i = jumps[commands[i + 1]];
    }
    if ((commands[i] == "jge") || (commands[i] == "jnl") || (commands[i] == "JGE") || (commands[i] == "JNL")) {
      if ((flags["ZF"] == 0) || (flags["GF"] == 1))
        i = jumps[commands[i + 1]];
    }
    if ((commands[i] == "jle") || (commands[i] == "jng") || (commands[i] == "JLE") || (commands[i] == "JNG")) {
      if ((flags["ZF"] == 0) || (flags["LF"] == 1))
        i = jumps[commands[i + 1]];
    }
    if ((commands[i] == "cmp") || (commands[i] == "CMP")) {
      if ((commands[i + 2] == "\\") || (commands[i + 2] == "/") || (commands[i + 2] == "*")) {
        cmp(netl->returnNetMap(commands[i + 1]), commands[i + 2]);
      } else {
        cmp(netl->returnNetMap(commands[i + 1]), atol(commands[i + 2]));
      }
      i += 2;
    }
    if ((commands[i] == "mov") || (commands[i] == "MOV")) {
      if (commands[i + 2] == "~") {
        mov(not(netl->returnNetMap(commands[i + 3])), (netl->returnNetMap(commands[i + 1])));
      }
      else {
        if (netl->returnNetMap(commands[i + 2])) {
          mov(netl->returnNetMap(commands[i + 2])->value, (netl->returnNetMap(commands[i + 1])));
        }
        else {
          mov(atol(commands[i + 2]), (netl->returnNetMap(commands[i + 1])));
        }
        i += 2;
      }
    }
    if ((commands[i] == "add") || (commands[i] == "ADD")) {
      if (atol(commands[i + 1]) != level_u) {
        add(atol(commands[i + 1]), atol(commands[i + 2]));
      } else {
        add(atoi(commands[i + 1].c_str()), atoi(commands[i + 2].c_str()));
      }
      i += 2;
    }
    if ((commands[i] == "sub") || (commands[i] == "SUB")) {
      sub(atol(commands[i + 1]), atol(commands[i + 2]));
      i += 2;
    }
    if ((commands[i] == "and") || (commands[i] == "AND")) {
      pnand(atol(commands[i + 1]), atol(commands[i + 2]));
      i += 2;
    }
    if ((commands[i] == "or") || (commands[i] == "OR")) {
      pnor(atol(commands[i + 1]), atol(commands[i + 2]));
      i += 2;
    }
    if ((commands[i] == "xor") || (commands[i] == "XOR")) {
      pnxor(atol(commands[i + 1]), atol(commands[i + 2]));
      i += 2;
    }

    i++;
  }
  return;
}

