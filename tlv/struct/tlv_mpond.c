#include "tlv_mpond.h"

static void* tlv_mpond_new_item(tlv_mpond_t *v)
{
	switch(v->type)
	{
	case tlv_mpond_BITHEAP:
		return tlv_bit_heap_malloc(v->v.bitheap);
		break;
	case tlv_mpond_HEAP:
		return tlv_heap_malloc(v->v.heap,v->alloc);
		break;
	case tlv_mpond_CHEAP:
		return tlv_malloc(v->alloc);
		break;
	}
	return NULL;
}

static int tlv_mpond_delete_item(tlv_mpond_t *v,void *data)
{
	switch(v->type)
	{
	case tlv_mpond_BITHEAP:
		tlv_bit_heap_free(v->v.bitheap,data);
		break;
	case tlv_mpond_HEAP:
		break;
	case tlv_mpond_CHEAP:
		tlv_free(data);
		break;
	}

	return 0;
}

tlv_mpond_t* tlv_mpond_new(int bytes, int max_free)
{
	return tlv_mpond_new2(bytes, max_free, max_free, tlv_mpond_BITHEAP, 128);
}

tlv_mpond_t* tlv_mpond_new2(int bytes,int max_free,int reset_free,tlv_mpond_type_t type,int max_item)
{
	tlv_mpond_t *v;

	v=(tlv_mpond_t*)tlv_malloc(sizeof(*v));
	v->max=max_item;
	v->type=type;
	v->bytes=bytes;
	v->alloc=bytes+sizeof(tlv_queue_node_t);

	v->reset_free=reset_free;
	if(v->bytes>v->max)
	{
		v->type=tlv_mpond_CHEAP;
		tlv_cellar_init2(&(v->hoard),bytes,max_free,
				(tlv_new_handler_f)tlv_mpond_new_item,
				(tlv_delete_handler2_f)tlv_mpond_delete_item,
				v);
	}else
	{
		switch(v->type)
		{
		case tlv_mpond_BITHEAP:
			v->v.bitheap=tlv_bit_heap_new(v->alloc,4096/v->alloc,40960,1.0);
			break;
		case tlv_mpond_HEAP:
			v->v.heap=tlv_heap_new(4096);
			break;
		case tlv_mpond_CHEAP:
			break;
		}
		tlv_cellar_init2(&(v->hoard),bytes,max_free,
			(tlv_new_handler_f)tlv_mpond_new_item,
			(tlv_delete_handler2_f)0,
			v);
	}
	return v;
}

void* tlv_mpond_pop(tlv_mpond_t *v)
{
	return tlv_cellar_pop(&(v->hoard));
}

void tlv_mpond_push(tlv_mpond_t *v, void *usr_data)
{
	tlv_cellar_push(&(v->hoard),usr_data);
}

void tlv_mpond_delete(tlv_mpond_t *v)
{
	switch(v->type)
	{
	case tlv_mpond_BITHEAP:
		tlv_bit_heap_delete(v->v.bitheap);
		break;
	case tlv_mpond_HEAP:
		tlv_heap_delete(v->v.heap);
		break;
	case tlv_mpond_CHEAP:
		tlv_cellar_clean(&(v->hoard));
		break;
	}
	tlv_free(v);
}

void tlv_mpond_reset(tlv_mpond_t *v)
{
	int max_free;

	switch(v->type)
	{
	case tlv_mpond_BITHEAP:
		tlv_cellar_reset(&(v->hoard));
		tlv_bit_heap_reset(v->v.bitheap);
		break;
	case tlv_mpond_HEAP:
		tlv_cellar_reset(&(v->hoard));
		tlv_heap_reset(v->v.heap);
		break;
	case tlv_mpond_CHEAP:
		max_free=v->hoard.max_free;
		v->hoard.max_free=v->reset_free;

		tlv_cellar_reuse(&(v->hoard));
		v->hoard.max_free=max_free;

		break;
	}
}

int tlv_mpond_bytes(tlv_mpond_t *v)
{
	int n;

	switch(v->type)
	{
	case tlv_mpond_BITHEAP:
		n=tlv_bit_heap_bytes(v->v.bitheap);
		break;
	case tlv_mpond_HEAP:
		n=tlv_heap_bytes(v->v.heap);
		break;
	case tlv_mpond_CHEAP:
		n=v->hoard.use_length+v->hoard.cur_free;
		n*=v->alloc;
		break;
	default:
		n=0;
		break;
	}

	return n;
}
