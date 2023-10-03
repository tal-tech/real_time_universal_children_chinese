#ifndef TAL_SPHLIB_TLV_PACK2_H_
#define TAL_SPHLIB_TLV_PACK2_H_
#include "tlv/sphlib/pack/tlv_pack.h"
#include "tlv/struct/tlv_heap.h"
#include "tlv/struct/tlv_sys.h"
#include "tlv/struct/tlv_strfile.h"
#ifdef __cplusplus
extern "C" {
#endif

#define TPACK_BACKUP_LEN 199
typedef struct tlv_pack2 tlv_pack2_t;
#define tlv_pack2_get_s(rb,name) tlv_pack2_get(rb,name,sizeof(name)-1)
#define tlv_pack2_get2_s(rb,name) tlv_pack2_get2(rb,name,sizeof(name)-1)

typedef struct
{
	tlv_queue_node_t q_n;
	tlv_pack2_t *rb;
	tlv_string_t *fn;			/* file name */
	tlv_string_t *data;
	int pos;		            /* file pos  */
	int len;		            /* data len  */
	unsigned char *s;
	unsigned char *e;
	int seek_pos;	            /* seek pos */
	int buf_pos;	            /* buf pos  */
	unsigned reverse:1;
}tlv_pack2_item_t;

struct tlv_pack2
{
	tlv_queue_t list;
	tlv_heap_t *heap;
	FILE *f;
	char *fn;
	tlv_charbuf_t *buf;
};

tlv_pack2_t* tlv_pack2_new();
void tlv_pack2_delete(tlv_pack2_t *pack);

//-------------------- write  --------------------------------
int tlv_pack2_add(tlv_pack2_t *pack, tlv_string_t *name, char *realname);
void tlv_pack2_add2(tlv_pack2_t *pack, tlv_string_t *name, char *data, int len);
int tlv_pack2_write(tlv_pack2_t *pack, char *fn);

//------------------ read -------------------------
int tlv_pack2_read(tlv_pack2_t *pack, char *fn);
int tlv_pack2_extract(tlv_pack2_t *pack, char *dn);

tlv_pack2_item_t* tlv_pack2_get(tlv_pack2_t *pack, char *name, int len);
tlv_pack2_item_t* tlv_pack2_get2(tlv_pack2_t *pack, char *name, int len);
int tlv_pack2_load_item(tlv_pack2_t *pack, tlv_pack2_item_t *item, int use_heap);
void tlv_pack2_print(tlv_pack2_t *pack);


int tlv_pack2_load_file(tlv_pack2_t *pack, void *data, tlv_strfile_load_handler_t loader,char *fn);
void tlv_strfile_init_rbin2(tlv_strfile_t* s,tlv_pack2_item_t *i);
void tlv_strfile_init_rbin2_x(tlv_strfile_t* s,tlv_pack2_item_t *i);
#ifdef __cplusplus
};
#endif
#endif
