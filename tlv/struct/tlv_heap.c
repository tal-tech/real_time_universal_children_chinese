#include "tlv_heap.h"
#include "tlv_array.h"
#include "tlv_str_encode.h"
void* tlv_heap_malloc_large(tlv_heap_t* heap,size_t size);
int tlv_heap_reset_large(tlv_heap_t* heap);
#define tlv_heap_FAILED_COUNT 1

tlv_heap_block_t* tlv_heap_block_new(int size)
{
	tlv_heap_block_t *b;
	char *p;

    size=tlv_round(size, 8);
    p=tlv_malloc(tlv_round(sizeof(*b)+size,8));
    b=tlv_align_p(p+size,8);
    /*
    if((char*)b+sizeof(tlv_heap_block_t)>p+tlv_round(sizeof(*b)+size,8))
    {
    	tlv_log("found bug\n");
    	exit(0);
    }*/
	b->next=0;
	b->last=b->first=(uint8_t*)p;
	b->end=(uint8_t*)b->first+size;
	b->failed=0;
	//tlv_log("size=%d,first=%p,%d\n",size,b->first,(int)(b->first)%16);
	return b;
}

int tlv_heap_block_delete(tlv_heap_block_t *b)
{
	tlv_free(b->first);
	return 0;
}

void tlv_heap_block_reset(tlv_heap_block_t *b)
{
	b->last=b->first;
	b->failed=0;
}

tlv_heap_t* tlv_heap_new(size_t size)
{
	return tlv_heap_new2(size, TLV_PLAT_WORD);
}

tlv_heap_t* tlv_heap_new2(size_t size,int align_size)
{
	tlv_heap_t* p;

	p=(tlv_heap_t*)tlv_calloc(1,sizeof(tlv_heap_t));
	p->max=(size<TLV_MAX_ALLOC_FROM_HEAP)?size:TLV_MAX_ALLOC_FROM_HEAP;
	p->size=size;
	p->large=0;
	p->first=p->current=tlv_heap_block_new(size);
	p->align=align_size;
	return p;
}

int tlv_heap_delete(tlv_heap_t* heap)
{
	tlv_heap_block_t *p,*n;

	tlv_heap_reset_large(heap);
	for(p=heap->first;p;p=n)
	{
		n=p->next;
		tlv_heap_block_delete(p);
		if(!n){break;}
	}
	tlv_free(heap);
	return 0;
}

void tlv_heap_add_large(tlv_heap_t *heap,char *p,int size)
{
	tlv_heap_large_t* l;

	l=(tlv_heap_large_t*)tlv_heap_malloc(heap,sizeof(tlv_heap_large_t));
	l->data=p;
	l->size=size;
	l->next=heap->large;
	heap->large=l;
}

void* tlv_heap_malloc_large(tlv_heap_t* heap,size_t size)
{
	tlv_heap_large_t* l;
	void* p;

	//tlv_log("malloc larg=%d\n",size);
	p=tlv_malloc(size);
	if(!p){return 0;}
	l=(tlv_heap_large_t*)tlv_heap_malloc(heap,sizeof(tlv_heap_large_t));
	l->data=p;
	l->size=size;
	l->next=heap->large;
	heap->large=l;
	return p;
}

int tlv_heap_reset_large(tlv_heap_t* heap)
{
	tlv_heap_large_t* l;

	for(l=heap->large;l;l=l->next)
	{
		tlv_free(l->data);
	}
	return 0;
}

int tlv_heap_reset2(tlv_heap_t* heap)
{
	tlv_heap_block_t *p,*n;

	tlv_heap_reset_large(heap);
	heap->large=0;
	tlv_heap_block_reset(heap->first);
	p=heap->first->next;
	for(n=p;n;n=p)
	{
		p=n->next;
		tlv_heap_block_delete(n);
	}
	heap->first->next=0;
	heap->current=heap->first;
	return 0;
}


int tlv_heap_reset(tlv_heap_t* heap)
{
	tlv_heap_block_t *p,*n;

	tlv_heap_reset_large(heap);
	heap->large=0;
	p=heap->first;
	for(n=p;n;n=p)
	{
		p=n->next;
		tlv_heap_block_delete(n);
	}
	heap->first=heap->current=tlv_heap_block_new(heap->size);
	return 0;
}


void* tlv_heap_malloc_block(tlv_heap_t* heap,size_t size)
{
	uint8_t *m;
	tlv_heap_block_t *newb;

	newb=tlv_heap_block_new(heap->size);
	m=tlv_align_p(newb->last,heap->align);
	if(m+size>newb->end)
	{
		m=tlv_heap_malloc_large(heap,size);
	}else
	{
		newb->last=m+size;
	}
	heap->current->next=newb;
	heap->current=newb;
	return m;
}


void* tlv_heap_malloc_block2(tlv_heap_t* heap,size_t size)
{
	uint8_t *m;
	tlv_heap_block_t *newb,*cur,*p;

	newb=tlv_heap_block_new(heap->size);
	/*
	m=newb->last;
	newb->last = m+size;
	*/
	m=tlv_align_p(newb->last,heap->align);
	if(m+size>newb->end)
	{
		m=tlv_heap_malloc_large(heap,size);
	}else
	{
		newb->last=m+size;
	}
	if(0)
	{
		cur=heap->current;
		for(p=cur;p->next;p=p->next)
		{
			if(++(p->failed)>tlv_heap_FAILED_COUNT)
			{
				cur=p->next;
			}
		}
		p->next=newb;
		heap->current=cur?cur:newb;
	}else
	{
		heap->current->next=newb;
		heap->current=newb;
	}
	return m;
}


//#define USE_HEAP

#ifdef USE_HEAP

void* tlv_heap_malloc(tlv_heap_t* heap,size_t size)
{
	return malloc(size);
}
#else

void* tlv_heap_malloc(tlv_heap_t* heap,size_t size)
{
	tlv_heap_block_t *b;
	uint8_t* m;
	int align;

	if(size>heap->max)
	{
		return tlv_heap_malloc_block(heap,size);
	}
	align=heap->align;
	b=heap->current;
	if(align>1)
	{
		m=tlv_align_p(b->last,align);
	}else
	{
		m=b->last;
	}
	if((int)((b->end-m))>=size)
	{
		b->last=m+size;
		return m;
	}
	return tlv_heap_malloc_block(heap,size);
}

void* tlv_heap_malloc2(tlv_heap_t* heap,size_t size)
{
	tlv_heap_block_t *b;
	uint8_t* m;
	int align;

	if(size<=0){return 0;}
	if(size>heap->max){return tlv_heap_malloc_large(heap,size);}
	align=heap->align;
	b=heap->current;
	do
	{
		if(align>1)
		{
			m=tlv_align_p(b->last,align);
		}else
		{
			m=b->last;
		}
		//tlv_log("b->last=%p,m=%p,end=%p,size=%d\n",b->last,m,b->end,v);
		if((int)((b->end-m))>=size)
		{
			b->last=m+size;
			return m;
		}
		b=b->next;
	}while(b);
	return tlv_heap_malloc_block(heap,size);
}
#endif

tlv_string_t* tlv_heap_dup_string(tlv_heap_t *h,char *s,int sl)
{
	tlv_string_t *str;

	str=(tlv_string_t*)tlv_heap_malloc(h,sizeof(*str)+sl);
	str->len=sl;
	str->data=(char*)str+sizeof(*str);
	if(s)
	{
		memcpy(str->data,s,sl);
	}
	return str;
}

char* tlv_heap_dup_data(tlv_heap_t *h,const char *s,int l)
{
	char *data;

	data=(char*)tlv_heap_malloc(h,l);
	if(!data){goto end;}
	memcpy(data,s,l);
end:
	return data;
}

tlv_string_t* tlv_heap_dup_string2(tlv_heap_t *h,char *s,int sl)
{
	tlv_string_t *str;

	str=(tlv_string_t*)tlv_heap_malloc(h,sizeof(*str)+sl+1);
	str->len=sl;
	str->data=(char*)str+sizeof(*str);
	if(s)
	{
		memcpy(str->data,s,sl);
	}
	str->data[sl]=0;
	return str;
}

void* tlv_heap_zalloc(tlv_heap_t* heap,size_t size)
{
	void *p;

	p=tlv_heap_malloc(heap,size);
	if(p)
	{
		memset(p,0,size);
	}
	return p;
}

char *tlv_heap_dup_str(tlv_heap_t *heap,char* s)
{
    return tlv_heap_dup_str2(heap,s,s?strlen(s):0);
}

char* tlv_heap_dup_str2(tlv_heap_t *heap,char *data,int len)
{
	char *d=0;

	if(len<=0){goto end;}
	d=(char*)tlv_heap_malloc(heap,len+1);
	memcpy(d,data,len);
	d[len]=0;
end:
	return d;
}

void tlv_heap_fill_string(tlv_heap_t *heap,tlv_string_t *str,char *data,int bytes)
{
	str->data=tlv_heap_dup_data(heap,data,bytes);
	str->len=bytes;
}

int tlv_heap_bytes(tlv_heap_t *heap)
{
	tlv_heap_large_t* l;
	tlv_heap_block_t *p;
	int size;

	size=0;
	for(l=heap->large;l;l=l->next)
	{
		size+=l->size;
	}
	for(p=heap->first;p;p=p->next)
	{
		size+=sizeof(tlv_heap_block_t)+p->end-p->first;
	}
	return size;
}


void tlv_heap_print(tlv_heap_t *heap)
{
	tlv_heap_large_t* l;
	tlv_heap_block_t *p;
	int count,size;

	printf("########## Heap #############\n");
	for(l=heap->large,size=0,count=0;l;l=l->next)
	{
		++count;
		size+=l->size;
	}
	printf("large list:\t%d\n",count);
	printf("large bytes:\t%d\n",size);
	for(count=0,size=0,p=heap->first;p;p=p->next)
	{
		size+=p->last-p->first;
		++count;
	}
	printf("block list:\t%d\n",count);
	printf("block bytes:\t%d\n",size);
}
