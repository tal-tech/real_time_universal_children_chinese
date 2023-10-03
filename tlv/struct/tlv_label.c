#include "tlv_label.h"

tlv_label_t* tlv_label_new(int n)
{
	tlv_label_t *l;

	l = (tlv_label_t*)malloc(sizeof(*l));
	tlv_label_init(l, n);

	return l;
}

int tlv_label_init(tlv_label_t* l,int n)
{
	l->hash = tlv_str_hash_new(n);
	l->heap = tlv_heap_new(4096);

	return 0;
}

tlv_name_cell_t* tlv_label_find(tlv_label_t *l, char *s, int sl, int insert)
{
	tlv_str_hash_t *h = l->hash;
	tlv_heap_t *heap  = l->heap;
	tlv_name_cell_t *n;

	n = (tlv_name_cell_t*)tlv_str_hash_find(h,s,sl);
	if(n || insert==0){goto end;}
	n = (tlv_name_cell_t*)tlv_heap_malloc(heap,sizeof(*n));
	n->data = 0;
	n->name = tlv_heap_dup_string(heap,s,sl);
	tlv_str_hash_add_node(h,n->name->data,n->name->len,n,&(n->hash_n));
end:

	return n;
}

tlv_string_t* tlv_label_find2(tlv_label_t *l, char *s, int sl, int insert)
{
	tlv_string_t *v = NULL;
	tlv_name_cell_t *nm;

	nm = tlv_label_find(l, s, sl, insert);
	if(!nm) { goto end; }
	v = nm->name;

end:

	return v;
}

int tlv_label_delete(tlv_label_t *l)
{
	tlv_label_clean(l);
	free(l);

	return 0;
}

int tlv_label_clean(tlv_label_t *l)
{
	tlv_str_hash_delete(l->hash);
	tlv_heap_delete(l->heap);

	return 0;
}

int tlv_label_reset(tlv_label_t *l)
{
	tlv_str_hash_reset(l->hash);
	tlv_heap_reset(l->heap);

	return 0;
}
