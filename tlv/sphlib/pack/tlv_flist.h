#ifndef TAL_SPHLIB_TLV_PACK_FLIST_H_
#define TAL_SPHLIB_TLV_PACK_FLIST_H_
#include "tlv/struct/tlv_string.h"
#include "tlv/struct/tlv_heap.h"
#include "tlv/struct/tlv_queue.h"
#include "tlv/struct/tlv_charbuf.h"
#include "tlv/struct/tlv_sys.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct tlv_flist tlv_flist_t;

typedef enum
{
	TLV_FITEM_START,
	TLV_FITEM_APPEND,
}tlv_fitem_state_t;

typedef struct
{
	tlv_queue_node_t q_n;
	tlv_string_t *str;
}tlv_fitem_t;

struct tlv_flist
{
	tlv_queue_t queue;
	tlv_heap_t *heap;
	tlv_charbuf_t *buf;
	tlv_fitem_state_t state;
};

tlv_flist_t* tlv_flist_new(char *fn);
int tlv_flist_delete(tlv_flist_t *fl);

//typedef void(*tlv_flist_notify_f)(void *ths,char *fn);
//void tlv_flist_process(char *fn,void *ths,tlv_flist_notify_f notify);
#ifdef __cplusplus
};
#endif
#endif
