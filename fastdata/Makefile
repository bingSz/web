MAKE		= make
PREFIX		= /usr

all: obj

obj/fastdata_server:
	(cd server && make -f Makefile && cp fastdata_server ../)
	
obj/libfdata_client.so:
	(cd client/lib && make -f Makefile && cp libfdata_client.so ../../)

obj/fdata_cli:
	(cd client/cli && cp ../lib/libfdata_client.so ./ && make -f Makefile && cp fdata_cli ../../)

obj: obj/fastdata_server obj/libfdata_client.so obj/fdata_cli
	test -d obj || mkdir obj
	mv fastdata_server libfdata_client.so fdata_cli obj/

install: obj
	install obj/fastdata_server $(PREFIX)/bin/
	install obj/libfdata_client.so $(PREFIX)/lib/
	install obj/fdata_cli $(PREFIX)/bin/
	test -e $(PREFIX)/include/fastdata || mkdir $(PREFIX)/include/fastdata
	cp client/include/fdata_client.h $(PREFIX)/include/fastdata/fdata_client.h

uninstall:
	rm -f $(PREFIX)/bin/fastdata_server $(PREFIX)/lib/libfdata_client.so $(PREFIX)/bin/fdata_cli

clean:
	rm -rf obj
	(cd server && make -f Makefile clean)
	(cd client/lib && make -f Makefile clean)
	(cd client/cli && make -f Makefile clean)
