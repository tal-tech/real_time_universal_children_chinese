#ifndef TAL_STRUCT_TLV_STR_HASH_H_
#define TAL_STRUCT_TLV_STR_HASH_H_
#include "tlv/struct/tlv_define.h"
#include "tlv/struct/tlv_string.h"
#include "tlv_queue.h"
#include "tlv_heap.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct tlv_str_hash_node tlv_str_hash_node_t;
typedef struct tlv_str_hash tlv_str_hash_t;
#define tlv_str_hash_index(h,k,kb)  (hash_string_value_len(k,kb,(h)->nitem))
#define tlv_str_hash_add_s(h,k,v) tlv_str_hash_add(h,k,sizeof(k)-1,v)

struct tlv_str_hash_node
{
	tlv_queue_node_t n;
	tlv_string_t key;
	void* value;
};

struct tlv_str_hash
{
	tlv_heap_t *heap;
	tlv_queue_t **item;
	int nitem;
};

tlv_str_hash_t* tlv_str_hash_new(int n);

int tlv_str_hash_delete(tlv_str_hash_t *h);
int tlv_str_hash_reset(tlv_str_hash_t *h);

int tlv_str_hash_add_node(tlv_str_hash_t *h, char* key, int key_bytes, void *value, tlv_str_hash_node_t* n);
int tlv_str_hash_add(tlv_str_hash_t *h, char* key, int key_bytes, void *value);

tlv_str_hash_node_t* tlv_str_hash_find_node(tlv_str_hash_t *h, char* key,int key_bytes,uint32_t *rv);
void* tlv_str_hash_find(tlv_str_hash_t *h, char* key,int key_bytes);
int tlv_str_hash_findc(tlv_str_hash_t*h, char* k, int kb, tlv_cmp_handler_f cmp, void *user_data, void** v);

int tlv_str_hash_walk(tlv_str_hash_t* h, tlv_walk_handler_f handler, void* user_data);

int tlv_str_hash_elems(tlv_str_hash_t *h);
int tlv_str_hash_bytes(tlv_str_hash_t *h);

//-------------------------- test/example section ------------------
void tlv_str_hash_test_g();
#ifdef __cplusplus
};
#endif
#endif
