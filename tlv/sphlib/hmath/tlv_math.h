#ifndef TAL_SPHLIB_TLV_MATH_H_
#define TAL_SPHLIB_TLV_MATH_H_
#include <math.h>
#include "tlv_vector.h"
#include "tlv/struct/tlv_strfile.h"
#include "tlv_matrix.h"
#ifdef __cplusplus
extern "C" {
#endif
#ifdef PI
#undef PI                /* PI is defined in Linux */
#endif
#define PI   3.14159265358979
#define LZERO  (-1.0E10)   /* ~log(0) */
#define LSMALL (-0.5E10)   /* log values < LSMALL are set to LZERO */
#define MINLARG 2.45E-308  /* lowest log() arg  = exp(MINEARG) */

/**
 * @date	2018.3.22
 * @auth	jfyuan
 * @brief	常规运算函数
 */
void tlv_float_sigmoid(float *f, int len);
void tlv_float_softmax(float* f, int len);
double tlv_log_add(double x, double y, double min_log_exp);

tlv_vector_t* tlv_math_create_ham_window(int frame_size);
void tlv_vector_zero_mean_frame(tlv_vector_t* v);
void tlv_vector_pre_emphasise(tlv_vector_t* v, float k);
void tlv_vector_realft(tlv_vector_t* v);
void tlv_math_do_diff(tlv_vector_t** pv, int window_size, double sigma, int start_pos, int step);
void tlv_math_do_simple_diff(tlv_vector_t** pv, int window_size, int start_pos, int step);
void tlv_matrix_multiply_vector(tlv_vector_t *dst, tlv_matrix_t *m, tlv_vector_t *src);

int tlv_strfile_read_vector(tlv_strfile_t* s, tlv_vector_t* v, int bin);
int tlv_strfile_read_matrix(tlv_strfile_t *s, tlv_matrix_t *m, int bin);
int tlv_strfile_read_short_matrix(tlv_strfile_t *s, tlv_matrix_t *m, int bin, float scale);
int tlv_strfile_read_hlda(tlv_strfile_t *s, tlv_matrix_t **pm);
int tlv_hlda_read(tlv_matrix_t **pm, tlv_strfile_t *s);
int tlv_strfile_read_hlda_bin(tlv_matrix_t **pm, tlv_strfile_t *s);

#ifdef __cplusplus
};
#endif
#endif
