#ifndef WTK_CORE_tlv_loop_H_
#define WTK_CORE_tlv_loop_H_
#include "tlv/struct/tlv_define.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct tlv_loop tlv_loop_t;
#define tlv_loop_at(rb,i) ((rb)->r[((rb)->first+(i))%((rb)->nitem)])

struct tlv_loop
{
	void     **r;
	int      first;	//first
	uint32_t nitem;
	uint32_t used;	//length
};

tlv_loop_t* tlv_loop_new(uint32_t n);
int tlv_loop_delete(tlv_loop_t* r);
void tlv_loop_push(tlv_loop_t* r, void *d);
void* tlv_loop_pop(tlv_loop_t *r);
void tlv_loop_reset(tlv_loop_t *r);

#ifdef __cplusplus
};
#endif
#endif
