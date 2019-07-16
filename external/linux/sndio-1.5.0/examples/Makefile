# extra includes paths (-I options)
INCLUDE = -I../libsndio

# extra libraries paths (-L options)
LIB = -L../libsndio

# extra defines (-D options)
DEFS = -DDEBUG -D_GNU_SOURCE -DHAVE_SOCK_CLOEXEC -DHAVE_CLOCK_GETTIME -DUSE_ALSA

# extra libraries (-l options)
LDADD = -lsndio -lrt -lasound

# variables defined on configure script command line (if any)


PROG = play rec fd vol cap

all:		${PROG}

play:		play.o tools.o
		${CC} ${LDFLAGS} ${LIB} -o play play.o tools.o ${LDADD}

rec:		rec.o tools.o
		${CC} ${LDFLAGS} ${LIB} -o rec rec.o tools.o ${LDADD}

fd:		fd.o tools.o
		${CC} ${LDFLAGS} ${LIB} -o fd fd.o tools.o ${LDADD}

vol:		vol.o tools.o
		${CC} ${LDFLAGS} ${LIB} -o vol vol.o tools.o ${LDADD}

cap:		cap.o tools.o
		${CC} ${LDFLAGS} ${LIB} -o cap cap.o tools.o ${LDADD}

.c.o:
		${CC} ${CFLAGS} ${INCLUDE} ${DEFS} -c $<

tools.o:	tools.c
play.o:		play.c tools.h
rec.o:		rec.c tools.h
fd.o:		fd.c tools.h
cap.o:		cap.c tools.h
vol.o:		vol.c tools.h

clean:
		rm -f ${PROG} *.o
