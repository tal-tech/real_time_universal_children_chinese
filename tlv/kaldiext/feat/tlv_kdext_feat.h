/*
 * tlv_kdext_feat.h
 *
 *  Created on: Dec 27, 2018
 *      Author: jfyuan
 */

#ifndef TLV_KDEXT_FEAT_H_
#define TLV_KDEXT_FEAT_H_

#include "tlv_kdext_feat_cfg.h"
#include "tlv/struct/tlv_errinfo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlv_kdext_feat tlv_kdext_feat_t;
struct tlv_kdext_feat
{
	const tlv_kdext_feat_cfg_t* cfg;
	tlv_errinfo_t* errinfo;
	Fbank *fbank;

	Matrix<BaseFloat> *fea;

	/* for online */
//	char* remain_data;
//	int   remain_len;
	char odd_char;
	unsigned char is_odd:1;
	OnlineNnet2FeaturePipeline *feature_pipeline;
};

tlv_kdext_feat_t* tlv_kdext_feat_new(const tlv_kdext_feat_cfg_t* cfg);
int tlv_kdext_feat_delete(tlv_kdext_feat_t* f);
int tlv_kdext_feat_reset(tlv_kdext_feat_t* f);
int tlv_kdext_feat_feed(tlv_kdext_feat_t* f, char* data, int len, unsigned char is_end);

#ifdef __cplusplus
};
#endif

#endif /* TLV_KDEXT_FEAT_H_ */
