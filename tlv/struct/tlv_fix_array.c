#include "tlv_fix_array.h"

tlv_fix_array_t* tlv_fix_array_new(uint32_t n, uint32_t size)
{
	tlv_fix_array_t* fa;

	fa = (tlv_fix_array_t*)tlv_malloc(sizeof(*fa));

	fa->item  = tlv_calloc(n, size);
	fa->nitem = 0;

	fa->item_alloc = n;
	fa->item_size  = size;

	return fa;
}

void* tlv_fix_array_push_n(tlv_fix_array_t* fa, uint32_t n)
{
	uint32_t alloc;
	void *s;

	if(fa->nitem+n > fa->item_alloc)
	{
		alloc = 2*max(n,fa->item_alloc);
		s = tlv_calloc(alloc,fa->item_size);
		memcpy(s, fa->item, fa->item_size*fa->nitem);
		tlv_free(fa->item);
		fa->item = s;
		fa->item_alloc = alloc;
	}
	s = (char*)fa->item + fa->item_size*fa->nitem;
	fa->nitem += n;

	return s;
}

int tlv_fix_array_delete(tlv_fix_array_t* fa)
{
	tlv_free(fa->item);
	tlv_free(fa);
	return 0;
}

int tlv_fix_array_bytes(tlv_fix_array_t *fa)
{
	int bytes = sizeof(tlv_fix_array_t);

	bytes += fa->item_size * fa->item_alloc;

	return bytes;
}
