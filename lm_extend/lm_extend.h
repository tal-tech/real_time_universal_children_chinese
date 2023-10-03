#ifndef TLV_SCRIPT_RUNNER_
#define TLV_SCRIPT_RUNNER_

#include <string>
#include <vector>
#include <map>

namespace tlv {

// std::wstring StringToWstring(const std::string& str);
// std::string WstringToString(const std::wstring& wide);

int GenerateNewRes(const std::vector<std::string>& name_list,
    const std::string& orig_res_path, std::string& new_res_path);

class LmExtender {
public:
  LmExtender(const std::string& orig_res_path);
  ~LmExtender();

  int GetNewRes(std::vector<std::string>& name_list, std::string res_path);

private:
  std::vector<std::string> QueryWord(const std::string& word);
  std::vector<std::string> QueryName(const std::string& name);
  void ExtendNameList(std::vector<std::string>& name_list);
  void GenerateNameDict(const std::vector<std::string>& name_list, const std::string& new_dict_fn);

  const std::string orig_res_path_;
  std::vector<std::string> dict_;
  std::map<std::string, std::vector<std::string>> dict_cache_;
};

}

#endif // TLV_SCRIPT_RUNNER_
