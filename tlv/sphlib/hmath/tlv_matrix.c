#include "tlv_matrix.h"
#define LSMALL (-0.5E10)   /* log values < LSMALL are set to LZERO */

tlv_matrix_t* tlv_matrix_init(char *p, int nrows, int ncols)
{
	float **m;
	int csize;
	int i;

	m = (float**)p;
	*((int*)p) = nrows;
	csize = tlv_vector_bytes(ncols);
	p += tlv_round_word((nrows+1) * sizeof(float*));
	for(i=1; i<=nrows; ++i,p+=csize)
	{
		*((int*)p) = ncols;
		m[i] = (float*)p;
	}

	return m;
}

tlv_matrix_t* tlv_matrix_new(int nrows, int ncols)
{
	char *p;

	p=(char*)tlv_calloc(1, tlv_matrix_bytes(nrows,ncols));

	return tlv_matrix_init(p,nrows,ncols);
}

static int tlv_matrix_16_bytes(int r,int col)
{
	return sizeof(float*)*(r+1)+16+(sizeof(float)*(col+1)+16)*r;
}

tlv_matrix_t* tlv_matrix_new2(int nrows, int ncols)
{
	char *p;
	float **m;
	int i;
	int bytes;
	int col_bytes;

	bytes = tlv_matrix_16_bytes(nrows, ncols);
	p = tlv_malloc(bytes);
	m = (float**)p;
	*((int*)p) = nrows;
	p += sizeof(float*)*(nrows+1);
	col_bytes = sizeof(float)*(ncols+1);
	for(i=1; i<=nrows; ++i)
	{
		p = tlv_align_p(p+sizeof(float),16)-sizeof(float);

		*((int*)p) = ncols;
		m[i] = (float*)p;

		p += col_bytes;
	}

	return m;
}

tlv_matrix_t* tlv_matrix_newh(tlv_heap_t* h, int nrows, int ncols)
{
	char *p;

	p = (char*)tlv_heap_malloc(h,tlv_matrix_bytes(nrows,ncols));

	return tlv_matrix_init(p,nrows,ncols);
}

void tlv_matrix_multi(tlv_matrix_t *m, tlv_matrix_t *a, tlv_matrix_t *b)
{
	int rows=tlv_matrix_rows(m);
	int cols=tlv_matrix_cols(m);
	int ac=tlv_matrix_cols(a);
	int i,k;
	float *pa,*pm;
	register float *tpm,*tpb;
	register float pak;
	register float *e;

	for(i=1;i<=rows;++i)
	{
		pa=a[i];pm=m[i];
		e=pm+cols;
		for(k=1;k<=ac;++k)
		{
			tpb=b[k];pak=pa[k];
			tpm=pm;
			//tlv_log("%d/%d=%d\n",i,k,(int)(e-tpm));
			if(k==1)
			{

				while(e-tpm>=4)
				{
					*(++tpm)=pak*(*(++tpb));
					*(++tpm)=pak*(*(++tpb));
					*(++tpm)=pak*(*(++tpb));
					*(++tpm)=pak*(*(++tpb));
				}
				while(tpm<e)
				{
					*(++tpm)=pak*(*(++tpb));
				}
			}else
			{
				while(e-tpm>=4)
				{
					*(++tpm)+=pak*(*(++tpb));
					*(++tpm)+=pak*(*(++tpb));
					*(++tpm)+=pak*(*(++tpb));
					*(++tpm)+=pak*(*(++tpb));
				}
				while(tpm<e)
				{
					*(++tpm)+=pak*(*(++tpb));
				}
			}
		}
	}
}

void tlv_matrix_multi2(tlv_matrix_t *m, tlv_matrix_t *a, tlv_matrix_t *b)
{
	int rows=tlv_matrix_rows(m);
	int cols=tlv_matrix_cols(m);
	int ac=tlv_matrix_cols(a);
	int i,j,k;
	double t;
	float *p;

	for(i=1;i<=rows;++i)
	{
		p=a[i];
		for(j=1;j<=cols;++j)
		{
			for(t=0,k=1;k<=ac;++k)
			{
				t+=p[k]*b[k][j];
			}
			m[i][j]=t;
		}
	}
}

void tlv_matrix_multi3(tlv_matrix_t *m, tlv_matrix_t *a, tlv_matrix_t *b)
{
	int rows=tlv_matrix_rows(m);
	int cols=tlv_matrix_cols(m);
	int ac=tlv_matrix_cols(a);
	int i,j,k;
	register float *pi,*ai;
	float t;

	for(i=1;i<=rows;++i)
	{
		pi=m[i];ai=a[i];
		for(j=1;j<=cols;++j)
		{
			t=0;
			for(k=1;k<=ac;++k)
			{
				t+=ai[k]*b[k][j];

			}
			pi[j]=t;
		}
	}
}


void tlv_matrix_transpose(tlv_matrix_t *dst, tlv_matrix_t *src)
{
	int rows=tlv_matrix_rows(dst);
	int cols=tlv_matrix_cols(dst);
	int i,j;
	float *pm;

	for(i=1;i<=rows;++i)
	{
		pm=dst[i];
		for(j=1;j<=cols;++j)
		{
			pm[j]=src[j][i];
		}
	}
}

tlv_matrix_t* tlv_matrix_transpose2(tlv_matrix_t *a)
{
	tlv_matrix_t *b;

	b = tlv_matrix_new2(tlv_matrix_cols(a),tlv_matrix_rows(a));
	tlv_matrix_transpose(b, a);

	return b;
}

void tlv_matrix_cpy(tlv_matrix_t *src, tlv_matrix_t *dst)
{
	int i,rows;

	rows = tlv_matrix_rows(src);
	for(i=1; i<=rows; ++i)
	{
		tlv_vector_cpy(src[i], dst[i]);
	}
}

void tlv_matrix_scale(tlv_matrix_t *m, float scale)
{
	int r,c;
	int i,j;

	r=tlv_matrix_rows(m);
	c=tlv_matrix_cols(m);
	for(i=1;i<=r;++i)
	{
		for(j=1;j<=c;++j)
		{
			if(m[i][j]>LSMALL)
			{
				m[i][j]*=scale;
			}
		}
	}
}

void tlv_matrix_add(tlv_matrix_t *m, tlv_matrix_t *a)
{
	int rows = tlv_matrix_rows(m);
	int cols = tlv_matrix_cols(m);
	int i, j;
	float *pm, *pa;

	for(i=1; i<=rows; i++)
	{
		pm = m[i]; pa = a[i];
		for(j=1; j<=cols; j++)
		{
			pm[j] += pa[j];
		}
	}

}


double tlv_matrix_max(tlv_matrix_t *m)
{
	int rows=tlv_matrix_rows(m);
	int cols=tlv_matrix_cols(m);
	int i,j;
	double max=-100000.0;

	for(i=1;i<=rows;++i)
	{
		for(j=1;j<=cols;++j)
		{
			if(m[i][j]>max)
			{
				max=m[i][j];
			}
		}
	}

	return max;
}


double tlv_matrix_min(tlv_matrix_t *m)
{
	int rows=tlv_matrix_rows(m);
	int cols=tlv_matrix_cols(m);
	int i,j;
	double min=100000.0;

	for(i=1;i<=rows;++i)
	{
		for(j=1;j<=cols;++j)
		{
			if(m[i][j]<min)
			{
				min=m[i][j];
			}
		}
	}

	return min;
}

double tlv_matrix_max_abs(tlv_matrix_t *m)
{
	double max,min;

	max=tlv_matrix_max(m);
	min=tlv_matrix_min(m);
	if(max<0)
	{
		max=-max;
	}
	if(min<0)
	{
		min=-min;
	}
	if(max>min)
	{
		return max;
	}else
	{
		return min;
	}
}

void tlv_matrix_print(tlv_matrix_t *m)
{
	int i,rows;
	int j,cols;

	rows=tlv_matrix_rows(m);
	cols=tlv_matrix_cols(m);
	for(i=1;i<=rows;++i)
	{
		for(j=1;j<=cols;++j)
		{
			printf("v[%d][%d]=%f\n",i,j,m[i][j]);
		}
	}
}

tlv_smatrix_t* tlv_smatrix_newh(tlv_heap_t *h,int nrows,int ncols)
{
	float** m;
	char *p;
	int csize,j;

	p = (char*)tlv_heap_malloc(h,tlv_smatrix_bytes(nrows,ncols))+2*sizeof(void**);
	m = (float**)((char*)p);
	*(int*)m = nrows;
	csize = tlv_vector_bytes(ncols);
	p += (nrows+1)*sizeof(float*);
	for(j=1;j<=nrows;++j,p+=csize)
	{
		*(int*)p=ncols;
		m[j]=(float*)p;
	}
	tlv_set_hook((void**)m,0);
	tlv_set_use((void**)m,0);

	return m;
}


void tlv_matrix_double_cpy(tlv_matrix_double_t *src,tlv_matrix_double_t *dst)
{
	int i,rows;

	rows=tlv_matrix_rows(src);
	for(i=1;i<=rows;++i)
	{
		tlv_vector_double_cpy(src[i],dst[i]);
	}
}

void tlv_matrix_double_zero(tlv_matrix_double_t *m)
{
	int i,j,nr,nc;

	nr=tlv_matrix_rows(m);
	nc=tlv_matrix_cols(m);
	for(i=1;i<=nr;++i)
	{
		for(j=1;j<=nc;++j)
		{
			m[i][j]=0;
		}
	}
}

tlv_matrix_double_t* tlv_matrix_double_init(char *p,int nrows,int ncols)
{
	int csize;
	double **m;
	int i;

	m=(double**)p;
	*((int*)p)=nrows;
	csize=tlv_vector_double_bytes(ncols);
	p+=tlv_round_word((nrows+1)*sizeof(double*));
	for(i=1;i<=nrows;++i,p+=csize)
	{
		*((int*)p)=ncols;
		m[i]=(double*)p;
	}
	return m;
}

tlv_matrix_double_t* tlv_matrix_double_new(int nrows, int ncols)
{
	char *p;

	p = (char*)tlv_malloc(tlv_matrix_double_bytes(nrows,ncols));

	return tlv_matrix_double_init(p,nrows,ncols);
}

tlv_matrix_double_t* tlv_matrix_double_new_h(tlv_heap_t *heap, int nrows, int ncols)
{
	char *p;

	p = (char*)tlv_heap_malloc(heap, tlv_matrix_double_bytes(nrows, ncols));

	return tlv_matrix_double_init(p, nrows, ncols);
}

void tlv_matrix_double_init_identity(tlv_matrix_double_t *A)
{
	int i, size;

	tlv_matrix_double_zero(A);
	size = min(tlv_matrix_rows(A), tlv_matrix_cols(A));
	for(i=1; i <= size; i++)
	{
		A[i][i] = 1.0;
	}
}
