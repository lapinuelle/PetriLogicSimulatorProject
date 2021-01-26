#if !defined(SDF_HEADER)
#define SDF_HEADER

#include <string>
#include <vector>
#include <map>

struct CellLines {
  std::vector<std::string>  lines;
};

struct SDF {
  std::vector<CellLines *>          celllines;
  std::map   <std::string, float>   delays;
public:
 ~SDF();
public:
  bool read(std::string &filename);
private:
  void parse();
};

#endif
