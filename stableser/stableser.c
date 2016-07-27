#include "ss_config.h"
#include "ss_func.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#define DEFAULT_CONFIG_PATH "/etc/ss.conf"

static void printhelp();
static void printver();

int main(int argc, char * argv[])
{
	ss_int_t  isParse = 0;
	ss_char_t result_opt;
	ss_char_t short_opts[] = "c:hv";
	struct option long_opts[] = {{"version", no_argument, NULL, (int)'v'}, 
				     {"help", no_argument, NULL, (int)'h'}, 
				     {"config", required_argument, NULL, (int)'c'}, 
				     {0, 0, 0, 0}};

	while ((result_opt = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1)
	{
		switch (result_opt)
		{
			case 'h':
				printhelp();
				exit(EXIT_SUCCESS);
			case 'v':
				printver();
				exit(EXIT_SUCCESS);
			case 'c':
				isParse = 1;
				ss_parse_config(optarg);
				break;
			case '?':
				printf("Unknonw option: %c\n", optopt);
				printhelp();
				exit(EXIT_FAILURE);
		}
	}
	
	if (isParse != 1)
	{
		ss_parse_config(DEFAULT_CONFIG_PATH);
	}

	ss_worker();

	return 0;
}

static void printhelp()
{
	printver();
	printf("Usage: stableser [options] \n");
	printf("\n");
	printf("Options:\n");
	printf("  -h --help\t\t: print this help.\n");
	printf("  -v --version\t\t: print version.\n");
	printf("  -c --config\t\t: specify config file path.\n");
	printf("\n");
}

static void printver()
{
	printf("%s\n", SS_VER);
}
