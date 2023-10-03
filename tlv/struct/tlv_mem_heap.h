/*
 * tlv_mem_heap.h
 *
 *  Created on: May 14, 2018
 *      Author: jfyuan
 *
 */

#ifndef TLV_MEM_HEAP_H_
#define TLV_MEM_HEAP_H_
#include "tlv_define.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char* ByteP;
typedef void* Ptr;

typedef enum
{
	MHEAP,       /* 申请固定大小单元的内存 */
	MSTAK,       /* 内存池，想用内存就从这个内存池里申请 */
	CHEAP        /* malloc，内存会对齐 */
}tlv_memheap_type_t;

typedef struct _Block *BlockP;
typedef struct _Block
{
	size_t numFree;
	size_t firstFree;
	size_t numElem;
	ByteP  used;
	Ptr    data;
	BlockP next;
}Block;

typedef struct
{
	char *name;
	tlv_memheap_type_t type;
	float growf;
	size_t elemSize;
	size_t minElem;
	size_t maxElem;
	size_t curElem;
	size_t totUsed;
	size_t totAlloc;
	BlockP heap;
	unsigned char protectStk:1;
}tlv_mem_heap_t;

size_t tlv_mem_round(size_t size);
/**
 * @brief Create a memory heap x for elements of size elemSize and numElem in first block.
 *        If type is MSTAK or CHEAP then elemSize should be 1.
 */
void tlv_mem_heap_create(tlv_mem_heap_t *x, char* name, tlv_memheap_type_t type, size_t elemSize, float growf, size_t numElem, size_t maxElem);
void tlv_mem_heap_reset(tlv_mem_heap_t* x);
void tlv_mem_heap_delete(tlv_mem_heap_t* x);

/**
 * @brief  申请一小块内存
 */
Ptr tlv_mem_new(tlv_mem_heap_t* x, size_t size);
/**
 * @brief  申请一小块内存， 并做初始化。
 */
Ptr tlv_mem_cnew(tlv_mem_heap_t* x, size_t size);
/**
 * @brief  释放这一小块内存
 */
void tlv_mem_dispose(tlv_mem_heap_t* x, Ptr p);

#ifdef __cplusplus
};
#endif

#endif /* TLV_MEM_HEAP_H_ */
