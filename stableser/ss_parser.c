#include "ss_config.h"
#include "ss_func.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>

/*hash*/
#ifdef HASH_SEARCH
hash_table *type_table;
#endif
/**/

ss_int_t  target_port = 8000;
ss_int_t  backlog_number = 511;

ss_char_t cgi_bin_path[100] = "";

ss_char_t error_file_404[1024] = "";
ss_char_t error_file_502[1024] = "";

void	  *module_handle;
ss_char_t module_path[1024] = "";
ss_int_t  (*process_func)(int, char *); /*int socket, char recv_string*/

ss_char_t file_type[1000][100];
ss_char_t send_type[1000][100];

ss_char_t body_command[1000];

ss_int_t  worker_processes = 4;

struct timeval timeout = {3, 0}; /* Default, the timeout value is 3s */

/*private*/
ss_int_t  line_count = 1;

static void ss_parse_port();
static void ss_parse_home_dir();
static void ss_parse_addtype();
static void ss_parse_sendtype();
static void ss_parse_backlog();
static void ss_parse_error_404();
static void ss_parse_error_502();
static void ss_parse_module();
static void ss_parse_cgi_bin_path();
static void ss_parse_worker_processes();
static void ss_parse_timeout();

void ss_parse_config(ss_char_t *path)
{
	FILE      *file_handle = NULL;
	ss_char_t line_string[1024];
	ss_char_t *temp;
	ss_int_t  is_hash_existing = 0;

	file_handle = fopen(path, "r");
	if (file_handle == NULL)
	{
		perror("Read file failure");
		fprintf(stderr, "Please check your path,\n");
		fprintf(stderr, "or use option -c to specify path.\n");
		exit(EXIT_FAILURE);
	}

	while (fgets(line_string, sizeof(line_string), file_handle) != NULL)
	{
		if (0 == strncmp(line_string, "!", 1) || \
		    0 == strncmp(line_string, "\n", 1) || \
		    0 == strncmp(line_string, "\r", 1))
		{
			line_count++;
			continue;
		}

		temp = strstr(line_string, " ");
		if (temp == NULL)
		{
			memcpy(body_command, "", sizeof(body_command));
			goto skip_check; /*If don't jump, then
					   will have segmentation fault.*/
		}

		memcpy(body_command, temp + 1, sizeof(body_command));
		if (0 == strlen(body_command))
		{
			goto skip_check;
		}

		if (body_command[strlen(body_command)-1] == '\n')
		{
			body_command[strlen(body_command)-1] = '\0';
		}

skip_check:

		if (0 == strncmp(line_string, "port", 4))
		{
			ss_parse_port();
		}
		else if (0 == strncmp(line_string, "home_dir", 8))
		{
			ss_parse_home_dir();
		}
		else if (0 == strncmp(line_string, "backlog", 7))
		{
			ss_parse_backlog();
		}
		else if (0 == strncmp(line_string, "set_error_404", 13))
		{
			ss_parse_error_404();
		}
		else if (0 == strncmp(line_string, "set_error_502", 13))
		{
			ss_parse_error_502();
		}
		else if (0 == strncmp(line_string, "module", 6))
		{
			ss_parse_module();
		}
		else if (0 == strncmp(line_string, "cgi_bin_path", 12))
		{
			ss_parse_cgi_bin_path();
		}
		else if (0 == strncmp(line_string, "addtype", 7))
		{
			is_hash_existing++;
			ss_parse_addtype();
		}
		else if (0 == strncmp(line_string, "sendtype", 8))
		{
			is_hash_existing++;
			ss_parse_sendtype();
		}
		else if (0 == strncmp(line_string, "worker_processes", 16))
		{
			ss_parse_worker_processes();
		}
		else if (0 == strncmp(line_string, "timeout", 7))
		  {
		    ss_parse_timeout();
		  }
		else
		{
			fprintf(stderr, "Line: %d\nUnknown command.\n", line_count);
			exit(EXIT_FAILURE);
		}

		line_count++;
	}

#ifdef	HASH_SEARCH
	int hash_count;

	hash_init(&type_table);

	if (is_hash_existing == 2)		/* Means that can use hash_table */
	{
		for (hash_count = 0; 
				0 != strlen(file_type[hash_count]) &&
				0 != strlen(send_type[hash_count]); 
				hash_count++)
		{
			hash_insert(type_table, file_type[hash_count], 
						send_type[hash_count]);
		}

	}

#endif

	(void)fclose(file_handle);
}

static void ss_parse_port()
{
	target_port = atoi(body_command);
}

static void ss_parse_home_dir()
{
	if (-1 == chdir(body_command))
	{
		perror("chdir");
		fprintf(stderr, "Please check your path.\n");
		exit(EXIT_FAILURE);
	}
}

static void ss_parse_backlog()
{
	backlog_number = atoi(body_command);
}

static void ss_parse_addtype()
{
	ss_char_t  *get_str;
	ss_int_t    count = 0;

	get_str = strtok(body_command, " ");
	if (get_str == NULL)
	{
		fprintf(stderr, "Line: %d\nMissing options.\n", line_count);
		exit(EXIT_FAILURE);
	}

	(void)snprintf(file_type[count], sizeof(file_type[count]), "%s", get_str);
	count++;

	for (; NULL != (get_str = strtok(NULL, " ")); count++)
	{
		(void)snprintf(file_type[count], sizeof(file_type[count]), "%s", get_str);
	}
}

static void ss_parse_sendtype()
{
	ss_char_t *get_str;
	ss_int_t  count = 0;

	get_str = strtok(body_command, " ");
	if (get_str == NULL)
	{
		fprintf(stderr, "Line: %d\nMissing options.\n", line_count);
		exit(EXIT_FAILURE);
	}
	
	(void)snprintf(send_type[count], sizeof(send_type[count]), "%s", get_str);
	count++;

	for (; NULL != (get_str = strtok(NULL, " ")); count++)
	{
		(void)snprintf(send_type[count], sizeof(send_type[count]), "%s", get_str);
	}
}

static void ss_parse_error_404()
{
	memcpy(error_file_404, body_command, sizeof(error_file_404));
}

static void ss_parse_error_502()
{
	memcpy(error_file_502, body_command, sizeof(error_file_502));
}

static void ss_parse_module()
{
	memcpy(module_path, body_command, sizeof(module_path));
	module_handle = dlopen(module_path, RTLD_NOW);
	if (module_handle == NULL)
	{
		fprintf(stderr, "Link to %s error: %s\n", module_path, dlerror());
		fprintf(stderr, "Please check your path.\n");
		exit(EXIT_FAILURE);
	}

	process_func = dlsym(module_handle, "request_processor");
}

static void ss_parse_cgi_bin_path()
{
	memcpy(cgi_bin_path, body_command, sizeof(cgi_bin_path));
}

static void ss_parse_worker_processes()
{
  worker_processes = atoi(body_command);
}

static void ss_parse_timeout()
{
  timeout.tv_sec = atoi(body_command);
}

