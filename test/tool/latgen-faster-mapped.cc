/*
 * latgen-faster-mapped.cc
 *
 *  Created on: Dec 18, 2018
 *      Author: jfyuan
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "tlv/kaldiext/rec/tlv_kdext_rec.h"
#include "tlv/sphlib/pack/tlv_flist.h"
#include "tlv/toollib/cfg/tlv_main_cfg.h"

void print_usage()
{
	printf("Usage: -c cfg -l loglike.txt -r rslt.txt -lat lat.txt \n");
	exit(0);
}

int main(int argc, char* argv[])
{
	int ret = 0;
	tlv_arg_t *arg;
	char *cfg_fn = NULL;
	char *loglike_fn = NULL;
	char *rslt_fn = NULL;
	char *lat_fn = NULL;
	const char *rslt;

	tlv_main_cfg_t *main_cfg = NULL;
	tlv_kdext_rec_cfg_t* cfg;
	tlv_kdext_rec_t*     rec;

	if(argc < 4)
	{
		print_usage();
	}

	arg = tlv_arg_new(argc, argv);
	if(!arg) { print_usage(); }
	ret |= tlv_arg_get_str_s(arg, "c", &(cfg_fn));
	ret |= tlv_arg_get_str_s(arg, "l", &(loglike_fn));
	tlv_arg_get_str_s(arg, "r", &(rslt_fn));
	tlv_arg_get_str_s(arg, "lat", &(lat_fn));
	if(0 != ret) { print_usage(); }

	main_cfg = tlv_main_cfg_new_type(tlv_kdext_rec_cfg, cfg_fn);
	cfg = (tlv_kdext_rec_cfg_t*)main_cfg->cfg;
	if(!cfg)
	{
		printf("load res failed!\n");
		exit(0);
	}
	rec = tlv_kdext_rec_new(cfg);

	double tot_like = 0.0;
	kaldi::int64 frame_count = 0;
	int num_success = 0, num_fail = 0;
	SequentialBaseFloatMatrixReader loglike_reader(loglike_fn);
	for(; !loglike_reader.Done(); loglike_reader.Next())
	{
		std::string utt = loglike_reader.Key();
		Matrix<BaseFloat> loglikes (loglike_reader.Value());
		loglike_reader.FreeCurrent();
		if (loglikes.NumRows() == 0) {
			KALDI_WARN << "Zero-length utterance: " << utt;
			num_fail++;
			continue;
		}

		ret = tlv_kdext_rec_feed(rec, &loglikes);
		if(0 != ret)
		{
			num_fail++;
			continue;
		}

//		ret = tlv_kdext_rec_get_bestrslt(rec, &rslt);
//		if(0 == ret)
//		{
//			printf("%s: %s\n", utt.c_str(), rslt);
//		}
//		else
//		{
//			printf("%s: rec failed!\n", utt.c_str());
//		}
		rslt = tlv_kdext_rec_get_bestrslt(rec);
		if(rslt)
		{
			printf("%s: %s\n", utt.c_str(), rslt);
		}
		else
		{
			printf("%s: rec failed!\n", utt.c_str());
		}

		num_success++;
		tot_like += rec->like;
		frame_count += rec->frame_count;
		tlv_kdext_rec_reset(rec);
	}

	if(rec) tlv_kdext_rec_delete(rec);
	if(main_cfg) tlv_main_cfg_delete(main_cfg);
	if(arg) tlv_arg_delete(arg);

	return ret;
}
