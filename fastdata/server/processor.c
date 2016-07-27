#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include "io.h"
#include "hash_table.h"
#include "m_socket.h"
#include "fdata_login.h"
#include <errno.h>
#include <dirent.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>

pthread_key_t hash_key;
pthread_mutex_t hash_mutex;
hash_table *database_data[HASH_TABLE_SIZE];

void process_request(void *);
void kill_child(int);
void init_data(hash_table **);
static void init_text_data(hash_table **);
unsigned int hash_table_index(char *);

extern int e_max_connection;
int connection_counter = 0;

/* Variable that can export */

char login_info_path[1024];

/* End */

void processor(int lsocket)
{
	pthread_t id;
	int clsocket;
	int i;

	signal(SIGCHLD, kill_child);
	pthread_mutex_init(&hash_mutex, NULL);
	pthread_key_create(&hash_key, NULL);

	for (i = 0; i < HASH_TABLE_SIZE; i++)
	{
		hash_init(&database_data[i]);
	}

	init_data(database_data);
	if (0 != strlen(login_info_path))
	{
		if (EXIT_FAILURE == init_login_info(login_info_path))
		{
			fprintf(stderr, "Initialize login information unsuccessfully.\n");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		if (EXIT_FAILURE == init_login_info("/etc/fdata_user.conf"))
		{
			fprintf(stderr, "Initialize login information unsuccessfully.\n");
			exit(EXIT_FAILURE);
		}
	}
	
	for (;;)
	{
		clsocket = accept(lsocket, NULL, NULL);
		if (clsocket == -1)
		{
			continue;
		}

		if (connection_counter >= e_max_connection)
		{
			close(clsocket);
			continue;
		}

		connection_counter++;

		pthread_create(&id, NULL, (void *)process_request, (void *)&clsocket);
	}
}


void init_data(hash_table **database_data)
{
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;

	dp = opendir("./");

	while ((entry = readdir(dp)) != NULL)
	{
		lstat(entry->d_name, &statbuf);
		if (0 == strcmp(entry->d_name, ".") ||
		    0 == strcmp(entry->d_name, ".."))
		{
			continue;
		}

		if (S_ISDIR(statbuf.st_mode))
		{
			if (-1 == chdir(entry->d_name))
			{
				exit(EXIT_FAILURE);
			}
			init_text_data(database_data);
			if (-1 == chdir("../"))
			{
				exit(EXIT_FAILURE);
			}
		}
	}

	closedir(dp);
}

static void init_text_data(hash_table **database_data)
{
	FILE *file_H = NULL;
	char line_string[4096] = "";
	char insert_data[100][1024];
	char *temp_data;
	char *mmap_address;
	char data_table_name[100] = "";
	char data_name[100] = "";
	unsigned int data_index;
	int i;

	struct stat file_info;
	struct dirent *entry;
	DIR *dp;

	memset(insert_data, 0, sizeof(insert_data));

	if (NULL == (dp = opendir("./")))
	{
		exit(EXIT_FAILURE);
	}

	while ((entry = readdir(dp)) != NULL)
	{

		if (0 == strcmp(entry->d_name, ".") || 
		    0 == strcmp(entry->d_name, ".."))
		{
			continue;
		}

		file_H = fopen(entry->d_name, "r");
		if (file_H == NULL)
		{
			continue;
		}
	
		memset(line_string, 0, sizeof(line_string));
	
		fstat(fileno(file_H), &file_info);
	
		mmap_address = mmap(NULL, file_info.st_size, PROT_READ, MAP_PRIVATE, fileno(file_H), 0);
		memcpy(line_string, mmap_address, sizeof(line_string));
	
		/* Insert the data. */
	
		memset(insert_data, 0, sizeof(insert_data));
		if ((temp_data = strtok(line_string, " ")) == NULL)
		{
			break;
		}
		memcpy(data_table_name, temp_data, 100);
		
		if ((temp_data = strtok(NULL, " ")) == NULL)
		{
			break;
		}
		memcpy(data_name, temp_data, 100);
	
		data_index = hash_table_index(data_table_name);

		for (i = 0; i < HASH_DEFAULT_ITEM_SIZE; i++)
		{
			temp_data = NULL;
	
			if ((temp_data = strtok(NULL, " ")) == NULL)
			{
				break;
			}
			
			memcpy(insert_data[i], temp_data, 1024);
		}
		hash_insert(database_data[data_index], data_name, insert_data);
		memset(line_string, 0, sizeof(line_string));
		
		/* End */
	
		munmap(mmap_address, file_info.st_size);
		fclose(file_H);
	}
	
		closedir(dp);
}

void kill_child(int status)
{
	int child_status = 0;

	while (waitpid(-1, &child_status, WNOHANG) > 0);
}

unsigned int hash_table_index(char *str)
{
	unsigned int h = 1;

	while (*str != 0)
	{
		h = *str + h * 31;
		(void)*str++;
	}

	return h % HASH_TABLE_SIZE;
}

void child_thread_error(int status)
{
	connection_counter--;
	if (EBUSY == pthread_mutex_trylock(&hash_mutex))
	{
		pthread_mutex_unlock(&hash_mutex);
		pthread_exit(0);
	}
	pthread_mutex_unlock(&hash_mutex);
	pthread_exit(0);
}

void process_request(void *tsocket)
{
	int csocket;
	char recv_string[1024];
	char method[100];
	FILE *file_H = NULL;

	char path[1024] = "";
	DIR *temp_p;

	char old_password[1024] = "";
	char new_password[1024] = "";

	/* About hash table */
	bucket *ret;
	unsigned int table_number;
	char data_name[1024];
	char data_data[HASH_TABLE_ITEM_SIZE];
	char *data_temp = NULL;
	char tmp_data[1024] = "";
	char table_name[100];
	int data_i;
	char insert_data[100][1024];
	/* End */
	
	csocket = *((int *)tsocket);

	pthread_detach(pthread_self());

	signal(SIGSEGV, child_thread_error);

	if (-1 == login_table(csocket))
	{
		send(csocket, "unsuccessfully", 14, MSG_NOSIGNAL);
		goto end;
	}
	else
	{
		send(csocket, "successfully", 12, MSG_NOSIGNAL);
	}

start:
	memset(recv_string, 0, 1024);
	if (0 >= recv(csocket, recv_string, 1024, 0))
	{
		goto end;
	}

	memcpy(method, strtok(recv_string, " "), 100);

	if (0 == strcmp(method, "insert"))		/* insert [table] [name] [data1] [data2] ... */
	{
		memset(insert_data, 0, sizeof(insert_data));
		memset(table_name, 0, sizeof(table_name));
		memset(data_name, 0, sizeof(data_name));

		memcpy(table_name, strtok(NULL, " "), 100);
		table_number = hash_table_index(table_name);
		if (database_data[table_number] == NULL)
		{
			hash_init(&database_data[table_number]);
		}

		memcpy(data_name, strtok(NULL, " "), 1024);

		memcpy(tmp_data, table_name, 1024);
		strncat(table_name, "/", 1024);
		if (NULL == (temp_p = opendir(table_name)))
		{
			if (-1 == mkdir(table_name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
			{
				closedir(temp_p);
				goto start;
			}
		}
		closedir(temp_p);
		snprintf(path, sizeof(path), "%s/%s", table_name, data_name);

		file_H = fopen(path, "a+");
		if (file_H == NULL)
		{
			goto end;
		}

		fprintf(file_H, "%s ", tmp_data);
		fprintf(file_H, "%s", data_name);

		for (data_i = 0; ; data_i++)
		{
			data_temp = NULL;
			
			fprintf(file_H, " ");

			if ((data_temp = strtok(NULL, " ")) == NULL)
			{
				break;
			}

			memcpy(insert_data[data_i], data_temp, 1024);

			fprintf(file_H, "%s", data_temp);
			fflush(file_H);
		}
		pthread_mutex_lock(&hash_mutex);
		if (HASH_MALLOC_FAILURE == hash_insert(database_data[table_number], data_name, insert_data))
		{
			if (-1 == send(csocket, "malloc_error", 12, MSG_NOSIGNAL))
			{
				goto end;
			}
			
			goto start;
		}
		pthread_mutex_unlock(&hash_mutex);

		fprintf(file_H, "\n");

		fclose(file_H);

		if (-1 == send(csocket, "successfully", 12, MSG_NOSIGNAL))
		{
			goto end;
		}
		
		goto start;
	}
	else if (0 == strcmp(method, "create_table"))		/* create_table [table] */
	{
		memcpy(table_name, strtok(NULL, " "), 100);
		pthread_mutex_lock(&hash_mutex);
		hash_init(&database_data[hash_table_index(table_name)]);
		pthread_mutex_unlock(&hash_mutex);

		if (-1 == mkdir(table_name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
		{
			if (-1 == send(csocket, "unsuccessfully", 14, MSG_NOSIGNAL))
			{
				goto end;
			}

			goto start;
		}

		if (-1 == send(csocket, "successfully", 12, MSG_NOSIGNAL))
		{
			goto end;
		}
	
		goto start;
	}
	else if (0 == strcmp(method, "passwd"))			/* passwd [old_password] [new_password] */
	{
		memcpy(old_password, strtok(NULL, " "), 1024);
		memcpy(new_password, strtok(NULL, " "), 1024);
		
		if (0 == strlen(login_info_path))
		{
			passwd_user("/var/user.conf", old_password, new_password);
		}
		else
		{
			passwd_user(login_info_path, old_password, new_password);
		}
	}
	else if (0 == strcmp(method, "search"))			/* search [table] [name] [data] [num] [send_num] */
	{
		table_number = hash_table_index(strtok(NULL, " "));
		if (database_data[table_number] == NULL)
		{
			goto end;
		}

		data_temp = strtok(NULL, " ");
		if (data_temp == NULL)
		{
			goto start;
		}
		memcpy(data_name, data_temp, 1024);
		data_temp = strtok(NULL, " ");
		if (data_temp == NULL)
		{
			goto start;
		}
		memcpy(data_data, data_temp, 1024);

		pthread_mutex_lock(&hash_mutex);
		ret = hash_search(database_data[table_number], data_name, data_data, atoi(strtok(NULL, " ")));
		pthread_mutex_unlock(&hash_mutex);
		if (ret == NULL)
		{
			if (-1 == send(csocket, " ", 1, 0))
			{
				goto end;	
			}

			goto start;
		}
		if (-1 == send(csocket, ret->data[atoi(strtok(NULL, " "))], HASH_TABLE_ITEM_SIZE, MSG_NOSIGNAL))
		{
			goto end;
		}

		goto start;
	}
	else if (0 == strcmp(method, "like_search"))		/* like_search [table] [data name] [data data] [num] [send num] */
	{
		table_number = hash_table_index(strtok(NULL, " "));
		if (database_data[table_number] == NULL)
		{
			send(csocket, "Not found.", 9, MSG_NOSIGNAL);
			goto start;
		}

		memcpy(data_name, strtok(NULL, " "), 1024);
		memcpy(data_data, strtok(NULL, " "), 1024);

		pthread_mutex_lock(&hash_mutex);
		ret = hash_like_search(database_data[table_number], data_name, data_data, atoi(strtok(NULL, " ")));
		pthread_mutex_unlock(&hash_mutex);
		if (ret == NULL)
		{
			if (-1 == send(csocket, " ", 1, 0))
			{
				goto end;
			}

			goto start;
		}
		if (-1 == send(csocket, ret->data[atoi(strtok(NULL, " "))], HASH_TABLE_ITEM_SIZE, MSG_NOSIGNAL))
		{
			goto end;
		}

		goto start;
	}
	else if (0 == strcmp(method, "remove_group"))	/* remove_table [table] [name] [data] [num] */
	{
		memcpy(table_name, strtok(NULL, " "), sizeof(table_name));
		table_number = hash_table_index(table_name);
		if (database_data[table_number] == NULL)
		{
			goto start;
		}

		memcpy(data_name, strtok(NULL, " "), 1024);
		memcpy(data_data, strtok(NULL, " "), 1024);

		snprintf(path, sizeof(path), "%s/%s", table_name, data_name);
		if (-1 == remove(path))
		{
			send(csocket, "unsuccessfully", 14, MSG_NOSIGNAL);
			goto start;
		}

		pthread_mutex_lock(&hash_mutex);
		if (0 != hash_remove_all(database_data[table_number], data_name, data_data, atoi(strtok(NULL, " "))))
		pthread_mutex_unlock(&hash_mutex);
		{
			send(csocket, "unsuccessfully", 14, MSG_NOSIGNAL);
			goto start;
		}

		if (-1 == send(csocket, "successfully", 12, MSG_NOSIGNAL))
		{
			goto end;
		}

		goto start;
	}
	else if (0 == strcmp(method, "remove_table"))
	{
		memcpy(table_name, strtok(NULL, " "), 100);
		pthread_mutex_lock(&hash_mutex);
		hash_free(database_data[hash_index(table_name)]);
		pthread_mutex_unlock(&hash_mutex);
		if (-1 == rmdir(table_name))
		{
			send(csocket, "unsuccessfully", 14, MSG_NOSIGNAL);
			goto start;
		}
		goto start;
	}
	else
	{
		if (-1 == send(csocket, "Unknown command.", 16, MSG_NOSIGNAL))
		{
			goto end;
		}

		goto start;
	}

end:
	connection_counter--;
	close(csocket);
	pthread_exit(0);
}
