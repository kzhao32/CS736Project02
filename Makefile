CC=gcc
CFLAGS=-g -Wall -pedantic
LDFLAGS=
OBJ=filler.o master.o list.o
BIN=filler master

all: $(BIN)

%.o: %.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -c -o $@

filler: filler.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

master: master.o list.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

.PHONY: clean

clean:
	rm -fv $(OBJ) $(BIN)
