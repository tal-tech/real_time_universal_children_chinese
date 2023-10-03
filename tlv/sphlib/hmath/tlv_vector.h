#ifndef TAL_MATH_TLV_VECTOR_H_
#define TAL_MATH_TLV_VECTOR_H_
#include "tlv/struct/tlv_define.h"
#include "tlv/struct/tlv_heap.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef float tlv_vector_t;
typedef double tlv_vector_double_t;
typedef short tlv_vector_short_t;
typedef int tlv_vector_int_t;
typedef tlv_vector_t tlv_svector_t;

#define tlv_vector_type_bytes(size,type) (tlv_round((size+1)*sizeof(type),8))
#define tlv_vector_bytes(size) tlv_vector_type_bytes(size,float)
#define tlv_svector_bytes(size) ((size+1)*sizeof(float)+2*sizeof(void*))
#define tlv_vector_int_bytes(size) tlv_vector_type_bytes(size,int)
#define tlv_vector_double_bytes(size) tlv_vector_type_bytes(size,double)

#define tlv_vector_size(v) (*(int*)(v))
#define tlv_vector_init(v,size) (*((int*)v)=size)
#define tlv_vector_delete(v) tlv_free(v)
#define tlv_vector_short_size(v) (*(short*)v)

#define tlv_vector_type(type) CAT(CAT(tlv_vector_, type), _t)
#define tlv_vector_type_new_dec(t) tlv_vector_type(t)* CAT(CAT(tlv_vector_, t),_new)(int size)
#define tlv_vector_type_new_imp(t) \
tlv_vector_type_new_dec(t) \
{ \
	tlv_vector_type(t)* v; \
	\
	v=(tlv_vector_type(t)*)tlv_malloc(tlv_vector_type_bytes(size,t)); \
	if(sizeof(t)>=sizeof(int)) \
	{ \
		*((int*)v)=size; \
	}\
	{\
		*((short*)v)=size;\
	}\
	return v; \
}

#define tlv_vector_do_p(v,pre,after) \
{ \
	tlv_vector_t *s,*e;\
	s=v;e=v+tlv_vector_size(v);\
	while((e-s)>=4)\
	{\
		pre *(++s) after;\
		pre *(++s) after;\
		pre *(++s) after;\
		pre *(++s) after;\
	}\
	while(s<e)\
	{\
		pre *(++s) after; \
	}\
}

#define tlv_vector_do_i(v,pre,after) \
{ \
	int i,size; \
	i=1;size=tlv_vector_size(v);\
	for(i=1;i<=(size-4);)\
	{\
		pre v[i] after;++i;\
		pre v[i] after;++i;\
		pre v[i] after;++i;\
		pre v[i] after;++i;\
	}\
	for(;i<=size;++i)\
	{\
		pre v[i] after;\
	}\
}

tlv_vector_t* tlv_vector_new(int size);
tlv_vector_t* tlv_vector_new_h(tlv_heap_t *heap, int size);
tlv_vector_int_t* tlv_vector_int_new_h(tlv_heap_t *heap, int size);
tlv_vector_type_new_dec(short);

void tlv_vector_cpy(tlv_vector_t *src,tlv_vector_t *dst);
void tlv_vector_zero(tlv_vector_t *v);
float tlv_vector_max_abs(tlv_vector_t *v);
void tlv_vector_fix_scale(tlv_vector_t *v,float scale);

/* tlv_vector_double_t */
tlv_vector_type_new_dec(double);
tlv_vector_double_t* tlv_vector_double_newh(tlv_heap_t* heap, int size);
void tlv_vector_double_cpy(tlv_vector_double_t *src,tlv_vector_double_t *dst);
void tlv_vector_double_zero(tlv_vector_double_t *v);

tlv_svector_t* tlv_svector_newh(tlv_heap_t* heap, int size);
tlv_svector_t* tlv_svector_dup(tlv_heap_t* heap, tlv_svector_t *src);

void tlv_set_use(void **m, int n);
void tlv_inc_use(void **m);
void tlv_set_hook(void **m, void *h);
void* tlv_get_hook(void **m);
float tlv_math_max(float *a, int len);

/*---------- print --------*/
void tlv_vector_print(tlv_vector_t* v);
#ifdef __cplusplus
};
#endif
#endif
