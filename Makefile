CXX=g++
CC=gcc
CFLAGS=-g -Wall -pedantic
CXXFLAGS=$(CFLAGS) -std=c++11
LDFLAGS=
SO=followerCount_filter.so
BIN=filler master bonsai_fe bonsai_be followerCount_fe followerCount_be

OBJ=	filler.o master.o bonsai_fe.o bonsai_be.o list.o common.o \
	followerCount_fe.o followerCount_be.o

all: LDFLAGS+=-pthread -lmrnet -lxplat -ldl
all: $(BIN) $(SO)
	$(MAKE) -C collector_tweets

ringonly: filler master
	$(MAKE) -C collector_tweets

%.o: %.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -c -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -c -o $@

bonsai_fe: bonsai_fe.o
	$(CXX) $(CFLAGS) $^ $(LDFLAGS) -o $@

bonsai_be: bonsai_be.o
	$(CXX) $(CFLAGS) $^ $(LDFLAGS) -o $@

followerCount_fe: followerCount_fe.o
	$(CXX) $(CFLAGS) $^ $(LDFLAGS) -o $@

followerCount_filter.so: followerCount_filter.cpp
	$(CXX) $(CFLAGS) -shared -fPIC $^ $(LDFLAGS) -o $@

followerCount_be: followerCount_be.o
	$(CXX) $(CFLAGS) $^ $(LDFLAGS) -o $@

filler: filler.o common.o list.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

master: master.o common.o list.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

.PHONY: clean

clean:
	rm -fv $(OBJ) $(BIN) $(SO)
	$(MAKE) -C collector_tweets clean
