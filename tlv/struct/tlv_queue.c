#include "tlv_queue.h"
#include "tlv_heap.h"

void tlv_queue_init(tlv_queue_t *q)
{
	memset(q, 0, sizeof(*q));
}

int tlv_queue_push(tlv_queue_t *q,tlv_queue_node_t *n)
{
	n->prev=q->front;
	if(q->front)
	{
		q->front->next=n;
	}
	n->next=0;
	q->front=n;
	if(!q->rear)
	{
		q->rear=n;
	}

	++q->length;
	return 0;
}


int tlv_queue_push_front(tlv_queue_t *q,tlv_queue_node_t *n)
{
	n->next=q->rear;
	if(q->rear)
	{
		q->rear->prev=n;
	}
	n->prev=0;
	q->rear=n;
	if(!q->front)
	{
		q->front=n;
	}

	++q->length;
	return 0;
}

void tlv_queue_insert_to(tlv_queue_t *q,tlv_queue_node_t *n,tlv_queue_node_t* n2)
{
	if(n==q->front)
	{
		tlv_queue_push(q,n2);
	}else
	{
		n2->prev=n;
		n2->next=n->next;
		n2->next->prev=n2->prev->next=n2;
		++q->length;
	}
	return;
}

tlv_queue_node_t* tlv_queue_pop(tlv_queue_t *q)
{
	tlv_queue_node_t* n;

	if(q->length<=0){return 0;}
	n=0;
	if(!q->rear){goto end;}
	n=q->rear;
	q->rear=q->rear->next;
	if(q->rear)
	{
		q->rear->prev=0;
	}else
	{
		q->front=0;
	}
	--q->length;
end:
	return n;
}

int tlv_queue_remove(tlv_queue_t *q,tlv_queue_node_t *n)
{
	if(q->length<=0){return 0;}
	if(n->prev)
	{
		n->prev->next=n->next;
	}else
	{
		q->rear=n->next;
	}
	if(n->next)
	{
		n->next->prev=n->prev;
	}else
	{
		q->front=n->prev;
	}
	n->prev=n->next=0;
    --q->length;
	return 0;
}


void tlv_queue_touch_node(tlv_queue_t *q,tlv_queue_node_t *n)
{
	tlv_queue_remove(q,n);
	tlv_queue_push(q,n);
}

void* tlv_queue_find(tlv_queue_t *q, int offset, tlv_cmp_handler_f cmp, void *user_data)
{
	tlv_queue_node_t *n,*p;
	void *data;
	void *v;

	v=0;
	for(n=q->rear;n;n=p)
	{
		p=n->next;
		data=(void*)tlv_queue_node_data_offset2(n,offset);
		if(cmp(user_data,data)==0)
		{
			v=data;
			break;
		}
	}
	return v;
}

int tlv_queue_walk(tlv_queue_t *q, int offset, tlv_walk_handler_f walk, void *user_data)
{
	tlv_queue_node_t *n,*p;
	void *data;
	int ret=0;

	for(n=q->rear;n;n=p)
	{
		p = n->next;
		data = (void*)tlv_queue_node_data_offset2(n, offset);
		ret = walk(user_data, data);
		if(ret!=0){goto end;}
	}
end:
	return ret;
}

void tlv_queue_insert(tlv_queue_t *q,tlv_queue_node_t *n,tlv_cmp_handler_f cmp)
{
	tlv_queue_node_t *xn,*prev=0;
	int ret;

	for(xn=q->rear;xn;xn=xn->next)
	{
		ret=cmp(n,xn);
		if(ret>0)
		{
			prev=xn->prev;
			break;
		}
		if(!xn->next)
		{
			prev=xn;
		}
	}
	if(prev)
	{
		tlv_queue_insert_to(q,prev,n);
	}else
	{
		tlv_queue_push_front(q,n);
	}
}


void tlv_queue_link(tlv_queue_t *dst, tlv_queue_t *src)
{
	tlv_queue_node_t *n = src->rear;

	if(src->length <= 0) { return; }
	if(dst->length == 0)
	{
		dst->rear = dst->front = 0;
	}
	n->prev = dst->front;
	if(dst->front)
	{
		dst->front->next = n;
	}
	dst->front = src->front;
	if(!dst->rear)
	{
		dst->rear = n;
	}
	dst->length += src->length;
}

void tlv_queue_print(tlv_queue_t *q, int offset, tlv_print_handler_f print)
{
	tlv_queue_node_t *n,*p;
	void *data;

	for(n=q->rear;n;n=p)
	{
		p=n->next;
		data=(void*)tlv_queue_node_data_offset2(n,of);
		print(data);
	}
}
