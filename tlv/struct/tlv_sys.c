#include "tlv_sys.h"
#include <ctype.h>
#ifdef WIN32
#include <windows.h>
#include <time.h>
#include <sys/timeb.h>
#include <locale.h>

#define localtime_r localtime
#define   _WIN32_WINNT   0x0500

#else
#include <sys/time.h>
#define __USE_GNU
#include <sys/resource.h>
#endif


int tlv_mkdir(char* dn)
{
#ifdef WIN32
	return (tlv_file_exist(dn)==0) ? 0 : mkdir(dn);
#else
	return (tlv_file_exist(dn)==0) ? 0 : mkdir(dn,0777);
#endif
}

int tlv_mkdir_p(const char* fn,char sep,int create_last_entry)
{
	char *p,*t;
	int ret;

	ret=-1;
	p=tlv_str_dup(fn);
	if(!p){goto end;}
	for(t=p;;++t)
	{
		if( (*t==sep) || (*t==0) )
		{
			if(*t==0 && !create_last_entry)
			{
				break;
			}else if(t==p)
			{
				continue;
			}
			*t=0;
			ret=tlv_mkdir(p);
			if(ret!=0)
			{
				perror(__FUNCTION__);
				printf("%d:[%s] create failed.\n",ret,p);
				goto end;
			}
			if( *(fn+(t-p)) ==  DIR_SEP)
			{
				*t=DIR_SEP;
			}else
			{
				break;
			}
		}
	}
	ret=0;
end:
	if(p){free(p);}
	return ret;
}

tlv_string_t tlv_dir_name2(char *data,int len,char sep)
{
	char *s,*e;
	tlv_string_t v;

	s=data;e=s+len-1;
	while(e>s)
	{
		if(*e==sep){break;}
		--e;--len;
	}
	tlv_string_set(&v,data,len);
	return v;
}


tlv_string_t* tlv_dir_name(const char *fn,char sep)
{
	int len;
	const char *s,*e;
	tlv_string_t *str;

	len=strlen(fn);
	s=fn;e=s+len;
	while(e>s)
	{
		if(*e==sep){break;}
		--e;--len;
	}
	//printf("%d:%*.*s#\n",len,len,len,fn);
	str=tlv_string_dup_data(fn,len);
	return str;
}


#ifdef WIN32
double time_get_cpu()
{
    LARGE_INTEGER lt,ft;
    double v;

    QueryPerformanceFrequency(&ft);
    QueryPerformanceCounter(&lt);
    v=lt.QuadPart*1000.0/ft.QuadPart;
    return v;
}

double time_get_ms()
{
    struct timeb tm;
    double ret;

    ftime(&tm);
    ret=tm.time*1000;
    ret+=tm.millitm;
    return ret;
}
#else

double time_get_cpu()
{
#if defined __IPHONE_OS__ || defined __ANDROID__ || __APPLE__
	return time_get_ms();
#else
	struct rusage u;
	double v;
	int ret;

	ret=getrusage(RUSAGE_THREAD,&u);
	if(ret==0)
	{
		v=u.ru_utime.tv_sec*1000.0+u.ru_utime.tv_usec*1.0/1000
			+u.ru_stime.tv_sec*1000.0+u.ru_stime.tv_usec*1.0/1000;
	}else
	{
		v=0;
	}
	return v;
#endif
}

double time_get_ms()
{
    struct timeval tv;
    double ret;
    int err;

    err=gettimeofday(&tv,0);
    if(err==0)
    {
        ret=tv.tv_sec*1000.0+tv.tv_usec*1.0/1000;
        //ret maybe is NAN
        if(ret!=ret)
        {
            tlv_log("NAN(%.0f,sec=%.d,usec=%.d).\n",ret,(int)tv.tv_sec,(int)tv.tv_usec);
            ret=0;
        }
    }else
    {
        perror(__FUNCTION__);
        ret=0;
    }
    return ret;
}

#if defined __IPHONE_OS__ || defined __ANDROID__
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

int tlv_dir_walk(char *dir,void *ths,tlv_dir_walk_f walk)
{
	DIR *d;
	struct dirent *ent;
	int ret;
	tlv_charbuf_t *buf;
	tlv_string_t v;

	//tlv_log("import dir:%s\n",dir);
	buf=tlv_charbuf_new(1024,1);
	d=opendir(dir);
	if(!d){ret=-1;goto end;}
	ret=0;
	while(1)
	{
		ent=readdir(d);
		if(!ent){break;}
		//tlv_log("[%s]\n",ent->d_name);
		if(ent->d_type==DT_REG)
		{
			tlv_charbuf_reset(buf);
			tlv_charbuf_push_string(buf,dir);
			tlv_charbuf_push_c(buf,'/');
			tlv_charbuf_push_string(buf,ent->d_name);
			v.data=buf->data;
			v.len=buf->pos;
			tlv_charbuf_push_c(buf,0);
			ret=walk(ths,&(v));
		}else if(ent->d_type==DT_DIR)
		{
			if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
			{
				ret=0;
			}else
			{
				tlv_charbuf_reset(buf);
				tlv_charbuf_push_string(buf,dir);
				tlv_charbuf_push_c(buf,'/');
				tlv_charbuf_push_string(buf,ent->d_name);
				tlv_charbuf_push_c(buf,0);
				//tlv_log("%s[%s:%s]\n",buf->data,ent->d_name,dir);
				ret=tlv_dir_walk(buf->data,ths,walk);
			}
		}else
		{
			ret=0;
		}
		//tlv_log("ret=%d\n",ret);
		if(ret!=0){goto end;}
	}
end:
	//exit(0);
	if(d)
	{
		closedir(d);
	}
	//tlv_log("ret=%d\n",ret);
	tlv_charbuf_delete(buf);
	return ret;
}

#endif

#endif

uint64_t file_length(FILE *f)
{
	uint64_t len;

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);
    return len;
}

uint64_t tlv_file_size(char *fn)
{
	uint64_t fs=0;
	FILE *f;

	f=fopen(fn,"rb");
	if(f)
	{
		fs=file_length(f);
		fclose(f);
	}
	return fs;
}

char* file_read_buf(const char* fn, int *n)
{
	FILE* file = fopen(fn, "rb");
	char* p = 0;
	int len;

	if (file)
	{
        len=file_length(file);
		p = (char*) tlv_malloc(len + 1);
		len = fread(p, 1, len, file);
		if (n)
		{
			*n = len;
		}
		fclose(file);
		p[len] = 0;
	}
	return p;
}

#define FILE_STATE_NOERR 0
#define FILE_STATE_END -1
#define FILE_STATE_ERR -2

int file_state(FILE* file)
{
	int state;

	if (feof(file))
	{
		state = FILE_STATE_END;
	}
	else if (ferror(file))
	{
		state = FILE_STATE_ERR;
	}
	else
	{
		state = FILE_STATE_NOERR;
	}
	return state;
}

int file_write(FILE* file, const char* data, int len, int* writed)
{
	int ret = 0, index = 0, nLeft = len;

	if (!data)
	{
		return -1;
	}
	while (nLeft > 0)
	{
		ret = fwrite(&data[index], 1, nLeft, file);
		if (ret < nLeft)
		{
			ret = file_state(file);
			if (ret != FILE_STATE_NOERR)
			{
				break;
			}
		}
		index += ret;
		nLeft -= ret;
	}
	if (writed)
	{
		*writed = index;
	}
	return ret;
}

int file_write_buf(const char* fn, const char* data, size_t len)
{
	FILE* file;
	int ret = -1;

	tlv_mkdir_p(fn,DIR_SEP,0);
	file = fopen(fn, "wb");
	if (file)
	{
		ret = file_write(file, data, len, NULL);
		fclose(file);
		ret=ret==len?0:-1;
	}
	return ret;
}

#ifdef WIN32
//int tlv_dir_walk(const char* path, tlv_dir_walk_handler_t cb,void* user_data)
int tlv_dir_walk(char *dir, void *ths, tlv_dir_walk_f walk)
{
    char buf[2048];
    WIN32_FIND_DATA data;
    HANDLE hFind;
    int    ret;
    char*  p = buf;
    int    p_size = sizeof(buf);
    size_t count;

    count = strlen(dir) + 3;
    if(count > p_size)
    {
        p_size = count<<1;
        p = (char*)tlv_malloc(sizeof(char)*p_size);
    }
    count = _snprintf(p, p_size, "%s\\*", dir);
    hFind=FindFirstFile(p, &data);
    if(hFind != INVALID_HANDLE_VALUE)
    {
        ret = 0;
        do
        {
            count=strlen(data.cFileName);
            count=_snprintf(p,p_size,"%s\\%s", dir, data.cFileName);
            if(count>=p_size||count<0)
            {
                if(p!=buf)
                {
                    free(p);
                }
                p_size = strlen(dir)+strlen(data.cFileName)+3;
                p = (char*)tlv_malloc(sizeof(char)*p_size);
                count=_snprintf(p,p_size,"%s\\%s",dir,data.cFileName);
            }

            if(data.dwFileAttributes==FILE_ATTRIBUTE_DIRECTORY)
            {
                if(strcmp(data.cFileName,".")!=0&&strcmp(data.cFileName,"..")!=0)
                {
                    ret = tlv_dir_walk(p, ths, walk);
                    if(ret!=0)
                    {
                        break;
                    }
                }
            }else if(walk)
            {
                if(walk(ths, p)!=0)
                {
                    ret=-1;
                    break;
                }
            }
        }while(FindNextFile(hFind,&data));
        FindClose(hFind);
    }else
    {
        ret=-1;
    }
    if(p!=buf)
    {
        free(p);
    }
    return ret;
}

char* wcs_to_mbs(const wchar_t* wcs,int *ret_count)
{
    char* mbs=0;
    int c;

    if(wcs)
    {
        c=0;
        c=wcstombs(NULL,wcs,c);
        if(c>0)
        {
            c+=1;
            mbs=(char*)tlv_calloc(1,c);
            c=wcstombs(mbs,wcs,c-1);
            if(c==-1)
            {
                free(mbs);
                mbs=0;
            }
            if(ret_count)
            {
                *ret_count=c;
            }
        }
    }
    if(!mbs && ret_count)
    {
        *ret_count=0;
    }
    return mbs;
}

#endif


char* tlv_realpath(char *fn,char *buf)
{
#ifdef WIN32
    //GetFullPathNameA(fn,4096,buf,0);
    return 0;
#else
	return realpath(fn,buf);
#endif
}

tlv_string_t* tlv_dirname(char *fn,char sep)
{
	tlv_string_t *p=0;

	if(!fn){goto end;}
	p=tlv_str_left(fn,strlen(fn),sep);
end:
	return p;
}

tlv_string_t* tlv_str_right(char* fn,int len,char sep)
{
	tlv_string_t *p=0;
	char *s,*e,*t;

	if(!fn){goto end;}
	s=fn;e=fn+len-1;
	t=e;
	while(t>s)
	{
		if(*t==sep)
		{
			++t;
			break;
		}
		--t;
	}
	p=tlv_string_dup_data(t,e-t+1);
end:
	return p;
}

tlv_string_t* tlv_str_left(char *fn,int len,char sep)
{
	tlv_string_t *p=0;
	char *s,*e;

	if(!fn){goto end;}
	s=fn;e=fn+len-1;
	while(e>=s)
	{
		if(*e==sep)
		{
			p=tlv_string_dup_data(s,e-s);
			break;
		}
		--e;
	}
end:
	return p;
}

FILE* tlv_file_open(char* fn,char * mode)
{
	FILE* f;
	int ret;

	f=0;
#ifdef WIN32
    ret=tlv_mkdir_p(fn,'\\',0);
#else
	ret=tlv_mkdir_p(fn,'/',0);
#endif
	if(ret!=0){goto end;}
	f=fopen(fn,mode);
end:
	return f;
}

/*---------- qsort -------------*/
void tlv_qsort(void *s, void *e, size_t size, tlv_qsort_cmp_f cmp, void *ths, void *tmp_elem)
{
	char *x;
	char *i, *j;

	if(e<=s){return;}
	x = (char*)e;
	i = (char*)s-size;
	j = (char*)s;
	while(j<x)
	{
		if(cmp(ths, (void*)j, (void*)x)<=0)
		{
			i+=size;
			if(j!=i)
			{
				memcpy(tmp_elem,i,size);
				memcpy(i,j,size);
				memcpy(j,tmp_elem,size);
			}
		}
		j+=size;
	}
	i+=size;
	if(i!=x)
	{
		memcpy(tmp_elem,i,size);
		memcpy(i,x,size);
		memcpy(x,tmp_elem,size);
	}
	tlv_qsort(s, i-size, size, cmp, ths, tmp_elem);
	tlv_qsort(i+size, e, size, cmp, ths, tmp_elem);
}

void tlv_qsort2(void *base, size_t nmemb, size_t size, tlv_qsort_cmp_f cmp, void *ths)
{
	void *tmp_elem;

	tmp_elem = malloc(size);
	tlv_qsort(base, (void*)((char*)base+(nmemb-1)*size), size, cmp, ths, tmp_elem);
	free(tmp_elem);
}

void tlv_qsort3(void *base, size_t nmemb, size_t size, tlv_qsort_cmp_f cmp, void *app_data, void *tmp_elem)
{
	tlv_qsort(base, (void*)((char*)base+(nmemb-1)*size), size, cmp, app_data, tmp_elem);
}
