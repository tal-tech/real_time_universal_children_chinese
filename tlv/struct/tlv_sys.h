#ifndef TAL_STRUCT_TLV_INIT_H_
#define TAL_STRUCT_TLV_INIT_H_
#include <stdio.h>
#include "tlv_define.h"
#include "tlv_string.h"
#include "tlv_charbuf.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#include <stdlib.h>
#ifdef WINCE
#include <ceconfig.h>
#else
#include <direct.h>
#include <io.h>
#endif

#define F_OK 0
#define tlv_file_access(fn,m) _access(fn,m)
#define tlv_file_exist(fn) tlv_file_access(fn,F_OK)
#else

#define tlv_file_access(fn,m) access(fn,m)
#define tlv_file_exist(fn) tlv_file_access(fn,F_OK)

#endif

#ifdef WIN32
#define DIR_SEP '\\'
typedef int(*tlv_dir_walk_f)(void *ths,tlv_string_t *fn);
int tlv_dir_walk(char *dir,void *ths,tlv_dir_walk_f walk);

#else

#define DIR_SEP '/'

#if defined __IPHONE_OS__ || defined __ANDROID__
#else
typedef int(*tlv_dir_walk_f)(void *ths,tlv_string_t *fn);
int tlv_dir_walk(char *dir,void *ths,tlv_dir_walk_f walk);
#endif

#endif


int tlv_mkdir(char* dn);
int tlv_mkdir_p(const char* fn,char sep,int create_last_entry);
tlv_string_t* tlv_dir_name(const char *fn,char sep);
tlv_string_t tlv_dir_name2(char *data,int len,char sep);

char* tlv_realpath(char *fn,char *buf);

tlv_string_t* tlv_dirname(char *fn,char sep);
tlv_string_t* tlv_str_right(char* fn,int len,char sep);
tlv_string_t* tlv_str_left(char *fn,int len,char sep);
tlv_string_t* tlv_basename(char* fn,char sep);


FILE* tlv_file_open(char* fn,char * mode);
uint64_t file_length(FILE *f);
uint64_t tlv_file_size(char *fn);

char* file_read_buf(const char* fn, int *n);
int file_write_buf(const char* fn, const char* data, size_t len);

/*------- get cpu time  ----*/
double time_get_ms();
double time_get_cpu();

/*-------  sort  ------*/
typedef float (*tlv_qsort_cmp_f)(void *ths, void *src, void *dst);
void tlv_qsort(void *s, void *e, size_t size, tlv_qsort_cmp_f cmp, void *ths, void *tmp_elem);
void tlv_qsort2(void *base, size_t nmemb, size_t size, tlv_qsort_cmp_f cmp, void *ths);
void tlv_qsort3(void *base, size_t nmemb, size_t size, tlv_qsort_cmp_f cmp, void *app_data, void *tmp_elem);

#ifdef __cplusplus
};
#endif
#endif
