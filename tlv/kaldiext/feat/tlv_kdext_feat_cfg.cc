/*
 * tlv_kdext_feat_cfg.cc
 *
 *  Created on: Dec 27, 2018
 *      Author: jfyuan
 */
#include "tlv_kdext_feat_cfg.h"

int tlv_kdext_feat_cfg_init(tlv_kdext_feat_cfg_t* cfg)
{
	memset(cfg, 0, sizeof(tlv_kdext_feat_cfg_t));
	cfg->length_tolerance = 2;

	cfg->use_fbank = 1;
	cfg->vtln_warp = 1.0;

	cfg->channel   = 1;
	cfg->samp_freq = 16000;

	cfg->frame_shift_ms  = 10;
	cfg->frame_length_ms = 25;
	cfg->dither    = 1.0;

	cfg->dela_opts = new DeltaFeaturesOptions();

	/* for online */
	cfg->chunk_length_secs = 0.18;

	return 0;
}

int tlv_kdext_feat_cfg_load_param(tlv_kdext_feat_cfg_t* cfg, tlv_part_cfg_t* part)
{
	tlv_part_cfg_t *lc = part;
	tlv_string_t   *str;

	tlv_part_cfg_update_cfg_b(lc, cfg, is_online, str);
	tlv_part_cfg_update_cfg_b(lc, cfg, use_fbank, str);
	tlv_part_cfg_update_cfg_b(lc, cfg, use_mfcc, str);

	tlv_part_cfg_update_cfg_b(lc, cfg, use_pitch, str);
	tlv_part_cfg_update_cfg_i(lc, cfg, length_tolerance, str);

	tlv_part_cfg_update_cfg_b(lc, cfg, apply_cmvn, str);
	tlv_part_cfg_update_cfg_b(lc, cfg, use_norm_vars, str);

	tlv_part_cfg_update_cfg_str(lc, cfg, window_type, str);
	tlv_part_cfg_update_cfg_b(lc, cfg, use_energy, str);
	tlv_part_cfg_update_cfg_i(lc, cfg, dither, str);
	tlv_part_cfg_update_cfg_i(lc, cfg, num_mel_bins, str);

	tlv_part_cfg_update_cfg_f(lc, cfg, vtln_warp, str);
	tlv_part_cfg_update_cfg_i(lc, cfg, delta_order, str);
    tlv_part_cfg_update_cfg_i(lc, cfg, frame_shift_ms, str);
    tlv_part_cfg_update_cfg_i(lc, cfg, frame_length_ms, str);

	return 0;
}


int tlv_kdext_feat_cfg_load_res(tlv_kdext_feat_cfg_t* cfg, tlv_strfile_loader_t *sl)
{
	if(cfg->use_fbank)
	{
		//cfg->fbank_opts = (FbankOptions*)tlv_malloc(sizeof(FbankOptions));
		cfg->fbank_opts = new FbankOptions();
		if(cfg->window_type)
		{
			cfg->fbank_opts->frame_opts.window_type = cfg->window_type;
		}
		if(cfg->use_energy)
		{
			cfg->fbank_opts->use_energy = true;
		}
		cfg->fbank_opts->frame_opts.dither = 0.0;
		if(cfg->dither > 0)
		{
			cfg->fbank_opts->frame_opts.dither = cfg->dither;
		}
		if(cfg->num_mel_bins > 0)
		{
			cfg->fbank_opts->mel_opts.num_bins = cfg->num_mel_bins;
		}
        cfg->fbank_opts->frame_opts.frame_shift_ms = cfg->frame_shift_ms;
        cfg->fbank_opts->frame_opts.frame_length_ms = cfg->frame_length_ms;
        cfg->fbank_opts->frame_opts.samp_freq = cfg->samp_freq;
	}

	if(cfg->delta_order > 0)
	{
		cfg->dela_opts->order = cfg->delta_order;
	}

	/* for online */
	if(cfg->is_online)
	{
		cfg->chunk_length = int32(cfg->samp_freq * cfg->chunk_length_secs);
		if(0 == cfg->chunk_length) cfg->chunk_length = 1;

		cfg->feature_info = new OnlineNnet2FeaturePipelineInfo();
		if(cfg->use_fbank)
		{
			cfg->feature_info->feature_type = "fbank";
		}
		cfg->feature_info->fbank_opts = *(cfg->fbank_opts);

		if(cfg->use_pitch)
		{
			cfg->feature_info->add_pitch = true;
		}
		else
		{
			cfg->feature_info->add_pitch = false;
		}


		cfg->feature_info->use_ivectors = false;

        cfg->feature_info->pitch_opts.frame_length_ms = cfg->frame_length_ms;
        cfg->feature_info->pitch_opts.frame_shift_ms = cfg->frame_shift_ms;
	}/* if(cfg->is_online) */

	return 0;
}

int tlv_kdext_feat_cfg_clean(tlv_kdext_feat_cfg_t* cfg)
{
	if(cfg->fbank_opts) delete cfg->fbank_opts;
	if(cfg->dela_opts)  delete cfg->dela_opts;

	if(cfg->feature_info) delete cfg->feature_info;

	return 0;
}
