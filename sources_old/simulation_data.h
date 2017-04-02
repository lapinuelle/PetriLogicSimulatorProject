#if !defined(SD_HEADER)
#define SD_HEADER

#include "logiclevel.h"
#include "nets.h"
#include "gates.h"
#include <vector>
#include <string>
#include "event.h"
#include "stack.h"

class sim_data {
public:
  std::vector <LogicLevel> oneState;
	std::vector <std::vector <LogicLevel> > allStates;
	std::vector <std::string> dumpNames;
	std::vector <LogicLevel> outs_temp;
	
  // new data
  std::vector <Event> eventChain;
  Event* addEvent(int time, net* net, LogicLevel state);

  sim_data();
  void setVCDname(std::string name);
  std::string getVCDname();
private:
	std::string vcdName;
};

#endif
