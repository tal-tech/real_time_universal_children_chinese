/*
 * tlv_shash.c
 *
 *  Created on: Jun 20, 2018
 *      Author: jfyuan
 */
#include "tlv_define.h"
#include "tlv_shash.h"

tlv_shash_t* tlv_shash_new(int nslot)
{
	tlv_shash_t *hash;

	hash = (tlv_shash_t*)tlv_malloc(sizeof(tlv_shash_t));
	hash->nslot = nslot;
	hash->used  = 0;
	hash->slot  = (tlv_list_node_t*)tlv_malloc(sizeof(tlv_list_node_t));
	tlv_shash_reset(hash);

	return hash;
}

void tlv_shash_add(tlv_shash_t* h, unsigned int id, tlv_list_node_t* node)
{
	tlv_list_node_t *s;
	unsigned int index;

	++h->used;
	index = id % h->nslot;
	s = h->slot + index;
	node->prev = s->prev;
	s->prev = node;
}

void tlv_shash_reset(tlv_shash_t* h)
{
	if(h->used > 0)
	{
		if(h->nslot > 40960)
		{
			tlv_free(h->slot);
			h->slot = (tlv_list_node_t*)tlv_calloc(h->nslot, sizeof(tlv_list_node_t));
		}
		else
		{
			memset(h->slot, 0, sizeof(tlv_list_node_t) * h->nslot);
		}
		h->used = 0;
	}
}


void tlv_shash_delete(tlv_shash_t* h)
{
	tlv_free(h->slot);
	tlv_free(h);
}
