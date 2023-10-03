/*
 * tlv_extra_param.h
 *
 *  Created on: 12.10, 2019
 *      Author: will
 *  desc : 附加参数，或这从cfg中拷贝一些参数(如nbest)
 */

#ifndef TAL_STRUCT_TLV_EXTRA_PARAM_H_
#define TAL_STRUCT_TLV_EXTRA_PARAM_H_

#include "base/kaldi-types.h"

#include <vector>
#include <string>

struct tlv_kaldi_dec_cfg;
typedef struct tlv_kaldi_dec_cfg tlv_kaldi_dec_cfg_t;

typedef struct tlv_extra_param tlv_extra_param_t;
struct tlv_extra_param
{
    const tlv_kaldi_dec_cfg_t* cfg;

    int nbest;

    kaldi::BaseFloat hot_word_global_weight;
    std::vector<int64> hot_word_ids;
    std::vector<std::string> hot_words;
    std::vector<std::string> expend_hot_words;
    int hot_word_nbest;
    int hot_word_nbest_reorder;
    float vad_max_sentence_second;

    bool end_silence;//1=输出句末停顿时长，用于控制截停
    bool use_vad;  // VAD 开关, true 开VAD, false 关VAD
    int output_params;//1=out put params to result

    std::string original_params;

	/* for grammar fst */
	std::string part_fst_dir;

    tlv_extra_param(const tlv_kaldi_dec_cfg_t* c);

    tlv_extra_param& operator=(const tlv_extra_param& cls)
    {
        if (this != &cls)
        {
            this->cfg = cls.cfg;
            this->nbest = cls.nbest;
            this->hot_word_global_weight = cls.hot_word_global_weight;
            this->hot_word_ids = cls.hot_word_ids;
            this->hot_words = cls.hot_words;
            this->expend_hot_words = cls.expend_hot_words;
            this->hot_word_nbest = cls.hot_word_nbest;
            this->hot_word_nbest_reorder = cls.hot_word_nbest_reorder;
            this->end_silence = cls.end_silence;
            this->use_vad = cls.use_vad;
            this->output_params = cls.output_params;
            this->original_params = cls.original_params;
            this->vad_max_sentence_second = cls.vad_max_sentence_second;
            this->part_fst_dir = cls.part_fst_dir;
        }
        return *this;
    }
};

bool tlv_parse_extra_params(std::string json, tlv_extra_param_t* param);
void tlv_extra_params_reset(tlv_extra_param_t* param);

#endif /* TAL_STRUCT_TLV_EXTRA_PARAM_H_ */
