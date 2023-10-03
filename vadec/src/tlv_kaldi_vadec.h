#ifndef TLV_KALDI_VADEC_H_
#define TLV_KALDI_VADEC_H_

#include "tlv_kaldi_dec_cfg.h"
#include "tlv_kaldi_dec.h"
#include "vadec_stream.h"

#include <vector>
#include <map>
#include <string>
#include <boost/asio/io_service.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>

struct tlv_extra_param;
typedef struct tlv_extra_param tlv_extra_param_t;

typedef void(* tlv_kaldi_vadec_callback_t)(const char* json, unsigned int json_len, bool all_end, const std::string userdata);
class TlvKaldiVadec {
public:
  TlvKaldiVadec(const tlv_kaldi_dec_cfg_t* cfg);
  ~TlvKaldiVadec();

  int Init(tlv_kaldi_vadec_callback_t cb, const std::string uerdata);

  void Uninit();

  void SetExtralParams(const char* params, int len);
  const tlv_kaldi_dec_cfg_t* GetConfig(){return cfg_;}

  int Process(char* data, unsigned int len, bool class_end, bool all_end);

  int Reset();
  int Start();

private:
  int Decode(char* data, unsigned int len, const VadecDataInfo& info);

private:
  const tlv_kaldi_dec_cfg_t* const cfg_;
  tlv_kaldi_dec* app_;

  std::string userdata_;
  VadecStream vstream_;
  unsigned int snt_index_;

  volatile bool force_stop_;

  bool need_feed_end_;

  int64_t s_start_;
  int64_t asr_start_timestamp_;

  boost::function<void(const char*, unsigned int, bool all_end, const std::string userdata)> cb_;
  std::unique_ptr<boost::thread> thread_;

  tlv_extra_param_t* extra_params_;
  boost::mutex extra_param_mu_;

};

#endif // TLV_KALDI_VADEC_H_
