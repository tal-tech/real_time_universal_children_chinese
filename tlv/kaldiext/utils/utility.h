#ifndef UTILITY_WRAPPER_H_
#define UTILITY_WRAPPER_H_

#include <string>
#include <set>

namespace utils {

std::string GetWavFileName(const std::string& file_path);

void ParseExcludeHotWords(const std::string& file_path, std::set<std::string>& words_out);

} //namespace utils


#endif  // UTILITY_WRAPPER_H_
