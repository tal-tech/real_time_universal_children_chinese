/*
 * tlv_kaldi_dec_sub_func.cc
 *
 *  Created on: 2019.01.17
 *      Author: will
 */
#include "tlv_kaldi_dec_sub_func.h"
#include "tlv/kaldiext/tlv_kaldi_dec.h"
#include "third/json/cJSON.h"
#include "tlv/kaldiext/struct/tlv_kaldi_dec_extra_params.h"

void sub_func_add_timeinfo(tlv_kaldi_dec_t* dec, cJSON* json, tlv_array_t* arr, bool is_en_asr)
{
    cJSON *parent, *item;
    int i;
    tlv_wrdbound_t* wb;
    tlv_charbuf_t* buf;
    int frame_shift_ms = dec->cfg->feat.frame_shift_ms;
    int frame_factor = dec->cfg->rec.frame_subsampling_factor;

    if(!arr)
    {
        return;
    }

    buf = tlv_charbuf_new(16, 1);
    parent = cJSON_CreateArray();

    for(i=0; i < arr->nitem; i++)
    {
        wb = ((tlv_wrdbound_t**)arr->item)[i];
        item = cJSON_CreateObject();

        if(!wb || !buf || !(wb->name))
        {
            if(!wb)
            {
                tlv_log("error:sub_func_add_timeinfo:wb is null\n");
            }
            else if(!(wb->name))
            {
                tlv_log("error:sub_func_add_timeinfo:wb->name is null\n");
            }
            else
            {
                tlv_log("error:sub_func_add_timeinfo:buf is null\n");
            }
            continue;
        }

        tlv_charbuf_push(buf, wb->name->data, wb->name->len);
        tlv_charbuf_push_c(buf, '\0');
        std::string buffer_lower(buf->data);
        if(is_en_asr)
        {
            std::transform(buffer_lower.begin(),buffer_lower.end(),buffer_lower.begin(),::tolower);
        }
        cJSON_AddItemToObject(item, "word", cJSON_CreateString(buffer_lower.c_str()));
        cJSON_AddNumberToObject(item, "beginTime", wb->start * frame_shift_ms * frame_factor);
        cJSON_AddNumberToObject(item, "endTime", wb->end[wb->used] * frame_shift_ms * frame_factor);
        double silenceTime = 0;
        if(i < arr->nitem-1)
        {
            tlv_wrdbound_t* wbNext = ((tlv_wrdbound_t**)arr->item)[i+1];
            silenceTime = (wbNext->start-wb->end[wb->used]) * frame_shift_ms * frame_factor;
        }
        cJSON_AddNumberToObject(item, "silenceTime", silenceTime);
        cJSON_AddItemToArray(parent, item);

        tlv_charbuf_reset(buf);
    }

    cJSON_AddItemToObject(json, "details", parent);

    tlv_charbuf_delete(buf);
}

void sub_func_add_end_silence(tlv_kaldi_dec_t* dec, cJSON* json, const std::vector<int32>& alignment)
{
    if(!dec || !json)
    {
        return;
    }
    tlv_array_t* arr = dec->rec->onebest_arr;
    if(!arr)
    {
        return;
    }

    int last_word_end_align = 0;
    int frame_shift_ms = dec->cfg->feat.frame_shift_ms;
    int frame_factor = dec->cfg->rec.frame_subsampling_factor;
    if(arr->nitem > 0)
    {
        tlv_wrdbound_t* wb = ((tlv_wrdbound_t**)arr->item)[dec->rec->onebest_arr->nitem-1];
        int last_word_time = wb->end[wb->used] * frame_shift_ms * frame_factor;
        last_word_end_align = wb->end[wb->used];
        if(last_word_time <= 0)
        {
            last_word_end_align = wb->start;
        }
    }

    //find sil
    int sil_count = 0;
    if(alignment.size()>0 && last_word_end_align>0)
    {
        for(size_t j=last_word_end_align; j<alignment.size(); ++j)
        {
            int32 sil_code = dec->cfg->rec.sil_code;
            if(alignment[j] == sil_code)
            {
                ++sil_count;
            }
        }
    }
    int cur_sil_time = sil_count * frame_shift_ms * frame_factor;
    cJSON_AddItemToObject(json, "sil_dur_time", cJSON_CreateNumber(cur_sil_time));
}

bool sub_func_is_need_timeinfo(tlv_kaldi_dec_t* dec)
{
    if(!dec || !(dec->rec))
    {
        return false;
    }

    bool need_time_info = false;
    /////use_timeinfo控制是否输出词的时间信息，但加标点服务上线后，都得设置为输出词的时间信息，加标点服务需要
    if(dec->rec->cfg->use_timeinfo == 1)
    {
        need_time_info = true;
    }

    //输出静默时长，需要timeinfo信息
    if(dec->extra_params && dec->extra_params->end_silence)
    {
        need_time_info = true;
    }
    return need_time_info;
}

void sub_func_add_debug_params_info(tlv_kaldi_dec_t* dec, cJSON* json, bool is_en_asr);
void sub_func_add_request_params_info(tlv_kaldi_dec_t* dec, cJSON* json, bool is_en_asr)
{
    if(json==NULL || dec==NULL)
    {
        return;
    }

    //请求的参数解析一下原样返回到结果中，用于比对
    cJSON* cjson = cJSON_Parse(dec->extra_params->original_params.c_str());
    if(cjson)
    {
        cJSON_AddItemToObject(json, "request_params", cjson);
    }
    else if(!dec->extra_params->original_params.empty())
    {
        cJSON_AddItemToObject(json, "request_params", cJSON_CreateString("param parse failed"));
    }

    //for debug : control by param
    //请求参数中增加output_params=1，结果中输出调试信息，调试信息中包含最终使用的热词等参数，request_params由于内部判断条件，并不一定都会保留
    if(dec->extra_params->output_params == 1)
    {
        sub_func_add_debug_params_info(dec, json, is_en_asr);
        cJSON_AddItemToObject(json, "have_parsed_param", cJSON_CreateBool(true));
    }
    else
    {
        cJSON_AddItemToObject(json, "have_parsed_param", cJSON_CreateBool(false));
    }
}

void sub_func_add_debug_params_info(tlv_kaldi_dec_t* dec, cJSON* json, bool is_en_asr)
{
    cJSON *parent = cJSON_CreateObject();
    std::string hot_words = "";
    for(size_t i=0; i<dec->extra_params->hot_words.size(); i++)
    {
        if(i > 0)
        {
            hot_words += "_";
        }
        hot_words += (dec->extra_params->hot_words)[i];
    }
    if(is_en_asr)
    {
        std::transform(hot_words.begin(),hot_words.end(),hot_words.begin(),::tolower);
    }
    cJSON_AddItemToObject(parent, "hot_words_select", cJSON_CreateString(hot_words.c_str()));
    cJSON_AddItemToObject(parent, "nbest", cJSON_CreateNumber(dec->extra_params->nbest));
    cJSON_AddItemToObject(parent, "hot_word_nbest", cJSON_CreateNumber(dec->extra_params->hot_word_nbest));
    cJSON_AddItemToObject(parent, "hot_word_nbest_reorder", cJSON_CreateNumber(dec->extra_params->hot_word_nbest_reorder));
    cJSON_AddItemToObject(parent, "global_weight", cJSON_CreateNumber(dec->extra_params->hot_word_global_weight));

    cJSON_AddItemToObject(json, "requestparsedparams", parent);
}

void sub_func_add_nbest_rslt(cJSON* json, const std::vector<std::string>* results, bool is_en_asr, int output_num)
{
    if(json==NULL || results==NULL)
    {
        tlv_log("error:tlv_kaldi_dec_add_nbest_rslt:input is null\n");
        return;
    }
    if(output_num <= 1)
    {
        return;
    }

    cJSON *parent = cJSON_CreateArray();
    for(size_t i=0; i<(*results).size()&&i<output_num; i++)
    {
        std::string str_tmp = (*results)[i];
        if(i>0 && str_tmp.empty())
        {
            continue;
        }

        if(is_en_asr)
        {
            std::transform(str_tmp.begin(),str_tmp.end(),str_tmp.begin(),::tolower);
        }
        cJSON *item = cJSON_CreateString(str_tmp.c_str());
        cJSON_AddItemToArray(parent, item);
    }

    cJSON_AddItemToObject(json, "nbest", parent);
}

void sub_func_sort_by_score(std::vector<std::string>* nbest_rst, std::vector<int>& score_list)
{
    size_t size = nbest_rst->size();
    if(size != score_list.size())
    {
        return;
    }

    for(size_t i=0; i<size-1; ++i)
    {
        for(size_t j=0; j<size-i-1; ++j)
        {
            if(score_list[j] < score_list[j+1])
            {
                int score_tmp = score_list[j];
                std::string str_tmp = (*nbest_rst)[j];
                score_list[j] = score_list[j+1];
                (*nbest_rst)[j] = (*nbest_rst)[j+1];
                score_list[j+1] = score_tmp;
                (*nbest_rst)[j+1] = str_tmp;
            }
        }
    }
}

int sub_func_caculate_match_score(const std::string& sentence, const std::vector<std::string>& hot_words)
{
    int score = 0;
    for(size_t i=0; i<hot_words.size(); ++i)
    {
        if(sentence.find(hot_words[i]) != std::string::npos)
        {
            ++score;//TODO-cac number
        }
    }

    return score;
}

void sub_func_reorder_by_hot_words(tlv_kaldi_dec_t* dec, std::vector<std::string>* nbest_rst)
{
    if(dec==NULL || nbest_rst==NULL || dec->extra_params==NULL)
    {
        return;
    }
    if(dec->extra_params->hot_words.empty())
    {
        return;
    }
    std::vector<int> match_score_list;
    for(size_t i=0; i<nbest_rst->size(); ++i)
    {
        int match_score = sub_func_caculate_match_score((*nbest_rst)[i], dec->extra_params->expend_hot_words);
        match_score_list.push_back(match_score);
    }
    sub_func_sort_by_score(nbest_rst, match_score_list);
}

void sub_func_record_nbest_point(tlv_kaldi_dec_t* dec)
{
    if(!dec)
    {
        return;
    }

    if(dec->rec->cfg->use_grammar_fst) {
      dec->rec->last_nbest_frame_count =
            dec->rec->grammar_decoder_online->NumFramesDecoded() * dec->rec->cfg->frame_subsampling_factor;
    }
    else {
      dec->rec->last_nbest_frame_count =
            dec->rec->decoder_online->NumFramesDecoded() * dec->rec->cfg->frame_subsampling_factor;
    }
}

int32 sub_func_time_span_from_last_nbest(tlv_kaldi_dec_t* dec)
{
    if(!dec)
    {
        return 0;
    }
    kaldi::int64 cur_frame_count = 0;
    if(dec->rec->cfg->use_grammar_fst) {
      cur_frame_count = dec->rec->grammar_decoder_online->NumFramesDecoded() * dec->rec->cfg->frame_subsampling_factor;
    }
    else {
      cur_frame_count = dec->rec->decoder_online->NumFramesDecoded() * dec->rec->cfg->frame_subsampling_factor;
    }
    int32 span_time = (int32)((cur_frame_count-dec->rec->last_nbest_frame_count)*dec->cfg->feat.frame_shift_ms);
    return span_time;
}

