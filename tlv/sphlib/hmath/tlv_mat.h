#ifndef tlv_vite_MATH_WTK_MAT_H_
#define tlv_vite_MATH_WTK_MAT_H_
#include "tlv/struct/tlv_define.h"
#include "tlv_matrix.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	signed char *p;
	unsigned int row;
	unsigned int col;
	unsigned int p_row;
	unsigned int p_col;
	unsigned int p_row_x;
	unsigned int p_col_x;
}tlv_matc_t;

tlv_matc_t* tlv_matc_new(int row, int col);
tlv_matc_t* tlv_matc_new2(tlv_matrix_t *m, float scale);
tlv_matc_t* tlv_matc_new3(tlv_matrix_t *m, float scale);
void tlv_matc_delete(tlv_matc_t *m);
void tlv_matc_print(tlv_matc_t *mc);

typedef struct
{
	unsigned char *p;
	unsigned int row;
	unsigned int col;
	unsigned int p_row;
	unsigned int p_col;
	unsigned int p_row_x;
	unsigned int p_col_x;
}tlv_matuc_t;


tlv_matuc_t* tlv_matuc_new(int row,int col);
void tlv_matuc_init(tlv_matuc_t* cm, tlv_matrix_t *m, float scale);
void tlv_matuc_delete(tlv_matuc_t *mc);
void tlv_matuc_print(tlv_matuc_t *mc);

typedef struct
{
	int *p;
	unsigned int row;
	unsigned int col;
	unsigned int p_row;
	unsigned int p_col;
	unsigned int p_row_x;
	unsigned int p_col_x;
}tlv_mati_t;

tlv_mati_t* tlv_mati_new(int row, int col);
tlv_mati_t* tlv_mati_new2(tlv_matrix_t *m, float scale);
void tlv_mati_init(tlv_mati_t *im, tlv_matrix_t *m, float scale);
void tlv_mati_multi(tlv_mati_t *m,tlv_matuc_t *a,tlv_matc_t *b);
void tlv_mati_multi2(tlv_mati_t *m,tlv_mati_t *a,tlv_matc_t *b);
void tlv_mati_multi3(tlv_mati_t *m,tlv_mati_t *a,tlv_mati_t *b);
void tlv_mati_multi_dc(tlv_mati_t *m,tlv_mati_t *a,tlv_matc_t *b,int nx);
void tlv_mati_add(tlv_mati_t *a,tlv_mati_t *b);
void tlv_mati_delete(tlv_mati_t *im);
void tlv_mati_print(tlv_mati_t *mi);

typedef tlv_mati_t* (*tlv_mati_heap_new_f)(void *ths,int row,int col);
typedef void (*tlv_mati_heap_delete_f)(void *ths,tlv_mati_t *m);

typedef struct
{
	void *ths;
	tlv_mati_heap_new_f mat_new;
	tlv_mati_heap_delete_f mat_delete;
}tlv_mati_heap_t;

void tlv_mati_multi_dc_ext(tlv_mati_t *c,tlv_mati_t *a,tlv_matc_t *b,
		tlv_mati_heap_t *heap,int NX);
void tlv_mati_multi_dc2(tlv_mati_t *c,tlv_mati_t *a,tlv_mati_t *b,tlv_mati_heap_t *heap,int NX);
#ifdef __cplusplus
};
#endif
#endif
