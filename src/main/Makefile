#Makefile
# Created on: Jan 30, 2018
#     Author: pchero

CC = gcc
CFLAGS = -g -Wall -I../includes

.PHONY: default all clean
#.PRECIOUS: $(TARGET) $(OBJECTS)

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o ../$(BUILDDIR)/$@

default: $(OBJECTS)
all: default

clean:
	-rm -f *.o
	-rm -f $(TARGET)


