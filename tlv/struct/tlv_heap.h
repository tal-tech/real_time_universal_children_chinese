
#ifndef TAL_STRUCT_TLV_HEAP_H_
#define TAL_STRUCT_TLV_HEAP_H_
#include "tlv/struct/tlv_define.h"
#include "tlv_queue.h"
#include "tlv_string.h"
#ifdef __cplusplus
extern "C" {
#endif

#define tlv_heap_ALIGNMENT       16
#define TLV_MAX_ALLOC_FROM_HEAP  (4096 - 1)
#define tlv_heap_dup_string_s(h,s)  tlv_heap_dup_string(h,s,sizeof(s)-1)
typedef struct tlv_heap tlv_heap_t;
typedef struct tlv_heap_block tlv_heap_block_t;
typedef struct tlv_heap_large tlv_heap_large_t;

struct tlv_heap_block
{
	uint8_t 	*first;
	uint8_t 	*last;
	uint8_t 	*end;
	tlv_heap_block_t *next;
	unsigned int failed;
};

struct tlv_heap_large
{
	struct tlv_heap_large *next;
	void *data;
	int size;
};

struct tlv_heap
{
	tlv_heap_block_t *first;
	size_t 	max;
	size_t	size;
	size_t align;
	tlv_heap_block_t *current;
	tlv_heap_large_t *large;
};

/**
 *	@brief create heap;
 */
tlv_heap_t* tlv_heap_new(size_t size);

/**
 *  @brief align heap with specify alignment;
 */
tlv_heap_t* tlv_heap_new2(size_t size,int align_size);

/**
 * @brief bytes of heap occupied;
 */
int tlv_heap_bytes(tlv_heap_t *heap);

/**
 * @brief release heap memory.
 */
int tlv_heap_delete(tlv_heap_t* heap);

/**
 * @brief reset heap,free all large memory.
 */
int tlv_heap_reset(tlv_heap_t* heap);

/**
 * @brief reset heap,and all memory is not freed;
 */
int tlv_heap_reset2(tlv_heap_t* heap);

/**
 * @brief allocate memory with size bytes from heap.
 */
void* tlv_heap_malloc(tlv_heap_t* heap,size_t size);

/**
 * @brief allocate from heap and set zero.
 */
void* tlv_heap_zalloc(tlv_heap_t* heap,size_t size);

/**
 * @brief duplicate tlv_string_t;
 */
tlv_string_t* tlv_heap_dup_string(tlv_heap_t *h,char *s,int sl);

/**
 * @return string with 0 end.
 */
tlv_string_t* tlv_heap_dup_string2(tlv_heap_t *h,char *s,int sl);

/**
 * @brief return string will '\0' end.
 */
char *tlv_heap_dup_str(tlv_heap_t *heap,char* s);

/**
 * @brief return string will '\0 end with string;
 */
char* tlv_heap_dup_str2(tlv_heap_t *heap,char *data,int len);

/**
 * @brief duplicate data;
 */
char* tlv_heap_dup_data(tlv_heap_t *h,const char *s,int l);

/**
 * @brief print heap;
 */
void tlv_heap_print(tlv_heap_t *heap);

/**
 * @brief fill string with data;
 */
void tlv_heap_fill_string(tlv_heap_t *heap,tlv_string_t *str,char *data,int bytes);

void tlv_heap_add_large(tlv_heap_t *heap,char *p,int size);

#ifdef __cplusplus
};
#endif
#endif
