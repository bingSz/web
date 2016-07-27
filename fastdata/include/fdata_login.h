#ifndef FDATA_LOGIN_H
#define FDATA_LOGIN_H

typedef struct
{
	char *username;
	char *password;
}login_info;

#define		LOGIN_SUCCESS		0
#define		LOGIN_FAILURE		-1

extern int init_login_info(char *);
extern int login_table(int);
extern int passwd_user(char *, char *, char *);

#endif
