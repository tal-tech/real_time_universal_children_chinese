#include "utility.h"

#include "tlv/kaldiext/utils/tokenizer/tokenizer.h"

#include <fstream>
#include <vector>
#include <algorithm>

namespace utils {

std::string GetWavFileName(const std::string& file_path)
{
    std::string file_path_cpy = file_path;
    size_t pos = file_path_cpy.rfind('/');
    if(pos != std::string::npos)
    {
        file_path_cpy = file_path_cpy.substr(pos+1,std::string::npos);
    }
    pos = file_path_cpy.rfind('.');
    if(pos != std::string::npos)
    {
        file_path_cpy = file_path_cpy.substr(0,pos);
    }
    return file_path_cpy;
}

void ParseExcludeHotWords(const std::string& file_path, std::set<std::string>& words_out)
{
    //parse file
    std::ifstream file_stream(file_path);
    std::string one_line;
    if(!file_stream.is_open())
    {
        return;
    }

    words_out.clear();
    Tokenizer str_tokenizer;
    while(std::getline(file_stream, one_line))
    {
        str_tokenizer.set(one_line);
        std::vector<std::string> list_words = str_tokenizer.split();
        if(list_words.size() > 0)
        {
            std::string exclude_word = list_words[0];
            std::transform(exclude_word.begin(),exclude_word.end(),exclude_word.begin(),::toupper);
            words_out.insert(exclude_word);
        }
    }
}

}//namespace utils
