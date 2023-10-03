/*
 * tlv_kdext_feat_cfg.h
 *
 *  Created on: Dec 27, 2018
 *      Author: jfyuan
 */

#ifndef TLV_KDEXT_FEAT_CFG_H_
#define TLV_KDEXT_FEAT_CFG_H_

#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "feat/feature-fbank.h"
#include "online2/online-nnet2-feature-pipeline.h"

using namespace kaldi;

#ifdef __cplusplus
extern "C" {
#endif

#include "tlv/sphlib/cfg/tlv_part_cfg.h"
#include "tlv/sphlib/cfg/tlv_cfg_file.h"

typedef struct tlv_kdext_feat_cfg tlv_kdext_feat_cfg_t;
struct tlv_kdext_feat_cfg
{
	unsigned char is_online:1;
	unsigned char use_fbank:1;
	unsigned char use_mfcc:1;
	unsigned char subtract_mean:1;
	unsigned char use_log:1;        /* for debug */

	unsigned char use_pitch:1;
	int length_tolerance;

	unsigned char apply_cmvn:1;
	unsigned char use_norm_vars:1;

	int   channel;
	int   samp_freq;
	char* window_type;
	unsigned char use_energy;
	int   dither;             /* white noise */
	int   num_mel_bins;

	int frame_shift_ms;  // in milliseconds.
	int frame_length_ms;  // in milliseconds.

	float vtln_warp;
	int   delta_order;

	FbankOptions *fbank_opts;
	DeltaFeaturesOptions *dela_opts;

	/* for online */
	float chunk_length_secs;
	int32 chunk_length;
	OnlineNnet2FeaturePipelineInfo *feature_info;
};

int tlv_kdext_feat_cfg_init(tlv_kdext_feat_cfg_t* cfg);
int tlv_kdext_feat_cfg_load_param(tlv_kdext_feat_cfg_t* cfg, tlv_part_cfg_t* part);
int tlv_kdext_feat_cfg_load_res(tlv_kdext_feat_cfg_t* cfg, tlv_strfile_loader_t *sl);
int tlv_kdext_feat_cfg_clean(tlv_kdext_feat_cfg_t* cfg);


#ifdef __cplusplus
};
#endif

#endif /* TLV_KDEXT_FEAT_CFG_H_ */
