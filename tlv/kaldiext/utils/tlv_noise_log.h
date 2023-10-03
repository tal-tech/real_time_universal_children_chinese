#ifndef TLV_LOG_WRAPPER_H_
#define TLV_LOG_WRAPPER_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * desc  : tlv_noise_log will open log when define _OPEN_NOISE_LOG
 */
#ifdef _OPEN_NOISE_LOG
#define tlv_noise_log(...) printf(__VA_ARGS__);fflush(stdout);
#else
#define tlv_noise_log(...)
#endif


#ifdef __cplusplus
};
#endif

#endif  // TLV_LOG_WRAPPER_H_
