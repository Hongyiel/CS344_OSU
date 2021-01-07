CC = gcc
CFLAGS = -std=c89 -g -Wall -Wextra

ALL = luuph.buildrooms luuph.adventure

all: $(ALL)

luuph.buildrooms: luuph.buildrooms.c
	$(CC) $(CFLAGS) $? -o $@

luuph.adventure: luuph.adventure.c
	$(CC) $(CFLAGS) -lpthread $? -o $@

.PHONY: clean

clean:
	rm -f $(ALL)
