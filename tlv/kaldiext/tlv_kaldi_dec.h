/*
 * tlv_kaldi_dec.h
 *
 *  Created on: Jan 3, 2019
 *      Author: jfyuan
 */

#ifndef TLV_KALDI_DEC_H_
#define TLV_KALDI_DEC_H_

#include "tlv_kaldi_dec_cfg.h"
#include "feat/tlv_kdext_feat.h"
#include "forward/tlv_kdext_forward.h"
#include "rec/tlv_kdext_rec.h"
#include "tlv/struct/tlv_errinfo.h"

#include <string>

struct tlv_extra_param;
typedef struct tlv_extra_param tlv_extra_param_t;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlv_kaldi_dec tlv_kaldi_dec_t;
struct tlv_kaldi_dec
{
	const tlv_kaldi_dec_cfg_t* cfg;

	tlv_kdext_feat_t* feat;

#ifdef USE_NNET1
	tlv_kdext_forward_t* forward;
#endif

	tlv_kdext_rec_t*  rec;

	tlv_errinfo_t* errinfo;

	/* cal time */
	double t_forward;
	double t_rec;
	double t_tot;
	double t_data_feed_end;
	double t_delay;
    tlv_extra_param_t* extra_params;
};

tlv_kaldi_dec_t* tlv_kaldi_dec_new(const tlv_kaldi_dec_cfg_t* cfg);
int tlv_kaldi_dec_start(tlv_kaldi_dec_t* dec, tlv_extra_param_t* extra_params);
int tlv_kaldi_dec_feed(tlv_kaldi_dec_t* dec, char* data, int len, unsigned char is_end);
int tlv_kaldi_dec_reset(tlv_kaldi_dec_t* dec);
int tlv_kaldi_dec_clear(tlv_kaldi_dec_t* dec);
int tlv_kaldi_dec_delete(tlv_kaldi_dec_t* dec);
int tlv_kaldi_dec_get_rslt(tlv_kaldi_dec_t* dec, char** rslt, int* rslt_len);

#ifdef __cplusplus
};
#endif

#endif /* TLV_KALDI_DEC_H_ */
