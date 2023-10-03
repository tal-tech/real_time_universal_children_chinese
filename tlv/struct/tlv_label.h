#ifndef TAL_STRUCT_TLV_LABEL_H_
#define TAL_STRUCT_TLV_LABEL_H_
#include "tlv/struct/tlv_string.h"
#include "tlv/struct/tlv_str_hash.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlv_name_cell tlv_name_cell_t;
typedef struct tlv_label tlv_label_t;

#define tlv_label_find_s(l,s,i) tlv_label_find(l,s,sizeof(s)-1,i)

struct tlv_name_cell
{
	tlv_str_hash_node_t hash_n;
	tlv_string_t* name;
	void *data;
};

struct tlv_label
{
	tlv_heap_t     *heap;
	tlv_str_hash_t *hash;
};

tlv_label_t* tlv_label_new(int n);
int tlv_label_init(tlv_label_t* l, int n);
tlv_name_cell_t* tlv_label_find(tlv_label_t *l,char *s,int sl,int insert);
tlv_string_t* tlv_label_find2(tlv_label_t *l, char *s, int sl, int insert);
int tlv_label_reset(tlv_label_t *l);
int tlv_label_clean(tlv_label_t *l);
int tlv_label_delete(tlv_label_t *l);

#ifdef __cplusplus
};
#endif
#endif
