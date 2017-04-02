#include "VerilogHDL_Flattener.h"

VerilogHDL_Flattener::VerilogHDL_Flattener() : was_written(false) {
}

VerilogHDL_Flattener::~VerilogHDL_Flattener() {
  if (!was_written)
    return;
  /*
  printf("Deleting file... ");
  if (remove(flat_file_name.c_str())) {
    printf("sorry, can't remove file :(\n");
  }
  else
    printf("done!\n");
  //*/
}

bool VerilogHDL_Flattener::Read(std::string file_name, std::string root_module_name) {
  // Tokenize input file
  if (!Tokenize(file_name))
    return false;
  // Parse tokens to find verilog modules and fill up "modules" vector
  if (!ParseModules(root_module_name))
    return false;
  hier_file_name = file_name;
  flat_file_name = file_name;
  if (flatten) 
      return true;
    // Detect RTL modules
  PostProcess();

  if(!MakeTheBiggestJobEver(root_module_name))
    return false;
  
  //hier_file_name = file_name;
  //flat_file_name = file_name;
  size_t pos = flat_file_name.find_last_of(".");
  if(pos == std::string::npos) {
    flat_file_name += "_flat";
  } else {
    flat_file_name.insert(pos, "_flat");
  }

  Dump(flat_file_name, root_module_name);
  return true;
}

std::string VerilogHDL_Flattener::GetFlatFileName() {
  if (flatten)
    return hier_file_name;
  return flat_file_name;
}

#pragma warning(push)
#pragma warning(disable: 4996)
bool VerilogHDL_Flattener::Tokenize(std::string file_name) {
  FILE *p_file = fopen(file_name.c_str(), "rt");
  if (!p_file) {
    printf("__err__ : Can't open file");
    return false;
  }

  char buffer[4096];
  std::string line;
  token       tk;

  size_t line_no = 0, token_no = 0, pos = 0;

  while (!feof(p_file)) {
    fgets(buffer, 4095, p_file);
	line = buffer;
	std::regex e ("(\/\/.*$)");
	std::string result;
	std::regex_replace (std::back_inserter(result), line.begin(), line.end(), e, std::string(""));
	line = result;
    ++line_no;
    token_no = 0;

    clear_at_left(line);
    clear_at_right(line);

    if (line.empty())
      continue;

    while (!line.empty()) {
      clear_at_left(line);
      ++token_no;
      tk.line = line_no;
      tk.pos = token_no;
	  
      pos = line.find_first_of(" \t\n()+-*/=.,[]{}#@!~;");
      if (pos == std::string::npos) {
        tk.item = line;
        tokens.push_back(tk);
        break;
      }
      if (pos == 0) {
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

  // Find composite operands
  for (size_t i = 0; i < tokens.size() - 1; ++i) {
    // TopGun:
    // This solution has a huge bug: I can't find any difference between '//' and '/ /' :(
    if (tokens[i].item == "/" && tokens[i + 1].item == "/") {
      tokens[i + 1].item = "//";
      tokens.erase(tokens.begin() + i);
    }
    if (tokens[i].item == "/" && tokens[i + 1].item == "*") {
      tokens[i + 1].item = "/*";
      tokens.erase(tokens.begin() + i);
    }
    if (tokens[i].item == "*" && tokens[i + 1].item == "/") {
      tokens[i + 1].item = "*/";
      tokens.erase(tokens.begin() + i);
    }
    if (tokens[i].item == "=" && tokens[i + 1].item == "=") {
      tokens[i + 1].item = "==";
      tokens.erase(tokens.begin() + i);
    }
    if (tokens[i].item == "&" && tokens[i + 1].item == "&") {
      tokens[i + 1].item = "&&";
      tokens.erase(tokens.begin() + i);
    }
    if (tokens[i].item == "|" && tokens[i + 1].item == "|") {
      tokens[i + 1].item = "||";
      tokens.erase(tokens.begin() + i);
    }
    if (tokens[i].item == ">" && tokens[i + 1].item == "=") {
      tokens[i + 1].item = ">=";
      tokens.erase(tokens.begin() + i);
    }
    if (tokens[i].item == "<" && tokens[i + 1].item == "=") {
      tokens[i + 1].item = "<=";
      tokens.erase(tokens.begin() + i);
    }
    if (tokens[i].item == "!" && tokens[i + 1].item == "=") {
      tokens[i + 1].item = "!=";
      tokens.erase(tokens.begin() + i);
    }
  }

  // Remove comments from file
  for (size_t i = 0; i < tokens.size(); ++i) {
    if (tokens[i].item == "//") {
      line_no = tokens[i].line;
      while (tokens[i].line == line_no && i <= tokens.size())
        tokens.erase(tokens.begin() + i);
      --i;
    }
    if (tokens[i].item == "/*") {
      while (tokens[i].item != "*/" && i <= tokens.size())
        tokens.erase(tokens.begin() + i);
      --i;
    }
  }

  return true;
}
#pragma warning(pop)


bool VerilogHDL_Flattener::ParseModules(std::string root_module_name) {
  size_t        line_no = 0;
  std::string   line;
  Module       *p_module = NULL;
  ModuleLine   *p_line = NULL;

  for (size_t i = 0; i < tokens.size(); ++i) {
    // Put all lines that are not modules to "lines" vector
    if (tokens[i].item != "module") {
      line_no = tokens[i].line;
      while (i < tokens.size() && tokens[i].line == line_no) {
        line += tokens[i].item;
        line += " ";
        ++i;
      }
      continue;
    }
    // So we are on module, let's parse it!
    p_module = new Module;
    modules.push_back(p_module);

    p_module->module_name = tokens[++i].item;
    ++i; // (
    if (tokens[i + 1].item == ")") {
      i += 3;
    }
    else {
      while (tokens[++i].item != ";") {
        p_module->pins.push_back(tokens[i].item);

        ++i; // ,
      }
      ++i; // ;
    }
    
    while (tokens[i].item != "endmodule") {
      // Windows "\n" bug fix
      if(tokens[i].item == "\r") {
        ++i;
      }
      
      
      if (tokens[i].item == "input" || tokens[i].item == "output" || tokens[i].item == "wire" || tokens[i].item == "reg") {
        line = tokens[i].item + " ";
        while (tokens[++i].item != ";") {
          line += tokens[i].item;
          line += " ";
        }
        line += tokens[i].item;
        p_module->lines.push_back(line);
        ++i;
        continue;
      }

      if (tokens[i].item == "initial" && tokens[i + 1].item == "begin") {
        line = tokens[i].item + " ";
        while (tokens[++i].item != "end") {
          line += tokens[i].item;
          line += " ";
        }
        line += tokens[i].item;
        p_module->lines.push_back(line);
        ++i;
        continue;
      }

      if (tokens[i].item == "initial" && tokens[i + 1].item != "begin") {
        line = tokens[i].item + " ";
        while (tokens[++i].item != ";") {
          line += tokens[i].item;
          line += " ";
        }
        line += tokens[i].item;
        p_module->lines.push_back(line);
        ++i;
        continue;
      }

      if (tokens[i].item == "assign") {
        p_line = new ML_Assign;
        p_module->typed_lines.push_back(p_line);
        while (tokens[++i].item != ";") {
          ((ML_Assign *)p_line)->expression.push_back(tokens[i].item);
        }
        ++i;
        continue;
      }

      if (tokens[i].item == "always") {
        p_line = new ML_Always;
        p_module->typed_lines.push_back(p_line);
        while (tokens[++i].item != ";") {
          ((ML_Assign *)p_line)->expression.push_back(tokens[i].item);
        }
        ++i;
        continue;
      }

      // This is a very simple and primitive way to detect RTL description
      if (tokens[i + 2].item == "(") {
        p_line = new ML_Instance;
        p_module->typed_lines.push_back(p_line);

        ((ML_Instance *)p_line)->type = tokens[i++].item;
        ((ML_Instance *)p_line)->name = tokens[i++].item;
        //++i; // (
        while (tokens[++i].item != ";") {
          ((ML_Instance *)p_line)->nodes.push_back(tokens[i++].item);
        }
        ++i;
        continue;
      }
      // This RTL comes without names, just logic operators not, or, ...
      if (tokens[i + 1].item == "(") {
        p_line = new ML_Instance;
        p_module->typed_lines.push_back(p_line);

        ((ML_Instance *)p_line)->type = tokens[i++].item;
        //++i; // (
        while (tokens[++i].item != ";") {
          ((ML_Instance *)p_line)->nodes.push_back(tokens[i++].item);
        }
        ++i;
        continue;
      }

      // This is a very simple and primitive way to detect RTL description
      if (tokens[i + 1].item == "#") {
        p_line = new ML_Instance;
        p_module->typed_lines.push_back(p_line);
        
        ((ML_Instance *)p_line)->type = tokens[i++].item;
        i++;
        ((ML_Instance *)p_line)->delay = tokens[i++].item;
        ((ML_Instance *)p_line)->name = tokens[i++].item;
        //++i; // (
        while (tokens[++i].item != ";") {
          ((ML_Instance *)p_line)->nodes.push_back(tokens[i++].item);
        }
        ++i;
        continue;
      }

      /*
      // This RTL comes without names, just logic operators not, or, ...
      if (tokens[i + 1].item == "(") {
        p_line = new ML_Instance;
        p_module->typed_lines.push_back(p_line);

        ((ML_Instance *)p_line)->type = tokens[i++].item;
        //++i; // (
        while (tokens[++i].item != ";") {
          ((ML_Instance *)p_line)->nodes.push_back(tokens[i++].item);
        }
        ++i;
        continue;
      } */
    }  // if >> endmodule
  }
  if (modules.size() == 1) {
    flatten = true;
  } else {
    flatten = false;
  }
  for (size_t i = 0; i < modules.size(); ++i) {
    if (modules[i]->module_name == root_module_name) {
      printf("__inf__ : Root module '%s' found\n", root_module_name.c_str());
      return true;
    }
  }
  printf("__err__ : Can't find specified root module '%s'\n", root_module_name.c_str());
  printf("          Available modules are:\n");
  for (size_t i = 0; i < modules.size(); ++i)
    printf("            %s\n", modules[i]->module_name.c_str());
  return false;
}

void VerilogHDL_Flattener::PostProcess() {
  bool isRTL = true;
  std::string instance_type;
  for (size_t i = 0; i < modules.size(); ++i) {
    isRTL = true;
    for (size_t j = 0; j < modules[i]->typed_lines.size(); ++j) {
      if (modules[i]->typed_lines[j]->Type() == mlt_always) {
        isRTL = false;
        break;
      }
      if (modules[i]->typed_lines[j]->Type() == mlt_assign)
        continue;
      // Last chance - Type == mlt_instance
      instance_type = ((ML_Instance *)modules[i]->typed_lines[j])->type;
      if(instance_type != "not" && instance_type != "or" && instance_type != "and" && instance_type != "nor" && instance_type != "nand" && instance_type != "xor") {
        isRTL = false;
        break;
      }
    }
    modules[i]->isRTL = isRTL;
  }
}

bool VerilogHDL_Flattener::MakeTheBiggestJobEver(std::string root_module_name) {
  size_t  i_root = std::string::npos,
          i_inst = std::string::npos;

  std::string instance_type;
  std::string delay;

  for (size_t i = 0; i < modules.size(); ++i) {
    if (modules[i]->module_name == root_module_name) {
      i_root = i;
      break;
    }
  }

  // Go through all of lines with instances
  for (size_t i = 0; i < modules[i_root]->typed_lines.size(); ++i) {
    if (modules[i_root]->typed_lines[i]->Type() != mlt_instance)
      return false;
    
    // If it's RTL module - no need to convert hiererchy, just skip the line
    delay = ((ML_Instance *)modules[i_root]->typed_lines[i])->delay;
    instance_type = ((ML_Instance *)modules[i_root]->typed_lines[i])->type;
    if(instance_type == "not" || instance_type == "or" || instance_type == "and" || instance_type == "nor" || instance_type == "nand" || instance_type == "xor")
      continue;
    
    i_inst = std::string::npos;
    for (size_t j = 0; j < modules.size(); ++j)
      if (modules[j]->module_name == ((ML_Instance *)modules[i_root]->typed_lines[i])->type) {
        i_inst = j;
        break;
      }

    // i_root - index of root module
    // i_inst - index of module to substitute
    
    if(modules[i_inst]->isRTL) {
      ML_Instance *p_inst = NULL;
      for(size_t j = 0; j < modules[i_inst]->typed_lines.size(); ++j) {
        p_inst = new ML_Instance;
        modules[i_root]->typed_lines.push_back(p_inst);
        
        p_inst->type = ((ML_Instance *)modules[i_inst]->typed_lines[j])->type;
        p_inst->delay = ((ML_Instance *)modules[i_inst]->typed_lines[j])->delay;
        p_inst->name = ((ML_Instance *)modules[i_root]->typed_lines[i])->name + std::string("_") + ((ML_Instance *)modules[i_inst]->typed_lines[j])->name;
        
        for(size_t k = 0; k < ((ML_Instance *)modules[i_inst]->typed_lines[j])->nodes.size(); ++k) {
          std::string pin_name = ((ML_Instance *)modules[i_inst]->typed_lines[j])->nodes[k];
          size_t pin_position = std::string::npos;
          for(size_t n = 0; n < modules[i_inst]->pins.size(); ++n)
            if(modules[i_inst]->pins[n] == pin_name) {
              pin_position = n;
              break;
            }
          if(pin_position == std::string::npos) {
            // Not a port pin
            p_inst->nodes.push_back(((ML_Instance *)modules[i_root]->typed_lines[i])->name + std::string("_") + pin_name);
          } else {
            // Port pin
            p_inst->nodes.push_back(((ML_Instance *)modules[i_root]->typed_lines[i])->nodes[pin_position]);
          }
        }
        
      }
      modules[i_root]->typed_lines.erase(modules[i_root]->typed_lines.begin() + i);
      --i;
    }
    else {
      printf("__inf__ : Found non RTL module: %s\n", modules[i_inst]->module_name.c_str());
      printf("__inf__ : Trying to open hierarchy for this module...");
      MakeTheBiggestJobEver(modules[i_inst]->module_name);
      --i;
      printf("done!\n");
    }

  }
  modules[i_root]->isRTL = true;
  return true;
}

#pragma warning(push)
#pragma warning(disable: 4996)
bool VerilogHDL_Flattener::Dump(std::string file_name, std::string root_module_name) {
  FILE *p_file = fopen(file_name.c_str(), "wt");
  if(!p_file)
    return false;
  
  size_t i_root = std::string::npos;
  for(size_t i = 0; i < modules.size(); ++i)
    if(modules[i]->module_name == root_module_name) {
      i_root = i;
      break;
    }
  // пишем заголовок модуля
  fprintf(p_file, "module %s(", modules[i_root]->module_name.c_str());
  // пишем имена пинов
  for(size_t i = 0; i < modules[i_root]->pins.size(); ++i) {
    fprintf(p_file, "%s",  modules[i_root]->pins[i].c_str());
    if (i < modules[i_root]->pins.size()-1)
      fprintf(p_file, ", ");
  }
  fprintf(p_file, ");\n");
  // закончили писать заголовок модуля

  for(size_t i = 0; i < modules[i_root]->lines.size(); ++i)
    fprintf(p_file, "  %s\n", modules[i_root]->lines[i].c_str());

  fprintf(p_file, "\n");

  /*
  fprintf(p_file, "input D, C;\n");
  fprintf(p_file, "output Q1, Q2;\n");

  fprintf(p_file, "\n");
  //*/

  ML_Instance *p_inst;
  for(size_t i = 0; i < modules[i_root]->typed_lines.size(); ++i) {
    p_inst = (ML_Instance *)modules[i_root]->typed_lines[i];
    if(p_inst->name.empty())
      if(p_inst->delay.empty())
        fprintf(p_file, "  %s(", p_inst->type.c_str());
      else
        fprintf(p_file, "  %s #%s(", p_inst->type.c_str(), p_inst->delay.c_str());
    else
      if (p_inst->delay.empty())
        fprintf(p_file, "  %s %s(", p_inst->type.c_str(), p_inst->name.c_str());
      else
        fprintf(p_file, "  %s #%s %s(", p_inst->type.c_str(), p_inst->delay.c_str(), p_inst->name.c_str());
    
    for(size_t j = 0; j < p_inst->nodes.size(); ++j) {
      if(!j)
        fprintf(p_file, "%s", p_inst->nodes[j].c_str());
      else
        fprintf(p_file, ", %s", p_inst->nodes[j].c_str());
    }
    fprintf(p_file, ");\n");
    
  }

  
  fprintf(p_file, "endmodule\n");
  
  fclose(p_file);
  was_written = true;
  return true;
}
#pragma warning(pop)
