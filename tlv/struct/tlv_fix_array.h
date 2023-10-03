#ifndef TAL_STRUCT_TLV_FIX_ARRAY_H_
#define TAL_STRUCT_TLV_FIX_ARRAY_H_
#include "tlv/struct/tlv_define.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct tlv_fix_array tlv_fix_array_t;

struct tlv_fix_array
{
	void*    item;
	uint32_t item_size;
	uint32_t item_alloc;
	uint32_t nitem;
};

tlv_fix_array_t* tlv_fix_array_new(uint32_t n, uint32_t size);
void* tlv_fix_array_push_n(tlv_fix_array_t* fa, uint32_t n);
int tlv_fix_array_delete(tlv_fix_array_t *fa);
int tlv_fix_array_bytes(tlv_fix_array_t *fa);

#ifdef __cplusplus
};
#endif
#endif
