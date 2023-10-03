#ifndef TAL_TOOLLIB_TLV_MAINBIN_CFG_H_
#define TAL_TOOLLIB_TLV_MAINBIN_CFG_H_
#include "tlv/toollib/cfg/tlv_main_cfg.h"
#include "tlv/sphlib/pack/tlv_pack2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct tlv_mainbin_cfg tlv_mainbin_cfg_t;
#define tlv_mainbin_cfg_new_type(type,fn,cfg_fn) tlv_mainbin_cfg_new(sizeof(CAT(type,_t)), \
		(tlv_main_cfg_init_f)CAT(type,_init),\
		(tlv_main_cfg_clean_f)CAT(type,_clean),\
		(tlv_main_cfg_update_local_f)CAT(type,_load_param), \
		(tlv_main_cfg_update2_f)CAT(type,_load_res),fn,cfg_fn)

struct tlv_mainbin_cfg
{
	void *cfg;
	tlv_cfg_file_t *cfile;
	tlv_pack2_t *rbin;
	tlv_main_cfg_init_f init;
	tlv_main_cfg_clean_f clean;
	tlv_main_cfg_update_local_f update_lc;
	tlv_main_cfg_update2_f update;
};


tlv_mainbin_cfg_t* tlv_mainbin_cfg_new(int cfg_bytes,tlv_main_cfg_init_f init,
		tlv_main_cfg_clean_f clean,tlv_main_cfg_update_local_f update_lc,
		tlv_main_cfg_update2_f update,char *fn, char *cfg_fn);
void tlv_mainbin_cfg_delete(tlv_mainbin_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
