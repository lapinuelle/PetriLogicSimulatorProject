#if !defined(MODULE_HEADER)
#define MODULE_HEADER

class module {
public:
  vector <net*> ins;
  vector <net*> outs;
  bool isTestbench;
  std::string name;

};

#endif