#include "tlv/struct/tlv_loop.h"

tlv_loop_t* tlv_loop_new(uint32_t n)
{
	tlv_loop_t* r;

	r = (tlv_loop_t*)tlv_malloc(sizeof(*r)+n*sizeof(void**));
	r->nitem   = n;
	r->first   = 0;
	r->used    = 0;
	r->r       = (void**)((char*)r+sizeof(*r));
	memset(r->r, 0, n*sizeof(void**));
	return r;
}

void tlv_loop_reset(tlv_loop_t *r)
{
	r->first = 0;
	r->used  = 0;
}

int tlv_loop_delete(tlv_loop_t* r)
{
	tlv_free(r);

	return 0;
}

void tlv_loop_push(tlv_loop_t* r,void *d)
{
	int index;

	index = (r->first + r->used)%r->nitem;
	r->r[index] = d;
	++r->used;
}

void* tlv_loop_pop(tlv_loop_t *r)
{
	void *d;

	d = r->r[r->first];
	r->first = (r->first+1) % r->nitem;
	--r->used;

	return d;
}
