#ifndef WTK_MATH_tlv_matrix_H_
#define WTK_MATH_tlv_matrix_H_
#include "tlv_vector.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef float* tlv_matrix_t;
typedef float* tlv_smatrix_t;
typedef double* tlv_matrix_double_t;

#define tlv_matrix_bytes(r,c) tlv_round_word(tlv_vector_bytes(c)*r+tlv_round_word((r+1)*sizeof(tlv_vector_t*)))
#define tlv_matrix_rows(m) (*(int*)m)
#define tlv_matrix_cols(m) (*(int*)(m[1]))
#define tlv_matrix_delete(m) free(m)

#define tlv_matrix_double_bytes(r,c) tlv_round_word(tlv_vector_double_bytes(c)*r+tlv_round_word((r+1)*sizeof(tlv_vector_double_t*)))
#define tlv_smatrix_bytes(r,c) tlv_round_word(tlv_svector_bytes(c)*r+tlv_round_word((r+3)*sizeof(tlv_vector_t*)))

tlv_matrix_t* tlv_matrix_new(int nrows, int ncols);
tlv_matrix_t* tlv_matrix_new2(int nrows, int ncols);
tlv_matrix_t* tlv_matrix_newh(tlv_heap_t* h, int nrows, int ncols);

void tlv_matrix_multi(tlv_matrix_t *m, tlv_matrix_t *a, tlv_matrix_t *b);
void tlv_matrix_multi2(tlv_matrix_t *m, tlv_matrix_t *a, tlv_matrix_t *b);
void tlv_matrix_multi3(tlv_matrix_t *m, tlv_matrix_t *a, tlv_matrix_t *b);
void tlv_matrix_transpose(tlv_matrix_t *dst, tlv_matrix_t *src);
tlv_matrix_t* tlv_matrix_transpose2(tlv_matrix_t *a);
void tlv_matrix_cpy(tlv_matrix_t *src, tlv_matrix_t *dst);
void tlv_matrix_scale(tlv_matrix_t *m, float scale);
void tlv_matrix_add(tlv_matrix_t *m, tlv_matrix_t *a);

double tlv_matrix_max(tlv_matrix_t *m);
double tlv_matrix_min(tlv_matrix_t *m);
double tlv_matrix_max_abs(tlv_matrix_t *m);

/* smatrix */
tlv_smatrix_t* tlv_smatrix_newh(tlv_heap_t *h,int nrows,int ncols);

/* double matrix */
tlv_matrix_double_t* tlv_matrix_double_new(int nrows, int ncols);
tlv_matrix_double_t* tlv_matrix_double_new_h(tlv_heap_t *heap, int nrows, int ncols);
void tlv_matrix_double_zero(tlv_matrix_double_t *m);
void tlv_matrix_double_cpy(tlv_matrix_double_t *src, tlv_matrix_double_t *dst);
void tlv_matrix_double_init_identity(tlv_matrix_double_t *A);

/* print */
void tlv_matrix_print(tlv_matrix_t *m);

#ifdef __cplusplus
};
#endif
#endif
