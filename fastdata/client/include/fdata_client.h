#ifndef FSQL_CLIENT_H
#define FSQL_CLIENT_H

typedef struct
{
	char addr[15];
	int port;
	int csocket;
}FDATA;

#define FDATA_SUCCESS		EXIT_SUCCESS
#define FDATA_FAILURE		EXIT_FAILURE
#define FDATA_SEARCH_FAILURE	NULL

extern int fdata_init(FDATA **, char *, int, char *, char *);
extern void fdata_close(FDATA **);
extern char *fdata_query(FDATA *, char *, int);
extern int fdata_insert(FDATA *, char *, char *, char **);
extern char *fdata_search(FDATA *, char *, char *, char *, int ,int);
extern int fdata_remove(FDATA *, char *, char *, char *, int);
extern int fdata_remove_table(FDATA *, char *);

#endif
