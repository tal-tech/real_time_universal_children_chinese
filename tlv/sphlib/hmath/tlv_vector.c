#include "tlv_vector.h"

tlv_vector_type_new_imp(short);

tlv_vector_t *tlv_vector_new(int size)
{
	tlv_vector_t *v;

	v = (tlv_vector_t*)tlv_calloc(1, tlv_vector_bytes(size));
	tlv_vector_init(v, size);

	return v;
}

tlv_vector_t* tlv_vector_new_h(tlv_heap_t *h, int size)
{
	tlv_vector_t* v;

	v = (tlv_vector_t*)tlv_heap_malloc(h, tlv_vector_bytes(size));
	tlv_vector_init(v,size);

	return v;
}

tlv_vector_int_t* tlv_vector_int_new_h(tlv_heap_t *h, int size)
{
	tlv_vector_int_t *v;

	v = (tlv_vector_int_t*)tlv_heap_malloc(h, tlv_vector_int_bytes(size));
	tlv_vector_init(v, size);

	return v;
}

void tlv_vector_cpy(tlv_vector_t *src, tlv_vector_t *dst)
{
	int i, size;

	size = tlv_vector_size(src);
	for(i=1; i<=size; ++i)
	{
		dst[i] = src[i];
	}
}

void tlv_vector_double_cpy(tlv_vector_double_t *src, tlv_vector_double_t *dst)
{
	int i, size;

	size = tlv_vector_size(src);
	for(i=1; i<=size; ++i)
	{
		dst[i] = src[i];
	}
}

void tlv_vector_zero(tlv_vector_t *v)
{
	int i,n;

	n=tlv_vector_size(v);
	for(i=1;i<=n;++i)
	{
		v[i]=0;
	}
}

void tlv_vector_fix_scale(tlv_vector_t *v, float scale)
{
	float f;
	int *pi,*pe;
	int n;

	n  = tlv_vector_size(v);
	pi = ((int*)v)+1;
	pe = pi+n;
	while(pi < pe)
	{
		f = (*((float*)pi))*scale;
		*pi = tlv_float_round(f);
		++pi;
	}

}

float tlv_vector_max_abs(tlv_vector_t *v)
{
	float min = 10000;
	float max = -10000;
	int i,n;

	n = tlv_vector_size(v);
	for(i=1; i<n; ++i)
	{
		if(v[i] > max)
		{
			max = v[i];
		}
		if(v[i] < min)
		{
			min = v[i];
		}
	}
	if(min < 0)
	{
		min = -min;
	}
	if(max < 0)
	{
		max = -max;
	}
	if(max > min)
	{
		return max;
	}else
	{
		return min;
	}
}

/*=========  tlv_vector_double_t  ============*/
tlv_vector_double_t* tlv_vector_double_newh(tlv_heap_t* heap, int size)
{
	tlv_vector_double_t *v;

	v = (tlv_vector_double_t*)tlv_heap_malloc(heap, tlv_vector_type_bytes(size, double));
	if(sizeof(double) >= sizeof(int))
	{
		*((int*)v) = size;
	}
	else
	{
		*((short*)v) = size;
	}

	return v;
}

void tlv_vector_double_zero(tlv_vector_double_t *v)
{
	int i,n;

	n=tlv_vector_size(v);
	for(i=1;i<=n;++i)
	{
		v[i]=0;
	}
}

/*----------------------------------*/

tlv_svector_t* tlv_svector_newh(tlv_heap_t* heap, int size)
{
	tlv_svector_t* v;
	void *p;

	p=(void*)tlv_heap_malloc(heap,tlv_svector_bytes(size));
	v=(tlv_svector_t*)((char*)p+sizeof(void*)*2);
	(*(int*)v)=size;
	tlv_set_hook((void**)v,0);
	tlv_set_use((void**)v,0);
	return v;
}

tlv_svector_t* tlv_svector_dup(tlv_heap_t* heap, tlv_svector_t *src)
{
	tlv_svector_t *dst;

	dst=tlv_svector_newh(heap,tlv_vector_size(src));
	tlv_vector_cpy(src,dst);
	return dst;
}

void tlv_set_use(void **m,int n)
{
	*((int*)(m-1))=n;
}

void tlv_inc_use(void **m)
{
	++*((int*)(m-1));
}

void tlv_set_hook(void **m,void *h)
{
	*(m-2)=h;
}

void* tlv_get_hook(void **m)
{
	return *(m-2);
}


float tlv_math_max(float *a,int len)
{
	float max;
	float *s,*e;

	max=a[0];
	s=a+1;
	e=s+len-1;
	while(s<e)
	{
		if(*s>max)
		{
			max=*s;
		}
		++s;
	}
	return max;
}

void tlv_vector_print(tlv_vector_t* v)
{
	float t;

	tlv_log("========== vector ==========\n");
#ifdef INLINE
	tlv_vector_do_i(v,t=,;printf("%.3f %s",t,i%10==0?"\n":""));
	printf("\n");
#else
	tlv_vector_do_i(v,t=,;printf("v[%d]=%f\n",i,t));
#endif

}


