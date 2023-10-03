#include "tlv_cellar.h"

int tlv_cellar_init(tlv_cellar_t *c, int offset, int max_free, tlv_new_handler_f newer,
		tlv_delete_handler_f deleter, void *data)
{
	c->free      = NULL;
	c->use       = NULL;

	c->user_data = data;
	c->newer     = newer;
	c->deleter   = deleter;
	c->deleter2  = 0;

	c->offset     = offset;
	c->max_free   = max_free;
	c->cur_free   = 0;
    c->use_length = 0;
	return 0;
}

int tlv_cellar_init2(tlv_cellar_t *c, int offset, int max_free, tlv_new_handler_f newer,
		tlv_delete_handler2_f deleter, void *data)
{
	c->free        = NULL;
	c->use         = NULL;
	c->offset      = offset;
	c->max_free    = max_free;
	c->cur_free    = 0;

	c->user_data   = data;
	c->newer       = newer;
	c->deleter     = NULL;
	c->deleter2    = deleter;

    c->use_length  = 0;

	return 0;
}

void tlv_cellar_reset(tlv_cellar_t *c)
{
	c->free      =0;
	c->use       =0;
	c->cur_free  =0;
	c->use_length=0;
}

int tlv_cellar_queue_clean(tlv_cellar_t *c, tlv_queue_node_t* q)
{
	tlv_queue_node_t *t;
	int offset;

	if(c->deleter)
	{
		offset = c->offset;
		for(;q;q=t)
		{
			t = q->prev;
			c->deleter((void*)((char*)q-offset));
		}
	}else if(c->deleter2)
	{
		offset = c->offset;
		for(;q;q=t)
		{
			t = q->prev;
			c->deleter2(c->user_data,(void*)((char*)q-offset));
		}
	}

	return 0;
}

int tlv_cellar_clean(tlv_cellar_t *c)
{
    if(c)
    {
    	tlv_cellar_queue_clean(c, c->use);
	    tlv_cellar_queue_clean(c, c->free);

	    c->free=c->use=0;
	    c->use_length=0;
    }
	return 0;
}

void* tlv_cellar_pop(tlv_cellar_t *c)
{
	void*            data;
	tlv_queue_node_t *q;

	if(c->free)
	{
		q = c->free;
		c->free = q->prev;
		if(c->free)
		{
			c->free->next=0;
		}
		data = (void*)((char*)q-c->offset);
		--c->cur_free;
	}else
	{
		if(!c->newer){return 0;}
		data = c->newer(c->user_data);
        if(!data){return 0;}
		q = (tlv_queue_node_t *)((char*)data+c->offset);
	}
	q->prev = c->use;
	q->next = 0;
	if(c->use)
	{
		c->use->next = q;
		c->use       = q;
	}else
	{
		c->use = q;
	}
    ++c->use_length;

	return data;
}

int tlv_cellar_push(tlv_cellar_t *c, void* data)
{
	tlv_queue_node_t *q;

	q = (tlv_queue_node_t *)((char*)data + c->offset);
	if(q->prev)
	{
		q->prev->next=q->next;
	}
	if(q->next)
	{
		q->next->prev = q->prev;
	}else
	{
		if(q == c->use)
		{
			c->use = q->prev;
		}
	}
	if(c->cur_free < c->max_free)
	{

		q->prev = c->free;
		if(c->free)
		{
			c->free->next = q;
		}
		c->free = q;
		q->next = 0;
		++c->cur_free;
	}else
	{
		if(c->deleter)
		{
			c->deleter(data);
		}else if(c->deleter2)
		{
			c->deleter2(c->user_data, data);
		}
	}
    --c->use_length;

	return 0;
}

static void tlv_cellar_pack(tlv_cellar_t *c)
{
	tlv_queue_node_t *qn;
	int  cnt = c->cur_free-c->max_free;
	int  i;
	char *data;

	if(cnt <= 0)
	{
		return;
	}
	for(i=0; i<cnt; ++i)
	{
		qn = c->free;
		c->free = qn->prev;
		c->free->next = 0;
		data = (char*)qn-c->offset;
		if(c->deleter)
		{
			c->deleter(data);
		}else if(c->deleter2)
		{
			c->deleter2(c->user_data,data);
		}
	}
	c->cur_free = c->max_free;
}

void tlv_cellar_reuse(tlv_cellar_t *c)
{
	tlv_queue_node_t *n, *n2;

	for(n=c->use;n;n=n2)
	{
#ifdef _MSC_VER
		void *p;
		n2 = n->prev;
		p  = (unsigned)n-(unsigned)(c->offset);
		tlv_cellar_push(c, p);
#else
		n2 = n->prev;
		tlv_cellar_push(c, ((void*)(char*)n-c->offset));
#endif
	}

	tlv_cellar_pack(c);
}

int tlv_cellar_queue_bytes(tlv_cellar_t *c, tlv_cellar_bytes_f bf, tlv_queue_node_t *n)
{
	void *data;
	int b = 0;

	for(;n;n=n->prev)
	{
		data = (void*)((char*)n-c->offset);
		b    += bf(data);
	}

	return b;
}

int tlv_cellar_bytes(tlv_cellar_t *c, tlv_cellar_bytes_f bf)
{
	int b;

	b =  tlv_cellar_queue_bytes(c, bf, c->free);
	b += tlv_cellar_queue_bytes(c, bf, c->use);

	return b;
}
