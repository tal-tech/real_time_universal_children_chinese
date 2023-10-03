#ifndef TAL_STRUCT_TLV_BIT_HEAP_H_
#define TAL_STRUCT_TLV_BIT_HEAP_H_
#include "tlv/struct/tlv_define.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct tlv_bit_heap tlv_bit_heap_t;

typedef struct _HeapBlock
{
	size_t elem_num;
	size_t elem_free;
	size_t first_free;
	uint8_t	*bitmap;
	uint8_t	*data;
	struct _HeapBlock* next;
}HeapBlock;

struct tlv_bit_heap
{
	HeapBlock* block_list;
	//heap_clean_handler cleaner;
	size_t elem_size;
	size_t elem_cur;
	size_t elem_min;
	size_t elem_max;
	size_t tot_alloc;
	size_t tot_used;
	float growf;
};

/**
 * @brief allocate bitmap heap.
 */
tlv_bit_heap_t* tlv_bit_heap_new(size_t elem_size,size_t elem_min,size_t elem_max,float growf);

/**
 * @brief allocate bitmap heap with elem_size.
 */
tlv_bit_heap_t* tlv_bit_heap_new2(size_t elem_size);

/**
 * @brief release all memory.
 */
int tlv_bit_heap_delete(tlv_bit_heap_t* heap);


int tlv_bit_heap_bytes(tlv_bit_heap_t *heap);

/**
 * @brief reset heap.
 */
int tlv_bit_heap_reset(tlv_bit_heap_t* heap);

/**
 * @brief allocate memory from heap.
 */
void* tlv_bit_heap_malloc(tlv_bit_heap_t* heap);

/**
 * @brief allocate with zero.
 */
void* tlv_bit_heap_zmalloc(tlv_bit_heap_t *heap);

/**
 * @brief delete bitmap memory.
 */
int tlv_bit_heap_free(tlv_bit_heap_t* heap,void* p);

/**
 * @brief check heap is valid or not.
 */
int tlv_bit_heap_is_valid(tlv_bit_heap_t *heap);

#ifdef __cplusplus
};
#endif
#endif
