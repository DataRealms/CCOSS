#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sndio.h>
#include "tools.h"

struct sio_par par;
struct sio_cap cap;

static void
print_enc(struct sio_enc *enc)
{
	printf("%s%d", enc->sig ? "s" : "u", enc->bits);
	if (enc->bps > 1)
		printf("%s", enc->le ? "le" : "be");
	if (enc->bps != SIO_BPS(enc->bits))
		printf("%d%s", enc->bps, enc->msb ? "msb" : "lsb");
}

static void
print_cap(struct sio_cap *cap)
{
	unsigned n, i;

	for (n = 0; n < cap->nconf; n++) {
		printf("config %d\n", n);
		printf("\tenc:");
		for (i = 0; i < SIO_NENC; i++) {
			if (cap->confs[n].enc & (1 << i)) {
				printf(" ");
				print_enc(&cap->enc[i]);
			}
		}
		printf("\n\tpchan:");
		for (i = 0; i < SIO_NCHAN; i++) {
			if (cap->confs[n].pchan & (1 << i))
				printf(" %d", cap->pchan[i]);
		}
		printf("\n\trchan:");
		for (i = 0; i < SIO_NCHAN; i++) {
			if (cap->confs[n].rchan & (1 << i))
				printf(" %d", cap->rchan[i]);
		}
		printf("\n\trate:");
		for (i = 0; i < SIO_NRATE; i++) {
			if (cap->confs[n].rate & (1 << i))
				printf(" %d", cap->rate[i]);
		}
		printf("\n");
	}
}

int
main(int argc, char **argv) {
	int ch;
	unsigned mode = SIO_PLAY | SIO_REC;
	struct sio_hdl *hdl;

	while ((ch = getopt(argc, argv, "pr")) != -1) {
		switch(ch) {
		case 'p':
			mode &= ~SIO_REC;
			break;
		case 'r':
			mode &= ~SIO_PLAY;
			break;
		default:
			fprintf(stderr, "usage: cap [-pr]\n");
			exit(1);
			break;
		}
	}
	if (mode == 0) {
		fprintf(stderr, "-p and -r flags are mutually exclusive\n");
		exit(1);
	}
	hdl = sio_open(NULL, mode, 0);
	if (hdl == NULL) {
		fprintf(stderr, "sio_open() failed\n");
		exit(1);
	}
	if (!sio_getcap(hdl, &cap)) {
		fprintf(stderr, "sio_getcap() failed\n");
		exit(1);
	}
	print_cap(&cap);
	sio_close(hdl);
	return 0;
}
