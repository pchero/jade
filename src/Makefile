
TARGET = jade_backend

LIBS = -ljansson -levhtp -levent -levent_openssl -lssl -lcrypto -lpthread -luuid -lsqlite3 -lbsd -lonig -lzmq -levent_pthreads -lwebsockets -lm
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
