#if !defined(ST_HEADER)
#define ST_HEADER
#include "gates.h"
#include "simulation_data.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

class stack {
public:
  std::vector <gate*> gatesChain;                 // стек
  int free;                                       // первый свободный
  int busy;                                       // первый занятый
  int size;                                       // размер стека
  void push_back(gate* gate);                     // добавление вентиля в стек на позицию free
  void eject();                                   // "извлечение" вентиля из стека. Значенте busy++
  void reset();                                   // сброс стека
  stack(int size);
  ~stack();
};


#endif
