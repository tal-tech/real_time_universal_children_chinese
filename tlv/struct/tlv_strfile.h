#ifndef TAL_STRUCT_TLV_STRFILE_H_
#define TAL_STRUCT_TLV_STRFILE_H_
#include "tlv/struct/tlv_string.h"
#include "tlv/struct/tlv_charbuf.h"
#include "tlv/struct/tlv_str_encode.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct tlv_strfile tlv_strfile_t;
typedef struct tlv_strfile_loader tlv_strfile_loader_t;
typedef int (*tlv_strfile_get_handler_t)(void*);
typedef int (*tlv_strfile_unget_handler_t)(void*,int);

typedef int (*tlv_strfile_get_str_f)(void*,char *buf,int bytes);
typedef int (*tlv_strfile_read_str_f)(void*,tlv_charbuf_t *buf);
typedef tlv_string_t* (*tlv_strfile_get_file_f)(void*);

typedef int (*tlv_strfile_load_handler_t)(void *data_ths,tlv_strfile_t *s);
typedef int (*tlv_strfile_loader_v_t)(void* inst,void *data,tlv_strfile_load_handler_t loader,char *fn);

#define tlv_strfile_get(s) ((s)->get((s)->data))
#define tlv_strfile_unget(s,c) ((s)->unget((s)->data,c))
#define tlv_strfile_seek_to_s(s,d) tlv_strfile_seek_to(s,d,sizeof(d)-1)

struct tlv_strfile
{
	void *data;
	unsigned char swap:1;

	tlv_strfile_get_str_f get_str;
	tlv_strfile_read_str_f read_str;
	tlv_strfile_get_file_f get_file;

	tlv_strfile_get_handler_t get;
	tlv_strfile_unget_handler_t unget;
};

struct tlv_strfile_loader
{
	void *hook;
	tlv_strfile_loader_v_t vf;
};

int tlv_is_little_endian();
void tlv_swap_int32(int *i);
void tlv_swap_short(short *p);

void tlv_strfile_init(tlv_strfile_t *s);

int tlv_strfile_init_file(tlv_strfile_t *s, char *fn);
int tlv_strfile_clean_file(tlv_strfile_t *s);

int tlv_strfile_init_str(tlv_strfile_t *s, const char *data, int bytes);
int tlv_strfile_clean_str(tlv_strfile_t *s);
int tlv_strfile_fill(tlv_strfile_t* s,char* data,int len);

int tlv_strfile_read_string(tlv_strfile_t *s, tlv_charbuf_t *b);
int tlv_strfile_read_string2(tlv_strfile_t *s, tlv_charbuf_t *b);
int tlv_strfile_read_line(tlv_strfile_t *s, tlv_charbuf_t *b);
int tlv_strfile_read_line2(tlv_strfile_t *s, tlv_charbuf_t *b, int *eof);
int tlv_strfile_skip_sp(tlv_strfile_t *s, int *nl);
int tlv_strfile_peek(tlv_strfile_t *s);

int tlv_strfile_read_short(tlv_strfile_t* s, short* v, int n, int bin);
int tlv_strfile_read_int(tlv_strfile_t *s, int* v, int n, int bin);
int tlv_strfile_read_float(tlv_strfile_t *s, float *f, int n, int bin);

int tlv_strfile_atof(tlv_strfile_t* s, double *v);
int tlv_strfile_atoi(tlv_strfile_t* s, int* value);

int tlv_strfile_load_file(void *data,tlv_strfile_load_handler_t loader,char *fn);
int tlv_strfile_load_file_v(void *hook,void *data,tlv_strfile_load_handler_t loader,char *fn);
int tlv_strfile_loader_load(tlv_strfile_loader_t *l,void *data_ths,tlv_strfile_load_handler_t loader,char *fn);

int tlv_strfile_get_lines(int *nw, tlv_strfile_t *s);
int tlv_strfile_loader_file_lines(tlv_strfile_loader_t *sl, char* fn);

int tlv_file_write_float(FILE *file,float *f,int n,int bin,int swap);

tlv_string_t* tlv_strfile_read_file(tlv_strfile_t *s);

#ifdef __cplusplus
};
#endif
#endif
