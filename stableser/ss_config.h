#ifndef SS_CONFIG_H
#define SS_CONFIG_H

/*Hash*/
static int HASH_SIZE = 100;

typedef struct _hash_data
{
	char type[50];
	char descriptor[50];
	struct _hash_data *next;
}hash_data;

typedef struct
{
	hash_data *data;
}hash_table;


/*General definition*/
typedef int          ss_int_t;
typedef unsigned int ss_uint_t;
typedef char         ss_char_t;

/*Version definition*/
#define SS_VERSION     "1.1.1"

/*configura definition*/
#ifdef HAVE_CONFIG_H

#include "config.h"
#define SS_VER         PACKAGE "/"  VERSION

#else

#define SS_VER	       "stableser/" SS_VERSION

#endif

#endif
