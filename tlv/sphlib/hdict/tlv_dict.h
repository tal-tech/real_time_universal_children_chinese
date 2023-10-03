#ifndef TAL_SPHLIB_TLV_DICT_H_
#define TAL_SPHLIB_TLV_DICT_H_

#include "tlv/struct/tlv_label.h"
#include "tlv/struct/tlv_array.h"
#include "tlv/struct/tlv_strfile.h"
#include "tlv/sphlib/hmath/tlv_math.h"
//#include "tlv/sphlib/hmm/tlv_gmminfo.h"
#ifdef __cplusplus
extern "C" {
#endif

#define MINPRONPROB 1E-6

typedef struct tlv_dict tlv_dict_t;
typedef struct tlv_dict_phone tlv_dict_phone_t;
typedef	struct tlv_dict_word tlv_dict_word_t;
typedef struct tlv_dict_pron tlv_dict_pron_t;
typedef tlv_dict_word_t* (*tlv_dict_word_find_f)(void *data,char *w,int w_bytes);

typedef enum
{
	TLV_PHN_CD=0,
	TLV_PHN_CI,		// sil
	TLV_PHN_CF,		// sp
}tlv_phn_type_t;

struct tlv_dict_phone
{
	tlv_string_t *name;
	tlv_phn_type_t type;
};

/* pronunciation */
struct tlv_dict_pron
{
	tlv_dict_word_t *word;
	tlv_dict_pron_t *next;
	tlv_string_t *outsym;
	tlv_dict_phone_t **pPhones;
	int nPhones;
	int pnum;		///* number of the pronunciation */
	float prob;
	void *aux;      // spare;
};

struct tlv_dict_word
{
	tlv_string_t *name;
	tlv_dict_pron_t *pron_list;
	int npron;
	unsigned in_dict:1;  /* is oov */
	void *aux;
};

struct tlv_dict
{
	tlv_heap_t  *heap;
	tlv_label_t *label;
	tlv_str_hash_t* phone_hash;
	tlv_str_hash_t* word_hash;
	tlv_dict_pron_t	*null_pron;
	tlv_dict_word_t *null_word;
	int 	npron;
	int 	nword;
	int		nphone;
	unsigned use_db:1;
};

tlv_dict_t* tlv_dict_new(tlv_label_t *l, int use_db);
int tlv_dict_delete(tlv_dict_t *d);
int tlv_dict_init(tlv_dict_t *d, tlv_label_t *label, int use_db, int phn_hash_hint,int wrd_hash_hint);
int tlv_dict_load(tlv_dict_t *d, tlv_strfile_t *s);
int tlv_dict_clean(tlv_dict_t *d);
int tlv_dict_reset(tlv_dict_t *d);

int tlv_dict_add_word_sent_flag(tlv_dict_t *d);
tlv_dict_phone_t* tlv_dict_get_phone(tlv_dict_t* d, tlv_string_t *phn, int insert);
tlv_dict_word_t* tlv_dict_get_word(tlv_dict_t *d, tlv_string_t *name, int insert);
tlv_dict_word_t* tlv_dict_get_word2(tlv_dict_t *d,tlv_string_t *name, int insert);
tlv_dict_word_t* tlv_dict_find_word(tlv_dict_t *d,char* n,int nl);
//int tlv_dict_is_closed(tlv_dict_t *d, tlv_gmminfo_t *hl);

tlv_dict_pron_t* tlv_dict_add_pron(tlv_dict_t *d, tlv_dict_word_t *word, tlv_string_t *outsym, tlv_string_t **phones, int nphones, float prob);
tlv_dict_word_t* tlv_dict_get_dummy_wrd(tlv_dict_t *d, char *w, int bytes);

void tlv_dict_word_print(tlv_dict_word_t *dw);
void tlv_dict_pron_print(tlv_dict_pron_t *pron);

#ifdef __cplusplus
};
#endif
#endif
