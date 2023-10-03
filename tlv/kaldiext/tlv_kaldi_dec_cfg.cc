/*
 * tlv_kaldi_dec_cfg.cc
 *
 *  Created on: Jan 3, 2019
 *      Author: jfyuan
 */

#include "tlv_kaldi_dec_cfg.h"
#include <string>
#include "tlv/kaldiext/vadparam/vadparamloader.h"

int tlv_kaldi_dec_cfg_init(tlv_kaldi_dec_cfg_t* cfg)
{
	int ret=0;

    std::string cfg_path = cfg->cfg_path;
    memset(cfg, 0, sizeof(tlv_kaldi_dec_cfg_t));
	cfg->version = "cn.asr.0.0.1";
	cfg->is_online = 1;
	cfg->language = "ch";
    cfg->cfg_path = cfg_path;
	ret |= tlv_kdext_feat_cfg_init(&(cfg->feat));

#ifdef USE_NNET1
	ret |= tlv_kdext_forward_cfg_init(&(cfg->forward));
#endif

	ret |= tlv_kdext_rec_cfg_init(&(cfg->rec));

    cfg->vad_params = NULL;

	return ret;
}

int tlv_kaldi_dec_cfg_load_param(tlv_kaldi_dec_cfg_t* cfg, tlv_part_cfg_t *part)
{
	int ret = 0;
	tlv_part_cfg_t *lc = part;
	tlv_string_t   *str;

	tlv_part_cfg_update_cfg_str(lc, cfg, version, str);
	tlv_part_cfg_update_cfg_b(lc, cfg, use_nnet3, str);
	tlv_part_cfg_update_cfg_b(lc, cfg, is_online, str);
	tlv_part_cfg_update_cfg_str(lc, cfg, language, str);
	/* for debug */
	tlv_part_cfg_update_cfg_b(lc, cfg, use_log, str);

	lc = tlv_part_cfg_find_lc_s(part, "FEA");
	if(lc)
	{
		ret |= tlv_kdext_feat_cfg_load_param(&cfg->feat, lc);
		if(0 != ret) { goto end; }
		cfg->feat.use_log   = cfg->use_log;
		cfg->feat.is_online = cfg->is_online;
	}

	lc = tlv_part_cfg_find_lc_s(part, "NET");
	if(lc)
	{
#ifdef USE_NNET1
		ret |= tlv_kdext_forward_cfg_load_param(&cfg->forward, lc);
		if(0 != ret) { goto end; }
		cfg->forward.use_log = cfg->use_log;
#endif
	}

	lc = tlv_part_cfg_find_lc_s(part, "REC");
	if(lc)
	{
		cfg->rec.is_online = cfg->is_online;
		ret |= tlv_kdext_rec_cfg_load_param(&cfg->rec, lc);
		if(0 != ret) { goto end; }
		cfg->rec.use_nnet3 = cfg->use_nnet3;
		cfg->rec.use_log   = cfg->use_log;
        if(cfg->language && strcmp(cfg->language,"en") == 0)
        {
            cfg->rec.is_en_asr = 1;
        }
	}

	if(cfg->use_log)
	{
		tlv_log("use_nnet3=%d\n", cfg->use_nnet3);
	}

    cfg->rec.is_en_asr = (strcmp(cfg->language,"en")==0)?1:0;

    //load vad params
    if(!cfg->cfg_path.empty())
    {
        VadInterface::VadParamLoader vad_param_loader;
        std::string vad_cfg_path = cfg->cfg_path;
        vad_cfg_path.replace(vad_cfg_path.rfind("cfg"),3,"vad_cfg.json");
        cfg->vad_params = new VadInterface::VadParams();
        if(!vad_param_loader.loadVadParams(vad_cfg_path, *cfg->vad_params))
        {
            delete cfg->vad_params;
            cfg->vad_params = NULL;
        }
    }

end:

	return ret;
}

int tlv_kaldi_dec_cfg_load_res(tlv_kaldi_dec_cfg_t* cfg, tlv_strfile_loader_t *sl)
{
	int ret=0;
	tlv_strfile_loader_t loader;

	if(cfg->use_log)
	{
		tlv_log("enter!\n");
	}

	if(!sl)
	{
		loader.hook = NULL;
		loader.vf   = tlv_strfile_load_file_v;
		sl = &loader;
	}

	ret |= tlv_kdext_feat_cfg_load_res(&(cfg->feat), sl);

#ifdef USE_NNET1
	ret |= tlv_kdext_forward_cfg_load_res(&cfg->forward, sl);
#endif

	ret |= tlv_kdext_rec_cfg_load_res(&cfg->rec, sl);

	if(cfg->use_log)
	{
		tlv_log("leave! ret=%d\n", ret);
	}

	return ret;
}

int tlv_kaldi_dec_cfg_clean(tlv_kaldi_dec_cfg_t* cfg)
{
	int ret=0;

	if(cfg->use_log)
	{
		tlv_log("enter!\n");
	}

	ret |= tlv_kdext_feat_cfg_clean(&(cfg->feat));

#ifdef USE_NNET1
	ret |= tlv_kdext_forward_cfg_clean(&cfg->forward);
#endif

	ret |= tlv_kdext_rec_cfg_clean(&cfg->rec);

	if(cfg->cf) { tlv_cfg_file_delete(cfg->cf); }

	if(cfg->use_log)
	{
		tlv_log("leave! ret=%d\n", ret);
	}

    delete cfg->vad_params;
    cfg->vad_params = NULL;

	return ret;
}


/**********************  interface  *********************/
static int tlv_kaldi_dec_cfg_bin_init(tlv_kaldi_dec_cfg_t* cfg, tlv_part_cfg_t* root, tlv_strfile_loader_t* sl)
{
	int ret = 0;

	ret |= tlv_kaldi_dec_cfg_init(cfg);
	ret |= tlv_kaldi_dec_cfg_load_param(cfg, root);
	ret |= tlv_kaldi_dec_cfg_load_res(cfg, sl);

	return ret;
}

tlv_kaldi_dec_cfg_t* tlv_kaldi_dec_cfg_new(const char* fn)
{
    SetVerboseLevel(-1);//disable info log,keep warning and error log
	int ret = -1;
	tlv_cfg_file_t* cf;
	tlv_kaldi_dec_cfg_t* cfg = NULL;
	tlv_strfile_loader_t sl;

	sl.hook = NULL;
	sl.vf   = tlv_strfile_load_file_v;

	cf = tlv_cfg_file_new_fn((char*)fn);
	if(cf == NULL) { goto end; }
	cfg = (tlv_kaldi_dec_cfg_t*)tlv_calloc(1, sizeof(tlv_kaldi_dec_cfg_t));
    cfg->cfg_path = std::string(fn);
	ret = tlv_kaldi_dec_cfg_bin_init(cfg, cf->main, &sl);
	cfg->cf = cf;
	if(0 != ret)
	{
		tlv_kaldi_dec_cfg_delete(cfg);
		cfg = NULL;
	}

end:

	return cfg;
}

int tlv_kaldi_dec_cfg_get_version(tlv_kaldi_dec_cfg_t* cfg, char** version) {
  if (cfg) {
    std::string tmp(cfg->version);
    auto pos = tmp.rfind("V");
    if (pos != std::string::npos && (pos + 1) < strlen(cfg->version)) {
      *version = cfg->version + pos + 1;
    } else {
      *version = NULL;
    }
    return 0;
  } else {
    return -1;
  }
}

int tlv_kaldi_dec_cfg_get_is_en(tlv_kaldi_dec_cfg_t* cfg)
{
    int ret = 0;
    if(cfg && cfg->language)
    {
        std::string language(cfg->language);
        if(language == "en")
        {
            ret = 1;
        }
    }
    return ret;
}

void tlv_kaldi_dec_cfg_delete(tlv_kaldi_dec_cfg_t* cfg)
{
	tlv_kaldi_dec_cfg_clean(cfg);
	tlv_free(cfg);
}
/*---------------------  end --------------------*/
