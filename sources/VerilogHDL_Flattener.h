#pragma once

#include <vector>
#include <string>

enum ModuleLineType {
  mlt_undefined = 0,
  mlt_instance,
  mlt_assign,
  mlt_always,
};

// This is pure virtual class that is base of one line of Verilog code classes
class ModuleLine {
protected:
  ModuleLineType    line_type;
public:
  ModuleLine(ModuleLineType type) : line_type(type) {};
public:
  ModuleLineType Type() { return line_type; };
};

class ML_Instance : public ModuleLine {
public:
  std::string               name,
                            type;
  std::vector<std::string>  nodes;
public:
  ML_Instance() : ModuleLine(mlt_instance), name(""), type("") {};
};

class ML_Assign : public ModuleLine {
public:
  std::vector<std::string>  expression;
public:
  ML_Assign() : ModuleLine(mlt_assign) {};
};

class ML_Always : public ModuleLine {
public:
  std::vector<std::string>  expression;
public:
  ML_Always() : ModuleLine(mlt_always) {};
};

// This class stores module from "module" keyword to "endmodule" keyword 
class Module {
public:
  bool                      isRTL;
  std::string               module_name;
  std::vector<std::string>  pins;
  std::vector<std::string>  lines;
  std::vector<ModuleLine *> typed_lines;
public:
  Module() : isRTL(false), module_name("") {};
 ~Module() {
   for (size_t i = 0; i < lines.size(); ++i)
     delete typed_lines[i];
   lines.clear();
  }
};

struct token {
  std::string item;
  size_t      line,
    pos;
};

class VerilogHDL_Flattener {
  std::vector<token>        tokens;
  std::vector<Module *>     modules;
  std::vector<std::string>  lines;
public:
  bool Read(std::string file_name, std::string root_module_name);
private:
  bool Tokenize(std::string file_name);
  bool ParseModules(std::string root_module_name);
  void PostProcess();
  bool MakeTheBiggestJobEver(std::string root_module_name);
  bool Dump(std::string file_name, std::string root_module_name);
};
