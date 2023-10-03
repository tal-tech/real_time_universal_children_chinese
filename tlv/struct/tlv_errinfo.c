/*
 * tlv_errinfo.c
 *
 *  Created on: May 15, 2018
 *      Author: jfyuan
 */
#include <ctype.h>
#include <stdarg.h>
#include "tlv_errinfo.h"

tlv_errinfo_t* tlv_errinfo_new()
{
	tlv_errinfo_t* e;

	e      = (tlv_errinfo_t*)tlv_malloc(sizeof(*e));
	e->no  = 0;
	e->buf = tlv_charbuf_new(64, 0.5);

	return e;
}


void tlv_errinfo_reset(tlv_errinfo_t *e)
{
	e->no = 0;
	tlv_charbuf_reset(e->buf);
}

void tlv_errinfo_delete(tlv_errinfo_t *e)
{
	tlv_charbuf_delete(e->buf);
	tlv_free(e);
}

void tlv_errinfo_set(tlv_errinfo_t *e, int no, ...)
{
	va_list ap;
	char *p;

	e->no = no;
	tlv_charbuf_reset(e->buf);
	va_start(ap, no);
	while((p=va_arg(ap, char*)) != 0)
	{
		tlv_charbuf_push_string(e->buf, p);
	}
	va_end(ap);
}
