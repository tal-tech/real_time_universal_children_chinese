#ifndef TLV_KALDI_VADEC_INTERFACE_H_
#define TLV_KALDI_VADEC_INTERFACE_H_

#include "tlv_kaldi_dec_interface.h"
#include <string>
#include <map>

/* recognition api */
struct TlvKaldiVadec;
typedef struct  TlvKaldiVadec tlv_kaldi_vadec_t;

struct tlv_extra_param;
typedef struct tlv_extra_param tlv_extra_param_t;

typedef void(* tlv_kaldi_vadec_callback_t)(const char* json, unsigned int json_len, bool all_end, const std::string userdata);

tlv_kaldi_vadec_t* tlv_kaldi_vadec_new(const tlv_kaldi_dec_cfg_t* rec_cfg,
    tlv_kaldi_vadec_callback_t cb, const std::string userdata);

int tlv_kaldi_query_cfg_is_en(tlv_kaldi_vadec_t* dec);

int tlv_kaldi_vadec_start(tlv_kaldi_vadec_t* dec, const char* params, int len);

int tlv_kaldi_vadec_feed(tlv_kaldi_vadec_t* dec, char* data,
    int len, unsigned char class_end, unsigned char all_end);

int tlv_kaldi_vadec_reset(tlv_kaldi_vadec_t* dec);

int tlv_kaldi_vadec_delete(tlv_kaldi_vadec_t* dec);

#endif // TLV_KALDI_VADEC_INTERFACE_H_
