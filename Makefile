CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -I. -O2 -g
LDFLAGS = -L. -lmessage

TARGETS = receiver sender time when

SRCS = $(wildcard *.c)

.PHONY: clean

all: $(TARGETS)

$(TARGETS): % : libmessage.so %.o
	$(CC) $*.o $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

message.o: message.c message.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

libmessage.so: message.o
	$(CC) -shared -o $@ $^

clean:
	rm -f *.o

mrProper:
	rm -f *.o $(TARGETS)