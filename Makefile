CC=gcc
CXX=g++
CFLAGS=-g -I/u/k/r/kritphon/root/include -L/u/k/r/kritphon/root/lib -Wall -pedantic
LDFLAGS=-lmrnet -lxplat -lpthread -ldl
OBJ=filler.o master.o bonsai_fe.o bonsai_be.o list.o common.o
BIN=filler master bonsai_fe bonsai_be

all: $(BIN)

%.o: %.cpp
	$(CXX) $(CFLAGS) $< $(LDFLAGS) -c -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -c -o $@

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
