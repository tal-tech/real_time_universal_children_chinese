/*
 * tlv_extra_param.c
 *
 *  Created on: 12.10, 2019
 *      Author: will
 */

#include "tlv_kaldi_dec_extra_params.h"

#include "third/json/cJSON.h"
#include "tlv/struct/tlv_define.h"
#include "tlv/kaldiext/tlv_kaldi_dec_cfg.h"
#include "tlv/kaldiext/utils/tokenizer/tokenizer.h"
#include "vadec/src/vadec_stream.h"

tlv_extra_param::tlv_extra_param(const tlv_kaldi_dec_cfg_t* c) : cfg(c),
    end_silence(false),
    use_vad(true),
    output_params(0)
{
    nbest = c->rec.nbest;
    hot_word_nbest = c->rec.hot_word_nbest;
    hot_word_global_weight = c->rec.hot_word_global_weight;
    hot_word_nbest_reorder = c->rec.hot_word_nbest_reorder;
}

// json
//{
//	"nbest": 10,  //控制输出多少组结果
//	"prior_set": {
//		"hot_words": ["yes i can"], //热词列表，最多128个
//		"global_weight": 6,         //热词权重，越大代表越可能有
//		"hot_word_nbest": 10,       //培优定制，在有热词设置时，必须进行nbest查找，在nbest里面再按热词排序，hot_word_nbest是为了防止nbest不设置，但培优又要启用热词的情况
//		"hot_word_nbest_reorder": 1 //是否对nbest再根据热词匹配度排序
//	},
//	"output_params":1              //用于调试，传入1代表在result中输出热词相关参数
//}
//hot word doc:https://wiki.zhiyinlou.com/pages/viewpage.action?pageId=38239688
bool tlv_parse_extra_params(std::string json, tlv_extra_param_t* param)
{
    if(param == NULL)
    {
        tlv_log("error:tlv_parse_extra_params failed,param is null\n");
        return false;
    }
    param->original_params = json;

    cJSON* cjson = cJSON_Parse(json.c_str());
    if(!cjson)
    {
        tlv_log("error:tlv_parse_extra_params parse json failed\n");
        param->original_params.clear();
        return false;
    }

    //先赋默认值，如果用户传参的则使用传参值
    param->nbest = param->cfg->rec.nbest;
    param->hot_word_global_weight = param->cfg->rec.hot_word_global_weight;
    param->hot_word_nbest = param->cfg->rec.hot_word_nbest;
    param->hot_word_nbest_reorder = param->cfg->rec.hot_word_nbest_reorder;
    param->hot_words.clear();
    param->expend_hot_words.clear();
    param->hot_word_ids.clear();
   

    cJSON* nbestObj = cJSON_GetObjectItem(cjson,"nbest");
    if(nbestObj && nbestObj->valueint>=1)
    {
        param->nbest = nbestObj->valueint;
    }
    cJSON* partFstDir = cJSON_GetObjectItem(cjson, "part_fst_dir");
    if(partFstDir) {
      param->part_fst_dir = partFstDir->valuestring;
    }
    cJSON* priorSetObj = cJSON_GetObjectItem(cjson,"prior_set");
    if(priorSetObj)
    {
        //hot words weight
        cJSON* weightObj = cJSON_GetObjectItem(priorSetObj,"global_weight");
        if(weightObj && weightObj->valueint>=0 && weightObj->valueint<=100)
        {
            param->hot_word_global_weight = weightObj->valueint;
        }

        cJSON* hotwordNbestObj = cJSON_GetObjectItem(priorSetObj,"hot_word_nbest");
        if(hotwordNbestObj)
        {
            param->hot_word_nbest = hotwordNbestObj->valueint;
        }

        cJSON* hotwordNbestReorderObj = cJSON_GetObjectItem(priorSetObj,"hot_word_nbest_reorder");
        if(hotwordNbestReorderObj)
        {
            param->hot_word_nbest_reorder = hotwordNbestReorderObj->valueint;
        }
        //hot words list
        cJSON* hostWords = cJSON_GetObjectItem(priorSetObj,"hot_words");
        fst::SymbolTable *word_syms = NULL;
        if (param->cfg->rec.use_grammar_fst) {
          tlv_charbuf_t* buf;
          buf = tlv_charbuf_new(32, 1);
          tlv_charbuf_reset(buf);
          if(param->part_fst_dir != "") {
            tlv_charbuf_push(buf, param->part_fst_dir.c_str(),strlen(param->part_fst_dir.c_str()));
            tlv_charbuf_push_s(buf, "/words.txt");
          } else {
            tlv_charbuf_push(buf, param->cfg->rec.part_wrdsyb_filename, strlen(param->cfg->rec.part_wrdsyb_filename));
          }
          tlv_charbuf_push_c(buf, '\0');
          word_syms = fst::SymbolTable::ReadText(buf->data);
          if(word_syms == NULL) {
            return false;
          }
          tlv_charbuf_delete(buf);
        } else {
          word_syms = param->cfg->rec.word_syms;
        }
        if(hostWords && word_syms!=NULL)
        {
            int count = 0;
            const int kMaxHotwords = 20; // 热词数量限制20个, 超过20个后精度下降厉害
            for(int i=0; i<cJSON_GetArraySize(hostWords); ++i)
            {
                cJSON* itemobj = cJSON_GetArrayItem(hostWords,i);
                if(!itemobj)
                {
                    continue;
                }

                std::string word = itemobj->valuestring;
                Tokenizer toker(word);
                std::vector<std::string> word_split = toker.split();
                if(word_split.empty())
                {
                    continue;
                }

                //for result match
                for(size_t j=0; j<word_split.size(); ++j)
                {
                    std::string word_tmp = word_split[j];
                    std::transform(word_tmp.begin(),word_tmp.end(),word_tmp.begin(),::toupper);
                    param->expend_hot_words.push_back(word_tmp);
                }

                //exclude auxiliary word
                std::string word_select = "";
                std::vector<std::string>::reverse_iterator reverse_iter = word_split.rbegin();
                while(reverse_iter != word_split.rend())
                {
                    std::string word_to_upper = *reverse_iter;
                    std::transform(word_to_upper.begin(),word_to_upper.end(),word_to_upper.begin(),::toupper);
                    const std::set<std::string>& exclude_set = *((param->cfg->rec).exclude_hotwords_set);
                    std::set<std::string>::iterator iter_find = exclude_set.find(word_to_upper);
                    if(iter_find == exclude_set.end())
                    {
                        word_select = word_to_upper;
                        break;
                    }
                    ++reverse_iter;
                }
                if(word_select == "")
                {
                    continue;
                }

                int64 wordid = word_syms->Find(word_select);
                if(wordid != SymbolTable::kNoSymbol)
                {
                    param->hot_word_ids.push_back(wordid);
                    param->hot_words.push_back(word_select);

                    //limit
                    ++count;
                    if(count >= kMaxHotwords)
                    {
                        break;
                    }
                }//end of if(wordid != SymbolTable::kNoSymbol)
            }//end of for
        }

      if (param->cfg->rec.use_grammar_fst) {
        delete word_syms;
      }
    }
    cJSON* jsonObj = cJSON_GetObjectItem(cjson,"output_end_silence");
    if(jsonObj)
    {
        param->end_silence = jsonObj->valueint==1?true:false;
    }
    cJSON* jsonObjUseVad = cJSON_GetObjectItem(cjson,"use_vad");
    if(jsonObjUseVad)
    {
        param->use_vad = jsonObjUseVad->valueint==0?false:true;
    }
    cJSON* jsonObjVadMaxSecond = cJSON_GetObjectItem(cjson,"vad_max_sentence_second");
    if(jsonObjVadMaxSecond)
    {
        param->vad_max_sentence_second = jsonObjVadMaxSecond->valuedouble;
    }
    //can output the params to result json
    cJSON* outputSetObj = cJSON_GetObjectItem(cjson,"output_params");
    if(outputSetObj)
    {
        param->output_params = outputSetObj->valueint;
    }

    cJSON_Delete(cjson);
    return true;
}

void tlv_extra_params_reset(tlv_extra_param_t* param)
{
    if(param == NULL)
    {
        return;
    }
    //重置，默认值
    param->nbest = param->cfg->rec.nbest;
    param->hot_word_global_weight = param->cfg->rec.hot_word_global_weight;
    param->hot_word_nbest = param->cfg->rec.hot_word_nbest;
    param->hot_word_nbest_reorder = param->cfg->rec.hot_word_nbest_reorder;
    param->hot_words.clear();
    param->expend_hot_words.clear();
    param->hot_word_ids.clear();
    param->original_params.clear();
    param->end_silence = false;
    param->use_vad = true;
    param->vad_max_sentence_second =Vadec::VAD_MAX_SENTENCE_SECOND;
    param->part_fst_dir = "";
}
