#if !defined(NETS_HEADER)
#define NETS_HEADER

#include <string>
#include <vector>
#include <iostream>
#include "logiclevel.h"

class gate;

class net {
public:
  std::string name;
  std::string realName;
  LogicLevel value;
  char type;
  std::string stability;
  int delay;
  std::vector<gate *>  gates;
  net(const std::string &net_name);
};

#endif