#include "ss_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SUCCESS		0
#define HASH_MALLOC_FAILURE	1
#define HASH_NOT_FOUND		NULL

unsigned int get_hash_index(char *str)
{
	unsigned int h = 1;

	while (*str != 0)
	{
		h = h + (*str)*31;
		*str = *str + 1;
	}

	return h % HASH_SIZE;
}

int hash_init(hash_table **target)
{
	(*target) = (hash_table *)malloc(sizeof(hash_table));
	if ((*target) == NULL)
	{
		return HASH_MALLOC_FAILURE;
	}

	(*target)->data = (hash_data *)malloc(sizeof(hash_data) * HASH_SIZE);
	if ((*target)->data == NULL)
	{
		return HASH_MALLOC_FAILURE;
	}
	memset((*target)->data, 0, sizeof(hash_data) * HASH_SIZE);
	
	return HASH_SUCCESS;
}

void hash_insert(hash_table *target, char *type, char *descriptor)
{
	hash_data *data = &target->data[get_hash_index(type)];

	while (data->next != NULL)
	{
		data = data->next;
	}

	memcpy(data->type, type, sizeof(data->type));
	memcpy(data->descriptor, descriptor, sizeof(data->descriptor));
}

char *hash_search(hash_table *target, char *type)
{
	hash_data *data = &target->data[get_hash_index(type)];

	while (data != NULL)
	{
		if (0 == strcmp(data->type, type))
		{
			return data->descriptor;
		}

		data = data->next;
	}

	return HASH_NOT_FOUND;
}
