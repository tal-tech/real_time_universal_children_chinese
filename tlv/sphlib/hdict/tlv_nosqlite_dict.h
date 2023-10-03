#ifndef tlv_nosqlite_dict_H_
#define tlv_nosqlite_dict_H_

#include "tlv/sphlib/hdict/tlv_dict.h"

#ifdef __cplusplus
extern "C" {
#endif

struct  tlv_nosqlite_dict;

struct tlv_nosqlite_dict* tlv_nosqlite_dict_new(const char* fn);
int tlv_nosqlite_dict_delete(struct tlv_nosqlite_dict *ns);
int tlv_nosqlite_dict_reset(struct tlv_nosqlite_dict *ns);
struct tlv_dict_word* tlv_nosqlite_dict_get_word(struct tlv_nosqlite_dict *ns, const char *word, int len);
struct tlv_dict* tlv_nosqlite_dict_get_dict(struct tlv_nosqlite_dict *ns);

#ifdef __cplusplus
}
#endif
#endif
