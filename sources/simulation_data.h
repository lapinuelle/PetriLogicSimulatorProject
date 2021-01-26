#if !defined(SD_HEADER)
#define SD_HEADER

#include "logiclevel.h"
#include "nets.h"
#include "gates.h"
#include <vector>
#include <string>
#include "event.h"
#include "stack.h"
#include <map>

struct TimescaleInfo {
  std::string str_timescale, str_precision;
  double      timescale, precision;
};


class sim_data {
public:
  std::vector <LogicLevel> oneState;
	std::vector <std::vector <LogicLevel> > allStates;
	std::vector <std::string> dumpNames;
	std::vector <LogicLevel> outs_temp;
	
  // new data
  std::vector <Event> eventChain;
  Event* addEvent(float time, net* net, LogicLevel state);

  // map data
  float endTime;
  std::map<float, Event*> newEventChain;
  Event* addMapEvent(float time, net* net, LogicLevel state);
  Event* addMapEvent(float time, net* net, LogicLevel state, bool delayedState);


  // end of map data


  sim_data();
  void setVCDname(std::string name);
  std::string getVCDname();
  TimescaleInfo timescale;
private:
	std::string vcdName;
};

#endif
