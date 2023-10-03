/*
 * tlv_kdext_forward.cc
 *
 *  Created on: Dec 11, 2018
 *      Author: jfyuan
 */
#ifdef USE_NNET1

#include "tlv_kdext_forward.h"

static int tlv_kdext_forward_load_res(tlv_kdext_forward_t *fw);

tlv_kdext_forward_t* tlv_kdext_forward_new(const tlv_kdext_forward_cfg_t* cfg)
{
	tlv_kdext_forward_t* fw;

	fw = (tlv_kdext_forward_t*)tlv_malloc(sizeof(tlv_kdext_forward_t));
	fw->cfg = cfg;
	fw->errinfo = NULL;
	fw->nnet_out_host = new Matrix<BaseFloat>();

	tlv_kdext_forward_load_res(fw);

	return fw;
}

int tlv_kdext_forward_feed(tlv_kdext_forward_t* fw, Matrix<BaseFloat> *fea)
{
	int ret = -1;
	const tlv_kdext_forward_cfg_t* cfg = fw->cfg;
	CuMatrix<BaseFloat> feats, feats_transf, nnet_out;

	if(cfg->use_log)
	{
		tlv_log("enter! fea=%p\n", fea);
	}

	/* push it to gpu, */
	feats = *fea;

	/* fwd-pass, feature transform */
	fw->nnet_transf->Feedforward(feats, &feats_transf);
	if(!KALDI_ISFINITE(feats_transf.Sum()))
	{
		// check there's no nan/inf,
		if(fw->errinfo)
		{
			tlv_errinfo_set(fw->errinfo, 30007, "NaN or inf found in transformed-features for", 0);
		}
		tlv_log("NaN or inf found in transformed-features for!\n");
		goto end;
	}

	/*  fwd-pass, nnet */
	fw->nnet->Feedforward(feats_transf, &nnet_out);
	if(!KALDI_ISFINITE(nnet_out.Sum()))
	{
		/* check there's no nan/inf */
		if(fw->errinfo)
		{
			tlv_errinfo_set(fw->errinfo, 30007, "NaN or inf found in transformed-features for", 0);
		}
		tlv_log("NaN or inf found in nn-output! \n");
		goto end;
	}

	/* convert posteriors to log-posteriors */
	if(cfg->apply_log)
	{
		if(!(nnet_out.Min() >= 0.0 && nnet_out.Max() <= 1.0))
		{
			tlv_log("Applying 'log()' to data which don't seem to be probabilities\n");
		}

		nnet_out.Add(1e-20);  /* avoid log(0) */
		nnet_out.ApplyLog();
	}

	/* subtract log-priors from log-posteriors or pre-softmax */
	if(fw->pdf_prior)
	{
		fw->pdf_prior->SubtractOnLogpost(&nnet_out);
	}

	/* download from GPU */
	if(fw->nnet_out_host)
	{
		*(fw->nnet_out_host) = Matrix<BaseFloat>(nnet_out);
		ret = 0;
	}
	else
	{
		if(fw->errinfo)
		{
			tlv_errinfo_set(fw->errinfo, 30001, "fw->nnet_out_host is null", 0);
		}
		tlv_log("fw->nnet_out_host is null!\n");
	}

end:
	if(cfg->use_log)
	{
		tlv_log("leave! ret=%d\n", ret);
	}

	return ret;
}

int tlv_kdext_forward_reset(tlv_kdext_forward_t* fw)
{
	if(fw->nnet_out_host)
	{
		delete fw->nnet_out_host;
		fw->nnet_out_host = new Matrix<BaseFloat>();
	}

	return 0;
}

int tlv_kdext_forward_delete(tlv_kdext_forward_t* fw)
{
	if(fw->pdf_prior)   delete fw->pdf_prior;
	if(fw->nnet_transf) delete fw->nnet_transf;
	if(fw->nnet)        delete fw->nnet;

	if(fw->nnet_out_host) delete fw->nnet_out_host;
	tlv_free(fw);

	return 0;
}

/*****************  static method  ******************/
static int tlv_kdext_forward_load_res(tlv_kdext_forward_t *fw)
{
//	int ret = 0;
	const tlv_kdext_forward_cfg_t *cfg = fw->cfg;
	PdfPriorOptions prior_opts;
	using namespace kaldi;
	using namespace kaldi::nnet1;

	prior_opts.class_frame_counts = cfg->class_frame_counts;

	if(cfg->feature_transform)
	{
		fw->nnet_transf = new Nnet();
		fw->nnet_transf->Read(cfg->feature_transform);
	}

	fw->nnet = new Nnet();
	fw->nnet->Read(cfg->model_filename);

	/*  optionally remove softmax */
	Component::ComponentType last_comp_type = fw->nnet->GetLastComponent().GetType();
	if(cfg->no_softmax)
	{
		if(last_comp_type == Component::kSoftmax ||
				last_comp_type == Component::kBlockSoftmax	)
		{
			fw->nnet->RemoveLastComponent();
		}
		else
		{

		}

	}

	// we will subtract log-priors later
	fw->pdf_prior = new PdfPrior(prior_opts);

	// disable dropout
	fw->nnet_transf->SetDropoutRate(0.0);
	fw->nnet->SetDropoutRate(0.0);

	return 0;
}
/*-------------------  end  ---------------------*/

#endif
