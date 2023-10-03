#include "tlv_pack2.h"
#include <errno.h>
#include <string.h>

tlv_pack2_t* tlv_pack2_new()
{
	tlv_pack2_t *pack;

	pack       = (tlv_pack2_t*)tlv_malloc(sizeof(tlv_pack2_t));
	pack->heap = tlv_heap_new(4096);
	pack->f    = NULL;
	pack->fn   = NULL;
	pack->buf  = tlv_charbuf_new(4096,1);
	tlv_queue_init(&(pack->list));

	return pack;
}

void tlv_pack2_delete(tlv_pack2_t *pack)
{
	tlv_charbuf_delete(pack->buf);
	if(pack->f)
	{
		fclose(pack->f);
	}
	tlv_heap_delete(pack->heap);
	tlv_free(pack);
}


int tlv_pack2_add(tlv_pack2_t *pack, tlv_string_t *name, char *realname)
{
	char *data;
	int len;
	int ret=-1;

	data = file_read_buf(realname, &len);
	if(!data) {goto end;}
	tlv_heap_add_large(pack->heap, data, len);
	tlv_pack2_add2(pack, name, data, len);
	ret=0;
end:

	return ret;
}

tlv_pack2_item_t* tlv_pack2_new_item(tlv_pack2_t *rb)
{
	tlv_pack2_item_t *item;

	item=(tlv_pack2_item_t*)tlv_heap_malloc(rb->heap,sizeof(tlv_pack2_item_t));
	item->fn=NULL;
	item->data=NULL;
	item->pos=-1;
	item->len=item->seek_pos=0;
	item->buf_pos=0;
	item->reverse=0;
	item->rb=rb;
	return item;
}

void tlv_pack2_add2(tlv_pack2_t *rb,tlv_string_t *name,char *data,int len)
{
	tlv_pack2_item_t *item;
	tlv_heap_t *heap=rb->heap;
	tlv_charbuf_t *buf=rb->buf;

	tlv_charbuf_reset(buf);
	tlv_charbuf_push_s(buf,"./");
	tlv_charbuf_push(buf,name->data,name->len);
	//tlv_log("[%.*s]\n",name->len,name->data);
	item=tlv_pack2_new_item(rb);
	//item->fn=tlv_heap_dup_string(heap,name->data,name->len);
	item->fn=tlv_heap_dup_string(heap,buf->data,buf->pos);
	item->data=(tlv_string_t*)tlv_heap_malloc(heap,sizeof(tlv_string_t));
	if(1)
	{
		item->reverse=1;
	}

	tlv_string_set((item->data),data,len);
	tlv_queue_push(&(rb->list),&(item->q_n));
}

int tlv_pack2_write_data(FILE* f,char *data,int len)
{
	int ret;

	tlv_pack_reverse_data((unsigned char *)data,len);
	//skip error check;
	ret=fwrite(data,len,1,f);
	tlv_pack_reverse_data((unsigned char *)data,len);
	return ret==1?0:-1;
}

int tlv_pack2_write(tlv_pack2_t *rb,char *fn)
{
	tlv_queue_node_t *qn;
	tlv_pack2_item_t *item;
	int ret=-1;
	FILE *f;
	int vi;
	int pos;
	char buf[TPACK_BACKUP_LEN];

	//tlv_log("write %s\n",fn);
	f=fopen(fn,"wb");
	if(!f){goto end;}

	/* placeholder: jfyuan 20180509 */
	ret = fwrite(buf, 1, TPACK_BACKUP_LEN, f);
	if(ret != TPACK_BACKUP_LEN) { ret=-1; goto end; }
	pos = TPACK_BACKUP_LEN;

	vi=rb->list.length;
	ret=fwrite(&vi,1,4,f);
	if(ret!=4){ret=-1;goto end;}
	pos += 4;
	for(qn=rb->list.rear;qn;qn=qn->next)
	{
		item=data_offset(qn,tlv_pack2_item_t,q_n);
		pos+=4+item->fn->len+4+4;
	}
	for(qn=rb->list.rear;qn;qn=qn->next)
	{
		item=data_offset(qn,tlv_pack2_item_t,q_n);
		vi=item->fn->len;
		item->pos=pos;
		pos+=item->data->len;
		ret=fwrite(&(vi),1,4,f);
		if(ret!=4){ret=-1;goto end;}
		ret=tlv_pack2_write_data(f,item->fn->data,item->fn->len);
		//ret=fwrite(item->fn->data,item->fn->len,1,f);
		//tlv_log("[%.*s]\n",item->fn->len,item->fn->data);
		if(ret!=0){ret=-1;goto end;}
		vi=item->pos;
		ret=fwrite(&(vi),1,4,f);
		if(ret!=4){ret=-1;goto end;}
		vi=item->data->len;
		ret=fwrite(&(vi),1,4,f);
		if(ret!=4){ret=-1;goto end;}
	}
	for(qn=rb->list.rear;qn;qn=qn->next)
	{
		item=data_offset(qn,tlv_pack2_item_t,q_n);
		if(item->data && item->data->len>0)
		{
			/*
			tlv_log("[%.*s] reverse=%d,fpos=%d,pos=%d,len=%d\n",item->fn->len,item->fn->data,
					item->reverse,(int)ftell(f),item->pos,item->data->len);
				*/
			//print_data(item->data->data,10);
			if(item->reverse)
			{
				ret=tlv_pack2_write_data(f,item->data->data,item->data->len);
				//ret=fwrite(item->fn->data,item->fn->len,1,f);
				//tlv_log("[%.*s]\n",item->fn->len,item->fn->data);
				if(ret!=0){ret=-1;goto end;}
			}else
			{
				ret=fwrite(item->data->data,item->data->len,1,f);
				if(ret!=1){ret=-1;goto end;}
			}
		}
	}
	ret=0;
end:
	//tlv_log("ret=%d\n",ret);
	if(f)
	{
		fclose(f);
	}
	return ret;
}

void tlv_pack2_load_all(tlv_pack2_t *rb)
{
	tlv_queue_node_t *qn;
	tlv_pack2_item_t *item;

	for(qn=rb->list.rear;qn;qn=qn->next)
	{
		item=data_offset(qn,tlv_pack2_item_t,q_n);
		if(!item->data)
		{
			tlv_pack2_load_item(rb,item,1);
		}
	}
}

int tlv_pack2_read(tlv_pack2_t *pack, char *fn)
{
	int ret;
	tlv_pack2_item_t *item;
	tlv_heap_t *heap;
	FILE *f;
	int i;
	int n,vi;

	heap = pack->heap;
	f    = fopen(fn, "rb");
	if(!f) {ret=-1; goto end;}

	/* seek backup area: jfyuan20180509 */
	ret = fseek(f, TPACK_BACKUP_LEN, SEEK_SET);
	if(0 != ret) { goto end; }

	ret = fread((char*)&(n), 4, 1, f);
	if(ret!=1) {ret=-1; goto end;}
	for(i=0; i<n; ++i)
	{
		item = tlv_pack2_new_item(pack);
		ret  = fread((char*)&(vi),4,1,f);
		if(ret!=1) {ret=-1; goto end;}

		item->fn = tlv_heap_dup_string(heap,0,vi);
		ret = fread(item->fn->data, item->fn->len, 1, f);
		if(ret!=1) {ret=-1; goto end;}
		tlv_pack_reverse_data((unsigned char*)item->fn->data, item->fn->len);
//		if(item->fn->data[item->fn->len-1]=='r')
//		{
//			item->reverse=1;
//		}else
//		{
//			item->reverse=0;
//		}
		item->reverse = 1;
		ret = fread((char*)&(vi),4,1,f);
		if(ret!=1){ret=-1;goto end;}
		item->pos = vi;
		ret = fread((char*)&(vi),4,1,f);
		if(ret!=1) {ret=-1;goto end;}
		item->len = vi;
		//tlv_log("[%.*s]=%d/%d\n",item->fn->len,item->fn->data,item->pos,item->len);
		item->data = NULL;
		tlv_queue_push(&(pack->list), &(item->q_n));
	}

	pack->fn = tlv_heap_dup_str(pack->heap, fn);
	ret=0;

end:
	if(ret!=0)
	{
		if(f)
		{
			fclose(f);
		}
	}else
	{
		pack->f=f;
	}

	return ret;
}

void tlv_pack2_print(tlv_pack2_t *pack)
{
	tlv_queue_node_t *qn;
	tlv_pack2_item_t *item;

	for(qn=pack->list.rear;qn;qn=qn->next)
	{
		item=data_offset(qn,tlv_pack2_item_t,q_n);
		tlv_log("[%.*s],pos=%d,len=%d\n",item->fn->len,item->fn->data,item->pos,item->len);
	}
}

tlv_pack2_item_t* tlv_pack2_get(tlv_pack2_t *pack, char *name, int len)
{
	tlv_queue_node_t *qn;
	tlv_pack2_item_t *item;

	for(qn=pack->list.rear; qn; qn=qn->next)
	{
		item = data_offset(qn, tlv_pack2_item_t, q_n);
		if(tlv_string_cmp(item->fn,name,len)==0)
		{
			return item;
		}
	}

	return NULL;
}

tlv_pack2_item_t* tlv_pack2_get2(tlv_pack2_t *pack, char *name, int len)
{
	int ret;
	tlv_pack2_item_t *item;

	item = tlv_pack2_get(pack, name, len);
	if(item && !item->data)
	{
		ret = tlv_pack2_load_item(pack, item, 1);
		if(ret!=0)
		{
			return NULL;
		}
	}

	return item;
}

void tlv_pack2_item_release(tlv_pack2_item_t *item)
{
	if(item->data)
	{
		tlv_free(item->data);
		item->data=NULL;
	}
}

int tlv_pack2_load_item(tlv_pack2_t *pack, tlv_pack2_item_t *item, int use_heap)
{
	int ret;
	FILE *f = pack->f;

	//tlv_log("[%.*s] pos=%d r=%d\n",item->fn->len,item->fn->data,item->pos,item->reverse);
	ret = fseek(f,item->pos,SEEK_SET);
	if(ret!=0)
	{
		goto end;
	}
	if(use_heap)
	{
		item->data=tlv_heap_dup_string(pack->heap, 0, item->len);
	}else
	{
		item->data = tlv_string_new(item->len);
	}
	ret = fread(item->data->data,item->len,1,f);
	if(ret!=1){ret=-1;goto end;}
	//print_data(item->data->data,10);
	if(item->reverse)
	{
		tlv_pack_reverse_data((unsigned char*)item->data->data,item->data->len);
	}
	ret=0;
end:

	return ret;
}

int tlv_pack2_extract(tlv_pack2_t *rb,char *dn)
{
	tlv_queue_node_t *qn;
	tlv_pack2_item_t *item;
	int ret=-1;
	tlv_charbuf_t *buf;

	//tlv_pack2_print(rb);
	if(tlv_file_exist(dn)==0)
	{
		tlv_mkdir_p(dn,DIR_SEP,1);
	}
	buf=tlv_charbuf_new(1024,1);
	for(qn=rb->list.rear;qn;qn=qn->next)
	{
		item=data_offset(qn,tlv_pack2_item_t,q_n);
		if(!item->data) //item->len!=item->data->len)
		{
			ret=tlv_pack2_load_item(rb,item,1);
			if(ret!=0)
			{
				tlv_log("load [%.*s] failed\n",item->fn->len,item->fn->data);
				goto end;
			}
		}
		tlv_charbuf_reset(buf);
		tlv_charbuf_push(buf,dn,strlen(dn));
		tlv_charbuf_push_c(buf,'/');
		tlv_charbuf_push(buf,item->fn->data,item->fn->len);
		tlv_charbuf_push_c(buf,0);
		tlv_log("write [%s]\n",buf->data);
		file_write_buf(buf->data,item->data->data,item->data->len);
	}
end:
	tlv_charbuf_delete(buf);
	return ret;
}


//------------------ load ------------------
/*
int tlv_pack2_item_get(tlv_pack2_item_t *i)
{
	if(i->seek_pos<i->data->len)
	{
		//for EOF check;
		return (unsigned char)i->data->data[i->seek_pos++];
	}else
	{
		return EOF;
	}
}

int tlv_pack2_item_get2(tlv_pack2_item_t* i,char *buf,int bytes)
{
	int len;

	len=i->data->len-i->seek_pos;
	if(len<bytes)
	{
		return EOF;
	}
	memcpy(buf,i->data->data+i->seek_pos,bytes);
	i->seek_pos+=bytes;
	return bytes;
}

tlv_string_t* tlv_pack2_item_get3(tlv_pack2_item_t* i)
{
	return i->data;
}

int tlv_pack2_item_unget(tlv_pack2_item_t *i,int c)
{
	if(i->seek_pos>0){--i->seek_pos;}
	return 0;
}*/

#define tlv_pack2_item_get_c(i) ((i)->s<(i)->e)?*((i)->s++):EOF

int tlv_pack2_item_get(tlv_pack2_item_t *i)
{
	return (i->s<i->e)?*(i->s++):EOF;
}

int tlv_pack2_item_get2(tlv_pack2_item_t* i,char *buf,int bytes)
{
	if(i->e-i->s<bytes)
	{
		return EOF;
	}
	memcpy(buf,i->s,bytes);
	i->s+=bytes;
	return bytes;
}

#define SING_QUOTE '\''
#define DBL_QUOTE '"'
#define ESCAPE_CHAR '\\'
#include <ctype.h>

int tlv_pack2_read_string(tlv_pack2_item_t *item,tlv_charbuf_t *buf)
{
	char c;
	int ret=-1;
	int n;
	unsigned char *p;

	if(item->s>=item->e){goto end;}
	while(item->s<item->e)
	{
		c=*(item->s++);
		if(c=='\n' || c==' '|| c=='\t' || c=='\r' )
		{
			/* while read to last line: jfyuan 20180510 */
			if(item->s>=item->e) {  goto end; }

		}else
		{
			break;
		}
	}
	if(c==DBL_QUOTE||c==SING_QUOTE)
	{
		p=item->s;
		while(item->s<item->e && (*item->s++!=c));
		n=item->s-p-1;
		if(n>0)
		{
			tlv_charbuf_push(buf,(char*)p,n);
		}
	}else
	{
		p=item->s-1;
		while(1)
		{
			if(c==ESCAPE_CHAR)
			{
				c=*(item->s++);
				if(c>='0' && c<='7')
				{
					n=item->s-p-1;
					if(n>0)
					{
						tlv_charbuf_push(buf,(char*)p,n);
					}

					n=c-'0';
					c=*(item->s++);
					n=(n<<3)+c-'0';
					c=*(item->s++);
					n=(n<<3)+c-'0';
					c=n;

					tlv_charbuf_push_c(buf,c);
					p=item->s;
				}
			}
			//tlv_charbuf_push_c(buf,c);
			if(item->s>=item->e){break;}
			c=*(item->s++);
			if(c==' '||c=='\n' || c=='\r' || c=='\t')
			{
				--item->s;
				break;
			}
		}
		n=item->s-p;
		if(n>0)
		{
			tlv_charbuf_push(buf,(char*)p,n);
		}
	}
	ret=0;
end:
	//tlv_log("[%.*s]\n",buf->pos,buf->data);
	//exit(0);
	return ret;
}

int tlv_pack2_read_string3(tlv_pack2_item_t *item,tlv_charbuf_t *buf)
{
	char c,q;
	int ret=-1;
	int n,len;
	unsigned char *p;

	if(item->s>=item->e){goto end;}
	while(item->s<item->e)
	{
		c=*(item->s++);
		if(c=='\n' || c==' '|| c=='\t' || c=='\r' )
		{
		}else
		{
			break;
		}
	}
	if(c==DBL_QUOTE||c==SING_QUOTE)
	{
		q=c;
		while(item->s<item->e)
		{
			c=*(item->s++);
			if(c==q)
			{
				break;
			}else
			{
				tlv_charbuf_push_c(buf,c);
			}
		}
	}else
	{
		p=item->s-1;
		while(1)
		{
			if(c==ESCAPE_CHAR)
			{
				c=*(item->s++);
				if(c>='0' && c<='7')
				{
					len=item->s-p-1;
					if(len>0)
					{
						tlv_charbuf_push(buf,(char*)p,len);
					}
					n=c-'0';
					c=*(item->s++);
					n=(n<<3)+c-'0';
					c=*(item->s++);
					n=(n<<3)+c-'0';
					c=n;
					p=item->s;
				}
			}
			//tlv_charbuf_push_c(buf,c);
			if(item->s>=item->e)
			{
				len=item->s-p-1;
				if(len>0)
				{
					tlv_charbuf_push(buf,(char*)p,len);
				}
				break;
			}
			c=*(item->s++);
			if(c==' '||c=='\n' || c=='\r' ||c=='\t')
			{
				len=item->s-p-1;
				if(len>0)
				{
					tlv_charbuf_push(buf,(char*)p,len);
				}
				--item->s;
				break;
			}
		}
	}
	ret=0;
end:
	//tlv_log("[%.*s]\n",buf->pos,buf->data);
	//exit(0);
	return ret;
}

int tlv_pack2_read_string2(tlv_pack2_item_t *item,tlv_charbuf_t *buf)
{
	int isq,q=0,c,ret,n,i;
	//char t;

	ret=-1;
	c=tlv_pack2_item_get_c(item);
	while(c=='\n' || c==' '|| c=='\r' ||c=='\t')
	{
		c=tlv_pack2_item_get_c(item);
	}
	if(c==EOF){goto end;}
	if(c==DBL_QUOTE||c==SING_QUOTE)
	{
		isq=1;q=c;
		c=tlv_pack2_item_get_c(item);
	}else
	{
		isq=0;
	}
	while(1)
	{
		//tlv_log("%d:%c\n",c,c);
		if(c==EOF){ret=0;goto end;}
		if(isq)
		{
			if(c==q){break;}
		}else
		{
			if(c=='\n' || c==EOF||c==' ' || c=='\t' || c=='\r')
			{
				--item->s;
				break;
			}
		}
		if(c==ESCAPE_CHAR)
		{
			c=*(item->s++);
			if(c>='0' && c<='7')
			{
				n=c-'0';
				for(i=0;i<2;++i)
				{
					c=tlv_pack2_item_get_c(item);
					if(c==EOF||c<'0'||c>'7'){goto end;}
					n=(n<<3)+c-'0';
				}
				c=n;
			}
		}
		tlv_charbuf_push_c(buf,c);
		c=tlv_pack2_item_get_c(item);
	}
	ret=0;
end:
	return ret;
}

tlv_string_t* tlv_pack2_item_get3(tlv_pack2_item_t* i)
{
	return i->data;
}

int tlv_pack2_item_unget(tlv_pack2_item_t *i,int c)
{
	if(((char*)i->s)>i->data->data)
	{
		--i->s;
	}
	return 0;
}



void tlv_strfile_init_rbin2(tlv_strfile_t* s,tlv_pack2_item_t *i)
{
	tlv_strfile_init(s);
	i->seek_pos = 0;
	i->s        = (unsigned char*)i->data->data;
	i->e        = i->s+i->data->len;
	s->data     = i;
	s->get      = (tlv_strfile_get_handler_t)tlv_pack2_item_get;
	s->unget    = (tlv_strfile_unget_handler_t)tlv_pack2_item_unget;
	s->get_str  = (tlv_strfile_get_str_f)tlv_pack2_item_get2;
	s->get_file = (tlv_strfile_get_file_f)tlv_pack2_item_get3;
	s->read_str = (tlv_strfile_read_str_f)tlv_pack2_read_string;
	s->swap     = tlv_is_little_endian();
}

//-------------------------------------------------------------
int tlv_pack2_item_get_x(tlv_pack2_item_t *i)
{
	tlv_charbuf_t *buf=i->rb->buf;
	int len;
	int v;

	//tlv_log("get %p pos=%d/%d\n",i,i->buf_pos,buf->pos);
	if(i->buf_pos==buf->pos)
	{
		i->seek_pos+=i->buf_pos;
		if(i->seek_pos>=i->len)
		{
			i->buf_pos=buf->pos=0;
			return EOF;
		}
		buf->pos=0;
		len=i->len-i->seek_pos;
		len = min(len, buf->alloc_size);
		//tlv_log("i=%p/%p %d len=%d/%d\n",i,i->rb,len,i->len,i->seek_pos);
		buf->pos=fread(buf->data,1,len,i->rb->f);
		//tlv_log("buf->pos=%d\n",buf->pos);
		//print_data(buf->data,buf->pos);
		if(buf->pos!=len)
		{
			return EOF;
		}
		if(i->reverse)
		{
			tlv_pack_reverse_data((unsigned char*)buf->data,buf->pos);
		}
		i->buf_pos=0;
	}
	v=*(((unsigned char*)buf->data)+i->buf_pos);
	++i->buf_pos;
	return v;
}

int tlv_pack2_item_get2_x(tlv_pack2_item_t* i,char *p,int bytes)
{
	tlv_charbuf_t *buf=i->rb->buf;
	int len,ret;

	len=buf->pos-i->buf_pos;
	if(len>=bytes)
	{
		memcpy(p,buf->data+i->buf_pos,bytes);
		i->buf_pos+=bytes;
		return bytes;
	}else
	{
		if(len>0)
		{
			memcpy(p,buf->data+i->buf_pos,len);
			i->buf_pos+=len;
			bytes-=len;
			p+=len;
		}
		ret=tlv_pack2_item_get_x(i);
		if(ret==EOF){return EOF;}
		--i->buf_pos;
		ret=tlv_pack2_item_get2_x(i,p,bytes);
		if(ret==EOF){return EOF;}
		return ret+len;
	}
}


int tlv_pack2_item_unget_x(tlv_pack2_item_t *i,int c)
{
	if(i->buf_pos>0)
	{
		--i->buf_pos;
	}else
	{
		ungetc(c,i->rb->f);
	}
	return 0;
}



void tlv_strfile_init_rbin2_x(tlv_strfile_t* s,tlv_pack2_item_t *i)
{
	fseek(i->rb->f,i->pos,SEEK_SET);
	tlv_strfile_init(s);
	tlv_charbuf_reset(i->rb->buf);
	i->seek_pos=0;
	i->buf_pos=0;
	s->data=i;
	s->get=(tlv_strfile_get_handler_t)tlv_pack2_item_get_x;
	s->unget=(tlv_strfile_unget_handler_t)tlv_pack2_item_unget_x;
	s->get_str=(tlv_strfile_get_str_f)tlv_pack2_item_get2_x;
	s->get_file=NULL;

	s->swap = tlv_is_little_endian();
}



int tlv_pack2_load_file(tlv_pack2_t *pack, void *data, tlv_strfile_load_handler_t loader, char *fn)
{
//#define DEBUG_T
	tlv_strfile_t s,*ps=&s;
	tlv_pack2_item_t *i;
	int ret=-1;
#ifdef DEBUG_T
	double t;
#endif

#ifdef DEBUG_T
	t=time_get_ms();
#endif
	i = tlv_pack2_get(pack, fn, strlen(fn));
	if(!i)
	{
		tlv_log("[%s] not found\n",fn);
		goto end;
	}
	if(!i->data)
	{
		ret = tlv_pack2_load_item(pack, i, 0);
		if(ret!=0)
		{
			tlv_log("[%s] load failed\n",fn);
			goto end;
		}
	}
#ifdef DEBUG_T
	tlv_log("time=%f file=%s len=%fKB\n",time_get_ms()-t,fn,i->len*1.0/1024);
#endif
	//tlv_packitem_print(i);
	tlv_strfile_init_rbin2(ps, i);
	ret = loader(data, ps);
	tlv_pack2_item_release(i);
end:
#ifdef DEBUG_T
	tlv_log("time=%f file=%s\n",time_get_ms()-t,fn);
#endif

	return ret;
}
