#include "ss_config.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

extern ss_char_t error_file_404[1024];
extern ss_char_t error_file_502[1024];

void senderror_404(ss_int_t send_socket)
{
	FILE      *error_file_handle = NULL;
	ss_int_t  read_bytes;
	ss_uint_t file_size;
	ss_char_t read_string[1024];
	ss_char_t header[200];
	ss_char_t send_msg[1024];
	struct stat file_info;

	if (0 != strlen(error_file_404))
	{
		if (-1 == lstat(error_file_404, &file_info))
		{
			goto normal_send_error; /*Read file error
						 send built-in error message*/
		}

		file_size = file_info.st_size;

		(void)snprintf(header, sizeof(header), "\
HTTP/1.1 404 Not Found\r\n\
Content-Length: %d\r\n\
Content-type: text/html\r\n\r\n", file_size);

		/*Ignore error. The same below.*/
		(void)send(send_socket, header, strlen(header), MSG_NOSIGNAL);
		
		error_file_handle = fopen(error_file_404, "r");
		if (error_file_handle == NULL)
		{
			goto normal_send_error;
		}

		while ((read_bytes = (ss_int_t)fread(read_string,
						1,
						sizeof(read_string),
						error_file_handle)) > 0)
		{
			(void)send(send_socket, read_string, (size_t)read_bytes, MSG_NOSIGNAL);
		}

		(void)fclose(error_file_handle);
	}
	else
	{

normal_send_error:
		memcpy(send_msg, "\
<html>\n\
<head><title>404 Not Found</title></head>\n\
<body bgcolor=\"white\">\n\
<center><h1>404 Not Found</h1></center>\n\
<hr><center>stableser</center></hr>\n\
</body>\n\
</html>", sizeof(send_msg));
	
		(void)snprintf(header, sizeof(header), "\
HTTP/1.1 404 Not Found\r\n\
Content-Length: %d\r\n\
Content-type: text/html\r\n\r\n", (int)strlen(send_msg));

		(void)send(send_socket, header, strlen(header), MSG_NOSIGNAL);
		(void)send(send_socket, send_msg, strlen(send_msg), MSG_NOSIGNAL);
	}
}

void senderror_502(ss_int_t send_socket)
{
	FILE      *error_file_handle = NULL;
	ss_int_t  read_bytes;
	ss_uint_t file_size;
	ss_char_t read_string[1024];
	ss_char_t header[200];
	ss_char_t send_msg[1024];
	struct stat file_info;

	if (0 != strlen(error_file_502))
	{
		if (-1 == lstat(error_file_502, &file_info))
		{
			goto normal_send_error;
		}

		file_size = file_info.st_size;

		if (-1 == snprintf(header, sizeof(header), "HTTP/1.1 502 Bad Gateway\r\n\
				 Content-Length: %d\r\n\
				 Content-type: text/html\r\n\r\n", file_size))
		{
			return;
		}

		(void)send(send_socket, header, sizeof(header), MSG_NOSIGNAL);

		error_file_handle = fopen(error_file_502, "r");
		if (error_file_handle == NULL)
		{
			goto normal_send_error;
		}

		while ((read_bytes = (ss_int_t)fread(read_string, 
						1, 
						sizeof(read_string), 
						error_file_handle)) > 0)
		{
			(void)send(send_socket, read_string, read_bytes, MSG_NOSIGNAL);
		}

		(void)fclose(error_file_handle);
	}
	else
	{

normal_send_error:
		memcpy(send_msg, "\
<html>\n\
<head><title>502 Bad Gateway</title></head>\n\
<body bgcolor=\"white\">\n\
<center><h1>502 Bad Gateway</h1></center>\n\
<hr><center>stableser</center></hr>\n\
</body>\n\
</html>", sizeof(send_msg));

		(void)snprintf(header, sizeof(header), "\
HTTP/1.1 502 Bad Gateway\r\n\
Content-Length: %d\r\n\
Content-type: text/html\r\n\r\n", (int)strlen(send_msg));

		(void)send(send_socket, header, strlen(header), MSG_NOSIGNAL);
		(void)send(send_socket, send_msg, strlen(send_msg), MSG_NOSIGNAL);
	}
}
