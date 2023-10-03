#ifndef TAL_SPHLIB_TLV_PART_CFG_H_
#define TAL_SPHLIB_TLV_PART_CFG_H_
#include <stdlib.h>
#include "tlv_arg.h"
#include "tlv/struct/tlv_define.h"
#include "tlv/struct/tlv_sys.h"
#include "tlv/struct/tlv_string.h"
#include "tlv/struct/tlv_heap.h"
#include "tlv/struct/tlv_str_hash.h"
#include "tlv_cfg_queue.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct tlv_part_cfg tlv_part_cfg_t;

#define tlv_part_cfg_update_cfg_str(lc,cfg,item,v) {v=tlv_part_cfg_find_string_s(lc,STR(item)); if(v){cfg->item=v->data;}}
#define tlv_part_cfg_update_cfg_str_local(lc,cfg,item,v) {v=tlv_part_cfg_find_string2_s(lc,STR(item),0); if(v){cfg->item=v->data;}}
#define tlv_part_cfg_update_cfg_string_v(lc,cfg,item,v) {v=tlv_part_cfg_find_string_s(lc,STR(item)); if(v){cfg->item=*v;}}
#define tlv_part_cfg_update_cfg_f(lc,cfg,item,v) {v=tlv_part_cfg_find_string_s(lc,STR(item)); if(v){cfg->item=atof(v->data);}}
#define tlv_part_cfg_update_cfg_i(lc,cfg,item,v) {v=tlv_part_cfg_find_string_s(lc,STR(item)); if(v){cfg->item=atoi(v->data);}}
#define tlv_part_cfg_update_cfg_b(lc,cfg,item,v) {v=tlv_part_cfg_find_string_s(lc,STR(item)); if(v){cfg->item=(atoi(v->data)==1)?1:0;}}
#define tlv_part_cfg_update_cfg_b_local(lc,cfg,item,v) {v=tlv_part_cfg_find_string2_s(lc,STR(item),0); if(v){cfg->item=(atoi(v->data)==1)?1:0;}}

#define tlv_part_cfg_update_cfg_f2(lc,cfg,item,key,v) {v=tlv_part_cfg_find_string_s(lc,STR(key)); if(v){cfg->item=atof(v->data);}}
#define tlv_part_cfg_update_cfg_i2(lc,cfg,item,key,v) {v=tlv_part_cfg_find_string_s(lc,STR(key)); if(v){cfg->item=atoi(v->data);}}
#define tlv_part_cfg_update_cfg_b2(lc,cfg,item,key,v) {v=tlv_part_cfg_find_string_s(lc,STR(key)); if(v){cfg->item=(atoi(v->data)==1)?1:0;}}

#define tlv_part_cfg_find_string_s(cfg,s) tlv_part_cfg_find_string(cfg, (char*)s,sizeof(s)-1)
#define tlv_part_cfg_find_string2_s(cfg,s,r) tlv_part_cfg_find_string2(cfg,s,sizeof(s)-1,r)
#define tlv_part_cfg_find_lc_s(cfg,s) tlv_part_cfg_find_lc(cfg,(char*)s,sizeof(s)-1)
#define tlv_part_cfg_find_array_s(cfg,s) tlv_part_cfg_find_array(cfg,s,sizeof(s)-1)
#define tlv_part_cfg_find_s(cfg,n) tlv_part_cfg_find(cfg,n,sizeof(n)-1)

#define print_cfg_i(cfg,s) printf("%s:\t%d\n",STR(s),cfg->s)
#define print_cfg_f(cfg,s) printf("%s:\t%f\n",STR(s),cfg->s)

struct tlv_part_cfg
{
	tlv_queue_node_t q_n;
	tlv_string_t name;
	tlv_cfg_queue_t *queue;
	tlv_heap_t *heap;
	tlv_part_cfg_t *parent;
};

tlv_part_cfg_t *tlv_part_cfg_new_h(tlv_heap_t *h);
tlv_cfg_item_t* tlv_part_cfg_find(tlv_part_cfg_t *cfg, char *name, int len);
tlv_string_t* tlv_part_cfg_find_string(tlv_part_cfg_t *cfg, char *name, int len);
tlv_string_t* tlv_part_cfg_find_string2(tlv_part_cfg_t *cfg, char *name, int len, int recursive);
tlv_array_t* tlv_part_cfg_find_array(tlv_part_cfg_t *cfg, char *name, int len);
tlv_part_cfg_t* tlv_part_cfg_find_lc(tlv_part_cfg_t *cfg, char *name, int len);
tlv_part_cfg_t* tlv_part_cfg_find_section_lc(tlv_part_cfg_t* lc, char *section, int section_bytes);

void tlv_part_cfg_print(tlv_part_cfg_t *lc);
void tlv_part_cfg_update_arg(tlv_part_cfg_t *lc,tlv_arg_t *arg,int show);
void tlv_part_cfg_value_to_string(tlv_part_cfg_t *lc,tlv_charbuf_t *buf);
#ifdef __cplusplus
};
#endif
#endif
