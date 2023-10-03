#ifndef TLV_VAD_PARAM_DEFINE_H_
#define TLV_VAD_PARAM_DEFINE_H_

#include <vector>
#include <string>

namespace VadInterface {

struct VadParams
{
    int kMode;//Aggressiveness degree : "default" use 3, self optimization params("General") use 4
    int kTableSize;//NumChannels*NumGuaussians=6*2 fix 12
    std::vector<int> kNoiseDataMeans;
    std::vector<int> kSpeechDataMeans;
    std::vector<int> kNoiseDataStds;
    std::vector<int> kSpeechDataStds;
    int sampleRate;//采样率，默认16000
    int frameSize;//帧长：一帧对应的数据数，目前只支持10ms帧长(支持20ms需要修改MULTI宏定义)，10毫秒帧长下为(sampleRate/100)

    VadParams() : kMode(3),kTableSize(12),sampleRate(16000),frameSize(160)
    {}

    VadParams& operator=(const VadParams& cls)
    {
        if (this != &cls)
        {
            this->kMode = cls.kMode;
            this->kTableSize = cls.kTableSize;
            this->kNoiseDataMeans = cls.kNoiseDataMeans;
            this->kSpeechDataMeans = cls.kSpeechDataMeans;
            this->kNoiseDataStds = cls.kNoiseDataStds;
            this->kSpeechDataStds = cls.kSpeechDataStds;
            this->sampleRate = cls.sampleRate;
            this->frameSize = cls.frameSize;
        }
        return *this;
    }
};

struct Period
{
    int16_t* start_point; // int16_t*为每段的起始地址
    int64_t  data_size;   // 数据段长度,字节
    int64_t  dur_time;    // int64_t为每段音频的长度，单位为毫秒
    double   begin_time;  //片段的开始时间，毫秒
    double   end_time;    //片段的结束时间，毫秒

    Period() : start_point(0),data_size(0),dur_time(0),begin_time(0),end_time(0)
    {}
};

}//namespace

#endif  // TLV_VAD_PARAM_DEFINE_H_
