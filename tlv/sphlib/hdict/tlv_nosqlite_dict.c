#include "tlv_nosqlite_dict.h"
#include "third/nosqlite/nosqlite.h"
#include "tlv/sphlib/hdict/tlv_dict.h"

struct tlv_nosqlite_dict {
    tlv_label_t *label;
    tlv_dict_t *dict;
    struct nosqlite *db;
};

struct tlv_nosqlite_dict *
tlv_nosqlite_dict_new(const char* fn)
{
    int rv = -1;

    struct tlv_nosqlite_dict *ns;

    ns = (struct tlv_nosqlite_dict *)calloc(1, sizeof(struct tlv_nosqlite_dict));
    ns->label = tlv_label_new(1033);
    ns->dict = tlv_dict_new(ns->label, 1);

    ns->db = nosqlite_open(fn, 100000);
    if (!ns->db) {
        fprintf(stderr, "failed to open nosqlite db: %s\n", fn);
        goto end;
    }

    rv = 0;
end:
    if (rv && ns) {
        tlv_nosqlite_dict_delete(ns);
        ns = NULL;
    }

    return ns;
}


int
tlv_nosqlite_dict_delete(struct tlv_nosqlite_dict *ns)
{
    if (ns->db) {
        nosqlite_close(ns->db);
    }

    if (ns->dict) {
        tlv_dict_delete(ns->dict);
    }

    if (ns->label) {
        tlv_label_delete(ns->label);
    }

    free(ns);
    return 0;
}


int
tlv_nosqlite_dict_reset(struct tlv_nosqlite_dict *ns)
{
	tlv_dict_reset(ns->dict);
    return 0;
}


static struct tlv_dict_word *
_set_and_get_dict_word(struct tlv_nosqlite_dict *ns, const char *word, int wlen, const char *pron, int plen)
{
    int i, j;
    char *p, *pend;

    tlv_string_t *ps[512];
    tlv_heap_t *heap = ns->dict->heap;

    tlv_string_t *w;
    tlv_dict_word_t*  dw=0;

    w = tlv_heap_dup_string(heap, (char *)word, wlen);
    dw = tlv_dict_get_word(ns->dict, w, 1);

    for (p = (char *)pron, pend = (char *)pron + plen - 1, i = 0, j = 0; p <= pend; ++p) {
        if (*p == '|') {
            if (i != 0) {
                ps[j++] = tlv_heap_dup_string(heap, p - i, i);
                i = 0;
            }
            if (j != 0) {
                tlv_dict_add_pron(ns->dict, dw, 0, ps, j, -1.0f);
                j = 0;
            }
        } else if (*p == ' ' || *p == '\t') {
            if (i != 0) {
                ps[j++] = tlv_heap_dup_string(heap, p - i, i);
                i = 0;
            }
        } else {
            ++i;
        }

        if (p == pend) {
            if (i != 0) {
                ps[j++] = tlv_heap_dup_string(heap, p - i + 1, i);
            }
            if (j != 0) {
                tlv_dict_add_pron(ns->dict, dw, 0, ps, j, -1.0f);
            }
        }
    }

    return dw;
}


/**
 * @data	2018-6-6
 * @auth	jfyuan
 * @brief   针对数据库第三个版本进行特定的解析
 */
char *mono_phns[] = {"sp", "aa", "ae", "ah", "ao", "aw", "ax", "axr", "ay", "b", "ch",
	             "d", "dh", "ea", "eh", "er", "ey", "f", "g", "hh", "ia",
			     "ih", "iy", "jh", "k", "l", "m", "n", "ng", "oh", "ow", "oy",
				 "p", "r", "s", "sh", "sil", "t", "th", "ua", "uh", "uw", "v",
				 "w", "y", "z", "zh"};
static struct tlv_dict_word *
_set_and_get_dict_word3(struct tlv_nosqlite_dict *ns, const char *word, int wlen, const char *pron, int plen)
{
    int i;
    char *p, *pend;

    tlv_string_t *ps[512];
    tlv_string_t sp = tlv_string("sp"), sil = tlv_string("sil");
    tlv_heap_t *heap = ns->dict->heap;

    tlv_string_t *w;
    tlv_dict_word_t*  dw=0;

    w = tlv_heap_dup_string(heap, (char *)word, wlen);
    dw = tlv_dict_get_word(ns->dict, w, 1);

    for (p = (char *)pron, pend = (char *)pron + plen - 1, i = 0; p <= pend; ++p) {
        if (*p == '|') {

            if (i != 0) {
                tlv_dict_add_pron(ns->dict, dw, 0, ps, i, -1.0f);

                /* auto add sil/sp */
                if( i==1 && 0==tlv_string_cmp2(ps[0], &sil) )
                {
                	/* nothint to do */
                }
                else
                {
                    ps[i] = &sil;
                    tlv_dict_add_pron(ns->dict, dw, 0, ps, i+1, -1.0f);
                    ps[i] = &sp;
                    tlv_dict_add_pron(ns->dict, dw, 0, ps, i+1, -1.0f);
                }

                i = 0;
            }

        }
        else if ( *p>=0 && *p<=46 )
        {
        	ps[i++] = tlv_heap_dup_string(heap, mono_phns[*p], strlen(mono_phns[*p]));
        }
        else
        {
        	tlv_log("err: %d\n", *p);
        }

        if (p == pend) {
            if (i != 0) {
                tlv_dict_add_pron(ns->dict, dw, 0, ps, i, -1.0f);

                /* auto add sil/sp */
                if( i==1 && 0==tlv_string_cmp2(ps[0], &sil) )
                {
                	/* nothint to do */
                }
                else
                {
                    ps[i] = &sil;
                    tlv_dict_add_pron(ns->dict, dw, 0, ps, i+1, -1.0f);
                    ps[i] = &sp;
                    tlv_dict_add_pron(ns->dict, dw, 0, ps, i+1, -1.0f);
                }

                i = 0;
            }
        }
    }

    return dw;
}

struct tlv_dict_word *
tlv_nosqlite_dict_get_word(struct tlv_nosqlite_dict *ns, const char *word, int wlen)
{
    int rv;
    struct tlv_dict_word *dw = NULL;
    tlv_string_t str;

    char pron[65535];
    int plen = (int)sizeof(pron);

    tlv_string_set(&str, (char *)word, wlen);
    dw = tlv_dict_get_word(ns->dict, &str, 0);
    if (dw) {
        goto end;
    }

    rv = nosqlite_get(ns->db, word, wlen, pron, &plen);
    if (rv) {
        goto end;
    }

    if(0 == strncmp("database 0.3", nosqlite_version(ns->db), 12))
    {
    	dw = _set_and_get_dict_word3(ns, word, wlen, pron, (int)plen);
    }
    else
    {
    	dw = _set_and_get_dict_word(ns, word, wlen, pron, (int)plen);
    }
end:
    return dw;
}

struct tlv_dict_word* tlv_nosqlite_dict_get_word2(struct tlv_nosqlite_dict *ns, const char *word, int wlen)
{
    int rv;
    struct tlv_dict_word *dw = NULL;
    tlv_string_t str;

    char pron[65535];
    int plen = (int)sizeof(pron);

    tlv_string_set(&str, (char *)word, wlen);
    dw = tlv_dict_get_word(ns->dict, &str, 0);
    if (dw) {
        goto end;
    }

    rv = nosqlite_get(ns->db, word, wlen, pron, &plen);
    if (rv) {
        goto end;
    }
    dw = _set_and_get_dict_word(ns, word, wlen, pron, (int)plen);
//    tlv_dict_word_print(dw);

end:
    return dw;
}

struct tlv_dict *
tlv_nosqlite_dict_get_dict(struct tlv_nosqlite_dict *ns)
{
    return ns->dict;
}
