#ifndef TAL_STRUCT_TLV_ARRAY_H_
#define TAL_STRUCT_TLV_ARRAY_H_
#include "tlv/struct/tlv_define.h"
#include "tlv_heap.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @date	2018.3.26
 * @brief	创建数组，放置个数是可变的。
 */
typedef struct tlv_array
{
	tlv_heap_t* heap;
	void	*item;
	uint32_t nitem;
	uint32_t item_size;    /* 一个item的大小 */
	uint32_t item_alloc;   /* 实际申请空间时，可存储的item个数 */
}tlv_array_t;

tlv_array_t* tlv_array_new(tlv_heap_t* h, uint32_t n, uint32_t size);

void* tlv_array_push(tlv_array_t* a);
void tlv_array_push2(tlv_array_t *a, void *addr);
void* tlv_array_push_n(tlv_array_t* a, uint32_t n);

void tlv_array_reset(tlv_array_t *a);
int tlv_array_delete(tlv_array_t* a);

#ifdef __cplusplus
};
#endif

#endif  /* TAL_STRUCT_TLV_ARRAY_H_ */
