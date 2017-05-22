CC=c99
CFLAGS=-Wall -Wextra -Wunreachable-code -ftrapv -fPIC -g -D_POSIX_C_SOURCE=2 -D_REENTRANT -I/usr/include/SDL2
CONFIG_LIBS=-lconfig
GSL_LIBS=-lgsl -lgslcblas -lm
PFXTREE_LIBS=-lpfxtree
SDL2_LIBS=-lSDL2 -lSDL2_image
PREFIX=/usr/local
BINDIR=$(DESTDIR)$(PREFIX)/bin

all: liberti tibencode tibdecode

liberti_deps=src/colors.o src/font.o src/keys.o src/liberti.o src/log.o src/mode_default.o src/screen.o src/skin.o src/state.o libtib.a
liberti: $(liberti_deps)
	./mvobjs.sh
	$(CC) -o $@ $(liberti_deps) $(CONFIG_LIBS) $(GSL_LIBS) $(PFXTREE_LIBS) $(SDL2_LIBS)

tibencode_deps=src/tibencode.o libtib.a
tibencode: $(tibencode_deps)
	./mvobjs.sh
	$(CC) -o $@ $(tibencode_deps) $(GSL_LIBS) $(PFXTREE_LIBS)

tibdecode_deps=src/tibdecode.o libtib.a
tibdecode: $(tibdecode_deps)
	./mvobjs.sh
	$(CC) -o $@ $(tibdecode_deps) $(GSL_LIBS) $(PFXTREE_LIBS)

libtib_deps=src/tibchar.o src/tiberr.o src/tibeval.o src/tibexpr.o src/tibfunction.o src/tiblst.o src/tibtranscode.o src/tibtype.o src/tibvar.o src/util.o
libtib.a: $(libtib_deps)
	./mvobjs.sh
	$(AR) rcs $@ $(libtib_deps)

install: all
	install -m755 liberti $(BINDIR)/liberti
	install -m755 tibencode $(BINDIR)/tibencode
	install -m755 tibdecode $(BINDIR)/tibdecode

clean:
	rm -f src/*.o *.a liberti tibencode tibdecode
