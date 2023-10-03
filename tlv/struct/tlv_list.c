/*
 * tlv_list.c
 *
 *  Created on: Jun 19, 2018
 *      Author: jfyuan
 */
#include <stdio.h>
#include "tlv_list.h"

void tlv_list_init(tlv_list_t *list)
{
	list->prev = NULL;
}

void tlv_list_push(tlv_list_t *list, tlv_list_node_t *node)
{
	node->prev = list->prev;
	list->prev = node;
}

void tlv_list_push_front(tlv_list_t *list, tlv_list_node_t *node)
{
	while(list->prev)
	{
		list = list->prev;
	}
	node->prev = NULL;
	list->prev = node;
}

int  tlv_list_remove(tlv_list_t *list, tlv_list_node_t *node)
{
	tlv_list_node_t *tmp, *prev;

	for(prev=NULL, tmp=list->prev; tmp; tmp=tmp->prev)
	{
		if(tmp == node)
		{
			if(prev)
			{
				prev->prev = tmp->prev;
			}
			else
			{
				list->prev = tmp->prev;
			}

			return 1;
		}
		prev = tmp;
	}

	return 0;
}

tlv_list_node_t *tlv_list_pop(tlv_list_t *list)
{
	tlv_list_node_t *node;

	if(list->prev)
	{
		node = list->prev;
		list->prev = node->prev;
	}
	else
	{
		node = NULL;
	}

	return node;
}

int tlv_list_len(tlv_list_t *list)
{
	tlv_list_node_t *n;
	int len = 0;

	for(n=list->prev; n; n=n->prev)
	{
		++len;
	}

	return len;
}
