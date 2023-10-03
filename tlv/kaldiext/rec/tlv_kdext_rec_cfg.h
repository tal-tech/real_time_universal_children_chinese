/*
 * tlv_rec_cfg.h
 *
 *  Created on: Dec 13, 2018
 *      Author: jfyuan
 *  @brief decode
 *         record as latgen-faster-mapped
 *
 */

#ifndef TAL_KLDEXT_TLV_REC_CFG_H_
#define TAL_KLDEXT_TLV_REC_CFG_H_

#include "hmm/transition-model.h"
#include "fstext/fstext-lib.h"
#include "decoder/decoder-wrappers.h"

#ifndef USE_NNET1
#include "nnet3/nnet-am-decodable-simple.h"
#include "nnet3/nnet-utils.h"
#include "nnet3/decodable-simple-looped.h"
#endif

#include "tlv/sphlib/cfg/tlv_part_cfg.h"
#include "tlv/sphlib/cfg/tlv_cfg_file.h"
#include "tlv/sphlib/hdict/tlv_dict.h"
#include <set>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

using namespace kaldi;

#ifndef USE_NNET1
using namespace kaldi::nnet3;
#endif

using fst::SymbolTable;
using fst::Fst;
using fst::StdArc;
using fst::ConstFst;

typedef struct tlv_kdext_rec_cfg tlv_kdext_rec_cfg_t;
struct tlv_kdext_rec_cfg
{
	unsigned char is_online:1;
	unsigned char use_grammar_fst:1;
	unsigned char use_fst_combine:1;

	int min_active;
	int max_active;
	int max_mem;
	float beam;
	float lattice_beam;
	unsigned char allow_partial:1; /* If true, produce output even if end state was not reached. */

	float acoustic_scale;
	float lm_scale;
	float word_penalty;

	char* word_syms_filename;
	char* model_in_filename;
	char* fst_in_str;
	char* phn_syms_filename;
	char* dict_name;
	char* ques_mark_filename;     /* 问号标志词语 */

	TransitionModel* trans_model;
	fst::SymbolTable *word_syms;
	LatticeFasterDecoderConfig *config;
	Fst<StdArc> *decode_fst;

	/* for grammar fst */
	char* part_fst_filename;
	char* part_wrdsyb_filename;
	fst::ConstFst<StdArc> *top_fst;

	fst::SymbolTable *phn_syms;
	tlv_label_t* label;
	tlv_dict_t* dict;
	tlv_heap_t *heap;
	tlv_array_t *arr_ques_mark;

	/* nnet3 */
	unsigned char use_nnet3:1;
	int frame_subsampling_factor;
	int frames_per_chunk;
#ifndef USE_NNET1
	NnetSimpleComputationOptions *decodable_opts;

	/* for online */
	NnetSimpleLoopedComputationOptions *decodable_opts_loop;
#endif

	/* control output format */
	unsigned char use_timeinfo:1;
	unsigned char add_space:1;
	unsigned char add_punc:1;      /* 句末添加标点符号 */
	int nbest;

	/* the same as scripts */
	unsigned char use_scripts:1;
	float inv_acoustic_scale;
	float lat_acoustic_scale;

    unsigned char is_en_asr:1;

	unsigned char use_log:1;      /* for debug */
	
    /* hot words */
    kaldi::BaseFloat hot_word_global_weight;   /* 热词的权重，取负后代表graph cost减小值。这里是配置文件中的默认值，实际dec实例使用的，可能是用户传参过来的值 */
    int hot_word_nbest;  /*培优英语热词功能下，进行nbest匹配的条数。小于等于1的情况下，在热词功能仅调整路径cost，取onebest*/
    unsigned char  hot_word_nbest_reorder; /* 是否根据匹配度重新排序 */
    char* exclude_hotwords_file;
    std::set<std::string>* exclude_hotwords_set;
    unsigned char  hot_word_debug;

    int sil_code;
};

int tlv_kdext_rec_cfg_init(tlv_kdext_rec_cfg_t* cfg);
int tlv_kdext_rec_cfg_load_param(tlv_kdext_rec_cfg_t* cfg, tlv_part_cfg_t* part);
int tlv_kdext_rec_cfg_load_res(tlv_kdext_rec_cfg_t* cfg, tlv_strfile_loader_t *sl);
int tlv_kdext_rec_cfg_clean(tlv_kdext_rec_cfg_t* cfg);

#ifdef __cplusplus
};
#endif

#endif /* TAL_KLDEXT_TLV_REC_CFG_H_ */
