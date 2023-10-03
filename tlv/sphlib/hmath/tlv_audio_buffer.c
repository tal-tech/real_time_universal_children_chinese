#include "tlv_audio_buffer.h"

tlv_audio_buffer_t *tlv_audio_buffer_new(int size)
{
	tlv_audio_buffer_t *b;
	int t;

	t=tlv_round_word(sizeof(*b))+size*sizeof(float);
	b=(tlv_audio_buffer_t*)tlv_malloc(t);
	b->odd=0;
	b->rstart=b->cur=b->start=(float*)(((char*)b)+tlv_round_word((sizeof(*b))));
	b->end=b->rstart+size;
	return b;
}



int tlv_audio_buffer_push(tlv_audio_buffer_t *b, short* data, int samples)
{
	short* start = data;
	short* end = start+samples;

	while(start<end && b->cur<b->end)
	{
		*(b->cur++) = *(start++);
	}

	return start-data;
}

int tlv_audio_buffer_push_c(tlv_audio_buffer_t *b, char *data, int bytes)
{
	short odd;
	char *p;
	int cpy=0,left,samples;

	left = tlv_audio_buffer_left_samples(b);
	if(left<=0 || bytes<=0){goto end;}
	if(b->odd)
	{
		p=(char*)&odd;
		p[0]=b->odd_char;
		p[1]=data[0];
		bytes-=1;data+=1;
		b->odd=0;
		tlv_audio_buffer_push(b, &odd, 1);
		cpy+=1;
	}
	samples = bytes/2;
	left = tlv_audio_buffer_push(b,(short*)data,samples);
	cpy += left<<1;
	if((left==samples) && (bytes%2))
	{
		//pad odd data.
		b->odd_char = data[cpy];
		b->odd = 1;
		cpy += 1;
	}

end:
	return cpy;
}

int tlv_audio_buffer_peek(tlv_audio_buffer_t *b, tlv_vector_t *v, int is_end)
{
	int samples;
	int valid_len;
	int i;

	samples   = tlv_vector_size(v);
	valid_len = tlv_audio_buffer_valid_len(b);
	if(!is_end)
	{
		if(valid_len<samples)
		{
			return -1;
		}
		memcpy(&(v[1]),b->start,samples*sizeof(float));

		return 0;
	}else
	{
		memcpy(&(v[1]),b->start,valid_len*sizeof(float));
		for(i=valid_len+1;i<=samples;++i)
		{
			v[i]=0;
		}

		return 0;
	}
}

void tlv_audio_buffer_skip(tlv_audio_buffer_t *b, int samples, int left_enough)
{
	int size;

	b->start+=samples;
	if((b->end-b->start)<left_enough)
	{
		size=b->cur-b->start;
		memmove(b->rstart,b->start,size*sizeof(float));
		b->start=b->rstart;
		b->cur=b->start+size;
	}
}

float* tlv_audio_buffer_peek_data(tlv_audio_buffer_t *b, int samples)
{
	if(tlv_audio_buffer_valid_len(b) < samples)
	{
	    return 0;
	}

	return b->start;
}

int tlv_audio_buffer_delete(tlv_audio_buffer_t *b)
{
	tlv_free(b);

	return 0;
}

int tlv_audio_buffer_reset(tlv_audio_buffer_t *b)
{
	b->odd=0;
	b->rstart=b->cur=b->start=(float*)(((char*)b)+tlv_round_word((sizeof(*b))));

	return 0;
}
