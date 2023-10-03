/*
 * nnet-forward.cc
 *
 *  Created on: Dec 12, 2018
 *      Author: jfyuan
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#ifdef USE_NNET1
#include "tlv/kaldiext/forward/tlv_kdext_forward.h"
#include "tlv/sphlib/pack/tlv_flist.h"
#include "tlv/toollib/cfg/tlv_main_cfg.h"
#endif

void print_usage()
{
	printf("Usage: -c cfg -f feat -l logposteriors.txt \n");
	exit(0);
}

int main(int argc, char* argv[])
{
#ifdef USE_NNET1
	int ret = 0;
	tlv_arg_t *arg;
	char *cfg_fn = NULL;
	char *feats_fn = NULL;
	char *logposteriors_fn = NULL;

	tlv_main_cfg_t *main_cfg = NULL;
	tlv_kdext_forward_cfg_t* cfg;
	tlv_kdext_forward_t*     fw;

	if(argc < 3)
	{
		print_usage();
	}

	arg = tlv_arg_new(argc, argv);
	if(!arg) { print_usage(); }
	ret |= tlv_arg_get_str_s(arg, "c", &(cfg_fn));
	ret |= tlv_arg_get_str_s(arg, "f", &(feats_fn));
	ret |= tlv_arg_get_str_s(arg, "l", &(logposteriors_fn));
	if(0 != ret) { print_usage(); }

	main_cfg = tlv_main_cfg_new_type(tlv_kdext_forward_cfg, cfg_fn);
	cfg = (tlv_kdext_forward_cfg_t*)main_cfg->cfg;
	if(!cfg)
	{
		printf("load res failed!\n");
		exit(0);
	}
	fw = tlv_kdext_forward_new(cfg);

	SequentialBaseFloatMatrixReader feature_reader(feats_fn);
	BaseFloatMatrixWriter feature_writer(logposteriors_fn);

	for(; !feature_reader.Done(); feature_reader.Next())
	{
		 Matrix<BaseFloat> mat = feature_reader.Value();
		 std::string utt = feature_reader.Key();

		 ret |= tlv_kdext_forward_feed(fw, &mat);

		 feature_writer.Write(feature_reader.Key(), *(fw->nnet_out_host));

		 ret |= tlv_kdext_forward_reset(fw);
	}

	if(fw) tlv_kdext_forward_delete(fw);
	if(main_cfg) tlv_main_cfg_delete(main_cfg);
	if(arg) tlv_arg_delete(arg);

#endif

	return 0;
}


