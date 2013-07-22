# installation prefix
PREFIX ?= /usr/local

CFLAGS = -g -W -Wall -Wno-unused-parameter -O2 -fPIC

.PHONY: all clean install
all: libnetrobots.so

OBJS = netrobots.o net_utils.o

netrobots.o: netrobots.c netrobots.h net_utils.h net_command_list.h
net_utils.o: net_utils.c net_utils.h

libnetrobots.so: $(OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $+

install: libnetrobots.so
	install -d $(DESTDIR)$(PREFIX)/lib $(DESTDIR)$(PREFIX)/include
	install libnetrobots.so $(DESTDIR)$(PREFIX)/lib
	install -m 644 netrobots.h $(DESTDIR)$(PREFIX)/include
	if [ -z "$(DESTDIR)" ]; then ldconfig; fi

clean:
	rm -f libnetrobots.so $(OBJS)
