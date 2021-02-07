#include "datawriter.h"
// blah

#include <time.h>
#include <string>
#include <vector>
#include <fstream>
#include "logiclevel.h"

datawriter::datawriter(const std::string fileName) : fileNameVCD(fileName), timescale_value("1ns") {
/*  size_t pos = fileNameVCD.find_last_of("_flat");
  if(pos != std::string::npos)
    fileNameVCD.erase(pos-4, fileNameVCD.length() - (pos-4));
  fileNameVCD += ".vcd";
  */
	fileNameVCD = fileName;
}

void datawriter::AddDumpVar(net *net) {
  size_t i = 0;
  for(i = 0; i < nets.size(); ++i)
    if(nets[i]->getName() == net->getName())
      return;
  nets.push_back(net);
}

void datawriter::PrintHeader() {
  time_t t = time(NULL);
  tm timeNow = *localtime(&t);


  FILE *p_file = fopen(fileNameVCD.c_str(), "w+t");
  if(!p_file)
    return;
  fprintf(p_file,  "$date\n");
  fprintf(p_file,  "  %02d:%02d:%02d %02d/%02d/%04d\n", timeNow.tm_hour, timeNow.tm_min, timeNow.tm_sec, timeNow.tm_mday, timeNow.tm_mon + 1, timeNow.tm_year + 1900);
  fprintf(p_file,  "$end\n");
  fprintf(p_file,  "$version\n");
  fprintf(p_file,  "  Alexander Lapin Petri Nets Logic Simulator.\n");
  fprintf(p_file,  "$end\n");
  fprintf(p_file,  "$comment\n");
  fprintf(p_file,  "  Any comment text.\n");
  fprintf(p_file,  "$end\n");
  fprintf(p_file,  "$timescale %s $end\n", timescale_value.c_str());
  fprintf(p_file,  "$scope module logic $end\n");
  int add1 = 33;
  for (size_t i = 0; i < nets.size(); ++i) {
    if ((i % 93) == 92)
      add1++;
    fprintf(p_file, "$var wire 1 %c%c %s $end\n", (char)(i % 93) + 33, (char)(add1 > 33 ? add1 : 32), nets[i]->realName.c_str());
  }
  
  fprintf(p_file,  "$upscope $end\n");
  fprintf(p_file,  "$enddefinitions $end\n");
  fprintf(p_file,  "$dumpvars\n");
  
  int add = 33;
  for (size_t i = 0; i < nets.size(); ++i) {
    
    if ((i % 93) == 92)
      add++;
    switch (nets[i]->value) {
    case level_0: fprintf(p_file, "0%c%c\n", (char)(i % 93) + 33, (char)(add > 33 ? add : 32));
      break;
    case level_1: fprintf(p_file, "1%c%c\n", (char)(i % 93) + 33, (char)(add > 33 ? add : 32));
      break;
    case level_u: fprintf(p_file, "x%c%c\n", (char)(i % 93) + 33, (char)(add > 33 ? add : 32));
      break;
    }
  }
  
  fprintf(p_file,  "$end\n");
  fclose(p_file);
};

void datawriter::DumpVars(int time) {
  FILE *p_file = fopen(fileNameVCD.c_str(), "a+t");
  if(!p_file)
    return;
  fprintf(p_file, "#%d\n", time);
  int add = 33;
  for (size_t i = 0; i < nets.size(); ++i) {
    if ((i % 93) == 92)
      add++;
    switch (nets[i]->value) {
    case level_0: fprintf(p_file, "0%c%c\n", (char)(i % 93) + 33, (char)(add > 33 ? add : 32));
      break;
    case level_1: fprintf(p_file, "1%c%c\n", (char)(i % 93) + 33, (char)(add > 33 ? add : 32));
      break;
    case level_u: fprintf(p_file, "x%c%c\n", (char)(i % 93) + 33, (char)(add > 33 ? add : 32));
      break;
    }
  }
  fclose(p_file);
}

void datawriter::SetTimescale(std::string val) {
  timescale_value = val;
}
