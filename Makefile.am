ACLOCAL_AMFLAGS = -I m4 --install
dist_doc_DATA = README.md

noinst_LIBRARIES = libtib.a libliberti.a
bin_PROGRAMS = liberti tibencode tibdecode

libtib_a_SOURCES = src/tibchar.c src/tiberr.c src/tibeval.c src/tibexpr.c \
                   src/tibfunction.c src/tiblst.c src/tibtranscode.c \
                   src/tibtype.c src/tibvar.c
libtib_a_CFLAGS = -Wall -Wextra -Wunreachable-code -ftrapv -std=c89 \
                  $(gsl_CFLAGS)

libliberti_a_SOURCES = src/lbtscreen.c src/lbtstate.c
libliberti_a_CFLAGS = -Wall -Wextra -Wunreachable-code -ftrapv -std=c89

liberti_SOURCES = src/liberti.c
liberti_CFLAGS = -Wall -Wextra -Wunreachable-code -ftrapv -std=c89 \
                 $(sdl2_CFLAGS)
liberti_LDADD = libtib.a libliberti.a $(gsl_LIBS) $(libconfig_LIBS) \
                $(sdl2_LIBS)

tibencode_SOURCES = src/tibencode.c
tibencode_CFLAGS = -Wall -Wextra -Wunreachable-code -ftrapv -std=c89
tibencode_LDADD = libtib.a $(gsl_LIBS) $(pfxtree_LIBS)

tibdecode_SOURCES = src/tibdecode.c
tibdecode_CFLAGS = -Wall -Wextra -Wunreachable-code -ftrapv -std=c89
tibdecode_LDADD = libtib.a $(gsl_LIBS)

AM_CFLAGS = $(DEPS_CFLAGS)
AM_LIBS = $(DEPS_LIBS)
