#include "gates.h"
#include "interpreter.h"


void gate::setDelay(int extDelay) {
  delay = extDelay;
}

//methods for Petri
void gate::t_minus() {
    for(size_t i =0 ; i < ins.size(); ++i)
      ins_temp[i] = ins[i]->value;
  }

bool gate::t_plus() {
  ++(*repeat);
  bool changed = false;
    for(size_t i =0 ; i < outs.size(); ++i)
      if (outs[i]->value != outs_temp[i]) {
        outs[i]->value = outs_temp[i];
        changed = true;
      }
    return changed;
  }


// BEH Gate

gate_beh::gate_beh(std::string nameFile) {
    name = nameFile;
  }

void gate_beh::t_minus() {
    for (size_t i = 0; i < ins.size(); ++i) {
      if (((ins[i]->value == level_0) || (ins[i]->value == level_u)) && (ins_temp[i] == level_1))
        ins[i]->stability = '/';
      if (((ins[i]->value == level_1) || (ins[i]->value == level_u)) && (ins_temp[i] == level_0))
        ins[i]->stability = '\\';
    ins_temp[i] = ins[i]->value;
    }
  }

bool gate_beh::t_plus() {
  ++(*repeat);
  bool changed = false;
    for(size_t i = 0; i < ins.size(); ++i)
      ins[i]->stability = '_';
    for(size_t i = 0; i < outs.size(); ++i)
      if (outs[i]->value != outs_temp[i]) {
        outs[i]->value = outs_temp[i];
        changed = true;
      }
    return changed;
  }

bool gate_beh::postprocess() {
  if (ins.capacity() != 1)
    return false;
  if (outs.capacity() != 1)
    return false;
  ins_temp.resize(ins.capacity());
  outs_temp.resize(outs.capacity());
  return true;
}

void gate_beh::operate() {
  interpreter *interp = new interpreter();
}


// Inverter

gate_not::gate_not(std::string nameFile) {
    name = nameFile;
    ins.reserve(1);
    outs.reserve(1);
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

// Buffer

gate_buf::gate_buf(std::string nameFile) {
    name = nameFile;
    ins.reserve(1);
    outs.reserve(1);
  }

void gate_buf::operate() {
  outs_temp[0] = ins_temp[0];
  }

bool gate_buf::postprocess() {
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
    outs.reserve(1);
    name=nameFile;
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
    outs.reserve(1);
    name=nameFile;
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
    outs.reserve(1);
    name=nameFile;
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
    outs.reserve(1);
    name=nameFile;
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

// XOR-gate

gate_xor::gate_xor(std::string nameFile) {
    outs.reserve(1);
    name=nameFile;
  }

void gate_xor::operate() {
    outs_temp[0] = ins_temp[0] ^ ins_temp[1];
	if (ins_temp.size() > 2) {
		for (size_t i = 2; i < ins_temp.size(); i++) {
			outs_temp[0] = outs_temp[0] ^ ins_temp[i];
		}
	}
  }

bool gate_xor::postprocess() {
  if(outs.capacity() != 1)
    return false;
  ins_temp.resize(ins.capacity());
  outs_temp.resize(outs.capacity());
  return true;
}

// XNOR-gate

gate_xnor::gate_xnor(std::string nameFile) {
    outs.reserve(1);
    name=nameFile;
  }

void gate_xnor::operate() {
    outs_temp[0] = !(ins_temp[0] ^ ins_temp[1]);
	if (ins_temp.size() > 2) {
		for (size_t i = 2; i < ins_temp.size(); i++) {
			outs_temp[0] = !(outs_temp[0] ^ ins_temp[i]);
		}
	}
  }

bool gate_xnor::postprocess() {
  if(outs.capacity() != 1)
    return false;
  ins_temp.resize(ins.capacity());
  outs_temp.resize(outs.capacity());
  return true;
}