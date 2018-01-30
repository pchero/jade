
TARGET = jade_backend

LIBS = -ljansson -levhtp -levent -levent_openssl -lssl -lcrypto -lpthread -luuid -lsqlite3 -lbsd -lonig -lzmq -levent_pthreads -lwebsockets
BUILDDIR = build
SUBDIRS = modules main
CC = gcc

export BUILDDIR

.PHONY: all $(SUBDIRS)
.PHONY: clean $(SUBDIRS)

all:
	-mkdir -p $(BUILDDIR);
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir all; \
	done
	
	$(CC) -o $(BUILDDIR)/$(TARGET) $(BUILDDIR)/*.o $(LIBS)	


clean:
	-rm -rf $(BUILDDIR)
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

$(SUBDIRS):
	$(MAKE) -C $@




#TARGET = jade_backend
#LIBS = -ljansson -levhtp -levent -levent_openssl -lssl -lcrypto -lpthread -luuid -lsqlite3 -lbsd -lonig -lzmq -levent_pthreads -lwebsockets
#CC = gcc
#CFLAGS = -g -Wall -I/usr/local/include/evhtp
#
#.PHONY: default all clean
#
#default: $(TARGET)
#all: default
#
#OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
#HEADERS = $(wildcard *.h)
#
#%.o: %.c $(HEADERS)
#	$(CC) $(CFLAGS) -c $< -o $@
#
#.PRECIOUS: $(TARGET) $(OBJECTS)
#
#$(TARGET): $(OBJECTS)
#	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@
#
#clean:
#	-rm -f *.o
#	-rm -f $(TARGET)
