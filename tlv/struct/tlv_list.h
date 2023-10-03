/*
 * tlv_list.h
 *
 *  Created on: Jun 19, 2018
 *      Author: jfyuan
 */

#ifndef TAL_STRUCT_TLV_LIST_H_
#define TAL_STRUCT_TLV_LIST_H_

#include "tlv_list.h"

#ifdef _cplusplus
extern "C" {
#endif

typedef struct tlv_list_node tlv_list_t;
typedef struct tlv_list_node tlv_list_node_t;
typedef struct tlv_list_item tlv_list_item_t;

struct tlv_list_node
{
	tlv_list_node_t *prev;
};

struct tlv_list_item
{
	tlv_list_node_t list_n;
	void *hook;
};

void tlv_list_init(tlv_list_t *list);
void tlv_list_push(tlv_list_t *list, tlv_list_node_t* node);
void tlv_list_push_front(tlv_list_t *list, tlv_list_node_t *node);
int  tlv_list_remove(tlv_list_t *list, tlv_list_node_t *node);
tlv_list_node_t *tlv_list_pop(tlv_list_t *list);
int tlv_list_len(tlv_list_t *list);

#ifdef _cplusplus
};
#endif

#endif /* TAL_STRUCT_TLV_LIST_H_ */
