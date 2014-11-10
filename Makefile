CC=gcc
CFLAGS=-g -Wall -pedantic
LDFLAGS=
OBJ=filler.o master.o
BIN=filler master

all: $(BIN)

%.o: %.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -c -o $@

%: %.o
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

.PHONY: clean

clean:
	rm -fv $(OBJ) $(BIN)
