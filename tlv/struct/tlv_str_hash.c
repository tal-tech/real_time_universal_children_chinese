#include "tlv/struct/tlv_str_hash.h"

tlv_str_hash_t* tlv_str_hash_new(int n)
{
	tlv_str_hash_t *h;

	h        = (tlv_str_hash_t*)tlv_malloc(sizeof(*h));
	h->heap  = tlv_heap_new(1024);
	h->nitem = n;
	h->item  = (tlv_queue_t**)tlv_calloc(n, sizeof(tlv_queue_t*));
	return h;
}

int tlv_str_hash_delete(tlv_str_hash_t *h)
{
	tlv_heap_delete(h->heap);
	tlv_free(h->item);
	tlv_free(h);

	return 0;
}

int tlv_str_hash_reset(tlv_str_hash_t *h)
{
	tlv_heap_reset(h->heap);
	memset(h->item, 0, sizeof(tlv_queue_t*) * h->nitem);
	return 0;
}

int tlv_str_hash_add_node(tlv_str_hash_t *h,char* key,int key_bytes,void *value,tlv_str_hash_node_t* n)
{
	uint32_t index;
	int ret;

	tlv_string_set((&(n->key)),key,key_bytes);
	n->value=value;
	index=hash_string_value_len(key,key_bytes,h->nitem);
	if(!h->item[index])
	{
		h->item[index]=(tlv_queue_t*)tlv_heap_malloc(h->heap,sizeof(tlv_queue_t));
		tlv_queue_init(h->item[index]);
	}
	ret=tlv_queue_push(h->item[index],&(n->n));

	return ret;
}

int tlv_str_hash_add(tlv_str_hash_t *h,char* key,int key_bytes,void *value)
{
	tlv_str_hash_node_t *n;

	n = (tlv_str_hash_node_t*)tlv_heap_zalloc(h->heap,sizeof(tlv_str_hash_node_t));

	return tlv_str_hash_add_node(h,key,key_bytes,value,n);
}

tlv_str_hash_node_t* tlv_str_hash_find_node(tlv_str_hash_t *h, char* key,int key_bytes,uint32_t *rv)
{
	tlv_queue_node_t *qn;
	tlv_str_hash_node_t  *n, *r=NULL;
	uint32_t         index;
	tlv_queue_t      *q;

	index = hash_string_value_len(key, key_bytes, h->nitem);
	if(rv){ *rv=index; }
	q = h->item[index];
	if(!q)
	{
		goto end;
	}

	for(qn=q->rear;qn;qn=qn->next)
	{
		n = (tlv_str_hash_node_t*)data_offset2(qn,tlv_str_hash_node_t,n);
		if((n->key.len==key_bytes) && (memcmp(key,n->key.data,key_bytes)==0))
		{
			r=n;
			break;
		}
	}

end:
	return r;
}

void* tlv_str_hash_find(tlv_str_hash_t *h, char* key,int key_bytes)
{
	tlv_str_hash_node_t *n;

	n = tlv_str_hash_find_node(h, key, key_bytes, 0);

	return n ? (n->value) : 0;
}

int tlv_str_hash_cmp_findc(void** x,tlv_str_hash_node_t *n)
{
	tlv_cmp_handler_f cmp=(tlv_cmp_handler_f)x[0];
	void *u=x[1];

	return cmp(u,n->value);
}

int tlv_str_hash_findc(tlv_str_hash_t*h,char* k,int kb,tlv_cmp_handler_f cmp,void *user_data,void** v)
{
	tlv_queue_t* q;
	tlv_str_hash_node_t *hn;
	int ret,index;
	void* x[2];

	ret=-1;
	index=tlv_str_hash_index(h,k,kb);
	q=h->item[index];
	if(!q){goto end;}
	x[0]=cmp;
	x[1]=user_data;
	hn=(tlv_str_hash_node_t*)tlv_queue_find(q,offsetof(tlv_str_hash_node_t,n),(tlv_cmp_handler_f)tlv_str_hash_cmp_findc,x);
	if(!hn){goto end;}
	*v=hn->value;
	ret=0;

end:
	return ret;
}

int tlv_str_hash_walk(tlv_str_hash_t* h,tlv_walk_handler_f handler,void* user_data)
{
	int i,ret;
	ret=0;

	for(i=0;i<h->nitem;++i)
	{
		if(h->item[i])
		{
			ret=tlv_queue_walk(h->item[i],offsetof(tlv_str_hash_node_t,n),handler,user_data);
			if(ret!=0){goto end;}
		}
	}
end:
	return ret;
}

int tlv_str_hash_elems(tlv_str_hash_t *h)
{
	int i,c;

	c=0;
	for(i=0;i<h->nitem;++i)
	{
		if(h->item[i])
		{
			c+=h->item[i]->length;
		}
	}
	return c;
}

int tlv_str_hash_bytes(tlv_str_hash_t *h)
{
	return sizeof(*h)+h->nitem*sizeof(tlv_queue_t*)+tlv_heap_bytes(h->heap);
}

//-------------------------- test/example section ------------------
void tlv_str_hash_test_g()
{
	tlv_str_hash_t *hash;
	tlv_queue_node_t *qn;
	tlv_str_hash_node_t *hash_n;
	int i;

	hash=tlv_str_hash_new(13);
	tlv_str_hash_add_s(hash,"first","a");
	tlv_str_hash_add_s(hash,"second","b");
	tlv_str_hash_add_s(hash,"third","c");
	for(i=0;i<hash->nitem;++i)
	{
		if(!hash->item[i]){continue;}
		for(qn=hash->item[i]->rear;qn;qn=qn->next)
		{
			hash_n=data_offset(qn,tlv_str_hash_node_t,n);
			tlv_log("index=%d: [%.*s]=[%s]\n",i,hash_n->key.len,hash_n->key.data,(char*)hash_n->value);
		}
	}
}
