/*
 * tlv_errinfo.h
 *
 *  Created on: May 15, 2018
 *      Author: jfyuan
 *
 * 	code define:
 * 	错误码           错误描述              Error Description
 * 	30000           内核错误基码           Kernel error base code
 *  30001           未知错误              Unknown error
 *  30002	       	输入参数无效           Input params invalid
 *  30003           输入json解析失败       Input json parsing failed
 *  30004           输入json缺少必须的字段  Input json is missing the required fields
 *  30005           解码网络构建失败        Decoding network build failed
 *  30006           解码识别失败           Decoding recognition failed
 *  30007           计算溢出              nan/inf
 */

#ifndef TAL_STRUCT_TLV_ERRINFO_H_
#define TAL_STRUCT_TLV_ERRINFO_H_
#include "tlv_charbuf.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct tlv_errinfo tlv_errinfo_t;
struct tlv_errinfo
{
	int no;
	tlv_charbuf_t *buf;
};

tlv_errinfo_t* tlv_errinfo_new();
void tlv_errinfo_reset(tlv_errinfo_t *e);
void tlv_errinfo_delete(tlv_errinfo_t *e);
void tlv_errinfo_set(tlv_errinfo_t *e, int no, ...);

#ifdef __cplusplus
};
#endif

#endif /* TLV_ERRINFO_H_ */
