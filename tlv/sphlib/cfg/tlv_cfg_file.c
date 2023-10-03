#include "tlv_cfg_file.h"
#include "tlv/struct/tlv_sys.h"
#include <ctype.h>
int tlv_cfg_file_feed_expr_value_tok_end(tlv_cfg_file_t *cfg,char c);
int tlv_cfg_file_feed_expr_value_start(tlv_cfg_file_t *cfg,char c);
int tlv_cfg_file_feed_expr_value_tok_start(tlv_cfg_file_t *cfg,char c);
int tlv_cfg_file_feed_expr_tok_start(tlv_cfg_file_t* cfg,char c);
int tlv_cfg_file_feed_expr_start(tlv_cfg_file_t* cfg,char c);
int tlv_cfg_file_feed_string(tlv_cfg_file_t *c,char *d,int bytes);

int tlv_cfg_file_init(tlv_cfg_file_t *f)
{
	f->main=tlv_part_cfg_new_h(f->heap);
	f->cur=f->main;
	tlv_string_set_s(&(f->main->name),"main");
	tlv_queue_push(&(f->cfg_queue),&(f->main->q_n));
	f->scope=0;
	f->included=0;
	f->escaped=0;
	return 0;
}

tlv_cfg_file_t* tlv_cfg_file_new_fn(char *fn)
{
	return tlv_cfg_file_new_fn2(fn,1);
}

tlv_cfg_file_t* tlv_cfg_file_new_fn2(char *fn,int add_pwd)
{
	tlv_cfg_file_t *cfg=0;
	char *data=0;
	int len,ret;
	tlv_string_t *v=0;

	ret=0;
	data=file_read_buf(fn,&len);
	if(!data){goto end;}
	cfg=tlv_cfg_file_new();
	if(add_pwd)
	{
		v=tlv_dir_name(fn,'/');
		tlv_cfg_file_add_var_ks(cfg,"pwd",v->data,v->len);
	}
	ret=tlv_cfg_file_feed(cfg,data,len);
	if(v)
	{
		tlv_string_free(v);
	}
end:
	if(data){free(data);}
	if(ret!=0)
	{
		tlv_cfg_file_delete(cfg);
		cfg=0;
	}
	return cfg;
}

tlv_cfg_file_t *tlv_cfg_file_new()
{
	return tlv_cfg_file_new_ex(2048,4096);
}

tlv_cfg_file_t *tlv_cfg_file_new_ex(int buf_size,int heap_size)
{
	tlv_cfg_file_t *f;

	f=(tlv_cfg_file_t*)tlv_malloc(sizeof(*f));
	tlv_queue_init(&(f->cfg_queue));
	f->heap=tlv_heap_new(heap_size);
	f->tok=tlv_charbuf_new(buf_size,1);
	f->value=tlv_charbuf_new(buf_size,1);
	f->var=tlv_charbuf_new(buf_size,1);
	//f->comment=tlv_charbuf_new(buf_size,1);
	f->quoted=0;
	f->included=0;
	tlv_cfg_file_init(f);
	return f;
}

int tlv_cfg_file_bytes2(tlv_cfg_file_t *cfg)
{
	return sizeof(*cfg)+tlv_charbuf_bytes(cfg->tok)+tlv_charbuf_bytes(cfg->value)+tlv_charbuf_bytes(cfg->var)+tlv_heap_bytes(cfg->heap);
}

int tlv_cfg_file_bytes(tlv_cfg_file_t *c)
{
	int bytes;

	bytes=sizeof(tlv_cfg_file_t);
	bytes+=tlv_heap_bytes(c->heap);
	bytes+=tlv_charbuf_bytes(c->tok);
	bytes+=tlv_charbuf_bytes(c->value);
	bytes+=tlv_charbuf_bytes(c->var);
	return bytes;
}

//int tlv_cfg_file_reset(tlv_cfg_file_t *cfg)
//{
//	tlv_queue_init(&(cfg->cfg_queue));
//	tlv_heap_reset(cfg->heap);
//	cfg->quoted=0;
//	cfg->included=0;
//	return tlv_cfg_file_init(cfg);
//}

int tlv_cfg_file_delete(tlv_cfg_file_t *c)
{
	tlv_heap_delete(c->heap);
	tlv_charbuf_delete(c->tok);
	tlv_charbuf_delete(c->value);
	tlv_charbuf_delete(c->var);
	//tlv_charbuf_delete(c->comment);
	tlv_free(c);
	return 0;
}



int tlv_cfg_file_add_var(tlv_cfg_file_t *cfg,char *k,int kbytes,char *v,int vbytes)
{
	return tlv_cfg_queue_add_string(cfg->main->queue, k, kbytes, v, vbytes);
}

/*
	space=[ \r\t\n]
	digit=[0-9]
	alpha=[a-zA-Z]
	char=digit|alpha
	tok=	char+
	var=${tok}
	expr=	tok space*=space* value space*;
	value= 	tok | {space* expr* space*}
*/
#define is_value(c) (isalnum((c))||(c==':')||c=='/'||c=='\\'||c=='_'||c=='-'||c=='.')
#define is_char(c) (isalnum((c))||(c==':')||c=='_'||c=='.'||c=='-'||c=='/'||c=='@')

int tlv_cfg_file_process_include(tlv_cfg_file_t *cfg)
{
	int ret=-1;
	char *data;
	tlv_string_t *v;
	tlv_cfg_item_t *pth,*item;
	int n;
	tlv_cfg_queue_t *cq;

	tlv_charbuf_push_c(cfg->value,0);
	cfg->included=0;
	data=file_read_buf(cfg->value->data,&n);
	if(!data)
	{
		tlv_log("%s not found.\n",cfg->value->data);
		goto end;
	}
	cfg->state=CF_EXPR_START;
	cq = cfg->cur->queue;
	pth=tlv_cfg_queue_find(cq, "pwd", sizeof("pwd")-1);
	if(pth)
	{
		tlv_cfg_queue_remove(cq,pth);
	}
	v=tlv_dir_name(cfg->value->data,'/');
	if(!v){goto end;}
	tlv_cfg_queue_add_string(cq,"pwd",3,v->data,v->len);
	tlv_string_free(v);
	ret=tlv_cfg_file_feed_string(cfg,data,n);
	tlv_free(data);
	if(ret!=0){goto end;}
	item=tlv_cfg_queue_find(cq,"pwd", sizeof("pwd")-1);
	if(item)
	{
		tlv_cfg_queue_remove(cq,item);
	}
	if(pth)
	{
		tlv_cfg_queue_add(cq,pth);
	}
	cfg->state=CF_EXPR_START;
end:
	return ret;
}

int tlv_cfg_file_feed_expr_value_tok_end(tlv_cfg_file_t *cfg,char c)
{
	int ret=0;

	if(c==';')
	{
		//tlv_charbuf_push_c(cfg->value,0);
		if(cfg->included)
		{
			ret=tlv_cfg_file_process_include(cfg);
		}else
		{
			tlv_cfg_queue_add_string(cfg->cur->queue, cfg->tok->data, cfg->tok->pos, cfg->value->data, cfg->value->pos);
			cfg->state=CF_EXPR_START;
		}
	}else if(!isspace(c))
	{
		tlv_log("expect \";\"\n");
		ret=-1;
	}

	return ret;
}

void tlv_cfg_file_set_state(tlv_cfg_file_t *cfg,tlv_cfg_file_state_t state)
{
	cfg->state=state;
	cfg->quoted=0;
	cfg->escaped=0;
}

int tlv_cfg_file_feed_array_tok_end(tlv_cfg_file_t *cfg,char c)
{
	int ret=0;

	if(c==',')
	{
		tlv_cfg_file_set_state(cfg,CFG_ARRAY_START);
	}else if (c==']')
	{
		tlv_cfg_file_set_state(cfg,CF_EXPR_START);
	}else if(!isspace(c))
	{
		tlv_log("expect array tok like \",\" or \"]\",buf found[%c]\n",c);
		ret=-1;
	}

	return ret;
}

int tlv_cfg_file_feed_array_tok_start(tlv_cfg_file_t *cfg,char c)
{
	tlv_string_t *s;
	int ret=0;

	//tlv_log("c=[%c]\n",c);
	if(cfg->escaped)
	{
		tlv_charbuf_push_c(cfg->value,c);
		cfg->escaped=0;
		return 0;
	}
	if(cfg->quoted)
	{
		if(c==cfg->quoted_char)
		{
			s=tlv_heap_dup_string(cfg->heap,cfg->value->data,cfg->value->pos+1);
			--s->len;
			s->data[s->len]=0;
			((tlv_string_t**)tlv_array_push(cfg->array))[0]=s;
			//tlv_log("[%.*s]\n",s->len,s->data);
			//cfg->state=CFG_ARRAY_TOK_END;
			tlv_cfg_file_set_state(cfg,CFG_ARRAY_TOK_END);
		}else
		{
			if(c=='\\')
			{
				cfg->escaped=1;
			}else
			{
				tlv_charbuf_push_c(cfg->value,c);
			}
		}
	}else
	{
		if(isspace(c)||c==','||c==']')
		{
			if(cfg->value->pos>0)
			{
				s=tlv_heap_dup_string(cfg->heap,cfg->value->data,cfg->value->pos+1);
				--s->len;
				s->data[s->len]=0;
				((tlv_string_t**)tlv_array_push(cfg->array))[0]=s;
			}
			//cfg->state=CFG_ARRAY_TOK_END;
			tlv_cfg_file_set_state(cfg,CFG_ARRAY_TOK_END);
			if(!isspace(c))
			{
				ret=tlv_cfg_file_feed_array_tok_end(cfg,c);
			}
		}else if(c=='$')
		{
			cfg->var_cache_state=CFG_ARRAY_TOK_START;
			//cfg->state=CF_VAR_START;
			tlv_cfg_file_set_state(cfg,CF_VAR_START);
		}else
		{
			if(cfg->value->pos==0 && (c=='\'' || c=='"'))
			{
				cfg->quoted=1;
				cfg->quoted_char=c;
			}else
			{
				tlv_charbuf_push_c(cfg->value,c);
			}
		}
	}
	return ret;
}

int tlv_cfg_file_feed_array_start(tlv_cfg_file_t *cfg,char c)
{
	int ret;

	if(!isspace(c))
	{
		//cfg->state=CFG_ARRAY_TOK_START;
		tlv_charbuf_reset(cfg->value);
		//cfg->quoted=0;
		tlv_cfg_file_set_state(cfg,CFG_ARRAY_TOK_START);
		ret=tlv_cfg_file_feed_array_tok_start(cfg,c);
	}else
	{
		ret=0;
	}
	return ret;
}

int tlv_cfg_file_feed_expr_value_start(tlv_cfg_file_t *cfg,char c)
{
	tlv_heap_t *h=cfg->heap;
	tlv_part_cfg_t *lc;
	int ret=0;

	if(c=='{')
	{
		tlv_cfg_item_t* item;

		item=tlv_cfg_queue_find(cfg->cur->queue, cfg->tok->data, cfg->tok->pos);
		if(item && item->type==TLV_CFG_LC)
		{
			lc=item->value.cfg;
		}else
		{
			lc=tlv_part_cfg_new_h(h);
			tlv_cfg_queue_add_lc(cfg->cur->queue,cfg->tok->data,cfg->tok->pos,lc);
			tlv_queue_push(&(cfg->cfg_queue),&(lc->q_n));
			lc->parent=cfg->cur;
		}
		cfg->cur=lc;
		cfg->state=CF_EXPR_START;
		++cfg->scope;
		ret=tlv_cfg_file_feed_expr_start(cfg,c);
	}else if(c=='[')
	{
		cfg->state=CFG_ARRAY_START;
		cfg->array=tlv_array_new(cfg->heap,5,sizeof(tlv_string_t*));
		tlv_cfg_queue_add_array(cfg->cur->queue, cfg->tok->data, cfg->tok->pos, cfg->array);
	}else if(is_char(c)||c=='$'||c=='"')
	{
		cfg->state=CF_EXPR_VALUE_TOK_START;
		tlv_charbuf_reset(cfg->value);
		cfg->quoted=c=='"';
		if(cfg->quoted)
		{
			cfg->quoted_char=c;
		}else
		{
			ret=tlv_cfg_file_feed_expr_value_tok_start(cfg,c);
		}
	}else if(!isspace(c))
	{
		tlv_log("expect expr value start %c.\n",c);
		ret=-1;
	}
	return ret;
}

int tlv_cfg_file_feed_var_tok_start(tlv_cfg_file_t *cfg,char c)
{
	int ret=0;
	tlv_string_t *n;

	if(is_char(c))
	{
		tlv_charbuf_push_c(cfg->var,c);
	}else if(c=='}')
	{
		n=tlv_part_cfg_find_string(cfg->cur,cfg->var->data,cfg->var->pos);
		if(n)
		{
			tlv_charbuf_push(cfg->value,n->data,n->len);
			cfg->state=cfg->var_cache_state;
			//cfg->state=CF_EXPR_VALUE_TOK_START;
		}else
		{
			tlv_log("var %*.*s not found.\n",cfg->var->pos,cfg->var->pos,cfg->var->data);
			ret=-1;
		}
	}else if(!isspace(c))
	{
		tlv_log("expect expr tok start.\n");
		ret=-1;
	}
	return ret;
}

int tlv_cfg_file_feed_var_tok(tlv_cfg_file_t *cfg,char c)
{
	int ret;

	if(!isspace(c))
	{
		cfg->state=CF_VAR_TOK_START;
		tlv_charbuf_reset(cfg->var);
		ret=tlv_cfg_file_feed_var_tok_start(cfg,c);
	}else
	{
		ret=0;
	}
	return ret;
}

int tlv_cfg_file_feed_var_start(tlv_cfg_file_t *cfg,char c)
{
	int ret;

	if(c=='{')
	{
		cfg->state=CF_VAR_TOK;
		ret=0;
	}else
	{
		tlv_log("expect var { start.\n");
		ret=-1;
	}
	return ret;
}


int tlv_cfg_file_feed_escape_x2(tlv_cfg_file_t *cfg,char c)
{
	int ret;
	int v;

	v=tlv_char_to_hex(c);
	if(v==-1)
	{
		ret=-1;
	}else
	{
		cfg->escape_char=(cfg->escape_char<<4)+v;
		tlv_charbuf_push_c(cfg->value,cfg->escape_char);
		cfg->state=CF_EXPR_VALUE_TOK_START;
		ret=0;
	}
	return ret;
}

int tlv_cfg_file_feed_escape_x1(tlv_cfg_file_t *cfg,char c)
{
	int ret;
	int v;

	v=tlv_char_to_hex(c);
	if(v==-1)
	{
		ret=-1;
	}else
	{
		cfg->escape_char=v;
		cfg->state=CFG_ESCAPE_X2;
		ret=0;
	}
	return ret;
}

int tlv_cfg_file_feed_escape_o2(tlv_cfg_file_t *cfg,char c)
{
	int ret;

	if(c>='0' && c<='7')
	{
		cfg->escape_char=(cfg->escape_char<<2)+c-'0';
		cfg->state=CF_EXPR_VALUE_TOK_START;
		ret=0;
	}else
	{
		ret=-1;
	}
	return ret;
}

int tlv_cfg_file_feed_escape_o1(tlv_cfg_file_t *cfg,char c)
{
	int ret;

	if(c>='0' && c<='7')
	{
		cfg->escape_char=(cfg->escape_char<<2)+c-'0';
		cfg->state=CFG_ESCAPE_O2;
		ret=0;
	}else
	{
		ret=-1;
	}
	return ret;
}

int tlv_cfg_file_feed_escape_start(tlv_cfg_file_t *cfg,char c)
{
	if(c=='x'||c=='X')
	{
		cfg->escape_char=0;
		cfg->state=CFG_ESCAPE_X1;
	}else if(c>='0' && c<='7')
	{
		cfg->escape_char=c-'0';
		cfg->state=CFG_ESCAPE_O1;
	}else
	{
		switch(c)
		{
		case 't':
			tlv_charbuf_push_c(cfg->value,'\t');
			break;
		case 'n':
			tlv_charbuf_push_c(cfg->value,'\n');
			break;
		case 'r':
			tlv_charbuf_push_c(cfg->value,'\r');
			break;
		case '\'':
			tlv_charbuf_push_c(cfg->value,'\'');
			break;
		case '\"':
			tlv_charbuf_push_c(cfg->value,'\"');
			break;
		case '\\':
			tlv_charbuf_push_c(cfg->value,'\\');
			break;
		default:
			tlv_charbuf_push_c(cfg->value,c);
			break;
		}
		cfg->state=CF_EXPR_VALUE_TOK_START;
	}
	return 0;
}

int tlv_cfg_file_feed_expr_value_tok_start(tlv_cfg_file_t *cfg,char c)
{
	int ret=0;

	if(c=='\\')
	{
		cfg->state=CFG_ESCAPE_START;
		return 0;
	}
	if(cfg->quoted)
	{
		if(c==cfg->quoted_char)
		{
			cfg->quoted=0;
			cfg->state=CF_EXPR_VALUE_TOK_END;
		}else
		{
			tlv_charbuf_push_c(cfg->value,c);
		}
		return 0;
	}
	if(is_value(c))
	{
		tlv_charbuf_push_c(cfg->value,c);
	}else if(c==';')
	{
		cfg->state=CF_EXPR_VALUE_TOK_END;
		ret=tlv_cfg_file_feed_expr_value_tok_end(cfg,c);
	}else if(c=='$')
	{
		cfg->var_cache_state=CF_EXPR_VALUE_TOK_START;
		cfg->state=CF_VAR_START;
	}else
	{
		tlv_log("expect var value %c end.\n",c);
		ret=-1;
	}
	return ret;
}

int tlv_cfg_file_feed_expr_tok_start(tlv_cfg_file_t* cfg,char c)
{
	int ret=0;

	if(cfg->quoted)
	{
		if(c!=cfg->quoted_char)
		{
			tlv_charbuf_push_c(cfg->tok,c);
		}else
		{
			cfg->quoted=0;
			cfg->state=CF_EXPR_TOK_WAIT_EQ;
		}
	}else
	{
		if(is_char(c))
		{
			tlv_charbuf_push_c(cfg->tok,c);
		}else if(c=='=')
		{
			cfg->state=CF_EXPR_VALUE_START;
			//ret=tlv_cfg_file_feed_expr_value_start(cfg,c);
		}else if(!isspace(c))
		{
			ret=-1;
		}
	}
	return ret;
}

int tlv_cfg_file_feed_expr_tok_wait_eq(tlv_cfg_file_t* cfg,char c)
{
	if(c=='=')
	{
		cfg->state=CF_EXPR_VALUE_START;
	}
	return 0;
}


int tlv_cfg_file_feed_comment(tlv_cfg_file_t *cfg,char c)
{
	int len=sizeof("include")-1;
	tlv_charbuf_t *buf;

	if(c=='\n')
	{
		//tlv_charbuf_reset(cfg->tok);
		cfg->state=CF_EXPR_START;
	}else
	{
		buf=cfg->tok;
		if(buf->pos<len)
		{
			tlv_charbuf_push_c(buf,c);
			if(buf->pos==len)
			{
				if(strncmp(buf->data,"include",buf->pos)==0)
				{
					cfg->state=CF_EXPR_VALUE_START;
					cfg->included=1;
				}
			}
		}
	}
	return 0;
}

int tlv_cfg_file_feed_expr_start(tlv_cfg_file_t* cfg,char c)
{
	int ret=0;

	if(is_char(c) ||c=='"' ||c=='\'')
	{
		cfg->state=CF_EXPR_TOK_START;
		tlv_charbuf_reset(cfg->tok);
		if(c=='"' ||c=='\'')
		{
			cfg->quoted=1;
			cfg->quoted_char=c;
		}else
		{
			ret=tlv_cfg_file_feed_expr_tok_start(cfg,c);
		}
	}else if(c=='}')
	{
		if(cfg->scope<=0)
		{
			ret=-1;
		}else
		{
			--cfg->scope;
			cfg->cur=cfg->cur->parent;
		}
	}else if(c=='#')
	{
		cfg->state=CFG_COMMENT;
		tlv_charbuf_reset(cfg->tok);
	}else
	{
		//tlv_log("c=%c\n",c);
		ret=0;
	}
	return ret;
}

int tlv_cfg_file_feed_fn(tlv_cfg_file_t* cfg,const char *fn)
{
	char *data=0;
	int len,ret=-1;

	data=file_read_buf(fn,&len);
	if(!data){goto end;}
	//print_data(data,len);
	ret=tlv_cfg_file_feed(cfg,data,len);
end:
	if(data){free(data);}
	return ret;
}

int tlv_cfg_file_feed_string(tlv_cfg_file_t *c,char *d,int bytes)
{
	char *s=d,*e=d+bytes;
	int ret=-1;

	while(s<e)
	{
		//printf("%c",*s);
		//tlv_log("[%c]:%d\n",*s,c->state);
		switch(c->state)
		{
		case CF_EXPR_START:
			ret=tlv_cfg_file_feed_expr_start(c,*s);
			break;
		case CF_EXPR_TOK_START:
			ret=tlv_cfg_file_feed_expr_tok_start(c,*s);
			break;
		case CF_EXPR_TOK_WAIT_EQ:
			ret=tlv_cfg_file_feed_expr_tok_wait_eq(c,*s);
			break;
		case CF_EXPR_VALUE_START:
			ret=tlv_cfg_file_feed_expr_value_start(c,*s);
			break;
		case CFG_ESCAPE_START:
			ret=tlv_cfg_file_feed_escape_start(c,*s);
			break;
		case CFG_ESCAPE_X1:
			ret=tlv_cfg_file_feed_escape_x1(c,*s);
			break;
		case CFG_ESCAPE_X2:
			ret=tlv_cfg_file_feed_escape_x2(c,*s);
			break;
		case CFG_ESCAPE_O1:
			ret=tlv_cfg_file_feed_escape_o1(c,*s);
			break;
		case CFG_ESCAPE_O2:
			ret=tlv_cfg_file_feed_escape_o2(c,*s);
			break;
		case CF_EXPR_VALUE_TOK_START:
			ret=tlv_cfg_file_feed_expr_value_tok_start(c,*s);
			break;
		case CF_EXPR_VALUE_TOK_END:
			ret=tlv_cfg_file_feed_expr_value_tok_end(c,*s);
			break;
		case CF_VAR_START:
			ret=tlv_cfg_file_feed_var_start(c,*s);
			break;
		case CF_VAR_TOK:
			ret=tlv_cfg_file_feed_var_tok(c,*s);
			break;
		case CF_VAR_TOK_START:
			ret=tlv_cfg_file_feed_var_tok_start(c,*s);
			break;
		case CFG_ARRAY_START:
			ret=tlv_cfg_file_feed_array_start(c,*s);
			break;
		case CFG_ARRAY_TOK_START:
			ret=tlv_cfg_file_feed_array_tok_start(c,*s);
			break;
		case CFG_ARRAY_TOK_END:
			ret=tlv_cfg_file_feed_array_tok_end(c,*s);
			break;
		case CFG_COMMENT:
			ret=tlv_cfg_file_feed_comment(c,*s);
			break;
		default:
			ret=-1;
			break;
		}
		if(ret!=0)
		{
			print_data(d,s-d);
			break;
		}
		++s;
	}
	//tlv_cfg_file_print(c);
	return ret;
}

int tlv_cfg_file_feed(tlv_cfg_file_t *c,char *d,int bytes)
{
	int ret;

	//tlv_log("%*.*s\n",bytes,bytes,d);
	c->state=CF_EXPR_START;
	c->cur=c->main;
	ret=tlv_cfg_file_feed_string(c,d,bytes);
	return ret;
}


void tlv_cfg_file_print(tlv_cfg_file_t *c)
{

	tlv_part_cfg_print(c->main);

}
