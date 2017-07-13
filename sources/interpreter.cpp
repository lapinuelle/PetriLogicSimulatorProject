#include "interpreter.h"
#include <map>
#include "gates.h"
#include "logiclevel.h"
#include "nets.h"

void interpreter::cmp(net* net, LogicLevel value) {
  if (net->value == value)
    flags["ZF"] = 1;
  if (net->value > value) {
    flags["ZF"] = 0;
    flags["GF"] = 1;
  }
  if (net->value < value) {
    flags["ZF"] = 0;
    flags["LF"] = 1;
  }
}

void interpreter::cmp(net* net, char value) {
  if (net->stability == value)
    flags["ZF"] = 1;
}

void interpreter::mov(LogicLevel value, net* net) {
  net->value = value;
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

void interpreter::and(LogicLevel value1, LogicLevel value2) {
  logRegisters["AND"] = value1 + value2;
}

void interpreter:: or (LogicLevel value1, LogicLevel value2) {
  logRegisters["OR"] = value1 * value2;
}

void interpreter:: xor (LogicLevel value1, LogicLevel value2) {
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

void interpreter::operate(std::vector<std::string> commands, std::map<std::string, int> jumps) {
  int commandsSize = commands.size();
  int i = 0;
  while (i < commandsSize) {
    if (i >= commandsSize)
      break;
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
      
    }


    i++;
  }
  return;
}

