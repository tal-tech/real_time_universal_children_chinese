/*
 * tlv_kaldi_dec.cc
 *
 *  Created on: Jan 3, 2019
 *      Author: jfyuan
 */
#include "tlv/kaldiext/tlv_kaldi_dec.h"
#include "third/json/cJSON.h"
#include "tlv/kaldiext/struct/tlv_kaldi_dec_extra_params.h"
#include "tlv/kaldiext/utils/tlv_kaldi_dec_sub_func.h"
#include "tlv/struct/tlv_sys.h"


//declare
static void tlv_kaldi_dec_get_errrslt(tlv_kaldi_dec_t* dec, cJSON* json);
static void tlv_kaldi_dec_get_rslt_inner(tlv_kaldi_dec_t* dec, cJSON* json);

tlv_kaldi_dec_t* tlv_kaldi_dec_new(const tlv_kaldi_dec_cfg_t* cfg)
{
	tlv_kaldi_dec_t* dec = NULL;

	if(cfg->use_log)
	{
		tlv_log("enter!\n");
	}

	dec = (tlv_kaldi_dec_t*)tlv_malloc(sizeof(tlv_kaldi_dec_t));
	dec->feat = tlv_kdext_feat_new(&(cfg->feat));

#ifdef USE_NNET1
	dec->forward = tlv_kdext_forward_new(&(cfg->forward));
#endif

	if(cfg->is_online)
	{
		dec->rec = tlv_kdext_rec_online_new(&(cfg->rec), dec->feat->feature_pipeline);
	}
	else
	{
		dec->rec  = tlv_kdext_rec_new(&(cfg->rec));
	}

	dec->cfg  = cfg;
	dec->errinfo = tlv_errinfo_new();
	dec->feat->errinfo = dec->errinfo;

#ifdef USE_NNET1
	dec->forward->errinfo = dec->errinfo;
#endif

	dec->rec->errinfo  = dec->errinfo;
    dec->t_tot = time_get_ms();
	dec->t_data_feed_end = 0;

    dec->t_delay =0;
	
    dec->extra_params = new tlv_extra_param(cfg);

	if(cfg->use_log)
	{
		tlv_log("leave! dec=%p\n", dec);
	}

	return dec;
}

int tlv_kaldi_dec_start(tlv_kaldi_dec_t* dec, tlv_extra_param_t* extra_params)
{
	if(dec->cfg->use_log)
	{
		tlv_log("enter!\n");
	}

    dec->t_tot = time_get_ms();

    if(extra_params != NULL)
    {
        *(dec->extra_params) = *(extra_params);
    }

    if(dec->cfg->rec.use_fst_combine) {
      if(dec->extra_params) {
        dec->rec->part_fst_dir = dec->extra_params->part_fst_dir.c_str();
      }
      tlv_kdext_rec_make_grammar_fst(dec->rec);
    }
	return 0;
}

int tlv_kaldi_dec_feed(tlv_kaldi_dec_t* dec, char* data, int len, unsigned char is_end)
{
	int ret = 0;
	double t;
	Matrix<BaseFloat> *nnet_out_host = NULL;

	if(dec->cfg->use_log)
	{
		tlv_log("enter!\n");
	}

//	if(!is_end)
//	{
//		tlv_log("not surport data stream!\n");
//		return -1;
//	}

	/* at least 50ms data */
//	if(len <= 1600 || data == NULL)
//	{
//		tlv_errinfo_set(dec->errinfo, 30002, "data[param] or len[param] is invalid!", 0);
//		return -1;
//	}

    t = time_get_ms();

	ret |= tlv_kdext_feat_feed(dec->feat, data, len, is_end);
	if(0 != ret) { goto end; }

	if(dec->cfg->use_nnet3)
	{
		nnet_out_host = dec->feat->fea;
	}
	else
	{
#ifdef USE_NNET1
    t = time_get_ms();
	ret |= tlv_kdext_forward_feed(dec->forward, dec->feat->fea);
    dec->t_forward = time_get_ms() - t;
	if(0 != ret) { goto end; }
	nnet_out_host = dec->forward->nnet_out_host;
#endif
	}

	if(dec->cfg->is_online)
	{
		ret |= tlv_kdext_rec_online_feed(dec->rec, dec->extra_params, is_end);
	}
	else
	{
		ret |= tlv_kdext_rec_feed(dec->rec, nnet_out_host, dec->extra_params);
	}
    dec->t_rec += time_get_ms() - t;

	if(0 != ret) { goto end; }

end:
//	printf("forward: %f ,rec: %f rate: %f ,wavtime: %d\n", dec->t_forward, dec->t_rec,
//			(dec->t_forward+dec->t_rec) / (dec->cfg->feat.frame_shift_ms * dec->rec->frame_count), dec->cfg->feat.frame_shift_ms * dec->rec->frame_count);
	if(is_end)
	{
        dec->t_data_feed_end = time_get_ms();
	}

	return ret;
}

int tlv_kaldi_dec_reset(tlv_kaldi_dec_t* dec)
{
  int ret = 0;
  ret |= tlv_kdext_feat_reset(dec->feat);
#ifdef USE_NNET1
  ret |= tlv_kdext_forward_reset(dec->forward);
#endif
  if(dec->cfg->is_online) {
    ret |= tlv_kdext_rec_online_reset(dec->rec, dec->feat->feature_pipeline);
  }
  else {
    ret |= tlv_kdext_rec_reset(dec->rec);
  }
  tlv_errinfo_reset(dec->errinfo);
  dec->t_tot = dec->t_rec = dec->t_data_feed_end =  0;
  return ret;
}

int tlv_kaldi_dec_clear(tlv_kaldi_dec_t* dec) {
  int ret = 0;
  ret |= tlv_kdext_feat_reset(dec->feat);
  dec->rec->fea_pipline = dec->feat->feature_pipeline;
#ifdef USE_NNET1
  ret |= tlv_kdext_forward_reset(dec->forward);
#endif
  if(dec->cfg->is_online) {
    ret |= tlv_kdext_rec_online_clear(dec->rec);
  } else {
    ret |= tlv_kdext_rec_reset(dec->rec);
  }
  tlv_errinfo_reset(dec->errinfo);
  dec->t_tot = dec->t_rec = dec->t_data_feed_end =  0;
  return ret;
}

int tlv_kaldi_dec_delete(tlv_kaldi_dec_t* dec)
{
	int ret=0;

	ret |= tlv_kdext_feat_delete(dec->feat);

#ifdef USE_NNET1
	ret |= tlv_kdext_forward_delete(dec->forward);
#endif

	ret |= tlv_kdext_rec_delete(dec->rec);

	tlv_errinfo_delete(dec->errinfo);

    delete dec->extra_params;
    dec->extra_params = NULL;

	tlv_free(dec);
	//malloc_trim(0);

	return ret;
}

int tlv_kaldi_dec_get_rslt(tlv_kaldi_dec_t* dec, char** rslt, int* rslt_len)
{
	int ret = 0, len;
	cJSON *json;
	char *p = 0;

	json = cJSON_CreateObject();
	cJSON_AddItemToObject(json, "version", cJSON_CreateString(dec->cfg->version));

	if(dec->errinfo->no != 0)
	{
		tlv_kaldi_dec_get_errrslt(dec, json);
	}
	else
	{
        tlv_kaldi_dec_get_rslt_inner(dec, json);
	}

	if(dec->t_data_feed_end > 0)
	{
        dec->t_delay = time_get_ms() - dec->t_data_feed_end;
	}
	dec->t_tot = dec->t_rec + dec->t_delay;
	cJSON_AddNumberToObject(json, "wavetime", tlv_float_round(dec->cfg->feat.frame_shift_ms * dec->rec->frame_count));
    cJSON_AddNumberToObject(json, "systime", tlv_float_round(dec->t_tot));//no use
    cJSON_AddNumberToObject(json, "delaytime", tlv_float_round(dec->t_delay));//no use

	p = cJSON_PrintUnformatted(json);
	cJSON_Delete(json);
	len = strlen(p);
	*rslt = p;
	*rslt_len = len;

	return ret;
}

/********************* static method begin ***********************/

void sub_func_get_nbest_rslt(tlv_kaldi_dec_t* dec, cJSON* json, cJSON* resultItem, bool is_en_asr);
static void tlv_kaldi_dec_get_rslt_inner(tlv_kaldi_dec_t* dec, cJSON* json)
{
	cJSON *item;
	const char* best_rslt = 0;

    bool is_changed = true;
    std::vector<int32> alignment;
    if(!dec->rec->is_end)
	{
        bool need_time_info = sub_func_is_need_timeinfo(dec);
        best_rslt = tlv_kdext_rec_online_get_currslt(dec->rec, need_time_info, is_changed, alignment);
	}
	else
	{
        bool need_time_info = sub_func_is_need_timeinfo(dec);
        if(dec->cfg->is_online)
        {
            best_rslt = tlv_kdext_rec_online_get_bestrslt(dec->rec, need_time_info, alignment);
        }
        else
        {
            best_rslt = tlv_kdext_rec_get_bestrslt(dec->rec, need_time_info, alignment);
        }
	}
	if(!best_rslt) { best_rslt = ""; }

    //add onebest result
    item = cJSON_CreateObject();
    cJSON_AddItemToObject(item, "text", cJSON_CreateString(best_rslt));
    cJSON_AddNumberToObject(item, "conf", dec->rec->like);
    if(!dec->rec->is_end)
    {
        cJSON_AddNumberToObject(item, "is_end", 0);
    }
    else
    {
        cJSON_AddNumberToObject(item, "is_end", 1);
    }
    cJSON_AddItemToObject(json, "result", item);

    //add nbest
    bool is_en_asr = (strcmp(dec->cfg->language,"en") == 0);
    int nbest_span_limit = 1000;//nbest相对于获取中间结果来说，很耗时，所以得限制频次，这里简单限定为1秒算一次
    int32 nbest_span_time = sub_func_time_span_from_last_nbest(dec);
    kaldi::int64 cur_frame_count = 0;
    if(dec->rec->cfg->use_grammar_fst) {
      cur_frame_count = dec->rec->grammar_decoder_online->NumFramesDecoded();
    } else {
      cur_frame_count = dec->rec->decoder_online->NumFramesDecoded();
    }
    if(dec &&
            ((cur_frame_count > 0 && is_changed && nbest_span_time>=nbest_span_limit) ||
             dec->rec->is_end) &&
            strlen(best_rslt)>0)
    {
        sub_func_get_nbest_rslt(dec, json, item, is_en_asr);
        sub_func_record_nbest_point(dec);
    }


    //add time info
    if(dec->cfg->rec.use_timeinfo)
    {
        sub_func_add_timeinfo(dec, json, dec->rec->onebest_arr, is_en_asr);
        cJSON_AddItemToObject(json, "have_time_info", cJSON_CreateBool(true));
    }
    else
    {
        //have_time_info字段是用于控制：是否在最终结果中输出details字段，本身不会体现在最终结果输出中，处理staticjson不够动态的问题
        cJSON_AddItemToObject(json, "have_time_info", cJSON_CreateBool(false));
    }

    //extral info for test
    if(dec->cfg->rec.hot_word_debug == 1)
    {
        int cur_frame_time = dec->cfg->feat.frame_shift_ms * dec->rec->frame_count;
        cJSON_AddItemToObject(json, "last_decode_time", cJSON_CreateNumber(cur_frame_time));
    }

    if(dec->extra_params && dec->extra_params->end_silence && is_changed)
    {
        sub_func_add_end_silence(dec, json, alignment);
        cJSON_AddItemToObject(json, "have_end_silence", cJSON_CreateBool(true));
    }
    else
    {
        cJSON_AddItemToObject(json, "have_end_silence", cJSON_CreateBool(false));
    }
    if(dec->extra_params) {
      cJSON_AddItemToObject(json, "use_vad", cJSON_CreateBool(dec->extra_params->use_vad));
    }

    //echo request param
    sub_func_add_request_params_info(dec, json, is_en_asr);

    cJSON_AddItemToObject(json, "is_changed", cJSON_CreateBool(is_changed));
}

static void tlv_kaldi_dec_get_errrslt(tlv_kaldi_dec_t* dec, cJSON* json)
{
	cJSON *item;
	tlv_errinfo_t* errinfo = dec->errinfo;

	item = cJSON_CreateObject();
	cJSON_AddNumberToObject(item, "errID", errinfo->no);
	tlv_charbuf_push_c(errinfo->buf, '\0');
	cJSON_AddStringToObject(item, "errMsg", errinfo->buf->data);
	cJSON_AddItemToObject(json, "error", item);
}

/*---------------------  static method end ----------------*/

void sub_func_get_nbest_rslt(tlv_kaldi_dec_t* dec, cJSON* json, cJSON* resultItem, bool is_en_asr)
{
    if(dec==NULL || dec->extra_params==NULL)
    {
        return;
    }
    if(json == NULL)
    {
        tlv_log("error:sub_func_get_nbest_rslt:json is null\n");
        return;
    }

    cJSON_AddItemToObject(json, "hot_word_debug", cJSON_CreateNumber(dec->cfg->rec.hot_word_debug));

    //hot word doc:https://wiki.zhiyinlou.com/pages/viewpage.action?pageId=38239688
    bool have_hot_words = (dec->extra_params->hot_words.size()>0);
    int output_num = dec->extra_params->nbest;
    int nbest_for_caculate = dec->extra_params->nbest;
    if(have_hot_words)
    {
        nbest_for_caculate = std::max(nbest_for_caculate,dec->extra_params->hot_word_nbest);
    }
    if(nbest_for_caculate > 1)
    {
        std::vector<std::string>* nbest = tlv_kdext_rec_get_online_nbestrslt(dec->rec, nbest_for_caculate);
        if(dec->extra_params->hot_word_nbest_reorder==1 && have_hot_words)
        {
            sub_func_reorder_by_hot_words(dec, nbest);

            //update the one best
            if(resultItem && nbest->size()>0)
            {
                std::string one_best_str = (*nbest)[0];
                if(is_en_asr)
                {
                    std::transform(one_best_str.begin(),one_best_str.end(),one_best_str.begin(),::tolower);
                }
                cJSON_ReplaceItemInObject(resultItem,"text", cJSON_CreateString(one_best_str.c_str()));
                *(dec->rec->onebest_rslt) = one_best_str;
            }
        }

        if(output_num > 1)
        {
            sub_func_add_nbest_rslt(json, nbest, is_en_asr, output_num);
            cJSON_AddItemToObject(json, "have_nbest", cJSON_CreateBool(true));
        }
        else
        {
            cJSON_AddItemToObject(json, "have_nbest", cJSON_CreateBool(false));
        }
    }
    if(dec->cfg->rec.hot_word_debug == 1)
    {
        if(dec->extra_params->expend_hot_words.size() > 0)
        {
            bool match_all = true;
            std::string result_str = cJSON_GetObjectItem(resultItem,"text")->valuestring;
            if(is_en_asr)
            {
                std::transform(result_str.begin(),result_str.end(),result_str.begin(),::tolower);
            }
            for(size_t i=0; i<dec->extra_params->expend_hot_words.size(); ++i)
            {
                std::string hot_word = (dec->extra_params->expend_hot_words)[i];
                if(is_en_asr)
                {
                    std::transform(hot_word.begin(),hot_word.end(),hot_word.begin(),::tolower);
                }
                if(result_str.find(hot_word) == std::string::npos)
                {
                    match_all = false;
                    break;
                }
            }
            if(match_all)
            {
                cJSON_AddItemToObject(json, "match_hotwords", cJSON_CreateNumber(1));
            }
            else
            {
                cJSON_AddItemToObject(json, "match_hotwords", cJSON_CreateNumber(0));
            }
        }

        tlv_array_t* arr = dec->rec->onebest_arr;
        if(arr && arr->nitem > 0)
        {
            tlv_array_t* arr = dec->rec->onebest_arr;
            int frame_shift_ms = dec->cfg->feat.frame_shift_ms;
            int frame_factor = dec->cfg->rec.frame_subsampling_factor;
            tlv_wrdbound_t* wb = ((tlv_wrdbound_t**)arr->item)[dec->rec->onebest_arr->nitem-1];
            double end_time = wb->end[wb->used] * frame_shift_ms * frame_factor;
            cJSON_AddItemToObject(json, "last_word_time", cJSON_CreateNumber(end_time));
        }
    }
}
