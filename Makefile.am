ACLOCAL_AMFLAGS = -I m4 --install
dist_doc_DATA = README.md

noinst_LIBRARIES = libtibasic.a libliberti.a
bin_PROGRAMS = liberti

libtibasic_a_SOURCES = src/tibasic.c
libtibasic_a_CFLAGS = -Wall -Wextra -Wunreachable-code -ftrapv -std=c89


libliberti_a_SOURCES = src/evalformula.c
libliberti_a_CFLAGS = -Wall -Wextra -Wunreachable-code -ftrapv -std=c89

liberti_SOURCES = src/liberti.c
liberti_CFLAGS = -Wall -Wextra -Wunreachable-code -ftrapv -std=c89 \
                 $(sdl2_CFLAGS)
liberti_LDADD = $(sdl2_LIBS)

AM_CFLAGS = $(DEPS_CFLAGS)
AM_LIBS = $(DEPS_LIBS)
