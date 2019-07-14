# extra includes paths (-I options)
INCLUDE = -I../libsndio -I../bsd-compat

# extra libraries paths (-L options)
LIB = -L../libsndio

# extra defines (-D options)
DEFS = -DDEBUG -DSNDIO_USER=\"sndiod\" -DADATA_BITS=16 -D_GNU_SOURCE -DHAVE_SOCK_CLOEXEC -DHAVE_CLOCK_GETTIME -DUSE_ALSA

# extra libraries (-l options)
LDADD = -lsndio -lrt -lasound

# variables defined on configure script command line (if any)


#
# binaries, documentation, man pages and examples will be installed in
# ${BIN_DIR}, ${MAN8_DIR}
#
BIN_DIR = /home/ian/source/CCOSS/external/linux/sndio-1.5.0/install/bin
MAN8_DIR = /home/ian/source/CCOSS/external/linux/sndio-1.5.0/install/share/man/man8

#
# programs to build
#
PROG = sndiod
MAN8 = sndiod.8

all:		${PROG}

install:
		mkdir -p ${DESTDIR}${BIN_DIR} ${DESTDIR}${MAN8_DIR}
		rm -f ${DESTDIR}${BIN_DIR}/${PROG} ${DESTDIR}${MAN8_DIR}/${MAN8}
		cp ${PROG} ${DESTDIR}${BIN_DIR}
		cp ${MAN8} ${DESTDIR}${MAN8_DIR}

uninstall:
		cd ${DESTDIR}${BIN_DIR} && rm -f ${PROG}
		cd ${DESTDIR}${MAN8_DIR} && rm -f ${MAN8}

clean:
		rm -f -- *.o ${PROG}

# ---------------------------------------------------------- dependencies ---

OBJS = \
abuf.o utils.o dev.o dsp.o file.o listen.o midi.o miofile.o \
opt.o siofile.o sndiod.o sock.o

sndiod:		${OBJS}
		${CC} ${LDFLAGS} ${LIB} -o sndiod ${OBJS} ${LDADD}

.c.o:
		${CC} ${CFLAGS} ${INCLUDE} ${DEFS} -c $<

abuf.o:		abuf.c abuf.h utils.h
dev.o:		dev.c ../bsd-compat/bsd-compat.h abuf.h defs.h dev.h \
		dsp.h siofile.h file.h midi.h miofile.h sysex.h utils.h
dsp.o:		dsp.c dsp.h defs.h utils.h
file.o:		file.c file.h utils.h
listen.o:	listen.c listen.h file.h sock.h ../libsndio/amsg.h \
		utils.h ../bsd-compat/bsd-compat.h
midi.o:		midi.c abuf.h defs.h dev.h dsp.h siofile.h file.h midi.h \
		miofile.h sysex.h utils.h ../bsd-compat/bsd-compat.h
miofile.o:	miofile.c defs.h file.h midi.h abuf.h miofile.h utils.h
opt.o:		opt.c dev.h abuf.h dsp.h defs.h siofile.h file.h opt.h \
		utils.h
siofile.o:	siofile.c abuf.h defs.h dev.h dsp.h siofile.h file.h \
		utils.h
sndiod.o:	sndiod.c ../libsndio/amsg.h defs.h dev.h abuf.h dsp.h \
		siofile.h file.h listen.h midi.h miofile.h opt.h sock.h \
		utils.h ../bsd-compat/bsd-compat.h
sock.o:		sock.c abuf.h defs.h dev.h dsp.h siofile.h file.h midi.h \
		miofile.h opt.h sock.h ../libsndio/amsg.h utils.h
utils.o:	utils.c utils.h
