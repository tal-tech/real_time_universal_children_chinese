#include "lm_extend.h"

#include <chrono>
#include <iostream>
#include <sstream>

#include "rapidjson/writer.h"
#include "rapidjson/filereadstream.h"
#include "boost/filesystem.hpp"

namespace tlv
{

namespace base
{

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

template<typename From, typename To, typename Input, typename Output>
bool trans(const Input& input, Output& output) {
  rapidjson::GenericStringStream<From> source(input.c_str());
  rapidjson::GenericStringBuffer<To> target;
  bool hasError = false;
  while(source.Peek() != 0) {
    if (!rapidjson::Transcoder<From, To>::Transcode(source, target)) {
      hasError = true;
      break;
    }
  }

  if (!hasError) {
    auto re = target.GetString();
    output = re;
    return true;
  }
  else {
    return false;
  }
}

}

std::wstring StringToWstring(const std::string& str) {
  std::wstring wide;
  using namespace rapidjson;
  base::trans<UTF8<>, UTF16<>>(str, wide);
  return wide;
}

std::string WstringToString(const std::wstring& wide) {
  std::string str;
  using namespace rapidjson;
  base::trans<UTF16<>, UTF8<>>(wide, str);
  return str;
}

LmExtender::LmExtender(const std::string& orig_res_path): orig_res_path_(orig_res_path) {
  boost::filesystem::path dict_path(orig_res_path);
  dict_path /= std::string("mini_dict.txt");

  std::ifstream dict(dict_path.string());
  if (dict.is_open()) {
    std::string line;
    while(std::getline(dict, line)) {
      dict_.push_back(line);
    }
  }
}

LmExtender::~LmExtender() {}

std::vector<std::string> LmExtender::QueryWord(const std::string& word) {
  if (dict_cache_[word].size() != 0) {
    return dict_cache_[word];
  }
  int s = 0;
  int e = dict_.size() - 1;
  while(s <= e) {
    int m = (s + e) / 2;
    auto key = base::StringSplit<std::vector<std::string>>(dict_[m])[0];
    if (word > key) {
      s = m + 1;
    }
    else if (word < key) {
      e = m - 1;
    }
    else {
      std::vector<std::string> prons;
      std::vector<int> prons_index = {m};
      int range = 1;
      for(;; range++) {
        auto prev_key = base::StringSplit<std::vector<std::string>>(dict_[m - range])[0];
        auto next_key = base::StringSplit<std::vector<std::string>>(dict_[m + range])[0];
        if (prev_key != word && next_key != word) {
          break;
        }
        else {
          if (prev_key == word) {
            prons_index.push_back(m - range);
          }
          if (next_key == word) {
            prons_index.push_back(m + range);
          }
        }
      }
      for(auto pi : prons_index) {
        auto item = dict_[pi];
        auto pos = item.find(" ");
        auto pron = item.substr(pos, item.size() - pos);
        prons.push_back(pron);
        dict_cache_[word].push_back(pron);
      }
      return prons;
    }
  }
  return {};
}

std::vector<std::string> LmExtender::QueryName(const std::string& name) {
  std::wstring wn = StringToWstring(name);
  std::vector<std::string> name_prons;
  for(int i = 0; i < wn.size(); i++) {
    auto word = WstringToString(wn.substr(i, 1));
    auto word_prons = QueryWord(word);
    if (i == 0) {
      for(auto wp : word_prons) {
        name_prons.push_back(wp);
      }
    }
    else {
      auto len = name_prons.size();
      for(auto i = 0; i < len; i++) {
        std::string tmp = name_prons[i];
        for(auto j = 0; j < word_prons.size(); j++) {
          if(j == 0) {
            name_prons[i] += word_prons[j];
          }
          else {
            name_prons.push_back(tmp + word_prons[j]);
          }
        }
      }
    }
  }

  return name_prons;
}

void LmExtender::GenerateNameDict(const std::vector<std::string>& name_list,
    const std::string& new_dict_fn) {
  std::ofstream ofs(new_dict_fn);
  for (auto name : name_list) {
    auto prons = QueryName(name);
    double probability = 1.0 / prons.size();
    for (auto p : prons) {
      ofs << name << " " << probability << p << std::endl;
    }
  }
}

void LmExtender::ExtendNameList(std::vector<std::string>& name_list) {
  auto nl_size = name_list.size();
  for(int i = 0; i < nl_size; i++) {
    auto name_w = StringToWstring(name_list[i]);
    if(name_w.size() > 2) {
      auto first_name = name_w.substr(name_w.size() - 2, 2);
      name_list.push_back(WstringToString(first_name));
    }
  }
}

int LmExtender::GetNewRes(std::vector<std::string>& name_list, std::string res_path) {
  using namespace std::chrono;

  ExtendNameList(name_list);

  boost::filesystem::path cmd(orig_res_path_); 
  cmd /= std::string("scripts");
  cmd /= std::string("run.sh");

  boost::filesystem::path output_path(res_path);

  boost::filesystem::create_directory(res_path);
  auto name_dict_fn = output_path / std::string("name_dict.txt");
  GenerateNameDict(name_list, name_dict_fn.string());

  auto fullcmd = "timeout 300 " + cmd.string() + " " + output_path.string();
  auto rc = std::system(fullcmd.c_str());
  return rc;
}

}
