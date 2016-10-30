CC		= gcc
CFLAGS		= -lpthread -g -Wall -Wno-int-to-pointer-cast -std=gnu99 -I../include -O0 -o # -Ofast -funroll-loops -flto -o
BIN		= init.o processor.o hash.o user.o

all: $(BIN) fastdata_server

fastdata_server: $(BIN)
	$(CC) $(CFLAGS) $@ $(BIN)

%.o: %.c
	$(CC) -c $(CFLAGS) $@ $<

clean:
	rm -f $(BIN) fastdata_server
