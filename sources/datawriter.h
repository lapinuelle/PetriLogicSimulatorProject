#if !defined(DW_HEADER)
#define DW_HEADER

#include "gates.h"
#include "nets.h"
#include "netlist.h"
#include <vector>
#include <fstream>
#include <string>
#include <iostream>

/*
class datawriter {
public:
  std::string netNamesLine;
  std::string netStatesLine;
  std::ofstream outFile;
  datawriter();
  void collect(S* net);
  void cleanState();
  void printState();
  void write2file();
  void closeFile();
};
*/

class datawriter {
  std::string fileNameVCD;
  std::vector<net *>  nets;
  std::string         timescale_value;
public:
  datawriter(std::string fileName);
public:
  void AddDumpVar(net *net);
  void PrintHeader();
  void DumpVars(int time);
  void SetTimescale(std::string val);
};

#endif
