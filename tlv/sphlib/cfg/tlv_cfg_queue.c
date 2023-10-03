#include "tlv_cfg_queue.h"
#include "tlv_part_cfg.h"
static tlv_string_t* tlv_cfg_queue_dup_string(tlv_cfg_queue_t *cfg, char *v, int vbytes);

tlv_cfg_queue_t* tlv_cfg_queue_new_h(tlv_heap_t *h)
{
	tlv_cfg_queue_t *cfg;

	cfg = (tlv_cfg_queue_t*)tlv_heap_malloc(h, sizeof(*cfg));
	tlv_queue_init(&(cfg->queue));
	cfg->heap = h;
	return cfg;
}

tlv_cfg_item_t* tlv_cfg_queue_find(tlv_cfg_queue_t *cfg, char *k, int klen)
{
	tlv_cfg_item_t *item = 0, *p;
	tlv_queue_node_t *n;

	for(n=cfg->queue.rear; n; n=n->next)
	{
		p = data_offset(n, tlv_cfg_item_t, n);
		if(0 == tlv_string_cmp(p->key, k, klen))
		{
			item = p;
			break;
		}
	}

	return item;
}

void tlv_cfg_queue_remove(tlv_cfg_queue_t *cfg, tlv_cfg_item_t *item)
{
	tlv_queue_remove(&(cfg->queue), &(item->n));
}

int tlv_cfg_queue_add(tlv_cfg_queue_t *cfg, tlv_cfg_item_t *item)
{
	return tlv_queue_push(&(cfg->queue), &(item->n));
}

int tlv_cfg_queue_add_string(tlv_cfg_queue_t *cfg, char *k, int klen, char *v, int vlen)
{
	tlv_heap_t *h = cfg->heap;
	tlv_cfg_item_t *item;
	tlv_string_t *sv;

	sv = tlv_cfg_queue_dup_string(cfg, v, vlen);
	item = (tlv_cfg_item_t*)tlv_cfg_queue_find(cfg, k, klen);
	if(tlv_str_equal_s(v, vlen, "nil"))
	{
		if(item)
		{
			tlv_cfg_queue_remove(cfg, item);
		}
	}else
	{
		if(!item)
		{
			item = (tlv_cfg_item_t*)tlv_heap_malloc(h, sizeof(*item));
			item->key = tlv_cfg_queue_dup_string(cfg, k, klen);
			tlv_cfg_queue_add(cfg, item);
		}
		item->type = TLV_CFG_STRING;
		item->value.str = sv;
	}

	return 0;
}

int tlv_cfg_queue_add_lc(tlv_cfg_queue_t *cfg,char *k, int klen, tlv_part_cfg_t *lc)
{
	tlv_heap_t *h = cfg->heap;
	tlv_cfg_item_t *item;

	item = (tlv_cfg_item_t*)tlv_cfg_queue_find(cfg, k, klen);
	if(!item)
	{
		item = (tlv_cfg_item_t*)tlv_heap_malloc(h, sizeof(*item));
		item->key = tlv_cfg_queue_dup_string(cfg, k, klen);
		tlv_string_set(&(lc->name), item->key->data, item->key->len);
		tlv_cfg_queue_add(cfg, item);
	}
	item->type = TLV_CFG_LC;
	item->value.cfg = lc;

	return 0;
}

int tlv_cfg_queue_add_array(tlv_cfg_queue_t *cfg, char *k, int klen, tlv_array_t *a)
{
	tlv_heap_t *h = cfg->heap;
	tlv_cfg_item_t *item;

	item = (tlv_cfg_item_t*)tlv_cfg_queue_find(cfg, k, klen);
	if(!item)
	{
		item = (tlv_cfg_item_t*)tlv_heap_malloc(h, sizeof(*item));
		item->key = tlv_cfg_queue_dup_string(cfg, k, klen);
		tlv_cfg_queue_add(cfg, item);
	}

	item->type = TLV_CFG_ARRAY;
	item->value.array =  a;

	return 0;
}

/*--------------- static method --------------*/
static tlv_string_t* tlv_cfg_queue_dup_string(tlv_cfg_queue_t *cfg,char *v,int vbytes)
{
	tlv_string_t *sv;

	if(vbytes<=0)
	{
		v=0;
	}
	sv=(tlv_string_t*)tlv_heap_malloc(cfg->heap,sizeof(*sv));
	if(vbytes<=0)
	{
		sv->len=0;
		sv->data=0;
	}else
	{
		sv->len=vbytes;
		sv->data=(char*)tlv_heap_malloc(cfg->heap,vbytes+1);
		memcpy(sv->data,v,vbytes);
		sv->data[vbytes]=0;
	}
	return sv;
}


/*--------------------- end ------------------*/

void tlv_cfg_item_print(tlv_cfg_item_t *item)
{
	tlv_string_t *name;
	tlv_string_t **str;
	int j;

	printf("%*.*s=",item->key->len, item->key->len, item->key->data);
	switch(item->type)
	{
	case TLV_CFG_STRING:
		name = item->value.str;
		printf("%*.*s", name->len, name->len, name->data);
		break;
	case TLV_CFG_LC:
		printf("{\n");
		tlv_part_cfg_print(item->value.cfg);
		printf("}");
		break;
	case TLV_CFG_ARRAY:
		str = (tlv_string_t**)(item->value.array->item);
		printf("[");
		for(j=0; j<item->value.array->nitem; ++j)
		{
			name=str[j];
			if(j>0){printf(",");}
			printf("%*.*s", name->len, name->len, name->data);
		}
		printf("]");
		break;
	default:
		break;
	}
	printf(";\n");
}

void tlv_cfg_queue_print(tlv_cfg_queue_t *cfg)
{
	tlv_cfg_item_t *p;
	tlv_queue_node_t *n;

	for(n=cfg->queue.rear; n; n=n->next)
	{
		p = data_offset(n, tlv_cfg_item_t, n);
		tlv_cfg_item_print(p);
	}
}

void tlv_cfg_queue_to_string(tlv_cfg_queue_t *cfg, tlv_charbuf_t *buf)
{
	tlv_cfg_item_t *i;
	tlv_queue_node_t *n;
	tlv_string_t *v;
	tlv_string_t **ss;
	int j;

	for(n=cfg->queue.rear; n; n=n->next)
	{
		i = data_offset(n, tlv_cfg_item_t, n);
		tlv_charbuf_push(buf, i->key->data, i->key->len);
		tlv_charbuf_push_s(buf,"=");
		switch(i->type)
		{
		case TLV_CFG_STRING:
			v=i->value.str;
            tlv_charbuf_push_s(buf,"\"");
			tlv_charbuf_push(buf,v->data,v->len);
            tlv_charbuf_push_s(buf,"\"");
			break;
		case TLV_CFG_LC:
			tlv_charbuf_push_s(buf,"{");
			tlv_part_cfg_value_to_string(i->value.cfg,buf);
			tlv_charbuf_push_s(buf,"}");

			break;
		case TLV_CFG_ARRAY:
			ss=(tlv_string_t**)(i->value.array->item);
			tlv_charbuf_push_s(buf,"[");
			for(j=0;j<i->value.array->nitem;++j)
			{
				v=ss[j];
				if(j>0){tlv_charbuf_push_s(buf,",");}
				tlv_charbuf_push(buf,v->data,v->len);
			}
			tlv_charbuf_push_s(buf,"]");
			break;
		default:
			break;
		}
		tlv_charbuf_push_s(buf,";");
	}
}
