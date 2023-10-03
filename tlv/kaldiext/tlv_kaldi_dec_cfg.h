/*
 * tlv_kaldi_dec_cfg.h
 *
 *  Created on: Jan 3, 2019
 *      Author: jfyuan
 */

#ifndef TLV_KALDI_DEC_CFG_H_
#define TLV_KALDI_DEC_CFG_H_

#include "feat/tlv_kdext_feat_cfg.h"
#include "forward/tlv_kdext_forward_cfg.h"
#include "rec/tlv_kdext_rec_cfg.h"
#include "tlv/kaldiext/vadparam/vadparamdefine.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlv_kaldi_dec_cfg tlv_kaldi_dec_cfg_t;
struct tlv_kaldi_dec_cfg
{
	char* version;
	unsigned char use_nnet3:1;
	unsigned char is_online:1;
	char* language;

	tlv_kdext_feat_cfg_t feat;

#ifdef USE_NNET1
	tlv_kdext_forward_cfg_t forward;
#endif

	tlv_kdext_rec_cfg_t  rec;

	/* for load */
	tlv_cfg_file_t *cf;

    /* defaut is "webrtc_vad" or use "webrtc_vad_ex" */
    VadInterface::VadParams* vad_params;

    std::string cfg_path;//cfg

	/* for debug */
	unsigned char use_log:1;
};

int tlv_kaldi_dec_cfg_init(tlv_kaldi_dec_cfg_t* cfg);
int tlv_kaldi_dec_cfg_load_param(tlv_kaldi_dec_cfg_t* cfg, tlv_part_cfg_t *part);
int tlv_kaldi_dec_cfg_load_res(tlv_kaldi_dec_cfg_t* cfg, tlv_strfile_loader_t *sl);
int tlv_kaldi_dec_cfg_clean(tlv_kaldi_dec_cfg_t* cfg);

tlv_kaldi_dec_cfg_t* tlv_kaldi_dec_cfg_new(const char* fn);

int tlv_kaldi_dec_cfg_get_version(tlv_kaldi_dec_cfg_t* cfg, char** version);
int tlv_kaldi_dec_cfg_get_is_en(tlv_kaldi_dec_cfg_t* cfg);//en:return 1

void tlv_kaldi_dec_cfg_delete(tlv_kaldi_dec_cfg_t* cfg);

#ifdef __cplusplus
};
#endif

#endif /* TLV_KALDI_DEC_CFG_H_ */
