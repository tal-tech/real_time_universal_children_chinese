#include "tlv_arg.h"

static int tlv_arg_process(tlv_arg_t *arg,int argc,char** argv);

tlv_arg_t* tlv_arg_new(int argc,char** argv)
{
    tlv_arg_t *arg;

    arg = (tlv_arg_t*)tlv_malloc(sizeof(*arg));
    tlv_queue_init(&(arg->queue));
    arg->heap = tlv_heap_new(4096);
    tlv_arg_process(arg,argc,argv);

    return arg;
}

int tlv_arg_delete(tlv_arg_t *arg)
{
	tlv_heap_delete(arg->heap);
    tlv_free(arg);

    return 0;
}

void tlv_arg_add_item(tlv_arg_t *arg, char *k, char *v)
{
	tlv_arg_item_t *item;

	item = (tlv_arg_item_t*)tlv_heap_malloc(arg->heap,sizeof(*item));
	tlv_string_set(&(item->k), k, strlen(k));
	if(v)
	{
		tlv_string_set(&(item->v), v, strlen(v));
	}else
	{
		tlv_string_set(&(item->v), 0, 0);
	}
	tlv_queue_push(&(arg->queue), &(item->q_n));
}

int tlv_arg_item_cmp(tlv_string_t *v, tlv_arg_item_t *i)
{
	return tlv_string_cmp(v, i->k.data, i->k.len);
}

tlv_string_t* tlv_arg_get_value(tlv_arg_t *arg, const char *k, int bytes)
{
	tlv_arg_item_t *item;
	tlv_string_t str;

	tlv_string_set(&(str),(char*)k,bytes);
	item = tlv_queue_find(&(arg->queue), offsetof(tlv_arg_item_t,q_n), (tlv_cmp_handler_f)tlv_arg_item_cmp,&str);
	if(item)
	{
		return &(item->v);
	}else
	{
		return 0;
	}
}


int tlv_arg_get_int(tlv_arg_t *arg, const char *key, int bytes, int* number)
{
    tlv_string_t *v;

    v = tlv_arg_get_value(arg,key,bytes);
    if(v && v->len>0)
    {
        *number = atoi(v->data);
    }

    return v?0:-1;
}

int tlv_arg_get_number(tlv_arg_t *arg, const char *key, int bytes, double *n)
{
    tlv_string_t *v;

    v = tlv_arg_get_value(arg,key,bytes);
    if(v && v->len>0)
    {
        *n = atof(v->data);
    }

    return v?0:-1;
}

int tlv_arg_get_str(tlv_arg_t *arg, const char *key, int bytes, char** pv)
{
    tlv_string_t *s;
    char *v;

    s = tlv_arg_get_value(arg,key,bytes);
    if(s && s->len > 0)
    {
    	v = s->data;
    }else
    {
    	v = 0;
    }
    *pv = v;

    return v?0:-1;
}

int tlv_arg_exist(tlv_arg_t *arg, const char* key, int bytes)
{
    tlv_string_t *v;

    v = tlv_arg_get_value(arg,key,bytes);

    return v?1:0;
}

static int tlv_arg_process(tlv_arg_t *arg,int argc,char** argv)
{
    typedef enum
    {
            ARG_KEY_WAIT,
            ARG_VALUE_WAIT
    }arg_state_t;

    char *magic=0;
    char *k,*v,*p;
    int i;
    arg_state_t s;

    s = ARG_KEY_WAIT;
    k = v= p = 0;
    for(i=0;i<argc;++i)
    {
        p=argv[i];
        switch(s)
        {
        case ARG_KEY_WAIT:
            if(*p=='-' && *(p+1)!=0)
            {
                k=p+1;
                if(i==argc-1)
                {
                    v=magic;
                    tlv_arg_add_item(arg,k,v);
                }else
                {
                    s=ARG_VALUE_WAIT;
                }
            }else if(i>0)
            {
            	tlv_arg_add_item(arg,p,0);
            }
            break;
        case ARG_VALUE_WAIT:
            if(*p=='-')
            {
                v=magic;
                tlv_arg_add_item(arg,k,v);
                k = p+1;
            }else
            {
                v = p;
                tlv_arg_add_item(arg,k,v);
                s = ARG_KEY_WAIT;
            }
            break;
        }
    }

    return 0;
}

