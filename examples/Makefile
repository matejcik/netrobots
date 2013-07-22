ROBOTS = counter rabbit rook sniper spot

PREFIX ?= /usr/local
CFLAGS = -g -Wuninitialized -O2 -I../lib
LDFLAGS = -g -lm -L../lib -lnetrobots

.PHONY: all clean install
all: $(ROBOTS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(ROBOTS)

install: $(ROBOTS)
	install -d $(DESTDIR)$(PREFIX)/bin
	install $(ROBOTS) $(DESTDIR)$(PREFIX)/bin
