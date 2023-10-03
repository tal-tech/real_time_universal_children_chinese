#include "lm_extend/lm_extend.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

template<class Container>
Container StringSplit(const std::string& str, char delim = ' ')
{
  Container container;
  std::stringstream ss(str);
  std::string token;
  while(std::getline(ss, token, delim)) {    
     if (token != "") {    
     container.push_back(token);    
    }    
  }    
  return container;
}

int main(int argc, char* argv[]) {
  if(argc != 4) {
    printf("Usage:%s data_dir hotwords_txt output_dir\n", argv[0]);
    return 0;
  }
  using namespace tlv;

  std::string names_fn(argv[2]);
  std::ifstream names_ifs(names_fn);
  std::string line;
  std::vector<std::string> names;
  while(std::getline(names_ifs, line)) {
    names.push_back(line);
  }

  if(names.size() == 0) return 0;

  std::string orig_res_path(argv[1]);
  LmExtender lme(orig_res_path);
  std::string out_dir(argv[3]);
  int rc = lme.GetNewRes(names, out_dir);
  if(rc == 0) {
    std::cout << "Resource has generated successfully" << std::endl;
  } else {
    std::cout << "Resource has generated failed" << std::endl;
  }
  return 0;
}
