CC		= gcc
OTHER_CFLAGS	= 
CFLAGS		= -march=native -Ofast -funroll-loops -flto $(OTHER_CFLAGS) -D_FILE_OFFSET_BITS=64 -D_LARGE_FILES -D_GNU_SOURCE -I. -Wall -Wno-unused-variable -std=gnu99 -lpthread -o
BIN		= stableser.o ss_parser.o ss_worker.o ss_processor.o ss_error.o ss_hash.o

.PHONY: all clean install uninstall

all: stableser

stableser: $(BIN)
	$(CC) -ldl $(CFLAGS) $@ $(BIN)

%.o: %.c
	$(CC) -c $(CFLAGS) $@ $<

install: stableser
	install stableser /usr/local/bin/stableser

uninstall: stableser
	rm -f /usr/local/bin/stableser

clean:
	rm -f $(BIN) stableser
