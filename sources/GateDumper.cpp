#include "GateDumper.h"



GateDumper::GateDumper()
{
}

void GateDumper::init(std::string name)
{
  this->fileName = name + ".gat";
  if (!(this->dumpFile = fopen(fileName.c_str(), "w+")))
    printf("Can't open file!");
  fclose(this->dumpFile);
    
}

void GateDumper::dumpTime(int time)
{
  FILE *p_file = fopen(this->fileName.c_str(), "a+t");
  if (!p_file)
    return;
  fprintf(p_file, "#%d\n", time);
  fclose(p_file);
}

void GateDumper::dumpGateName(std::string gateName)
{
  FILE *p_file = fopen(this->fileName.c_str(), "a+t");
  if (!p_file)
    return;
  fprintf(p_file, "\t%s\n", gateName.c_str());
  fclose(p_file);
}

void GateDumper::finish()
{
  //fclose(this->dumpFile);
  dumpFile = nullptr;

}


GateDumper::~GateDumper()
{
  
}
