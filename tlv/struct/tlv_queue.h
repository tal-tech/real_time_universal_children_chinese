#ifndef TLV_STRUCT_TLV_QUEUE_H_
#define TLV_STRUCT_TLV_QUEUE_H_
#include "tlv/struct/tlv_define.h"
#include "tlv/struct/tlv_string.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
struct tlv_heap;
typedef struct tlv_queue_node tlv_queue_node_t;
typedef struct tlv_queue tlv_queue_t;

#define tlv_queue_node_data_offset(q,type,link) (type*)((q)>0 ? (void*)((char*)q-offsetof(type,link))  : 0)
#define tlv_queue_node_data_offset2(q,offsest) ((q)>0 ? ((void*)((char*)q - offset)) : 0)

struct tlv_queue_node
{
	tlv_queue_node_t* prev;
	tlv_queue_node_t* next;
};

struct tlv_queue
{
	unsigned int length;
	void         *data;
	tlv_queue_node_t *front;  /* push */
	tlv_queue_node_t *rear;   /* pop */
};

void tlv_queue_init(tlv_queue_t *q);

/**
 * @brief push node
 */
int tlv_queue_push(tlv_queue_t *q, tlv_queue_node_t *n);
int tlv_queue_push_front(tlv_queue_t *q, tlv_queue_node_t *n);


int tlv_queue_remove(tlv_queue_t *q,tlv_queue_node_t *n);

tlv_queue_node_t* tlv_queue_pop(tlv_queue_t *q);

void tlv_queue_insert(tlv_queue_t *q, tlv_queue_node_t *n, tlv_cmp_handler_f cmp);
/**
 * @brief inser n2  afer n
 */
void tlv_queue_insert_to(tlv_queue_t *q, tlv_queue_node_t *n, tlv_queue_node_t* n2);


void* tlv_queue_find(tlv_queue_t *q, int offset, tlv_cmp_handler_f cmp, void *user_data);

/**
 * @brief move n to end
 */
void tlv_queue_touch_node(tlv_queue_t *q, tlv_queue_node_t *n);

int tlv_queue_walk(tlv_queue_t *q, int offset, tlv_walk_handler_f walk, void *user_data);

void tlv_queue_link(tlv_queue_t *dst, tlv_queue_t *src);

void tlv_queue_print(tlv_queue_t *q, int offset, tlv_print_handler_f print);

#ifdef __cplusplus
};
#endif
#endif
