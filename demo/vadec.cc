#include "tlv_kaldi_vadec_interface.h"
#include "tlv_kaldi_dec_cfg.h"
#include "vadec/src/vadec_stream.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <ctime>
#include <mutex>
#include <condition_variable>

#include <staticjson/staticjson.hpp>
#include "easylogging++.h"

using namespace staticjson;

struct RecResult {
  bool is_partial;
  int begin_time;
  int end_time;

  void staticjson_init(ObjectHandler* h) {
    h->add_property("is_partial", &is_partial);
    h->add_property("begin_time", &begin_time);
    h->add_property("end_time", &end_time);
  }
};

std::ofstream logfile;

bool all_end = false;
std::mutex g_mu;
std::condition_variable g_cv;
std::string GetLocalTime() {
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
  
  return std::string(buffer);
}

void vadec_callback(const char* json, unsigned int len, bool all_end, const std::string userdata) {
  ParseStatus ps;
  RecResult rr;
  if (!from_json_string(std::string(json, len).c_str(), &rr, &ps)) {
    LOG(ERROR) << ps.description() << std::endl;
  }
  else {
    if (rr.is_partial == false) {
      LOG(INFO) << std::string(json, len) << std::endl;
    }
  }

  if (all_end) {
    g_cv.notify_all();
  }
}

int main(int argc, char* argv[]) {
  std::unique_lock<std::mutex> ul(g_mu);
  std::string cfg_fn = argv[1];
  std::string wav_fn = argv[2];
  // logfile.open(GetLocalTime(), std::ios::out | std::ios::app);
  tlv_kaldi_dec_cfg_t* cfg = tlv_kaldi_dec_cfg_new(cfg_fn.c_str());
  char* version;
  tlv_kaldi_dec_cfg_get_version(cfg, &version);
  std::cout << version << std::endl;
  auto userdata = "12345";
  tlv_kaldi_vadec_t* dec = tlv_kaldi_vadec_new(cfg, vadec_callback, userdata);

  std::ifstream wav(wav_fn, std::ios::binary);
  std::vector<char> wav_content(static_cast<std::istreambuf_iterator<char>>(wav), {});

  int data_offset = 0;
  for(auto i = wav_content.begin(); i != wav_content.end(); i++)
  {
      if(*i == 'd' && *(i + 1) == 'a' && *(i + 2) == 't' && *(i + 3) == 'a')
      {
          break;
      }
      ++data_offset;
  }
  data_offset += 8;
  // data_offset = 0;
  char* wav_data = wav_content.data() + data_offset;
  int wav_len = static_cast<int>(wav_content.size()) - data_offset;
  // int wav_len = 32000 * 10;

  int step = 32000;
  int i = 0;
  int size = 0;
  VadecStream vs;
  vs.Init(0);//0=old params;1=new params
  // while (1) {
    while(i < wav_len) {
      // std::this_thread::sleep_for(std::chrono::milliseconds(30));
      size = step;
      tlv_kaldi_vadec_feed(dec, wav_data + i, size, 0, 0);
      i += size;
      if (i + size >= wav_len) {
        tlv_kaldi_vadec_feed(dec, wav_data + i, wav_len - i, 1, 1);
        break;
        // tlv_kaldi_vadec_feed(dec, 0, 0, 1, 1);
      }
    }
  //   i = 0;
  // }
  g_cv.wait(ul);
  tlv_kaldi_vadec_delete(dec);
  return 0;
}
