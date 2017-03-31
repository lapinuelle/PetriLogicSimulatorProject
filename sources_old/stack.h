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
  std::vector <gate*> gatesChain;                 // ����
  int free;                                       // ������ ���������
  int busy;                                       // ������ �������
  int size;                                       // ������ �����
  void push_back(gate* gate);                     // ���������� ������� � ���� �� ������� free
  void eject();                                   // "����������" ������� �� �����. �������� busy++
  void reset();                                   // ����� �����
  stack(int size);
  ~stack();
};


#endif
