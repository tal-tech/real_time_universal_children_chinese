#include "tlv_dict.h"

static tlv_string_t pre_dict_words[]=
{
	tlv_string("!SUBLATID"),
};

static tlv_string_t pre_dict_sntflag_words[][2]=
{
	{tlv_string("!SENT_START"),tlv_string("sil")},
	{tlv_string("!SENT_END"),tlv_string("sil")},
};


static tlv_string_t null_word=tlv_string("!NULL");

typedef struct
{
	tlv_string_t name;
	tlv_phn_type_t type;
}tlv_dict_pre_phn_t;

static tlv_dict_pre_phn_t pre_dict_phns[]=
{
	{tlv_string("sil"), TLV_PHN_CI},
	{tlv_string("sp"), TLV_PHN_CF}
};

tlv_dict_t* tlv_dict_new(tlv_label_t *l,int use_db)
{
	tlv_dict_t *d;
	int hash_int = 257;

	d=(tlv_dict_t*)malloc(sizeof(*d));
	tlv_dict_init(d, l, use_db, hash_int, hash_int);
	return d;
}

int tlv_dict_delete(tlv_dict_t *d)
{
	tlv_dict_clean(d);
	free(d);
	return 0;
}

int tlv_dict_add_words_sent_flag(tlv_dict_t *d)
{
	tlv_string_t *s;
	tlv_dict_word_t *w;
	int i,count;

	count=2;
	for(i=0;i<count;++i)
	{
		w=tlv_dict_get_word(d,&(pre_dict_sntflag_words[i][0]),1);
		s=&(pre_dict_sntflag_words[i][1]);
		tlv_dict_add_pron(d,w,0,&(s),1,-1);
	}

	return 0;
}

int tlv_dict_setup(tlv_dict_t *d)
{
	int i,count;
	tlv_dict_phone_t *phn;
	tlv_dict_pre_phn_t *pvar;
	tlv_dict_word_t *w;

	d->null_word = w = tlv_dict_get_word(d,&null_word,1);
	d->null_pron = tlv_dict_add_pron(d,w,0,0,0,1.0);
	count = sizeof(pre_dict_words)/sizeof(tlv_string_t);
	for(i=0;i<count;++i)
	{
		tlv_dict_get_word(d,&(pre_dict_words[i]),1);
	}
	count = sizeof(pre_dict_phns)/sizeof(tlv_dict_pre_phn_t);
	for(i=0;i<count;++i)
	{
		pvar = &(pre_dict_phns[i]);
		phn  = tlv_dict_get_phone(d,&(pvar->name),1);
		phn->type=pvar->type;
	}
	if(d->use_db)
	{
		tlv_dict_add_words_sent_flag(d);
	}

	return 0;
}

int tlv_dict_init(tlv_dict_t *d, tlv_label_t *label, int use_db,
		          int phn_hash_hint, int wrd_hash_hint)
{
	memset(d,0,sizeof(*d));
	d->label=label;
	d->heap=tlv_heap_new(4096);
	d->phone_hash=tlv_str_hash_new(phn_hash_hint);
	d->word_hash=tlv_str_hash_new(wrd_hash_hint);
	d->use_db=use_db;
	tlv_dict_setup(d);
	return 0;
}

int tlv_dict_clean(tlv_dict_t *d)
{
	tlv_str_hash_delete(d->phone_hash);
	tlv_str_hash_delete(d->word_hash);
	tlv_heap_delete(d->heap);
	return 0;
}

int tlv_dict_reset(tlv_dict_t *d)
{
    d->nword = 0;
    d->npron = 0;
    d->nphone = 0;
	tlv_heap_reset(d->heap);
	tlv_str_hash_reset(d->phone_hash);
	tlv_str_hash_reset(d->word_hash);
	tlv_dict_setup(d);

	return 0;
}

/*
int tlv_dict_is_closed(tlv_dict_t *d, tlv_gmminfo_t *hl)
{
#ifdef USE_CHECK
	tlv_str_hash_t *hash=d->phone_hash;
	tlv_queue_t *q;
	tlv_queue_node_t *n;
	tlv_str_hash_node_t *hn;
	tlv_dict_phone_t *phn;
	void *hmm;
	int i;
	int closed;

	for(i=0;i<hash->nitem;++i)
	{
		//tlv_log("%d:%d\n",i,hash->nitem);
		q=hash->item[i];
		if(!q){continue;}
		for(n=q->pop;n;n=n->next)
		{
			hn=tlv_queue_node_data_offset(n,tlv_str_hash_node_t,n);
			phn=(tlv_dict_phone_t*)hn->value;
			print_data(phn->name->data,phn->name->len);
			hmm=find(hl,phn->name->data,phn->name->len);
			if(!hmm){closed=0;goto end;}
		}
	}
	closed=1;
end:
	//tlv_log("%d\n",closed);
	return closed;
#else
	return 1;
#endif
}
*/

tlv_dict_phone_t* tlv_dict_get_phone(tlv_dict_t* d, tlv_string_t *phn, int insert)
{
	tlv_dict_phone_t *p;

	p = (tlv_dict_phone_t*)tlv_str_hash_find(d->phone_hash, phn->data, phn->len);
	if(p){goto end;}
	if(0 == insert){ goto end; }
	p = (tlv_dict_phone_t*)tlv_heap_zalloc(d->heap, sizeof(*p));
	p->name = phn;
	p->type = TLV_PHN_CD;
	tlv_str_hash_add(d->phone_hash, p->name->data, p->name->len, p);
	++d->nphone;

end:

	return p;
}

tlv_dict_word_t* tlv_dict_find_word(tlv_dict_t *d,char* n,int nl)
{
	return (tlv_dict_word_t*)tlv_str_hash_find(d->word_hash,n,nl);
}

tlv_dict_word_t* tlv_dict_get_dummy_wrd(tlv_dict_t *d, char *w, int bytes)
{
	static tlv_string_t ps[]={
			tlv_string("s"),
			tlv_string("ax"),
			tlv_string("t"),
			tlv_string("n"),
			tlv_string("ih"),
	};
	tlv_string_t name;
	tlv_string_t *v;
	tlv_dict_word_t *dw;
	int i;

	//print_data(w,bytes);
	tlv_string_set(&name,w,bytes);
	dw=tlv_dict_get_word2(d,&name,1);
	for(i=0;i<5;++i)
	{
		v=&(ps[i]);
		tlv_dict_add_pron(d,dw,0,&v,1,-1);
	}

	return dw;
}

tlv_dict_word_t* tlv_dict_get_word(tlv_dict_t *d, tlv_string_t *name, int insert)
{
	tlv_dict_word_t *w;

	w = (tlv_dict_word_t*)tlv_str_hash_find(d->word_hash, name->data, name->len);
	if(w) { goto end; }
	if(0 == insert) { goto end; }
	w = (tlv_dict_word_t*)tlv_heap_zalloc(d->heap, sizeof(*w));
	w->name    = name;
	w->in_dict = 1;
	tlv_str_hash_add(d->word_hash, w->name->data, w->name->len, w);
	++d->nword;

end:

	return w;
}


tlv_dict_word_t* tlv_dict_get_word2(tlv_dict_t *d, tlv_string_t *name, int insert)
{
	tlv_dict_word_t *w;

	w = (tlv_dict_word_t*)tlv_str_hash_find(d->word_hash, name->data, name->len);
	if(w) { goto end; }
	if(0 == insert) { goto end; }
	w = (tlv_dict_word_t*)tlv_heap_zalloc(d->heap, sizeof(*w));
	w->name = tlv_heap_dup_string(d->heap, name->data, name->len);
	tlv_str_hash_add(d->word_hash, w->name->data, w->name->len, w);
	++d->nword;

end:

	return w;
}

tlv_dict_pron_t* tlv_dict_add_pron(tlv_dict_t *d, tlv_dict_word_t *word, tlv_string_t *outsym, tlv_string_t **phones, int nphones, float prob)
{
	tlv_heap_t *h=d->heap;
	tlv_dict_pron_t *pron,**tmp;
	int i;

	//tlv_log("%d\n",d->n)
	pron=(tlv_dict_pron_t*)tlv_heap_zalloc(h,sizeof(*pron));
	pron->word    = word;
	pron->nPhones = nphones;
	if(pron->nPhones>0)
	{
		pron->pPhones=(tlv_dict_phone_t**)tlv_heap_malloc(h,pron->nPhones*sizeof(tlv_dict_phone_t*));
	}
	for(i=0;i<pron->nPhones;++i)
	{
		//print_data(s[i+2]->data,s[i+2]->len);
		pron->pPhones[i]=tlv_dict_get_phone(d,phones[i],1);
	}
	pron->outsym = outsym? outsym : word->name;
	if(prob>=MINPRONPROB && prob<=1.0)
	{
		pron->prob = log(prob);
	}else if(prob>=0.0 && prob<MINPRONPROB)
	{
		pron->prob = LZERO;
	}else
	{
		pron->prob = 0.0f;
	}
	tmp = &(word->pron_list);
	i=0;

	while(*tmp)
	{
		//tlv_log("tmp: %p,%p\n",tmp,w->pron_list);
		++i;
		tmp=&((*tmp)->next);
	}
	pron->pnum=i+1;
	*tmp=pron;
	++word->npron;
	++d->npron;

	return pron;
}

void tlv_dict_pron_print(tlv_dict_pron_t *pron)
{
	int i;
	if (!pron) return;
	printf("pron: %.*s\n",pron->outsym->len,pron->outsym->data);
	for(i=0;i<pron->nPhones;++i)
	{
		printf("\tp[%d]=%.*s\n",i,pron->pPhones[i]->name->len,pron->pPhones[i]->name->data);
	}
}

void tlv_dict_word_print(tlv_dict_word_t *dw)
{
	tlv_dict_pron_t *pron;

	pron=dw->pron_list;
	tlv_log("========= wrd=(%.*s:%d) =========\n",dw->name->len,
			dw->name->data,dw->npron);
	while(pron)
	{
		tlv_dict_pron_print(pron);
		pron=pron->next;
	}
}

int tlv_dict_add_word(tlv_dict_t *d, tlv_array_t *a, float prob)
{
	tlv_dict_word_t *w;
	tlv_string_t** s;

	s=(tlv_string_t**)a->item;
	w=tlv_dict_get_word(d,s[0],1);
	//tlv_log("[%.*s]=%d/%p\n",s[0]->len,s[0]->data,a->nitem,w);
	tlv_dict_add_pron(d,w,s[1],s+2,a->nitem-2,prob);

	return 0;
}

int tlv_dict_read_word(tlv_dict_t*d, tlv_strfile_t *s, tlv_charbuf_t *b, tlv_array_t *pa, float *prob)
{
	tlv_string_t* nw;
	tlv_label_t *l=d->label;
	tlv_string_t **st;
	int ret,nl,nphones;
	float p=-1,v;
	char *ptr;

	ret=tlv_strfile_read_string(s,b);
	if(ret!=0){goto end;}
	//tlv_log("word: %.*s\n",b->pos,b->data);
	nw=tlv_label_find(l,b->data,b->pos,1)->name;

	st=(tlv_string_t**)tlv_array_push_n(pa,2);
	st[0]=nw;st[1]=0;
	nphones=0;
	while(1)
	{
		ret=tlv_strfile_skip_sp(s,&nl);
		if(ret!=0){goto end;}
		if(nl){break;}
		ret=tlv_strfile_read_string(s,b);
		if(ret!=0){goto end;}

		if(b->data[0]=='[' && b->data[b->pos-1]==']')
		{
			if(st[1]){ret=-1;goto end;}
			st[1]=tlv_label_find(l,&(b->data[1]),b->pos-2,1)->name;
		}else
		{
			if(nphones==0 && p<0)
			{
				char c=0;
				tlv_charbuf_push(b,&c,1);
				b->pos-=1;
				v=(float)strtod(b->data,&ptr);
			}else
			{
				v=0.0;ptr=b->data;
			}
			if(ptr!=b->data)
			{
				if(v<=0.0 || v>1.0 ||*ptr!=0){ret=-1;goto end;}
				p=v;
			}else
			{
				st=(tlv_string_t**)tlv_array_push(pa);
				//print_data(b->data,b->pos);
				st[0]=tlv_label_find(l,b->data,b->pos,1)->name;
				//print_data(st[0]->data,st[0]->len);
			}
		}
	}
	*prob=p;

end:

	return ret;
}


int tlv_dict_load(tlv_dict_t *d, tlv_strfile_t *s)
{
	tlv_charbuf_t *b;
	tlv_heap_t *h;
	tlv_array_t *a;
	int ret,eof;
	float prob;

	ret=-1;
	h=tlv_heap_new(4096);
	a=tlv_array_new(h,256,sizeof(tlv_string_t*));
	b=tlv_charbuf_new(64,1);eof=0;
	while(1)
	{
		tlv_array_reset(a);
		ret=tlv_dict_read_word(d,s,b,a,&prob);
		if(ret!=0)
		{
			eof=tlv_strfile_get(s)==EOF;
			if(!eof){break;}
			ret=0;
			if(a->nitem==0){break;}
		}
		ret=tlv_dict_add_word(d,a,prob);
		if(ret!=0){goto end;}
		if(eof){break;}
	}
end:
	//tlv_log("%d\n",ret);
	tlv_charbuf_delete(b);
	tlv_heap_delete(h);
	return ret;
}

