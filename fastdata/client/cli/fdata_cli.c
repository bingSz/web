#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fdata_client.h"

void Getbuf(char *str)
{
start:
	if (NULL == fgets(str, sizeof(str), stdin))
	{
		goto start;
	}
	str[strlen(str) - 1] = '\0';
	getchar();
}

int main()
{
	FDATA *fdata;
	char command[1024] = "";
	char port[6];
	char ip_addr[15] = "";
	char username[1024] = "";
	char password[1024] = "";

	printf("FastDATA cli.\n");

	printf("IP address:");
redo1:
	if (NULL == fgets(ip_addr, sizeof(ip_addr), stdin))
	{
		goto redo1;
	}
	ip_addr[strlen(ip_addr)-1] = '\0';

	printf("Port:");
redo2:
	if (NULL == fgets(port, sizeof(port), stdin))
	{
		goto redo2;
	}
	port[strlen(port)-1] = '\0';

	printf("Username:");
redo3:
	if (NULL == fgets(username, sizeof(username), stdin))
	{
		goto redo3;
	}
	username[strlen(username)-1] = '\0';

	printf("Password:");
redo4:
	if (NULL == fgets(password, sizeof(password), stdin))
	{
		goto redo4;
	}
	password[strlen(password)-1] = '\0';

	if (EXIT_FAILURE == fdata_init(&fdata, ip_addr, atoi(port), username, password))
	{
		printf("Initialize unsuccessfully.\n");
		exit(EXIT_FAILURE);
	}

	for (;;)
	{
		printf(">");

		if (NULL == fgets(command, sizeof(command), stdin))
		{
			continue;
		}
		command[strlen(command)-1] = '\0';

		if (0 == strcmp(command, "quit"))
		{
			break;
		}

		printf("%s\n", fdata_query(fdata, command, 5));
	}

	fdata_close(&fdata);

	return 0;
}
