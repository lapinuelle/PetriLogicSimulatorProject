#include "sdf.h"

SDF::~SDF() {
  for (size_t i = 0; i < celllines.size(); ++i)
    delete celllines[i];
  celllines.clear();
}

bool SDF::read(std::string &filename) {
  FILE *p_file = fopen(filename.c_str(), "rt");
  if (!p_file) {
    printf("__err__ : Specified SDF file does not exist. Abort.");
    return false;
  }

  CellLines *p_cell = NULL;

  char buf[256];
  while (!feof(p_file)) {
    fgets(buf, 255, p_file);
    if (!strcmp(buf, "(CELL\n")) {
      p_cell = new CellLines;
      celllines.push_back(p_cell);
      fgets(buf, 255, p_file);
      while (strcmp(buf, ")\n")) {
        p_cell->lines.push_back(buf);
        fgets(buf, 255, p_file);
      }
    }
  }
  fclose(p_file);

  parse();

  return true;
}

void clr_at_left(std::string &line) {
  size_t pos = line.find_first_not_of(" \t\n");
  if (!pos)
    return;
  if (pos == std::string::npos) {
    line.erase();
    return;
  }
  line.erase(line.begin(), line.begin() + pos);
}

void clr_at_right(std::string &line) {
  size_t pos = line.find_last_not_of(" \t\n");
  if (pos == std::string::npos) {
    line.erase();
    return;
  }
  if (pos != line.length())
    line.erase(line.begin() + pos + 1, line.end());
}


void SDF::parse() {
  std::vector<std::string>  tokens;
  std::string line;
  size_t pos = std::string::npos;

  std::string cellName, instanceName;

  for(size_t i = 0; i < celllines.size(); ++i) {

    tokens.clear();
    for (size_t j = 0; j < celllines[i]->lines.size(); ++j) {
      line = celllines[i]->lines[j];

      clr_at_left(line);
      clr_at_right(line);

      while (!line.empty()) {
        clr_at_left(line);

        pos = line.find_first_of(" \t\n\"():");
        if (pos == std::string::npos) {
          tokens.push_back(line);
          break;
        }
        if (pos == 0) {
          tokens.push_back(line.substr(0, 1));
          line.erase(line.begin(), line.begin() + 1);
          continue;
        }
        tokens.push_back(line.substr(0, pos));
        line.erase(line.begin(), line.begin() + pos);
      }
    }
    
    for (size_t j = 0; j < tokens.size(); ++j) {
      if (tokens[j] == "CELLTYPE")
        cellName = tokens[j + 2];
      if (tokens[j] == "INSTANCE")
        instanceName = tokens[j + 1];
    }
    
    if (instanceName == ")")
      continue;

    double delay = 0.0;
    for (size_t j = 0; j < tokens.size(); ++j) {
      if (tokens[j] != "IOPATH")
        continue;
      if (tokens[j + 1] == "(")
        delay = atof(tokens[j + 7].c_str());
      else
        delay = atof(tokens[j + 4].c_str());
      break;
    }
    
    delays[instanceName] = delay;
  }
}


