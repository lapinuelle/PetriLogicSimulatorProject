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
  std::vector <gate*> gatesChain;                 ///< Стек
  int free;                                       ///< Первый свободный
  int busy;                                       ///< Первый занятый
  int size;                                       ///< Размер стека
  void push_back(gate* gate);                     ///< Добавление вентиля в стек на позицию free 1
  void push_back(std::vector <gate*> &);          ///< Добавление вентиля в стек на позицию free 2
  void eject();                                   ///< "Извлечение" вентиля из стека. Значенте busy++
  void reset();                                   ///< Сброс стека
  stack(int size);
  ~stack();
};


#endif
