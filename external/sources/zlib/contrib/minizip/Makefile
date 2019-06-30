CC=cc
CFLAGS=-O -I../..

AR=ar rc
RANLIB=ranlib

LIB_OBJS = zip.o unzip.o ioapi.o ../../libz.a
UNZ_OBJS = miniunz.o unzip.o ioapi.o ../../libz.a
ZIP_OBJS = minizip.o zip.o   ioapi.o ../../libz.a

.c.o:
	$(CC) -c $(CFLAGS) $*.c

all: miniunz minizip

miniunz:  $(UNZ_OBJS)
	$(CC) $(CFLAGS) -o $@ $(UNZ_OBJS)

minizip:  $(ZIP_OBJS)
	$(CC) $(CFLAGS) -o $@ $(ZIP_OBJS)

libzipio.a: $(LIB_OBJS)
	$(AR) $@ $(LIB_OBJS)
	-@ ($(RANLIB) $@ || true) >/dev/null 2>&1

test:	miniunz minizip
	./minizip test readme.txt
	./miniunz -l test.zip
	mv readme.txt readme.old
	./miniunz test.zip

clean:
	/bin/rm -f *.o *~ minizip miniunz
