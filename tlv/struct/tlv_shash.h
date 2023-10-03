/*
 * tlv_shash.h
 * @brief a type of linear hash
 *
 *  Created on: Jun 20, 2018
 *      Author: jfyuan
 */

#ifndef TAL_STRUCT_TLV_SHASH_H_
#define TAL_STRUCT_TLV_SHASH_H_

#include "tlv_list.h"

#ifdef _cplusplus
extern "C" {
#endif

typedef struct tlv_shash tlv_shash_t;

struct tlv_shash
{
	unsigned int nslot;
	unsigned int used;
	tlv_list_node_t *slot;
};

tlv_shash_t* tlv_shash_new(int nslot);
void tlv_shash_add(tlv_shash_t* h, unsigned int id, tlv_list_node_t* node);
void tlv_shash_reset(tlv_shash_t* h);
void tlv_shash_delete(tlv_shash_t* h);

#ifdef _cplusplus
};
#endif

#endif /* TLV_SHASH_H_ */
