#include "ss_config.h"
#include "ss_func.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_CONNECTION 511

static ss_int_t init_socket(ss_int_t);
static void     child_quit(int);

extern ss_int_t target_port;
extern ss_int_t backlog_number;

void ss_worker()
{
	ss_int_t  connect_socket;
	ss_int_t  listen_socket;
	ss_int_t  fork_ret;
	ss_int_t  child_fork_ret;

	if ((fork_ret = fork()) < 0)
	{
		fprintf(stderr, "Create process failure: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	else if (fork_ret == 0)
	{
		/*Ignored any error. The same below.*/
		(void)signal(SIGCHLD, child_quit);

		if (-1 == (connect_socket = init_socket(target_port)))
		{
			perror("initialize socket");
			exit(EXIT_FAILURE);
		}

		for (;;)
		{
			listen_socket = accept(connect_socket, NULL, 0);
			if (listen_socket == -1)
			{
				continue;
			}

			if ((child_fork_ret = fork()) == 0)
			{
				ss_processor(listen_socket);
				_exit(EXIT_SUCCESS);
			}
			else if (child_fork_ret < 0)
			{
				senderror_502(listen_socket);
				(void)close(listen_socket);
				continue;
			}
			else
			{
				(void)close(listen_socket);
			}
		}
	}
}

static ss_int_t init_socket(ss_int_t port)
{
	ss_int_t temp_socket;
	ss_int_t opt = 1;
	struct sockaddr_in addr;

	if (-1 == (temp_socket = socket(AF_INET, SOCK_STREAM, 0)))
	{
		return -1;
	}

	if (setsockopt(temp_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (-1 == bind(temp_socket, (struct sockaddr *)&addr, sizeof(addr)))
	{
		return -1;
	}

	if (-1 == listen(temp_socket, backlog_number))
	{
		return -1;
	}

	return temp_socket;
}

static void child_quit(int status)
{
	int child_status = 0;

	while (waitpid(-1, &child_status, WNOHANG) > 0);

}
