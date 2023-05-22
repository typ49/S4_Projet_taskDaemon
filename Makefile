CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -I. -O2 -g -D_DEFAULT_SOURCE
LDFLAGS = -L. -lmessage -ldata

TARGETS = receiver sender time when taskcli taskd launch_daemon

SRCS = $(wildcard *.c)

.PHONY: clean

all: $(TARGETS) libmessage.so libdata.so

$(TARGETS): % : %.o libdata.so libmessage.so
	$(CC) $*.o -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

message.o: message.c message.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

libmessage.so: message.o
	$(CC) -shared -o $@ $^

data.o: data.c data.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

libdata.so: data.o
	$(CC) -shared -o $@ $^


clean:
	rm -f *.o

mrProper:
	rm -f *.o *.so $(TARGETS)