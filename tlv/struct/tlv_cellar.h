#ifndef TAL_STRUCT_TLV_CELLAR_H_
#define TAL_STRUCT_TLV_CELLAR_H_
#include "tlv/struct/tlv_define.h"
#include "tlv_queue.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct tlv_cellar  tlv_cellar_t;
typedef int (*tlv_cellar_bytes_f)(void *data);

struct tlv_cellar
{
	tlv_queue_node_t	  *free;
	tlv_queue_node_t	  *use;
	void 		          *user_data;
	tlv_new_handler_f 	  newer;
	tlv_delete_handler_f  deleter;
	tlv_delete_handler2_f deleter2;
	int			          offset;
    unsigned int	      max_free;
    unsigned int	      cur_free;
    unsigned int          use_length;
};

int tlv_cellar_init(tlv_cellar_t *c, int offset, int max_free, tlv_new_handler_f newer,
		tlv_delete_handler_f deleter, void *data);
int tlv_cellar_init2(tlv_cellar_t *c, int offset, int max_free, tlv_new_handler_f newer,
		tlv_delete_handler2_f deleter, void *data);

void tlv_cellar_reset(tlv_cellar_t *c);
int tlv_cellar_clean(tlv_cellar_t *c);

void* tlv_cellar_pop(tlv_cellar_t *c);
int tlv_cellar_push(tlv_cellar_t *c, void* data);
void tlv_cellar_reuse(tlv_cellar_t *c);

int tlv_cellar_bytes(tlv_cellar_t *c, tlv_cellar_bytes_f bf);
#ifdef __cplusplus
};
#endif
#endif
