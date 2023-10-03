/*
 * tlv_rec_cfg.c
 *
 *  Created on: Dec 13, 2018
 *      Author: jfyuan
 */

#include "tlv_kdext_rec_cfg.h"
#include "tlv/kaldiext/utils/utility.h"

int tlv_kdext_rec_cfg_init(tlv_kdext_rec_cfg_t* cfg)
{
	memset(cfg, 0, sizeof(tlv_kdext_rec_cfg_t));

	cfg->frame_subsampling_factor = 1;

	cfg->config = new LatticeFasterDecoderConfig();
	cfg->nbest  = 1;
	cfg->heap   = tlv_heap_new(4096);
    cfg->is_en_asr = 0;

    cfg->hot_word_global_weight = 0;
    cfg->hot_word_nbest = 1;
    cfg->hot_word_nbest_reorder = 0;
    cfg->hot_word_debug = 0;
    cfg->exclude_hotwords_set = new std::set<std::string>();

    cfg->sil_code = 1;

	return 0;
}

int tlv_kdext_rec_cfg_load_param(tlv_kdext_rec_cfg_t* cfg, tlv_part_cfg_t* part)
{
	tlv_part_cfg_t *lc = part;
	tlv_string_t   *str;

	tlv_part_cfg_update_cfg_i(lc, cfg, min_active, str);
	tlv_part_cfg_update_cfg_i(lc, cfg, max_active, str);
	tlv_part_cfg_update_cfg_i(lc, cfg, max_mem, str);
	tlv_part_cfg_update_cfg_f(lc, cfg, beam, str);
	tlv_part_cfg_update_cfg_f(lc, cfg, lattice_beam, str);

	tlv_part_cfg_update_cfg_f(lc, cfg, acoustic_scale, str);
	tlv_part_cfg_update_cfg_f(lc, cfg, lm_scale, str);
	tlv_part_cfg_update_cfg_f(lc, cfg, word_penalty, str);

	tlv_part_cfg_update_cfg_str(lc, cfg, word_syms_filename, str);
	tlv_part_cfg_update_cfg_str(lc, cfg, model_in_filename, str);
	tlv_part_cfg_update_cfg_str(lc, cfg, fst_in_str, str);
	tlv_part_cfg_update_cfg_str(lc, cfg, phn_syms_filename, str);
	tlv_part_cfg_update_cfg_str(lc, cfg, dict_name, str);
	tlv_part_cfg_update_cfg_str(lc, cfg, ques_mark_filename, str);

	/* grammar fst */
	tlv_part_cfg_update_cfg_b(lc, cfg, use_grammar_fst, str);
	tlv_part_cfg_update_cfg_b(lc, cfg, use_fst_combine, str);
	tlv_part_cfg_update_cfg_str(lc, cfg, part_fst_filename, str);
	tlv_part_cfg_update_cfg_str(lc, cfg, part_wrdsyb_filename, str);

	/* nnet3 */
	tlv_part_cfg_update_cfg_b(lc, cfg, use_nnet3, str);
	tlv_part_cfg_update_cfg_i(lc, cfg, frame_subsampling_factor, str);
	tlv_part_cfg_update_cfg_i(lc, cfg, frames_per_chunk, str);

	/* output format */
	tlv_part_cfg_update_cfg_b(lc, cfg, use_timeinfo, str);
	tlv_part_cfg_update_cfg_b(lc, cfg, add_space, str);
	tlv_part_cfg_update_cfg_b(lc, cfg, add_punc, str);
	tlv_part_cfg_update_cfg_i(lc, cfg, nbest, str);

	tlv_part_cfg_update_cfg_b(lc, cfg, use_scripts, str);
	tlv_part_cfg_update_cfg_f(lc, cfg, inv_acoustic_scale, str);
	tlv_part_cfg_update_cfg_f(lc, cfg, lat_acoustic_scale, str);

	/* update decoder params */
	if(cfg->min_active > 0) cfg->config->min_active = cfg->min_active;
	if(cfg->max_active > 0) cfg->config->max_active = cfg->max_active;
	if(cfg->max_mem > 0)    cfg->config->det_opts.max_mem = cfg->max_mem;
	if(cfg->beam > 0)       cfg->config->beam    = cfg->beam;
	if(cfg->lattice_beam > 0)   cfg->config->lattice_beam = cfg->lattice_beam;

	if(cfg->use_nnet3)
	{
#ifndef USE_NNET1
		if(cfg->is_online)
		{
			cfg->decodable_opts_loop = new NnetSimpleLoopedComputationOptions;

			if(cfg->frame_subsampling_factor > 1) cfg->decodable_opts_loop->frame_subsampling_factor = cfg->frame_subsampling_factor;
			if(cfg->frames_per_chunk > 0)         cfg->decodable_opts_loop->frames_per_chunk = cfg->frames_per_chunk;
			if(cfg->acoustic_scale > 0.1)         cfg->decodable_opts_loop->acoustic_scale = cfg->acoustic_scale;
		}
		else
		{
			cfg->decodable_opts = new NnetSimpleComputationOptions;

			if(cfg->frame_subsampling_factor > 1) cfg->decodable_opts->frame_subsampling_factor = cfg->frame_subsampling_factor;
			if(cfg->frames_per_chunk > 0)         cfg->decodable_opts->frames_per_chunk = cfg->frames_per_chunk;
			if(cfg->acoustic_scale > 0.1)         cfg->decodable_opts->acoustic_scale  = cfg->acoustic_scale;

			cfg->decodable_opts->debug_computation = true;
		}

#endif
	}

    /* expect words weight*/
    tlv_part_cfg_update_cfg_f(lc, cfg, hot_word_global_weight, str);
    tlv_part_cfg_update_cfg_i(lc, cfg, hot_word_nbest, str);
    tlv_part_cfg_update_cfg_i(lc, cfg, hot_word_nbest_reorder, str);
    tlv_part_cfg_update_cfg_str(lc, cfg, exclude_hotwords_file, str);
    tlv_part_cfg_update_cfg_i(lc, cfg, hot_word_debug, str);


	return 0;
}

void tlv_parse_exclude_hot_words(tlv_kdext_rec_cfg_t* cfg);
int tlv_kdext_rec_cfg_load_res(tlv_kdext_rec_cfg_t* cfg, tlv_strfile_loader_t *sl)
{
	int ret=0;
	tlv_strfile_loader_t loader;

	if(!sl)
	{
		loader.hook = NULL;
		loader.vf   = tlv_strfile_load_file_v;
		sl = &loader;
	}


	if(cfg->word_syms_filename)
	{
		cfg->word_syms = fst::SymbolTable::ReadText(cfg->word_syms_filename);
		if(cfg->word_syms == NULL)
		{
			tlv_log("Could not read symbol table from file %s\n", cfg->word_syms_filename);
		}
	}

	if(cfg->model_in_filename)
	{
		cfg->trans_model = new TransitionModel;

		ReadKaldiObject(cfg->model_in_filename, cfg->trans_model);
	}

	if(cfg->fst_in_str)
	{
		/* Input FST is just one FST */
		cfg->decode_fst = fst::ReadFstKaldiGeneric(cfg->fst_in_str);

		if(cfg->decode_fst && cfg->use_fst_combine)
		{
			cfg->top_fst = dynamic_cast<ConstFst<StdArc>* >(cfg->decode_fst);
			if(!cfg->top_fst)
			{
				cfg->top_fst = new ConstFst<StdArc>(*cfg->decode_fst);
				delete cfg->decode_fst;
				cfg->decode_fst = NULL;
			}
		}/* if(cfg->decode_fst && cfg->use_fst_combine) */

	}

	if(cfg->phn_syms_filename)
	{
		cfg->phn_syms = fst::SymbolTable::ReadText(cfg->phn_syms_filename);
        if(cfg->phn_syms == NULL)
		{
			tlv_log("Could not read symbol table from file %s\n", cfg->phn_syms_filename);
		}
        else
        {
            cfg->sil_code = cfg->phn_syms->Find("sil");
        }
	}

	/* read dict */
	if(cfg->dict_name)
	{
		cfg->label = tlv_label_new(5007);
		cfg->dict = tlv_dict_new(cfg->label, 0);
		ret = tlv_strfile_load_file(cfg->dict, (tlv_strfile_load_handler_t)tlv_dict_load, cfg->dict_name);
		if(0 != ret) { goto end; }
	}

	/* 读取问号规则匹配词 */
	if(cfg->ques_mark_filename)
	{
		bool binary_in;
		Input ki(cfg->ques_mark_filename, &binary_in);
		std::istream &istream = ki.Stream();
		std::string line;
		char *word;

		cfg->arr_ques_mark = tlv_array_new(cfg->heap, 1024, sizeof(char*));
		while(std::getline(istream, line))
		{
			word = (char*)tlv_heap_malloc(cfg->heap, line.length()+1);
			memset(word, 0, line.length()+1);
			memcpy(word, line.c_str(), line.length());
			//tlv_array_push2(cfg->arr_ques_mark, &(buf->data));
			*((char**)tlv_array_push(cfg->arr_ques_mark)) = word;
		}

	}

    if(cfg->exclude_hotwords_file)
    {
        tlv_parse_exclude_hot_words(cfg);
    }

end:

	return ret;
}

int tlv_kdext_rec_cfg_clean(tlv_kdext_rec_cfg_t* cfg)
{
	if(cfg->phn_syms) delete cfg->phn_syms;
	if(cfg->word_syms) delete cfg->word_syms;
	if(cfg->trans_model) delete cfg->trans_model;
	if(cfg->config) delete cfg->config;
	
	/* for grammar fst */
	if(cfg->use_grammar_fst)
	{
		if(cfg->use_fst_combine && NULL == cfg->decode_fst && cfg->top_fst)
		{
			delete cfg->top_fst;
		}

	}/* if(cfg->use_grammar_fst) */

	if(cfg->decode_fst) delete cfg->decode_fst;
	if(cfg->use_nnet3)
	{
#ifndef USE_NNET1
		if(cfg->decodable_opts) delete cfg->decodable_opts;
		if(cfg->decodable_opts_loop) delete cfg->decodable_opts_loop;
#endif
	}

	if(cfg->dict) tlv_dict_delete(cfg->dict);
	if(cfg->label) tlv_label_delete(cfg->label);

	if(cfg->arr_ques_mark) tlv_array_delete(cfg->arr_ques_mark);
	if(cfg->heap) tlv_heap_delete(cfg->heap);
    if(cfg->exclude_hotwords_set)
    {
        delete cfg->exclude_hotwords_set;
        cfg->exclude_hotwords_set = NULL;
    }

	return 0;
}

void tlv_parse_exclude_hot_words(tlv_kdext_rec_cfg_t* cfg)
{
    if(cfg==NULL || cfg->exclude_hotwords_file==NULL)
    {
        tlv_log("tlv_parse_exclude_hot_words:file not set");
        return;
    }
    utils::ParseExcludeHotWords(std::string(cfg->exclude_hotwords_file), *(cfg->exclude_hotwords_set));
}
