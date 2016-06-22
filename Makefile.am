ACLOCAL_AMFLAGS = -I m4 --install
dist_doc_DATA = README.md

noinst_LIBRARIES = libtib.a libliberti.a
bin_PROGRAMS = liberti tibencode tibdecode

COMMON_CFLAGS = -Wall -Wextra -Wunreachable-code -ftrapv -std=c11

libtib_a_SOURCES = src/tibchar.c src/tiberr.c src/tibeval.c src/tibexpr.c \
                   src/tibfunction.c src/tiblst.c src/tibtranscode.c \
                   src/tibtype.c src/tibvar.c
libtib_a_CFLAGS = $(COMMON_CFLAGS) $(gsl_CFLAGS)

libliberti_a_SOURCES = src/lbtscreen.c src/lbtstate.c
libliberti_a_CFLAGS = -Wall -Wextra -Wunreachable-code -ftrapv -std=c89

liberti_SOURCES = src/liberti.c src/log.c src/skin.c src/ttf.c
liberti_CFLAGS = $(COMMON_CFLAGS) $(sdl2_CFLAGS)
liberti_LDADD = libliberti.a libtib.a $(gsl_LIBS) $(libconfig_LIBS) \
                $(pfxtree_LIBS) $(sdl2_LIBS) -lSDL2_image -lSDL2_ttf

tibencode_SOURCES = src/tibencode.c
tibencode_CFLAGS = $(COMMON_CFLAGS)
tibencode_LDADD = libtib.a $(gsl_LIBS) $(pfxtree_LIBS)

tibdecode_SOURCES = src/tibdecode.c
tibdecode_CFLAGS = $(COMMON_CFLAGS)
tibdecode_LDADD = libtib.a $(gsl_LIBS) $(pfxtree_LIBS)

AM_CFLAGS = $(DEPS_CFLAGS)
AM_LIBS = $(DEPS_LIBS)
