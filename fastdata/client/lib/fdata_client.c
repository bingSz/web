#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>

#include <signal.h>

/* Socket */

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>

/* End */

/* Fastsql */

#include "fdata_client.h"

/* End */

int send_mutex = 0;
int *send_mutex_addr;

char fdata_status[10] = "";

int fdata_init_socket(int *csocket, char *addr, int port)
{
	struct sockaddr_in sock_info;
	int sock_opts = 1;

	if (((*csocket) = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return FDATA_FAILURE;
	}
	
	if (setsockopt((*csocket), SOL_SOCKET, SO_REUSEADDR, &sock_opts, sizeof(sock_opts)) < 0)
	{
		return FDATA_FAILURE;
	}

	sock_info.sin_family = AF_INET;
	sock_info.sin_port = htons(port);
	sock_info.sin_addr.s_addr = inet_addr(addr);

	if (connect((*csocket), (struct sockaddr *)&sock_info, sizeof(sock_info)) < 0)
	{
		return FDATA_FAILURE;
	}

	return FDATA_SUCCESS;
}

int fdata_init(FDATA **target, char *addr, int port, char *username, char *password)
{
	char *send_command;
	char recv_buf[14] = "";

	(*target) = (FDATA *)malloc(sizeof(FDATA));
	if ((*target) == NULL)
	{
		return FDATA_FAILURE;
	}

	(*target)->port = port;
	memcpy((*target)->addr, addr, sizeof((*target)->addr));

	if (FDATA_FAILURE == fdata_init_socket(&(*target)->csocket, addr, port))
	{
		return FDATA_FAILURE;
	}

	send_command = (char *)malloc(sizeof(char) * (strlen(username) + strlen(password) + 8));
	memset(send_command, 0, (strlen(username) + strlen(password) + 8));
	snprintf(send_command, strlen(username) + strlen(password) + 8, "login %s %s", username, password);
	if (-1 == send((*target)->csocket, send_command, strlen(send_command), MSG_NOSIGNAL))
	{
		return FDATA_FAILURE;
	}
	if (-1 == recv((*target)->csocket, recv_buf, 14, 0))
	{
		return FDATA_FAILURE;
	}
	if (0 != strcmp(recv_buf, "successfully"))
	{
		return FDATA_FAILURE;
	}

	free(send_command);

	send_mutex_addr = (int *)mmap((void *)&send_mutex, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (send_mutex_addr == MAP_FAILED)
	{
		return FDATA_FAILURE;
	}
	*send_mutex_addr = 0;

	return FDATA_SUCCESS;
}

int fdata_passwd(FDATA *target, char *old_password, char *new_password)
{
	char send_command[4096] = "";
	char recv_buf[14] = "";

	snprintf(send_command, sizeof(send_command), "login %s %s", old_password, new_password);

	if (0 >= send(target->csocket, send_command, strlen(send_command), MSG_NOSIGNAL))
	{
		return FDATA_FAILURE;
	}
	if (0 >= recv(target->csocket, recv_buf, sizeof(recv_buf), 0))
	{
		return FDATA_FAILURE;
	}
	if (0 != strcmp(recv_buf, "successfully"))
	{
		return FDATA_FAILURE;
	}

	return FDATA_SUCCESS;
}

void fdata_close(FDATA **target)
{
	(*target)->port = 0;
	memset((*target)->addr, 0, 15);
	close((*target)->csocket);
}

char *fdata_query(FDATA *target, 
		char *command, 
		int timeout)
{
	char recv_buf[1024] = "";
	char *p;

recheck:
	if (*send_mutex_addr != 0)
	{
		goto recheck;
	}
	*send_mutex_addr = 1;

	alarm(timeout);

	send(target->csocket, command, strlen(command), MSG_NOSIGNAL);
redo:
	if (-1 == recv(target->csocket, recv_buf, 1024, 0))
	{
		return " ";
	}
	if (0 == strlen(recv_buf))
	{
		goto redo;
	}
	alarm(0);

	*send_mutex_addr = 0;

	p = recv_buf;

	return p;
}

int fdata_insert(FDATA *target, 
		char *table_name, 
		char *name, 
		char **data)		/* insert [table] [name] [data...] */
{
	char recv_buf[13];
	char command[4096] = "";
	int i;

	snprintf(command, sizeof(command), "insert %s %s", table_name, name);

	for (i = 0; 0 != strlen(data[i]); i++)
	{
		strncat(command, " ", sizeof(command) - strlen(command));
		strncat(command, data[i], sizeof(command) - strlen(command));
		if (data[i + 1] == NULL)
		{
			break;
		}
	}

	if (0 >= send(target->csocket, command, strlen(command), MSG_NOSIGNAL))
	{
		return FDATA_FAILURE;
	}

	if (0 >= recv(target->csocket, recv_buf, 13, 0))
	{
		return FDATA_FAILURE;
	}

	if (0 == strcmp(recv_buf, "successfully"))
	{
		return FDATA_SUCCESS;
	}

	return FDATA_FAILURE;
}

char *fdata_search(FDATA *target, 
		char *table, 
		char *name, 
		char *data, 
		int num, 
		int send_num)		/* search [table] [name] [data] [num] [send_num] */
{
	char recv_buf[1024] = "";
	char command[4096] = "";
	char *ret_str;

	snprintf(command, 4096, "search %s %s %s %d %d", table, name, data, num, send_num);

	if (0 >= send(target->csocket, command, strlen(command), MSG_NOSIGNAL))
	{
		return FDATA_SEARCH_FAILURE;
	}

	if (0 >= recv(target->csocket, recv_buf, 1024, 0))
	{
		return FDATA_SEARCH_FAILURE;
	}

	ret_str = recv_buf;

	return ret_str;
}

int fdata_remove(FDATA *target, 
		char *table, 
		char *name, 
		char *data, 
		int num)		/* remove [table] [name] [data] [num] */
{
	char command[4096] = "";
	char recv_buf[13] = "";

	snprintf(command, 4096, "remove_group %s %s %s %d", table, name, data, num);

	if (0 >= send(target->csocket, command, strlen(command), MSG_NOSIGNAL))
	{
		return FDATA_FAILURE;
	}

	if (0 >= recv(target->csocket, recv_buf, 13, 0))
	{
		return FDATA_FAILURE;
	}
	if (0 == strcmp(recv_buf, "successfully"))
	{
		return FDATA_SUCCESS;
	}

	return FDATA_FAILURE;
}

int fdata_remove_table(FDATA *target, 
		char *table)		/* remove_table [table] */
{
	char command[4096] = "";
	char recv_buf[13] = "";

	snprintf(command, 4096, "remove_table %s", table);

	if (0 >= send(target->csocket, command, strlen(command), MSG_NOSIGNAL))
	{
		return FDATA_FAILURE;
	}

	if (0 >= recv(target->csocket, recv_buf, 13, 0))
	{
		return FDATA_FAILURE;
	}
	if (0 == strcmp(recv_buf, "successfully"))
	{
		return FDATA_SUCCESS;
	}

	return FDATA_FAILURE;
}
