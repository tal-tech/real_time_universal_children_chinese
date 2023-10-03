/*
 * multi-decoder.cc
 *
 *  Created on: Jan 23, 2019
 *      Author: jfyuan
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "tlv/sphlib/pack/tlv_flist.h"
#include "tlv/toollib/cfg/tlv_main_cfg.h"
#include "third/json/cJSON.h"
#include "include/tlv_kaldi_dec_interface.h"
#include <pthread.h>
#include <sys/syscall.h>
#include "tlv/kaldiext/tlv_kaldi_dec.h"
#include "feat/wave-reader.h"

#define MAXTHREAD 100

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

typedef struct ThreadRes
{
	char* wavscp;
	tlv_kaldi_dec_cfg_t* cfg;

	pthread_t pid[MAXTHREAD];
}thrdres_t;

pthread_mutex_t lock;
BaseFloatMatrixWriter kaldi_writer;

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

tlv_flist_t* read_scp(char *fn)
{
	tlv_flist_t *fl;

	fl=tlv_flist_new(fn);
	return fl;
}

pid_t gettid()
{
	return syscall(SYS_gettid);
}

int test_decoder_file(tlv_kaldi_dec_t* dec, char* wav_fn)
{
	int ret = 0;
	int len;
	char *data, *p, *rslt;
	//wave_t wave;

	if((p = strrchr(wav_fn, '#')) != NULL) { return 0; };

	if((p = strrchr(wav_fn, '\n')) != NULL) *p = '\0';
	if((p = strrchr(wav_fn, '\r')) != NULL) *p = '\0';
//	data = p = file_read_buf(wav_fn, &len);
//	if(data == NULL)
//	{
//		printf("open wav file failed: %s!\n", wav_fn);
//		goto end;
//	}
//
//	p += 44;
//	len -= 44;

	//pthread_mutex_lock(&lock);

	p = data = read_wav_file(wav_fn, &len);
	if(data == NULL)
	{
		return 0;
	}
	//pthread_mutex_unlock(&lock);


//#define USE_STEP 1
#ifdef USE_STEP
	{
		int step = 2 * 1024 + 1;
		int i = 0;
		int size;

		while(i < len)
		{
			size = step > (len-i) ? (len-i) : step;
			ret  = tlv_kaldi_dec_feed(dec, p+i, size, 0);
			if(0 != ret) { tlv_log("dec feed failed at: %d!\n", i); goto end; }
			i += size;

//			tlv_kaldi_dec_get_rslt(dec, &rslt, &size);
//			if(rslt) { printf("rec: %s\n", rslt); }
		}

		ret  = tlv_kaldi_dec_feed(dec, 0, 0, 1);
		if(0 != ret) { tlv_log("dec feed failed at end!\n"); goto end; }
	}
#else
	//pthread_mutex_lock(&lock);
	ret = tlv_kaldi_dec_feed(dec, p, len, 1);

	//printf("%p\n", dec->feat->fea);

//	tlv_charbuf_t *buf = tlv_charbuf_new(32,1);
//	tlv_charbuf_push_f(buf, "%d_%s", (long int)gettid(), strrchr(wav_fn, '/')+1);
//	tlv_charbuf_push_c(buf, '\0');
//
//	kaldi_writer.Write(buf->data, *(dec->feat->fea));
//
//	tlv_charbuf_delete(buf);
//	pthread_mutex_unlock(&lock);

	if(0 != ret) { goto end; }
#endif

	tlv_kaldi_dec_get_rslt(dec, &rslt, &len);
	//printf("netforward: %f ,rec: %f\n", dec->t_forward, dec->t_rec);

end:
	if(rslt)
	{
		printf("%ld %.*s ", (long int)gettid(), (int)(strlen(strrchr(wav_fn, '/') + 1) - 4), strrchr(wav_fn, '/')+1 );
		printf("%s\n", rslt);
		free(rslt);
	}

	tlv_kaldi_dec_reset(dec);
	free(data);

	return ret;
}

int test_decoder_scp(tlv_kaldi_dec_t* dec, tlv_flist_t* wav_fl)
{
	int ret = 0;
	tlv_fitem_t *item;
	tlv_queue_node_t *n;
	char* wav_fn;
	tlv_charbuf_t *buf = tlv_charbuf_new(32, 1);

	for(n=wav_fl->queue.rear; n; n=n->next)
	{
		item = data_offset(n, tlv_fitem_t, q_n);
		//wav_fn = item->str->data;

		tlv_charbuf_push(buf, item->str->data, item->str->len);
		tlv_charbuf_push_c(buf, '\0');
		wav_fn = buf->data;
		ret = test_decoder_file(dec, wav_fn);
		tlv_charbuf_reset(buf);
	}

	tlv_charbuf_delete(buf);
	return ret;
}

void* thread_run(void* arg)
{
	thrdres_t* res = (thrdres_t*)arg;
	tlv_kaldi_dec_cfg_t* cfg = res->cfg;
	tlv_kaldi_dec_t*     dec;
	tlv_flist_t*         wav_fl;
	char* wavscp_fn = res->wavscp;

	//tlv_kaldi_dec_cfg_t* cfg2;

	if( cfg == NULL )
	{
		printf("cfg is null！\n");
		return NULL;
	}

	//cfg2 = tlv_kaldi_dec_cfg_new("/home/jfyuan/work/hw-proj/project/kaldi-decode/res/cn.asr.Paizhk.Xgeneral.Ttch.V0.2.2/cfg");
	dec = tlv_kaldi_dec_new(cfg);
	wav_fl = read_scp(wavscp_fn);
	test_decoder_scp(dec, wav_fl);

	if(dec)    tlv_kaldi_dec_delete(dec);
	if(wav_fl) tlv_flist_delete(wav_fl);

	return NULL;
}

int test_thread(tlv_kaldi_dec_cfg_t* cfg, char* wavscp_fn, int num)
{
	int i;
	thrdres_t res;

	if(num > MAXTHREAD)
	{
		num = MAXTHREAD;
	}

	for(i=0; i < num; i++)
	{
//		tlv_charbuf_t* buf = tlv_charbuf_new(64, 1);
//		tlv_charbuf_push_f(buf,"%s%d", wavscp_fn, i);
//		tlv_charbuf_push_c(buf, '\0');

		res.cfg = cfg;
		res.wavscp = wavscp_fn;

		printf("thread[%d] start!\n", i);
		pthread_create(&(res.pid[i]), NULL, thread_run, &res);
		sleep(5);
	}

	for(i=0; i < num; i++)
	{
		pthread_join(res.pid[i], 0);
	}

	return 0;
}

void print_usage()
{
	printf("Usage: cfg scp num\n");
	exit(0);
}

int main(int argc, char* argv[])
{
	tlv_kaldi_dec_cfg_t* cfg = NULL;

	if(argc < 3)
	{
		print_usage();
	}

	pthread_mutex_init(&lock, 0);

	if(!kaldi_writer.Open("ark,t:test.feat"))
	{
			 KALDI_ERR << "Could not initialize output with wspecifier ";
			 exit(-1);
	}

	cfg = tlv_kaldi_dec_cfg_new(argv[1]);
	test_thread(cfg, argv[2], atoi(argv[3]));

	if(cfg) { tlv_kaldi_dec_cfg_delete(cfg); }
	pthread_mutex_destroy(&lock);

	return 0;
}
