#include "vadec_stream.h"

using namespace Vadec;
VadecStream::VadecStream()
    : stream_(),
      vad_(nullptr),
      all_end_(false),
      frame_count_(0),
      state_(speech_zero),
      sil_count_(0),
      speech_count_(0),
      begin_frame_(-1),
      params_(NULL),
      by_pass_(false),
      sentence_max_frame_(VAD_MAX_SENTENCE_SECOND * 1000 / TIME_PER_FRAME) {} // 默认15s最长句子

VadecStream::~VadecStream() {}

int VadecStream::Init(VadInterface::VadParams* params) {
  current_ = stream_.begin();
  vad_ = WebRtcVad_Create();

  if(params_ == NULL)
  {
      params_ = new VadInterface::VadParams();
  }
  if(params != NULL)
  {
      (*params_) = (*params);
  }

  int agressive_mode = WEBRTC_VAD_MODE;
  int rc = InitWithParams(*params_, agressive_mode);

  if (rc != 0) {
    WebRtcVad_Free(vad_);
    vad_ = nullptr;
    return -1;
  }
  else {
    rc = WebRtcVad_set_mode(vad_, agressive_mode);
    if (rc != 0) {
      WebRtcVad_Free(vad_);
      vad_ = nullptr;
      return -1;
    }
    else {
      return 0;
    }
  }
}

void VadecStream::ForceWakeup() {
  boost::lock_guard<boost::mutex> lg(mu_);
  cv_.notify_all();
}

void VadecStream::Uninit() {
  if (vad_) {
    WebRtcVad_Free(vad_);
    vad_ = nullptr;
  }
  else {
    // TODO(Jack): add log
  }
  delete params_;
  params_ = NULL;
}

void VadecStream::Reset() {
  boost::lock_guard<boost::mutex> log(mu_);
  stream_.clear();
  state_ = speech_zero;
  sil_count_ = 0;
  speech_count_ = 0;
  frame_count_ = 0;
  begin_frame_ = -1;
}

void VadecStream::AppendStream(char* data, const unsigned int len, bool class_end,
bool all_end) {
  boost::lock_guard<boost::mutex> lg(mu_);
  if (all_end_) {
    return;
  }
  auto current_offset = std::distance(stream_.begin(), current_);
  all_end_ = all_end;
  char* remain_data = data;
  unsigned int remain_len = len;
  if (!stream_.empty()) {
    auto& last_vf = stream_.back();
    if (last_vf.len != FRAME_SIZE) {
      unsigned int lack_len = FRAME_SIZE - last_vf.len;
      if (len > lack_len) {
        memcpy(last_vf.data + last_vf.len, data, lack_len);
        remain_data = data + lack_len;
        remain_len = len - lack_len;
        last_vf.len = FRAME_SIZE;
      }
      else {
        memcpy(last_vf.data + last_vf.len, data, len);
        last_vf.len += len;
        last_vf.last_frame = class_end;
        if (all_end) {
          cv_.notify_all();
        }
        return;
      }
    }
    else {
      if (class_end) {
        // std::cout << "[Jack Debug]last frame data len: " << last_vf.len << ", all_end: " << all_end << std::endl;
      }
    }
  }
  else {
    if (remain_len == 0) {
      // std::cout << "[Jack Debug]stream is empty, class_end:" << class_end << ", all_end: " << all_end << std::endl;
    }
  }
  
  unsigned int i = 0;
  while (i < remain_len) {
    VadecFrame vf;
    unsigned int size = std::min(FRAME_SIZE, remain_len - i);
    memcpy(vf.data, remain_data + i, size);
    vf.len = size;
    vf.index = frame_count_;
    frame_count_++;
    i += size;
    if (i >= remain_len) {
      vf.last_frame = class_end;
    }
    stream_.push_back(vf);
  }
  current_ = std::next(stream_.begin(), current_offset);
  cv_.notify_all();
}

int VadecStream::InitWithParams(const VadInterface::VadParams& params, int& agressive_mode)
{
    bool use_default = false;
    int kMode = params.kMode;
    if(params.kTableSize != 12)
    {
        use_default = true;
        kMode = 3;
    }

    //default params
    int16_t noise_data_means[12] = {6738, 4892, 7065, 6715, 6771, 3369, 7646, 3863, 7820, 7266, 5020, 4362};
    int16_t speech_data_means[12] = {8306, 10085, 10078, 11823, 11843, 6309, 9473, 9571, 10879, 7581, 8180, 7483};
    int16_t noise_data_stds[12] = {378, 1064, 493, 582, 688, 593, 474, 697, 475, 688, 421, 455};
    int16_t speech_data_stds[12] = {555, 505, 567, 524, 585, 1231, 509, 828, 492, 1540, 1079, 850};

    if(params.kNoiseDataMeans.size()!=12 ||
            params.kSpeechDataMeans.size()!=12 ||
            params.kNoiseDataStds.size()!=12 ||
            params.kSpeechDataStds.size()!=12)
    {
        use_default = true;
        kMode = 3;
    }

    if(!use_default)
    {
        for(size_t i=0; i<params.kTableSize; ++i)
        {
            noise_data_means[i] = params.kNoiseDataMeans[i];
            speech_data_means[i] = params.kSpeechDataMeans[i];
            noise_data_stds[i] = params.kNoiseDataStds[i];
            speech_data_stds[i] = params.kSpeechDataStds[i];
        }
    }

    int rc = WebRtcVad_Init(vad_,
                             noise_data_means,
                             speech_data_means,
                             noise_data_stds,
                             speech_data_stds);
    agressive_mode = kMode;
    return rc;
}

void VadecStream::PopFeedingData(std::deque<VadecFrame>::iterator end_pos,
                                 char* data, unsigned int& len) {
  std::for_each(stream_.begin(), end_pos, [data, &len](const VadecFrame& vf) {
    memcpy(data + len, vf.data, vf.len);
    len += vf.len;
  });
  current_ = stream_.erase(stream_.begin(), end_pos);
}

int VadecStream::DetectSingleFrame(char* data) {
//    int sample_rate = (params_==NULL)?SAMPLE_RATE:params_->sampleRate;//TODO-改变采样率涉及几乎所有宏，暂时不调整
//    int frame_size = (params_==NULL)?FRAME_SIZE_IN_INT16:params_->frameSize;
    return WebRtcVad_Process(
      vad_, SAMPLE_RATE, reinterpret_cast<int16_t*>(data),
      FRAME_SIZE_IN_INT16);
}

int VadecStream::GetFeedingData(char* data, unsigned int& len, VadecDataInfo& info) {
  boost::unique_lock<boost::mutex> ul(mu_);
  len = 0;
  info.period_end = false;
  info.class_end = false;
  info.all_end = false;
  if (stream_.empty()) {
    if (!all_end_) {
      cv_.wait_for(ul, boost::chrono::milliseconds(100));
    }
    else {
      info.end_frame = frame_count_;
      info.all_end = true;
      info.class_end = true;
    }
    return stream_.size();
  }
  else {
    while (current_ != stream_.end()) {
      int is_active = false;
      if (current_->last_frame) {
        if (current_ + 1 == stream_.end()) {
          info.all_end = all_end_;
        }
        info.begin_frame = begin_frame_;
        info.end_frame = current_->index;
        if (state_ == speech_start || state_ == pre_speech_end) {
          PopFeedingData(current_ + 1, data, len);
          info.period_end = true;
        }
        else {
          current_ = stream_.erase(stream_.begin(), current_ + 1);
          info.period_end = false;
        }
        info.class_end = true;
        state_ = speech_zero;
        sil_count_ = 0;
        speech_count_ = 0;
        frame_count_ = 0;
        begin_frame_ = -1;
        return stream_.size();
      }
      if (current_->len != FRAME_SIZE) {
        if (!all_end_) {
          cv_.wait_for(ul, boost::chrono::milliseconds(100));
        }
        return 0;
      }

      //TODO-改变采样率涉及几乎所有宏，暂时不调整
//      int sample_rate = (params_==NULL)?SAMPLE_RATE:params_->sampleRate;
//      int frame_size = (params_==NULL)?FRAME_SIZE_IN_INT16:params_->frameSize;
      is_active = WebRtcVad_Process(
          vad_, SAMPLE_RATE, reinterpret_cast<int16_t*>(current_->data),
          FRAME_SIZE_IN_INT16);

      if(by_pass_)
      {
          is_active = true;
      }

      // std::cout << is_active << std::flush;
      SpeechState previous_ss = state_;
      UpdateState(is_active);
      switch (state_) {
        case speech_zero:
          stream_.pop_front();
          current_ = stream_.begin();
          break;
        case pre_speech_start:
          current_++;
          break;
        case speech_start:
          if (begin_frame_ == -1) {
            begin_frame_ = stream_.begin()->index;
          }
          if (previous_ss == pre_speech_start ||
              previous_ss == pre_speech_end) {
            info.begin_frame = begin_frame_;
            info.end_frame = current_->index;
            PopFeedingData(current_ + 1, data, len);
            return stream_.size();
          }
          else {
            if (current_ - stream_.begin() + 1 >= FEED_FRAMES) {
              info.begin_frame = begin_frame_;
              info.end_frame = current_->index;
              PopFeedingData(current_ + 1, data, len);
              return stream_.size();
            }
            else {
              current_++;
            }
          }
          break;
        case pre_speech_end:
          if (previous_ss == speech_start && (current_ - stream_.begin()) > 0) {
            info.begin_frame = begin_frame_;
            info.end_frame = current_->index;
            PopFeedingData(current_, data, len);
            current_++;
            return stream_.size();
          }
          else {
            current_++;
          }
          break;
        case speech_end:
          int sil_count = sil_count_;
          UpdateState(false); // reset speech state
          if (previous_ss == speech_start) {
            info.begin_frame = begin_frame_;
            info.end_frame = current_->index;
            info.period_end = true;
            PopFeedingData(current_ + 1, data, len);
            // ClearStreamTail();
            begin_frame_ = -1;
            return stream_.size();
          }
          if (previous_ss == pre_speech_end) {
            info.begin_frame = begin_frame_;
            info.period_end = true;
            if (sil_count == DEFAULT_MAX_SIL_AS_END) {
              info.end_frame = current_->index - DEFAULT_MAX_SIL_AS_END;
              current_ = stream_.erase(stream_.begin(), current_ + 1);
            }
            else {
              info.end_frame = current_->index;
              PopFeedingData(current_ + 1, data, len);
            }
            // ClearStreamTail();
            begin_frame_ = -1;
            return stream_.size();
          }
          break;
      }
    }
    if (current_ == stream_.end() && all_end_) {
      info.period_end = true;
      info.class_end = true;
      info.all_end = true;
      return 0;
    }
    else {
      cv_.wait_for(ul, boost::chrono::milliseconds(100));
      return stream_.size();
    }
  }
}

void VadecStream::ClearStreamTail() {
  if (stream_.back().last_frame) {
    if (stream_.end() - current_ <= FEED_FRAMES) {
      stream_.erase(current_, stream_.end());
    }
  }
}

void VadecStream::UpdateState(bool is_active) {
  switch(state_) {
    case speech_zero:
      if (is_active) {
        speech_count_ = 1;
        state_ = pre_speech_start;
      }
      else {
        state_ = speech_zero;
      }
      break;
    case pre_speech_start:
      if (is_active) {
        speech_count_++;
        if (speech_count_ >= DEFAULT_MIN_SPEECH_AS_START) {
          state_ = speech_start;
        }
        else {
          state_ = pre_speech_start;
        }
      }
      else {
        speech_count_ = 0;
        sil_count_ = 1;
        state_ = speech_zero;
      }
      break;
    case speech_start:
      if (is_active) {
        speech_count_++;
        if (speech_count_>= sentence_max_frame_&& !by_pass_) {
          state_ = speech_end;
        }
        else {
          state_ = speech_start;
        }
      }
      else {
        sil_count_ = 1;
        state_ = pre_speech_end;
      }
      break;
    case pre_speech_end:
      if (is_active) {
        speech_count_++;
        speech_count_ += sil_count_;
        sil_count_ = 0;
        if (speech_count_ >= sentence_max_frame_ && !by_pass_) {
          state_ = speech_end;
        }
        else {
          state_ = speech_start;
        }
      }
      else {
        sil_count_++;
        if (sil_count_ >= DEFAULT_MAX_SIL_AS_END) {
          state_ = speech_end;
        }
        else {
          state_ = pre_speech_end;
        }
      }
      break;
    case speech_end:
      sil_count_ = 0;
      speech_count_ = 0;
      state_ = speech_zero;
      break;
  }
}
void VadecStream::SetSentenceMaxSecond(float second) {
  if(second <= 0.0f) {
    return;
  }
  sentence_max_frame_ =  second * 1000 / TIME_PER_FRAME;
}
