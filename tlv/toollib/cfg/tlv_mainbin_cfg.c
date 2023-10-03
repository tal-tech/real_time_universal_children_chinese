#include "tlv_mainbin_cfg.h"
#include "tlv/struct/tlv_strfile.h"

tlv_mainbin_cfg_t* tlv_mainbin_cfg_new(int cfg_bytes,tlv_main_cfg_init_f init,
		tlv_main_cfg_clean_f clean,tlv_main_cfg_update_local_f update_lc,
		tlv_main_cfg_update2_f update,char *bin_fn,char *cfg_fn)
{
	tlv_mainbin_cfg_t *cfg;
	tlv_pack2_item_t *item;
	int ret;
	tlv_strfile_loader_t sl;

	cfg=(tlv_mainbin_cfg_t*)tlv_malloc(sizeof(tlv_mainbin_cfg_t));
	cfg->cfg=(void*)tlv_calloc(1,cfg_bytes);
	cfg->init=init;
	cfg->clean=clean;
	cfg->update_lc=update_lc;
	cfg->update=update;
	cfg->rbin=tlv_pack2_new();
	ret=tlv_pack2_read(cfg->rbin,bin_fn);
	if(ret!=0)
	{
		//tlv_log("read failed\n");
		goto end;
	}
	//tlv_log("f=%p\n",cfg->rbin->f);
	item=tlv_pack2_get2(cfg->rbin,cfg_fn,strlen(cfg_fn));
	if(!item){ret=-1;goto end;}
	cfg->cfile=tlv_cfg_file_new();
	//tlv_log("f=%p\n",cfg->rbin->f);
	tlv_cfg_file_add_var_ks(cfg->cfile,"pwd",".",1);
	ret=tlv_cfg_file_feed(cfg->cfile,item->data->data,item->data->len);
	if(ret!=0){goto end;}
	ret=cfg->init(cfg->cfg);
	if(ret!=0){goto end;}
	//tlv_log("update lc\n");
	ret=cfg->update_lc(cfg->cfg,cfg->cfile->main);
	if(ret!=0){goto end;}
	sl.hook=cfg->rbin;
	sl.vf=(tlv_strfile_loader_v_t)tlv_pack2_load_file;
	ret=cfg->update(cfg->cfg,&sl);
end:
	//tlv_log("ret=%d\n",ret);
	//tlv_log("f=%p\n",cfg->rbin->f);
	if(ret!=0)
	{
		tlv_mainbin_cfg_delete(cfg);
		cfg=NULL;
		ret=0;
	}
	return cfg;
}

void tlv_mainbin_cfg_delete(tlv_mainbin_cfg_t *cfg)
{
	cfg->clean(cfg->cfg);
	tlv_free(cfg->cfg);
	tlv_cfg_file_delete(cfg->cfile);
	tlv_pack2_delete(cfg->rbin);
	tlv_free(cfg);
}

