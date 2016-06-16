# Makefile for pcanflash

DESTDIR ?=
PREFIX ?= /usr/local

MAKEFLAGS = -k

CFLAGS    = -O2 -Wall -Wno-parentheses \
	    -fno-strict-aliasing

CPPFLAGS += -D_FILE_OFFSET_BITS=64

PROGRAMS = pcanflash

all: $(PROGRAMS)

clean:
	rm -f $(PROGRAMS) *.o

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(PROGRAMS) $(DESTDIR)$(PREFIX)/bin

distclean:
	rm -f $(PROGRAMS) *.o *~

pcanflash.o:	crc16.h pcanfunc.h pcanhw.h

pcanflash:	pcanflash.o pcanfunc.o pcanhw.c crc16.o
