#include "tlv_array.h"

tlv_array_t* tlv_array_new(tlv_heap_t* h, uint32_t n, uint32_t size)
{
	tlv_array_t* a;

	a = (tlv_array_t*)tlv_heap_malloc(h, sizeof(*a));
	a->nitem = 0;
	a->item_size = size;
	a->item_alloc = n;
	a->heap = h;
	a->item = tlv_heap_malloc(h,n*size);
	return a;
}

int tlv_array_delete(tlv_array_t* a)
{
	tlv_heap_block_t *b;

	b=a->heap->current;
	if((uint8_t*)(a->item)+a->item_size*a->item_alloc == b->last)
	{
		b->last-=a->item_size*a->item_alloc;
	}
	if((uint8_t*)a+sizeof(*a)==b->last)
	{
		b->last=(uint8_t*)a;
	}
	return 0;
}

void tlv_array_reset(tlv_array_t *a)
{
	a->nitem = 0;
}

void* tlv_array_push_n(tlv_array_t* a,uint32_t n)
{
	tlv_heap_block_t *b;
	uint32_t size,alloc;
	void *s;

	size=n*a->item_size;
	if(a->nitem+n > a->item_alloc)
	{
		b=a->heap->current;
		if( ((uint8_t*)a->item+a->item_size*a->item_alloc == b->last)
				&& (b->last+size <= b->end) )
		{
			b->last+=size;
			a->item_alloc+=n;
		}else
		{
			alloc=2*max(n,a->item_alloc);
			s=tlv_heap_malloc(a->heap,alloc*a->item_size);
			memcpy(s,a->item,a->item_size*a->nitem);
			a->item=s;
			a->item_alloc=alloc;
		}
	}
	s=(char*)a->item+a->item_size*a->nitem;
	a->nitem+=n;
	return s;
}

void* tlv_array_push(tlv_array_t* a)
{
	return tlv_array_push_n(a,1);
}

void tlv_array_push2(tlv_array_t *a,void *src)
{
	void *dst;

	dst=tlv_array_push_n(a,1);

	memcpy(dst,src,a->item_size);
}
