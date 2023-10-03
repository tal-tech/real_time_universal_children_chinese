// #include "tlv_kaldi_vadec_export.h"
#include "tlv_kaldi_vadec_interface.h"

#include "tlv_kaldi_vadec.h"

typedef TlvKaldiVadec tlv_kaldi_vadec_t;

tlv_kaldi_vadec_t* tlv_kaldi_vadec_new(const tlv_kaldi_dec_cfg_t* rec_cfg,
    tlv_kaldi_vadec_callback_t cb, const std::string userdata) {
  tlv_kaldi_vadec_t* dec = new tlv_kaldi_vadec_t(rec_cfg);
  int r = dec->Init(cb, userdata);
  if (r == 0) {
    return dec;
  }
  else {
    delete dec;
    dec = nullptr;
    return nullptr;
  }
}

int tlv_kaldi_query_cfg_is_en(tlv_kaldi_vadec_t* dec)
{
    int ret = 0;
    const tlv_kaldi_dec_cfg_t* cfg = dec->GetConfig();
    if(cfg && cfg->language)
    {
        std::string language(cfg->language);
        if(language == "en")
        {
            ret = 1;
        }
    }
    return ret;
}

int tlv_kaldi_vadec_start(tlv_kaldi_vadec_t* dec, const char* params, int len)
{
    if(dec == NULL)
    {
        return -1;
    }
    dec->SetExtralParams(params, len);
    dec->Start();
    return 0;
}

int tlv_kaldi_vadec_feed(tlv_kaldi_vadec_t* dec, char* data,
    int len, unsigned char class_end, unsigned char all_end) {
  if (dec) {
    return dec->Process(data, len, class_end, all_end);
  }
  else {
    return -1;
  }
}

int tlv_kaldi_vadec_reset(tlv_kaldi_vadec_t* dec) {
  if (dec) {
    return dec->Reset();
  }
  else {
    return -1;
  }
}

int tlv_kaldi_vadec_delete(tlv_kaldi_vadec_t* dec) {
  if (dec) {
    dec->Uninit();
    delete dec;
    dec = nullptr;
    return 0;
  }
  else {
    return -1;
  }
}
