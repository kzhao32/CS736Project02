CC=gcc
CFLAGS=-Wall -pedantic
LDFLAGS=-lsqlite3
OBJ=filler.o
BIN=filler

all: $(BIN)

%.o: %.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -c -o $@

%: %.o
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

.PHONY: clean

clean:
	rm -fv $(OBJ) $(BIN)
