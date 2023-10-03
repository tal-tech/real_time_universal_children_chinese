#ifndef TAL_STRING_TLV_STR_H_
#define TAL_STRING_TLV_STR_H_
#include "tlv/struct/tlv_define.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct tlv_string tlv_string_t;

#define tlv_string_end(s) ((s)->data+(s)->len)
#define tlv_string_set(str,d,bytes) (str)->data=d;(str)->len=bytes;

#define tlv_str_equal(s1,ns1,s2,ns2) ( (ns1==ns2) && (strncmp(s1,s2,ns1)==0))
#define tlv_string_equal(str1,str2) (((str1)->len==(str2)->len) && (strncmp((str1)->data,(str2)->data,(str1)->len)==0))

#define tlv_string(s) {s,sizeof(s)-1}
#define tlv_string_set_s(str,data) tlv_string_set(str,data,sizeof(data)-1)
#define tlv_string_cmp_s(str,s) tlv_string_cmp(str,s,sizeof(s)-1)
#define tlv_string_equal_s(str,s) (((str)->len == (sizeof(s)-1)) && (strncmp((str)->data,s,sizeof(s)-1)==0))
#define tlv_str_equal_s(s1,ns1,s2) tlv_str_equal(s1,ns1,s2,sizeof(s2)-1)

struct tlv_string
{
	char* data;
	int len;
};
tlv_string_t* tlv_string_new(int len);
tlv_string_t* tlv_string_dup_data(const char* data, int len);
int tlv_string_free(tlv_string_t *str);

int tlv_string_cmp(tlv_string_t *str, char* s, int len);
int tlv_string_cmp2(tlv_string_t *str1, tlv_string_t *str2);
int tlv_string_is_char_in(tlv_string_t *str, char c);

char* tlv_str_dup(const char* s);
long long tlv_str_atoi(char* s, int len);
double tlv_str_atof(char *s, int len);
char* tlv_str_chr(char* s, int slen, char c);
int tlv_str_str(char *src, int src_len, char *sub, int sub_len);

void print_hex(char *data,int len);
void print_data(char* data, int len);

int tlv_char_to_hex(char c);

uint32_t hash_string_value(char* s);
uint32_t hash_string(char* s, uint32_t hash_size);
uint32_t hash_string_value_len_seed(unsigned char* p, int len, int hash_size);
#define hash_string_value_len(p, len, item) hash_string_value_len_seed((unsigned char*)p,len,item)

/* for hmm context */
void tlv_string_get_midname(tlv_string_t *src, tlv_string_t *dst);

#ifdef __cplusplus
};
#endif

#endif  /* TAL_STRING_TLV_STR_H_ */
