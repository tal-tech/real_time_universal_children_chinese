#ifndef WTK_CORE_CFG_tlv_main_CFG_H_
#define WTK_CORE_CFG_tlv_main_CFG_H_
#include "tlv/sphlib/cfg/tlv_cfg_file.h"
#include "tlv/sphlib/cfg/tlv_arg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct tlv_main_cfg tlv_main_cfg_t;
typedef int (*tlv_main_cfg_init_f)(void *cfg);
typedef int (*tlv_main_cfg_clean_f)(void *cfg);
typedef int (*tlv_main_cfg_update_local_f)(void *cfg,tlv_part_cfg_t *lc);
typedef int (*tlv_main_cfg_update_f)(void *cfg, tlv_strfile_loader_t *sl);
typedef int (*tlv_main_cfg_update2_f)(void *cfg,tlv_strfile_loader_t *sl);
typedef void (*tlv_main_cfg_update_arg_f)(void *cfg,tlv_arg_t *arg);

#define tlv_main_cfg_new_type(type,fn) tlv_main_cfg_new(sizeof(CAT(type,_t)), \
		(tlv_main_cfg_init_f)CAT(type,_init),\
		(tlv_main_cfg_clean_f)CAT(type,_clean),\
		(tlv_main_cfg_update_local_f)CAT(type,_load_param), \
		(tlv_main_cfg_update_f)CAT(type,_load_res),fn)

#define tlv_main_cfg_new_str_type(type,data,bytes,dn) tlv_main_cfg_new_str(sizeof(CAT(type,_t)), \
		(tlv_main_cfg_init_f)CAT(type,_init),\
		(tlv_main_cfg_clean_f)CAT(type,_clean),\
		(tlv_main_cfg_update_local_f)CAT(type,_update_local), \
		(tlv_main_cfg_update_f)CAT(type,_update),data,bytes,dn)

#define tlv_main_cfg_new_type2(type,fn,arg) tlv_main_cfg_new6(sizeof(CAT(type,_t)), \
		(tlv_main_cfg_init_f)CAT(type,_init),\
		(tlv_main_cfg_clean_f)CAT(type,_clean),\
		(tlv_main_cfg_update_local_f)CAT(type,_update_local), \
		(tlv_main_cfg_update_f)CAT(type,_update),\
		(tlv_main_cfg_update_arg_f)CAT(type,_update_arg),fn,arg,0)

#define tlv_main_cfg_new_type3(type,fn,section) tlv_main_cfg_new6(sizeof(CAT(type,_t)), \
		(tlv_main_cfg_init_f)CAT(type,_init),\
		(tlv_main_cfg_clean_f)CAT(type,_clean),\
		(tlv_main_cfg_update_local_f)CAT(type,_update_local), \
		(tlv_main_cfg_update_f)CAT(type,_update),\
		(tlv_main_cfg_update_arg_f)0,fn,0,section)


struct tlv_main_cfg
{
	tlv_cfg_file_t *cfile;
	void* cfg;							
	int cfg_bytes;
	tlv_main_cfg_init_f init;
	tlv_main_cfg_clean_f clean;
	tlv_main_cfg_update_local_f update_lc;
	tlv_main_cfg_update_f update;
	tlv_main_cfg_update2_f update2;
};

tlv_main_cfg_t *tlv_main_cfg_new(int cfg_bytes,tlv_main_cfg_init_f init,
		tlv_main_cfg_clean_f clean,tlv_main_cfg_update_local_f update_lc,
		tlv_main_cfg_update_f update,char *fn);

/**
 * @brief create configure, update configure or not by update_cfg;
 */
tlv_main_cfg_t *tlv_main_cfg_new2(int cfg_bytes,tlv_main_cfg_init_f init,
		tlv_main_cfg_clean_f clean,tlv_main_cfg_update_local_f update_lc,
		tlv_main_cfg_update_f update,char *fn,int update_cfg);

/**
 * @brief update from argc,argv;
 */
tlv_main_cfg_t* tlv_main_cfg_new3(int cfg_bytes,tlv_main_cfg_init_f init,
		tlv_main_cfg_clean_f clean,tlv_main_cfg_update_local_f update_lc,
		tlv_main_cfg_update_f update,char *fn,int argc,char **argv);
/**
 * @brief update from arg;
 */
tlv_main_cfg_t *tlv_main_cfg_new4(int cfg_bytes,tlv_main_cfg_init_f init,
		tlv_main_cfg_clean_f clean,tlv_main_cfg_update_local_f update_lc,
		tlv_main_cfg_update_f update,char *fn,tlv_arg_t *arg);

/**
 *@param cfg_section, if cfg_section is "http:nk", update by nk local cfg;
 */
tlv_main_cfg_t* tlv_main_cfg_new5(int cfg_bytes,tlv_main_cfg_init_f init,
		tlv_main_cfg_clean_f clean,tlv_main_cfg_update_local_f update_lc,
		tlv_main_cfg_update_f update,char *fn,tlv_arg_t *arg,char *cfg_section);

tlv_main_cfg_t* tlv_main_cfg_new6(int cfg_bytes,tlv_main_cfg_init_f init,
		tlv_main_cfg_clean_f clean,tlv_main_cfg_update_local_f update_lc,
		tlv_main_cfg_update_f update,
		tlv_main_cfg_update_arg_f update_arg,
		char *fn,tlv_arg_t *arg,char *cfg_section);


tlv_main_cfg_t *tlv_main_cfg_new_bin(int cfg_bytes,tlv_main_cfg_init_f init,
		tlv_main_cfg_clean_f clean,tlv_main_cfg_update_local_f update_lc,
		tlv_main_cfg_update2_f update,char *bin_fn,char *cfg_fn);

int tlv_main_cfg_delete(tlv_main_cfg_t *cfg);
void tlv_main_cfg_update(tlv_main_cfg_t *cfg,int argc,char **argv);
int tlv_main_cfg_update_cfg(tlv_main_cfg_t *cfg);

int tlv_main_cfg_bytes(tlv_main_cfg_t *cfg);
/**
 * @param section,if section is "httpd:nk",update by nk local cfg;
 */
int tlv_main_cfg_update_cfg2(tlv_main_cfg_t *cfg,char *section);


tlv_main_cfg_t *tlv_main_cfg_new_str(int cfg_bytes,tlv_main_cfg_init_f init,
		tlv_main_cfg_clean_f clean,tlv_main_cfg_update_local_f update_lc,
		tlv_main_cfg_update_f update,char *data,int bytes,char *dn);
#ifdef __cplusplus
};
#endif
#endif
