CXX=g++
CC=gcc
CFLAGS=-g -Wall -pedantic
CXXFLAGS=$(CFLAGS)
LDFLAGS=-lrt
SO=followerCount_filter.so search_filter.so

BIN=	filler master bonsai_fe bonsai_be collector_rand followerCount_fe \
	followerCount_be search_fe search_be

OBJ=	filler.o master.o bonsai_fe.o bonsai_be.o list.o common.o \
	collector_rand.o followerCount_fe.o followerCount_be.o \
	search_fe.o search_be.o

all: LDFLAGS+=-pthread -lmrnet -lxplat -ldl
all: $(BIN) $(SO)
	$(MAKE) -C collector_tweets

ringonly: filler master collector_rand
	$(MAKE) -C collector_tweets

%.o: %.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -c -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -c -o $@

collector_rand: collector_rand.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

bonsai_fe: bonsai_fe.o
	$(CXX) $(CFLAGS) $^ $(LDFLAGS) -o $@

bonsai_be: bonsai_be.o
	$(CXX) $(CFLAGS) $^ $(LDFLAGS) -o $@

followerCount_fe: followerCount_fe.o
	$(CXX) $(CFLAGS) $^ $(LDFLAGS) -o $@

followerCount_be: followerCount_be.o
	$(CXX) $(CFLAGS) $^ $(LDFLAGS) -o $@

followerCount_filter.so: followerCount_filter.cpp
	$(CXX) $(CFLAGS) -shared -fPIC $^ $(LDFLAGS) -o $@

search_fe: search_fe.o
	$(CXX) $(CFLAGS) $^ $(LDFLAGS) -o $@

search_be: search_be.o
	$(CXX) $(CFLAGS) $^ $(LDFLAGS) -o $@

search_filter.so: search_filter.cpp
	$(CXX) $(CFLAGS) -shared -fPIC $^ $(LDFLAGS) -o $@

filler: filler.o common.o list.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

master: master.o common.o list.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

.PHONY: clean

clean:
	rm -fv $(OBJ) $(BIN) $(SO)
	$(MAKE) -C collector_tweets clean
#for CSL machine, use "make CFLAGS="-I$HOME/root/include -L$HOME/root/lib""
