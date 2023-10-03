#ifndef TAL_STRUCT_TLV_CHAEBUF_H_
#define TAL_STRUCT_TLV_CHAEBUF_H_
#include "tlv/struct/tlv_define.h"
#include "tlv_string.h"
#include "tlv_str_encode.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct tlv_charbuf tlv_charbuf_t;
#define tlv_charbuf_push_s(b,s) tlv_charbuf_push(b,s,sizeof(s)-1)
#define tlv_charbuf_push_string(b,s) tlv_charbuf_push(b,s,strlen(s))
#define tlv_charbuf_reset(b) ((b)->pos=0)

#define tlv_charbuf_push_c(buf,b) \
{\
	    if(buf->alloc_size<=buf->pos) \
	    {\
	        tlv_charbuf_expand(buf,1); \
	    }\
	    buf->data[buf->pos++]=b; \
}

struct tlv_charbuf
{
    char  *data;
    int   pos;
    int   alloc_size;
    float scale;
};

tlv_charbuf_t* tlv_charbuf_new(int len, float scale);

int tlv_charbuf_delete(tlv_charbuf_t* b);

void tlv_charbuf_push(tlv_charbuf_t *s,const char *buf,int bytes);
void tlv_charbuf_push_f(tlv_charbuf_t *b,const char *fmt,...);

int tlv_charbuf_pop(tlv_charbuf_t *s, char* data, int bytes);

void tlv_charbuf_expand(tlv_charbuf_t *s, int bytes);
void tlv_charbuf_strip(tlv_charbuf_t *buf);
int tlv_charbuf_bytes(tlv_charbuf_t *b);

#ifdef __cplusplus
};
#endif
#endif
