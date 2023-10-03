#include "tlv/sphlib/cfg/tlv_part_cfg.h"

tlv_part_cfg_t *tlv_part_cfg_new_h(tlv_heap_t *h)
{
	tlv_part_cfg_t *cfg;

	cfg = (tlv_part_cfg_t*)tlv_heap_zalloc(h, sizeof(*cfg));
	cfg->queue = tlv_cfg_queue_new_h(h);
	cfg->heap = h;

	return cfg;
}


tlv_cfg_item_t* tlv_part_cfg_find(tlv_part_cfg_t *cfg, char *name, int len)
{
	tlv_cfg_item_t *item;

	item = (tlv_cfg_item_t*)tlv_cfg_queue_find(cfg->queue, name, len);
	if(item){goto end;}
	if(cfg->parent)
	{
		item = tlv_part_cfg_find(cfg->parent, name, len);
	}else
	{
		item = NULL;
	}

end:

	return item;
}


tlv_part_cfg_t* tlv_part_cfg_find_lc(tlv_part_cfg_t *cfg, char *name, int len)
{
	tlv_cfg_item_t *i;
	tlv_part_cfg_t *lc=0;

	if(!cfg){goto end;}
	i = (tlv_cfg_item_t*)tlv_cfg_queue_find(cfg->queue, name, len);
	if(i && tlv_cfg_item_is_lc(i)){lc=i->value.cfg;goto end;}
	lc = tlv_part_cfg_find_lc(cfg->parent, name, len);

end:
	return lc;
}


tlv_string_t* tlv_part_cfg_find_string(tlv_part_cfg_t *cfg, char *name, int len)
{
	return tlv_part_cfg_find_string2(cfg, name, len, 1);
}

tlv_string_t* tlv_part_cfg_find_string2(tlv_part_cfg_t *cfg, char *name, int len, int recursive)
{
	tlv_string_t* value = 0;
	tlv_cfg_item_t *item;

	if(!cfg){goto end;}
	item = (tlv_cfg_item_t*)tlv_cfg_queue_find(cfg->queue, name, len);
	if(item && (item->type == TLV_CFG_STRING) ){value=item->value.str;goto end;}
	if(recursive)
	{
		value = tlv_part_cfg_find_string(cfg->parent, name, len);
	}
end:
	return value;
}

tlv_array_t* tlv_part_cfg_find_array(tlv_part_cfg_t *cfg, char *name, int len)
{
	tlv_array_t* a=0;
	tlv_cfg_item_t *i;

	if(!cfg){goto end;}
	i = (tlv_cfg_item_t*)tlv_cfg_queue_find(cfg->queue, name, len);
	if(i && tlv_cfg_item_is_array(i)){a=i->value.array;goto end;}
	a = tlv_part_cfg_find_array(cfg->parent, name, len);

end:

	return a;
}

tlv_part_cfg_t* tlv_part_cfg_find_section(tlv_part_cfg_t *lc, char *section, int section_bytes, tlv_string_t *last_field)
{
	char *ks,*ke,*ls;
	int len;

	ls = ks=section;
	ke = ks+section_bytes;
	while(ks < ke)
	{
		if(*ks==':')
		{
			len=ks-ls;
			lc=tlv_part_cfg_find_lc(lc,ls,len);

			ls=ks+1;

		}
		++ks;
	}
	if(!lc){goto end;}
	len=ke-ls;
	tlv_string_set(last_field,ls,len);

end:

	return lc;
}

tlv_part_cfg_t* tlv_part_cfg_find_section_lc(tlv_part_cfg_t* lc, char *section, int section_bytes)
{
	tlv_string_t last_field;

	lc = tlv_part_cfg_find_section(lc,section,section_bytes,&last_field);
	if(!lc){goto end;}
	lc = tlv_part_cfg_find_lc(lc,last_field.data,last_field.len);

end:

	return lc;
}




int tlv_part_cfg_update_hash(tlv_part_cfg_t* lc,tlv_arg_item_t *n,int show)
{
	tlv_string_t last_field;
	char *v;
	tlv_string_t *xv;

	if(n->v.len<=0){return 0;}
	v=(char*)n->v.data;
	lc=tlv_part_cfg_find_section(lc,n->k.data,n->k.len,&last_field);
	if(!lc){goto end;}
	xv=tlv_part_cfg_find_string(lc,last_field.data,last_field.len);
	if(xv)
	{
		tlv_string_set(xv,v,strlen(v));
		if(show)
		{
			printf("[cmd] update %.*s=%s\n",n->k.len,n->k.data,v);
		}
	}else
	{
		tlv_cfg_queue_add_string(lc->queue,last_field.data,last_field.len,v,strlen(v));
		if(show)
		{
			printf("[cmd] set %.*s=%s\n",n->k.len,n->k.data,v);
		}
	}
end:
	//tlv_log("%*.*s=%s\n",n->key.len,n->key.len,n->key.data,v);
	return 0;
}

int tlv_part_cfg_hook_update(void **hook,tlv_arg_item_t *item)
{
	return tlv_part_cfg_update_hash((tlv_part_cfg_t*)hook[0],item,*(int*)hook[1]);
}

void tlv_part_cfg_update_arg(tlv_part_cfg_t *lc,tlv_arg_t *arg,int show)
{
	void *hook[2]={lc,&show};

	if(show)
	{
		printf("================ update ===============\n");
	}
	//tlv_str_hash_walk(arg->hash,(tlv_walk_handler_f)tlv_part_cfg_hook_update,hook);
	tlv_queue_walk(&(arg->queue),offsetof(tlv_arg_item_t,q_n),(tlv_walk_handler_f)tlv_part_cfg_hook_update,hook);
	if(show)
	{
		printf("=======================================\n\n");
	}
}


void tlv_part_cfg_print(tlv_part_cfg_t *lc)
{
	//printf("################  %*.*s ########################\n",lc->name.len,lc->name.len,lc->name.data);
	tlv_cfg_queue_print(lc->queue);
	//printf("################################################\n");
}

void tlv_part_cfg_value_to_string(tlv_part_cfg_t *lc,tlv_charbuf_t *buf)
{
	tlv_cfg_queue_to_string(lc->queue, buf);
}
