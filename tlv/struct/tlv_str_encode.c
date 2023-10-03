#include "tlv_str_encode.h"
#include <ctype.h>

#ifdef WIN32
#include <Windows.h>
#else

#ifdef NOT_USE_ICONV
#else
#include <iconv.h>
#endif

#endif /* #ifdef WIN32 */

int str_is_utf8(const unsigned char* utf, int len)
{
    unsigned char c;
    int i,count;
    int ret;

    count=0;ret=0;
    for(i=0;i<len;++i)
    {
        c=*(utf+i);
        if(c<0x80 && count==0){continue;}
        if(count==0)
        {
            if(c>=0xF0)
            {
                count=3;
            }else if(c>=0xE0)
            {
                count=2;
            }else if(c>=0x80)
            {
                count=1;
            }else
            {
                goto end;
            }
        }else
        {
            if((c&0xc0)!=0x80)
            {
                goto end;
            }
            --count;
        }
    }
    ret=1;
end:
     return ret;
}

int tlv_utf8_bytes(char c)
{
	int num;

	if(c&0x80)
	{
		num=2;
		c=c<<2;
		while(c & 0x80)
		{
			c=c<<1;
			++num;
		}
	}else
	{
		num=1;
	}
	return num;
}

#ifdef WIN32
char* str_convert(const char * src,unsigned int  code_from,unsigned int code_to)
{
    wchar_t *u;
    char *dst;
    int ret, len;

    dst=0;u=0;
    ret=-1;
    if(!src){goto end;}
    len = MultiByteToWideChar(code_from, 0,src, -1, NULL,0 );
    if(len<1){goto end;}
    u=(wchar_t*)tlv_malloc( sizeof(wchar_t) * len);
    len = MultiByteToWideChar( code_from, 0, src, -1, u, len );
    if(len<1){goto end;}
    len = WideCharToMultiByte( code_to, 0, u, -1, NULL, 0, NULL, NULL );
    if(len<1){goto end;}
    dst = (char *) tlv_calloc(1, sizeof(char) * len+1);
    len = WideCharToMultiByte( code_to, 0, u, -1, dst, len, NULL, NULL );
    if(len<1){goto end;}
    ret=0;

end:
    if(u){free(u);}
    if(ret!=0 && dst)
    {
        free(dst);
        dst=0;
    }

    return dst;
}

char* gbk_to_utf8( const char * gbk)
{
    return str_convert(gbk,CP_ACP,CP_UTF8);
}
#else

#ifdef NOT_USE_ICONV
char* gbk_to_utf8(const char* gbk,int len)
{
	return 0;
}
#else

int tlv_str_convert(const char* src,int src_bytes, const char* src_code, const char *dst_code,char *dst,int dst_bytes)
{
	iconv_t id;
	size_t in,out,c;
	int ret=-1;

	id=0;
	if(!src){goto end;}
	id=iconv_open(src_code,dst_code);
	if(!id){goto end;}
	in=src_bytes;
	out=dst_bytes;
	c=iconv(id,(char**)&src,&in,&dst,&out);
	//tlv_log("c=%d,in=%d,out=%d\n",c,in,out);
	if(c<0){ret=-1;goto end;}
	ret=c;//dst_bytes-out;
	//tlv_log("ret=%d\n",ret);
end:
	if(id){iconv_close(id);}
	return ret;
}

/**
 * @return chars is in static buffer,do not need freed.
 */
char* tlv_str_convert2(const char* src,int len,const char* src_code,const char *dst_code)
{
#define N 1024
	static char buf[N];
	char *p;
	int ret;

	p=buf;
	memset(buf,0,N);
	ret=tlv_str_convert(src,len,src_code,dst_code,p,N);
	p=ret<0?0:buf;
	return p;
}

char* gbk_to_utf8(const char* gbk, int len)
{
	return tlv_str_convert2(gbk, len, "utf-8", "gb18030");
}

#endif

#endif
