#ifndef HASH_TABLE_H
#define HASH_TABLE_H

typedef struct _bucket
{
	char *name;
	char **data;
	struct _bucket *next;
}bucket;

typedef struct
{
	bucket *data;
}hash_table;

#define		HASH_SIZE		10000
#define		HASH_DEFAULT_ITEM_SIZE	50
#define		HASH_TABLE_ITEM_SIZE	1024
#define		HASH_TABLE_SIZE		1024

#define		HASH_SUCCESS		0
#define		HASH_MALLOC_FAILURE	1
#define		HASH_NOT_FOUND		2
#define		HASH_SEARCH_NOT_FOUND	NULL

extern unsigned int hash_index(char *);
extern int hash_init(hash_table **);
extern int hash_insert_internal(hash_table *, char *, char [][1024]);
extern bucket *hash_search(hash_table *, char *, char *, int);
extern int hash_remove(hash_table *, char *, char *, int);
extern int hash_remove_all(hash_table *, char *, char *, int);
extern void hash_free(hash_table *);
extern bucket *hash_like_search(hash_table *, char *, char *, int);
extern int hash_insert(hash_table *, char *, char [][1024]);

#endif
