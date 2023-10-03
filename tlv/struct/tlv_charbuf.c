#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include "tlv_charbuf.h"
#include "tlv_sys.h"

tlv_charbuf_t* tlv_charbuf_new(int len,float scale)
{
    tlv_charbuf_t *b;
    char *data;

    data = (char*)tlv_malloc(len);
    if(!data){b=0;goto end;}
    b         = (tlv_charbuf_t*)tlv_malloc(sizeof(*b));
    b->data   = data;
    b->alloc_size = len;
    b->pos    = 0;
    b->scale  = 1.0 + scale;
end:

    return b;
}

int tlv_charbuf_delete(tlv_charbuf_t* b)
{
    tlv_free(b->data);
    tlv_free(b);

    return 0;
}

void tlv_charbuf_expand(tlv_charbuf_t *s, int bytes)
{
    int left,alloc;
    char *p;
    int t1, t2;

    left=s->alloc_size - s->pos;
    if(bytes>left)
    {
    	t1        = s->alloc_size * s->scale;
    	t2        = s->pos+bytes;
        alloc     = max(t1, t2);
        p         = s->data;
        s->data   = (char*)tlv_malloc(alloc);
        s->alloc_size = alloc;
        memcpy(s->data, p, s->pos);
        tlv_free(p);
    }

    return;
}

void tlv_charbuf_push(tlv_charbuf_t *s, const char *buf, int bytes)
{
    if(!buf || bytes<0){return;}

    if(bytes > s->alloc_size-s->pos)
    {
        tlv_charbuf_expand(s,bytes);
    }

    memcpy(s->data+s->pos, buf, bytes);
    s->pos += bytes;

    return;
}

int tlv_charbuf_bytes(tlv_charbuf_t *b)
{
	return sizeof(*b)+b->alloc_size;
}


void tlv_charbuf_push_f(tlv_charbuf_t *b,const char *fmt,...)
{
	char buf[4096]={0};
	va_list ap;
	int n;

	va_start(ap,fmt);
	n=vsprintf(buf,fmt,ap);
	tlv_charbuf_push(b,buf,n);
	va_end(ap);
}

int tlv_charbuf_pop(tlv_charbuf_t *s,char* data,int bytes)
{
	int ret;

	if(s->pos<bytes){ret=-1;goto end;}
	if(data)
	{
		memcpy(data,s->data,bytes);
	}
	s->pos-=bytes;
	memmove(s->data,&(s->data[bytes]),s->pos);
	ret=0;
end:
	return ret;
}


void tlv_charbuf_strip(tlv_charbuf_t *buf)
{
	int i,n;

	for(i=0,n=0;i<buf->pos;++i)
	{
		if(isspace(buf->data[i]))
		{
			++n;
		}else
		{
			break;
		}
	}

	if(n>0)
	{
		tlv_charbuf_pop(buf,0,n);
	}
	for(i=buf->pos-1;i>=0;--i)
	{
		if(isspace(buf->data[i]))
		{
			--buf->pos;
		}else
		{
			break;
		}
	}
}
