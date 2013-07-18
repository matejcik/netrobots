# installation prefix
PREFIX ?= /usr/local
# valid server backends: sdl, gtk
BACKEND ?= sdl

.PHONY: all server library examples clean_examples clean install
library: libnetrobots.so
server: robotserver
all: server library examples

CFLAGS = -g -Wuninitialized -O2 -Ilib

## library

libnetrobots.so: lib/netrobots.c lib/net_utils.c
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $+

install: libnetrobots.so
	install -d $(PREFIX)/lib $(PREFIX)/include
	install libnetrobots.so $(PREFIX)/lib
	install -m 644 lib/netrobots.h $(PREFIX)/include
	ldconfig

## server

sdl_CFLAGS = `pkg-config sdl --cflags`
sdl_LDFLAGS = `pkg-config sdl --libs`
gtk_CFLAGS = `pkg-config gtk+-3.0 --cflags`
gtk_LDFLAGS = `pkg-config gtk+-3.0 --libs`

SERVER_CFLAGS = $(CFLAGS) `pkg-config cairo --cflags` $($(BACKEND)_CFLAGS)
SERVER_LDFLAGS = -g `pkg-config cairo --libs` $($(BACKEND)_LDFLAGS) -lm

SERVER_OBJS = server/main.o server/toolkit_$(BACKEND).o \
	server/field.o lib/net_utils.o server/net_commands.o \
	server/net_core.o server/robotserver.o

robotserver: $(SERVER_OBJS)
	$(CC) $(SERVER_CFLAGS) $(SERVER_LDFLAGS) -o $@ $^
server/%.o: server/%.c server/field.h server/net_defines.h server/robotserver.h server/toolkit.h
	$(CC) $(SERVER_CFLAGS) -c -o $@ $<

## example robots

examples:
	$(MAKE) -C examples

## clean

clean_examples:
	$(MAKE) -C examples clean

clean: clean_examples
	rm -f libnetrobots.so robotserver $(SERVER_OBJS) server/toolkit_*.o
