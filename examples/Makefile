####### libk8000 examples makefile
CC  = gcc
CPP = g++

CFLAGS = -Wall -O3
LDFLAGS = -lk8000

####### Build rules

all: k8000-running-light k8000-clearall k8000-IO-AD k8000-chains

k8000-running-light : k8000-running-light.cpp
	$(CPP) $(CFLAGS) k8000-running-light.cpp -o k8000-running-light $(LDFLAGS)

k8000-clearall : k8000-clearall.cpp
	$(CPP) $(CFLAGS) k8000-clearall.cpp -o k8000-clearall $(LDFLAGS)

k8000-IO-AD : k8000-IO-AD.cpp
	$(CPP) $(CFLAGS) k8000-IO-AD.cpp -o k8000-IO-AD $(LDFLAGS)

k8000-chains : k8000-chains.c
	$(CC) $(CFLAGS) k8000-chains.c -o k8000-chains $(LDFLAGS)
clean:
	rm -f k8000-running-light k8000-clearall k8000-IO-AD k8000-chains
