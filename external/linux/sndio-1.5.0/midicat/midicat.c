/*
 * Copyright (c) 2015 Alexandre Ratchov <alex@caoua.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <fcntl.h>
#include <sndio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "bsd-compat.h"

#define MIDI_BUFSZ	1024

char usagestr[] = "usage: midicat [-d] [-i in-file] [-o out-file] "
	"[-q in-port] [-q out-port]\n";

char *port0, *port1, *ifile, *ofile;
struct mio_hdl *ih, *oh;
unsigned char buf[MIDI_BUFSZ];
int buf_used = 0;
int ifd = -1, ofd = -1;
int dump;

static int
midi_flush(void)
{
	int i, n, sep;

	if (buf_used == 0)
		return 1;

	if (ofile != NULL) {
		n = write(ofd, buf, buf_used);
		if (n != buf_used) {
			fprintf(stderr, "%s: short write\n", ofile);
			buf_used = 0;
			return 0;
		}
	} else {
		n = mio_write(oh, buf, buf_used);
		if (n != buf_used) {
			fprintf(stderr, "%s: port disconnected\n",
			   ih == oh ? port0 : port1);
			buf_used = 0;
			return 0;
		}
	}

	if (dump) {
		for (i = 0; i < buf_used; i++) {
			sep = (i % 16 == 15 || i == buf_used - 1) ?
			    '\n' : ' ';
			fprintf(stderr, "%02x%c", buf[i], sep);
		}
	}

	buf_used = 0;
	return 1;
}

int
main(int argc, char **argv)
{
	int c, mode;

	while ((c = getopt(argc, argv, "di:o:q:")) != -1) {
		switch (c) {
		case 'd':
			dump = 1;
			break;
		case 'q':
			if (port0 == NULL)
				port0 = optarg;
			else if (port1 == NULL)
				port1 = optarg;
			else {
				fputs("too many -q options\n", stderr);
				return 1;
			}
			break;
		case 'i':
			ifile = optarg;
			break;
		case 'o':
			ofile = optarg;
			break;
		default:
			goto bad_usage;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc != 0) {
	bad_usage:
		fputs(usagestr, stderr);
		return 1;
	}

	/* we don't support more than one data flow */
	if (ifile != NULL && ofile != NULL) {
		fputs("-i and -o are exclusive\n", stderr);
		return 1;
	}

	/* second port makes sense only for port-to-port transfers */
	if (port1 != NULL && !(ifile == NULL && ofile == NULL)) {
		fputs("too many -q options\n", stderr);
		return 1;
	}

	/* if there're neither files nor ports, then we've nothing to do */
	if (port0 == NULL && ifile == NULL && ofile == NULL)
		goto bad_usage;

	/* if no port specified, use default one */
	if (port0 == NULL)
		port0 = MIO_PORTANY;

	/* open input or output file (if any) */
	if (ifile) {
		if (strcmp(ifile, "-") == 0)
			ifd = STDIN_FILENO;
		else {
			ifd = open(ifile, O_RDONLY, 0);
			if (ifd < 0) {
				perror(ifile);
				return 1;
			}
		}
	} else if (ofile) {
		if (strcmp(ofile, "-") == 0)
			ofd = STDOUT_FILENO;
		else {
			ofd = open(ofile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
			if (ofd < 0) {
				perror(ofile);
				return 1;
			}
		}
	}

	/* open first port for input and output (if output needed) */
	if (ofile)
		mode = MIO_IN;
	else if (ifile)
		mode = MIO_OUT;
	else if (port1 == NULL)
		mode = MIO_IN | MIO_OUT;
	else
		mode = MIO_IN;
	ih = mio_open(port0, mode, 0);
	if (ih == NULL) {
		fprintf(stderr, "%s: couldn't open port\n", port0);
		return 1;
	}

	/* open second port, output only */
	if (port1 == NULL)
		oh = ih;
	else {
		oh = mio_open(port1, MIO_OUT, 0);
		if (oh == NULL) {
			fprintf(stderr, "%s: couldn't open port\n", port1);
			exit(1);
		}
	}

	/* transfer until end-of-file or error */
	for (;;) {
		if (ifile != NULL) {
			buf_used = read(ifd, buf, sizeof(buf));
			if (buf_used < 0) {
				perror("stdin");
				break;
			}
			if (buf_used == 0)
				break;
			if (!midi_flush())
				break;
		} else {
			buf_used = mio_read(ih, buf, sizeof(buf));
			if (buf_used == 0) {
				fprintf(stderr, "%s: disconnected\n", port0);
				break;
			}
			if (!midi_flush())
				break;
		}
	}

	/* clean-up */
	if (port0)
		mio_close(ih);
	if (port1)
		mio_close(oh);
	if (ifile)
		close(ifd);
	if (ofile)
		close(ofd);
	return 0;
}
