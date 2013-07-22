# installation prefix
PREFIX ?= /usr/local
# valid server backends: sdl, gtk
BACKEND ?= sdl

.PHONY: all server library examples clean_lib clean_server clean_examples clean \
	install install_server install_examples install_all
library:
server: robotserver
all: server library examples
clean: clean_server clean_lib clean_examples

## library

library:
	$(MAKE) -C lib

install:
	$(MAKE) -C lib PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install

clean_lib:
	$(MAKE) -C lib clean

## server

sdl_CFLAGS = `pkg-config sdl --cflags`
sdl_LDFLAGS = `pkg-config sdl --libs`
gtk_CFLAGS = `pkg-config gtk+-3.0 --cflags`
gtk_LDFLAGS = `pkg-config gtk+-3.0 --libs`

COMMON_CFLAGS = -g -W -Wall -Wno-unused-parameter -O2 -Ilib

CFLAGS = $(COMMON_CFLAGS) `pkg-config cairo --cflags` $($(BACKEND)_CFLAGS)
LDFLAGS = -g `pkg-config cairo --libs` $($(BACKEND)_LDFLAGS) -lm

SERVER_OBJS = server/main.o server/toolkit_$(BACKEND).o \
	server/field.o server/net_utils.o server/net_commands.o \
	server/net_core.o server/robotserver.o

robotserver: $(SERVER_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
server/%.o: server/%.c server/field.h server/net_defines.h server/robotserver.h server/toolkit.h
server/net_utils.o: lib/net_utils.c lib/net_utils.h
	$(CC) $(CFLAGS) -c -o $@ $<

install_server: robotserver
	install -d $(DESTDIR)$(PREFIX)/bin
	install robotserver $(DESTDIR)$(PREFIX)/bin

clean_server:
	rm -f robotserver $(SERVER_OBJS) server/toolkit_*.o

## example robots

examples:
	$(MAKE) -C examples

install_examples:
	$(MAKE) -C examples PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install

clean_examples:
	$(MAKE) -C examples clean

## packaging

VERSION = `git describe master | sed 's/-/./g'`

tar:
	git archive --prefix=netrobots-$(VERSION)/ master | bzip2 > netrobots-$(VERSION).tar.bz2

install_all: install install_server install_examples
