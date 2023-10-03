#include <ctype.h>
#include <stdarg.h>
#include "tlv_string.h"
#include "tlv/struct/tlv_strfile.h"
#include "tlv/struct/tlv_str_encode.h"
#include "tlv/struct/tlv_charbuf.h"

tlv_string_t* tlv_string_new(int len)
{
	tlv_string_t *str;

	str = (tlv_string_t*)tlv_malloc(len + sizeof(*str));
	str->len = len;
	if(len>0)
	{
		str->data = (char*)str + sizeof(*str);
	}else
	{
		str->data = NULL;
	}
	return str;
}

tlv_string_t* tlv_string_dup_data(const char* data, int len)
{
	tlv_string_t* str;

	str = tlv_string_new(len);
	if(str && data)
	{
		memcpy(str->data, data, len);
	}

	return str;
}

int tlv_string_free(tlv_string_t *str)
{
	tlv_free(str);

	return 0;
}

int tlv_string_cmp(tlv_string_t *str, char* s, int len)
{
	int ret=-1;

	if(str->len != len){goto end;}
	ret = strncmp(str->data, s, len);

end:
	return ret;
}

int tlv_string_cmp2(tlv_string_t *str1, tlv_string_t *str2)
{
	int ret;

	if(str1!=str2)
	{
		if(str1 && str2)
		{
			if(tlv_string_cmp(str1,str2->data,str2->len)!=0)
			{
				ret=-1;goto end;
			}
		}else
		{
			ret=-1;goto end;
		}
	}
	ret=0;
end:
	return ret;
}

int tlv_string_is_char_in(tlv_string_t *str, char c)
{
	int ret=0;
	int i;

	for(i=0;i<str->len;++i)
	{
		if(str->data[i]==c)
		{
			ret=1;
			break;
		}
	}

	return ret;
}

char* tlv_str_dup_len(const char* str, size_t len)
{
    char* p=0;

    if(str)
    {
        p=(char*)tlv_malloc(len);
        strcpy(p,str);
    }
    return p;
}

char* tlv_str_dup(const char* s)
{
	char* p = NULL;

	if(s)
	{
        p = (char*)tlv_malloc(strlen(s)+1);
        strcpy(p, s);
	}

    return p;
}

long long tlv_str_atoi(char* s, int len)
{
	char *p,*end,c;
	long long v;
	int sign;

	v=0;
	if(len<=0){goto end;}
	p=s;end=s+len;
	while(p<end && isspace(*p))
	{
		++p;
	}
	if(*p=='-')
	{
		++p;
		sign=-1;
	}else
	{
		sign=1;
	}
	while(p<end)
	{
		c=*p;
		if(c< '0' || c > '9')
		{
			break;
		}
		v=v*10+c-'0';
		++p;
	}
	if(sign==-1)
	{
		v=-v;
	}
end:
	return v;
}

double tlv_str_atof(char *s, int len)
{
	tlv_strfile_t src;
	double v;

	v=0;
	tlv_strfile_init_str(&(src),s,len);
	tlv_strfile_atof(&(src),&v);
	tlv_strfile_clean_str(&(src));
	return v;
}

char* tlv_str_chr(char* s,int slen, char c)
{
	char *p = 0;
	char *e = s+slen;

	while(s<e)
	{
		if(*s==c)
		{
			p=s;
			break;
		}
		++s;
	}

	return p;
}

int tlv_str_str(char *src, int src_len, char *sub, int sub_len)
{
	char *s, *e;
	int i, index;

	s=sub; e=s+sub_len;
	for(i=0,index=0; i<src_len; ++i)
	{

		if(src[i]==*s)
		{
			++s;
			if(s>=e)
			{
				return index;
			}
		}else
		{
			i=i-(s-sub);
			index=i+1;
			s=sub;
		}
	}

	return -1;
}

void print_data(char* data, int len)
{
	printf("%.*s\n", len , data);
}

void print_hex(char *data,int len)
{
	int i;

	printf("(%d,",len);
	for (i = 0; i < len; ++i)
	{
		printf("\\x%02x", (unsigned char) data[i]);
	}
	printf(")\n");
}

int tlv_char_to_hex(char c)
{
	int v;

	if(c>='0' && c<='9')
	{
		v=c-'0';
	}else if(c>='A' && c<='F')
	{
		v=c-'A'+10;
	}else if(c>='a' && c<='f')
	{
		v=c-'a'+10;
	}else
	{
		v=-1;
	}
	return v;
}

uint32_t hash_string_value(char* s)
{
	uint32_t hashval;

	hashval = 0;
	if (s)
	{
		for (; *s != 0; ++s)
		{
			hashval = *s + (hashval << 4) - 1;
		}
	}

	return hashval;
}

uint32_t hash_string(char* s, uint32_t hash_size)
{
	uint32_t hashval;

	hashval = hash_string_value(s);

	return hashval % hash_size;
}

uint32_t hash_string_value_len_seed(unsigned char* p, int len, int hash_size)
{
	uint32_t hashval = 0;
	unsigned char *e;

	e = p + len;
	while(p < e)
	{
		hashval = (hashval*131) + (*p++);
	}

	return (hashval & 0x7FFFFFFF) % hash_size;
}


/**
 * @brief "a-b+c" => "b"
 */
void tlv_string_get_midname(tlv_string_t *src, tlv_string_t *dst)
{
	char *p;

	p = tlv_str_chr(src->data, src->len, '-');
	if(p)
	{
		dst->data = p + 1;
		dst->len = src->data - dst->data + src->len;
	}
	else
	{
		dst->data = src->data;
		dst->len = src->len;
	}

	p = tlv_str_chr(dst->data, dst->len, '+');
	if(p)
	{
		dst->len = p - dst->data;
	}
}
