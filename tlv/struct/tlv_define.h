#ifndef WTK_STRUCT_TLV_DEFINE_H_
#define WTK_STRUCT_TLV_DEFINE_H_

#ifdef WIN32

#include <windows.h>

#else

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined __ANDROID__ || defined __IPHONE_OS__
#else
#include <sys/timeb.h>
#endif

#include <sys/time.h>

#include <semaphore.h>
#include <pthread.h>
#include <signal.h>

#endif /* #ifdef WIN32 */

#include <stdio.h>
#include <stdlib.h>

/* alloc memofy */
#define TLV_PLAT_WORD   ((int)(sizeof(unsigned long)))

#define tlv_align_p(p, align) \
	((align)>0 ? (void*)(((size_t)(p)+(align-1)) & (~(align-1))) : p)

#define tlv_round(size,align) \
 ((align)>0 ? (((size)+((align)-1))&(~((align)-1))) : size)

#define tlv_round_16(size) ((((size)&15)==0)? (size) : ((size)+16-((size)&15)))
#define tlv_round_word(size) tlv_round_16(size)

#define tlv_free(p)		free(p)
#define tlv_malloc(n)		malloc(n)
#define tlv_calloc(nmem,size) calloc(nmem,size)
/*-------------------  end  ----------------*/

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#define tlv_log(...) printf("%s:%d:",__FUNCTION__,__LINE__);printf(__VA_ARGS__);fflush(stdout);
#define tlv_str(x) #x
#define STR(X) tlv_str(X)
#define cat(x,y) x##y
#define CAT(X,Y) cat(X,Y)

#ifndef __cplusplus
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#endif

#define tlv_float_round(f) (int)((f)>0?(f+0.5):(f-0.5))

#ifdef WIN32
#if __STDC_VERSION__ < 199901L
typedef __int8 int8_t;
typedef __int32 int32_t;
typedef __int16 int16_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
#else
#include <stdint.h>
#endif
#else
#include <stdint.h>
#endif
#define data_offset(p,type,link) (type*)((p) ? (void*)((char*)p-offsetof(type,link)) : NULL)
#define data_offset2(p,type,link) (type*)((void*)((char*)p-offsetof(type,link)))

typedef void (*tlv_print_handler_f)(void *data);
typedef void (*tlv_write_f)(void *inst,const char *data,int bytes);

typedef int (*tlv_cmp_handler_f)(void *d1,void *d2);
typedef int (*tlv_walk_handler_f)(void *user_data,void* data);

typedef void* (*tlv_new_handler_f)(void* user_data);
typedef int (*tlv_delete_handler_f)(void* data);
typedef int (*tlv_delete_handler2_f)(void *user_data,void* data);

#ifdef __cplusplus
};
#endif

#endif /* WTK_STRUCT_TLV_DEFINE_H_ */
