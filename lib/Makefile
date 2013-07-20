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
	install -d $(PREFIX)/lib $(PREFIX)/include
	install libnetrobots.so $(PREFIX)/lib
	install -m 644 netrobots.h $(PREFIX)/include
	ldconfig

clean:
	rm -f libnetrobots.so $(OBJS)
