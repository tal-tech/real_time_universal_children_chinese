/*
 * tlv_kdext_feat.cc
 *
 *  Created on: Dec 27, 2018
 *      Author: jfyuan
 */
#include "tlv_kdext_feat.h"
#include "transform/cmvn.h"
#include "feat/pitch-functions.h"

static bool tlv_kdext_append_feats(const std::vector<Matrix<BaseFloat> > &in,int tolerance, Matrix<BaseFloat> *out);

tlv_kdext_feat_t* tlv_kdext_feat_new(const tlv_kdext_feat_cfg_t* cfg)
{
	tlv_kdext_feat_t* feat;

	feat = (tlv_kdext_feat_t*)tlv_malloc(sizeof(tlv_kdext_feat_t));
	feat->cfg = cfg;
	feat->errinfo = NULL;

	if(cfg->is_online)
	{
		feat->feature_pipeline = new OnlineNnet2FeaturePipeline(*(cfg->feature_info));
		feat->is_odd = 0;
	}/* if(cfg->is_online) */

	if(cfg->use_fbank)
	{
		feat->fbank = new Fbank(*(cfg->fbank_opts));
	}

	feat->fea = new Matrix<BaseFloat>;

	return feat;
}

int tlv_kdext_feat_delete(tlv_kdext_feat_t* f)
{
	if(f->cfg->is_online)
	{
		if(f->feature_pipeline) delete f->feature_pipeline;
	}

	delete f->fbank;
	delete f->fea;
	tlv_free(f);

	return 0;
}

int tlv_kdext_feat_reset(tlv_kdext_feat_t* f)
{
	if(f->cfg->is_online)
	{
		if(f->feature_pipeline) delete f->feature_pipeline;
		f->feature_pipeline = new OnlineNnet2FeaturePipeline(*(f->cfg->feature_info));

		f->is_odd = 0;
	}

	f->fea->Resize(0, 0);

	return 0;
}

int tlv_kdext_feat_feed(tlv_kdext_feat_t* f, char* data, int len,unsigned char is_end)
{
	int ret = -1;
	const tlv_kdext_feat_cfg_t* cfg = f->cfg;
	uint32 i=0, j=0;
	Matrix<BaseFloat> data_;
	Matrix<BaseFloat> *fea = f->fea;
	Matrix<BaseFloat> base_fea;
	Matrix<BaseFloat> pitch_fea;
	uint16 *data_ptr = reinterpret_cast<uint16*>(data);

	if(cfg->use_log)
	{
		tlv_log("enter! data=%p, len=%d\n", data, len);
	}

	 /* The matrix is arranged row per channel, column per sample. */
	if(cfg->is_online && len > 0)
	{
		if(f->is_odd)
		{
			data_.Resize(cfg->channel, (len+1)/2);
		}
		else
		{
			data_.Resize(cfg->channel, (len / 2));
		}

		if(f->is_odd)
		{
			int16 odd;
			char* p;

			p = (char*)&odd;
			p[0] = f->odd_char;
			p[1] = data[0];
			data_(0, 0) = odd;
			i = 1;
			//tlv_log("1: %d, short: %d\n", data[0], odd);

			data += 1;
			len  -= 1;
		}

		data_ptr = reinterpret_cast<uint16*>(data);
	    for(; i < data_.NumCols(); i++)
	    {
		   for(j=0; j < data_.NumRows(); j++)
		   {
			  int16 k = *data_ptr++;
			  data_(j, i) = k;
		    }
	     }

	    if(len % 2)
	    {
	    	f->is_odd = 1;
	    	f->odd_char = data[len-1];
	    	//tlv_log("0: %d\n", data[len-1]);
	    }
	    else
	    {
	    	f->is_odd = 0;
	    }

	}/* if(cfg->is_online) */
	else if(len > 0)
	{
		 data_.Resize(cfg->channel, len / 2);
		 for(i=0; i < data_.NumCols(); i++)
		 {
			 for(j=0; j < data_.NumRows(); j++)
			 {
				 int16 k = *data_ptr++;
				 data_(j, i) = k;
			 }
		 }
	}
	else
	{
		data_.Resize(cfg->channel, 1);
	}

	SubVector<BaseFloat> waveform(data_, 0);

	if(cfg->is_online)
	{
		if(0 == len && is_end)
		{
			f->feature_pipeline->InputFinished();
			ret = 0;
			goto end;
		}

		f->feature_pipeline->AcceptWaveform(cfg->samp_freq, waveform);
		if(is_end)
		{
			f->feature_pipeline->InputFinished();
		}

		ret = 0;
		goto end;
	}

	if(cfg->use_fbank)
	{
		try{
			f->fbank->ComputeFeatures(waveform, cfg->samp_freq, cfg->vtln_warp, f->fea);
		}
		catch(...) {
			if(f->errinfo)
			{
				tlv_errinfo_set(f->errinfo, 30001,"Failed to compute features!", 0);
			}
			tlv_log("Failed to compute features!\n");
			goto end;
		}
	}

	if(cfg->use_pitch)
	{
		PitchExtractionOptions pitch_opts;
		ProcessPitchOptions process_opts;
        pitch_opts.samp_freq = cfg->samp_freq;
        pitch_opts.frame_length_ms = cfg->frame_length_ms;
        pitch_opts.frame_shift_ms  = cfg->frame_shift_ms;

		try{
			ComputeAndProcessKaldiPitch(pitch_opts, process_opts,
		                                    waveform, &pitch_fea);
		}
		catch (...) {
			if(f->errinfo)
			{
				tlv_errinfo_set(f->errinfo, 30001, "Failed to compute pitch!", 0);
			}
			tlv_log("Failed to compute pitch!\n");
			goto end;
		}

		std::vector<Matrix<BaseFloat>> feats(2);
		base_fea = *fea;
		feats[0] = base_fea;
		feats[1] = pitch_fea;

		fea->Resize(0, 0);
		if( !tlv_kdext_append_feats(feats, cfg->length_tolerance, fea) )
		{
			if(f->errinfo)
			{
				tlv_errinfo_set(f->errinfo, 30001, "append feats failed!", 0);
			}
			tlv_log("append feats failed!\n");
			goto end;
		}
	}

	if(cfg->subtract_mean)
	{
		Vector<BaseFloat> mean(fea->NumCols());
		mean.AddRowSumMat(1.0, *(fea));
		mean.Scale(1.0/ fea->NumRows());
		for(i=0; i < fea->NumRows(); i++)
		{
			fea->Row(i).AddVec(-1.0, mean);
		}
	}


	/* apply cmvn */
	//bool is_init = false;
	if(cfg->apply_cmvn)
	{
		Matrix<double> stats;

		InitCmvnStats(fea->NumCols(), &stats);

		AccCmvnStats(*fea, NULL, &stats);
		ApplyCmvn(stats, false, fea);
	}

	/* add-deltas */
	if(cfg->delta_order > 0)
	{
		data_.Resize(fea->NumRows(), fea->NumCols());
		data_.CopyFromMat(*fea);
		ComputeDeltas(*(cfg->dela_opts), data_, fea);
	}
	fea->Write(std::cout, false);

	ret = 0;
end:

	if(cfg->use_log)
	{
		tlv_log("leave! ret = %d\n", ret);
	}

	return ret;
}

/***************************************/
/* returns true if successfully appened */
static bool tlv_kdext_append_feats(const std::vector<Matrix<BaseFloat> > &in,int tolerance, Matrix<BaseFloat> *out)
{
	/* Check the lengths */
	int32 min_len = in[0].NumRows(),
			max_len = in[0].NumRows(),
			tot_dim = in[0].NumCols();

	for(int32 i=1; i < in.size(); i++)
	{
		int32 len = in[i].NumRows(), dim = in[i].NumCols();

		tot_dim += dim;
		if(len < min_len) min_len = len;
		if(len > max_len) max_len = len;
	}

	if((max_len-min_len)>tolerance || 0 == min_len )
	{
		tlv_log("Length mismatch max_len[%d] vs. min_len[%d]\n", max_len, min_len);
		out->Resize(0, 0);

		return false;
	}

	out->Resize(min_len, tot_dim);
	int32 dim_offset = 0;
	for(int32 i=0; i < in.size(); i++)
	{
		int32 this_dim = in[i].NumCols();
		out->Range(0, min_len, dim_offset, this_dim).CopyFromMat(
				in[i].Range(0, min_len, 0, this_dim));
		dim_offset += this_dim;
	}

	return true;
}


/*================ end =================*/
