#ifndef TAL_SPHLIB_TLV_PACK_H_
#define TAL_SPHLIB_TLV_PACK_H_
#include "tlv/struct/tlv_define.h"
#include "tlv/struct/tlv_sys.h"
#include "tlv/struct/tlv_strfile.h"
#include "tlv_flist.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct tlv_pack tlv_pack_t;
//#define tlv_pack_find_s(rb,s) tlv_pack_find(rb,s,sizeof(s)-1)
/**
 * HDR_ENTRYS(8)
 * ENTRY: FN_SIZE:FN:OFFSET:LENGTH
 */
typedef struct
{
	tlv_queue_node_t q_n;
	tlv_string_t *fn;
	tlv_string_t data;
	int pos;
}tlv_packitem_t;

struct tlv_pack
{
	tlv_queue_t list;
	tlv_flist_t *fl;
	tlv_charbuf_t *buf;
};

tlv_pack_t* tlv_pack_new();
int tlv_pack_delete(tlv_pack_t *pack);
int tlv_pack_read_scp(tlv_pack_t *pack, char *fn);
int tlv_pack_write(tlv_pack_t *pack, char *res_dn, char *bin);
int tlv_pack_read(tlv_pack_t *pack, char *bin);
int tlv_pack_extract(tlv_pack_t *pack, char *dn);
tlv_packitem_t* tlv_pack_find(tlv_pack_t *pack, char *data, int len);

int tlv_packitem_get(tlv_packitem_t *i);
int tlv_packitem_unget(tlv_packitem_t *i, int c);
void tlv_packitem_print(tlv_packitem_t *i);

int tlv_pack_load_file(tlv_pack_t *pack, void *data, tlv_strfile_load_handler_t loader,char *fn);
void tlv_file_write_reverse(FILE* f, char *data, int len);

void tlv_pack_reverse_data(unsigned char *p, int len);

#ifdef __cplusplus
};
#endif
#endif
