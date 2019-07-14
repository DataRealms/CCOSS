#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sndio.h>
#include "tools.h"

void cb(void *, int);
void usage(void);

unsigned char *buf;
struct sio_par par;
char *xstr[] = SIO_XSTRINGS;

unsigned long long recpos = 0, readpos = 0;
int tick = 0;

void
cb(void *addr, int delta)
{
	unsigned int bytes;

	bytes = delta * par.bps * par.rchan;
	// fprintf(stderr, "advanced by %d\n", bytes);
	recpos += bytes;
	tick = 1;
}

void
usage(void)
{
	fprintf(stderr,
	    "usage: rec [-b size] [-c nchan] [-e enc] [-n nbytes] "
	    "[-r rate] [-x xrun]\n");
}

int
main(int argc, char **argv)
{
	int ch;
	struct sio_hdl *hdl;
	size_t bufsz;
	ssize_t n, nbytes;
	
	/*
	 * defaults parameters
	 */
	sio_initpar(&par);
	par.sig = 1;
	par.bits = 16;
	par.rchan = 2;
	par.rate = 48000;
	nbytes = -1;

	while ((ch = getopt(argc, argv, "b:c:e:n:r:x:")) != -1) {
		switch (ch) {
		case 'b':
			if (sscanf(optarg, "%u", &par.appbufsz) != 1) {
				fprintf(stderr, "%s: bad buf size\n", optarg);
				exit(1);
			}
			break;
		case 'c':
			if (sscanf(optarg, "%u", &par.rchan) != 1) {
				fprintf(stderr, "%s: bad channels number\n", optarg);
				exit(1);
			}
			break;
		case 'e':
			if (!strtoenc(&par, optarg)) {
				fprintf(stderr, "%s: unknown encoding\n", optarg);
				exit(1);
			}
			break;
		case 'n':
			if (sscanf(optarg, "%zu", &nbytes) != 1) {
				fprintf(stderr, "%s: bad bytes count\n", optarg);
				exit(1);
			}
			break;
		case 'r':
			if (sscanf(optarg, "%u", &par.rate) != 1) {
				fprintf(stderr, "%s: bad rate\n", optarg);
				exit(1);
			}
			break;
		case 'x':
			for (par.xrun = 0;; par.xrun++) {
				if (par.xrun == sizeof(xstr) / sizeof(char *)) {
					fprintf(stderr, 
					    "%s: bad xrun mode\n", optarg);
					exit(1);
				}
				if (strcmp(xstr[par.xrun], optarg) == 0)
					break;
			}
			break;			
		default:
			usage();
			exit(1);
			break;
		}
	}

	hdl = sio_open(SIO_DEVANY, SIO_REC, 0);
	if (hdl == NULL) {
		fprintf(stderr, "sio_open() failed\n");
		exit(1);
	}
	sio_onmove(hdl, cb, NULL);
	if (!sio_setpar(hdl, &par)) {
		fprintf(stderr, "sio_setpar() failed\n");
		exit(1);
	}
	if (!sio_getpar(hdl, &par)) {
		fprintf(stderr, "sio_getpar() failed\n");
		exit(1);
	}
	bufsz = par.bps * par.rchan * par.round;
	buf = malloc(bufsz);
	if (buf == NULL) {
		fprintf(stderr, "failed to allocate %zu bytes\n", bufsz);
		exit(1);
	}
	fprintf(stderr, "%zu bytes buffer, max latency %u frames\n",
	    bufsz, par.bufsz);
	if (!sio_start(hdl)) {
		fprintf(stderr, "sio_start() failed\n");
		exit(1);
	}
	while (nbytes != 0) {
		n = bufsz;
		if (nbytes >= 0 && n > nbytes)
			n = nbytes;
		n = sio_read(hdl, buf, n);
		if (n == 0) {
			fprintf(stderr, "sio_write: failed\n");
			exit(1);
		}
		nbytes -= n;
		readpos += n;
		if (tick) {
			fprintf(stderr,
			    "recpos = %lld, readpos = %lld, latency = %lld\n",
			    recpos, readpos, recpos - readpos);
			tick = 0;
		}
		if (write(STDOUT_FILENO, buf, n) < 0) {
			perror("stdout");
			exit(1);
		}
	}
	sio_close(hdl);
	return 0;
}
