#include "third/json/cJSON.h"
#include "tlv/kaldi/base/kaldi-types.h"
#include "tlv/kaldiext/tlv_kaldi_dec_cfg.h"
#include "tlv/kaldiext/utils/tokenizer/tokenizer.h"
#include "tlv/kaldiext/utils/utility.h"
#include "tlv/struct/tlv_sys.h"
#include "tlv_kaldi_vadec_interface.h"
#include "vadec/src/vadec_stream.h"
#ifdef PROFILE
#include <gperftools/profiler.h>
#endif

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mutex>
#include <staticjson/staticjson.hpp>
#include <string>
#include <thread>
#include <vector>

const uint32_t kSampleRate = 16000;
std::ofstream g_res_f;
std::vector<float> g_rtfs;
bool g_show_log = false;
double g_total_wav_time = 0;
bool g_show_mid_result = false;
#ifdef DELAY_TEST
float g_wav_time = 0.0f;
std::chrono::high_resolution_clock::time_point g_start_time;
#endif

int kMaxThreadNumber = 100;
std::vector<std::mutex> g_mutexs(kMaxThreadNumber);
std::vector<std::condition_variable> g_cvs(kMaxThreadNumber);
std::vector<std::string> g_wav_name(kMaxThreadNumber);
std::vector<int> g_all_end_flags(kMaxThreadNumber, 0);
std::mutex g_mu_wav;
std::mutex g_mu_res;
std::mutex g_mu_rtf;

using namespace staticjson;

struct RecResult {
  bool is_partial;
  int begin_time;
  int end_time;
  int code;
  int last_decode_time;
  int sil_dur_time{0};
  std::vector<std::string> hot_words;
  std::string result;

  RecResult() {}

  void staticjson_init(ObjectHandler* h) {
    h->add_property("is_partial", &is_partial);
    h->add_property("begin_time", &begin_time);
    h->add_property("end_time", &end_time);
    h->add_property("code", &code);
    h->add_property("last_decode_time", &last_decode_time, Flags::Optional);
    h->add_property("hot_words", &hot_words, Flags::Optional);
    h->add_property("result", &result, Flags::Optional);
    h->add_property("sil_dur_time", &sil_dur_time, Flags::Optional);
  }
};

struct extram_param_input {
  int nbest;
  std::vector<std::string> hot_words;
  kaldi::BaseFloat global_weight;
  int hot_word_nbest;
  int hot_word_nbest_reorder;
  int output_end_silence;
  int use_vad;
  float vad_max_sentence_second;
  std::string part_fst_dir;

  extram_param_input()
      : nbest(0),
        global_weight(0),
        hot_word_nbest(0),
        hot_word_nbest_reorder(0),
        output_end_silence(0),
        use_vad(1),
        vad_max_sentence_second(15.0f),
        part_fst_dir("") {}
  extram_param_input& operator=(const extram_param_input& epi) {
    if (this != &epi) {
      this->nbest = epi.nbest;
      this->hot_word_nbest = epi.hot_word_nbest;
      this->hot_word_nbest_reorder = epi.hot_word_nbest_reorder;
      this->output_end_silence = epi.output_end_silence;
      this->use_vad = epi.use_vad;
      this->vad_max_sentence_second = epi.vad_max_sentence_second;
      this->hot_words = epi.hot_words;
      this->global_weight = epi.global_weight;
      this->part_fst_dir = epi.part_fst_dir;
    }
    return *this;
  }
};

std::string formate_param_json(extram_param_input* params);

void parse_params_lable_file(
    const extram_param_input& epi, const std::string& file_name,
    std::map<std::string, std::string>& extra_params_map);

void vadec_callback(const char* json, unsigned int len, bool all_end,
                    const std::string userdata) {
  ParseStatus ps;
  RecResult rr;
  if (!from_json_string(std::string(json, len).c_str(), &rr, &ps)) {
    if (g_show_log) {
      std::cout << "json parse failed:" << ps.description() << std::endl;
    }
  } else {
    if (rr.code == 0) {
      if (g_show_mid_result || !rr.is_partial) {
        std::string key_name = g_wav_name[atoi(userdata.c_str())];
#ifdef DELAY_TEST
        float delay_time = g_wav_time - rr.end_time;
        auto end = std::chrono::high_resolution_clock::now();
        double real_delay_time =
            std::chrono::duration<double>(end - g_start_time).count();
        g_res_f << key_name << " " << std::string(json, len)
                << " delay_time:" << delay_time << "ms"
                << " real_delay_time:" << real_delay_time * 1000.0f << "ms"
                << std::endl;
        std::cout << key_name << " " << real_delay_time << std::endl;
#else
        g_mu_res.lock();
        g_res_f << key_name << " " << std::string(json, len) << std::endl;
        g_mu_res.unlock();
        std::cout << key_name << " " << std::string(json, len) << std::endl;
#endif
      }
    }
  }

  if (all_end) {
    int thread_index = atoi(userdata.c_str());
    g_all_end_flags[thread_index] = 1;
    g_cvs[thread_index].notify_all();
  }
}

int CheckWavHeader(std::string wav_path) {
  std::ifstream wav(wav_path, std::ios::binary);
  if (!wav) {
    printf("%s is not exist!\n", wav_path.c_str());
    return -1;
  }
  char tag[5];
  memset(tag, 0, 5);
  // check RIFF
  wav.read(tag, 4);
  if (strcmp(tag, "RIFF") != 0) {
    std::cout << wav_path << " Wav header is wrong, expectd \"RIFF\" not \""
              << tag << "\"" << std::endl;
    return -1;
  }
  uint32_t chunk_size;
  wav.read((char*)&chunk_size, 4);
  // check WAVE
  wav.read(tag, 4);
  if (strcmp(tag, "WAVE") != 0) {
    std::cout << wav_path << " Wav header is wrong, expectd \"WAVE\" not \""
              << tag << "\"" << std::endl;
    return -1;
  }
  wav.read(tag, 4);
  // check "JUNK", Apple devices produce a filler tag 'JUNK' for memroy
  // alignment.
  if (strcmp(tag, "JUNK") == 0) {
    wav.read((char*)&chunk_size, 4);
    if (chunk_size & 1) chunk_size += 1;
    for (int i = 0; i < chunk_size; i++) {
      wav.read(tag, 1);
    }
    wav.read(tag, 4);
  }
  // check "fmt "
  if (strcmp(tag, "fmt ") != 0) {
    std::cout << wav_path << " Wav header is wrong, expectd \"fmt \"  not \""
              << tag << "\"" << std::endl;
    return -1;
  }
  // size1
  wav.read((char*)&chunk_size, 4);
  // format tag
  uint16_t format_tag;
  wav.read((char*)&format_tag, 2);
  // channel
  uint16_t channel;
  wav.read((char*)&channel, 2);
  if (channel != 1) {
    std::cout << wav_path << " Wav header is wrong, expectd one channel  not "
              << channel << std::endl;
    return -1;
  }
  // sample rate
  uint32_t sample_rate;
  wav.read((char*)&sample_rate, 4);
  if (sample_rate != kSampleRate) {
    std::cout << wav_path << " Wav header is wrong, expectd sample rate "
              << kSampleRate << " not " << sample_rate << std::endl;
    return -1;
  }
  // TODO: check bits_rate, data tag
  wav.close();
  return 0;
}

void SingleTest(tlv_kaldi_dec_cfg_t* cfg, std::ifstream& wav_f,
                int thread_index,
                std::map<std::string, std::string>& extra_params_map,
                std::string extra_params_base) {
  while (true) {
    g_all_end_flags[thread_index] = 0;
    std::unique_lock<std::mutex> ul(g_mu_wav);
    std::string wav_fn;
    if (!std::getline(wav_f, wav_fn)) {
      break;
    }
    ul.unlock();
    if (CheckWavHeader(wav_fn) != 0) {
      continue;
    }
    std::ifstream wav(wav_fn, std::ios::binary);
    std::vector<char> wav_content(
        static_cast<std::istreambuf_iterator<char>>(wav), {});
    int data_offset = 44;
    char* wav_data = wav_content.data() + data_offset;
    int wav_len = static_cast<int>(wav_content.size()) - data_offset;

    std::string wav_file_name = utils::GetWavFileName(wav_fn);
    std::string extra_params = "";
    if (extra_params_map.find(wav_file_name) != extra_params_map.end()) {
      extra_params = extra_params_map[wav_file_name];
    } else {
      extra_params = extra_params_base;
    }
    tlv_kaldi_vadec_t* dec =
        tlv_kaldi_vadec_new(cfg, vadec_callback, std::to_string(thread_index));
    if (tlv_kaldi_vadec_start(dec, extra_params.c_str(),
                              extra_params.length()) != 0) {
      tlv_kaldi_vadec_delete(dec);
      continue;
    }
    g_wav_name[thread_index] = wav_file_name;
    // test
    const int step = 1024;  // 1024bytes, 32ms
    int i = 0;
    auto start = std::chrono::high_resolution_clock::now();
#ifdef DELAY_TEST
    g_wav_time = 0.0f;
    g_start_time = start;
#endif
    while (i <= wav_len) {
      int real_len = i + step >= wav_len ? wav_len - i : step;
#ifdef DELAY_TEST
      std::this_thread::sleep_for(std::chrono::milliseconds(real_len / 32));
      g_wav_time += real_len / 32.0f;
#endif
      if ((i + step) >= wav_len) {
        tlv_kaldi_vadec_feed(dec, wav_data + i, real_len, 1, 1);
        break;
      } else {
        tlv_kaldi_vadec_feed(dec, wav_data + i, real_len, 0, 0);
      }
      i += step;
    }
    std::unique_lock<std::mutex> um(g_mutexs[thread_index]);
    float wav_time = wav_len / 2.0f / 16000.0f;  // seconds
    g_total_wav_time += wav_time;
    g_cvs[thread_index].wait(um, [=]() {
      return g_all_end_flags[thread_index] == 1 ? true : false;
    });
    auto end = std::chrono::high_resolution_clock::now();
    double recognize_time = std::chrono::duration<double>(end - start).count();
    float rtf = (float)recognize_time / wav_time;
    g_mu_rtf.lock();
    g_rtfs.push_back(rtf);
    g_mu_rtf.unlock();
#ifndef DELAY_TEST
    std::cout << "wav_time:" << wav_time << " asr_time:" << recognize_time
              << "s RTF:" << rtf << "x" << std::endl;
#endif
    tlv_kaldi_vadec_delete(dec);
  }
}

int main(int argc, char* argv[]) {
  const char* usage =
      "ASR Online SDK test tool.\n"
      "Usage: vades_s [options...] <cfg_path> <scp_path> <res_path>\n";
  ParseOptions po(usage);
  std::string hotwords_path = "";
  std::string part_fst_dir = "";
  int nbest = 0;
  int output_end_silence = 0;
  float vad_max_sentence_second = 15;
  int threads_number = 1;
  int log_flag = 0;
  int interim_result_flag = 0;
  int global_weight = 0;
  int use_vad = 1;
  g_rtfs.resize(0);

  po.Register("hotwords_path", &hotwords_path, "Hot words file path");
  po.Register("part_fst_dir", &part_fst_dir, "Hot words resource dir");
  po.Register("nbest", &nbest, "Set nbest");
  po.Register("output_end_silence", &output_end_silence,
              "output silence duration, 0 open, 1 close");
  po.Register("use_vad", &use_vad,
              "VAD switch, 1 using VAD, 0 close VAD, default is 1");
  po.Register("vad_max_sentence_second", &vad_max_sentence_second,
              "Vad max sentence second");
  po.Register("global_weight", &global_weight, "Hot words global weight");
  po.Register("threads_number", &threads_number, "Max threads number");
  po.Register("log_flag", &log_flag, "Log flag");
  po.Register("interim_result_flag", &interim_result_flag,
              "Interim result flag");
  po.Read(argc, argv);

  if (po.NumArgs() != 3) {
    po.PrintUsage();
    exit(1);
  }
#ifdef PROFILE
  ProfilerStart("vadec_s.prof");
#endif
  std::string cfg_fn = po.GetArg(1);
  std::string wav_list = po.GetArg(2);
  std::string g_res_file = po.GetArg(3);
  // std::cout << "cfg_fn:" << cfg_fn << std::endl;
  // std::cout << "wav_list:" << wav_list << std::endl;
  // std::cout << "g_res_file:" << g_res_file << std::endl;
  std::map<std::string, std::string> extra_params_map;
  std::string extra_params = "";
  extram_param_input epi;
  epi.nbest = nbest;
  epi.vad_max_sentence_second = vad_max_sentence_second;
  epi.output_end_silence = output_end_silence;
  epi.use_vad = use_vad;
  epi.global_weight = global_weight;
  epi.part_fst_dir = part_fst_dir;
  if (hotwords_path != "") {
    parse_params_lable_file(epi, hotwords_path, extra_params_map);
  }
  extra_params = formate_param_json(&epi);
  if (interim_result_flag == 1) {
    g_show_mid_result = true;
  }
  // 1. init resources
  auto start = std::chrono::high_resolution_clock::now();
  tlv_kaldi_dec_cfg_t* cfg = tlv_kaldi_dec_cfg_new(cfg_fn.c_str());
  int is_en = tlv_kaldi_dec_cfg_get_is_en(cfg);
  if (g_show_log) {
    std::cout << "is en resource:" << is_en << std::endl;
  }
  char* version;
  tlv_kaldi_dec_cfg_get_version(cfg, &version);
  std::cout << "model_version:" << version << std::endl;
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> init_time = end - start;
  std::cout << "Finit init using time:" << init_time.count() << "s"
            << std::endl;

  g_res_f.open(g_res_file, std::ios::out);
  std::ifstream wav_f(wav_list);
  if (!wav_f) {
    printf("%s file is not exist!\n", wav_list.c_str());
    return -1;
  }
  // 2. init threads
  threads_number = threads_number <= 0 ? 1 : threads_number;
  start = std::chrono::high_resolution_clock::now();
  std::vector<std::thread> threads;
  for (auto i = 0; i < threads_number; i++) {
    std::thread t(SingleTest, cfg, std::ref(wav_f), i,
                  std::ref(extra_params_map), extra_params);
    threads.push_back(std::move(t));
  }
  for (auto& t : threads) t.join();
  end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> total_time = end - start;
  if (g_show_log) {
    std::cout << "wav total duration:" << g_total_wav_time << std::endl;
  }
#ifdef PROFILE
  ProfilerStop();
#endif
#ifndef DELAY_TEST
  if (g_rtfs.size() > 0) {
    float sum_rtfs = 0.0f;
    for (auto i = 0; i < g_rtfs.size(); i++) {
      sum_rtfs += g_rtfs[i];
    }
    printf(
        "Test %d wavs, total_time:%0.4fs, total_wav_tim:%0.4fs, "
        "average_rtf:%0.4fx, "
        "max_rtf:%0.4fx, min_rtf:%0.4fx\n",
        (int)g_rtfs.size(), total_time.count(), g_total_wav_time,
        sum_rtfs / (float)g_rtfs.size(),
        g_rtfs[max_element(g_rtfs.begin(), g_rtfs.end()) - g_rtfs.begin()],
        g_rtfs[min_element(g_rtfs.begin(), g_rtfs.end()) - g_rtfs.begin()]);
  }
#endif
  return 0;
}

std::string formate_param_json(extram_param_input* params) {
  if (params == NULL) {
    return "";
  }

  cJSON* parent = cJSON_CreateObject();
  if (params->nbest > 1) {
    cJSON_AddItemToObject(parent, "nbest", cJSON_CreateNumber(params->nbest));
  }

  cJSON* priorSetObj = cJSON_CreateObject();

  cJSON* hotWordsObj = cJSON_CreateArray();
  for (size_t i = 0; i < params->hot_words.size(); ++i) {
    std::string& word = (params->hot_words)[i];
    cJSON* item = cJSON_CreateString(word.c_str());
    cJSON_AddItemToArray(hotWordsObj, item);
  }
  cJSON_AddItemToObject(priorSetObj, "hot_words", hotWordsObj);

  if (params->global_weight > 0) {
    cJSON_AddItemToObject(priorSetObj, "global_weight",
                          cJSON_CreateNumber(params->global_weight));
  }

  cJSON_AddItemToObject(parent, "prior_set", priorSetObj);
  if (g_show_log) {
    cJSON_AddItemToObject(parent, "output_params", cJSON_CreateNumber(1));
  }

  cJSON_AddItemToObject(parent, "output_end_silence",
                        cJSON_CreateBool(params->output_end_silence));
  cJSON_AddItemToObject(parent, "use_vad", cJSON_CreateBool(params->use_vad));
  cJSON_AddItemToObject(parent, "vad_max_sentence_second",
                        cJSON_CreateNumber(params->vad_max_sentence_second));

  cJSON_AddItemToObject(parent, "part_fst_dir",
                        cJSON_CreateString(params->part_fst_dir.c_str()));
  char* p = cJSON_PrintUnformatted(parent);
  std::string str_rst(p);

  cJSON_Delete(parent);
  free(p);
  return str_rst;
}

void parse_params_lable_file(
    const extram_param_input& epi, const std::string& file_name,
    std::map<std::string, std::string>& extra_params_map) {
  if (file_name == "") {
    return;
  }
  // parse file
  std::ifstream label_file_stream(file_name);
  std::string one_line;
  if (!label_file_stream.is_open()) {
    return;
  }
  extra_params_map.clear();
  // 1576579131655101620_2_7_0_eval@赋得古原草送别@赋得古原草送别
  Tokenizer str_tokenizer;
  while (std::getline(label_file_stream, one_line)) {
    str_tokenizer.set(one_line, "@");
    std::vector<std::string> liststr = str_tokenizer.split();
    if (liststr.size() < 2) {
      tlv_log("error:parse_params_lable_file:lack of label\n");
      continue;
    }

    std::string wav_file_name = liststr[0];
    if (wav_file_name.find(" ") != std::string::npos) {
      tlv_log("error:find space in wav_file_name:%s", wav_file_name.c_str());
      continue;
    }
    extram_param_input params = epi;

    // int liststr:the last is the label,the middle are the hot words
    for (size_t j = 1; j < liststr.size(); ++j) {
      std::string hot_word = liststr[j];
      if (hot_word.size() < 15) {
        params.hot_words.push_back(hot_word);
      }

      if (hot_word.find(" ") != std::string::npos) {
      }
    }
    std::string json = formate_param_json(&params);
    extra_params_map[wav_file_name] = json;
  }
}
