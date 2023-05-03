CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -g
LDFLAGS = -L. -lmessage

FILES = sender receiver pid-taskd-user time when

all: libmessage.so $(FILES)

libmessage.so: message.o
	$(CC) -shared -o libmessage.so message.o

message.o: message.c message.h
	$(CC) $(CFLAGS) -c -fPIC message.c -o message.o

sender: sender.c libmessage.so
	$(CC) $(CFLAGS) sender.c -o sender $(LDFLAGS)

receiver: receiver.c libmessage.so
	$(CC) $(CFLAGS) receiver.c -o receiver $(LDFLAGS)

pid-taskd-user: pid-taskd-user.c
	$(CC) $(CFLAGS) pid-taskd-user.c -o pid-taskd-user

time: time.c
	$(CC) $(CFLAGS) time.c -o time

when: when.c
	$(CC) $(CFLAGS) when.c -o when

.PHONY: install

install:
	mkdir -p ~/lib
	cp libmessage.so ~/lib
	echo "~/lib" >> ~/.bashrc
	. ~/.bashrc

.PHONY: clean
clean:
	rm -f *.o *.so $(FILES)
