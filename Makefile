CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra
prefix ?= /usr/local
bindir ?= $(prefix)/bin

bootguard-status: bootguard-status.c

install: bootguard-status
	install -D -m 0755 bootguard-status $(DESTDIR)$(bindir)/bootguard-status

clean:
	rm -f bootguard-status

.PHONY: install clean
