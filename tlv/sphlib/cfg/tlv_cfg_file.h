#ifndef tlv_cfg_tlv_cfg_FILE_H_
#define tlv_cfg_tlv_cfg_FILE_H_
#include "tlv/struct/tlv_define.h"
#include "tlv/struct/tlv_heap.h"
#include "tlv/struct/tlv_str_hash.h"
#include "tlv/struct/tlv_queue.h"
#include "tlv/struct/tlv_strfile.h"
#include "tlv_part_cfg.h"
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct tlv_cfg_file tlv_cfg_file_t;
#define tlv_cfg_file_add_var_ks(cfg,k,v,vlen) tlv_cfg_file_add_var(cfg,k,sizeof(k)-1,v,vlen)
#define tlv_cfg_file_feed_s(cfg,data) tlv_cfg_file_feed(cfg,data,sizeof(data)-1)

typedef enum
{
	CF_EXPR_START,
	CF_EXPR_TOK_START,
	CF_EXPR_TOK_WAIT_EQ,
	CF_EXPR_VALUE_START,
	CF_EXPR_VALUE_TOK_START,
	CF_EXPR_VALUE_TOK_END,
	CF_VAR_START,
	CF_VAR_TOK,
	CF_VAR_TOK_START,
	CFG_ARRAY_START,
	CFG_ARRAY_TOK_START,
	CFG_ARRAY_TOK_END,
	CFG_COMMENT,
	CFG_ESCAPE_START,
	CFG_ESCAPE_X1,
	CFG_ESCAPE_X2,
	CFG_ESCAPE_O1,
	CFG_ESCAPE_O2,
}tlv_cfg_file_state_t;

struct tlv_cfg_file
{
	tlv_queue_t cfg_queue;
	tlv_heap_t *heap;
	tlv_part_cfg_t *main;	//main configure;
	tlv_part_cfg_t *cur;
	tlv_cfg_file_state_t state;
	tlv_cfg_file_state_t var_cache_state;
	tlv_charbuf_t *tok;
	tlv_charbuf_t *value;
	tlv_charbuf_t *var;
	//tlv_charbuf_t *comment;	//comment;
	tlv_array_t *array;
	int scope;
	char quoted_char;
	unsigned char escape_char;
	unsigned escaped:1;
	unsigned quoted:1;
	unsigned included:1;
};

tlv_cfg_file_t* tlv_cfg_file_new_fn(char *fn);
tlv_cfg_file_t* tlv_cfg_file_new_fn2(char *fn, int add_pwd);
tlv_cfg_file_t *tlv_cfg_file_new();
int tlv_cfg_file_bytes(tlv_cfg_file_t *cfg);

/**
 * @param buf_size bytes of buf used by cfg;
 * @param heap_size block of heap used by cfg;
 */
tlv_cfg_file_t *tlv_cfg_file_new_ex(int buf_size, int heap_size);

//int tlv_cfg_file_reset(tlv_cfg_file_t *cfg);
int tlv_cfg_file_delete(tlv_cfg_file_t *c);
int tlv_cfg_file_feed_fn(tlv_cfg_file_t* cfg,const char *fn);
int tlv_cfg_file_feed(tlv_cfg_file_t *c,char *d,int bytes);

/**
 *	v is end with 0. vbytes not with include 0
 */
int tlv_cfg_file_add_var(tlv_cfg_file_t *cfg,char *k,int kbytes,char *v,int vlen);
void tlv_cfg_file_print(tlv_cfg_file_t *c);
#ifdef __cplusplus
};
#endif
#endif
