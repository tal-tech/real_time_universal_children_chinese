#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "tlv/kaldiext/feat/tlv_kdext_feat.h"
#include "feat/wave-reader.h"

#include "tlv/sphlib/pack/tlv_flist.h"
#include "tlv/toollib/cfg/tlv_main_cfg.h"


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
	char temp;
	wave_t wave;
	char* data = NULL;

	wave.fp = fopen(fn, "rb");
	if(wave.fp == NULL)
	{
		printf("open failed: %s\n", fn);
		return NULL;
	}

	do
	{
		ret = fread(&temp, sizeof(char), 1, wave.fp);
	}while('d' != temp);

	wave.data_flag[0] = 'd';
	if(3 != fread(&wave.data_flag[1], sizeof(uint8), 3, wave.fp))
	{
		printf("read header data error!\n");
		return NULL;
	}

	if(1 != fread(&wave.length, sizeof(uint32), 1, wave.fp))
	{
		printf("read header data length err!\n");
		return NULL;
	}


	if(0 != strncmp(wave.data_flag, "data", 4))
	{
		printf("error: cannot read data!\n");
		return NULL;
	}

	data = (char*)malloc(wave.length);
	ret = fread(data, sizeof(char), wave.length, wave.fp);
	if(ret != wave.length)
	{
		printf("error: cannot read enough data:%s!\n", fn);
		return NULL;
	}

	*len = wave.length;

	return data;
}

void print_usage()
{
	printf("Usage: -c cfg -scp scp,p:./fea_wav.scp -fea ark,t:fea.txt\n");
	exit(0);
}

int main(int argc, char* argv[])
{
	int ret = 0;
	tlv_arg_t *arg;
	char *cfg_fn = NULL;
	char *scp_fn = NULL;
	char *feat_fn = NULL;
	char *data, *p;
	int   len;

	tlv_main_cfg_t *main_cfg = NULL;
	tlv_kdext_feat_cfg_t *cfg;
	tlv_kdext_feat_t     *feat;
	tlv_charbuf_t* buf = tlv_charbuf_new(32, 1);

	if(argc < 3)
	{
		print_usage();
	}

	arg = tlv_arg_new(argc, argv);
	if(!arg) { print_usage(); }
	ret |= tlv_arg_get_str_s(arg, "c", &(cfg_fn));
	ret |= tlv_arg_get_str_s(arg, "scp", &(scp_fn));
	ret |= tlv_arg_get_str_s(arg, "fea", &(feat_fn));
	if(0 != ret) { print_usage(); }

	main_cfg = tlv_main_cfg_new_type(tlv_kdext_feat_cfg, cfg_fn);
	cfg = (tlv_kdext_feat_cfg_t*)main_cfg->cfg;
	if(!cfg)
	{
		printf("load res failed!\n");
		exit(0);
	}

	feat = tlv_kdext_feat_new(cfg);

	SequentialTableReader<WaveHolder> reader(scp_fn);
	BaseFloatMatrixWriter kaldi_writer;
	TableWriter<HtkMatrixHolder> htk_writer;

	if(!kaldi_writer.Open(feat_fn))
	{
		 KALDI_ERR << "Could not initialize output with wspecifier ";
		 print_usage();
	}

	for (; !reader.Done(); reader.Next()) {
		std::string utt = reader.Key();
		//const WaveData &wave_data = reader.Value();
		tlv_charbuf_reset(buf);
		tlv_charbuf_push_s(buf, "./.feat/wav/");
		tlv_charbuf_push(buf, utt.c_str(), utt.size());
		tlv_charbuf_push_c(buf, '\0');

//		data = p = file_read_buf(buf->data, &len);
//		if(NULL == data)
//		{
//			tlv_log("read wav file: %s failed!\n", buf->data);
//			continue;
//		}
//		data += 44;
//		len  -= 44;

		p = data = read_wav_file(buf->data, &len);
		if(data == NULL)
		{
			continue;
		}

		tlv_kdext_feat_feed(feat, data, len, 1);

		kaldi_writer.Write(utt, *feat->fea);

		tlv_kdext_feat_reset(feat);
		free(p);
	}


	if(feat) tlv_kdext_feat_delete(feat);
	if(main_cfg) tlv_main_cfg_delete(main_cfg);
	if(arg) tlv_arg_delete(arg);

	return 0;
}

