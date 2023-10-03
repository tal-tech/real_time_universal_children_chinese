#include "tlv_kaldi_vadec.h"

#include "tlv/kaldiext/struct/tlv_kaldi_dec_extra_params.h"
#include "tlv_kaldi_vadec_define.h"
#include "tlv/struct/tlv_sys.h"
#include "logging.h"

#include <chrono>

INITIALIZE_EASYLOGGINGPP

//#define OUT_PUT_SPEED_LOG

////log-test
//double g_totoal_use_time = 0;

using namespace std::chrono;

TlvKaldiVadec::TlvKaldiVadec(const tlv_kaldi_dec_cfg_t* cfg)
    : cfg_(cfg), app_(nullptr), userdata_(), vstream_(), snt_index_(1), force_stop_(false)
    , need_feed_end_(false), s_start_(0), asr_start_timestamp_(0), extra_params_(NULL) {}

TlvKaldiVadec::~TlvKaldiVadec() {}

int TlvKaldiVadec::Init(tlv_kaldi_vadec_callback_t cb, const std::string userdata) {
    base::LoggerInitOnce();
    userdata_ = userdata;
    const_cast<tlv_kaldi_dec_cfg_t*>(cfg_)->is_online = 1;
    int rc = vstream_.Init(cfg_->vad_params);
    if (rc != 0) {
        return rc;
    }
    if (cb) {
        cb_ = [cb](const char* json, unsigned int len, bool all_end, const std::string userdata) { cb(json, len, all_end, userdata); };
        app_ = tlv_kaldi_dec_new(cfg_);
        if (app_) {
            thread_.reset(new boost::thread([this] {
                char* data = (char*)malloc(Vadec::FRAME_SIZE * Vadec::DEFAULT_MAX_SIL_AS_END);
                while(!force_stop_) {

                    memset(data, 0, Vadec::FRAME_SIZE * Vadec::DEFAULT_MAX_SIL_AS_END);
                    unsigned int len = 0;
                    VadecDataInfo info;
                    int remain = vstream_.GetFeedingData(data, len, info);
                    if (force_stop_) {
                        break;
                    }
                    if (len > 0 ||
                            info.period_end ||
                            info.class_end ||
                            (remain == 0 && info.all_end)) {
                        Decode(data, len, info);
                    }
                    if (remain == 0 && info.all_end) {
                        break;
                    }
                }
                free(data);
            }));
            return 0;
        }
        else {
            return -1;
        }
    }
    else {
        return -1;
    }
}

void TlvKaldiVadec::Uninit() {
    force_stop_ = true;
    vstream_.ForceWakeup();
#if defined (VADEC_ALLOW_UNINIT_ON_DIFFERENT_THREAD)
    thread_->detach();
#else 
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
#endif
    thread_.reset();
    vstream_.Uninit();
    if (app_) {
        if (need_feed_end_) {
            tlv_kaldi_dec_feed(app_, 0, 0, 1);
        }
        tlv_kaldi_dec_delete(app_);
        app_ = nullptr;
    }

    {
        boost::lock_guard<boost::mutex> lg(extra_param_mu_);
        delete extra_params_;
        extra_params_ = NULL;
        (void)lg;
    }
}

void TlvKaldiVadec::SetExtralParams(const char* params, int len)
{
    bool use_vad = true;
    {
        boost::lock_guard<boost::mutex> lg(extra_param_mu_);
        if(extra_params_ == NULL)
        {
            extra_params_ = new tlv_extra_param_t(cfg_);
        }
        tlv_extra_params_reset(extra_params_);
        if(params!=NULL && len>0)
        {
            std::string paramstmp(params,len);
            if(!paramstmp.empty())
            {
                tlv_parse_extra_params(paramstmp, extra_params_);
            }
        }
        use_vad = extra_params_->use_vad;
        (void)lg;
    }

    vstream_.SetByPass(!use_vad);
    if(extra_params_->vad_max_sentence_second > 0.0f && extra_params_->use_vad) {
        vstream_.SetSentenceMaxSecond(extra_params_->vad_max_sentence_second);
    }
}

int TlvKaldiVadec::Process(char* data, unsigned int len,
                           bool class_end, bool all_end) {
    if (s_start_ == 0) {
        s_start_ = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }
    vstream_.AppendStream(data, len, class_end, all_end);
    return 0;
}

int TlvKaldiVadec::Decode(char* data, unsigned int len, const VadecDataInfo& info) {
    char* dec_result;
    int dec_len;
    int dec_rc;
    KaldiVadecFinalResult vadec_res;
    vadec_res.is_end = info.class_end;
    vadec_res.duration = (info.end_frame + 1) * Vadec::TIME_PER_FRAME;
    vadec_res.code = 0;
    vadec_res.msg = "decode success.";
    vadec_res.index = snt_index_;
    vadec_res.the_very_first_ts = s_start_;
    if(asr_start_timestamp_ == 0) {
      asr_start_timestamp_ = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }
    vadec_res.first_asr_timestamp = asr_start_timestamp_;
    std::string tmp_version(cfg_->version);
    auto start_v = tmp_version.rfind("V");
    vadec_res.model_version = tmp_version.substr(start_v + 1);
    vadec_res.model = tmp_version.substr(0, start_v - 1);
    vadec_res.model_name = tmp_version;
    if ((len == 0 && !info.period_end && info.class_end) ||
            (len == 0 && info.all_end)) {
      vadec_res.result = "";
      vadec_res.is_partial = false;
      if (info.all_end) {
        vadec_res.is_end = info.all_end;
      }
      if (extra_params_ != NULL) {
        vadec_res.request_params.use_vad = extra_params_->use_vad;
        vadec_res.request_params.vad_max_sentence_second = extra_params_->vad_max_sentence_second;
        vadec_res.request_params.output_end_silence = extra_params_->end_silence;
      }
      std::string vadec_res_str = staticjson::to_json_string(vadec_res);
      cb_(vadec_res_str.c_str(), vadec_res_str.length(), info.all_end, userdata_);
      return 0;
    }

    milliseconds before_feed = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    dec_rc = tlv_kaldi_dec_feed(app_, data, len, info.period_end);
    need_feed_end_ = !info.period_end;
    milliseconds s_end = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    vadec_res.time_delay = s_end.count() - s_start_;
    // LOG(INFO) << "time delay: " << vadec_res.time_delay;
    if (dec_rc == 0) {
        dec_rc = tlv_kaldi_dec_get_rslt(app_, &dec_result, &dec_len);
        milliseconds after_feed = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        std::string result(dec_result, dec_len);

        free(dec_result);
        staticjson::ParseStatus ps;
        KaldiDecMidResult mid_res;
        if (staticjson::from_json_string(result.c_str(), &mid_res, &ps)) {
            vadec_res.begin_time = info.begin_frame * Vadec::TIME_PER_FRAME;
            vadec_res.end_time = (info.end_frame + 1) * Vadec::TIME_PER_FRAME;
            vadec_res.result = mid_res.result.text;
            vadec_res.time_delay_inner = after_feed.count() - before_feed.count();
            vadec_res.is_partial = !mid_res.result.is_end;

            vadec_res.have_time_info = mid_res.have_time_info;
            vadec_res.details = mid_res.details;
            vadec_res.have_end_silence = mid_res.have_end_silence;
            vadec_res.sil_dur_time = mid_res.sil_dur_time;
            vadec_res.hot_word_debug = mid_res.hot_word_debug;
            vadec_res.match_hotwords = mid_res.match_hotwords;
            vadec_res.last_word_time = mid_res.last_word_time;
            vadec_res.last_decode_time = mid_res.last_decode_time;
            vadec_res.have_parsed_param = mid_res.have_parsed_param;
            vadec_res.request_params = mid_res.request_params;
            vadec_res.requestparsedparams = mid_res.requestparsedparams;
            vadec_res.nbest = mid_res.nbest;
            vadec_res.have_nbest = mid_res.have_nbest;
        }
        else {
            vadec_res.code = -1;
            vadec_res.msg = "decode failed.";
            std::cout << "MidResult json parse failed:" << ps.description() << std::endl;
        }

#ifdef OUT_PUT_SPEED_LOG
        if(info.period_end == 1)
        {
            int use_time = (after_feed.count() - before_feed.count());
            double real_rate = (use_time+0.0)/(vadec_res.end_time-vadec_res.begin_time);
            LOG(INFO) << "decode_use_time:" << use_time<< "    real_time_rate:" << real_rate << "   wav_time:" << (vadec_res.end_time-vadec_res.begin_time);
        }
#endif

        if(mid_res.is_changed)
        {
            if (info.period_end) {
                tlv_kaldi_dec_clear(app_);
                snt_index_++;
            }
            std::string vadec_res_str = staticjson::to_json_string(vadec_res);
            if (vadec_res.result != "" || info.all_end) {
              cb_(vadec_res_str.c_str(), vadec_res_str.length(), info.all_end, userdata_);
            }
        }

        return dec_rc;
    }
    return dec_rc;
}

int TlvKaldiVadec::Reset() {
    s_start_ = 0;
    asr_start_timestamp_ = 0;
    vstream_.Reset();
    {
        boost::lock_guard<boost::mutex> lg(extra_param_mu_);
        tlv_extra_params_reset(extra_params_);
        (void)lg;
    }

    return tlv_kaldi_dec_reset(app_);
}
int TlvKaldiVadec::Start() {
  boost::lock_guard<boost::mutex> lg(extra_param_mu_);
  tlv_kaldi_dec_start(app_, extra_params_);
  (void)lg;
  return 0;
}
