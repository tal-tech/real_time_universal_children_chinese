#include "tlv_mem_heap.h"

static BlockP tlv_mem_alloc_block(size_t size, size_t num, tlv_memheap_type_t type);
static void tlv_mem_block_reorder(BlockP *p, int n);
static void *tlv_mem_get_elem(BlockP p, size_t elemSize, tlv_memheap_type_t type);

#define FWORD 8    /* size of a full word = basic alignment quanta */

size_t tlv_mem_round(size_t size)
{
	return ( (size % FWORD) == 0 ) ? size : ( size/FWORD + 1 ) * FWORD;
}


/**
 *  1.CreateHeap(&gstack, "Global Stack",  MSTAK, 1, 0.0, 100000, ULONG_MAX );
 *  2.CreateHeap(&gcheap, "Global C Heap", CHEAP, 1, 0.0, 0,      0 );
 *  3.CreateHeap (&slaHeap, "LatExpand a", MHEAP, sizeof (SubLArc), 1.0, 1000, 128000);
 *
 */
void tlv_mem_heap_create(tlv_mem_heap_t *x, char* name, tlv_memheap_type_t type, size_t elemSize, float growf, size_t numElem, size_t maxElem)
{
	x->name = (char*)malloc(strlen(name)+1);

	strcpy(x->name, name);
	x->type       = type;
	x->growf      = growf;
	x->elemSize   = elemSize;
	x->maxElem    = maxElem;
	x->curElem    = x->minElem = numElem;
	x->totAlloc   = x->totUsed = 0;
	x->heap       = NULL;
	x->protectStk = 0;
}


void tlv_mem_heap_reset(tlv_mem_heap_t* x)
{
	BlockP cur, next;

	switch(x->type)
	{
	case MHEAP:
		cur = x->heap;
		while(cur != NULL)
		{
			next = cur->next;
			free(cur->data);
			free(cur->used);
			free(cur);
			cur = next;
		}
		x->curElem  = x->minElem;
		x->totAlloc = 0;
		x->heap = NULL;
		break;

	case MSTAK:
		cur = x->heap;
		if(cur != NULL)
		{
			while(cur->next != NULL)
			{
				next = cur->next;
				x->totAlloc -= cur->numElem;
				free(cur->data);
				free(cur);
				cur = next;
			}
			x->heap = cur;
		}
		x->curElem = x->minElem;
		if(cur != NULL)
		{
			cur->numFree = cur->numElem;
			cur->firstFree = 0;
		}
		break;

	case CHEAP:
		break;

	}

	x->totUsed = 0;
}


void tlv_mem_heap_delete(tlv_mem_heap_t* x)
{
	tlv_mem_heap_reset(x);
	if(x->heap != NULL)
	{
		free(x->heap->data);
		free(x->heap);
	}

	free(x->name);
}

Ptr tlv_mem_new(tlv_mem_heap_t* x, size_t size)
{
	void *q;
	BlockP newp;
	size_t num, bytes, *ip, chdr;
	unsigned char noSpace;
	Ptr *pp;

	switch(x->type)
	{
	case MHEAP:
		noSpace = x->totUsed == x->totAlloc;
		if(noSpace || (q=tlv_mem_get_elem(x->heap, x->elemSize, x->type)) == NULL )
		{
			if(!noSpace) tlv_mem_block_reorder(&(x->heap), 1);
			if(noSpace || (q=tlv_mem_get_elem(x->heap, x->elemSize, x->type)) == NULL )
			{
				num = (size_t)((double)x->curElem * (x->growf + 1.0) + 0.5 );
				if(num > x->maxElem) num = x->maxElem;
				newp = tlv_mem_alloc_block(x->elemSize, num, x->type);
				x->totAlloc += num;
				x->curElem = num;
				newp->next = x->heap;
				x->heap = newp;
				q = tlv_mem_get_elem(x->heap, x->elemSize, x->type);
			}
		}

		x->totUsed++;
		return q;

	case CHEAP:
		chdr = tlv_mem_round(sizeof(size_t));
		q = malloc(size+chdr);
		x->totUsed += size;
		x->totAlloc += size + chdr;
		ip = (size_t*)q;
		*ip = size;

		return (Ptr)((ByteP)q + chdr);

	case MSTAK:
		/* set required size -- must alloc on double boundarise */
		if(x->protectStk) size += sizeof(Ptr);
		size = tlv_mem_round(size);
		/*  get elem from current block if possible */
		if((q=tlv_mem_get_elem(x->heap, size, x->type)) == NULL )
		{
			/* nospace - so add a new block */
			bytes = (size_t)((double)x->curElem * (x->growf + 1.0) + 0.5 );
			if(bytes > x->maxElem) bytes = x->maxElem;
			x->curElem = bytes;
			if(bytes < size) bytes = size;
			bytes = tlv_mem_round(bytes);
			newp = tlv_mem_alloc_block(1, bytes, x->type);
			x->totAlloc += bytes;
			newp->next = x->heap;
			x->heap = newp;
			q = tlv_mem_get_elem(x->heap, size, x->type);
		}

		x->totUsed += size;
		if(x->protectStk)
		{
			pp = (Ptr *)((long)q + size - sizeof(Ptr));
			*pp = q;
		}

		return q;

	}

	return NULL;
}

Ptr tlv_mem_cnew(tlv_mem_heap_t* x, size_t size)
{
	void *ptr;

	ptr = tlv_mem_new(x, size);
	if(x->type == MHEAP && size == 0)
		size = x->elemSize;

	memset(ptr, 0, size);

	return ptr;
}

void tlv_mem_dispose(tlv_mem_heap_t* x, Ptr p)
{
	BlockP head, cur, prev;
	unsigned char found = 0;
	ByteP bp;
	size_t size, chdr;
	size_t num, index, *ip;
	Ptr *pp;

	if(x->totUsed == 0)
	{
		tlv_log("Dispose: heap %s is empty", x->name);
	}

	switch(x->type)
	{
	case MHEAP:
		head = x->heap;
		cur = head;
		prev = NULL;
		size = x->elemSize;
		while(cur != NULL && !found)
		{
			num = cur->numElem;
			found = cur->data <= p &&
					(((void*)((ByteP)cur->data + (num-1)*size)) >= p );
			if(!found)
			{
				prev = cur;
				cur = cur->next;
			}
		}

		if(cur == NULL)
		{
			tlv_log("Dispose: Item to free in MHEAP %s not found\n", x->name);
		}

		index = ((size_t)p - (size_t)cur->data) / size;
		cur->used[index/8] &= ~(1 << (index&7));
		if(index < cur->firstFree) cur->firstFree = index;
		cur->numFree++;
		x->totUsed--;
		if(cur->numFree == cur->numElem)
		{
			if(cur != head)               /* free the whole block */
				prev->next = cur->next;
			else
				head = cur->next;
			x->heap = head;
			x->totAlloc -= cur->numElem;
			free(cur->data);
			free(cur->used);
			free(cur);
		}

		return;
	case MSTAK:
		/* search for item to dispose */
		cur = x->heap;
		if(x->protectStk)
		{
			if(cur->firstFree > 0)
				pp = (Ptr *)((size_t)cur->data + cur->firstFree - sizeof(Ptr));
			else
			{
				if(cur->next == NULL)
				{
					tlv_log("Dispose: empty stack\n");
				}
				pp = (Ptr *)((size_t)cur->next->data + cur->next->firstFree-sizeof(Ptr));
			}

			if(*pp != p)
			{
				tlv_log("Dispose: Dispose: violation of stack discipline in %s [%p != %p]", x->name, *pp, p);
			}
		} /* if(x->protectStk) */

		while(cur != NULL && !found)
		{
			/* check current block */
			num = cur->numElem;
			found = cur->data <= p &&
					(((void*)((ByteP)cur->data+num)) > p);
			if(!found)
			{
				x->heap = cur->next;
				x->totAlloc -= cur->numElem;
				x->totUsed -= cur->firstFree;
				free(cur->data);
				free(cur);
				cur = x->heap;
			}
		}

		/* finally cut back the stack in the current block */
		size = ((ByteP)cur->data + cur->firstFree) - (ByteP)p;
		if(((ByteP)cur->data + cur->firstFree) < (ByteP)p  )
		{
			tlv_log("Dispose: item to free in MSTAK %s is above stack top\n", x->name);
		}
		cur->firstFree -= size;
		cur->numFree += size;
		x->totUsed -= size;

		return;

		case CHEAP:
			chdr = tlv_mem_round(sizeof(size_t));
			bp = (ByteP)p - chdr;
			ip = (size_t *)bp;
			x->totAlloc -= (*ip + chdr);
			x->totUsed -= *ip;
			free(bp);
			return;

	}

}

/*----------------- static method -----------------*/
static BlockP tlv_mem_alloc_block(size_t size, size_t num, tlv_memheap_type_t type)
{
	BlockP p;
	ByteP c;
	int i;

	if((p = (BlockP)malloc(sizeof(Block))) == NULL )
	{
		tlv_log("AllocBlock: Cannot allocate Block\n");
	}
	if((p->data = (void*)malloc(size*num)) == NULL)
	{
		tlv_log("AllocBlock: Cannot allocate block data of %zu bytes\n", size*num);
	}

	switch(type) {
	case MHEAP:
		if((p->used = (ByteP)malloc((num+7)/8)) == NULL)
		{
			tlv_log("AllocBlock: Cannot allocate block used array\n");
		}
		for(i=0, c=p->used; i < (num+7)/8; i++,c++) *c = 0;
		break;
	case MSTAK:
		p->used = NULL;
		break;
	default:
		tlv_log("AllocBlock: bad type %d", type);
	}

	p->numElem = p->numFree = num;
	p->firstFree = 0;
	p->next = NULL;

	return p;
}

/**
 * @brief BlockReorder: reorder blks so that one with n free elems/bytes is 1st
 */
static void tlv_mem_block_reorder(BlockP *p, int n)
{
	BlockP head, cur, prev;

	if(p == NULL) return;
	head = cur = *p;
	prev = NULL;
	while(cur != NULL)
	{
		if(cur->numFree >= n)
		{
			if(prev != NULL)
			{
				prev->next = cur->next;
				cur->next = head;
			}
			*p = cur;
			return;
		}
		prev = cur;
		cur = cur->next;

	} /* while(cur != NULL) */
}

/* GetElem: return a pointer to the next free item in the block p */
static void *tlv_mem_get_elem(BlockP p, size_t elemSize, tlv_memheap_type_t type)
{
	int i, index;

	if(p == NULL) return NULL;
	switch(type)
	{
	case MHEAP:
		if(p->numFree == 0) return NULL;
		index = p->firstFree;
		p->used[p->firstFree/8] |= 1 << (p->firstFree&7);
		p->numFree--;
		/* Look thru 'used' bitmap for next free elem */
		if(p->numFree > 0)
		{
			for(i=p->firstFree+1; i < p->numElem; i++)
			{
				if( (p->used[i/8] & (1 << (i&7)) ) == 0 )
				{
					p->firstFree = i;
					break;
				}
			}
		}
		else
		{
			p->firstFree = p->numElem;  /* one over the end */
		}

		return (void*)((ByteP)p->data+index*elemSize);

	case MSTAK:
		/* take elemSize bytes from top of stack */
		if(p->numFree < elemSize) return NULL;
		index = p->firstFree;
		p->firstFree += elemSize;
		p->numFree -= elemSize;
		return (void*)((ByteP)p->data + index);
	default:
		tlv_log("GetElem: bad type %d", type);
	}

	return NULL;
}



/*----------------- end ----------------*/
