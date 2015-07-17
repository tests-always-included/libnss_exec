CC=gcc
CFLAGS=-Wall -Wstrict-prototypes -Werror
CFLAGS_LIB=-fPIC -shared -Wl,-soname,libnss_exec.so.2
CFLAGS_TEST=-DNSS_EXEC_SCRIPT=\"./nss_exec\"
LIBS=-lpthread

LIBRARY=libnss_exec.so.2.0
LINKS=libnss_exec.so.2 libnss_exec.so

MACHINE=$(shell uname -m)
DESTDIR=/
PREFIX=$(DESTDIR)/usr
LIBDIR.x86_64=$(PREFIX)/lib64
LIBDIR.i686=$(PREFIX)/lib
LIBDIR.i386=$(PREFIX)/lib
LIBDIR=$(LIBDIR.$(MACHINE))
SOURCE=nss_exec.c nss_exec-*.c

all: $(LIBRARY) nss_exec_test

$(LIBRARY): $(SOURCE) nss_exec.h
	$(CC) $(CFLAGS) $(CFLAGS_LIB) $(LIBS) $(SOURCE) -o $(LIBRARY)

nss_exec_test: $(SOURCE) nss_exec_test.c nss_exec.h
	$(CC) $(CFLAGS) $(CFLAGS_TEST) $(LIBS) $(SOURCE) nss_exec_test.c -o nss_exec_test $(LIBS)

clean:
	rm -rf $(OBJECTS) $(LIBRARY) nss_exec test

install:
	[ -d $(LIBDIR) ] || install -d $(LIBDIR)
	install $(LIBRARY) $(LIBDIR)
	cd $(LIBDIR); for link in $(LINKS); do ln -sf $(LIBRARY) $$link ; done

.PHONY: clean install
