/*
 * tlv_kdext_forward_cfg.cc
 *
 *  Created on: Dec 11, 2018
 *      Author: jfyuan
 */
#ifdef USE_NNET1

#include "tlv_kdext_forward_cfg.h"

int tlv_kdext_forward_cfg_init(tlv_kdext_forward_cfg_t* cfg)
{
	memset(cfg, 0, sizeof(*cfg));

	return 0;
}

int tlv_kdext_forward_cfg_load_param(tlv_kdext_forward_cfg_t* cfg, tlv_part_cfg_t* part)
{
	tlv_part_cfg_t *lc = part;
	tlv_string_t   *str;

	tlv_part_cfg_update_cfg_b(lc, cfg, no_softmax, str);
	tlv_part_cfg_update_cfg_b(lc, cfg, apply_log, str);

	tlv_part_cfg_update_cfg_f(lc, cfg, prior_scale, str);
	tlv_part_cfg_update_cfg_f(lc, cfg, prior_floor, str);
	tlv_part_cfg_update_cfg_str(lc, cfg, class_frame_counts, str);

	tlv_part_cfg_update_cfg_str(lc, cfg, feature_transform, str);
	tlv_part_cfg_update_cfg_str(lc, cfg, model_filename, str);

	return 0;
}

int tlv_kdext_forward_cfg_load_res(tlv_kdext_forward_cfg_t *cfg, tlv_strfile_loader_t *sl)
{
//	int ret = 0;
//	PdfPriorOptions prior_opts;
//	using namespace kaldi;
//	using namespace kaldi::nnet1;
//
//	prior_opts.class_frame_counts = cfg->class_frame_counts;
//
//	if(cfg->feature_transform)
//	{
//		cfg->nnet_transf = new Nnet();
//		cfg->nnet_transf->Read(cfg->feature_transform);
//	}
//
//	cfg->nnet = new Nnet();
//	cfg->nnet->Read(cfg->model_filename);
//
//	/*  optionally remove softmax */
//	Component::ComponentType last_comp_type = cfg->nnet->GetLastComponent().GetType();
//	if(cfg->no_softmax)
//	{
//		if(last_comp_type == Component::kSoftmax ||
//				last_comp_type == Component::kBlockSoftmax	)
//		{
//			cfg->nnet->RemoveLastComponent();
//		}
//		else
//		{
//
//		}
//
//	}

	// avoid some bad option combinations
	if (cfg->apply_log && cfg->no_softmax) {
		tlv_log("Cannot use both --apply-log=true --no-softmax=true, use only one of the two!\n");
	}

//	// we will subtract log-priors later
//	cfg->pdf_prior = new PdfPrior(prior_opts);
//
//	// disable dropout
//	cfg->nnet_transf->SetDropoutRate(0.0);
//	cfg->nnet->SetDropoutRate(0.0);

	return 0;
}

int tlv_kdext_forward_cfg_clean(tlv_kdext_forward_cfg_t* cfg)
{
	if(cfg->pdf_prior) delete cfg->pdf_prior;
	if(cfg->nnet_transf) delete cfg->nnet_transf;
	if(cfg->nnet) delete cfg->nnet;

	return 0;
}

#endif
