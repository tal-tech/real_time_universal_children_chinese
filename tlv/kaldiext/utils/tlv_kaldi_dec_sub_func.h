/*
 * tlv_kaldi_dec.h
 *
 *  Created on: Jan 3, 2019
 *      Author: jfyuan
 */

#ifndef TLV_KALDI_DEC_SUB_FUNC_H_
#define TLV_KALDI_DEC_SUB_FUNC_H_

#include "tlv/struct/tlv_array.h"
#include "base/kaldi-common.h"

#include <string>
#include <vector>

struct tlv_extra_param;
typedef struct tlv_extra_param tlv_extra_param_t;

struct tlv_kaldi_dec;
typedef struct tlv_kaldi_dec tlv_kaldi_dec_t;

struct cJSON;

#ifdef __cplusplus
extern "C" {
#endif

void sub_func_add_timeinfo(tlv_kaldi_dec_t* dec, cJSON* json, tlv_array_t* arr, bool is_en_asr);

void sub_func_add_end_silence(tlv_kaldi_dec_t* dec, cJSON* json, const std::vector<int32>& alignment);

bool sub_func_is_need_timeinfo(tlv_kaldi_dec_t* dec);

void sub_func_add_request_params_info(tlv_kaldi_dec_t* dec, cJSON* json, bool is_en_asr);

void sub_func_reorder_by_hot_words(tlv_kaldi_dec_t* dec, std::vector<std::string>* nbest_rst);
void sub_func_add_nbest_rslt(cJSON* json, const std::vector<std::string>* results, bool is_en_asr, int output_num);

void sub_func_record_nbest_point(tlv_kaldi_dec_t* dec);
int32 sub_func_time_span_from_last_nbest(tlv_kaldi_dec_t* dec);

#ifdef __cplusplus
};
#endif

#endif /* TLV_KALDI_DEC_SUB_FUNC_H_ */
