#include <unistd.h>
#include "io.h"
#include "m_socket.h"
#include "version.h"
#include <errno.h>

#define CONFIG_DEFAULT_PATH	"/etc/fdata.conf"

/* variable that can export */
int e_port = 8000;
int e_max_connection = 1;
int e_backlog = 511;
/* End */

/* imported variable */

extern char login_info_path[1024];

/* end */

void parser_options(int, char **);
void parser_config(char *);
void print_version();
void print_help();
int init_socket();

extern void processor(int);

int main(int argc, char *argv[])
{
	int lsocket;
	int fork_ret;
	parser_options(argc, argv);

	if ((fork_ret = fork()) == -1)
	{
		printf("Create a new processor unsuccessfully.\n");
		exit(EXIT_FAILURE);
	}
	else if (fork_ret == 0)
	{
		 if ((lsocket = init_socket()) == -1)
		 {
			 printf("Initialize socket unsuccessfully.\n");
			 printf("Error message: %s\n", strerror(errno));
			 exit(EXIT_FAILURE);
		 }
		 processor(lsocket);
	}

	return 0;
}

void print_version()
{
	printf("%s\n", FDATA_VER);
}

void print_help()
{
	print_version();
	printf("\n");
	printf("Options:\n");
	printf("\t\t-h: print this help.\n");
	printf("\t\t-v: print version.\n");
	printf("\t\t-c: specify configuration file.\n");
	printf("\n");
}

int init_socket()
{
	int tsocket;
	int sock_opt = 1;
	struct sockaddr_in addr;

	if ((tsocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		return -1;
	}

	if (-1 == setsockopt(tsocket, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt)))
	{
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(e_port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (-1 == bind(tsocket, (struct sockaddr *)&addr, sizeof(addr)))
	{
		return -1;
	}

	if (-1 == listen(tsocket, e_backlog))
	{
		return -1;
	}

	return tsocket;
}

void parser_options(int argc, char *argv[])
{
	char config_path[1024];
	char ch;

	memcpy(config_path, CONFIG_DEFAULT_PATH, 1024);
	while ((ch = getopt(argc, argv, "hvc:")) != -1)
	{
		switch(ch)
		{
			case 'h':
				print_help();
				exit(EXIT_SUCCESS);
				break;
			case 'v':
				print_version();
				exit(EXIT_SUCCESS);
				break;
			case 'c':
				memcpy(config_path, optarg, 1024);
				break;
			case '?':
				printf("Unknown command.\n");
				exit(EXIT_FAILURE);
				break;
		}
	}

	parser_config(config_path);
}

void parser_config(char *config_path)
{
	FILE *file_H = NULL;
	char line_string[1024] = "";
	char method[100] = "";
	char argu[924] = "";
	int line_count = 1;

	file_H = fopen(config_path, "r");
	if (file_H == NULL)
	{
		printf("Open file unsuccessfully.\n");
		exit(EXIT_FAILURE);
	}

	while (fgets(line_string, 1024, file_H) != NULL)
	{
		if (line_string[0] == '\n' ||
		    line_string[0] == '!')
		{
			continue;
		}

		if (2 > sscanf(line_string, "%s %s", method, argu))
		{
			printf("Line: %d, syntax error.\n", line_count);
			exit(EXIT_FAILURE);
		}

		if (argu[strlen(argu)-1] == '\n')
		{
			argu[strlen(argu)-1] = '\0';
		}

		if (0 == strcmp(method, "port"))
		{
			e_port = atoi(argu);
		}
		else if (0 == strcmp(method, "home_dir"))
		{
			if (-1 == chdir(argu))
			{
				printf("Cannot switch to %s\n", argu);
				printf("Error message: %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		else if (0 == strcmp(method, "max_connection"))
		{
			e_max_connection = atoi(argu);
		}
		else if (0 == strcmp(method, "backlog"))
		{
			e_backlog = atoi(argu);
		}
		else if (0 == strcmp(method, "login_info_path"))
		{
			memcpy(login_info_path, argu, 1024);
		}
		else
		{
			printf("Line: %d, unknown command.\n", line_count);
			exit(EXIT_FAILURE);
		}

		line_count++;
	}

	fclose(file_H);
}
