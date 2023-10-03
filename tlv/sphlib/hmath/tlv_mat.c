#include "tlv_mat.h"

#define tlv_mati_prow_at(c,i,j) (*((c)->p+(i+(c)->p_row_x)*(c)->p_col+j+(c)->p_col_x))

tlv_matc_t* tlv_matc_new(int row, int col)
{
	tlv_matc_t *m;
	char *p;

	p          = tlv_malloc(sizeof(tlv_matc_t)+row*col*sizeof(char)+16);
	m          = (tlv_matc_t*)p;
	m->row     = row;
	m->col     = col;
	m->p_row   = row;
	m->p_col   = col;
	m->p_row_x = 0;
	m->p_col_x = 0;
	m->p = tlv_align_p(p+sizeof(tlv_matc_t),16);

	return m;
}

tlv_matc_t* tlv_matc_new2(tlv_matrix_t *m, float scale)
{
	tlv_matc_t *cm;
	int row,col;
	int i,j;
	signed char *px,*ppx;
	float *fp;
	float f;

	row = tlv_matrix_rows(m);
	col = tlv_matrix_cols(m);
	cm  = tlv_matc_new(row,col);
	for(px=cm->p,i=0;i<row;++i,px+=col)
	{
		for(ppx=px,fp=m[i+1],j=0;j<col;++j)
		{
			f=(*(++fp))*scale;
			*(ppx++)=(signed char)(tlv_float_round(f));
		}
	}

	return cm;
}

tlv_matc_t* tlv_matc_new3(tlv_matrix_t *m, float scale)
{
	tlv_matc_t *cm;
	int row,col;
	int i,j;
	signed char *p;
	float f;

	row=tlv_matrix_rows(m);
	col=tlv_matrix_cols(m);
	cm=tlv_matc_new(row,col);
	for(p=cm->p,i=1;i<=col;++i)
	{
		for(j=1;j<=row;++j)
		{
			f=m[j][i]*scale;
			*(p++)=(signed char)(tlv_float_round(f));
		}
	}

	return cm;
}

void tlv_matc_delete(tlv_matc_t *m)
{
	tlv_free(m);
}

void tlv_matc_print(tlv_matc_t *mc)
{
	int i,j;
	//char *px,*ppx;

	tlv_log("============= mi=%p ===========\n",mc);
	for(i=0;i<mc->row;++i)
	{
		for(j=0;j<mc->col;++j)
		{
			//printf("v[%d][%d]=%d\n",i,j,*(ppx++));
			printf("v[%d][%d]=%d\n",i,j,tlv_mati_prow_at(mc,i,j));
		}
	}
}

/*-------  matuc  ------*/
tlv_matuc_t* tlv_matuc_new(int row, int col)
{
	tlv_matuc_t *m;
	char *p;

	p          = tlv_malloc(sizeof(tlv_matuc_t)+row*col*sizeof(char));
	m          = (tlv_matuc_t*)p;
	m->row     = row;
	m->col     = col;
	m->p_row   = row;
	m->p_col   = col;
	m->p_row_x = 0;
	m->p_col_x = 0;
	m->p = (unsigned char*)(p+sizeof(tlv_matuc_t));

	return m;
}

void tlv_matuc_init(tlv_matuc_t* cm, tlv_matrix_t *m, float scale)
{
	int row,col;
	int i,j;
	unsigned char *px,*ppx;
	float *fp;
	float f;

	row=tlv_matrix_rows(m);
	col=tlv_matrix_cols(m);
	for(px=cm->p,i=0;i<row;++i,px+=col)
	{
		for(ppx=px,fp=m[i+1],j=0;j<col;++j)
		{
			f=(*(++fp))*scale;
			*(ppx++)=(unsigned char)(tlv_float_round(f));
		}
	}
}

void tlv_matuc_delete(tlv_matuc_t *mc)
{
	tlv_free(mc);
}

void tlv_matuc_print(tlv_matuc_t *mc)
{
	int i,j;
	//char *px,*ppx;

	tlv_log("============= mi=%p ===========\n",mc);
	for(i=0;i<mc->row;++i)
	{
		for(j=0;j<mc->col;++j)
		{
			//printf("v[%d][%d]=%d\n",i,j,*(ppx++));
			printf("v[%d][%d]=%d\n",i,j,tlv_mati_prow_at(mc,i,j));
		}
	}
}


tlv_mati_t* tlv_mati_new(int row,int col)
{
	tlv_mati_t *m;
	char *p;

	p          = tlv_malloc(sizeof(tlv_mati_t)+row*col*sizeof(int)+16);
	m          = (tlv_mati_t*)p;
	m->row     = row;
	m->col     = col;
	m->p_row   = row;
	m->p_col   = col;
	m->p_row_x = 0;
	m->p_col_x = 0;
	m->p=(int*)tlv_align_p((p+sizeof(tlv_mati_t)),16);

	return m;
}

void tlv_mati_init(tlv_mati_t *im, tlv_matrix_t *m, float scale)
{
	int row,col;
	int i,j;
	int *px,*ppx;
	float *fp;
	float f;

	row = tlv_matrix_rows(m);
	col = tlv_matrix_cols(m);
	for(px=im->p,i=0;i<row;++i,px+=col)
	{
		for(ppx=px,fp=m[i+1],j=0;j<col;++j)
		{
			f=(*(++fp))*scale;
			*(ppx++)=(int)(tlv_float_round(f));
		}
	}
}

tlv_mati_t* tlv_mati_new2(tlv_matrix_t *m,float scale)
{
	tlv_mati_t *im;

	im = tlv_mati_new(tlv_matrix_rows(m),tlv_matrix_cols(m));
	tlv_mati_init(im,m,scale);

	return im;
}

void tlv_mati_delete(tlv_mati_t *im)
{
	tlv_free(im);
}

void tlv_mati_scale(tlv_mati_t *mi, double scale)
{
	int n=mi->row*mi->col;
	int *p,*pe;
	float f;

	p=mi->p;
	pe=p+n;
	while(p<pe)
	{
		f=(*p)*scale;
		(*p)=tlv_float_round(f);
		++p;
	}
}

void tlv_mati_print(tlv_mati_t *mi)
{
	int i,j;
	//char *px,*ppx;

	tlv_log("============= mi=%p ===========\n",mi);
	for(i=0;i<mi->row;++i)
	{
		for(j=0;j<mi->col;++j)
		{
			//printf("v[%d][%d]=%d\n",i,j,*(ppx++));
			printf("v[%d][%d]=%d\n",i,j,tlv_mati_prow_at(mi,i,j));
		}
	}
}

#define MUL_X_b(j,epm,pm,N) \
if(j==0) \
{\
	while(epm-pm>=4) \
	{\
		*(pm++)=(*(pb++))<<N;\
		*(pm++)=(*(pb++))<<N;\
		*(pm++)=(*(pb++))<<N;\
		*(pm++)=(*(pb++))<<N;\
	} \
	while(epm>pm) \
	{\
		*(pm++)=(*(pb++))<<N;\
	}\
}else\
{\
	while(epm-pm>=4) \
	{\
		*(pm++)+=(*(pb++))<<N;\
		*(pm++)+=(*(pb++))<<N;\
		*(pm++)+=(*(pb++))<<N;\
		*(pm++)+=(*(pb++))<<N;\
	}\
	while(epm>pm)\
	{\
		*(pm++)+=(*(pb++))<<N;\
	}\
}


#define MUL_X3(j,epm,pm,N) \
if(j==0) \
{\
	while(epm-pm>=4) \
	{\
		t=*((int*)pb);\
		pb+=4;\
		*(pm++)=((char)((t>>8) & 0x00FF))<<N; \
		*(pm++)=((char)((t>>16) & 0x00FF))<<N; \
		*(pm++)=((char)((t>>24) & 0x00FF))<<N; \
		*(pm++)=((char)((t>>24) & 0x00FF))<<N; \
	} \
	while(epm>pm) \
	{\
		*(pm++)=(*(pb++))<<N;\
	}\
}else\
{\
	while(epm-pm>=4) \
	{\
		t=*((int*)pb);\
		pb+=4;\
		*(pm++)+=((char)(t & 0x00FF))<<N; \
		*(pm++)+=((char)((t>>8) & 0x00FF))<<N; \
		*(pm++)+=((char)((t>>16) & 0x00FF))<<N; \
		*(pm++)+=((char)((t>>24) & 0x00FF))<<N; \
	}\
	while(epm>pm)\
	{\
		*(pm++)+=(*(pb++))<<N;\
	}\
}

#define MUL_XT(j,epm,pm,N) \
if(j==0) \
{\
	nx1=8-N;\
	nx2=16-N;\
	nx3=24-N;\
	while(epm-pm>=4) \
	{\
		t=*((int*)pb);\
		pb+=4;\
		*(pm++)=(char)((t&0x00FF)<<N);\
		*(pm++)=(char)((t&0x00FF00)<<nx1);\
		*(pm++)=(char)((t&0x00FF0000)<<nx2);\
		*(pm++)=(char)((t&0x00FF000000)<<nx3);\
	} \
	while(epm>pm) \
	{\
		*(pm++)=(*(pb++))<<N;\
	}\
}else\
{\
	nx1=8-N;\
	nx2=16-N;\
	nx3=24-N;\
	while(epm-pm>=4) \
	{\
		t=*((int*)pb);\
		pb+=4;\
		*(pm++)+=(char)((t&0x00FF)<<N);\
		*(pm++)+=(char)((t&0x00FF00)<<nx1);\
		*(pm++)+=(char)((t&0x00FF0000)<<nx2);\
		*(pm++)+=(char)((t&0x00FF000000)<<nx3);\
	}\
	while(epm>pm)\
	{\
		*(pm++)+=(*(pb++))<<N;\
	}\
}

#define MUL_X(j,epm,pm,N) \
if(j==0) \
{\
	while(epm-pm>=4) \
	{\
		*(pm++)=(*(pb++))<<N;\
		*(pm++)=(*(pb++))<<N;\
		*(pm++)=(*(pb++))<<N;\
		*(pm++)=(*(pb++))<<N;\
	} \
	while(epm>pm) \
	{\
		*(pm++)=(*(pb++))<<N;\
	}\
}else\
{\
	while(epm-pm>=4) \
	{\
		*(pm++)+=(*(pb++))<<N;\
		*(pm++)+=(*(pb++))<<N;\
		*(pm++)+=(*(pb++))<<N;\
		*(pm++)+=(*(pb++))<<N;\
	}\
	while(epm>pm)\
	{\
		*(pm++)+=(*(pb++))<<N;\
	}\
}

char* tlv_mati_xxx_2(int j,register int pak,register int *pm,register int *epm,register char *pb)
{
	register int t,t1;

	if(j==0)
	{
		while(epm-pm>=4)
		{
			t=*((int*)pb);
			t1=pak*(t&0x00FF00FF);

			pm[0]=t1&0x00FFFF;
			pm[2]=(t1&0xFFFF000)>>16;

			t1=pak*((t>>8)&0x00FF00FF);
			pm[1]=t1&0x00FFFF;
			pm[3]=(t1&0xFFFF000)>>16;

			pb+=4;
			pm+=4;
			/*
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*/
		}
		while(epm>pm)
		{
			*(pm++)=pak*(*(pb++));
		}
	}else
	{
		while(epm-pm>=4)
		{
			t=*((int*)pb);
			t1=pak*(t&0x00FF00FF);

			pm[0]=t1&0x00FFFF;
			pm[2]=(t1&0xFFFF000)>>16;

			t1=pak*((t>>8)&0x00FF00FF);
			t1=pak*t;
			pm[1]=t1&0x00FFFF;
			pm[3]=(t1&0xFFFF000)>>16;

			pb+=4;
			pm+=4;
			/*
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));*/
		}
		while(epm>pm)
		{
			*(pm++)+=pak*(*(pb++));
		}
	}
	return pb;
}

char* tlv_mati_xxx2(int j,register int pak,register int *pm,register int *epm,register char *pb)
{
	if(j==0)
	{
		while(epm-pm>=8)
		{
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));

			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
		}
		while(epm-pm>=4)
		{
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
		}
		while(epm>pm)
		{
			*(pm++)=pak*(*(pb++));
		}
	}else
	{
		while(epm-pm>=8)
		{
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));

			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
		}
		while(epm-pm>=4)
		{
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
		}
		while(epm>pm)
		{
			*(pm++)+=pak*(*(pb++));
		}
	}
	return pb;
}

long int mat_mul_x(int pak,int *pm,int *epm,char *pb)
{
	register int t;

	while(epm-pm>=4)
	{
		t=*((short*)pb);
		t=((t&0x00FF)+( t&0x00FF00<<16))*pak;
		*(pm++)=t&0x00FFFF;
		*(pm++)=(t>>16)&0x00FFFF;

		t=*((short*)pb);
		t=((t&0x00FF)+( t&0x00FF00<<16))*pak;
		*(pm++)=t&0x00FFFF;
		*(pm++)=(t>>16)&0x00FFFF;
	}
	while(epm>pm)
	{
		*(pm++)=pak*(*(pb++));
	}
	return 0;
}

char* tlv_mati_xxx(int j,register int pak,int *pm,int *epm,char *pb)
{
//#define USE_PROFILE
#ifdef USE_PROFILE
	//int t;
#endif

	//tlv_log("pm=%p epm=%p\n",pm,epm);
	//exit(0);
	if(j==0)
	{
		while(epm-pm>=4)
		{
#ifdef USE_PROFILE
			*pm=pak*(*pb);
			pm[1]=pak*(pb[1]);
			pm[2]=pak*(pb[2]);
			pm[3]=pak*(pb[3]);

			/*
			t=pak*(*((int*)pb));
			pm[0]=t;
			pm[1]=t;
			pm[2]=t;
			pm[3]=t;*/
			/*
			*(pm++)=t;
			*(pm++)=t;
			*(pm++)=t;
			*(pm++)=t;
			*(pm++)=t;*/
			pb+=4;
			pm+=4;
#else
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
			*(pm++)=pak*(*(pb++));
#endif
		}
		while(epm>pm)
		{
			*(pm++)=pak*(*(pb++));
		}
	}else
	{
		while(epm-pm>=4)
		{
#ifdef USE_PROFILE
			*pm+=pak*(*pb);
			pm[1]+=pak*(pb[1]);
			pm[2]+=pak*(pb[2]);
			pm[3]+=pak*(pb[3]);

			/*
			t=pak*(*((int*)pb));
			pm[0]+=t;
			pm[1]+=t;
			pm[2]+=t;
			pm[3]+=t;*/
			/*
			*(pm++)+=t;
			*(pm++)+=t;
			*(pm++)+=t;
			*(pm++)+=t;
			*(pm++)=t;*/
			pb+=4;
			pm+=4;
#else
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
			*(pm++)+=pak*(*(pb++));
#endif
		}
		while(epm>pm)
		{
			*(pm++)+=pak*(*(pb++));
		}
	}
	return pb;
}

/**
 * |2*245|*|245*128|=|2*128|
 */
void tlv_mati_multi_x2(tlv_mati_t *m,tlv_matuc_t *a,tlv_matc_t *b)
{
	unsigned char *pa;
	register signed char *pb;
	register int pak;
	register int *pm,*epm;
	int *tpm;
	int i,j;
	//register int t;
	//int nx1,nx2,nx3;

	//tlv_log("a=[%d*%d] b=[%d*%d]\n",a->row,a->col,b->row,b->col);
	for(tpm=m->p,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);//pa[j];
			//tlv_log("pak=%d\n",pak);
			pm=tpm;
			epm=pm+b->col;
			if(j==0)
			{
				while(epm>pm)
				{
					*(pm++)=pak*(*(pb++));
				}
			}else
			{
				while(epm>pm)
				{
					*(pm++)+=pak*(*(pb++));
				}
			}
		}
	}
}

#ifdef USE_SIMD
#include <arm_neon.h>

void tlv_mati_multi2(tlv_mati_t *m,tlv_mati_t *a,tlv_matc_t *b)
{
	int *pa;
	register signed char *pb,*epb;
	register int pak;
	register int *pm;
	int *tpm;
	int i,j;
	int32x4_t fa,fb,fc;
	int buf[4];

	for(tpm=m->p,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);//pa[j];
			epb=pb+b->col;
			pm=tpm;
			fa=vdupq_n_s32(pak);
			if(j==0)
			{
				while(epb-pb>=4)
				{
					buf[0]=pb[0];
					buf[1]=pb[1];
					buf[2]=pb[2];
					buf[3]=pb[3];
					fb=vld1q_s32(buf);
					fc=vmulq_s32(fa,fb);
					vst1q_s32(pm,fc);
					pm+=4;
					pb+=4;
				}
				while(epb>pb)
				{
					*(pm++)=pak*(*(pb++));
				}
			}else
			{
				while(epb-pb>=4)
				{
					buf[0]=pb[0];
					buf[1]=pb[1];
					buf[2]=pb[2];
					buf[3]=pb[3];
					fb=vld1q_s32(buf);
					fc=vld1q_s32(pm);
					fc=vmlaq_s32(fc,fa,fb);
					vst1q_s32(pm,fc);
					pm+=4;
					pb+=4;
				}
				while(epb>pb)
				{
					*(pm++)+=pak*(*(pb++));
				}
			}
		}
	}
}

void tlv_mati_multi(tlv_mati_t *m,tlv_matuc_t *a,tlv_matc_t *b)
{
	unsigned char *pa;
	register signed char *pb;
	register int pak;
	register int *pm,*epm;
	int *tpm;
	int i,j;
	//short buf[8];
	int16x8_t fa,fb,fc;
	short buf[8];

	//tlv_log("a=[%d*%d] b=[%d*%d]\n",a->row,a->col,b->row,b->col);
	for(tpm=m->p,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);//pa[j];
			//tlv_log("pak=%d\n",pak);
			pm=tpm;
			epm=pm+b->col;
			fa=vdupq_n_s16(pak);
			if(j==0)
			{
				while(epm-pm>=8)
				{
					fb=vmovl_s8(vld1_s8(pb));
					fc=vmulq_s16(fa,fb);
					vst1q_s16(buf,fc);
					*(pm++)=buf[0];
					*(pm++)=buf[1];
					*(pm++)=buf[2];
					*(pm++)=buf[3];
					*(pm++)=buf[4];
					*(pm++)=buf[5];
					*(pm++)=buf[6];
					*(pm++)=buf[7];
					pb+=8;
				}
				while(epm-pm>=4)
				{
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
				}
				while(epm>pm)
				{
					*(pm++)=pak*(*(pb++));
				}
			}else
			{
				while(epm-pm>=8)
				{
					fb=vmovl_s8(vld1_s8(pb));
					fc=vmulq_s16(fa,fb);
					vst1q_s16(buf,fc);
					*(pm++)+=buf[0];
					*(pm++)+=buf[1];
					*(pm++)+=buf[2];
					*(pm++)+=buf[3];
					*(pm++)+=buf[4];
					*(pm++)+=buf[5];
					*(pm++)+=buf[6];
					*(pm++)+=buf[7];
					pb+=8;
				}
				while(epm-pm>=4)
				{
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
				}
				while(epm>pm)
				{
					*(pm++)+=pak*(*(pb++));
				}
			}
		}
	}
}

#else

void tlv_mati_multi(tlv_mati_t *m,tlv_matuc_t *a,tlv_matc_t *b)
{
	unsigned char *pa;
	register signed char *pb;
	register int pak;
	register int *pm,*epm;
	int *tpm;
	int i,j;
	//register int t;
	//int nx1,nx2,nx3;

	//tlv_log("a=[%d*%d] b=[%d*%d]\n",a->row,a->col,b->row,b->col);
	for(tpm=m->p,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);//pa[j];
			//tlv_log("pak=%d\n",pak);
			pm=tpm;
			epm=pm+b->col;
			switch(pak)
			{
			case 0:
				if(j==0)
				{
					memset(pm,0,b->col<<2);
				}
				pb+=b->col;
				break;
			case 1:
				if(j==0)
				{
					while(epm-pm>=4)
					{
						*(pm++)=(*(pb++));
						*(pm++)=(*(pb++));
						*(pm++)=(*(pb++));
						*(pm++)=(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)=(*(pb++));
					}
				}else
				{
					while(epm-pm>=4)
					{
						*(pm++)+=(*(pb++));
						*(pm++)+=(*(pb++));
						*(pm++)+=(*(pb++));
						*(pm++)+=(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)+=(*(pb++));
					}
				}
				break;
			case 2:
				MUL_X(j,epm,pm,1);
				break;
			case 4:
				MUL_X(j,epm,pm,2);
				break;
			case 8:
				MUL_X(j,epm,pm,3);
				break;
			case 16:
				MUL_X(j,epm,pm,4);
				break;
			case 32:
				MUL_X(j,epm,pm,5);
				break;
			case 64:
				MUL_X(j,epm,pm,6);
				break;
			case 128:
				MUL_X(j,epm,pm,7);
				break;
			default:
				//pb=tlv_mati_xxx(j,pak,pm,epm,pb);
				if(j==0)
				{
					while(epm-pm>=4)
					{
						*(pm++)=pak*(*(pb++));
						*(pm++)=pak*(*(pb++));
						*(pm++)=pak*(*(pb++));
						*(pm++)=pak*(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)=pak*(*(pb++));
					}
				}else
				{
					while(epm-pm>=4)
					{
						*(pm++)+=pak*(*(pb++));
						*(pm++)+=pak*(*(pb++));
						*(pm++)+=pak*(*(pb++));
						*(pm++)+=pak*(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)+=pak*(*(pb++));
					}
				}
				break;
			}
		}
	}
}


void tlv_mati_multi2(tlv_mati_t *m,tlv_mati_t *a,tlv_matc_t *b)
{
	int *pa;
	register signed char *pb;
	register int pak;
	register int *pm,*epm;
	int *tpm;
	int i,j;

	//tlv_log("a=[%d*%d] b=[%d*%d]\n",a->row,a->col,b->row,b->col);
	for(tpm=m->p,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);//pa[j];
			//tlv_log("pak=%d\n",pak);
			pm=tpm;
			switch(pak)
			{
			case 0:
				if(j==0)
				{
					memset(pm,0,b->col<<2);
				}
				pb+=b->col;
				break;
			case 1:
				epm=pm+b->col;
				if(j==0)
				{
					while(epm-pm>=4)
					{
						*(pm++)=(*(pb++));
						*(pm++)=(*(pb++));
						*(pm++)=(*(pb++));
						*(pm++)=(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)=(*(pb++));
					}
				}else
				{
					while(epm-pm>=4)
					{
						*(pm++)+=(*(pb++));
						*(pm++)+=(*(pb++));
						*(pm++)+=(*(pb++));
						*(pm++)+=(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)+=(*(pb++));
					}
				}
				break;
			default:
				epm=pm+b->col;
				if(j==0)
				{
					//pb=tlv_mati_mx(pm,epm,pb,pak);
					while(epm-pm>=4)
					{
						*(pm++)=pak*(*(pb++));
						*(pm++)=pak*(*(pb++));
						*(pm++)=pak*(*(pb++));
						*(pm++)=pak*(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)=pak*(*(pb++));
					}
				}else
				{
					//pb=tlv_mati_mx2(pm,epm,pb,pak);
					while(epm-pm>=4)
					{
						*(pm++)+=pak*(*(pb++));
						*(pm++)+=pak*(*(pb++));
						*(pm++)+=pak*(*(pb++));
						*(pm++)+=pak*(*(pb++));
					}
					while(epm>pm)
					{
						*(pm++)+=pak*(*(pb++));
					}
				}
				break;
			}
		}
	}
}

#endif

void tlv_mati_multi3(tlv_mati_t *m,tlv_mati_t *a,tlv_mati_t *b)
{
	int *pa;
	register int *pb,*epb;
	register int pak;
	register int *pm;
	int *tpm;
	int i,j;

	for(tpm=m->p,pa=a->p,i=0;i<a->row;++i,tpm+=m->col)
	{
		for(pb=b->p,j=0;j<a->col;++j)
		{
			pak=*(pa++);//pa[j];
			epb=pb+b->col;
			pm=tpm;
			if(j==0)
			{
				while(epb-pb>=4)
				{
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
				}
				while(epb>pb)
				{
					*(pm++)=pak*(*(pb++));
				}
			}else
			{
				while(epb-pb>=4)
				{
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
				}
				while(epb>pb)
				{
					*(pm++)+=pak*(*(pb++));
				}
			}
		}
	}
}

void tlv_mati_add(tlv_mati_t *a,tlv_mati_t *b)
{
	register int *pa,*epa,*pb;

	pa=a->p;
	pb=b->p;
	epa=pa+a->row*a->col;
	while(epa-pa>=4)
	{
		*(pa++)+=*(pb++);
		*(pa++)+=*(pb++);
		*(pa++)+=*(pb++);
		*(pa++)+=*(pb++);
	}
	while(epa>pa)
	{
		*(pa++)+=*(pb++);
	}
}


void tlv_mati_init_rc(tlv_mati_t *b,tlv_mati_t *a,int row_x,int col_x,int row,int col)
{
	b->p=a->p;
	b->row=row;
	b->col=col;
	b->p_row=a->p_row;
	b->p_col=a->p_col;
	b->p_row_x=a->p_row_x+row_x;
	b->p_col_x=a->p_col_x+col_x;
}

void tlv_matc_init_rc(tlv_matc_t *b,tlv_matc_t *a,int row_x,int col_x,int row,int col)
{
	b->p=a->p;
	b->row=row;
	b->col=col;
	b->p_row=a->p_row;
	b->p_col=a->p_col;
	b->p_row_x=a->p_row_x+row_x;
	b->p_col_x=a->p_col_x+col_x;
}

void tlv_mati_prow_sub2(tlv_mati_t *c,tlv_matc_t *a,tlv_matc_t *b)
{
	int i,j;

	for(i=0;i<a->row;++i)
	{
		for(j=0;j<a->col;++j)
		{
			tlv_mati_prow_at(c,i,j)=tlv_mati_prow_at(a,i,j)-tlv_mati_prow_at(b,i,j);
		}
	}
}

void tlv_mati_prow_sub(tlv_mati_t *c,tlv_mati_t *a,tlv_mati_t *b)
{
	int i;
	int *pci;
	int *pai;
	int *pbi;
	register int *paij,*pbij,*pcij,*pae;

	pci=c->p+c->p_row_x*c->p_col+c->p_col_x;
	pai=a->p+a->p_row_x*a->p_col+a->p_col_x;
	pbi=b->p+b->p_row_x*b->p_col+b->p_col_x;
	for(i=0;i<a->row;++i,pci+=c->p_col,pai+=a->p_col,pbi+=b->p_col)
	{
		pcij=pci;
		paij=pai;
		pbij=pbi;
		pae=pai+a->col;
		//for(j=0;j<a->col;++j)
		while(paij+4<=pae)
		{
			*(pcij++)=*(paij++)-*(pbij++);
			*(pcij++)=*(paij++)-*(pbij++);
			*(pcij++)=*(paij++)-*(pbij++);
			*(pcij++)=*(paij++)-*(pbij++);
		}
		while(paij<pae)
		{
			*(pcij++)=*(paij++)-*(pbij++);
		}
	}
}


void tlv_mati_prow_add(tlv_mati_t *c,tlv_mati_t *a,tlv_mati_t *b)
{
	int i;
	int *pci;
	int *pai;
	int *pbi;
	register int *paij,*pbij,*pcij,*pae;

	if(c->p_row_x>0)
	{
		pci=c->p+c->p_row_x*c->p_col+c->p_col_x;
	}else
	{
		pci=c->p+c->p_col_x;
	}
	if(a->p_row_x>0)
	{
		pai=a->p+a->p_row_x*a->p_col+a->p_col_x;
	}else
	{
		pai=a->p+a->p_col_x;
	}
	if(b->p_row_x>0)
	{
		pbi=b->p+b->p_row_x*b->p_col+b->p_col_x;
	}else
	{
		pbi=b->p+b->p_col_x;
	}
	for(i=0;i<a->row;++i,pci+=c->p_col,pai+=a->p_col,pbi+=b->p_col)
	{
		pcij=pci;
		paij=pai;
		pbij=pbi;
		pae=pai+a->col;
		//for(j=0;j<a->col;++j)
		while(paij+4<=pae)
		{
			*(pcij++)=*(paij++)+*(pbij++);
			*(pcij++)=*(paij++)+*(pbij++);
			*(pcij++)=*(paij++)+*(pbij++);
			*(pcij++)=*(paij++)+*(pbij++);
		}
		while(paij<pae)
		{
			*(pcij++)=*(paij++)+*(pbij++);
		}
	}
}

void tlv_mati_prow_add2(tlv_mati_t *c,tlv_matc_t *a,tlv_matc_t *b)
{
	int i,j;

	for(i=0;i<a->row;++i)
	{
		for(j=0;j<a->col;++j)
		{
			tlv_mati_prow_at(c,i,j)=tlv_mati_prow_at(a,i,j)+tlv_mati_prow_at(b,i,j);
		}
	}
}

void tlv_mati_multi_c0(tlv_mati_t *c,tlv_mati_t *m1,tlv_mati_t *m4,tlv_mati_t *m5,tlv_mati_t *m7)
{
	int i,j;

	for(i=0;i<c->row;++i)
	{
		for(j=0;j<c->col;++j)
		{
			tlv_mati_prow_at(c,i,j)=tlv_mati_prow_at(m1,i,j)+tlv_mati_prow_at(m4,i,j)
					-tlv_mati_prow_at(m5,i,j)+tlv_mati_prow_at(m7,i,j);
		}
	}
}

void tlv_mati_multi_c3(tlv_mati_t *c,tlv_mati_t *m1,tlv_mati_t *m2,tlv_mati_t *m3,tlv_mati_t *m6)
{
	int i,j;

	for(i=0;i<c->row;++i)
	{
		for(j=0;j<c->col;++j)
		{
			tlv_mati_prow_at(c,i,j)=tlv_mati_prow_at(m1,i,j)-tlv_mati_prow_at(m2,i,j)
					+tlv_mati_prow_at(m3,i,j)+tlv_mati_prow_at(m6,i,j);
		}
	}
}

void tlv_mati_multi_prow(tlv_mati_t *m,tlv_mati_t *a,tlv_matc_t *b)
{
	int *pa;
	register signed char *pb;
	register int pak;
	register int *pm,*epm;
	int *tpm;
	int i,j;
	//int nx;

	//nx=b->col*sizeof(int);
	for(i=0;i<a->row;++i)
	{
		tpm=m->p+(i+m->p_row_x)*m->p_col+m->p_col_x;
		pa=a->p+(i+a->p_row_x)*a->p_col+a->p_col_x;
		for(j=0;j<a->col;++j)
		{
			pak=*(pa++);//pa[j];
			//a[i][j] b[j][k]
			pm=tpm;
			epm=pm+b->col;
			pb=b->p+(j+b->p_row_x)*b->p_col+b->p_col_x;
			if(j==0)
			{
				while(epm-pm>=4)
				{
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
				}
				while(epm>pm)
				{
					*(pm++)=pak*(*(pb++));
				}
			}else
			{
				while(epm-pm>=4)
				{
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
				}
				while(epm>pm)
				{
					*(pm++)+=pak*(*(pb++));
				}
			}
		}
	}
}

void tlv_mati_multi_prow_2(tlv_mati_t *m,tlv_mati_t *a,tlv_mati_t *b)
{
	register int *pb;
	register int pak;
	register int *pm,*epm;
	int *pa;
	int *tpm;
	int i,j;
	//int nx;
	int *tpa;
	int *tpb,*tpbx;

	tpm=m->p+(m->p_row_x)*m->p_col+m->p_col_x;
	tpa=a->p+(a->p_row_x)*a->p_col+a->p_col_x;
	tpb=b->p+(b->p_row_x)*b->p_col+b->p_col_x;
	for(i=0;i<a->row;++i,tpm+=m->p_col,tpa+=a->p_col)
	{
		pa=tpa;//a->p+(i+a->p_row_x)*a->p_col+a->p_col_x;
		for(tpbx=tpb,j=0;j<a->col;++j,tpbx+=b->p_col)
		{
			pak=*(pa++);//pa[j];
			//a[i][j] b[j][k]
			pm=tpm;
			epm=pm+b->col;
			pb=tpbx;//b->p+(j+b->p_row_x)*b->p_col+b->p_col_x;
			if(j==0)
			{
				while(epm-pm>=4)
				{
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
					*(pm++)=pak*(*(pb++));
				}
				while(epm>pm)
				{
					*(pm++)=pak*(*(pb++));
				}
			}else
			{
				while(epm-pm>=4)
				{
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
					*(pm++)+=pak*(*(pb++));
				}
				while(epm>pm)
				{
					*(pm++)+=pak*(*(pb++));
				}
			}
		}
	}
}


void tlv_mati_multi_dc2(tlv_mati_t *c,tlv_mati_t *a,tlv_mati_t *b,tlv_mati_heap_t *heap,int NX)
{
	tlv_mati_t a0,a1,a2,a3;
	tlv_mati_t b0,b1,b2,b3;
	tlv_mati_t c0,c1,c2,c3;
	int min_a,min_b,min_c;
	tlv_mati_t *t1;
	tlv_mati_t *t2;
	tlv_mati_t *m1,*m2,*m3,*m4,*m5,*m6,*m7;

	if(a->row>=NX && a->col>=NX && b->col>=NX)
	{
		//a*b b*c;
		min_a=min(a->row,a->col);
		min_b=min(b->row,b->col);
		min_c=min(min_a,min_b);
		min_c=min_c&0xFFFE;
		if(a->row==min_c && a->row==a->col && b->row==b->col)
		{
			/*
				M1 = (A0 + A3) × (B0 + B3)
				M2 = (A2 + A3) × B0
				M3 = A0 × (B1 - B3)
				M4 = A3 × (B2 - B0)
				M5 = (A0 + A1) × B3
				M6 = (A2 - A0) × (B0 + B1)
				M7 = (A1 - A3) × (B2 + B3)
				C0 = M1 + M4 - M5 + M7
				C1 = M3 + M5
				C2 = M2 + M4
				C3 = M1 - M2 + M3 + M6
			 */
			min_c=min_c>>1;
			tlv_mati_init_rc(&(a0),a,0,0,min_c,min_c);
			tlv_mati_init_rc(&(a1),a,0,min_c,min_c,a->col-min_c);
			tlv_mati_init_rc(&(a2),a,min_c,0,a->row-min_c,min_c);
			tlv_mati_init_rc(&(a3),a,min_c,min_c,a->row-min_c,a->col-min_c);

			tlv_mati_init_rc(&(b0),b,0,0,min_c,min_c);
			tlv_mati_init_rc(&(b1),b,0,min_c,min_c,b->col-min_c);
			tlv_mati_init_rc(&(b2),b,min_c,0,b->row-min_c,min_c);
			tlv_mati_init_rc(&(b3),b,min_c,min_c,b->row-min_c,b->col-min_c);

			tlv_mati_init_rc(&(c0),c,0,0,min_c,min_c);
			tlv_mati_init_rc(&(c1),c,0,min_c,min_c,c->col-min_c);
			tlv_mati_init_rc(&(c2),c,min_c,0,c->row-min_c,min_c);
			tlv_mati_init_rc(&(c3),c,min_c,min_c,c->row-min_c,c->col-min_c);

			t1=heap->mat_new(heap->ths,min_c,min_c);
			t2=heap->mat_new(heap->ths,min_c,min_c);
			m1=heap->mat_new(heap->ths,min_c,min_c);
			m2=heap->mat_new(heap->ths,min_c,min_c);
			m3=heap->mat_new(heap->ths,min_c,min_c);
			m4=heap->mat_new(heap->ths,min_c,min_c);
			m5=heap->mat_new(heap->ths,min_c,min_c);
			m6=heap->mat_new(heap->ths,min_c,min_c);
			m7=heap->mat_new(heap->ths,min_c,min_c);

			tlv_mati_prow_add(t1,&(a0),&(a3));
			tlv_mati_prow_add(t2,&(b0),&(b3));
			tlv_mati_multi_dc2(m1,t1,t2,heap,NX);

			tlv_mati_prow_add(t1,&(a2),&(a3));
			tlv_mati_multi_dc2(m2,t1,&(b0),heap,NX);

			tlv_mati_prow_sub(t1,&(b1),&(b3));
			tlv_mati_multi_dc2(m3,&(a0),t1,heap,NX);

			tlv_mati_prow_sub(t1,&(b2),&(b0));
			tlv_mati_multi_dc2(m4,&(a3),t1,heap,NX);

			tlv_mati_prow_add(t1,&(a0),&(a1));
			tlv_mati_multi_dc2(m5,t1,&(b3),heap,NX);

			tlv_mati_prow_sub(t1,&(a2),&(a0));
			tlv_mati_prow_add(t2,&(b0),&(b1));
			tlv_mati_multi_dc2(m6,t1,t2,heap,NX);

			tlv_mati_prow_sub(t1,&(a1),&(a3));
			tlv_mati_prow_add(t2,&(b2),&(b3));
			tlv_mati_multi_dc2(m7,t1,t2,heap,NX);

			//tlv_mati_print(m7);
			tlv_mati_multi_c0(&(c0),m1,m4,m5,m7);
			tlv_mati_prow_add(&(c1),m3,m5);
			tlv_mati_prow_add(&(c2),m2,m4);
			tlv_mati_multi_c3(&(c3),m1,m2,m3,m6);

			heap->mat_delete(heap->ths,t1);
			heap->mat_delete(heap->ths,t2);
			heap->mat_delete(heap->ths,m1);
			heap->mat_delete(heap->ths,m2);
			heap->mat_delete(heap->ths,m3);
			heap->mat_delete(heap->ths,m4);
			heap->mat_delete(heap->ths,m5);
			heap->mat_delete(heap->ths,m6);
			heap->mat_delete(heap->ths,m7);
		}else
		{
			tlv_mati_init_rc(&(a0),a,0,0,min_c,min_c);
			tlv_mati_init_rc(&(a1),a,0,min_c,min_c,a->col-min_c);
			tlv_mati_init_rc(&(a2),a,min_c,0,a->row-min_c,min_c);
			tlv_mati_init_rc(&(a3),a,min_c,min_c,a->row-min_c,a->col-min_c);

			tlv_mati_init_rc(&(b0),b,0,0,min_c,min_c);
			tlv_mati_init_rc(&(b1),b,0,min_c,min_c,b->col-min_c);
			tlv_mati_init_rc(&(b2),b,min_c,0,b->row-min_c,min_c);
			tlv_mati_init_rc(&(b3),b,min_c,min_c,b->row-min_c,b->col-min_c);

			tlv_mati_init_rc(&(c0),c,0,0,min_c,min_c);
			tlv_mati_init_rc(&(c1),c,0,min_c,min_c,c->col-min_c);
			tlv_mati_init_rc(&(c2),c,min_c,0,c->row-min_c,min_c);
			tlv_mati_init_rc(&(c3),c,min_c,min_c,c->row-min_c,c->col-min_c);

			//tlv_mati_print(&(a0));
			//tlv_matc_print(&(b0));
			tlv_mati_multi_dc2(&(c0),&(a0),&(b0),heap,NX);
			if(a1.col>0)
			{
				t1=heap->mat_new(heap->ths,min_c,min_c);
				//t1=tlv_mati_new(min_c,min_c);
				tlv_mati_multi_dc2(t1,&(a1),&(b2),heap,NX);
				tlv_mati_prow_add(&(c0),&(c0),t1);
				//tlv_mati_print(&(c0));
				//tlv_mati_delete(t1);
				heap->mat_delete(heap->ths,t1);
			}

			//tlv_log("%d\n",b->col-min_c);
			if(b1.col>0)
			{
				tlv_mati_multi_dc2(&(c1),&(a0),&(b1),heap,NX);
				if(a1.col>0)
				{
					//t2=tlv_mati_new(min_c,b->col-min_c);
					t1=heap->mat_new(heap->ths,min_c,b->col-min_c);
					tlv_mati_multi_dc2(t1,&(a1),&(b3),heap,NX);
					tlv_mati_prow_add(&(c1),&(c1),t1);
					//tlv_mati_delete(t2);
					heap->mat_delete(heap->ths,t1);
				}
			}
			if(a2.row>0)
			{
				tlv_mati_multi_dc2(&(c2),&(a2),&(b0),heap,NX);
				if(a3.col>0)
				{
					t1=heap->mat_new(heap->ths,a->row-min_c,min_c);
					//t3=tlv_mati_new(a->row-min_c,min_c);
					tlv_mati_multi_dc2(t1,&(a3),&(b2),heap,NX);
					tlv_mati_prow_add(&(c2),&(c2),t1);
					//tlv_mati_delete(t3);
					heap->mat_delete(heap->ths,t1);
				}

				if(b1.col>0)
				{
					tlv_mati_multi_dc2(&(c3),&(a2),&(b1),heap,NX);
					if(a3.col>0)
					{
						t1=heap->mat_new(heap->ths,a->row-min_c,b->col-min_c);
						//t4=tlv_mati_new(a->row-min_c,b->col-min_c);
						tlv_mati_multi_dc2(t1,&(a3),&(b3),heap,NX);
						tlv_mati_prow_add(&(c3),&(c3),t1);
						//tlv_mati_delete(t4);
						heap->mat_delete(heap->ths,t1);
					}
				}
			}
		}
	}else
	{
		tlv_mati_multi_prow_2(c,a,b);
	}
}

void tlv_mati_multi_dc_ext(tlv_mati_t *c,tlv_mati_t *a,tlv_matc_t *b,
		tlv_mati_heap_t *heap,int NX)
{
	tlv_mati_t a0,a1,a2,a3;
	tlv_matc_t b0,b1,b2,b3;
	tlv_mati_t c0,c1,c2,c3;
	int min_a,min_b,min_c;
	tlv_mati_t *t1;
	tlv_mati_t *t2;//,*t3,*t4;
	tlv_mati_t *m1,*m2,*m3,*m4,*m5,*m6,*m7;//,*m3,*m4,*m5,*m6,*m7;

	if(a->row>=NX && a->col>=NX && b->col>=NX)
	{
		//a*b b*c;
		min_a=min(a->row,a->col);
		min_b=min(b->row,b->col);
		min_c=min(min_a,min_b);
		//tlv_log("min_c=%d\n",min_c);
		min_c=min_c&0xFFFE;
		//tlv_log("min_c=%d\n",min_c);

		if(a->row==min_c && a->row==a->col && b->row==b->col)
		{
			//4*4;
			/*
				M1 = (A0 + A3) × (B0 + B3)
				M2 = (A2 + A3) × B0
				M3 = A0 × (B1 - B3)
				M4 = A3 × (B2 - B0)
				M5 = (A0 + A1) × B3
				M6 = (A2 - A0) × (B0 + B1)
				M7 = (A1 - A3) × (B2 + B3)
				C0 = M1 + M4 - M5 + M7
				C1 = M3 + M5
				C2 = M2 + M4
				C3 = M1 - M2 + M3 + M6
			 */
			min_c=min_c>>1;
			tlv_mati_init_rc(&(a0),a,0,0,min_c,min_c);
			tlv_mati_init_rc(&(a1),a,0,min_c,min_c,a->col-min_c);
			tlv_mati_init_rc(&(a2),a,min_c,0,a->row-min_c,min_c);
			tlv_mati_init_rc(&(a3),a,min_c,min_c,a->row-min_c,a->col-min_c);

			tlv_matc_init_rc(&(b0),b,0,0,min_c,min_c);
			tlv_matc_init_rc(&(b1),b,0,min_c,min_c,b->col-min_c);
			tlv_matc_init_rc(&(b2),b,min_c,0,b->row-min_c,min_c);
			tlv_matc_init_rc(&(b3),b,min_c,min_c,b->row-min_c,b->col-min_c);

			tlv_mati_init_rc(&(c0),c,0,0,min_c,min_c);
			tlv_mati_init_rc(&(c1),c,0,min_c,min_c,c->col-min_c);
			tlv_mati_init_rc(&(c2),c,min_c,0,c->row-min_c,min_c);
			tlv_mati_init_rc(&(c3),c,min_c,min_c,c->row-min_c,c->col-min_c);

			t1=heap->mat_new(heap->ths,min_c,min_c);
			t2=heap->mat_new(heap->ths,min_c,min_c);
			m1=heap->mat_new(heap->ths,min_c,min_c);
			m2=heap->mat_new(heap->ths,min_c,min_c);
			m3=heap->mat_new(heap->ths,min_c,min_c);
			m4=heap->mat_new(heap->ths,min_c,min_c);
			m5=heap->mat_new(heap->ths,min_c,min_c);
			m6=heap->mat_new(heap->ths,min_c,min_c);
			m7=heap->mat_new(heap->ths,min_c,min_c);

			/*
			t1=tlv_mati_new(min_c,min_c);
			t2=tlv_mati_new(min_c,min_c);
			m1=tlv_mati_new(min_c,min_c);
			m2=tlv_mati_new(min_c,min_c);
			m3=tlv_mati_new(min_c,min_c);
			m4=tlv_mati_new(min_c,min_c);
			m5=tlv_mati_new(min_c,min_c);
			m6=tlv_mati_new(min_c,min_c);
			m7=tlv_mati_new(min_c,min_c);
			*/

			tlv_mati_prow_add(t1,&(a0),&(a3));
			tlv_mati_prow_add2(t2,&(b0),&(b3));
			tlv_mati_multi_dc2(m1,t1,t2,heap,NX);
			//tlv_mati_print(t1);
			//tlv_mati_print(t2);
			//tlv_mati_print(m1);
			//exit(0);

			tlv_mati_prow_add(t1,&(a2),&(a3));
			tlv_mati_multi_dc_ext(m2,t1,&(b0),heap,NX);

			tlv_mati_prow_sub2(t1,&(b1),&(b3));
			tlv_mati_multi_dc2(m3,&(a0),t1,heap,NX);

			tlv_mati_prow_sub2(t1,&(b2),&(b0));
			tlv_mati_multi_dc2(m4,&(a3),t1,heap,NX);

			tlv_mati_prow_add(t1,&(a0),&(a1));
			tlv_mati_multi_dc_ext(m5,t1,&(b3),heap,NX);

			tlv_mati_prow_sub(t1,&(a2),&(a0));
			tlv_mati_prow_add2(t2,&(b0),&(b1));
			tlv_mati_multi_dc2(m6,t1,t2,heap,NX);

			tlv_mati_prow_sub(t1,&(a1),&(a3));
			tlv_mati_prow_add2(t2,&(b2),&(b3));
			tlv_mati_multi_dc2(m7,t1,t2,heap,NX);

			//tlv_mati_print(m7);
			tlv_mati_multi_c0(&(c0),m1,m4,m5,m7);
			tlv_mati_prow_add(&(c1),m3,m5);
			tlv_mati_prow_add(&(c2),m2,m4);
			tlv_mati_multi_c3(&(c3),m1,m2,m3,m6);

			heap->mat_delete(heap->ths,t1);
			heap->mat_delete(heap->ths,t2);
			heap->mat_delete(heap->ths,m1);
			heap->mat_delete(heap->ths,m2);
			heap->mat_delete(heap->ths,m3);
			heap->mat_delete(heap->ths,m4);
			heap->mat_delete(heap->ths,m5);
			heap->mat_delete(heap->ths,m6);
			heap->mat_delete(heap->ths,m7);
			/*
			tlv_mati_delete(t1);
			tlv_mati_delete(t2);
			tlv_mati_delete(m1);
			tlv_mati_delete(m2);
			tlv_mati_delete(m3);
			tlv_mati_delete(m4);
			tlv_mati_delete(m5);
			tlv_mati_delete(m6);
			tlv_mati_delete(m7);
			*/
		}else
		{
			//c0=a0*b0+a1*b2
			//c1=a0*b1+a1*b3
			//c2=a2*b0+a3*b2
			//c3=a2*b1+a3*b3
			/*
			 *	a0=min_c*min_c;
			 *	a1=min_c*a->col-min_c;
			 *	a2=a->row-min_c * min_c;
			 *	a3=a->row-min_c * a->col-min_c;
			 *
			 *	b0=min_c*min_c;
			 *	b1=min_c*b->col-min_c;
			 *	b2=b->row-min_c*min_c;
			 *	b3=b->row-min_c*b->col-min_c;
			 */
			//tlv_log("a1=")
			tlv_mati_init_rc(&(a0),a,0,0,min_c,min_c);
			tlv_mati_init_rc(&(a1),a,0,min_c,min_c,a->col-min_c);
			tlv_mati_init_rc(&(a2),a,min_c,0,a->row-min_c,min_c);
			tlv_mati_init_rc(&(a3),a,min_c,min_c,a->row-min_c,a->col-min_c);

			tlv_matc_init_rc(&(b0),b,0,0,min_c,min_c);
			tlv_matc_init_rc(&(b1),b,0,min_c,min_c,b->col-min_c);
			tlv_matc_init_rc(&(b2),b,min_c,0,b->row-min_c,min_c);
			tlv_matc_init_rc(&(b3),b,min_c,min_c,b->row-min_c,b->col-min_c);

			tlv_mati_init_rc(&(c0),c,0,0,min_c,min_c);
			tlv_mati_init_rc(&(c1),c,0,min_c,min_c,c->col-min_c);
			tlv_mati_init_rc(&(c2),c,min_c,0,c->row-min_c,min_c);
			tlv_mati_init_rc(&(c3),c,min_c,min_c,c->row-min_c,c->col-min_c);

			//tlv_mati_print(&(a0));
			//tlv_matc_print(&(b0));
			tlv_mati_multi_dc_ext(&(c0),&(a0),&(b0),heap,NX);
			if(a1.col>0)
			{
				t1=heap->mat_new(heap->ths,min_c,min_c);
				//t1=tlv_mati_new(min_c,min_c);
				tlv_mati_multi_dc_ext(t1,&(a1),&(b2),heap,NX);
				tlv_mati_prow_add(&(c0),&(c0),t1);
				//tlv_mati_print(&(c0));
				//tlv_mati_delete(t1);
				heap->mat_delete(heap->ths,t1);
			}

			//tlv_log("%d\n",b->col-min_c);
			if(b1.col>0)
			{
				tlv_mati_multi_dc_ext(&(c1),&(a0),&(b1),heap,NX);
				if(a1.col>0)
				{
					t1=heap->mat_new(heap->ths,min_c,b->col-min_c);
					tlv_mati_multi_dc_ext(t1,&(a1),&(b3),heap,NX);
					tlv_mati_prow_add(&(c1),&(c1),t1);
					//tlv_mati_delete(t2);
					heap->mat_delete(heap->ths,t1);
				}
			}
			if(a2.row>0)
			{
				tlv_mati_multi_dc_ext(&(c2),&(a2),&(b0),heap,NX);
				if(a3.col>0)
				{
					t1=heap->mat_new(heap->ths,a->row-min_c,min_c);
					//t3=tlv_mati_new(a->row-min_c,min_c);
					tlv_mati_multi_dc_ext(t1,&(a3),&(b2),heap,NX);
					tlv_mati_prow_add(&(c2),&(c2),t1);
					//tlv_mati_delete(t3);
					heap->mat_delete(heap->ths,t1);
				}

				if(b1.col>0)
				{
					tlv_mati_multi_dc_ext(&(c3),&(a2),&(b1),heap,NX);
					if(a3.col>0)
					{
						t1=heap->mat_new(heap->ths,a->row-min_c,b->col-min_c);
						tlv_mati_multi_dc_ext(t1,&(a3),&(b3),heap,NX);

						/*
						tlv_log("================================\n");
						tlv_log("a3 row=%d col=%d\n",a3.row,a3.col);
						tlv_log("b3 row=%d col=%d\n",b3.row,b3.col);
						tlv_log("t1 row=%d col=%d\n",t1->row,t1->col);
						tlv_log("c3 row=%d col=%d\n",c3.row,c3.col);
						*/

						tlv_mati_prow_add(&(c3),&(c3),t1);
						//tlv_mati_delete(t4);
						heap->mat_delete(heap->ths,t1);
					}
				}
			}
		}
	}else
	{
		tlv_mati_multi_prow(c,a,b);
	}
}


tlv_mati_t *tlv_mati_heap_new(void *ths,int row,int col)
{
	return tlv_mati_new(row,col);
}

void tlv_mati_heap_delete(void *ths,tlv_mati_t *m)
{
	tlv_mati_delete(m);
}

void tlv_mati_multi_dc(tlv_mati_t *c,tlv_mati_t *a,tlv_matc_t *b,int nx)
{
	tlv_mati_heap_t heap;

	heap.ths=NULL;
	heap.mat_new=tlv_mati_heap_new;
	heap.mat_delete=tlv_mati_heap_delete;
	tlv_mati_multi_dc_ext(c,a,b,&heap,nx);
}
