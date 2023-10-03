#ifndef TAL_SPHLIB_TLV_ARG_H_
#define TAL_SPHLIB_TLV_ARG_H_
#include "tlv/struct/tlv_define.h"
#include "tlv/struct/tlv_str_hash.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct tlv_arg tlv_arg_t;
#define tlv_arg_get_int_s(arg,k,n) tlv_arg_get_int(arg,k,sizeof(k)-1,n)
#define tlv_arg_get_number_s(arg,k,n) tlv_arg_get_number(arg,k,sizeof(k)-1,n)
#define tlv_arg_exist_s(arg,k) tlv_arg_exist(arg,(char*)k,sizeof(k)-1)
#define tlv_arg_get_str_s(arg,k,pv) tlv_arg_get_str(arg,(char*)k,sizeof(k)-1,pv)

typedef struct tlv_arg_item
{
	tlv_queue_node_t q_n;
	tlv_string_t k;
	tlv_string_t v;
}tlv_arg_item_t;

struct tlv_arg
{
	tlv_queue_t queue;
    tlv_heap_t *heap;
};

tlv_arg_t* tlv_arg_new(int argc, char** argv);
int tlv_arg_delete(tlv_arg_t *arg);
int tlv_arg_get_int(tlv_arg_t *arg, const char *key, int bytes, int* number);
int tlv_arg_get_number(tlv_arg_t *arg, const char *key, int bytes, double *n);
int tlv_arg_exist(tlv_arg_t *arg, const char* key, int bytes);
int tlv_arg_get_str(tlv_arg_t *arg, const char *key, int bytes, char** pv);

#ifdef __cplusplus
};
#endif
#endif
