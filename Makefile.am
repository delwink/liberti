ACLOCAL_AMFLAGS = -I m4 --install
dist_doc_DATA = README.md

noinst_LIBRARIES = libtib.a libliberti.a
bin_PROGRAMS = liberti tibencode tibdecode

libtib_a_SOURCES = src/tibeval.c src/tiberr.c src/tibexpr.c src/tibchar.c \
                   src/tiblst.c src/tibtranscode.c src/tibtype.c
libtib_a_CFLAGS = -Wall -Wextra -Wunreachable-code -ftrapv -std=c89 \
                  $(gsl_CFLAGS)

libliberti_a_SOURCES = src/command.c
libliberti_a_CFLAGS = -Wall -Wextra -Wunreachable-code -ftrapv -std=c89

liberti_SOURCES = src/liberti.c
liberti_CFLAGS = -Wall -Wextra -Wunreachable-code -ftrapv -std=c89 \
                 $(sdl2_CFLAGS)
liberti_LDADD = $(sdl2_LIBS) $(gsl_LIBS) libtib.a libliberti.a

tibencode_SOURCES = src/tibencode.c
tibencode_CFLAGS = -Wall -Wextra -Wunreachable-code -ftrapv -std=c89
tibencode_LDADD = $(gsl_LIBS) libtib.a

tibdecode_SOURCES = src/tibdecode.c
tibdecode_CFLAGS = -Wall -Wextra -Wunreachable-code -ftrapv -std=c89
tibdecode_LDADD = $(gsl_LIBS) libtib.a

AM_CFLAGS = $(DEPS_CFLAGS)
AM_LIBS = $(DEPS_LIBS)
