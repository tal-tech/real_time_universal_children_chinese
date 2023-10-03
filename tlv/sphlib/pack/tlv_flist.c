#include <ctype.h>
#include "tlv_flist.h"
void tlv_flist_feed_start(tlv_flist_t *fl,signed char c);
void tlv_flist_feed_append(tlv_flist_t *fl,signed char c);
void tlv_flist_feed(tlv_flist_t *fl,char *data,int len);

tlv_flist_t* tlv_flist_new(char *fn)
{
	tlv_flist_t *fl=0;
	char *data;
	int len;

	data=file_read_buf(fn,&len);
	if(!data){goto end;}
	fl=(tlv_flist_t*)tlv_malloc(sizeof(*fl));
	fl->heap=tlv_heap_new(4096);
	fl->buf=tlv_charbuf_new(1024,1);
	tlv_queue_init(&(fl->queue));
	tlv_flist_feed(fl,data,len);
end:
	if(data){free(data);}
	return fl;
}

int tlv_flist_delete(tlv_flist_t *fl)
{
	tlv_charbuf_delete(fl->buf);
	tlv_heap_delete(fl->heap);
	tlv_free(fl);
	return 0;
}

tlv_fitem_t* tlv_flist_append(tlv_flist_t *fl,char *data,int len)
{
	tlv_fitem_t *item;

	//tlv_log("%*.*s\n",len,len,data);
	item=(tlv_fitem_t*)tlv_heap_malloc(fl->heap,sizeof(*item));
	item->str=tlv_heap_dup_string(fl->heap,data,len);
	tlv_queue_push(&(fl->queue),&(item->q_n));
	return item;
}

void tlv_flist_feed_start(tlv_flist_t *fl,signed char c)
{
	if(!isspace(c) && c!=EOF)
	{
		fl->state=TLV_FITEM_APPEND;
		tlv_charbuf_reset(fl->buf);
		tlv_flist_feed_append(fl,c);
	}
}

void tlv_flist_feed_append(tlv_flist_t *fl,signed char c)
{
	tlv_fitem_t *item;

	if(c=='\n' || c==EOF)
	{
		tlv_charbuf_push_c(fl->buf,0);
		item=tlv_flist_append(fl,fl->buf->data,fl->buf->pos);
		--item->str->len;
		fl->state=TLV_FITEM_START;
	}else
	{
		tlv_charbuf_push_c(fl->buf,c);
	}
}

void tlv_flist_feed_c(tlv_flist_t *fl,char c)
{
	switch(fl->state)
	{
	case TLV_FITEM_START:
		tlv_flist_feed_start(fl,c);
		break;
	case TLV_FITEM_APPEND:
		tlv_flist_feed_append(fl,c);
		break;
	}
}

void tlv_flist_feed(tlv_flist_t *fl,char *data,int len)
{
	char *s,*e;

	s=data;e=data+len;
	fl->state=TLV_FITEM_START;
	while(s<e)
	{
		tlv_flist_feed_c(fl,*s);
		++s;
	}
	tlv_flist_feed_c(fl,EOF);
}


//void tlv_flist_process(char *fn,void *ths,tlv_flist_notify_f notify)
//{
//	tlv_flist_t *fl;
//	tlv_queue_node_t *qn;
//	tlv_fitem_t *item;
//
//	fl=tlv_flist_new(fn);
//	for(qn=fl->queue.rear;qn;qn=qn->next)
//	{
//		item=data_offset2(qn,tlv_fitem_t,q_n);
//		notify(ths,item->str->data);
//	}
//	tlv_flist_delete(fl);
//}
