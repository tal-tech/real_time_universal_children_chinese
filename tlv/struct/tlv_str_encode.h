#ifndef TAL_STRUCT_TLV_STR_ENCODE_H_
#define TAL_STRUCT_TLV_STR_ENCODE_H_
#include "tlv/struct/tlv_define.h"
#include "tlv/struct/tlv_heap.h"
#include "tlv/struct/tlv_array.h"
#ifdef __cplusplus
extern "C" {
#endif

int str_is_utf8(const unsigned char* utf, int len);
int tlv_utf8_bytes(char c);

#ifdef WIN32
char* gbk_to_utf8(const char* gbk);
#else
char* gbk_to_utf8(const char* gbk, int len);
#endif /* #ifdef WIN32 */

#ifdef __cplusplus
};
#endif
#endif
