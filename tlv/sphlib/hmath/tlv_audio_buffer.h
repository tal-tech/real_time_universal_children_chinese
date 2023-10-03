#ifndef TAL_MATH_TLV_AUDIO_BUFFER_H_
#define TAL_MATH_TLV_AUDIO_BUFFER_H_
#include "tlv_vector.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct tlv_audio_buffer tlv_audio_buffer_t;
#define tlv_audio_buffer_valid_len(b) ((b->cur)-(b->start))
#define tlv_audio_buffer_left_samples(b) ((b->end)-(b->cur))

struct tlv_audio_buffer
{
	float *rstart;
	float *start;
	float *cur;
	float *end;
	char odd_char;
	unsigned odd:1;
};

tlv_audio_buffer_t *tlv_audio_buffer_new(int size);

int tlv_audio_buffer_push(tlv_audio_buffer_t *b, short* data, int samples);
int tlv_audio_buffer_push_c(tlv_audio_buffer_t *b, char *data, int bytes);
int tlv_audio_buffer_peek(tlv_audio_buffer_t *b, tlv_vector_t *v, int is_end);
void tlv_audio_buffer_skip(tlv_audio_buffer_t *b, int samples, int left_enough);
float* tlv_audio_buffer_peek_data(tlv_audio_buffer_t *b, int samples);

int tlv_audio_buffer_delete(tlv_audio_buffer_t *b);
int tlv_audio_buffer_reset(tlv_audio_buffer_t *b);
#ifdef __cplusplus
};
#endif
#endif
