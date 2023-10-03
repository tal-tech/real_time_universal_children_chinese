/*
 * tlv_kdext_forward.h
 *
 *  Created on: Dec 11, 2018
 *      Author: jfyuan
 *
 *  @brief 做前向计算出后验概率值
 */

#ifndef TLV_KDEXT_FORWARD_H_
#define TLV_KDEXT_FORWARD_H_

#ifdef USE_NNET1

#include "tlv_kdext_forward_cfg.h"
#include "tlv/struct/tlv_errinfo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlv_kdext_forward tlv_kdext_forward_t;
struct tlv_kdext_forward
{
	const tlv_kdext_forward_cfg_t* cfg;
	tlv_errinfo_t* errinfo;

	PdfPrior *pdf_prior;
	Nnet *nnet_transf;
	Nnet *nnet;

	//CuMatrix<BaseFloat>* nnet_out;
	Matrix<BaseFloat> *nnet_out_host;
};

tlv_kdext_forward_t* tlv_kdext_forward_new(const tlv_kdext_forward_cfg_t* cfg);
int tlv_kdext_forward_feed(tlv_kdext_forward_t* fw, Matrix<BaseFloat> *fea);
int tlv_kdext_forward_reset(tlv_kdext_forward_t* fw);
int tlv_kdext_forward_delete(tlv_kdext_forward_t* fw);

#ifdef __cplusplus
};
#endif

#endif /* #ifdef USE_NNET1 */

#endif /* TLV_KDEXT_FORWARD_H_ */
