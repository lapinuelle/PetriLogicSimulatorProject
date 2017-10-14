#include "netlistreader.h"

void clear_at_left(std::string &line) {
  size_t pos = line.find_first_not_of(" \t\n");
  if(!pos)
    return;
  if(pos == std::string::npos) {
    line.erase();
    return;
  }
  line.erase(line.begin(), line.begin() + pos);
}

void clear_at_right(std::string &line) {
  size_t pos = line.find_last_not_of(" \t\n");
  if(pos == std::string::npos) {
    line.erase();
    return;
  }
  if(pos != line.length())
    line.erase(line.begin() + pos + 1, line.end());
}

bool netlistreader_verilog::tokenize() {
  FILE *p_file = fopen(fileName.c_str(), "rt");
  if(!p_file)
    return false;
  
  char buffer[4096];
  std::string line;
  token       tk;
  
  size_t line_no = 0, token_no = 0, pos = 0;
  
  while(!feof(p_file)) {
    fgets(buffer, 4095, p_file);
    line = buffer;
    
    ++line_no;
    token_no = 0;
    
    clear_at_left(line);
    clear_at_right(line);
    
    if(line.empty())
      continue;
    
    while(!line.empty()) {
      clear_at_left(line);
	  
      ++token_no;
      tk.line = line_no;
      tk.pos = token_no;
      pos = line.find_first_of(" \t\n\"()+-*/=.,[]{}#@!~;`");
      if(pos == std::string::npos) {
        tk.item = line;
        tokens.push_back(tk);
        break;
      }
      if(pos == 0) {
        tk.item = line.substr(0, 1);
        tokens.push_back(tk);
        line.erase(line.begin(), line.begin() + 1);
        continue;
      }
      tk.item = line.substr(0, pos);
      tokens.push_back(tk);
      line.erase(line.begin(), line.begin() + pos);
    }
  }
  fclose(p_file);
  if(tokens[tokens.size() - 1].item == tokens[tokens.size() - 2].item)
	tokens.pop_back();
  
  // Find composite operands
  for(size_t i = 0; i < tokens.size() - 1; ++i) {
    // TopGun:
    // Here we should also check line numbers and so on to be shure that this is a comment
    // But it's not implemented yet.
    // This solution has a huge bug: I can't find any difference between '//' and '/ /' :(
    if(tokens[i].item == "/" && tokens[i + 1].item == "/") {
      tokens[i + 1].item = "//";
      tokens.erase(tokens.begin() + i);
    }
    if(tokens[i].item == "/" && tokens[i + 1].item == "*") {
      tokens[i + 1].item = "/*";
      tokens.erase(tokens.begin() + i);
    }
    if(tokens[i].item == "*" && tokens[i + 1].item == "/") {
      tokens[i + 1].item = "*/";
      tokens.erase(tokens.begin() + i);
    }
    if(tokens[i].item == "=" && tokens[i + 1].item == "=") {
      tokens[i + 1].item = "==";
      tokens.erase(tokens.begin() + i);
    }
    if(tokens[i].item == "&" && tokens[i + 1].item == "&") {
      tokens[i + 1].item = "&&";
      tokens.erase(tokens.begin() + i);
    }
    if(tokens[i].item == "|" && tokens[i + 1].item == "|") {
      tokens[i + 1].item = "||";
      tokens.erase(tokens.begin() + i);
    }
    if(tokens[i].item == ">" && tokens[i + 1].item == "=") {
      tokens[i + 1].item = ">=";
      tokens.erase(tokens.begin() + i);
    }
    if(tokens[i].item == "<" && tokens[i + 1].item == "=") {
      tokens[i + 1].item = "<=";
      tokens.erase(tokens.begin() + i);
    }
    if(tokens[i].item == "!" && tokens[i + 1].item == "=") {
      tokens[i + 1].item = "!=";
      tokens.erase(tokens.begin() + i);
    }
  }
  
  // Remove comments from file
  for(size_t i = 0; i < tokens.size(); ++i) {
    if(tokens[i].item == "//") {
      line_no = tokens[i].line;
      while(tokens[i].line == line_no && i <= tokens.size())
        tokens.erase(tokens.begin() + i);
      --i;
      continue;
    }
    if(tokens[i].item == "/*") {
      while(tokens[i].item != "*/" && i <= tokens.size())
        tokens.erase(tokens.begin() + i);
      --i;
      continue;
    }
  }
  
  return true;
}

inetlistreader::inetlistreader(std::string fileName) : fileName(fileName) {
}

inetlistreader::~inetlistreader() {
}

inetlistreader *get_appropriate_reader(std::string fileName) {
  // First of all let's try to open file for reading
  FILE *p_file = fopen(fileName.c_str(), "r");
  if(!p_file) {
    printf("__err__ : Can't open input file '%s'\n          Abort.\n", fileName.c_str());
    return NULL;
  }
  fclose(p_file);
  p_file = NULL;

  // And then we have to try to create verilog reader
  inetlistreader *p_reader = NULL;
  p_reader = new netlistreader_verilog(fileName);
  if(p_reader->is_my_format())
    return p_reader;
  delete p_reader;
  p_reader = NULL;
  
  return NULL;
}

void free_reader(inetlistreader *p_reader) {
  if(!p_reader)
    return;
  delete p_reader;
  p_reader = NULL;
}
