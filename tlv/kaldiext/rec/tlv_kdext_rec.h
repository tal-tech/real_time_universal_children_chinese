/*
 * tlv_kdext_rec.h
 *
 *  Created on: Dec 17, 2018
 *      Author: jfyuan
 */

#ifndef TLV_KDEXT_REC_H_
#define TLV_KDEXT_REC_H_

#ifndef USE_NNET1
#include "online2/online-nnet3-decoding.h"
#endif

#include <string>
#include <vector>
#include "tlv_kdext_rec_cfg.h"
#include "tlv/struct/tlv_errinfo.h"

struct tlv_extra_param;
typedef struct tlv_extra_param tlv_extra_param_t;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlv_kdext_rec tlv_kdext_rec_t;
struct tlv_kdext_rec
{
	const tlv_kdext_rec_cfg_t* cfg;
	const char* part_fst_dir;
	tlv_heap_t* heap;
	tlv_errinfo_t* errinfo;

	/* nnet3 */
#ifndef USE_NNET1
	AmNnetSimple *am_nnet;
	CachingOptimizingCompiler *compiler;

	/* for online */
	unsigned char is_end:1;
	DecodableNnetSimpleLoopedInfo *decodable_info;
	SingleUtteranceNnet3Decoder   *decoder_online;
	int frames_decoded;
#endif

	OnlineNnet2FeaturePipeline* fea_pipline;
	Fst<StdArc> *decode_fst;
	LatticeFasterDecoder* decoder;
	std::string* onebest_rslt;
    std::vector<std::string>* nbest_rslt;
	Lattice* lat;
	CompactLattice* clat;
	tlv_array_t* onebest_arr;

	kaldi::int64 frame_count;
	double like;

    /* for online nbest*/
    Lattice* process_lat;
    CompactLattice* process_clat;
    kaldi::int64 last_nbest_frame_count;

	/* grammar decoder */
	fst::GrammarFst *grammar_fst;
	fst::ConstFst<StdArc> *part_fst;
	fst::SymbolTable *word_syms;
	SingleUtteranceNnet3GrammarDecoder  *grammar_decoder_online;
};

typedef struct tlv_wrdbound tlv_wrdbound_t;
struct tlv_wrdbound
{
	int id;
	tlv_string_t* name;
	int start;
	int end[3];
	unsigned char len;
	unsigned char used;  /* 实际结束时间边界 */
};

/* for offline not using now */
tlv_kdext_rec_t* tlv_kdext_rec_new(const tlv_kdext_rec_cfg_t* cfg);
int tlv_kdext_rec_delete(tlv_kdext_rec_t* r);
int tlv_kdext_rec_reset(tlv_kdext_rec_t* r);
int tlv_kdext_rec_feed(tlv_kdext_rec_t* r, Matrix<BaseFloat> *nnet_forward, tlv_extra_param_t* extra_param);
const char* tlv_kdext_rec_get_bestrslt(tlv_kdext_rec_t* r, bool need_timeinfo, std::vector<int32>& alignment);
int tlv_kdext_rec_make_grammar_fst(tlv_kdext_rec_t* r);

/* for online */
tlv_kdext_rec_t* tlv_kdext_rec_online_new(const tlv_kdext_rec_cfg_t* cfg, OnlineNnet2FeaturePipeline *fea_pipline);
int tlv_kdext_rec_online_feed(tlv_kdext_rec_t* r, tlv_extra_param_t* extra_param, unsigned char is_end);
int tlv_kdext_rec_online_reset(tlv_kdext_rec_t* r, OnlineNnet2FeaturePipeline *fea_pipline);
int tlv_kdext_rec_online_clear(tlv_kdext_rec_t* r);
const char* tlv_kdext_rec_online_get_bestrslt(tlv_kdext_rec_t* r, bool need_timeinfo, std::vector<int32>& alignment);
const char* tlv_kdext_rec_online_get_currslt(tlv_kdext_rec_t* r, bool need_timeinfo, bool& is_changed, std::vector<int32>& alignment);
std::vector<std::string>* tlv_kdext_rec_get_online_nbestrslt(tlv_kdext_rec_t* r, int nbest);

#ifdef __cplusplus
};
#endif

#endif /* TLV_KDEXT_REC_H_ */
