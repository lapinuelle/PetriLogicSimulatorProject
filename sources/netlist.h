#if !defined(NL_HEADER)
#define NL_HEADER
#include "logiclevel.h"
#include "nets.h"
#include "gates.h"

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <list>
#include <map>

class netlist {
public:
  // ������ ��������� ��� �������� ���������� �� ����
  std::vector <net*> nets;
  // ����� ���������, ��� �������� �� ����� �����, �������� ��� assign
  std::map<std::string, net*> netsMap;
  // ������ ��������� ��� �������� ���������� �� �������
  std::vector <gate*> gates;
  // ����� ���������, ��� �������� �� ����� �����, �������� ��� ��������������
  std::map<std::string, gate*> gatesMap;
  int *repeats;
  //std::vector <>
  net *addNet(const std::string &net_name, gate *gate);
  net* returnNet(std::string netName);

  net *addNetMap(const std::string &net_name, gate *gate);
  net* returnNetMap(std::string netName);
  gate* returnMapGate(std::string);
  std::vector<gate*> &returnGate(net* net);
  /*
    addGate - ������� �������, �������� � ������
  */
  void addGate(gate* gate);
  /*
    addGateMap - ����� �������. ������� �������, �������� � ��������� ���� std::map<std::string, gate*>. ������ � ����� ��������� ��������� ��� �������
  */
  void addGateMap(gate* gate);
	void connectNG();
  int neededSteps;
	netlist();
	~netlist();
};

#endif
