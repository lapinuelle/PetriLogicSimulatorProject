#if !defined(GD_HEADER)
#define GD_HEADER

#include <string>

class GateDumper
{
private:
  std::string fileName;
  FILE *dumpFile = nullptr;
public:
  GateDumper();
  void init(std::string);
  void dumpTime(int time);
  void dumpGateName(std::string);
  void finish();

  ~GateDumper();
};

#endif

