/*
 * tlv_kdext_rec.cc
 *
 *  Created on: Dec 17, 2018
 *      Author: jfyuan
 */
#include "tlv_kdext_rec.h"
#include "decoder/decodable-matrix.h"
#include "lat/lattice-functions.h"
#include "tlv/kaldiext/struct/tlv_kaldi_dec_extra_params.h"

//declare
int tlv_kdext_rec_get_lat(tlv_kdext_rec_t* r);
static int tlv_kdext_rec_get_wrdtimeinfo(tlv_kdext_rec_t* r,std::vector<int32>& ali, std::vector<int32>& words);
static string tlv_kdext_rec_add_endques(tlv_kdext_rec_t* r, const std::vector<int32> &words);
const char* tlv_kdext_rec_get_bestrslt_inner(tlv_kdext_rec_t* r, bool need_timeinfo, std::vector<int32>& ali_out);
void tlv_kdext_rec_get_sentence(tlv_kdext_rec_t* r, const std::vector<int32>& words, bool is_sentence_end, std::string& sentence_out);

//not using now
tlv_kdext_rec_t* tlv_kdext_rec_new(const tlv_kdext_rec_cfg_t* cfg)
{
	tlv_kdext_rec_t* r=0;

	r = (tlv_kdext_rec_t*)tlv_malloc(sizeof(tlv_kdext_rec_t));
	memset(r, 0, sizeof(tlv_kdext_rec_t));
	r->cfg = cfg;
	r->heap = tlv_heap_new(4096);
	r->errinfo = NULL;

	if(cfg->use_nnet3)
	{
#ifndef USE_NNET1
		r->am_nnet = new AmNnetSimple;
		ReadKaldiObject(cfg->model_in_filename, r->am_nnet);
		SetBatchnormTestMode(true, &(r->am_nnet->GetNnet()));
		SetDropoutTestMode(true, &(r->am_nnet->GetNnet()));
		CollapseModel(CollapseModelConfig(), &(r->am_nnet->GetNnet()));

		r->compiler = new CachingOptimizingCompiler(r->am_nnet->GetNnet(), cfg->decodable_opts->optimize_config);

#endif
	}

	r->decoder = new LatticeFasterDecoder(*(cfg->decode_fst), *(cfg->config) );

	r->onebest_rslt = new std::string("");
	r->lat = new Lattice;
	r->clat = new CompactLattice;

	r->frame_count = 0;
	r->like = 0.0;
    r->onebest_arr = NULL;

    r->process_lat = new Lattice;
    r->process_clat = new CompactLattice;
    r->nbest_rslt = new std::vector<std::string>();
    r->last_nbest_frame_count = 0;

	return r;
}

tlv_kdext_rec_t* tlv_kdext_rec_online_new(const tlv_kdext_rec_cfg_t* cfg, OnlineNnet2FeaturePipeline *fea_pipline)
{
	tlv_kdext_rec_t* r=0;

	if(cfg->use_log)
	{
		tlv_log("tlv_kdext_rec_online_new() enter! cfg=%p, fea_pipline=%p\n", cfg, fea_pipline);
	}

	r = (tlv_kdext_rec_t*)tlv_malloc(sizeof(tlv_kdext_rec_t));
	memset(r, 0, sizeof(tlv_kdext_rec_t));
	r->cfg = cfg;
	r->heap = tlv_heap_new(4096);
	r->errinfo = NULL;
	r->fea_pipline = fea_pipline;

	if(cfg->use_nnet3)
	{
#ifndef USE_NNET1
		bool binary;
		TransitionModel trans_model;
		r->am_nnet = new AmNnetSimple;
		Input ki(cfg->model_in_filename, &binary);
		trans_model.Read(ki.Stream(), binary);
		r->am_nnet->Read(ki.Stream(), binary);
		SetBatchnormTestMode(true, &(r->am_nnet->GetNnet()));
		SetDropoutTestMode(true, &(r->am_nnet->GetNnet()));
		CollapseModel(CollapseModelConfig(), &(r->am_nnet->GetNnet()));

		r->decodable_info = new DecodableNnetSimpleLoopedInfo(*(cfg->decodable_opts_loop), r->am_nnet);
		if(r->cfg->use_grammar_fst)
		{
			// nothing to do
		}
		else
		{
			r->decoder_online = new SingleUtteranceNnet3Decoder(*(cfg->config),
									*(cfg->trans_model),
									*(r->decodable_info),
									*(cfg->decode_fst),
									fea_pipline);
		}
		r->frames_decoded = 0;
#endif
	}

	r->onebest_rslt = new std::string("");
	r->lat = new Lattice;
	r->clat = new CompactLattice;

	r->frame_count = 0;
	r->like = 0.0;

    r->process_lat = new Lattice;
    r->process_clat = new CompactLattice;
    r->nbest_rslt = new std::vector<std::string>();
    r->last_nbest_frame_count = 0;

	return r;
}

//not using now
int tlv_kdext_rec_delete(tlv_kdext_rec_t* r)
{
	if(r->cfg->use_nnet3)
	{
#ifndef USE_NNET1
		if(r->am_nnet)  delete r->am_nnet;
		if(r->compiler) delete r->compiler;

		if(r->decodable_info) delete r->decodable_info;
		if(r->decoder_online) delete r->decoder_online;
    if(r->grammar_decoder_online) {
      delete r->grammar_decoder_online;
      if(r->cfg->is_online) {
        if(r->part_fst) delete r->part_fst;
          if(r->grammar_fst) delete r->grammar_fst;
          if(r->word_syms) delete r->word_syms;
        }
      }
		r->frames_decoded = 0;
#endif
	}

	if(r->decoder) delete r->decoder;
	if(r->onebest_rslt) delete r->onebest_rslt;
	if(r->lat) delete r->lat;
	if(r->clat) delete r->clat;
    if(r->process_lat) delete r->process_lat;
    if(r->process_clat) delete r->process_clat;
    if(r->nbest_rslt) delete r->nbest_rslt;
	if(r->heap) tlv_heap_delete(r->heap);
	tlv_free(r);

	return 0;
}

//not using now
int tlv_kdext_rec_reset(tlv_kdext_rec_t* r)
{
	tlv_heap_reset(r->heap);
	r->onebest_rslt->clear();
	r->frame_count = 0;
    r->last_nbest_frame_count = 0;

//	if(r->cfg->use_nnet3)
//	{
//		if(r->am_nnet)  delete r->am_nnet;
//		if(r->compiler) delete r->compiler;
//#ifndef USE_NNET1
//		r->am_nnet = new AmNnetSimple;
//		ReadKaldiObject(r->cfg->model_in_filename, r->am_nnet);
//		SetBatchnormTestMode(true, &(r->am_nnet->GetNnet()));
//		SetDropoutTestMode(true, &(r->am_nnet->GetNnet()));
//		CollapseModel(CollapseModelConfig(), &(r->am_nnet->GetNnet()));
//
//		r->compiler = new CachingOptimizingCompiler(r->am_nnet->GetNnet(), r->cfg->decodable_opts->optimize_config);
//
//#endif
//	}


	return 0;
}

const char* tlv_kdext_rec_online_get_bestrslt(tlv_kdext_rec_t* r, bool need_timeinfo, std::vector<int32>& alignment)
{
    return tlv_kdext_rec_get_bestrslt_inner(r, need_timeinfo, alignment);
}

int tlv_kdext_rec_online_reset(tlv_kdext_rec_t* r, OnlineNnet2FeaturePipeline *fea_pipline)
{
	if(r->cfg->use_nnet3)
	{
#ifndef USE_NNET1
		if(r->cfg->use_grammar_fst) {
          // TODO: reset
        } else{
	      if(r->decoder_online) delete r->decoder_online;
			r->decoder_online = new SingleUtteranceNnet3Decoder(*(r->cfg->config),
				                *(r->cfg->trans_model),
				                *(r->decodable_info),
				                *(r->cfg->decode_fst),
				                fea_pipline);
		}
#endif
	}

	tlv_heap_reset(r->heap);
	r->onebest_rslt->clear();
	r->frame_count = 0;
	r->like = 0.0;
	r->frames_decoded = 0;
    r->onebest_arr = NULL;
    r->last_nbest_frame_count = 0;

	return 0;
}

int tlv_kdext_rec_online_clear(tlv_kdext_rec_t* r) {
  if(r->cfg->use_nnet3) {
#ifndef USE_NNET1
    if(r->cfg->use_grammar_fst) {
          if(r->grammar_decoder_online) {
            delete r->grammar_decoder_online;
          }
          r->grammar_decoder_online = new SingleUtteranceNnet3GrammarDecoder(*(r->cfg->config),
            *(r->cfg->trans_model), *(r->decodable_info), *(r->grammar_fst), r->fea_pipline);
     } else {
        if(r->decoder_online) delete r->decoder_online;
          r->decoder_online = new SingleUtteranceNnet3Decoder(*(r->cfg->config),
                        *(r->cfg->trans_model),
                        *(r->decodable_info),
                        *(r->cfg->decode_fst),
                        r->fea_pipline);
     }
#endif
  }
  tlv_heap_reset(r->heap);
  r->onebest_rslt->clear();
  r->frame_count = 0;
  r->like = 0.0;
  r->frames_decoded = 0;
  r->onebest_arr = NULL;
  r->last_nbest_frame_count = 0;
  return 0;
}

//not using now
int tlv_kdext_rec_feed(tlv_kdext_rec_t* r, Matrix<BaseFloat> *nnet_forward, tlv_extra_param_t* extra_param)
{
	int ret = -1;
	LatticeFasterDecoder* decoder = r->decoder;
	Matrix<BaseFloat> loglikes = *nnet_forward;

	if(r->cfg->use_log)
	{
		tlv_log("enter! nnet_forward=%p\n", nnet_forward);
	}

	if(r->cfg->use_nnet3)
	{
#ifndef USE_NNET1
		r->is_end = 1;
		DecodableAmNnetSimple decodable(*(r->cfg->decodable_opts),
				                        *(r->cfg->trans_model),
				                        *(r->am_nnet),
				                        loglikes, NULL, NULL, 0, r->compiler);
        if(extra_param)
        {
            decoder->SetExpectWords(extra_param->hot_word_ids, extra_param->hot_word_global_weight);
        }

		if (!decoder->Decode(&decodable)) {
			if(r->errinfo)
			{
				tlv_errinfo_set(r->errinfo, 30006, "Failed to decode file", 0);
			}
			tlv_log("Failed to decode file\n");
			goto end;
		}

#endif
	}
	else
	{
		DecodableMatrixScaledMapped decodable(*(r->cfg->trans_model), loglikes, r->cfg->acoustic_scale);
		if (!decoder->Decode(&decodable)) {
			if(r->errinfo)
			{
				tlv_errinfo_set(r->errinfo, 30006, "Failed to decode file", 0);
			}
			tlv_log("Failed to decode file\n");
			goto end;
		}

	}

	r->frame_count = loglikes.NumRows();

	if (!decoder->ReachedFinal()) {
		if(r->errinfo)
		{
			tlv_errinfo_set(r->errinfo, 30006, "since no final-state reached", 0);
		}
		tlv_log("since no final-state reached\n");
		goto end;
	}
	ret = 0;

end:
	if(r->cfg->use_log)
	{
		tlv_log("leave! ret=%d\n", ret);
	}

	return ret;
}

//not using now
const char* tlv_kdext_rec_get_bestrslt(tlv_kdext_rec_t* r, bool need_timeinfo, std::vector<int32>& alignment)
{
    return tlv_kdext_rec_get_bestrslt_inner(r, need_timeinfo, alignment);
}

int tlv_kdext_rec_online_feed(tlv_kdext_rec_t* r, tlv_extra_param_t* extra_param, unsigned char is_end) {
  if(extra_param) {
    if(r->cfg->use_grammar_fst) {
      r->grammar_decoder_online->SetExpectWords(extra_param->hot_word_ids,
                                          extra_param->hot_word_global_weight);
    } else {
      r->decoder_online->SetExpectWords(extra_param->hot_word_ids,
                                          extra_param->hot_word_global_weight);
    }
  }
  r->is_end = is_end;
  if(r->cfg->use_grammar_fst) {
    r->grammar_decoder_online->AdvanceDecoding();
    if(is_end) {
      r->grammar_decoder_online->FinalizeDecoding();
    }
  } else {
    r->decoder_online->AdvanceDecoding();
    if(is_end) {
      r->decoder_online->FinalizeDecoding();
    }
  }
  return 0;
}

/**
 * @brief 解码过程中获取当前最佳识别内容
 */
const char* tlv_kdext_rec_online_get_currslt(tlv_kdext_rec_t* r, bool need_timeinfo, bool& is_changed, std::vector<int32>& ali_out)
{
	using fst::VectorFst;
	int ret = -1;
	fst::SymbolTable *word_syms = r->cfg->word_syms;
	VectorFst<LatticeArc> decoded;
	std::vector<int32> alignment;
	std::vector<int32> words;
	LatticeWeight weight;
    ali_out.clear();

	//tlv_log("frames_decoded: %d\n", r->frames_decoded);
	if(r->cfg->use_grammar_fst)
	{
		if(r->grammar_decoder_online->NumFramesDecoded() <= r->frames_decoded)
		{
			is_changed = false;	
			return r->onebest_rslt->c_str();
		}
		r->frames_decoded = r->grammar_decoder_online->NumFramesDecoded();
		is_changed = true;
		
		r->grammar_decoder_online->GetBestPath(false, &decoded);
	}
	else
	{

		if(r->decoder_online->NumFramesDecoded() <= r->frames_decoded)
		{
			is_changed = false;
			return r->onebest_rslt->c_str();
		}
		r->frames_decoded = r->decoder_online->NumFramesDecoded();
		is_changed = true;

		r->decoder_online->GetBestPath(false, &decoded);
	}

	if(!GetLinearSymbolSequence(decoded, &alignment, &words, &weight))
	{
		if(r->cfg->use_log)
		{
			tlv_log("get cur rslt is failed!\n");
		}
		return NULL;
	}
    ali_out = alignment;

    if(!word_syms)
    {
        tlv_log("no word symbole!\n");
        goto end;
    }

    tlv_kdext_rec_get_sentence(r, words, false, *(r->onebest_rslt));

    if(need_timeinfo)
    {
        tlv_kdext_rec_get_wrdtimeinfo(r, alignment, words);
    }

	if(r->cfg->frame_subsampling_factor > 0)
	{
		if(r->cfg->use_grammar_fst)
		{
			r->frame_count = r->grammar_decoder_online->NumFramesDecoded() * r->cfg->frame_subsampling_factor;
		}
		else
		{
			r->frame_count = r->decoder_online->NumFramesDecoded() * r->cfg->frame_subsampling_factor;
		}
	}
	else
	{
		if(r->cfg->use_grammar_fst)
		{
			r->frame_count = r->grammar_decoder_online->NumFramesDecoded();
		}
		else
		{
			r->frame_count = r->decoder_online->NumFramesDecoded();
		}

	}

	//*rslt = const_cast<char*>(r->onebest_rslt->c_str());
	r->like = -(weight.Value1() + weight.Value2());
	ret = 0;

end:

    if(0 != ret)
    {
        return NULL;
    }
    else
    {
        return r->onebest_rslt->c_str();
    }
}

void tlv_kdext_rec_get_sentence(tlv_kdext_rec_t* r,
                                const std::vector<int32>& words,
                                bool is_sentence_end,
                                std::string& sentence_out)
{
    sentence_out.clear();
    if(r && r->cfg->word_syms)
    {
        fst::SymbolTable* word_syms = r->cfg->word_syms;

		if(r->cfg->use_fst_combine && r->word_syms)
		{
			word_syms = r->word_syms;
		}

        for (size_t i = 0; i < words.size(); i++)
        {
            std::string s = word_syms->Find(words[i]);
            if(s == "")
            {
                tlv_log("Word-id %d not in symbol table.\n", words[i]);
                continue;
            }
            else if(s == "<SPOKEN_NOISE>")
            {
                continue;
            }
            else if(s == "<eps>")
            {
                continue;
            }

            sentence_out += s;
            if( r->cfg->add_space && (i < words.size()-1) )
            {
                sentence_out.push_back(' ');
            }

            /**
             * @brief 在末尾添加标点符号
             * @brief 基础能力v1.6开始，服务中增加了加标点服务，此处不用了 2020.02.05
             */
            if(r->cfg->add_punc && is_sentence_end && r->cfg->is_en_asr==1)
            {
                if(i == words.size()-1)
                {
                    if( (sentence_out.find("是不是") != std::string::npos)
                        or ( sentence_out.find("可不可以") != std::string::npos )
                        or ( sentence_out.find("要不要") != std::string::npos )
                        or ( sentence_out.find("为什么") != std::string::npos )
                        or ( sentence_out.find("怎么可以") != std::string::npos )
                        or ( sentence_out.find("是否") != std::string::npos )
                        or ( sentence_out.find("能否") != std::string::npos ) )
                    {
                        sentence_out += "？";
                    }
                    else
                    {
                        sentence_out += tlv_kdext_rec_add_endques(r, words);
                    }
                }
            }
        }

        if(r->cfg->is_en_asr == 1)
        {
            std::transform(sentence_out.begin(),sentence_out.end(),sentence_out.begin(),::tolower);
        }
    }
    else
    {
        tlv_log("no word symbole!\n");
    }
}

const char* tlv_kdext_rec_get_bestrslt_inner(tlv_kdext_rec_t* r, bool need_timeinfo, std::vector<int32>& ali_out)
{
	using fst::VectorFst;
	int ret = -1;
	VectorFst<LatticeArc> decoded;
	std::vector<int32> alignment;
	std::vector<int32> words;
	LatticeWeight weight;
    ali_out.clear();

	//fst::ScaleLattice(fst::LatticeScale(r->cfg->lm_scale, r->cfg->acoustic_scale), &decoded);
	//AddWordInsPenToCompactLattice(r->cfg->word_penalty, &decoded);
	if(r->cfg->use_scripts)
	{
		BaseFloat acoustic_scale = 1.0;
		BaseFloat lm_scale = 1.0;
		BaseFloat acoustic2lm_scale = 0.0;
		BaseFloat lm2acoustic_scale = 0.0;
		CompactLattice best_path;
		std::vector<std::vector<double> > scale(2);

		scale[0].resize(2);
		scale[1].resize(2);


		ret = tlv_kdext_rec_get_lat(r);
		if(0 != ret && r->clat != NULL) { goto end; }

		//CompactLatticeWriter compact_lattice_writer("ark,t:test.lat");
		//compact_lattice_writer.Write("raw:", *r->clat);

		/* lattice-scale --inv-acoustic-scale=17 */
		if(r->cfg->inv_acoustic_scale != 1.0 )
		{
			acoustic_scale = 1.0 / r->cfg->inv_acoustic_scale;
		}
		if(r->cfg->lat_acoustic_scale > 1.0)
		{
			acoustic_scale = acoustic_scale * r->cfg->lat_acoustic_scale;
		}
		scale[0][0] = lm_scale;
		scale[0][1] = acoustic2lm_scale;
		scale[1][0] = lm2acoustic_scale;
		scale[1][1] = acoustic_scale;
		ScaleLattice(scale, r->clat);
		AddWordInsPenToCompactLattice(r->cfg->word_penalty, r->clat);
		//compact_lattice_writer.Write("scale:", *r->clat);

		/* lattice-1best */
//		fst::ScaleLattice(fst::LatticeScale(lm_scale, acoustic_scale), r->clat);
//		if(r->cfg->word_penalty > 0.0)
//		{
//			AddWordInsPenToCompactLattice(r->cfg->word_penalty, r->clat);
//		}

		CompactLatticeShortestPath(*r->clat, &best_path);
		if(best_path.Start() == fst::kNoStateId)
		{
			tlv_log("Possibly empty lattice for utterance-id(no output)\n");
			goto end;
		}
		else
		{
			fst::ScaleLattice(fst::LatticeScale(1.0/lm_scale, 1.0/acoustic_scale), &best_path);
			if(r->cfg->word_penalty > 0.0)
			{
				AddWordInsPenToCompactLattice(r->cfg->word_penalty, &best_path);
			}

			ConvertLattice(best_path, &decoded);
		}

	}
	else
	{
		if(r->cfg->is_online)
		{
			if(r->cfg->use_grammar_fst)
			{
				r->grammar_decoder_online->GetBestPath(true, &decoded);
			}
			else
			{
				r->decoder_online->GetBestPath(true, &decoded);
			}
		}
		else
		{
			if (!r->decoder->GetBestPath(&decoded))
			{
				// Shouldn't really reach this point as already checked success.
				tlv_log("Failed to get traceback for utterance\n");
				goto end;
			}
		}
	}


	GetLinearSymbolSequence(decoded, &alignment, &words, &weight);
    ali_out = alignment;

    if(!r->cfg->word_syms)
    {
        tlv_log("no word symbole!\n");
        goto end;
    }

    tlv_kdext_rec_get_sentence(r, words, true, *(r->onebest_rslt));

    if(need_timeinfo)
	{
		tlv_kdext_rec_get_wrdtimeinfo(r, alignment, words);
	}

	//*rslt = const_cast<char*>(r->onebest_rslt->c_str());
	r->like = -(weight.Value1() + weight.Value2());
	ret = 0;

end:

	if(0 != ret)
	{
		return NULL;
	}
	else
	{
		return r->onebest_rslt->c_str();
	}
}

int tlv_kdext_rec_get_lat(tlv_kdext_rec_t* r)
{
	int ret = -1;
	float acoustic_scale = r->cfg->acoustic_scale;

	if(r->cfg->is_online)
	{
//		r->decoder_online->GetLattice(true, r->clat);
//
//		ret = 0;
//		goto end;
		if(r->cfg->frame_subsampling_factor > 0)
		{
			if(r->cfg->use_grammar_fst)
			{
				r->frame_count = r->grammar_decoder_online->NumFramesDecoded() * r->cfg->frame_subsampling_factor;
			}
			else
			{
				r->frame_count = r->decoder_online->NumFramesDecoded() * r->cfg->frame_subsampling_factor;
			}
		}
		else
		{
			if(r->cfg->use_grammar_fst)
			{
				r->frame_count = r->grammar_decoder_online->NumFramesDecoded();
			}
			else
			{
				r->frame_count = r->decoder_online->NumFramesDecoded();
			}
		}

		if(r->cfg->use_grammar_fst)
		{
			r->grammar_decoder_online->GetRawLattice(true, r->lat);
		}
		else
		{
			r->decoder_online->GetRawLattice(true, r->lat);
		}
	}
	else
	{
		r->decoder->GetRawLattice(r->lat);
	}

	if(0 == r->lat->NumStates())
	{
		tlv_log("Unexpected problem getting lattice for utterance.\n");
		goto end;
	}

	fst::Connect(r->lat);
	if(r->cfg->config->determinize_lattice)
	{
		if( !DeterminizeLatticePhonePrunedWrapper(
				*(r->cfg->trans_model),
				r->lat,
				r->cfg->config->lattice_beam,
				r->clat,
				r->cfg->config->det_opts) )
		{
			tlv_log("Determinization finished earlier than the beam for utterance.\n");
		}

		if (acoustic_scale != 0.0)
		{
			fst::ScaleLattice(fst::AcousticLatticeScale(1.0/acoustic_scale), r->clat);
		}
	}
	else
	{
		// We'll write the lattice without acoustic scaling.
		if (acoustic_scale != 0.0)
		{
			fst::ScaleLattice(fst::AcousticLatticeScale(1.0/acoustic_scale), r->lat);
		}
	}
	ret = 0;

end:

	return ret;
}

void scale_clat_before_get_result(tlv_kdext_rec_t* r, CompactLattice* p_clat);
void tlv_kdext_rec_get_clat_inprocess(tlv_kdext_rec_t* r)
{
    using fst::VectorFst;

    float acoustic_scale = r->cfg->acoustic_scale;

	if(r->cfg->use_grammar_fst)
	{
		r->grammar_decoder_online->GetRawLattice(true, r->process_lat);
	}
	else
	{
		r->decoder_online->GetRawLattice(true, r->process_lat);
	}

    if(0 == r->process_lat->NumStates())
    {
        tlv_log("Unexpected problem getting lattice for utterance.\n");
        return;
    }

    if( !DeterminizeLatticePhonePrunedWrapper(
            *(r->cfg->trans_model),
            r->process_lat,
            r->cfg->config->lattice_beam,
            r->process_clat,
            r->cfg->config->det_opts) )
    {
        tlv_log("Determinization finished earlier than the beam for utterance.\n");
        return;
    }

    if(r->process_lat==NULL || r->process_clat==NULL)
    {
        return;
    }

    if (acoustic_scale != 0.0)
    {
        fst::ScaleLattice(fst::AcousticLatticeScale(1.0/acoustic_scale), r->process_clat);
    }
}

bool get_best_rslt_from_clat(tlv_kdext_rec_t* r, CompactLattice* clat, bool is_end, std::string& result_out);
std::vector<std::string>* tlv_kdext_rec_get_online_nbestrslt(tlv_kdext_rec_t* r, int nbest)
{
    using fst::VectorFst;
    if(!r)
    {
        tlv_log("tlv_kdext_rec_get_online_nbestrslt:r is null!\n");
        return NULL;
    }

    tlv_kdext_rec_get_clat_inprocess(r);

    if(r->process_clat==NULL || 0==r->process_clat->NumStates())
    {
        tlv_log("tlv_kdext_rec_get_online_nbestrslt:lat is empty!\n");
        return NULL;
    }
    r->nbest_rslt->clear();

    CompactLattice clat_cpy = *(r->process_clat);
    Lattice lat_determinize;
    scale_clat_before_get_result(r, &clat_cpy);
    ConvertLattice(clat_cpy, &lat_determinize);

    std::vector<Lattice> nbest_lats;
    {
      Lattice nbest_lat;
      fst::ShortestPath(lat_determinize, &nbest_lat, nbest);
      fst::ConvertNbestToVector(nbest_lat, &nbest_lats);
    }
    for(size_t i=0; i<nbest_lats.size(); ++i)
    {
        CompactLattice nbest_clat;
        ConvertLattice(nbest_lats[i], &nbest_clat); // write in compact form.

        std::string str_rslt = "";
        get_best_rslt_from_clat(r, &nbest_clat, r->is_end, str_rslt);
        if(!str_rslt.empty() || (str_rslt.empty() && i==0))
        {
            r->nbest_rslt->push_back(str_rslt);//sometimes rlst_str is empty,but still output
        }
    }
    return r->nbest_rslt;
}

bool get_best_rslt_from_clat(tlv_kdext_rec_t* r, CompactLattice* clat, bool is_end, std::string& result_out)
{
    using fst::VectorFst;

    CompactLattice best_path;
    CompactLatticeShortestPath(*clat, &best_path);
    if(best_path.Start() == fst::kNoStateId)
    {
        tlv_log("get_best_rslt_from_clat:Possibly empty lattice\n");
        return false;
    }

    VectorFst<LatticeArc> decoded;
    ConvertLattice(best_path, &decoded);

    fst::SymbolTable *word_syms = r->cfg->word_syms;
    std::vector<int32> alignment;
    std::vector<int32> words;
    LatticeWeight weight;
    GetLinearSymbolSequence(decoded, &alignment, &words, &weight);

    if(!word_syms)
    {
        tlv_log("no word symbole!\n");
        return false;
    }

    tlv_kdext_rec_get_sentence(r, words, is_end, result_out);

    return true;
}

void scale_clat_before_get_result(tlv_kdext_rec_t* r, CompactLattice* p_clat)
{
    if(p_clat == NULL)
    {
        return;
    }

    BaseFloat acoustic_scale = 1.0;
    BaseFloat lm_scale = 1.0;
    BaseFloat acoustic2lm_scale = 0.0;
    BaseFloat lm2acoustic_scale = 0.0;
    std::vector<std::vector<double> > scale(2);

    scale[0].resize(2);
    scale[1].resize(2);

    /* lattice-scale --inv-acoustic-scale=17 */
    if(r->cfg->inv_acoustic_scale != 1.0 )
    {
        acoustic_scale = 1.0 / r->cfg->inv_acoustic_scale;
    }
    if(r->cfg->lat_acoustic_scale > 1.0)
    {
        acoustic_scale = acoustic_scale * r->cfg->lat_acoustic_scale;
    }
    scale[0][0] = lm_scale;
    scale[0][1] = acoustic2lm_scale;
    scale[1][0] = lm2acoustic_scale;
    scale[1][1] = acoustic_scale;
    ScaleLattice(scale, p_clat);
    AddWordInsPenToCompactLattice(r->cfg->word_penalty, p_clat);
}

int tlv_kdext_rec_make_grammar_fst(tlv_kdext_rec_t* r) {
  if(!r) {
    return -1;
  }
  if(r->cfg->is_online) {
    if(r->part_fst) delete r->part_fst;
    if(r->grammar_fst) delete r->grammar_fst;
    if(r->word_syms) delete r->word_syms;
    r->part_fst = NULL;
    r->grammar_fst = NULL;
    r->word_syms = NULL;
  }
  int ret=0;
  fst::Fst<StdArc> *std_fst=NULL;
  std::vector<std::pair<int32, const ConstFst<StdArc>* > > pairs;
  int nonterminal = 328, offset=324;
  tlv_charbuf_t* buf;
  const char* part_fst_dir = NULL;
  if(r->part_fst_dir != NULL) {
    part_fst_dir = r->part_fst_dir;
  }
  if(r->cfg->phn_syms) {
    offset = r->cfg->phn_syms->Find("#nonterm_bos");
    nonterminal = r->cfg->phn_syms->Find("#nonterm:unk");
  }
  buf = tlv_charbuf_new(32, 1);
  if(part_fst_dir && strlen(part_fst_dir) > 0) {
    tlv_charbuf_push(buf, part_fst_dir, strlen(part_fst_dir));
    tlv_charbuf_push_s(buf, "/HCLG.fst");
  } else {
    tlv_charbuf_push(buf, r->cfg->part_fst_filename, strlen(r->cfg->part_fst_filename));
  }
  tlv_charbuf_push_c(buf, '\0');
  std_fst = fst::ReadFstKaldiGeneric(buf->data);
  if(std_fst) {
    r->part_fst = dynamic_cast<ConstFst<StdArc>* >(std_fst);
    if(!r->part_fst) {
      r->part_fst = new ConstFst<StdArc>(*std_fst);
      delete std_fst;
    }
    pairs.push_back(std::pair<int32, const ConstFst<StdArc>* >(nonterminal, r->part_fst));
    /* read word.txt */
    tlv_charbuf_reset(buf);
    if(part_fst_dir && strlen(part_fst_dir) > 0) {
      tlv_charbuf_push(buf, part_fst_dir,strlen(part_fst_dir));
      tlv_charbuf_push_s(buf, "/words.txt");
    } else {
      tlv_charbuf_push(buf, r->cfg->part_wrdsyb_filename, strlen(r->cfg->part_wrdsyb_filename));
    }
    tlv_charbuf_push_c(buf, '\0');
    r->word_syms = fst::SymbolTable::ReadText(buf->data);
    if(r->word_syms == NULL) {
      tlv_log("Could not read symbol table from file %s\n", buf->data);
    }
  } else {
    tlv_log("read part fst failed:%s\n", buf->data);
    ret = -1;
    goto end;
  }
  r->grammar_fst = new fst::GrammarFst(324, *(r->cfg->top_fst), pairs);
  if(r->cfg->is_online) {
    r->grammar_decoder_online = new SingleUtteranceNnet3GrammarDecoder(*(r->cfg->config),
                                    *(r->cfg->trans_model),
                                    *(r->decodable_info),
                                    *(r->grammar_fst),
                                    r->fea_pipline);
  }
end:
  tlv_charbuf_delete(buf);
  return ret;
}

/*******************  static method  ********************/
static int tlv_kdext_rec_search_wrdbound(tlv_kdext_rec_t* r, int wrdid, std::vector<int32>& ali)
{
	int i, end_idx;
	tlv_wrdbound_t* wrdbound;
	tlv_dict_word_t* wrd;
	tlv_dict_pron_t* pron;
	tlv_dict_phone_t **phones;
	char flag = 0;
	tlv_charbuf_t *buf = tlv_charbuf_new(5, 1);

	fst::SymbolTable *phn_syms = r->cfg->phn_syms;
	int32 phnid, tmp_phnid;
	std::string s;

	wrdbound = ((tlv_wrdbound_t**)(r->onebest_arr->item))[wrdid];
	if(r->cfg->word_syms)
	{
		s = r->cfg->word_syms->Find(wrdbound->id);
		if(s == "")
		{
            //tlv_log("Word-id %d not in symbol table.\n", wrdbound->id);
			wrdid = -1; goto end;
		}
	}

	tlv_charbuf_push(buf, s.c_str(), s.size());
	wrd = tlv_dict_find_word(r->cfg->dict, buf->data, buf->pos);
	if(NULL == wrd) {
        //tlv_log("wrd[%s] not in dict!\n", s.c_str());
		wrdid = -1; goto end;
	}
	wrdbound->name = wrd->name;

	/* skip sil */
	phnid = ali[wrdbound->start];
	while(phnid == 1 && wrdbound->start < (ali.size()-2) )
	{
		phnid = ali[++wrdbound->start];
	}
	end_idx = wrdbound->start;

	if(wrdbound->len == 0)
	{
		pron = wrd->pron_list;
		while(pron)
		{
			phones = pron->pPhones;
			for(i=0; i < pron->nPhones; i++)
			{
				tlv_charbuf_reset(buf);
				tlv_charbuf_push(buf, phones[i]->name->data, phones[i]->name->len);
				tlv_charbuf_push_c(buf, '\0');
				tmp_phnid = phn_syms->Find(buf->data);

				if(tmp_phnid == ali[end_idx])
				{
					phnid = ali[++end_idx];
					while(phnid == tmp_phnid && end_idx < (ali.size()-2))
					{
						phnid = ali[++end_idx];
					}
				}
				else
				{
					break;
				}

				if(i == (pron->nPhones-1))
				{
					wrdbound->end[wrdbound->len] = end_idx;
					wrdbound->len++;
					flag = 1;
				}
			}

			end_idx = wrdbound->start;
			pron = pron->next;
		}
	}
	else
	{
		wrdbound->end[wrdbound->used] = -1;
		wrdbound->used++;
		if(wrdbound->used < wrdbound->len)
		{
			flag = 1;
		}
	}

	if(flag) /* success match */
	{
		wrdid += 1;
		if(wrdid == r->onebest_arr->nitem)
		{
			goto end;
		}

		end_idx = wrdbound->end[wrdbound->used];
		wrdbound = ((tlv_wrdbound_t**)(r->onebest_arr->item))[wrdid];
		wrdbound->start = end_idx;
	}
	else
	{
		wrdid -= 1;
	}

end:
	tlv_charbuf_delete(buf);

	return wrdid;
}

static int tlv_kdext_rec_get_wrdtimeinfo(tlv_kdext_rec_t* r,std::vector<int32>& ali, std::vector<int32>& words)
{
	int ret=0;
    int frame=0, phnid, wrdid=0, tmp_wrdid=0;;
	TransitionModel* trans_model_ = r->cfg->trans_model;
	tlv_wrdbound_t* wrdbound;
	std::vector<int32>::iterator it;

	/* alloc memory */
	r->onebest_arr = tlv_array_new(r->heap, words.size(), sizeof(tlv_wrdbound_t*));

	/* 删除<SPOKEN_NOISE>字符， 因为它不占时间 */
	for(it=words.begin(); it != words.end();)
	{
		if(*it == 2) /* <SPOKEN_NOISE> 2 */
		{
			it = words.erase(it);
		}
		else
		{
			it++;
		}
	}

	/* 初始化 */
	for(wrdid=0; wrdid < words.size(); wrdid++)
	{
		wrdbound = (tlv_wrdbound_t*)tlv_heap_malloc(r->heap, sizeof(tlv_wrdbound_t));
		memset(wrdbound, 0, sizeof(tlv_wrdbound_t));
		wrdbound->id = words[wrdid];
		*((tlv_wrdbound_t**)tlv_array_push(r->onebest_arr)) = wrdbound;
	}

	for(frame=0; frame < ali.size(); frame++)
	{
		phnid = trans_model_->TransitionIdToPhone(ali[frame]);
		ali[frame] = phnid;
	}

	/* search */
	wrdid = 0;
	while(wrdid < words.size())
	{
		wrdid = tlv_kdext_rec_search_wrdbound(r, wrdid, ali);
        if(tmp_wrdid == -1)
		{
			ret = -1; goto end;
		}
	}

	/* printf */
	for(wrdid=0; wrdid < words.size(); wrdid++)
	{
		wrdbound = ((tlv_wrdbound_t**)r->onebest_arr->item)[wrdid];
		if((wrdid == 0 && wrdbound->start == wrdbound->end[wrdbound->used]) || wrdbound->name == NULL)
		{
			r->onebest_arr->nitem = 0;
			break;
		}
		//printf("%.*s\t%d\t%d\n", wrdbound->name->len, wrdbound->name->data, wrdbound->start, wrdbound->end[wrdbound->used]);
	}

end:

	return ret;
}

//基础能力v1.6开始，服务中增加了加标点服务，此处不用了 2020.02.05
static string tlv_kdext_rec_add_endques(tlv_kdext_rec_t* r, const std::vector<int32> &words)
{
    string s = "。";
    string dst;
    fst::SymbolTable *word_syms = r->cfg->word_syms;
    int i, j, len;
    char** marks;

    marks = (char**)r->cfg->arr_ques_mark->item;
    for(i=0; i < r->cfg->arr_ques_mark->nitem; i++)
    {
        len = strlen(marks[i]);
        dst.clear();
        j = words.size();
        while(dst.length() < len && j > 0)
        {
            std::string s = word_syms->Find(words[--j]);
            if(s == "<SPOKEN_NOISE>")
            {
                continue;
            }
            dst += s;
        }
        //printf("dst=%s\n", dst.c_str());
        dst.clear();
        for(; j<words.size(); j++)
        {
            std::string s = word_syms->Find(words[j]);
            if(s == "<SPOKEN_NOISE>")
            {
                continue;
            }
            dst += s;
        }
        //printf("dst=%s\n", dst.c_str());

        if(dst.find(marks[i]) != std::string::npos )
        {
            //printf("dst=%s, marks[%d]=%s\n", dst.c_str(), i, marks[i]);
            return "？";
        }
    }

    return s;
}

/*------------------- end -------------------*/
