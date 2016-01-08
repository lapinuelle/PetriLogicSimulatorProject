#include "gates.h"

//methods for Petri
void gate::t_minus() {
    for(size_t i =0 ; i < ins.size(); ++i)
      ins_temp[i] = ins[i]->value;
  }

void gate::t_plus() {
    for(size_t i =0 ; i < outs.size(); ++i)
      outs[i]->value = outs_temp[i];
  }

// Inverter

gate_not::gate_not(std::string nameFile) {
    name = nameFile;
    ins.reserve(1);
    outs.reserve(1);
    //ins_temp.resize(ins.capacity());
    //outs_temp.resize(outs.capacity());
  }

void gate_not::operate() {
  outs_temp[0] = ! ins_temp[0];
  }

bool gate_not::postprocess() {
  if(ins.capacity() != 1)
    return false;
  if(outs.capacity() != 1)
    return false;
  ins_temp.resize(ins.capacity());
  outs_temp.resize(outs.capacity());
  return true;
}

// AND-gate

gate_and::gate_and(std::string nameFile) {
    //ins.reserve(2);
    outs.reserve(1);
    name=nameFile;
    //ins_temp.resize(ins.capacity());
    //outs_temp.resize(outs.capacity());
  }

void gate_and::operate() {
    outs_temp[0] = ins_temp[0] * ins_temp[1];
	if (ins_temp.size() > 2) {
		for (size_t i = 2; i < ins_temp.size(); i++) {
			outs_temp[0] = outs_temp[0] * ins_temp[i];
		}
	}
  }

bool gate_and::postprocess() {
  if(outs.capacity() != 1)
    return false;
  ins_temp.resize(ins.capacity());
  outs_temp.resize(outs.capacity());
  return true;
}

// OR-gate

gate_or::gate_or(std::string nameFile) {
    //ins.reserve(2);
    outs.reserve(1);
    name=nameFile;
    //ins_temp.resize(ins.capacity());
    //outs_temp.resize(outs.capacity());
  }

void gate_or::operate() {
    outs_temp[0] = ins_temp[0] + ins_temp[1];
	if (ins_temp.size() > 2) {
		for (size_t i = 2; i < ins_temp.size(); i++) {
			outs_temp[0] = outs_temp[0] + ins_temp[i];
		}
	}
  }

bool gate_or::postprocess() {
  if(outs.capacity() != 1)
    return false;
  ins_temp.resize(ins.capacity());
  outs_temp.resize(outs.capacity());
  return true;
}

// NAND-gate

gate_nand::gate_nand(std::string nameFile) {
    //ins.reserve(2);
    outs.reserve(1);
    name=nameFile;
    //ins_temp.resize(ins.capacity());
    //outs_temp.resize(outs.capacity());
  }

void gate_nand::operate() {
    outs_temp[0] = !(ins_temp[0] * ins_temp[1]);
	if (ins_temp.size() > 2) {
		for (size_t i = 2; i < ins_temp.size(); i++) {
			outs_temp[0] = ! (outs_temp[0] * ins_temp[i]);
		}
	}
  }

bool gate_nand::postprocess() {
  if(outs.capacity() != 1)
    return false;
  ins_temp.resize(ins.capacity());
  outs_temp.resize(outs.capacity());
  return true;
}

// NOR-gate

gate_nor::gate_nor(std::string nameFile) {
    //ins.reserve(2);
    outs.reserve(1);
    name=nameFile;
    //ins_temp.resize(ins.capacity());
    //outs_temp.resize(outs.capacity());
  }

void gate_nor::operate() {
    outs_temp[0] = ! (ins_temp[0] + ins_temp[1]);
	if (ins_temp.size() > 2) {
		for (size_t i = 2; i < ins_temp.size(); i++) {
			outs_temp[0] = ! (outs_temp[0] + ins_temp[i]);
		}
	}
  }

bool gate_nor::postprocess() {
  if(outs.capacity() != 1)
    return false;
  ins_temp.resize(ins.capacity());
  outs_temp.resize(outs.capacity());
  return true;
}

