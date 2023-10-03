#include "tlv_math.h"

void tlv_float_sigmoid(float *f, int len)
{
	float *s;
	float *e;
	
	s = f;
	e = f + len;
	while(s < e)
	{
		*s = 1.0/(1.0 + expf(-*s));
		++s;
	}
}

void tlv_float_softmax(float* f, int len)
{
	float max, sum=0.0;
	float *s, *e;

	max = tlv_math_max(f, len);

	s = f;
	e = f+ len;
	while(s < e)
	{
		*s = expf(*s - max);
		sum += *s;
		++s;
	}
	sum = 1.0f / sum;

	s = f;
	e = f + len;
	while(s < e)
	{
		*s *= sum;
		++s;
	}
}

double tlv_log_add(double x, double y, double min_log_exp)
{
	double temp,diff,z;

   if (x<y)
   {
      temp = x; x = y; y = temp;
   }
   diff = y-x;
   if (diff<min_log_exp)
   {
      return  (x<LSMALL)?LZERO:x;
   }
   else
   {
      z = exp(diff);
      return x+log(1.0+z);
   }
}



tlv_vector_t* tlv_math_create_ham_window(int frame_size)
{
	tlv_vector_t *v;
	float a;

	v = tlv_vector_new(frame_size);
	a = (PI*2)/(frame_size-1);
	tlv_vector_do_i(v,,=0.54-0.46*cos(a*(i-1)));

	return v;
}

void tlv_vector_zero_mean_frame(tlv_vector_t* v)
{
	float sum=0,off;

	tlv_vector_do_p(v,sum+=,);
	off=sum/tlv_vector_size(v);
	tlv_vector_do_p(v,,-=off);
}

void tlv_vector_pre_emphasise(tlv_vector_t* v,float k)
{
	int i;

	for(i=tlv_vector_size(v);i>=2;--i)
	{
		v[i]-=v[i-1]*k;
	}
	v[1]*=1.0-k;
}

tlv_vector_t* tlv_math_create_ham_window_h(tlv_heap_t *h,int frame_size)
{
	tlv_vector_t *v;
	float a;

	v=tlv_vector_new_h(h,frame_size);
	a=(PI*2)/(frame_size-1);
	tlv_vector_do_i(v,,=0.54-0.46*cos(a*(i-1)));
	return v;
}


static void tlv_fft(tlv_vector_t* s, int invert)
{
   int ii,jj,n,nn,limit,m,j,inc,i;
   double wx,wr,wpr,wpi,wi,theta;
   double xre,xri,x;

   n=tlv_vector_size(s);
   nn=n / 2; j = 1;
   for (ii=1;ii<=nn;ii++) {
      i = 2 * ii - 1;
      if (j>i) {
         xre = s[j]; xri = s[j + 1];
         s[j] = s[i];  s[j + 1] = s[i + 1];
         s[i] = xre; s[i + 1] = xri;
      }
      m = n / 2;
      while (m >= 2  && j > m) {
         j -= m; m /= 2;
      }
      j += m;
   };
   limit = 2;
   while (limit < n) {
      inc = 2 * limit; theta = (PI*2) / limit;
      if (invert) theta = -theta;
      x = sin(0.5 * theta);
      wpr = -2.0 * x * x; wpi = sin(theta);
      wr = 1.0; wi = 0.0;
      for (ii=1; ii<=limit/2; ii++) {
         m = 2 * ii - 1;
         for (jj = 0; jj<=(n - m) / inc;jj++) {
            i = m + jj * inc;
            j = i + limit;
            xre = wr * s[j] - wi * s[j + 1];
            xri = wr * s[j + 1] + wi * s[j];
            s[j] = s[i] - xre; s[j + 1] = s[i + 1] - xri;
            s[i] = s[i] + xre; s[i + 1] = s[i + 1] + xri;
         }
         wx = wr;
         wr = wr * wpr - wi * wpi + wr;
         wi = wi * wpr + wx * wpi + wi;
      }
      limit = inc;
   }
   if (invert)
      for (i = 1;i<=n;i++)
         s[i] = s[i] / nn;

}


void tlv_vector_realft (tlv_vector_t* v)
{
   int n, n2, i, i1, i2, i3, i4;
   double xr1, xi1, xr2, xi2, wrs, wis;
   double yr, yi, yr2, yi2, yr0, theta, x;

   n=tlv_vector_size(v) / 2; n2 = n/2;
   theta = PI / n;
   tlv_fft(v, 0);
   x = sin(0.5 * theta);
   yr2 = -2.0 * x * x;
   yi2 = sin(theta); yr = 1.0 + yr2; yi = yi2;
   for (i=2; i<=n2; i++) {
      i1 = i + i - 1;      i2 = i1 + 1;
      i3 = n + n + 3 - i2; i4 = i3 + 1;
      wrs = yr; wis = yi;
      xr1 = (v[i1] + v[i3])/2.0; xi1 = (v[i2] - v[i4])/2.0;
      xr2 = (v[i2] + v[i4])/2.0; xi2 = (v[i3] - v[i1])/2.0;
      v[i1] = xr1 + wrs * xr2 - wis * xi2;
      v[i2] = xi1 + wrs * xi2 + wis * xr2;
      v[i3] = xr1 - wrs * xr2 + wis * xi2;
      v[i4] = -xi1 + wrs * xi2 + wis * xr2;
      yr0 = yr;
      yr = yr * yr2 - yi  * yi2 + yr;
      yi = yi * yr2 + yr0 * yi2 + yi;
   }
   xr1 = v[1];
   v[1] = xr1 + v[2];
   v[2] = 0.0;
}

void tlv_math_do_diff(tlv_vector_t** pv,int window_size,double sigma,int start_pos,int step)
{
	int i,j,k,end=start_pos+step;
	tlv_vector_t *v=pv[window_size];
	tlv_vector_t *p,*n;
	int vs=start_pos-step;

	for(i=1;i<=window_size;++i)
	{
		p=pv[window_size-i];
		n=pv[window_size+i];
		for(j=start_pos,k=vs;j<end;++j,++k)
		{
			if(i==1)
			{
				v[j]=(n[k]-p[k]);
			}else
			{
				v[j]+=i*(n[k]-p[k]);
			}
			if(i==window_size)
			{
				v[j]/=sigma;
			}
		}
	}
}

/**
 *	dt=(C{t+w}-C{t-w))/2w
 */
void tlv_math_do_simple_diff(tlv_vector_t** pv,int window_size,int start_pos,int step)
{
	int j,k,end=start_pos+step;
	tlv_vector_t *v=pv[window_size];
	tlv_vector_t *p,*n;
	int vs=start_pos-step;
	int dw;

	dw=window_size<<1;
	p=pv[0];
	n=pv[dw];
	for(j=start_pos,k=vs;j<end;++j,++k)
	{
		v[j]=(n[k]-p[k])/dw;
	}
}


int tlv_strfile_read_vector(tlv_strfile_t* s,tlv_vector_t* v,int bin)
{
	return tlv_strfile_read_float(s,v+1,tlv_vector_size(v),bin);
}

int tlv_strfile_read_matrix(tlv_strfile_t *s,tlv_matrix_t *m,int bin)
{
	int i,nrows,ret=0;

	//tlv_log("row=%d col=%d\n",tlv_matrix_rows(m),tlv_matrix_cols(m));
	nrows=tlv_matrix_rows(m);
	for(i=1;i<=nrows;++i)
	{
		//tlv_log("col=%d\n",tlv_vector_size(m[i]));
		ret=tlv_strfile_read_vector(s,m[i],bin);
		if(ret!=0)
		{
			tlv_log("%d,sm=%d\n",ret,tlv_vector_size(m[i]));
			goto end;
		}
	}
end:
	return ret;
}

/**
 * @date	20180418
 * @auth	jfyuan
 * @brief 	read short m, and per value / scale, and  save in the tlv_matrix_t struct.
 */
int tlv_strfile_read_short_matrix(tlv_strfile_t *s, tlv_matrix_t *m, int bin, float scale)
{
	int i,j,nrows,ncols,ret=0;
	tlv_vector_short_t *v;

	//tlv_log("row=%d col=%d\n",tlv_matrix_rows(m),tlv_matrix_cols(m));
	nrows = tlv_matrix_rows(m);
	ncols = tlv_matrix_cols(m);
	v = (tlv_vector_type(short)*)tlv_malloc(tlv_vector_type_bytes(ncols, short));
	*((short*)v)= ncols;
	for(i=1;i<=nrows;++i)
	{
		//tlv_log("col=%d\n",tlv_vector_size(m[i]));
		ret = tlv_strfile_read_short(s, v+1, tlv_vector_short_size(v), bin);
		if(ret!=0)
		{
			tlv_log("%d,sm=%d\n",ret,tlv_vector_size(m[i]));
			goto end;
		}

		for(j=1; j <= ncols; j++)
		{
			m[i][j] = v[j] / scale;
		}
	}
end:
	tlv_vector_delete(v);

	return ret;
}

int tlv_strfile_read_hlda_bin(tlv_matrix_t **pm,tlv_strfile_t *s)
{
	tlv_matrix_t *m;
	int r_c[2];
	int ret;
	int i;


	s->swap=0;
	ret=tlv_strfile_read_int(s,r_c,2,1);
	if(ret!=0){goto end;}

	m=tlv_matrix_new2(r_c[0],r_c[1]);

	for(i=1;i<=r_c[0];++i)
	{
		ret=tlv_strfile_read_vector(s,m[i],1);
		if(ret!=0){goto end;}
	}
	*pm=m;

	ret=0;
end:

	return ret;
}

int tlv_strfile_read_hlda(tlv_strfile_t *s,tlv_matrix_t **pm)
{
	tlv_charbuf_t *buf;
	int ret;
	int row_col[2];
	tlv_matrix_t *m=0;

	buf=tlv_charbuf_new(64,1);
	while(1)
	{
		ret=tlv_strfile_read_string(s,buf);
		if(ret!=0){goto end;}
		if(tlv_str_equal_s(buf->data,buf->pos,"<XFORM>"))
		{
			break;
		}
	}
	ret=tlv_strfile_read_int(s,row_col,2,0);
	if(ret!=0){goto end;}
	m=tlv_matrix_new(row_col[0],row_col[1]);
	ret=tlv_strfile_read_matrix(s,m,0);

end:
	if(ret==0)
	{
		*pm=m;
	}else
	{
		if(m)
		{
			tlv_matrix_delete(m);
		}
	}
	tlv_charbuf_delete(buf);
	return ret;
}

int tlv_hlda_read(tlv_matrix_t **pm,tlv_strfile_t *s)
{
	return tlv_strfile_read_hlda(s,pm);
}


void tlv_matrix_multiply_vector(tlv_vector_t *dst,tlv_matrix_t *m,tlv_vector_t *src)
{
	int rows,cols,i,j;
	float vi;
	float *mi;

	rows=tlv_matrix_rows(m);
	cols=tlv_matrix_cols(m);
	for(i=1;i<=rows;++i)
	{
		vi=0;mi=m[i];
		for(j=1;j<=cols;++j)
		{
			vi+=mi[j]*src[j];
		}
		dst[i]=vi;
	}
}
