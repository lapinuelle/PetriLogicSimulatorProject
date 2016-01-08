#include "logiclevel.h"

LogicLevel operator ! (LogicLevel val) {
  if(val == level_0)
    return level_1;
  if(val == level_1)
    return level_0;
  return level_u;
}

LogicLevel operator * (LogicLevel val1, LogicLevel val2) {
  if ((val1==level_1)&&(val2==level_1))
    return level_1;
  if ((val1==level_0)||(val2==level_0))
    return level_0;
  if ((val1==level_u)||(val2==level_u))
    return level_u;
  return level_u;
  
}

LogicLevel operator + (LogicLevel val1, LogicLevel val2) {
  if ((val1==level_0)&&(val2==level_0))
    return level_0;
  if ((val1==level_1)||(val2==level_1))
    return level_1;
  if ((val1==level_u)||(val2==level_u))
    return level_u;
  return level_u;
}
