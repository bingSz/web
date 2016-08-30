/*We will ignore some error about result.*/
#include "ss_config.h"
#include "ss_func.h"

#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#define ss_post_static(listen_socket, real_path) \
	ss_get_static(listen_socket, real_path)

/*hash*/
#ifdef	HASH_SEARCH
extern hash_table *type_table;
#endif
/**/

extern ss_int_t  target_port;
extern ss_char_t **environ;
extern ss_char_t file_type[1000][100];
extern ss_char_t send_type[1000][100];
extern ss_char_t cgi_bin_path[100];

extern ss_char_t module_path[1024];
extern ss_int_t	 (*process_func)(int, char *); /*int socket, char recv_string*/

static ss_char_t *ss_get_file_type(ss_char_t *);
static void ss_get_static(ss_int_t, ss_char_t *);
static void ss_get_dynamic(ss_int_t, ss_char_t *, ss_char_t *);
static void ss_post_dynamic(ss_int_t, ss_char_t *, ss_char_t *);
static void set_based_env(ss_char_t *, ss_char_t *);
static void set_based_env_get(ss_char_t *, ss_char_t *, ss_char_t *);

void ss_processor(ss_int_t listen_socket)
{
	ss_int_t  module_ret;
	ss_char_t recv_string[1024] = "";
	ss_char_t method[50];
	ss_char_t request_path[10000];
	ss_char_t procotol[10];
	ss_char_t real_path[10001];

	alarm(5);		/* set timeout */

	if (0 < recv(listen_socket, recv_string, sizeof(recv_string), 0))
	{
		alarm(0);	/* stop alarm */

		if (0 != strlen(module_path))
		{
			if (-1 == (module_ret = process_func(listen_socket, recv_string))) /*return -1 means 502 error occurs.*/
			{
				senderror_502(listen_socket);
				(void)close(listen_socket);
				_exit(EXIT_FAILURE);
			}
			else if (module_ret == 0) /*Return 1 means continue execution.
						    If not return 1, then exit.*/
			{
				(void)close(listen_socket);
				_exit(EXIT_SUCCESS);
			}
		}

		sscanf(recv_string, "%s %s %s", method, request_path, procotol);
		if (strlen(request_path) == 0) /*request is NULL*/
		{
			(void)close(listen_socket);
			_exit(EXIT_FAILURE);
		}

		(void)snprintf(real_path, sizeof(real_path), ".%s", request_path);
		
		if (0 != strlen(cgi_bin_path))
		{
			if (NULL != strstr(request_path, cgi_bin_path))
			{
		 
				if (0 == strcasecmp(method, "GET"))
				{
					ss_get_dynamic(listen_socket, real_path, recv_string);
				}
				else if (0 == strcasecmp(method, "POST"))
				{
					ss_post_dynamic(listen_socket, real_path, recv_string);
				}
				goto end; /*Next is process static.*/
			}
		}
		else
		{
			if (NULL != strstr(request_path, "cgi-bin"))
			{
				
				if (0 == strcasecmp(method, "GET"))
				{
					ss_get_dynamic(listen_socket, real_path, recv_string);
				}
				else if (0 == strcasecmp(method, "POST"))
				{
				        ss_post_dynamic(listen_socket, real_path, recv_string);
				}
				goto end; /*Next is process static.*/
			}
		}

		if (0 == strcasecmp(method, "GET"))
		{
			ss_get_static(listen_socket, real_path);
		}
		else if (0 == strcasecmp(method, "POST"))
		{
			ss_post_static(listen_socket, real_path);
		}
	}
	else
	{
		(void)close(listen_socket);
		_exit(EXIT_FAILURE);
	}

end:
	(void)close(listen_socket);
	_exit(EXIT_SUCCESS);
}

static ss_char_t *ss_get_file_type(ss_char_t *file_path)
{

#ifdef	HASH_SEARCH

	return hash_search(type_table, strstr(file_path, "."));

#else
	ss_int_t count;
	
	for (count = 0; file_type[count] != NULL; count++)
	{
		if (NULL != strstr(file_path, file_type[count]))
		{
			return send_type[count];
		}
	}

	return "";
#endif

}

static void ss_get_static(ss_int_t listen_socket, ss_char_t *real_path)
{
  /*ss_int_t  read_bytes;*/
   	ss_uint_t file_size;
	ss_char_t *send_str;
	ss_char_t *file_type;
    	ss_char_t header[1024];
    	/*ss_char_t read_string[1024];*/
    	FILE      *target_file_handle;
    	struct stat file_info;
	
	target_file_handle = fopen(real_path, "r");
    	if (target_file_handle == NULL)
    	{
	  strncat(real_path, "/index.html", 1001);
	  target_file_handle = fopen(real_path, "r");
	  if (target_file_handle == NULL)
	    {
	      senderror_404(listen_socket);
	      _exit(EXIT_FAILURE);
	    }
    	}

	if (-1 == fstat(fileno(target_file_handle), &file_info))
	  {
	    senderror_502(listen_socket);
	    _exit(EXIT_FAILURE);
	  }

	if (S_ISDIR(file_info.st_mode))
	  {
	    (void)fclose(target_file_handle);
	    strncat(real_path, "/index.html", 1001);
	    target_file_handle = fopen(real_path, "r");
	    if (target_file_handle == NULL)
	      {
		senderror_404(listen_socket);
		_exit(EXIT_FAILURE);
	      }

	    if (-1 == fstat(fileno(target_file_handle), &file_info))
	      {
		senderror_502(listen_socket);
		_exit(EXIT_FAILURE);
	      }
	    
	  }
	
	file_size = file_info.st_size;

	file_type = ss_get_file_type(real_path);
	if (0 == strlen(file_type))
	  {
	    file_type = "application/octet-stream";
	  }

	(void)snprintf(header, sizeof(header), "\
HTTP/1.1 200 OK\r\n\
Content-Length: %u\r\n\
Content-type: %s\r\n\r\n", file_size, file_type);
	(void)send(listen_socket, header, strlen(header), MSG_NOSIGNAL);

	send_str = (char *)mmap(0, file_size, PROT_READ, MAP_PRIVATE,
				fileno(target_file_handle), 0);
	(void)send(listen_socket, send_str, file_size, MSG_NOSIGNAL);
	munmap(send_str, file_size);
	
    	/*
	while ((read_bytes = fread(read_string, 
			       1, 
			       sizeof(read_string), 
			       target_file_handle)) > 0)
    	{
		if (-1 == send(listen_socket, read_string, read_bytes, MSG_NOSIGNAL))
		{
			(void)close(listen_socket);
			_exit(EXIT_FAILURE);
		}
    	}
	*/
	
    	(void)fclose(target_file_handle);
}

static void ss_get_dynamic(ss_int_t listen_socket, ss_char_t *real_path, ss_char_t *recv_string)
{
	ss_char_t read_path[1000];
	ss_char_t *parameter;
	struct stat file_info;
	
	if (NULL != (parameter = strstr(real_path, "?")))
	{
		parameter++;
		memcpy(read_path, real_path, strlen(real_path) - strlen(parameter) - 1);
		set_based_env_get("GET", recv_string, parameter);
	}
	else
	{
		memcpy(read_path, real_path, sizeof(read_path));
		set_based_env_get("GET", recv_string, "");
	}

	if (-1 == lstat(read_path, &file_info))
	{
	    	senderror_502(listen_socket);
		(void)close(listen_socket);
		_exit(EXIT_FAILURE);
	}
	if (S_ISDIR(file_info.st_mode))
	{
		ss_get_static(listen_socket, read_path);
	}

	if (-1 == setenv("QUERY_STRING", parameter, 1))
	{
		senderror_502(listen_socket);
		(void)close(listen_socket);
		_exit(EXIT_FAILURE);
	}

	if (-1 == dup2(listen_socket, STDOUT_FILENO))
	{
		senderror_502(listen_socket);
		(void)close(listen_socket);
		_exit(EXIT_FAILURE);
	}
	if (-1 == dup2(listen_socket, STDERR_FILENO))
	{
		senderror_502(listen_socket);
		(void)close(listen_socket);
		_exit(EXIT_FAILURE);
	}

	if (-1 == access(read_path, F_OK))
	{
		senderror_404(listen_socket);
		(void)close(listen_socket);
		_exit(EXIT_FAILURE);
	}
	(void)send(listen_socket,
	     "HTTP/1.1 200 OK\r\n",
	     strlen("HTTP/1.1 200 OK\r\n"),
	     MSG_NOSIGNAL);
	/*if (-1 == execve(read_path, argv, environ))*/
	if (-1 == execl(read_path, read_path, (char *)NULL))
	{
		senderror_502(listen_socket);
		(void)close(listen_socket);
		_exit(EXIT_FAILURE);
	}

	(void)close(listen_socket);
	_exit(EXIT_SUCCESS);
}

static void ss_post_dynamic(ss_int_t listen_socket, ss_char_t *real_path, ss_char_t *recv_string)
{
	ss_char_t *parameter;
	ss_char_t *argv[] = {real_path, NULL};
	struct stat file_info;

	if (-1 == lstat(real_path, &file_info))
	{
		senderror_502(listen_socket);
		(void)close(listen_socket);
		_exit(EXIT_FAILURE);
	}
	if (S_ISDIR(file_info.st_mode))
	{
		ss_post_static(listen_socket, real_path);
	}

	if (NULL != (parameter = strstr(recv_string, "\r\n\r\n")))
	{
		parameter += 4;
	}

	set_based_env("POST", recv_string);

	setenv("QUERY_STRING", parameter, 1);

	if (-1 == dup2(listen_socket, STDOUT_FILENO))
	{
		senderror_502(listen_socket);
		(void)close(listen_socket);
		_exit(EXIT_FAILURE);
	}
	if (-1 == dup2(listen_socket, STDERR_FILENO))
	{
		senderror_502(listen_socket);
		(void)close(listen_socket);
		_exit(EXIT_FAILURE);
	}

	if (-1 == access(real_path, F_OK))
	{
		senderror_404(listen_socket);
		(void)close(listen_socket);
		_exit(EXIT_FAILURE);
	}
	(void)send(listen_socket,
	     "HTTP/1.1 200 OK\r\n",
	     strlen("HTTP/1.1 200 OK\r\n"),
	     MSG_NOSIGNAL);
	if (-1 == execve(real_path, argv, environ))
	{
		senderror_502(listen_socket);
		(void)close(listen_socket);
		_exit(EXIT_FAILURE);
	}
	
	(void)close(listen_socket);
	_exit(EXIT_SUCCESS);
}

static void set_based_env_get(ss_char_t *method, ss_char_t *recv_string, ss_char_t *parameter)
{
	ss_char_t host_name[255];
        ss_char_t length[255];
	ss_char_t port_string[10];
	ss_char_t parameter_temp[1024];

	memcpy(parameter_temp, parameter, sizeof(parameter_temp));
	
	(void)snprintf(length, sizeof(length), "%d", (int)strlen(parameter_temp));

	(void)snprintf(port_string, sizeof(port_string), "%d", target_port);

	if (-1 == gethostname(host_name, sizeof(host_name)))
	{
		memcpy(host_name, "", sizeof(host_name));
	}
	
      	setenv("REQUEST_METHOD", method, 1);
	setenv("SERVER_SOFTWARE", SS_VER, 1);
	setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
	setenv("SERVER_PORT", port_string, 1);
	setenv("SERVER_NAME", host_name, 1);
	setenv("CONTENT_LENGTH", length, 1);

}

static void set_based_env(ss_char_t *method, ss_char_t *recv_string)
{
	ss_char_t host_name[255];
	ss_char_t length[255];
	ss_char_t length_string_temp[255];
	ss_char_t length_string[255];
	ss_char_t port_string[10];
	ss_char_t *temp;

	temp = strcasestr(recv_string, "Content-Length");
	if (temp == NULL)
	{
		memcpy(length, "0", sizeof(length));
		goto skip_copy; /*If the length_string is NULL, then
				the next step will error. The same below.*/
	}

	memcpy(length_string, temp, sizeof(length_string));

	if (0 == strlen(length_string))
	{
		memcpy(length, "0", sizeof(length));
		goto skip_copy; 
	}

	temp = strtok(length_string, "\r\n");
	if (temp == NULL)
	{
		memcpy(length, "0", sizeof(length));
		goto skip_copy;
	}

	memcpy(length_string_temp, temp, sizeof(length_string_temp));
	
	if (0 == sscanf(length_string_temp, "Content-Length: %s", length) && 
	    0 == sscanf(length_string_temp, "Content-length: %s", length) &&
	    0 == sscanf(length_string_temp, "content-Length: %s", length) && 
	    0 == sscanf(length_string_temp, "content-length: %s", length))
	{
		memcpy(length, "0", sizeof(length));
	}

skip_copy:

	(void)snprintf(port_string, sizeof(port_string), "%d", target_port);
	
	if (-1 == gethostname(host_name, sizeof(host_name)))
	{
		memcpy(host_name, "", sizeof(host_name));
	}

	setenv("REQUEST_METHOD", method, 1);
	setenv("SERVER_SOFTWARE", SS_VER, 1);
	setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
	setenv("SERVER_PORT", port_string, 1);
	setenv("SERVER_NAME", host_name, 1);
	setenv("CONTENT_LENGTH", length, 1);
}
