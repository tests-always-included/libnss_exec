CC=gcc
CFLAGS=-Wall -Wstrict-prototypes -Werror -static -fPIC

LD_SONAME=-Wl,-soname,libnss_exec.so.2
LIBRARY=libnss_exec.so.2.0
LINKS=libnss_exec.so.2 libnss_exec.so

DESTDIR=/
PREFIX=$(DESTDIR)/usr
LIBDIR=$(PREFIX)/lib
OBJECTS=nss_exec.o nss_exec-group.o nss_exec-shadow.o nss_exec-passwd.o

all: $(LIBRARY)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBRARY): $(OBJECTS)
	$(CC) -shared $(LD_SONAME) $< -o $(LIBRARY)

clean:
	rm -rf $(OBJECTS) $(LIBRARY)

install:
	[ -d $(LIBDIR) ] || install -d $(LIBDIR)
	install $(LIBRARY) $(LIBDIR)
	cd $(LIBDIR); for link in $(LINKS); do ln -sf $(LIBRARY) $$link ; done

.PHONY: clean install
