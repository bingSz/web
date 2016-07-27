CC		= gcc
CFLAGS		= -Wall -std=gnu99 -Ofast -funroll-loops -flto -I ../include -L../lib -lfdata_client -o

all: fdata_cli

fdata_cli: fdata_cli.o
	$(CC) $(CFLAGS) $@ $< 

fdata_cli.o: fdata_cli.c
	$(CC) -c $(CFLAGS) $@ $<

clean:
	rm -f fdata_cli fdata_cli.o libfdata_client.so
