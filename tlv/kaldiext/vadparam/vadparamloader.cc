#include "vadparamloader.h"

#include "third/json/cJSON.h"

#include <iostream>
#include <fstream>
#include <sstream>

namespace VadInterface {


VadParamLoader::VadParamLoader() {}

VadParamLoader::~VadParamLoader() {}

bool VadParamLoader::loadVadParams(const std::string& file_path, VadParams& params_out)
{
    std::string json_str = readFileIntoString(file_path);
    if(json_str.empty())
    {
        return false;
    }

    cJSON* cjson = cJSON_Parse(json_str.c_str());
    if(!cjson)
    {
        return false;
    }

    cJSON* aggressivenessModeObj = cJSON_GetObjectItem(cjson,"aggressivenessmode");
    if(aggressivenessModeObj)
    {
        params_out.kMode = aggressivenessModeObj->valueint;
    }

    //noise means
    cJSON* noiseMeans = cJSON_GetObjectItem(cjson,"noisemeans");
    if(noiseMeans && cJSON_GetArraySize(noiseMeans)==params_out.kTableSize)
    {
        for(int i=0; i<params_out.kTableSize; ++i)
        {
            cJSON* itemobj = cJSON_GetArrayItem(noiseMeans,i);
            params_out.kNoiseDataMeans.push_back(itemobj->valueint);
        }
    }

    //speech means
    cJSON* speechMeans = cJSON_GetObjectItem(cjson,"speechmeans");
    if(speechMeans && cJSON_GetArraySize(speechMeans)==params_out.kTableSize)
    {
        for(int i=0; i<params_out.kTableSize; ++i)
        {
            cJSON* itemobj = cJSON_GetArrayItem(speechMeans,i);
            params_out.kSpeechDataMeans.push_back(itemobj->valueint);
        }
    }

    //noise stds
    cJSON* noiseStds = cJSON_GetObjectItem(cjson,"noisestds");
    if(noiseStds && cJSON_GetArraySize(noiseStds)==params_out.kTableSize)
    {
        for(int i=0; i<params_out.kTableSize; ++i)
        {
            cJSON* itemobj = cJSON_GetArrayItem(noiseStds,i);
            params_out.kNoiseDataStds.push_back(itemobj->valueint);
        }
    }

    //speech stds
    cJSON* speechStds = cJSON_GetObjectItem(cjson,"speechstds");
    if(speechStds && cJSON_GetArraySize(speechStds)==params_out.kTableSize)
    {
        for(int i=0; i<params_out.kTableSize; ++i)
        {
            cJSON* itemobj = cJSON_GetArrayItem(speechStds,i);
            params_out.kSpeechDataStds.push_back(itemobj->valueint);
        }
    }

    cJSON* samepleRateObj = cJSON_GetObjectItem(cjson,"sample_rate");
    if(samepleRateObj)
    {
        params_out.sampleRate = samepleRateObj->valueint;
        params_out.frameSize = params_out.sampleRate/100;//default
    }

    cJSON_Delete(cjson);

    return true;
}

std::string VadParamLoader::readFileIntoString(const std::string& file_path)
{
    std::ifstream ifile(file_path.c_str());
    if(!ifile.is_open())
    {
        return "";
    }

    std::ostringstream buf;
    char ch;
    ifile >> std::noskipws;
    while(buf && ifile.get(ch))
    {
        buf.put(ch);
    }

    return buf.str();
}

}//namespace

