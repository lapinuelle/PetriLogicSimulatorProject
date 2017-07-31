#if !defined(LOGLEV_HEADER)
#define LOGLEV_HEADER

#include <string>

enum LogicLevel {
  level_0 = 0,
  level_1,
  level_u,
};

LogicLevel atol(std::string);
LogicLevel operator ! (LogicLevel val);
LogicLevel operator * (LogicLevel val1, LogicLevel val2);
LogicLevel operator + (LogicLevel val1, LogicLevel val2);
LogicLevel operator ^ (LogicLevel val1, LogicLevel val2);
LogicLevel operator - (LogicLevel val1, LogicLevel val2);

#endif
