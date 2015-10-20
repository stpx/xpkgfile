NAME = xpkgfile
VERSION = 0.1.0

CPPFLAGS  += -DPACKAGE_VERSION=\"$(VERSION)\" -DPACKAGE_NAME=\"$(NAME)\"
CFLAGS    += -std=c99 -pedantic -Wall -Wextra -I$(PREFIX)/include
CFLAGS    += -Wno-unused-parameter
LIBS       = -lxbps

DEBUG	   = 0

ifneq (${DEBUG},0)
CFLAGS    += -g
endif

PREFIX    ?= /usr/local
BINPREFIX  = $(PREFIX)/bin
MANPREFIX  = $(PREFIX)/share/man
DOCPREFIX  = $(PREFIX)/share/doc/xpkgfile
BASHCOMP   = $(PREFIX)/share/bash-completion/completions
ZSHCOMP    = $(PREFIX)/share/zsh/site-functions

SRC = main.c search.c update.c list.c match.c
OBJ = $(SRC:.c=.o)

.c.o: defs.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(NAME): defs.h $(OBJ)
	$(CC) -o $@ ${OBJ} $(LDFLAGS) $(LIBS)

all: ${NAME}

defs.h:
	sed -e "s;@PACKAGE_NAME@;${NAME};" \
		-e "s;@PACKAGE_VERSION@;${VERSION};" defs.h.in > defs.h

install:
	install -Dm755 ${NAME} ${DESTDIR}${PREFIX}/bin/${NAME}
	install -Dm644 doc/${NAME}.1 ${DESTDIR}${MANPREFIX}/man1/${NAME}.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/${NAME}
	rm -f ${DESTDIR}${MANPREFIX}/man1/${NAME}.1

clean:
	rm -f $(OBJ) $(NAME)
	rm -f defs.h

.PHONY: all clean install uninstall
