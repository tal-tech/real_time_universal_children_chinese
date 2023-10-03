#ifndef TLV_KALDI_VADEC_DEFINE_H_
#define TLV_KALDI_VADEC_DEFINE_H_

#include "staticjson/staticjson.hpp"


struct DetailNodes {
    std::string word;
    int begin_time{0};
    int end_time{0};
    int silence_time{0};

    void staticjson_init(staticjson::ObjectHandler* h) {
        h->add_property("word", &word, staticjson::Flags::Optional);
        h->add_property("beginTime", &begin_time, staticjson::Flags::Optional);
        h->add_property("endTime", &end_time, staticjson::Flags::Optional);
        h->add_property("silenceTime", &silence_time, staticjson::Flags::Optional);
    }

    DetailNodes& operator=(const DetailNodes& cls)
    {
        if (this != &cls)
        {
            this->word = cls.word;
            this->begin_time = cls.begin_time;
            this->end_time = cls.end_time;
            this->silence_time = cls.silence_time;
        }
        return *this;
    }
};

struct KaldiDecInnerResult {
    std::string text;
    int is_end{false};

    void staticjson_init(staticjson::ObjectHandler* h) {
        h->add_property("text", &text);
        h->add_property("is_end", &is_end);
    }

    KaldiDecInnerResult& operator=(const KaldiDecInnerResult& cls)
    {
        if (this != &cls)
        {
            this->text = cls.text;
            this->is_end = cls.is_end;
        }
        return *this;
    }
};

struct KaldiReqParamsPriorSet {
    std::vector<std::string> hot_words;

    void staticjson_init(staticjson::ObjectHandler* h) {
        h->add_property("hot_words", &hot_words, staticjson::Flags::Optional);
    }

    KaldiReqParamsPriorSet& operator=(const KaldiReqParamsPriorSet& cls)
    {
        if (this != &cls)
        {
            this->hot_words = cls.hot_words;
        }
        return *this;
    }
};

struct KaldiDecInnerReqParams {
    int nbest{0};
    KaldiReqParamsPriorSet prior_set;
    bool output_end_silence{0};
    bool use_vad{true};
    float vad_max_sentence_second{15.0f};

    void staticjson_init(staticjson::ObjectHandler* h) {
        h->add_property("nbest", &nbest, staticjson::Flags::Optional);
        h->add_property("prior_set", &prior_set, staticjson::Flags::Optional);
        h->add_property("output_end_silence", &output_end_silence, staticjson::Flags::Optional);
        h->add_property("use_vad", &use_vad, staticjson::Flags::Optional);
        h->add_property("vad_max_sentence_second", &vad_max_sentence_second, staticjson::Flags::Optional);
    }

    KaldiDecInnerReqParams& operator=(const KaldiDecInnerReqParams& cls)
    {
        if (this != &cls)
        {
            this->nbest = cls.nbest;
            this->prior_set = cls.prior_set;
            this->output_end_silence = cls.output_end_silence;
            this->use_vad = cls.use_vad;
            this->vad_max_sentence_second = cls.vad_max_sentence_second;
        }
        return *this;
    }
};

struct KaldiDecParsedReqParams {
    std::string hot_words_select;
    int nbest{0};
    int hot_word_nbest{0};
    int hot_word_nbest_reorder{0};
    int global_weight{0};

    void staticjson_init(staticjson::ObjectHandler* h) {
        h->add_property("hot_words_select", &hot_words_select, staticjson::Flags::Optional);
        h->add_property("nbest", &nbest, staticjson::Flags::Optional);
        h->add_property("hot_word_nbest", &hot_word_nbest, staticjson::Flags::Optional);
        h->add_property("hot_word_nbest_reorder", &hot_word_nbest_reorder, staticjson::Flags::Optional);
        h->add_property("global_weight", &global_weight, staticjson::Flags::Optional);
    }

    KaldiDecParsedReqParams& operator=(const KaldiDecParsedReqParams& cls)
    {
        if (this != &cls)
        {
            this->hot_words_select = cls.hot_words_select;
            this->nbest = cls.nbest;
            this->hot_word_nbest = cls.hot_word_nbest;
            this->hot_word_nbest_reorder = cls.hot_word_nbest_reorder;
            this->global_weight = cls.global_weight;
        }
        return *this;
    }
};

struct KaldiDecMidResult {
    std::string version;
    KaldiDecInnerResult result;

    bool have_time_info{false};
    std::vector<DetailNodes> details;

    bool have_end_silence{false};
    int sil_dur_time{0};

    int delay_time{0};
    bool is_changed{true};

    int hot_word_debug{0};
    int match_hotwords{0};
    int last_word_time{0};
    int last_decode_time{0};

    KaldiDecInnerReqParams request_params;

    bool have_parsed_param{false};
    KaldiDecParsedReqParams requestparsedparams;

    bool have_nbest{false};
    std::vector<std::string> nbest;

    void staticjson_init(staticjson::ObjectHandler* h) {
        h->add_property("version", &version, staticjson::Flags::Optional);
        h->add_property("result", &result);

        h->add_property("have_time_info", &have_time_info, staticjson::Flags::Optional);
        h->add_property("details", &details, staticjson::Flags::Optional);

        h->add_property("have_end_silence", &have_end_silence, staticjson::Flags::Optional);
        h->add_property("sil_dur_time", &sil_dur_time, staticjson::Flags::Optional);

        h->add_property("delaytime", &delay_time);
        h->add_property("is_changed", &is_changed, staticjson::Flags::Optional);

        h->add_property("hot_word_debug", &hot_word_debug, staticjson::Flags::Optional);
        h->add_property("match_hotwords", &match_hotwords, staticjson::Flags::Optional);
        h->add_property("last_word_time", &last_word_time, staticjson::Flags::Optional);
        h->add_property("last_decode_time", &last_decode_time, staticjson::Flags::Optional);

        h->add_property("request_params", &request_params, staticjson::Flags::Optional);
        h->add_property("have_parsed_param", &have_parsed_param, staticjson::Flags::Optional);
        h->add_property("requestparsedparams", &requestparsedparams, staticjson::Flags::Optional);

        h->add_property("have_nbest", &have_nbest, staticjson::Flags::Optional);
        h->add_property("nbest", &nbest, staticjson::Flags::Optional);
    }
};

struct KaldiVadecFinalResult {
    int index {0};
    int duration {0};
    std::string result;
    std::string model_version;
    std::string model;
    std::string model_name;
    int begin_time {0};
    int end_time {0};

    int time_delay {0};
    int time_delay_inner {0};

    int64_t the_very_first_ts {0};
    int64_t first_asr_timestamp {0};
    bool is_partial{true};
    bool is_end{false};

    bool have_time_info{false};
    std::vector<DetailNodes> details;

    bool have_end_silence{false};
    int sil_dur_time {0};

    int hot_word_debug{0};
    int match_hotwords{0};
    int last_word_time{0};
    int last_decode_time{0};

    KaldiDecInnerReqParams request_params;

    bool have_parsed_param{false};
    KaldiDecParsedReqParams requestparsedparams;

    bool have_nbest{false};
    std::vector<std::string> nbest;

    int code {0};
    std::string msg;

    void staticjson_init(staticjson::ObjectHandler* h) {
        h->add_property("index", &index);
        h->add_property("duration", &duration);
        h->add_property("result", &result);
        h->add_property("model_version", &model_version);
        h->add_property("model", &model);
        h->add_property("model_name", &model_name);
        h->add_property("begin_time", &begin_time);
        h->add_property("end_time", &end_time);
        h->add_property("time_delay", &time_delay);
        h->add_property("time_delay_inner", &time_delay_inner);
        h->add_property("first_pkg_timestamp", &the_very_first_ts);
        h->add_property("first_asr_timestamp", &first_asr_timestamp);
        h->add_property("is_partial", &is_partial);
        h->add_property("is_end", &is_end);
        h->add_property("code", &code);
        h->add_property("msg", &msg);

        if(have_time_info)
        {
            h->add_property("details", &details, staticjson::Flags::Optional);
        }
        if(have_end_silence)
        {
            h->add_property("sil_dur_time", &sil_dur_time, staticjson::Flags::Optional);
        }

        if(hot_word_debug == 1)
        {
            h->add_property("match_hotwords", &match_hotwords, staticjson::Flags::Optional);
            h->add_property("last_word_time", &last_word_time, staticjson::Flags::Optional);
            h->add_property("last_decode_time", &last_decode_time, staticjson::Flags::Optional);
        }
        h->add_property("request_params", &request_params, staticjson::Flags::Optional);
        if(have_parsed_param)
        {
            h->add_property("requestparsedparams", &requestparsedparams, staticjson::Flags::Optional);
        }

        if(have_nbest)
        {
            h->add_property("nbest", &nbest, staticjson::Flags::Optional);
        }
    }
};

#endif
