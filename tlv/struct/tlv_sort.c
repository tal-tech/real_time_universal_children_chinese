/*
 * tlv_sort.c
 *
 *  Created on: Sep 4, 2018
 *      Author: jfyuan
 */
#include "tlv_sort.h"

//void tlv_qsort(void* s, void* e, size_t size, tlv_qsort_cmp_f cmp, void *app_data, void *tmp_elem)
//{
//	char *x;
//	char *i, *j;
//
//	if(e <= s) { return; }
//	x = (char*)e;
//	i = (char*)s - size;
//	j = (char*)s;
//
//	while(j < x)
//	{
//		if(cmp(app_data, (void*)j, (void*)x) <= 0)
//		{
//			i += size;
//			if(j != i)
//			{
//				memcpy(tmp_elem, i, size);
//				memcpy(i, j, size);
//				memcpy(j, tmp_elem, size);
//			}
//		}
//
//		j += size;
//	}
//
//	if(i != x)
//	{
//		memcpy(tmp_elem, i, size);
//		memcpy(i, x, size);
//		memcpy(x, tmp_elem, size);
//	}
//
//	tlv_qsort(s, i-size, size, cmp, app_data, tmp_elem);
//	tlv_qsort(i+size, e, size, cmp, app_data, tmp_elem);
//}
//
//void tlv_qsort2(void *base, size_t nmemb, size_t size, tlv_qsort_cmp_f cmp, void *app_data)
//{
//	void *tmp_elem;
//
//	tmp_elem = malloc(size);
//	tlv_qsort(base, (void*)((char*)base+(nmemb-1)*size), size, cmp, app_data, tmp_elem);
//	free(tmp_elem);
//}
//
//void tlv_qsort3(void *base, size_t nmemb, size_t size, tlv_qsort_cmp_f cmp, void *app_data, void *tmp_elem)
//{
//	tlv_qsort(base, (void*)((char*)base+(nmemb-1)*size), size, cmp, app_data, tmp_elem);
//}
