CC		= gcc
CFLAGS		= -lpthread -Wall -std=gnu99 -Ofast -funroll-loops -flto -I../include -march=native -fPIC -shared -o

all: libfdata_client.so

libfdata_client.so: fdata_client.o
	$(CC) $(CFLAGS) $@ $<

fdata_client.o: fdata_client.c
	$(CC) -c $(CFLAGS) $@ $<

clean:
	rm -f fdata_client.o libfdata_client.so
