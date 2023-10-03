#ifndef TAL_SPHLIB_CFG_TLV_CFG_QUEUE_H_
#define TAL_SPHLIB_CFG_TLV_CFG_QUEUE_H_
#include "tlv/struct/tlv_define.h"
#include "tlv/struct/tlv_queue.h"
#include "tlv/struct/tlv_string.h"
#include "tlv/struct/tlv_charbuf.h"
#include "tlv/struct/tlv_heap.h"
#include "tlv/struct/tlv_array.h"
#ifdef __cplusplus
extern "C" {
#endif
struct tlv_part_cfg;
typedef struct tlv_cfg_item tlv_cfg_item_t;
typedef struct tlv_cfg_queue tlv_cfg_queue_t;

#define tlv_cfg_item_is_lc(cfg) ((cfg)->type==TLV_CFG_LC)
#define tlv_cfg_item_is_array(cfg) ((cfg)->type==TLV_CFG_ARRAY)

typedef enum
{
	TLV_CFG_STRING=0,
	TLV_CFG_ARRAY,
	TLV_CFG_LC,
}tlv_cfg_type_t;

struct tlv_cfg_item
{
	tlv_queue_node_t n;
	tlv_cfg_type_t type;
	tlv_string_t *key;
	union
	{
		tlv_string_t *str;
		tlv_array_t *array;
		struct tlv_part_cfg *cfg;
	}value;
};

struct tlv_cfg_queue
{
	tlv_queue_t queue;
	tlv_heap_t *heap;
};

tlv_cfg_queue_t* tlv_cfg_queue_new_h(tlv_heap_t *h);
int tlv_cfg_queue_add(tlv_cfg_queue_t *cfg, tlv_cfg_item_t *item);
int tlv_cfg_queue_add_string(tlv_cfg_queue_t *cfg, char *k, int klen, char *v, int vlen);
int tlv_cfg_queue_add_lc(tlv_cfg_queue_t *cfg, char *k, int klen, struct tlv_part_cfg *lc);
int tlv_cfg_queue_add_array(tlv_cfg_queue_t *cfg, char *k, int klen, tlv_array_t *a);
tlv_cfg_item_t* tlv_cfg_queue_find(tlv_cfg_queue_t *cfg, char *k, int klen);
void tlv_cfg_queue_remove(tlv_cfg_queue_t *cfg, tlv_cfg_item_t *item);

void tlv_cfg_item_print(tlv_cfg_item_t *item);
void tlv_cfg_queue_print(tlv_cfg_queue_t *cfg);
void tlv_cfg_queue_to_string(tlv_cfg_queue_t *cfg, tlv_charbuf_t *buf);
#ifdef __cplusplus
};
#endif
#endif
