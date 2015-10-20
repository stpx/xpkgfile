PROG = xpkgfile
VERSION = 0.1.0

CC 		  ?= gcc
LIBS	   = -lxbps
CFLAGS    += -std=c99 -pedantic -Wall -Wextra -I$(PREFIX)/include
CFLAGS    += -DPACKAGE_VERSION=\"$(VERSION)\" -DPACKAGE_NAME=\"$(PROG)\"
CFLAGS    += -D_GNU_SOURCE -Wno-unused-parameter

CFLAGS    += -g

PREFIX    ?= /usr/local
BINPREFIX  = $(PREFIX)/bin
MANPREFIX  = $(PREFIX)/share/man
DOCPREFIX  = $(PREFIX)/share/doc/xpkgfile
# BASHCOMP   = $(PREFIX)/share/bash-completion/completions
ZSHCOMP    = $(PREFIX)/share/zsh/site-functions

SRC = main.c search.c update.c list.c match.c
OBJ = $(SRC:.c=.o)

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(PROG): $(OBJ)
	$(CC) -o $@ ${OBJ} $(LDFLAGS) $(LIBS)

all: ${PROG}

clean:
	rm -f $(OBJ) $(PROG)
