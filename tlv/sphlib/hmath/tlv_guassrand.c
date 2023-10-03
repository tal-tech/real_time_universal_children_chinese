/*
 * tlv_guassrand.c
 *
 *  Created on: Sep 3, 2018
 *      Author: jfyuan
 */
#include "tlv_guassrand.h"

void tlv_guassrand_init(tlv_guassrand_t *r)
{
	tlv_guassrand_reset(r);
}

void tlv_guassrand_clean(tlv_guassrand_t *r)
{

}

void tlv_guassrand_reset(tlv_guassrand_t *r)
{
	r->V1 = r->V2 = r->S = 0;
	r->phase = 0;
	srand(1264122275);
}

float tlv_guassrand_rand(tlv_guassrand_t *r, float mean, float delta)
{
	float X;

	if(r->phase == 0)
	{
		do{
			float U1 = (float)rand() / RAND_MAX;
			float U2 = (float)rand() / RAND_MAX;

			r->V1 = 2 * U1 - 1;
			r->V2 = 2 * U2 - 1;
			r->S  = r->V1 * r->V1 + r->V2 * r->V2;
		}while(r->S >= 1 || r->S == 0);

		X = r->V1 * sqrt(-2 * log(r->S) / r->S);
	}
	else
	{
		X = r->V2 * sqrt(-2 * log(r->S) / r->S);
	}

	r->phase = 1 - r->phase;

	return X * sqrt(delta) + mean;
}

