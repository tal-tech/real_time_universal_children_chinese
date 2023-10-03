#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "tlv/sphlib/pack/tlv_flist.h"
#include "tlv/toollib/cfg/tlv_main_cfg.h"
#include "third/json/cJSON.h"
#include "include/tlv_kaldi_dec_interface.h"
#include <dirent.h>

#include "feat/wave-reader.h"
typedef struct WaveHeader
{
    uint8 riff[4];             //资源交换文件标志
    uint32 size;               //从下个地址开始到文件结尾的字节数
    uint8 wave_flag[4];        //wave文件标识
    uint8 fmt[4];              //波形格式标识
    uint32 fmt_len;            //过滤字节(一般为00000010H)
    uint16 tag;                //格式种类，值为1时，表示PCM线性编码
    uint16 channels;           //通道数，单声道为1，双声道为2
    uint32 samp_freq;          //采样频率
    uint32 byte_rate;          //数据传输率 (每秒字节＝采样频率×每个样本字节数)
    uint16 block_align;        //块对齐字节数 = channles * bit_samp / 8
    uint16 bit_samp;           //bits per sample
} wave_header_t;

typedef struct WaveStruct
{
    FILE *fp;                  //file pointer
    wave_header_t header;      //header
    char data_flag[4];        //数据标识符
    int length;             //采样数据总数
    char *pData;             //data
} wave_t;

char* read_wav_file(char* fn, int* len)
{
    int ret;
    char temp, temp2, temp3, temp4;
    wave_t wave;
    char* data = NULL;

    wave.fp = fopen(fn, "rb");
    if(wave.fp == NULL)
    {
        tlv_log("read file failed:%s!\n", fn);
        return NULL;
    }

    do
    {
        ret = fread(&temp, sizeof(char), 1, wave.fp);
        ret = fread(&temp2, sizeof(char), 1, wave.fp);
    }while(!('d' == temp && 'a' == temp2));

    ret = fread(&temp3, sizeof(char), 1, wave.fp);
    ret = fread(&temp4, sizeof(char), 1, wave.fp);
    if('t' == temp3 && 'a' == temp4)
    {

    }
    else
    {
        do
        {
            ret = fread(&temp, sizeof(char), 1, wave.fp);
            ret = fread(&temp2, sizeof(char), 1, wave.fp);
        }while(!('d' == temp && 'a' == temp2));

        ret = fread(&temp3, sizeof(char), 1, wave.fp);
        ret = fread(&temp4, sizeof(char), 1, wave.fp);
    }

    wave.data_flag[0] = 'd';
    wave.data_flag[1] = 'a';
    wave.data_flag[2] = 't';
    wave.data_flag[3] = 'a';
//	if(2 != fread(&wave.data_flag[2], sizeof(uint8), 2, wave.fp))
//	{
//		tlv_log("read header data error:%s!\n", fn);
//		return NULL;
//	}

    if(1 != fread(&wave.length, sizeof(uint32), 1, wave.fp))
    {
        tlv_log("read header data length err!\n");
        return NULL;
    }


    if(0 != strncmp(wave.data_flag, "data", 4))
    {
        tlv_log("error: cannot read data!\n");
        return NULL;
    }

    data = (char*)malloc(wave.length);
    ret = fread(data, sizeof(char), wave.length, wave.fp);
    if(ret != wave.length)
    {
        tlv_log("error: cannot read enough data:%s!\n", fn);
        return NULL;
    }

    *len = wave.length;

    if(wave.fp)
    {
        fclose(wave.fp);
    }

    return data;
}

void TestMemoryLeak(const std::string& cfg_fn, const std::string& wav_dir)
{
    // std::cout << cfg_fn << std::endl;
    // std::cout << wav_dir << std::endl;
    DIR *dir;
    std::vector<std::string> wav_files;
    struct dirent *ent;
    if ((dir = opendir (wav_dir.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if (static_cast<int>(ent->d_type) != 4) {
                wav_files.push_back(wav_dir + "/" + std::string(ent->d_name));
            }
            // printf ("%s\n", ent->d_name);
        }
        closedir (dir);
    } else {
        perror ("opendir failed.");
    }

    for(auto wf : wav_files) {
        std::cout << wf << std::endl;
    }

    tlv_kaldi_dec_cfg_t* cfg = tlv_kaldi_dec_cfg_new(cfg_fn.c_str());
    int file_index = 0;
    int test_times = 600;
    while(test_times > 0) {
        if (file_index > wav_files.size() - 1) {
            int i = 0;
            std::cin >> i;
            // file_index = 0;
        }
        else {
        }
        char *p, *data;
        p = data = nullptr;
        int len = 0;
        p = data = read_wav_file(const_cast<char*>(wav_files[file_index].c_str()), &len);
        tlv_kaldi_dec_t* dec = tlv_kaldi_dec_new(cfg);
        int ret = tlv_kaldi_dec_feed(dec, p, len, 1);
        if(ret != 0) {
            // delete instance
            tlv_kaldi_dec_delete(dec);
            free(data);
            continue;
        }
        char* rslt = nullptr;
        tlv_kaldi_dec_get_rslt(dec, &rslt, &len);
        if(rslt) {
            printf("%s\n", rslt);
            free(rslt);
        }
        tlv_kaldi_dec_delete(dec);
        free(data);
        file_index++;
        test_times--;
    }
    tlv_kaldi_dec_cfg_delete(cfg);
}

int main(int argc, char* argv[])
{
    const std::string cfg(argv[1]);
    const std::string wav_dir(argv[2]);
    TestMemoryLeak(cfg, wav_dir);
    return 0;
}
