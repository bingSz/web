#include "io.h"
#include "fdata_login.h"
#include "m_socket.h"
#include <sys/stat.h>

extern char login_info_path[1024];

static login_info major_info;

int init_login_info(char *path)
{
	FILE *file_H = NULL;
	char *line_string = NULL;
	char *temp;
	struct stat file_info = {0};

	file_H = fopen(path, "a+");
	if (file_H == NULL)
	{
		return EXIT_FAILURE;
	}

	if (-1 == fstat(fileno(file_H), &file_info))
	{
		fprintf(stderr, "Get file size failure.\n");
		exit(EXIT_FAILURE);
	}

	line_string = (char *)calloc(sizeof(char), file_info.st_size + 1);

	if (fgets(line_string, file_info.st_size, file_H) == NULL)
	{
		if (10 > fwrite("root root\n", 1, 10, file_H))
		{
			return EXIT_FAILURE;
		}
	}

	if (line_string[strlen(line_string)-1] == '\n')
	{
		line_string[strlen(line_string)-1] = '\0';
	}

	if (major_info.username != NULL)
	{
		free(major_info.username);
	}
	if (major_info.password != NULL)
	{
		free(major_info.password);
	}

	temp = strtok(line_string, " ");
	if (temp == NULL)
	{
		return EXIT_FAILURE;
	}
	major_info.username = (char *)calloc(sizeof(char), strlen(temp) + 1);
	memcpy(major_info.username, temp, strlen(temp));
	
	temp = strtok(NULL, " ");
	if (temp == NULL)
	{
		return EXIT_FAILURE;
	}
	major_info.password = (char *)calloc(sizeof(char), strlen(temp) + 1);
	memcpy(major_info.password, temp, strlen(temp));

	fclose(file_H);

	free(line_string);
	line_string = NULL;

	return EXIT_SUCCESS;
}

int login_table(int csocket)
{
	char login_command[10000];
	char method[5] = "";
	char *username;
	char *password;
	char *temp_str;

	memset(login_command, 0, sizeof(login_command));
	if (0 >= recv(csocket, login_command, 10000, 0))
	{
		return LOGIN_FAILURE;
	}

	if (0 == strlen(major_info.username) ||
	    0 == strlen(major_info.password))
	{
		init_login_info(login_info_path);
	}

	temp_str = strtok(login_command, " ");
	if (temp_str == NULL)
	{
		return LOGIN_FAILURE;
	}
	memcpy(method, temp_str, 5);

	if (0 == strcmp(method, "login"))
	{
		temp_str = NULL;

		/* Get username and password. */
		temp_str = strtok(NULL, " ");
		if (temp_str == NULL)
		{
			return LOGIN_FAILURE;
		}
		username = (char *)calloc(sizeof(char), strlen(temp_str) + 1);
		memcpy(username, temp_str, strlen(temp_str));

		temp_str = NULL;

		temp_str = strtok(NULL, " ");
		if (temp_str == NULL)
		{
			return LOGIN_FAILURE;
		}
		password = (char *)calloc(sizeof(char), strlen(temp_str) + 1);
		memcpy(password, temp_str, strlen(temp_str));
		/* End */

		if (0 == strcmp(username, major_info.username) && 0 == strcmp(password, major_info.password))
		{
			free(username);
			free(password);
			return LOGIN_SUCCESS;
		}
		else
		{
			free(username);
			free(password);
			return LOGIN_FAILURE;
		}
	}

	return LOGIN_SUCCESS;
}

int passwd_user(char *path, char *old_password, char *new_password)
{
	FILE *file_H = NULL;

	if (0 == strcmp(major_info.password, old_password))
	{
		file_H = fopen(path, "w+");
		if (file_H == NULL)
		{
			return -1;
		}

		fprintf(file_H, "root %s\n", new_password);

		fclose(file_H);
		
		major_info.password = strdup(new_password);
	}
	else
	{
		return -1;
	}
	
	return 0;
}
