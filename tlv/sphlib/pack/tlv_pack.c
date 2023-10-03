#include <stdlib.h>
#include "tlv_pack.h"

int tlv_packitem_delete(tlv_packitem_t *i)
{
	if(i->data.len>0)
	{
		tlv_free(i->data.data);
	}
	tlv_string_free(i->fn);
	tlv_free(i);
	return 0;
}

void tlv_packitem_print(tlv_packitem_t *i)
{
	print_data(i->fn->data,i->fn->len);
	printf("data: %d\n",i->data.len);
}

int tlv_packitem_get(tlv_packitem_t *i)
{
	int c;

	if(i->pos<i->data.len)
	{
		//for EOF check;
		c=(unsigned char)i->data.data[i->pos];
		++i->pos;
	}else
	{
		//tlv_log("%*.*s EOF.\n",i->fn->len,i->fn->len,i->fn->data);
		c=EOF;
	}
	//tlv_log("v[%d/%d]=%x,%x\n",i->pos,i->data.len,c,EOF);
	return c;
}

int tlv_packitem_get2(tlv_packitem_t* i,char *buf,int bytes)
{
	int len;

	len=i->data.len-i->pos;
	if(len<bytes)
	{
		return EOF;
	}
	memcpy(buf,i->data.data+i->pos,bytes);
	i->pos+=bytes;
	return bytes;
}

int tlv_packitem_unget(tlv_packitem_t *i,int c)
{
	if(i->pos>0){--i->pos;}
	return 0;
}


tlv_pack_t* tlv_pack_new()
{
	tlv_pack_t* pack;

	pack = (tlv_pack_t*)tlv_malloc(sizeof(*pack));
	tlv_queue_init(&(pack->list));
	pack->buf = tlv_charbuf_new(1024, 1.0);
	pack->fl  = NULL;

	return pack;
}

int tlv_pack_delete(tlv_pack_t *pack)
{
	tlv_queue_node_t *n,*p;
	tlv_packitem_t *i;

	for(n=pack->list.rear; n; n=p)
	{
		p = n->next;
		i = data_offset(n,tlv_packitem_t,q_n);
		tlv_packitem_delete(i);
	}

	if(pack->fl) {tlv_flist_delete(pack->fl);}
	tlv_charbuf_delete(pack->buf);
	tlv_free(pack);

	return 0;
}

int tlv_pack_read_scp(tlv_pack_t *pack, char *fn)
{
	pack->fl = tlv_flist_new(fn);

	return 0;
}

tlv_packitem_t* tlv_pack_new_item(tlv_pack_t *rb,tlv_string_t *fn,char *rfn)
{
	tlv_packitem_t* item;
	int len;
	char *data;
	int ret=-1;

	//printf("%d:%*.*s#\n",fn->len,fn->len,fn->len,fn->data);
	item=(tlv_packitem_t*)tlv_malloc(sizeof(*item));
	item->fn=tlv_string_dup_data(fn->data,fn->len);
	item->data.len=0;
	data=file_read_buf(rfn,&len);
	if(!data){goto end;}
	item->data.data=data;
	item->data.len=len;
	ret=0;
end:
	if(ret!=0)
	{
		tlv_packitem_delete(item);
		item=0;
	}
	return item;
}

int tlv_pack_read_dn(tlv_pack_t *rb,char *res_dn)
{
	tlv_queue_node_t *n;
	tlv_queue_t *q=&(rb->list);
	tlv_fitem_t *i;
	tlv_packitem_t *ri;
	tlv_charbuf_t *buf=rb->buf;
	int rlen=strlen(res_dn);
	int ret=-1;
	int add_sep;

	//tlv_log("%s\n",res_dn);
	if(rlen>0 && res_dn[rlen-1]!='/')
	{
		add_sep=1;
	}else
	{
		add_sep=0;
	}
	for(n=rb->fl->queue.rear;n;n=n->next)
	{
		i=data_offset(n,tlv_fitem_t,q_n);
		tlv_charbuf_reset(buf);
		tlv_charbuf_push(buf,res_dn,rlen);
		if(add_sep)
		{
			//add directory sep;
			tlv_charbuf_push_c(buf,'/');
		}
		tlv_charbuf_push(buf,i->str->data,i->str->len);
		tlv_charbuf_push_c(buf,0);
		//tlv_log("%s\n",buf->data);
		ri=tlv_pack_new_item(rb,i->str,buf->data);
		if(!ri){goto end;}
		tlv_queue_push(q,&(ri->q_n));
		//hdr_size+=RBIN_INT_SIZE+i->str->len+RBIN_INT_SIZE;
	}
	//tlv_log("%d,hs=%d\n",q->length,hdr_size);
	ret=0;
end:
	return ret;
}

#define tlv_pack_INT_SIZE 10

void tlv_pack_write_int_f(tlv_pack_t *rb,int v,FILE* f)
{
	char buf[tlv_pack_INT_SIZE]={0,0};

	sprintf(buf,"%d",v);
	fwrite(buf,sizeof(buf),1,f);
}

int tlv_pack_read_int_f(tlv_pack_t *rb,FILE* f,int *v)
{
	char buf[tlv_pack_INT_SIZE];
	int ret;

	ret=fread(buf,sizeof(buf),1,f);
	if(ret!=1){ret=-1;goto end;}
	*v=atoi(buf);
	ret=0;
end:
	return ret;
}

void tlv_file_write_reverse(FILE* f,char *data,int len)
{
	int i;
	unsigned char c;

	for(i=0;i<len;++i)
	{
		c=~((unsigned char)data[i]);
		fwrite(&c,1,1,f);
	}
}


void tlv_pack_write_data2(tlv_pack_t *rb,FILE* f,char *data,int len)
{
	int i;
	unsigned char c;

	//fwrite(data,len,1,f);
	for(i=0;i<len;++i)
	{
		//c=255-((unsigned char)data[i]);
		c=~((unsigned char)data[i]);
		fwrite(&c,1,1,f);
	}
}

int tlv_pack_read_data2(tlv_pack_t *rb,FILE* f,unsigned char *data,int len)
{
	unsigned char c;
	int i,ret=0;

	//fwrite(data,len,1,f);
	for(i=0;i<len;++i)
	{
		ret=fread(&c,1,1,f);
		if(ret!=1){ret=-1;goto end;}
		data[i]=~(c);
	}
	ret=0;
end:
	return ret;
}

void tlv_pack_reverse_data(unsigned char *p,int len)
{
	unsigned char *e;

	e=p+len;
	while(p<e)
	{
		*p=~*p;
		++p;
	}
}

void tlv_pack_write_data(tlv_pack_t *rb,FILE* f,char *data,int len)
{
	tlv_pack_reverse_data((unsigned char *)data,len);
	//skip error check;
	fwrite(data,len,1,f);
	tlv_pack_reverse_data((unsigned char *)data,len);
}

int tlv_pack_read_data(tlv_pack_t *rb,FILE* f,unsigned char *data,int len)
{
	int ret;

	ret = fread(data,1,len,f);
	if(ret!=len){ret=-1;goto end;}
	tlv_pack_reverse_data(data,len);
	ret=0;
end:
	return ret;
}

int tlv_pack_write(tlv_pack_t *pack, char *res_dn, char *bin)
{
	int ret=-1;
	tlv_queue_node_t *n;
	tlv_queue_t *q=&(pack->list);
	tlv_packitem_t *i;
	FILE* f=0;

	f = fopen(bin,"wb");
	if(!f) {goto end;}
	ret = tlv_pack_read_dn(pack, res_dn);
	if(ret!=0){goto end;}
	tlv_pack_write_int_f(pack, q->length, f);
	for(n=q->rear;n;n=n->next)
	{
		i = data_offset(n,tlv_packitem_t, q_n);
		tlv_pack_write_int_f(pack, i->fn->len, f);
		tlv_pack_write_data(pack, f, i->fn->data, i->fn->len);
		tlv_pack_write_int_f(pack, i->data.len, f);
		tlv_pack_write_data(pack, f, i->data.data, i->data.len);
	}
	ret=0;

end:
	if(f) {fclose(f);}

	return ret;
}


static int tlv_bin_read_bin(tlv_pack_t *rb,FILE* f)
{
	tlv_queue_t *q=&(rb->list);
	tlv_packitem_t *item;
	int c,v,ret;
	int i;

	ret=tlv_pack_read_int_f(rb,f,&c);
	if(ret!=0){goto end;}
	for(i=0;i<c;++i)
	{
		ret=tlv_pack_read_int_f(rb,f,&v);
		if(ret!=0){goto end;}
		item=(tlv_packitem_t*)tlv_malloc(sizeof(*item));
		item->data.len=0;
		item->fn=tlv_string_new(v);
		ret=tlv_pack_read_data(rb,f,(unsigned char*)item->fn->data,v);
		if(ret!=0){goto end;}
		ret=tlv_pack_read_int_f(rb,f,&v);
		if(ret!=0){goto end;}
		item->data.len=v;
		//tlv_log("v=%d\n",v);
		if(v==0)
		{
			item->data.data=0;
		}else
		{
			item->data.data=(char*)tlv_malloc(v);
		}
		ret=tlv_pack_read_data(rb,f,(unsigned char*)item->data.data,v);
		if(ret!=0){goto end;}
		//tlv_log("%.*s: %d\n",item->fn->len,item->fn->data,item->data.len);
		//printf("%d: %*.*s\n",item->fn->len,item->fn->len,item->fn->len,item->fn->data);
		tlv_queue_push(q,&(item->q_n));
	}
	ret=0;
end:
	return ret;
}

int tlv_pack_read(tlv_pack_t *pack, char *bin)
{
	int ret=-1;
	FILE* f;

	f = fopen(bin,"rb");
	if(!f)
	{
		tlv_log("%s not exist.\n",bin);
		goto end;
	}
	ret=tlv_bin_read_bin(pack, f);
end:
	if(f){fclose(f);}

	return ret;
}

int tlv_pack_write_item(tlv_pack_t *rb,tlv_packitem_t *i,char *dn,int len)
{
	tlv_charbuf_t *buf=rb->buf;

	tlv_charbuf_reset(buf);
	tlv_charbuf_push(buf,dn,len);
	tlv_charbuf_push_s(buf,"/");
	tlv_charbuf_push(buf,i->fn->data,i->fn->len);
	tlv_charbuf_push_c(buf,0);
	//printf("%s\n",buf->data);
	return file_write_buf(buf->data,i->data.data,i->data.len);
}

int tlv_pack_extract(tlv_pack_t *pack, char *dn)
{
	int ret=-1;
	tlv_queue_node_t *n;
	tlv_queue_t *q=&(pack->list);
	tlv_packitem_t *i;
	int len = strlen(dn);

	if(tlv_file_exist(dn)==0)
	{
		tlv_mkdir_p(dn, DIR_SEP, 1);
	}
	for(n=q->rear;n;n=n->next)
	{
		i   = data_offset(n, tlv_packitem_t, q_n);
		ret = tlv_pack_write_item(pack, i, dn, len);
		if(ret!=0)
		{
			tlv_log("write %*.*s failed.\n",i->fn->len,i->fn->len,i->fn->data);
			goto end;
		}
	}
end:

	return ret;
}

tlv_packitem_t* tlv_pack_find(tlv_pack_t *pack, char *data, int len)
{
	tlv_queue_node_t *n;
	tlv_queue_t *q=&(pack->list);
	tlv_packitem_t *i=0,*x;

	for(n=q->rear; n; n=n->next)
	{
		x = data_offset(n,tlv_packitem_t,q_n);
		if(tlv_string_cmp(x->fn,data,len)==0)
		{
			i=x;
			break;
		}
	}

	return i;
}

void tlv_strfile_init_rbin(tlv_strfile_t* s,tlv_packitem_t *i)
{
	tlv_strfile_init(s);
	i->pos=0;
	s->data=i;
	s->get=(tlv_strfile_get_handler_t)tlv_packitem_get;
	s->unget=(tlv_strfile_unget_handler_t)tlv_packitem_unget;
	s->get_str=(tlv_strfile_get_str_f)tlv_packitem_get2;
	s->swap=tlv_is_little_endian();
}


int tlv_pack_load_file(tlv_pack_t *pack, void *data, tlv_strfile_load_handler_t loader,char *fn)
{
	tlv_strfile_t s,*ps=&s;
	tlv_packitem_t *i;
	int ret=-1;

	i = tlv_pack_find(pack, fn, strlen(fn));
	if(!i){goto end;}

	tlv_strfile_init_rbin(ps,i);
	ret = loader(data,ps);
end:

	return ret;
}
