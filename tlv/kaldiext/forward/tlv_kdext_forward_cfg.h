/*
 * tlv_kdext_forward_cfg.h
 *
 *  Created on: Dec 11, 2018
 *      Author: jfyuan
 */

#ifndef TLV_KDEXT_FORWARD_CFG_H_
#define TLV_KDEXT_FORWARD_CFG_H_

#ifdef USE_NNET1

#include "nnet/nnet-nnet.h"
#include "nnet/nnet-loss.h"
#include "nnet/nnet-pdf-prior.h"
#include "base/kaldi-common.h"
#include "util/common-utils.h"

using namespace kaldi;
using namespace kaldi::nnet1;

#ifdef __cplusplus
extern "C" {
#endif

#include "tlv/sphlib/cfg/tlv_part_cfg.h"
#include "tlv/sphlib/cfg/tlv_cfg_file.h"

typedef struct tlv_kdext_forward_cfg tlv_kdext_forward_cfg_t;
struct tlv_kdext_forward_cfg
{
	unsigned char no_softmax:1;  /* Removes the last component with Softmax, if found. */
	unsigned char apply_log:1;   /* Transform NN output by log() */
	unsigned char use_log:1;     /* for debug */

	float         prior_scale;   /* Scaling factor to be applied on pdf-log-priors */
	float         prior_floor;   /* 1.00000001e-10 */
	char*         class_frame_counts;

	char*         feature_transform;
	char*         model_filename;

	//PdfPriorOptions prior_opts;
	PdfPrior *pdf_prior;
	Nnet *nnet_transf;
	Nnet *nnet;
};

int tlv_kdext_forward_cfg_init(tlv_kdext_forward_cfg_t* cfg);
int tlv_kdext_forward_cfg_load_param(tlv_kdext_forward_cfg_t* cfg, tlv_part_cfg_t* part);
int tlv_kdext_forward_cfg_load_res(tlv_kdext_forward_cfg_t* cfg, tlv_strfile_loader_t *sl);
int tlv_kdext_forward_cfg_clean(tlv_kdext_forward_cfg_t* cfg);

#ifdef __cplusplus
};
#endif

#endif

#endif /* TLV_KDEXT_FORWARD_CFG_H_ */
