CC=gcc
CFLAGS=-Wall -Wstrict-prototypes -Werror -fPIC -lpthread
LIBS=-lpthread

LD_SONAME=-Wl,-soname,libnss_exec.so.2
LIBRARY=libnss_exec.so.2.0
LINKS=libnss_exec.so.2 libnss_exec.so

MACHINE=$(shell uname -m)
DESTDIR=/
PREFIX=$(DESTDIR)/usr
LIBDIR.x86_64=$(PREFIX)/lib64
LIBDIR.i686=$(PREFIX)/lib
LIBDIR.i386=$(PREFIX)/lib
LIBDIR=$(LIBDIR.$(MACHINE))
OBJECTS=nss_exec.o nss_exec-group.o nss_exec-shadow.o nss_exec-passwd.o

all: $(LIBRARY)

%.o: %.c nss_exec.h
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBRARY): $(OBJECTS)
	$(CC) -shared -o $(LIBRARY) $(LD_SONAME) $^ $(LIBS)

nss_test: *.c nss_exec.h
	$(CC) $(CFLAGS) -DNSS_EXEC_SCRIPT=\"./nss_exec\" nss_exec*.c nss_test.c -o nss_test $(LIBS)

clean:
	rm -rf $(OBJECTS) $(LIBRARY) nss_test

install:
	[ -d $(LIBDIR) ] || install -d $(LIBDIR)
	install $(LIBRARY) $(LIBDIR)
	cd $(LIBDIR); for link in $(LINKS); do ln -sf $(LIBRARY) $$link ; done

.PHONY: clean install
