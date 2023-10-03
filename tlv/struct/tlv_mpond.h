#ifndef TAL_STRUCT_TLV_MPOND_H_
#define TAL_STRUCT_TLV_MPOND_H_
#include "tlv/struct/tlv_bit_heap.h"
#include "tlv/struct/tlv_cellar.h"
#include "tlv/struct/tlv_heap.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct tlv_mpond tlv_mpond_t;

typedef enum
{
	tlv_mpond_BITHEAP,
	tlv_mpond_HEAP,
	tlv_mpond_CHEAP,
}tlv_mpond_type_t;

struct tlv_mpond
{
	union
	{
		tlv_heap_t *heap;
		tlv_bit_heap_t *bitheap;
	}v;
	tlv_cellar_t hoard;
	int bytes;
	int max;
	int alloc;
	int reset_free;
	tlv_mpond_type_t type;
};

tlv_mpond_t* tlv_mpond_new(int bytes, int max_free);
tlv_mpond_t* tlv_mpond_new2(int bytes, int max_free, int reset_free, tlv_mpond_type_t type, int max_item);
void* tlv_mpond_pop(tlv_mpond_t *v);
void tlv_mpond_push(tlv_mpond_t *v, void *usr_data);
void tlv_mpond_delete(tlv_mpond_t *v);
void tlv_mpond_reset(tlv_mpond_t *v);
int tlv_mpond_bytes(tlv_mpond_t *v);

#ifdef __cplusplus
};
#endif
#endif
