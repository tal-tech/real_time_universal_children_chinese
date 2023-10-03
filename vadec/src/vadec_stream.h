#ifndef TLV_KALDI_VADEC_STREAM_H_
#define TLV_KALDI_VADEC_STREAM_H_

#include <common_audio/vad/include/webrtc_vad.h>
#include "tlv/kaldiext/vadparam/vadparamdefine.h"
#include <boost/thread.hpp>

#include <deque>

// 默认是16KHz, 单通道, unsigned int, wav数据

namespace Vadec
{

constexpr unsigned int SAMPLE_RATE = 16000;
constexpr unsigned int PER_MS_SIZE = SAMPLE_RATE / 1000 * 2; // 每毫秒数据字节数
constexpr unsigned int FRAME_SIZE = 10 * PER_MS_SIZE; // 每一帧字节数， 一帧10ms
constexpr unsigned int FRAME_SIZE_IN_INT16 = FRAME_SIZE / 2; // 每一帧int16数
constexpr unsigned int FEED_FRAMES = 10;

constexpr unsigned int TIME_PER_FRAME = FRAME_SIZE / PER_MS_SIZE; // in millisecond
constexpr int WEBRTC_VAD_MODE = 3;
constexpr int WEBRTC_VAD_MODE_EX = 4;//self train params,use this mode
constexpr unsigned int DEFAULT_MAX_SPEECH_LEN = 1500;
constexpr unsigned int DEFAULT_MIN_SPEECH_AS_START = 10;
constexpr unsigned int DEFAULT_MAX_SIL_AS_END = 40;
constexpr float VAD_MAX_SENTENCE_SECOND = 15.0f;
}

struct VadecFrame {
  char data[Vadec::FRAME_SIZE];
  unsigned int len {0};
  bool last_frame {false};
  
  int index {0};
};

struct VadecDataInfo {
  bool period_end {false};
  bool class_end {false};
  bool all_end {false};
  unsigned int begin_frame;
  unsigned int end_frame;
};

class VadecStream {
 public:
  enum SpeechState {
    speech_zero = 0,
    pre_speech_start,
    speech_start,
    pre_speech_end,
    speech_end
  };
  VadecStream();
  ~VadecStream();

  int Init(VadInterface::VadParams* params);

  void Uninit();

  void Reset();

  void SetByPass(bool by_pass){by_pass_ = by_pass;}
  void SetSentenceMaxSecond(float second);
  void AppendStream(char* data, const unsigned int len, bool class_end, bool all_end);
  int GetFeedingData(char* data, unsigned int& len, VadecDataInfo& info);

  void ForceWakeup();

 private:
  int InitWithParams(const VadInterface::VadParams& params, int& agressive_mode);

  void PopFeedingData(std::deque<VadecFrame>::iterator end_pos, char* data,
                      unsigned int& len);
  void UpdateState(bool is_active);

  int DetectSingleFrame(char* data);

  void ClearStreamTail();

 private:
  std::deque<VadecFrame> stream_;

  VadInst* vad_;
  bool all_end_;

  int frame_count_;
  std::deque<VadecFrame>::iterator current_;

  SpeechState state_;
  unsigned int sil_count_;
  unsigned int speech_count_;

  unsigned int begin_frame_;

  boost::mutex mu_;
  boost::condition_variable cv_;

  VadInterface::VadParams* params_;
  bool by_pass_;
  float sentence_max_frame_;
};

#endif  // TLV_KALDI_VADEC_STREAM_H_
