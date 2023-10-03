#include "tlv_strfile.h"
#include <ctype.h>
#include <float.h>
#include <math.h>
#define DBL_QUOTE '"'
#define SING_QUOTE '\''
#define ESCAPE_CHAR '\\'

void tlv_swap_int32(int *i)
{
	char c;
	char *p;

	p    = (char*)i;
	c    = p[0];
	p[0] = p[3];
	p[3] = c;

	c    = p[1];
	p[1] = p[2];
	p[2] = c;
}

void tlv_swap_short(short *p)
{
	char temp,*q;

	q = (char*) p;
	temp = *q;
	*q = *(q+1);
	*(q+1) = temp;
}

int tlv_is_little_endian()
{
	short x, *px;
	unsigned char *pc;

	px = &x;
	pc = (unsigned char *) px;
	*pc = 1; *(pc+1) = 0;

	return x==1;
}

typedef struct
{
	FILE *f;
	unsigned char *buf;
	unsigned char *cur;
	unsigned char *valid;
	unsigned eof:1;
	int alloc;
}tlv_strfile_file_item_t;

tlv_strfile_file_item_t* tlv_strfile_file_item_new(FILE *f)
{
	tlv_strfile_file_item_t *item;

	item        = (tlv_strfile_file_item_t*)tlv_malloc(sizeof(tlv_strfile_file_item_t));
	item->f     = f;
	item->alloc = 4096;
	item->buf   = (unsigned char*)tlv_malloc(item->alloc);

	item->valid = 0;
	item->cur   = 0;
	item->eof   = 0;

	return item;
}

void tlv_strfile_file_item_delete(tlv_strfile_file_item_t *item)
{
	if(item->f)
	{
		fclose(item->f);
	}

	tlv_free(item->buf);
	tlv_free(item);
}

int tlv_strfile_file_item_get(tlv_strfile_file_item_t *item)
{
	int len;

	if(item->cur == item->valid)
	{
		if(item->eof)
		{
			return EOF;
		}
		item->cur   = item->buf;
		len         = fread(item->buf, 1, item->alloc, item->f);
		item->valid = item->cur + len;
		if(len < item->alloc)
		{
			item->eof = 1;
		}
		if(len <= 0)
		{
			item->eof=1;

			return EOF;
		}
	}

	return *(item->cur++);
}

int tlv_strfile_file_item_get_buf(tlv_strfile_file_item_t *item,char *buf,int bytes)
{
	int len;
	int ret;

	len = item->valid - item->cur;
	if(len >= bytes)
	{
		memcpy(buf, item->cur, bytes);
		item->cur += bytes;
		ret = bytes;
	}
	else
	{
		if(len > 0)
		{
			memcpy(buf, item->cur, len);
			item->cur = item->valid;
			buf   += len;
			bytes -= len;
		}
		ret = tlv_strfile_file_item_get(item);
		if(ret==EOF){return EOF;}
		--item->cur;
		ret = tlv_strfile_file_item_get_buf(item, buf, bytes);
		if(ret==EOF){return EOF;}
		ret += len;
	}

	return ret;
}

int tlv_strfile_file_item_unget(tlv_strfile_file_item_t* item,int c)
{
	if(item->cur > item->buf)
	{
		--item->cur;
		*item->cur = c;
	}
	else
	{
//		tlv_log("unget\n");
		ungetc(c,item->f);
	}
	return 0;
}


void tlv_strfile_init(tlv_strfile_t *s)
{
//	src->data     = NULL;
//	src->read_str = NULL;
//	src->get      = NULL;
//	src->get_str  = NULL;
//	src->unget    = NULL;
//	src->get_file = NULL;
//
//	src->swap=tlv_is_little_endian();

	memset(s, 0, sizeof(*s));
	s->swap = tlv_is_little_endian();
}


int tlv_strfile_init_file(tlv_strfile_t* s,char *fn)
{
	FILE* f;
	int ret = -1;
	tlv_strfile_file_item_t *item;

	f=fopen(fn,"rb");
	if(!f){s->data=0;goto end;}
	item=tlv_strfile_file_item_new(f);
	tlv_strfile_init(s);
	s->data    = item;
	s->get     = (tlv_strfile_get_handler_t)tlv_strfile_file_item_get;
	s->unget   = (tlv_strfile_unget_handler_t)tlv_strfile_file_item_unget;
	s->get_str = (tlv_strfile_get_str_f)tlv_strfile_file_item_get_buf;
	s->swap    = tlv_is_little_endian();
	ret = 0;

end:
	return ret;
}

int tlv_strfile_clean_file(tlv_strfile_t *s)
{
	if(s->data)
	{
		tlv_strfile_file_item_delete((tlv_strfile_file_item_t*)s->data);
	}
	return 0;
}

typedef struct tlv_strfile_str
{
	const unsigned char *data;
	int len;
	int pos;
}tlv_strfile_str_t;

void tlv_strfile_str_set(tlv_strfile_str_t *s, const char *data, int len)
{
	s->data = (const unsigned char*)data;
	s->len  = len;
	s->pos  = 0;
}

tlv_strfile_str_t* tlv_strfile_str_new(const char *data,int len)
{
	tlv_strfile_str_t *s;

	s=(tlv_strfile_str_t*)tlv_malloc(sizeof(*s));
	tlv_strfile_str_set(s,data,len);
	return s;
}

int tlv_strfile_str_get(tlv_strfile_str_t *s)
{
	if(s->pos<s->len)
	{
		return s->data[s->pos++];
	}else
	{
		return EOF;
	}
}

int tlv_strfiles_str_unget(tlv_strfile_str_t *s,int c)
{
	if(s->pos>0 && c!=EOF)
	{
		--s->pos;
	}
	return 0;
}

int tlv_strfile_init_str(tlv_strfile_t *s, const char *data, int bytes)
{
	tlv_strfile_init(s);
	s->data = tlv_strfile_str_new(data,bytes);
	s->get  = (tlv_strfile_get_handler_t)tlv_strfile_str_get;
	s->unget=(tlv_strfile_unget_handler_t)tlv_strfiles_str_unget;
	s->get_str=NULL;
	s->swap = tlv_is_little_endian();

	return 0;
}

int tlv_strfile_clean_str(tlv_strfile_t *s)
{
	if(s->data)
	{
		tlv_free(s->data);
	}

	return 0;
}

int tlv_strfile_peek(tlv_strfile_t *s)
{
	int c;

	c=tlv_strfile_get(s);
	tlv_strfile_unget(s,c);

	return c;
}

int tlv_strfile_read_line(tlv_strfile_t *s,tlv_charbuf_t *b)
{
	int ret;
	char c;

	tlv_charbuf_reset(b);
	while(1)
	{
		c=tlv_strfile_get(s);
		if(c=='\n'||c==EOF){goto end;}
		tlv_charbuf_push_c(b,c);
	}

end:
	ret=0;

	return ret;
}

int tlv_strfile_read_line2(tlv_strfile_t *s,tlv_charbuf_t *b,int *eof)
{
	int ret;
	char c;

	if(eof)
	{
		*eof = 0;
	}
	tlv_charbuf_reset(b);

	while(1)
	{
		c = tlv_strfile_get(s);
		if(c=='\n' || c==EOF)
		{
			if(eof && c==EOF)
			{
				*eof = 1;
			}
			goto end;
		}
		tlv_charbuf_push_c(b, c);
	}

end:
	ret=0;

	return ret;
}


int tlv_strfile_read_string(tlv_strfile_t *s,tlv_charbuf_t *b)
{
	tlv_charbuf_reset(b);

	return tlv_strfile_read_string2(s, b);
}

int tlv_strfile_read_string2(tlv_strfile_t *s, tlv_charbuf_t *b)
{
	int ret;
	int isq,q=0,c,n,i;

	if(s->read_str)
	{
		return s->read_str(s->data,b);
	}
	ret=-1;
	while(isspace(c=tlv_strfile_get(s)));
	if(c==EOF){goto end;}
	if(c==DBL_QUOTE||c==SING_QUOTE)
	{
		isq=1;q=c;
		c=tlv_strfile_get(s);
	}else
	{
		isq=0;
	}
	while(1)
	{
		if(c==EOF){ret=0;goto end;}
		if(isq)
		{
			if(c==q){break;}
		}else
		{
			if(c==EOF||isspace(c))
			{
				tlv_strfile_unget(s,c);
				break;
			}
		}
		if(c==ESCAPE_CHAR)
		{
			c=tlv_strfile_get(s);
			if(c==EOF){goto end;}
			if(c>='0' && c<='7')
			{
				n=c-'0';
				for(i=0;i<2;++i)
				{
					c=tlv_strfile_get(s);
					if(c==EOF||c<'0'||c>'7'){goto end;}
					n=(n<<3)+c-'0';
				}
				c=n;
			}
		}

		tlv_charbuf_push_c(b,c);
		c=tlv_strfile_get(s);
	}
	ret=0;
end:
	return ret;
}

int tlv_strfile_skip_sp(tlv_strfile_t *s,int *nl)
{
	int c,ret,n;

	ret=-1;n=0;
	while(1)
	{
		c=tlv_strfile_get(s);
		if(c==EOF){n=1;ret=0;goto end;}
		if(!isspace(c)){break;}
		if(c=='\n'){n=1;}
	}
	ret=0;
end:
	if(c!=EOF){tlv_strfile_unget(s,c);}
	if(nl){*nl=n;}
	return ret;
}

int tlv_strfile_fill(tlv_strfile_t* s, char* data, int len)
{
	int ret,i;
	int c;
	unsigned char*p;

	if(s->get_str)
	{
		ret = s->get_str(s->data,data,len);

		if( ret != len)
		{
			ret=-1;
		}else
		{
			ret=0;
		}
	}else
	{
		p=(unsigned char*)data;
		ret=0;
		for(i=0;i<len;++i)
		{
			c=tlv_strfile_get(s);

			if(c==EOF){ret=-1;break;}
			p[i]=c;
		}
	}

	return ret;
}

int tlv_strfile_atoi(tlv_strfile_t* s,int* value)
{
	int c,ret,i;


	while(1)
	{
		c=tlv_strfile_get(s);

		if(isspace(c))
		{
			continue;
		}else
		{
			if(isdigit(c)==0)
			{
				ret=-1;goto end;
			}
			break;
		}
	}
	i=0;
	do
	{
		i=(c-'0')+i*10;
		c=tlv_strfile_get(s);

	}while(isdigit(c));
	tlv_strfile_unget(s,c);
	*value=i;ret=0;

end:

	return ret;
}

#include <stdarg.h>
int tlv_strfile_atof(tlv_strfile_t* s, double *v)
{
	double number;
	int exponent;
	int negative;
	double p10;
	int n;
	int num_digits;
	int num_decimals;
	char c;
	int ret;

	while(1)
	{
		c=tlv_strfile_get(s);
		if(isspace(c)==0){break;}
	}
	negative=0;
	switch(c)
	{
	case '-':
		negative=1;
	case '+':
		c=tlv_strfile_get(s);
		break;
	}
	number = 0.;
	exponent = 0;
	num_digits = 0;
	num_decimals = 0;

	// Process string of digits
	while (isdigit(c))
	{
		number = number * 10. + (c - '0');
		c=tlv_strfile_get(s);
		++num_digits;
	}
	if (c == '.')
	{
		c=tlv_strfile_get(s);
		while (isdigit(c))
		{
			number = number * 10. + (c- '0');
			c=tlv_strfile_get(s);
			++num_digits;
			++num_decimals;
		}
		exponent -= num_decimals;
	}
    if (num_digits == 0)
	{

    	ret=-1;goto end;
	}
    if (negative) number = -number;
	if (c == 'e' || c == 'E')
	{

		negative = 0;
		c=tlv_strfile_get(s);
		switch(c)
		{
		case '-':
			negative = 1;
		case '+':
			c=tlv_strfile_get(s);
			break;
		}
		n = 0;
		while (isdigit(c))
		{
			n = n * 10 + (c - '0');
			c=tlv_strfile_get(s);
		}
		if (negative)
				exponent -= n;
		else
				exponent += n;
	}
	if (exponent < DBL_MIN_EXP  || exponent > DBL_MAX_EXP)
    {

		ret=-1;goto end;
    }

	p10 = 10.;
	n = exponent;
	if (n < 0) n = -n;
	while (n)
	{
		if (n & 1)
		{
			if (exponent < 0)
					number /= p10;
			else
					number *= p10;
		}
		n >>= 1;
		p10 *= p10;
	}
	if (number == HUGE_VAL)
	{

		ret=-1;goto end;
	}
	tlv_strfile_unget(s,c);
	*v=number;
	ret=0;

end:

    return ret;
}

int tlv_strfile_read_int(tlv_strfile_t *s, int* v, int n, int bin)
{
	int ret=0,x;
	int *p,*e;

	e=v+n;
	if(bin)
	{
		ret=tlv_strfile_fill(s,(char*)v,sizeof(int)*n);
		if(ret!=0 || !s->swap){goto end;}
		for(p=v;p<e;++p)
		{
			tlv_swap_int32(p);
		}
	}else
	{
		for(p=v;p<e;++p)
		{
			ret=tlv_strfile_atoi(s,&x);
			if(ret!=0){goto end;}
			*p=x;
		}
	}
end:

	return ret;
}

int tlv_strfile_read_short(tlv_strfile_t* s,short* v,int n,int bin)
{
	int ret=0,x;
	short *p,*e;

	e=v+n;
	if(bin)
	{
		ret=tlv_strfile_fill(s,(char*)v,sizeof(short)*n);
		if(ret!=0 || !s->swap){goto end;}
		for(p=v;p<e;++p)
		{
			tlv_swap_short(p);
		}
	}else
	{
		for(p=v;p<e;++p)
		{
			ret=tlv_strfile_atoi(s,&x);
			if(ret!=0){goto end;}
			*p=x;
		}
	}
end:
	return ret;
}


int tlv_strfile_read_float(tlv_strfile_t *s,float *f,int n,int bin)
{
	int ret=0;
	float *p,*e;
	double d;

	e=f+n;
	if(bin)
	{
		ret=tlv_strfile_fill(s,(char*)f,n*sizeof(float));
		if(ret!=0 || !s->swap){goto end;}
		for(p=f;p<e;++p)
		{
			tlv_swap_int32((int*)p);
		}
	}else
	{

		for(p=f;p<e;++p)
		{
			ret=tlv_strfile_atof(s,&d);

			if(ret!=0)
			{
				goto end;
			}
			*p=d;
		}
	}
end:
	return ret;
}

int tlv_file_write_float(FILE *file,float *f,int n,int bin,int swap)
{
	int ret=0;
	float *p,*e;

	e=f+n;
	if(bin)
	{
		if(swap)
		{
			for(p=f;p<e;++p)
			{
				tlv_swap_int32((int*)p);
			}
		}
		ret=fwrite(f,sizeof(float),n,file);
		ret=ret==n?0:-1;
		if(swap)
		{
			for(p=f;p<e;++p)
			{
				tlv_swap_int32((int*)p);
			}
		}
	}else
	{
		for(p=f;p<e;++p)
		{
			fprintf(file,"%e\n",*p);
		}
	}
	return ret;
}


int tlv_strfile_load_file(void *data,tlv_strfile_load_handler_t loader,char *fn)
{
	tlv_strfile_t s,*ps=&s;
	int ret;

	ret=tlv_strfile_init_file(ps,fn);
	if(ret!=0){goto end;}
	ret=loader(data,ps);
	tlv_strfile_clean_file(ps);
end:
	if(ret!=0)
	{
		tlv_log("load %s failed.\n",fn);
	}

	return ret;
}

int tlv_strfile_load_file_v(void *hook,void *data,tlv_strfile_load_handler_t loader,char *fn)
{
	return tlv_strfile_load_file(data,loader,fn);
}

int tlv_strfile_loader_load(tlv_strfile_loader_t *l,void *data,tlv_strfile_load_handler_t loader,char *fn)
{
	return l->vf(l->hook,data,loader,fn);
}

tlv_string_t* tlv_strfile_read_file(tlv_strfile_t *s)
{
	tlv_charbuf_t *buf;
	tlv_string_t *data;
	char c;

	buf = tlv_charbuf_new(256, 1);
	while(1)
	{
		c = tlv_strfile_get(s);
		if(c==EOF) { break; }
		tlv_charbuf_push_c(buf, c);
	}
	data = tlv_string_dup_data(buf->data, buf->pos);
	tlv_charbuf_delete(buf);

	return data;
}

int tlv_strfile_get_lines(int *nw, tlv_strfile_t *s)
{
	int n = 0;
	char c;
	int newline;

	if(!s) { goto end; }
	newline = 1;
	while(1)
	{
		c = tlv_strfile_get(s);
		if(c == EOF)
		{
			goto end;
		}
		else if(c == '\n')
		{
			newline = 1;
		}
		else
		{
			if(newline)
			{
				++n;
				newline = 0;
			}
		}
	}

end:
	*nw = n;

	return 0;
}

int tlv_strfile_loader_file_lines(tlv_strfile_loader_t *sl, char* fn)
{
	int ret;
	int line = 0;

	ret = tlv_strfile_loader_load(sl, &line, (tlv_strfile_load_handler_t)tlv_strfile_get_lines, fn);
	if(0 == ret)
	{
		return line;
	}
	else
	{
		return -1;
	}
}

