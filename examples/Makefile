ROBOTS = counter rabbit rook sniper spot

CFLAGS = -g -Wuninitialized -O2 -I../lib
LDFLAGS = -g -lm -L../lib

.PHONY: all clean
all: $(ROBOTS)

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< -lnetrobots

clean:
	rm -f $(ROBOTS)
