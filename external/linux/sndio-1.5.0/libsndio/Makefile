# extra includes paths (-I options)
INCLUDE = -I../bsd-compat

# extra libraries paths (-L options)
LIB =

# extra defines (-D options)
DEFS = -DDEBUG -D_GNU_SOURCE -DHAVE_SOCK_CLOEXEC -DHAVE_CLOCK_GETTIME -DUSE_ALSA

# extra libraries (-l options)
LDADD = -lrt -lasound

# extra compiler flags to produce objects for shared library
SO_CFLAGS = -fPIC

# extra compiler flags to produce a shared library with the given name
SO_LDFLAGS = -shared -Wl,-soname=libsndio.so.${MAJ}.${MIN}

# variables defined on configure script command line (if any)


#
# headers, libraries and man pages installation paths
#
INCLUDE_DIR = /home/ian/source/CCOSS/external/linux/sndio-1.5.0/install/include
LIB_DIR = /home/ian/source/CCOSS/external/linux/sndio-1.5.0/install/lib
MAN3_DIR = /home/ian/source/CCOSS/external/linux/sndio-1.5.0/install/share/man/man3
MAN7_DIR = /home/ian/source/CCOSS/external/linux/sndio-1.5.0/install/share/man/man7

# man3 and man7 pages
MAN3 = \
	sio_open.3 \
	sio_close.3 sio_setpar.3 sio_getpar.3 sio_getcap.3 \
	sio_start.3 sio_stop.3 sio_read.3 sio_write.3 sio_onmove.3 \
	sio_nfds.3 sio_pollfd.3 sio_revents.3 sio_eof.3 \
	sio_setvol.3 sio_onvol.3 sio_initpar.3 \
	mio_open.3 \
	mio_close.3 mio_read.3 mio_write.3 mio_nfds.3 mio_pollfd.3 \
	mio_revents.3 mio_eof.3
MAN7 = sndio.7

#
# libraries to build and install
#
MAJ = 7
MIN = 0
SO = libsndio.so.${MAJ}.${MIN}
SO_LINK = libsndio.so

all:		${SO} ${SO_LINK}


install:
		mkdir -p ${DESTDIR}${INCLUDE_DIR}
		mkdir -p ${DESTDIR}${LIB_DIR}
		mkdir -p ${DESTDIR}${MAN3_DIR}
		mkdir -p ${DESTDIR}${MAN7_DIR}
		cp sndio.h ${DESTDIR}${INCLUDE_DIR}
		cp -R ${SO} ${SO_LINK} ${DESTDIR}${LIB_DIR}
		cp sio_open.3 ${DESTDIR}${MAN3_DIR}
		ln -sf sio_open.3 ${DESTDIR}${MAN3_DIR}/sio_close.3
		ln -sf sio_open.3 ${DESTDIR}${MAN3_DIR}/sio_setpar.3
		ln -sf sio_open.3 ${DESTDIR}${MAN3_DIR}/sio_getpar.3
		ln -sf sio_open.3 ${DESTDIR}${MAN3_DIR}/sio_getcap.3
		ln -sf sio_open.3 ${DESTDIR}${MAN3_DIR}/sio_start.3
		ln -sf sio_open.3 ${DESTDIR}${MAN3_DIR}/sio_stop.3
		ln -sf sio_open.3 ${DESTDIR}${MAN3_DIR}/sio_read.3
		ln -sf sio_open.3 ${DESTDIR}${MAN3_DIR}/sio_write.3
		ln -sf sio_open.3 ${DESTDIR}${MAN3_DIR}/sio_onmove.3
		ln -sf sio_open.3 ${DESTDIR}${MAN3_DIR}/sio_nfds.3
		ln -sf sio_open.3 ${DESTDIR}${MAN3_DIR}/sio_pollfd.3
		ln -sf sio_open.3 ${DESTDIR}${MAN3_DIR}/sio_revents.3
		ln -sf sio_open.3 ${DESTDIR}${MAN3_DIR}/sio_eof.3
		ln -sf sio_open.3 ${DESTDIR}${MAN3_DIR}/sio_setvol.3
		ln -sf sio_open.3 ${DESTDIR}${MAN3_DIR}/sio_onvol.3
		ln -sf sio_open.3 ${DESTDIR}${MAN3_DIR}/sio_initpar.3
		cp mio_open.3 ${DESTDIR}${MAN3_DIR}
		ln -sf mio_open.3 ${DESTDIR}${MAN3_DIR}/mio_close.3
		ln -sf mio_open.3 ${DESTDIR}${MAN3_DIR}/mio_read.3
		ln -sf mio_open.3 ${DESTDIR}${MAN3_DIR}/mio_write.3
		ln -sf mio_open.3 ${DESTDIR}${MAN3_DIR}/mio_nfds.3
		ln -sf mio_open.3 ${DESTDIR}${MAN3_DIR}/mio_pollfd.3
		ln -sf mio_open.3 ${DESTDIR}${MAN3_DIR}/mio_revents.3
		ln -sf mio_open.3 ${DESTDIR}${MAN3_DIR}/mio_eof.3
		cp sndio.7 ${DESTDIR}${MAN7_DIR}

uninstall:
		rm -f ${DESTDIR}${INCLUDE_DIR}/sndio.h
		cd ${DESTDIR}${LIB_DIR} && rm -f ${SO} ${SO_LINK}
		cd ${DESTDIR}${MAN3_DIR} && rm -f ${MAN3}
		cd ${DESTDIR}${MAN7_DIR} && rm -f ${MAN7}

clean:
		rm -f -- ${SO} ${SO_LINK} *.o

# ---------------------------------------------------------- dependencies ---

#
# object files, sorted following dependencies to allow the
# loader to determine dependencies in a single pass
#
OBJS = debug.o aucat.o \
mio.o mio_rmidi.o mio_alsa.o mio_aucat.o \
sio.o sio_alsa.o sio_aucat.o sio_oss.o sio_sun.o \
issetugid.o strlcat.o strlcpy.o strtonum.o clock_gettime.o

.c.o:
		${CC} ${CFLAGS} ${SO_CFLAGS} -I. ${INCLUDE} ${DEFS} -o $@ -c $<

${SO}:		${OBJS}
		${CC} ${LDFLAGS} ${SO_CFLAGS} ${SO_LDFLAGS} -o ${SO} ${OBJS} ${LDADD}

${SO_LINK}:
		ln -sf ${SO} ${SO_LINK}

issetugid.o:	../bsd-compat/issetugid.c
		${CC} ${CFLAGS} ${SO_CFLAGS} ${INCLUDE} ${DEFS} -c -o issetugid.o ../bsd-compat/issetugid.c

strlcat.o:	../bsd-compat/strlcat.c
		${CC} ${CFLAGS} ${SO_CFLAGS} ${INCLUDE} ${DEFS} -c -o strlcat.o ../bsd-compat/strlcat.c

strlcpy.o:	../bsd-compat/strlcpy.c
		${CC} ${CFLAGS} ${SO_CFLAGS} ${INCLUDE} ${DEFS} -c -o strlcpy.o ../bsd-compat/strlcpy.c

strtonum.o:	../bsd-compat/strtonum.c
		${CC} ${CFLAGS} ${SO_CFLAGS} ${INCLUDE} ${DEFS} -c -o strtonum.o ../bsd-compat/strtonum.c

clock_gettime.o:	../bsd-compat/clock_gettime.c
		${CC} ${CFLAGS} ${SO_CFLAGS} ${INCLUDE} ${DEFS} -c -o clock_gettime.o ../bsd-compat/clock_gettime.c

aucat.o:	aucat.c aucat.h amsg.h debug.h \
		../bsd-compat/bsd-compat.h
debug.o:	debug.c debug.h ../bsd-compat/bsd-compat.h
mio.o:		mio.c debug.h mio_priv.h sndio.h \
		../bsd-compat/bsd-compat.h
mio_alsa.o:	mio_alsa.c debug.h mio_priv.h sndio.h
mio_aucat.o:	mio_aucat.c aucat.h amsg.h debug.h mio_priv.h sndio.h \
		../bsd-compat/bsd-compat.h
mio_rmidi.o:	mio_rmidi.c debug.h mio_priv.h sndio.h
sio.o:		sio.c debug.h sio_priv.h sndio.h \
		../bsd-compat/bsd-compat.h
sio_alsa.o:	sio_alsa.c debug.h sio_priv.h sndio.h \
		../bsd-compat/bsd-compat.h
sio_aucat.o:	sio_aucat.c aucat.h amsg.h debug.h sio_priv.h sndio.h \
		../bsd-compat/bsd-compat.h
sio_oss.o:	sio_oss.c debug.h sio_priv.h sndio.h \
		../bsd-compat/bsd-compat.h
sio_sun.o:	sio_sun.c debug.h sio_priv.h sndio.h \
		../bsd-compat/bsd-compat.h
