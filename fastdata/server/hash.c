#include "io.h"
#include "hash_table.h"

unsigned int hash_index(char *str)
{
	unsigned int h = 1;

	while (*str != 0)
	{
		h = *str + h * 31;
		(void)*str++;
	}

	return h % HASH_SIZE;
}

int hash_init(hash_table **hash)
{
	(*hash) = (hash_table *)calloc(sizeof(hash_table), 1);
	if ((*hash) == NULL)
	{
		return HASH_MALLOC_FAILURE;
	}

	(*hash)->data = (bucket *)calloc(sizeof(bucket), HASH_SIZE);
	if ((*hash)->data == NULL)
	{
		return HASH_MALLOC_FAILURE;
	}

	(*hash)->data->name = NULL;

	return HASH_SUCCESS;
}

int hash_insert_internal(hash_table *hash, char *name, char data[][1024])
{
	bucket *addr = &hash->data[hash_index(name)];
	int insert_data_i;
	int len;

	char temp_name[1024] = "";
	memcpy(temp_name, name, 1024);

	while (addr->next != NULL)
	{
		addr = addr->next;
	}

	if (addr->name == NULL)
	{
		addr->name = (char *)calloc(sizeof(char), strlen(temp_name));
		if (addr->name == NULL)
		{
			return HASH_MALLOC_FAILURE;
		}
	}
	memcpy(addr->name, temp_name, sizeof(char) * strlen(temp_name));

	if (addr->data == NULL)
	{
		addr->data = (char **)calloc(sizeof(char *), HASH_DEFAULT_ITEM_SIZE);
		if (addr->name ==  NULL)
		{
			return HASH_MALLOC_FAILURE;
		}
	}

	for (insert_data_i = 0; ; insert_data_i ++)
	{
		if (data[insert_data_i] == NULL)
		{
			break;
		}
		if (0 == strlen(data[insert_data_i]))
		{
			break;
		}

		len = strlen(data[insert_data_i]);

		if (addr->data[insert_data_i] == NULL)
		{
			addr->data[insert_data_i] = (char *)calloc(sizeof(char *), len);
			if (addr->data[insert_data_i] == NULL)
			{
				return HASH_MALLOC_FAILURE;
			}
			memcpy(addr->data[insert_data_i], data[insert_data_i], len);
		}
		else
		{
			free(addr->data[insert_data_i]);
			addr->data[insert_data_i] = (char *)calloc(sizeof(char *), len);
			if (addr->data[insert_data_i] == NULL)
			{
				return HASH_MALLOC_FAILURE;
			}
			memcpy(addr->data[insert_data_i], data[insert_data_i], len);
		}

	}

	addr->next = (bucket *)calloc(sizeof(bucket), 1);
	if (addr->next == NULL)
	{
		return HASH_MALLOC_FAILURE;
	}
	addr->next->name = NULL;
	addr->next->next = NULL;

	return HASH_SUCCESS;
}

bucket *hash_search(hash_table *hash, char *name, char *data, int number)
{
	if (hash->data == NULL)
	{
		return HASH_SEARCH_NOT_FOUND;
	}

	bucket *addr = &hash->data[hash_index(name)];

	if (addr == NULL)
	{
		return HASH_SEARCH_NOT_FOUND;
	}

	while (addr != NULL)
	{
		if (addr->data == NULL)
		{
			goto next;
		}
		if (addr->data[number] == NULL)
		{
			goto next;
		}

		if (0 == strcmp(data, addr->data[number]))
		{
			if (0 == strcmp(name, addr->name))
			{
				return addr;
			}
		}
	
next:
		addr = addr->next;
	}

	return HASH_SEARCH_NOT_FOUND;		/* If not found, return NULL. */
}

int hash_remove(hash_table *hash, char *name, char *data, int num)
{
	bucket *addr = &hash->data[hash_index(name)];

	while (addr != NULL)
	{
		if (0 == strcmp(data, addr->data[num]))
		{
			free(addr->data[num]);
			addr->data[num] = NULL;
			return HASH_SUCCESS;
		}

		addr = addr->next;
	}

	return HASH_NOT_FOUND;
}

int hash_remove_all(hash_table *hash, char *name, char *data, int num)
{
	bucket *addr = &hash->data[hash_index(name)];
	int i;

	while (addr != NULL)
	{
		if (addr->data == NULL)
		{
			goto next;
		}

		if (0 == strcmp(data, addr->data[num]))
		{
			for (i = 0; i < HASH_TABLE_ITEM_SIZE; i++)
			{
				if (addr->data[i] == NULL)
				{
					break;
				}

				free(addr->data[i]);
				addr->data[i] = NULL;
			}
			free(addr->data);
			addr->data = NULL;
			return HASH_SUCCESS;
		}

next:
		addr = addr->next;
	}

	return HASH_NOT_FOUND;
}

void hash_free(hash_table *hash)
{
	bucket *addr;
	bucket *temp;
	unsigned int i;
	unsigned int data_i;

	for (i = 0; i < HASH_SIZE; i++)
	{
		addr = &hash->data[i];

		if (addr == NULL)
		{
			continue;
		}

		for (data_i = 0; data_i < HASH_DEFAULT_ITEM_SIZE; data_i++)
		{
			if (addr->data == NULL)
			{
				break;
			}
			if (addr->data[data_i] == NULL)
			{
				continue;
			}
			free(addr->data[data_i]);
		}
		free(addr->data);
	
		addr = addr->next;
		while (addr != NULL)
		{
			temp = addr;
			addr = addr->next;
			free(temp);
		}
	}

	free(hash->data);
	free(hash);
	hash = NULL;
}

bucket *hash_like_search(hash_table *hash, char *name, char *data, int num)
{
	bucket *addr = &hash->data[hash_index(name)];
	char temp_str[HASH_TABLE_ITEM_SIZE];

	while (addr != NULL)
	{
		if (addr->data == NULL)
		{
			goto next;
		}
		memcpy(temp_str, addr->data[num], HASH_TABLE_ITEM_SIZE);

		if (NULL != strstr(temp_str, data))
		{
			if (0 == strcmp(name, addr->name))
			{
				return addr;
			}
		}

next:
		addr = addr->next;
	}

	return HASH_SEARCH_NOT_FOUND;
}

int hash_insert(hash_table *test, char *name, char data[][1024])
{
	return hash_insert_internal(test, name, data);
}

