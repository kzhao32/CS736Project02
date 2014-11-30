CXX=g++
CC=gcc
CFLAGS=-g -Wall -pedantic
LDFLAGS=-ldl -lpthread -lmrnet -lxplat
OBJ=filler.o master.o bonsai_fe.o bonsai_be.o list.o common.o
BIN=filler master bonsai_fe bonsai_be

all: $(BIN)
	$(MAKE) -C collector

ringonly: filler master
	$(MAKE) -C collector

%.o: %.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -c -o $@

%.o: %.cpp
	$(CXX) $(CFLAGS) $< $(LDFLAGS) -c -o $@

bonsai_fe: bonsai_fe.o
	$(CXX) $(CFLAGS) $^ $(LDFLAGS) -o $@

bonsai_be: bonsai_be.o
	$(CXX) $(CFLAGS) $^ $(LDFLAGS) -o $@

filler: filler.o common.o list.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

master: master.o common.o list.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

.PHONY: clean

clean:
	rm -fv $(OBJ) $(BIN)
	$(MAKE) -C collector clean
