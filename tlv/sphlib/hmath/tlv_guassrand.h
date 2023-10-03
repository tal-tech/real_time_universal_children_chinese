/*
 * tlv_guassrand.h
 *
 *  Created on: Sep 3, 2018
 *      Author: jfyuan
 */

#ifndef TAL_SPHLIB_TLV_HMATH_GUASSRAND_H_
#define TAL_SPHLIB_TLV_HMATH_GUASSRAND_H_

#include <math.h>
#include "tlv/struct/tlv_define.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlv_guassrand tlv_guassrand_t;

struct tlv_guassrand
{
	 float V1;
	 float V2;
	 float S;
	 int phase;
};

void tlv_guassrand_init(tlv_guassrand_t *r);
void tlv_guassrand_clean(tlv_guassrand_t *r);
void tlv_guassrand_reset(tlv_guassrand_t *r);
float tlv_guassrand_rand(tlv_guassrand_t *r, float mean, float delta);

#ifdef __cplusplus
};
#endif

#endif /* TLV_GUASSRAND_H_ */
