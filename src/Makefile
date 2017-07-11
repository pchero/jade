TARGET = jade_backend
LIBS = -ljansson -levhtp -levent -levent_openssl -lssl -lcrypto -lpthread -luuid -lsqlite3 -lbsd
CC = gcc
CFLAGS = -g -Wall -I/usr/local/include/evhtp

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
