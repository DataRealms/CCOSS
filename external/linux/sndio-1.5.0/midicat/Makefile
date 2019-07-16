# extra includes paths (-I options)
INCLUDE = -I../libsndio -I../bsd-compat

# extra libraries paths (-L options)
LIB = -L../libsndio

# extra defines (-D options)
DEFS = -DDEBUG -D_GNU_SOURCE -DHAVE_SOCK_CLOEXEC -DHAVE_CLOCK_GETTIME -DUSE_ALSA

# extra libraries (-l options)
LDADD = -lsndio -lrt -lasound

# variables defined on configure script command line (if any)


#
# binaries, documentation, man pages and examples will be installed in 
# ${BIN_DIR}, ${MAN1_DIR}
#
BIN_DIR = /home/ian/source/CCOSS/external/linux/sndio-1.5.0/install/bin
MAN1_DIR = /home/ian/source/CCOSS/external/linux/sndio-1.5.0/install/share/man/man1

#
# programs to build
#
PROG = midicat
MAN1 = midicat.1

all:		${PROG} ${MAN1}

install:
		mkdir -p ${DESTDIR}${BIN_DIR} ${DESTDIR}${MAN1_DIR}
		cp midicat ${DESTDIR}${BIN_DIR}
		cp midicat.1 ${DESTDIR}${MAN1_DIR}

uninstall:
		cd ${DESTDIR}${BIN_DIR} && rm -f ${PROG}
		cd ${DESTDIR}${MAN1_DIR} && rm -f ${MAN1}

clean:
		rm -f -- *.o midicat

# ---------------------------------------------------------- dependencies ---

OBJS = midicat.o

midicat:	${OBJS}
		${CC} ${LDFLAGS} ${LIB} -o midicat ${OBJS} ${LDADD} 

.c.o:
		${CC} ${CFLAGS} ${INCLUDE} ${DEFS} -c $<

midicat.o:	midicat.c
