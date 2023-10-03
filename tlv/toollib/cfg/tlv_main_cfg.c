#include "tlv_main_cfg.h"

tlv_main_cfg_t *tlv_main_cfg_new(int cfg_bytes,tlv_main_cfg_init_f init,
		tlv_main_cfg_clean_f clean,tlv_main_cfg_update_local_f update_lc,
		tlv_main_cfg_update_f update,char *fn)
{
	return tlv_main_cfg_new2(cfg_bytes,init,clean,update_lc,update,fn,1);
}

tlv_main_cfg_t* tlv_main_cfg_new2(int cfg_bytes,tlv_main_cfg_init_f init,
		tlv_main_cfg_clean_f clean,tlv_main_cfg_update_local_f update_lc,
		tlv_main_cfg_update_f update,char *fn,int update_cfg)
{
	tlv_main_cfg_t *cfg;
	void *mc;
	int ret;

	cfg = (tlv_main_cfg_t*)tlv_calloc(1,sizeof(*cfg));
	cfg->init       = init;
	cfg->clean      = clean;
	cfg->update_lc  = update_lc;
	cfg->update     = update;
	cfg->update2    = NULL;
	//cfg->update_arg=0;
	cfg->cfg_bytes=cfg_bytes;
	mc=cfg->cfg=tlv_calloc(1,cfg_bytes);
	ret=cfg->init(mc);
	if(ret!=0)
	{
		tlv_log("init failed.\n");
		goto end;
	}
	if(fn)
	{
		cfg->cfile=tlv_cfg_file_new_fn(fn);
		if(!cfg->cfile)
		{
			tlv_log("%s invalid.\n",fn);
			ret=-1;goto end;
		}
	}else
	{
		cfg->cfile=0;
	}
	if(update_cfg)
	{
		ret=tlv_main_cfg_update_cfg(cfg);
	}
end:
	if(ret!=0)
	{
		tlv_main_cfg_delete(cfg);
		cfg=0;
	}
	return cfg;
}

tlv_main_cfg_t* tlv_main_cfg_new3(int cfg_bytes,tlv_main_cfg_init_f init,
		tlv_main_cfg_clean_f clean,tlv_main_cfg_update_local_f update_lc,
		tlv_main_cfg_update_f update,char *fn,int argc,char **argv)
{
	tlv_arg_t *arg;
	tlv_main_cfg_t *cfg;

	arg=tlv_arg_new(argc,argv);
	cfg=tlv_main_cfg_new4(cfg_bytes,init,clean,update_lc,update,fn,arg);
	tlv_arg_delete(arg);
	return cfg;
}

tlv_main_cfg_t* tlv_main_cfg_new4(int cfg_bytes,tlv_main_cfg_init_f init,
		tlv_main_cfg_clean_f clean,tlv_main_cfg_update_local_f update_lc,
		tlv_main_cfg_update_f update,char *fn,tlv_arg_t *arg)
{
	return tlv_main_cfg_new5(cfg_bytes,init,clean,update_lc,update,fn,arg,0);
}

tlv_main_cfg_t* tlv_main_cfg_new5(int cfg_bytes,tlv_main_cfg_init_f init,
		tlv_main_cfg_clean_f clean,tlv_main_cfg_update_local_f update_lc,
		tlv_main_cfg_update_f update,char *fn,tlv_arg_t *arg,char *cfg_section)
{
	return tlv_main_cfg_new6(cfg_bytes,init,clean,update_lc,update,0,fn,arg,cfg_section);
}

tlv_main_cfg_t* tlv_main_cfg_new6(int cfg_bytes,tlv_main_cfg_init_f init,
		tlv_main_cfg_clean_f clean,tlv_main_cfg_update_local_f update_lc,
		tlv_main_cfg_update_f update,
		tlv_main_cfg_update_arg_f update_arg,
		char *fn,tlv_arg_t *arg,char *cfg_section)
{
	tlv_main_cfg_t *cfg;
	int ret=0;

	cfg=tlv_main_cfg_new2(cfg_bytes,init,clean,update_lc,update,fn,0);
	if(!cfg){goto end;}
	if(arg)
	{
		tlv_part_cfg_update_arg(cfg->cfile->main,arg,1);
	}
	ret=tlv_main_cfg_update_cfg2(cfg,cfg_section);
	if(ret!=0)
	{
		tlv_log("update cfg failed.\n");
		goto end;
	}
	if(update_arg)
	{
		update_arg(cfg->cfg,arg);
	}
end:
	if(ret!=0)
	{
		tlv_main_cfg_delete(cfg);
		cfg=0;
	}
	return cfg;
}

int tlv_main_cfg_delete(tlv_main_cfg_t *cfg)
{
	if(cfg->cfg)
	{
		cfg->clean(cfg->cfg);
		tlv_free(cfg->cfg);
	}
	if(cfg->cfile)
	{
		tlv_cfg_file_delete(cfg->cfile);
	}
	tlv_free(cfg);
	return 0;
}

int tlv_main_cfg_bytes(tlv_main_cfg_t *cfg)
{
	int bytes=cfg->cfg_bytes;

	if(cfg->cfile)
	{
		bytes+=tlv_cfg_file_bytes(cfg->cfile);
	}
	return bytes;
}

void tlv_main_cfg_update(tlv_main_cfg_t *cfg,int argc,char **argv)
{
	tlv_arg_t *arg;

	arg=tlv_arg_new(argc,argv);
	tlv_part_cfg_update_arg(cfg->cfile->main,arg,0);
	tlv_arg_delete(arg);
}

int tlv_main_cfg_update_cfg_lc(tlv_main_cfg_t *cfg,tlv_part_cfg_t *lc)
{
	int ret;
	void *mc;
	tlv_strfile_loader_t sl;

    sl.hook = NULL;
    sl.vf = tlv_strfile_load_file_v;

	mc=cfg->cfg;
	ret=cfg->update_lc(mc,lc);
	if(ret!=0)
	{
		tlv_log("update lc failed\n");
		goto end;
	}
	ret=cfg->update(mc, &sl);
	if(ret!=0)
	{
		tlv_log("update failed\n");
		goto end;
	}
end:
	return ret;
}

int tlv_main_cfg_update_cfg(tlv_main_cfg_t *cfg)
{
	if(cfg->cfile)
	{
		return tlv_main_cfg_update_cfg_lc(cfg,cfg->cfile->main);
	}else
	{
		return 0;
	}
}

int tlv_main_cfg_update_cfg2(tlv_main_cfg_t *cfg,char *section)
{
	tlv_part_cfg_t *lc;

	if(section)
	{
		lc=tlv_part_cfg_find_section_lc(cfg->cfile->main,section,strlen(section));
	}else
	{
		lc=cfg->cfile->main;
	}
	return tlv_main_cfg_update_cfg_lc(cfg,lc);
}

#include "tlv/sphlib/pack/tlv_pack.h"

tlv_main_cfg_t *tlv_main_cfg_new_bin(int cfg_bytes,tlv_main_cfg_init_f init,
		tlv_main_cfg_clean_f clean,tlv_main_cfg_update_local_f update_lc,
		tlv_main_cfg_update2_f update,char *bin_fn,char *cfg_fn)
{
	tlv_main_cfg_t *cfg;
	tlv_pack_t *rbin;
	tlv_packitem_t *item;
	tlv_cfg_file_t *cf=0;
	void *xcfg;
	int ret=-1;
	tlv_strfile_loader_t sl;

	cfg=(tlv_main_cfg_t*)tlv_calloc(1,sizeof(*cfg));
	cfg->init=init;
	cfg->clean=clean;
	cfg->update_lc=update_lc;
	cfg->update=0;
	cfg->update2=update;
	rbin=tlv_pack_new();
	ret=tlv_pack_read(rbin,bin_fn);
	if(ret!=0){goto end;}
	item=tlv_pack_find(rbin,cfg_fn,strlen(cfg_fn));
	if(!item){goto end;}
	cf=tlv_cfg_file_new();
	tlv_cfg_file_add_var_ks(cf,"pwd",".",1);
	ret=tlv_cfg_file_feed(cf,item->data.data,item->data.len);
	if(ret!=0){goto end;}
	cfg->cfile=cf;
	xcfg=(void*)calloc(1,cfg_bytes);
	ret=cfg->init(xcfg);
	if(ret!=0){goto end;}
	ret=cfg->update_lc(xcfg,cfg->cfile->main);
	if(ret!=0){goto end;}
	sl.hook=rbin;
	sl.vf=(tlv_strfile_loader_v_t)tlv_pack_load_file;
	ret=cfg->update2(xcfg,&sl);
	if(ret!=0){goto end;}
	cfg->cfg_bytes=cfg_bytes;
	cfg->cfg=xcfg;
	cf=0;
	ret=0;
end:
	if(cf)
	{
		tlv_cfg_file_delete(cf);
	}
	tlv_pack_delete(rbin);
	if(ret!=0)
	{
		tlv_free(cfg);
		cfg=0;
	}
	return cfg;
}

tlv_main_cfg_t *tlv_main_cfg_new_str(int cfg_bytes,tlv_main_cfg_init_f init,
		tlv_main_cfg_clean_f clean,tlv_main_cfg_update_local_f update_lc,
		tlv_main_cfg_update_f update,char *data,int bytes,char *dn)
{
	tlv_main_cfg_t *cfg;
	void *mc;
	int ret;
	tlv_strfile_loader_t sl;

    sl.hook = NULL;
    sl.vf = tlv_strfile_load_file_v;

	cfg=(tlv_main_cfg_t*)tlv_calloc(1,sizeof(*cfg));
	cfg->init=init;
	cfg->clean=clean;
	cfg->update_lc=update_lc;
	cfg->update=update;
	cfg->update2=0;
	//cfg->update_arg=0;
	cfg->cfg_bytes=cfg_bytes;
	mc=cfg->cfg=tlv_calloc(1,cfg_bytes);
	ret=cfg->init(mc);
	if(ret!=0)
	{
		tlv_log("init failed.\n");
		goto end;
	}
	cfg->cfile=tlv_cfg_file_new();
	//tlv_cfg_file_add_var_ks(cfg->cfile,"pwd","res",3);
	tlv_cfg_file_add_var_ks(cfg->cfile,"pwd",dn,strlen(dn));
	ret=tlv_cfg_file_feed(cfg->cfile,data,bytes);
	if(ret!=0)
	{
		goto end;
	}
	//tlv_part_cfg_print(cfg->cfile->main);
	if(update_lc)
	{
		ret=update_lc(cfg->cfg,cfg->cfile->main);
		if(ret!=0){goto end;}
	}
	if(update)
	{
		ret=update(cfg->cfg, &sl);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	if(ret!=0)
	{
		tlv_main_cfg_delete(cfg);
		cfg=0;
	}
	return cfg;
}
