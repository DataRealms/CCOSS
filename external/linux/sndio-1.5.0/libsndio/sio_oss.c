/*	$OpenBSD$	*/
/*
 * Copyright (c) 2008 Alexandre Ratchov <alex@caoua.org>
 * Copyright (c) 2016 Tobias Kortkamp <t@tobik.me>
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

#ifdef USE_OSS
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "sio_priv.h"
#include "bsd-compat.h"

#define DEVPATH_PREFIX	"/dev/dsp"
#define DEVPATH_MAX 	(1 +		\
	sizeof(DEVPATH_PREFIX) - 1 +	\
	sizeof(int) * 3)

struct sio_oss_fmt {
	int fmt;
	unsigned int bits;
	unsigned int bps;
	unsigned int sig;
	unsigned int le;
	unsigned int msb;
};

static struct sio_oss_fmt formats[] = {
	/* See http://manuals.opensound.com/developer/formats.html.
	 * AFMT_{S8,U16}_* are marked as obsolete so are missing here.
	 */

	/* le+msb not important */
	{ AFMT_U8,	8, 1, 0, 0, 0 },
	{ AFMT_U8,	8, 1, 0, 1, 0 },
	{ AFMT_U8,	8, 1, 0, 0, 1 },
	{ AFMT_U8,	8, 1, 0, 1, 1 },

	/* msb not important */
	{ AFMT_S16_BE, 16, 2, 1, 0, 0 },
	{ AFMT_S16_BE, 16, 2, 1, 0, 1 },
	{ AFMT_S16_LE, 16, 2, 1, 1, 0 },
	{ AFMT_S16_LE, 16, 2, 1, 1, 1 },
	{ AFMT_S24_BE, 24, 3, 1, 0, 0 },
	{ AFMT_S24_BE, 24, 3, 1, 0, 1 },
	{ AFMT_S24_LE, 24, 3, 1, 1, 0 },
	{ AFMT_S24_LE, 24, 3, 1, 1, 1 },
	{ AFMT_U24_BE, 24, 3, 0, 0, 0 },
	{ AFMT_U24_BE, 24, 3, 0, 0, 1 },
	{ AFMT_U24_LE, 24, 3, 0, 1, 0 },
	{ AFMT_U24_LE, 24, 3, 0, 1, 1 },

	{ AFMT_S32_BE, 32, 4, 1, 0, 1 },
	{ AFMT_S32_LE, 32, 4, 1, 1, 1 },
	{ AFMT_U32_BE, 32, 4, 0, 0, 1 },
	{ AFMT_U32_LE, 32, 4, 0, 1, 1 },
};

struct sio_oss_hdl {
	struct sio_hdl sio;
	int fd;
	int idelta, odelta;
	int iused;
	int oused;
	int bpf;

	int fmt;
	unsigned int rate;
	unsigned int chan;
	unsigned int appbufsz;
	unsigned int round;

	int filling;
};

static struct sio_hdl *sio_oss_fdopen(const char *, int, unsigned int, int);
static int sio_oss_getcap(struct sio_hdl *, struct sio_cap *);
static int sio_oss_getfd(const char *, unsigned int, int);
static int sio_oss_getpar(struct sio_hdl *, struct sio_par *);
static int sio_oss_nfds(struct sio_hdl *);
static int sio_oss_pollfd(struct sio_hdl *, struct pollfd *, int);
static int sio_oss_revents(struct sio_hdl *, struct pollfd *);
static int sio_oss_setpar(struct sio_hdl *, struct sio_par *);
static int sio_oss_start(struct sio_hdl *);
static int sio_oss_stop(struct sio_hdl *);
static int sio_oss_xrun(struct sio_oss_hdl *);
static size_t sio_oss_read(struct sio_hdl *, void *, size_t);
static size_t sio_oss_write(struct sio_hdl *, const void *, size_t);
static void sio_oss_close(struct sio_hdl *);
static int sio_oss_setvol(struct sio_hdl *, unsigned int);
static void sio_oss_getvol(struct sio_hdl *);

static struct sio_ops sio_oss_ops = {
	sio_oss_close,
	sio_oss_setpar,
	sio_oss_getpar,
	sio_oss_getcap,
	sio_oss_write,
	sio_oss_read,
	sio_oss_start,
	sio_oss_stop,
	sio_oss_nfds,
	sio_oss_pollfd,
	sio_oss_revents,
	sio_oss_setvol,
	sio_oss_getvol,
};

/*
 * guess device capabilities
 */
static int
sio_oss_getcap(struct sio_hdl *sh, struct sio_cap *cap)
{
	/* From sound(4):
	 * The FreeBSD multichannel matrix processor supports up to 18
	 * interleaved channels, but the limit is currently set to 8
	 * channels (as commonly used for 7.1 surround sound).
	 */
	static unsigned int chans[] = {
		1, 2, 4, 6, 8
	};
	static unsigned int rates[] = {
		8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100,
		48000, 64000, 88200, 96000, 192000
	};
	static int afmts[] = {
		AFMT_U8, AFMT_S16_LE, AFMT_S16_BE, AFMT_S24_LE, AFMT_U24_LE,
		AFMT_S32_LE, AFMT_U32_LE
	};
	struct sio_oss_hdl *hdl = (struct sio_oss_hdl *)sh;
	unsigned int nconf = 0;
	unsigned int enc_map = 0, rchan_map = 0, pchan_map = 0, rate_map;
	unsigned int i, j, k, conf;
	int fmts;

	if (ioctl(hdl->fd, SNDCTL_DSP_GETFMTS, &fmts) < 0) {
		DPERROR("sio_oss_getcap: GETFMTS");
		hdl->sio.eof = 1;
		return 0;
	}

	/*
	 * get a subset of supported encodings
	 */
	for (j = 0, i = 0; i < sizeof(afmts) / sizeof(afmts[0]); i++) {
		if (fmts & afmts[i]) {
			for (k = 0; k < sizeof(formats) / sizeof(formats[0]); k++) {
				if (formats[k].fmt == afmts[i]) {
					cap->enc[j].sig = formats[k].sig;
					cap->enc[j].bits = formats[k].bits;
					cap->enc[j].bps = formats[k].bps;
					cap->enc[j].le = formats[k].le;
					cap->enc[j].msb = formats[k].msb;
					enc_map |= 1 << j;
					j++;
					break;
				}
			}
		}
	}

	/*
	 * fill channels
	 */
	if (hdl->sio.mode & SIO_PLAY) {
		for (i = 0; i < sizeof(chans) / sizeof(chans[0]); i++) {
			cap->pchan[i] = chans[i];
			pchan_map |= (1 << i);
		}
	}
	if (hdl->sio.mode & SIO_REC) {
		for (i = 0; i < sizeof(chans) / sizeof(chans[0]); i++) {
			cap->rchan[i] = chans[i];
			rchan_map |= (1 << i);
		}
	}

	/*
	 * fill rates
	 */
	for (j = 0; j < sizeof(formats) / sizeof(formats[0]); j++) {
		rate_map = 0;
		if ((enc_map & (1 << j)) == 0)
			continue;
		for (i = 0; i < sizeof(rates) / sizeof(rates[0]); i++) {
			cap->rate[i] = rates[i];
			rate_map |= (1 << i);
		}
		for (conf = 0; conf < nconf; conf++) {
			if (cap->confs[conf].rate == rate_map) {
				cap->confs[conf].enc |= (1 << j);
				break;
			}
		}
		if (conf == nconf) {
			if (nconf == SIO_NCONF)
				break;
			cap->confs[nconf].enc = (1 << j);
			cap->confs[nconf].pchan = pchan_map;
			cap->confs[nconf].rchan = rchan_map;
			cap->confs[nconf].rate = rate_map;
			nconf++;
		}
	}
	cap->nconf = nconf;

	return 1;
}

static int
sio_oss_getfd(const char *str, unsigned int mode, int nbio)
{
	const char *p;
	char path[DEVPATH_MAX];
	unsigned int devnum;
	int fd, flags, val;
	audio_buf_info bi;

	p = _sndio_parsetype(str, "rsnd");
	if (p == NULL) {
		DPRINTF("sio_oss_getfd: %s: \"rsnd\" expected\n", str);
		return -1;
	}
	switch (*p) {
	case '/':
		p++;
		break;
	default:
		DPRINTF("sio_oss_getfd: %s: '/' expected\n", str);
		return -1;
	}
	if (strcmp(p, "default") == 0) {
		strlcpy(path, DEVPATH_PREFIX, sizeof(path));
	} else {
		p = _sndio_parsenum(p, &devnum, 255);
		if (p == NULL || *p != '\0') {
			DPRINTF("sio_sun_getfd: %s: number expected after '/'\n", str);
			return -1;
		}
		snprintf(path, sizeof(path), DEVPATH_PREFIX "%u", devnum);
	}
	if (mode == (SIO_PLAY | SIO_REC))
		flags = O_RDWR;
	else
		flags = (mode & SIO_PLAY) ? O_WRONLY : O_RDONLY;
	while ((fd = open(path, flags | O_NONBLOCK | O_CLOEXEC)) < 0) {
		if (errno == EINTR)
			continue;
		DPERROR(path);
		return -1;
	}

	/*
	 * Check if the device supports playing/recording.
	 * Unfortunately, it's possible for devices to be opened RDWR
	 * even when they don't support playing/recording.
	 */
	if (mode & SIO_PLAY && ioctl(fd, SNDCTL_DSP_GETOSPACE, &bi) < 0) {
		close(fd);
		return -1;
	}
	if (mode & SIO_REC && ioctl(fd, SNDCTL_DSP_GETISPACE, &bi) < 0) {
		close(fd);
		return -1;
	}

	val = 1;
	if (ioctl(fd, SNDCTL_DSP_LOW_WATER, &val) < 0) {
		DPERROR("sio_oss_start: LOW_WATER");
		close(fd);
		return -1;
	}
	return fd;
}

static struct sio_hdl *
sio_oss_fdopen(const char *str, int fd, unsigned int mode, int nbio)
{
	struct sio_oss_hdl *hdl;

	hdl = malloc(sizeof(struct sio_oss_hdl));
	if (hdl == NULL)
		return NULL;
	_sio_create(&hdl->sio, &sio_oss_ops, mode, nbio);

	/* Set default device parameters */
	hdl->fmt = AFMT_S16_LE;
	hdl->rate = 48000;
	hdl->chan = 2;
	hdl->round = 960;
	hdl->appbufsz = 8 * 960;
	hdl->filling = 0;
	hdl->fd = fd;

	return (struct sio_hdl *)hdl;
}

struct sio_hdl *
_sio_oss_open(const char *str, unsigned int mode, int nbio)
{
	struct sio_oss_hdl *hdl;
	int fd;

	fd = sio_oss_getfd(str, mode, nbio);
	if (fd < 0)
		return NULL;

	hdl = (struct sio_oss_hdl *)sio_oss_fdopen(str, fd, mode, nbio);
	if (hdl != NULL)
		return (struct sio_hdl*)hdl;

	while (close(fd) < 0 && errno == EINTR)
		; /* retry */

	return NULL;
}

static void
sio_oss_close(struct sio_hdl *sh)
{
	struct sio_oss_hdl *hdl = (struct sio_oss_hdl *)sh;

	while (close(hdl->fd) < 0 && errno == EINTR)
		; /* retry */
	free(hdl);
}

static int
sio_oss_start(struct sio_hdl *sh)
{
	struct sio_oss_hdl *hdl = (struct sio_oss_hdl *)sh;
	int trig;

	hdl->iused = 0;
	hdl->oused = 0;
	hdl->idelta = 0;
	hdl->odelta = 0;

	if (hdl->sio.mode & SIO_PLAY) {
		/*
		 * keep the device paused and let sio_oss_pollfd() trigger the
		 * start later, to avoid buffer underruns
		 */
		hdl->filling = 1;
		trig = 0;
	} else {
		/*
		 * no play buffers to fill, start now!
		 */
		trig = PCM_ENABLE_INPUT;
		_sio_onmove_cb(&hdl->sio, 0);
	}
	if (ioctl(hdl->fd, SNDCTL_DSP_SETTRIGGER, &trig) < 0) {
		DPERROR("sio_oss_start: SETTRIGGER");
		hdl->sio.eof = 1;
		return 0;
	}
	return 1;
}

static int
sio_oss_stop(struct sio_hdl *sh)
{
	struct sio_oss_hdl *hdl = (struct sio_oss_hdl*)sh;
	int trig;

	if (hdl->filling) {
		hdl->filling = 0;
		return 1;
	}
	trig = 0;
	if (ioctl(hdl->fd, SNDCTL_DSP_SETTRIGGER, &trig) < 0) {
		DPERROR("sio_oss_stop: SETTRIGGER");
		hdl->sio.eof = 1;
		return 0;
	}
	return 1;
}

static int
sio_oss_setpar(struct sio_hdl *sh, struct sio_par *par)
{
	struct sio_oss_hdl *hdl = (struct sio_oss_hdl *)sh;
	unsigned int i, round, bufsz;
	int frag_max, frag_shift, frag_count, frag;
	unsigned int le, sig, msb;

	le = par->le;
	sig = par->sig;
	msb = par->msb;

	if (le == ~0U)
		le = 0;
	if (sig == ~0U)
		sig = 0;
	if (msb == ~0U)
		msb = 0;

	hdl->fmt = AFMT_S16_LE;
	for (i = 0; i < sizeof(formats)/sizeof(formats[0]); i++) {
		if (formats[i].bits == par->bits &&
		    formats[i].le == le &&
		    formats[i].sig == sig &&
		    formats[i].msb == msb) {
			hdl->fmt = formats[i].fmt;
			break;
		}
	}

	if (par->rate != ~0U)
		hdl->rate = par->rate;
	if (hdl->rate < 8000)
		hdl->rate = 8000;
	if (hdl->rate > 192000)
		hdl->rate = 192000;

	if (hdl->sio.mode & SIO_PLAY)
		hdl->chan = par->pchan;
	else if (hdl->sio.mode & SIO_REC)
		hdl->chan = par->rchan;

	if (ioctl(hdl->fd, SNDCTL_DSP_SETFMT, &hdl->fmt) < 0) {
		DPERROR("sio_oss_setpar: SETFMT");
		hdl->sio.eof = 1;
		return 0;
	}

	for (i = 0; ; i++) {
		if (i == sizeof(formats) / sizeof(formats[0])) {
			DPRINTF("sio_oss_setpar: unknown fmt %d\n", hdl->fmt);
			hdl->sio.eof = 1;
			return 0;
		}
		if (formats[i].fmt == hdl->fmt)
			break;
	}

	if (ioctl(hdl->fd, SNDCTL_DSP_SPEED, &hdl->rate) < 0) {
		DPERROR("sio_oss_setpar: SPEED");
		hdl->sio.eof = 1;
		return 0;
	}

	if (ioctl(hdl->fd, SNDCTL_DSP_CHANNELS, &hdl->chan) < 0) {
		DPERROR("sio_oss_setpar: CHANNELS");
		hdl->sio.eof = 1;
		return 0;
	}

	hdl->bpf = formats[i].bps * hdl->chan;

	if (par->round != ~0U && par->appbufsz != ~0U) {
		round = par->round;
		bufsz = par->appbufsz;
	} else if (par->round != ~0U) {
		round = par->round;
		bufsz = 2 * par->round;
	} else if (par->appbufsz != ~0U) {
		round = par->appbufsz / 2;
		bufsz = par->appbufsz;
	} else {
		/*
		 * even if it's not specified, we have to set the
		 * block size to ensure that both play and record
		 * direction get the same block size. Pick an
		 * arbitrary value that would work for most players at
		 * 48kHz, stereo, 16-bit.
		 */
		round = 512;
		bufsz = 1024;
	}

	frag_max = round * hdl->chan * formats[i].bps;
	frag_shift = 8;
	while (1 << (frag_shift + 1) <= frag_max)
		frag_shift++;

	frag_count = bufsz / round;
	if (frag_count < 2)
		frag_count = 2;

	frag = frag_count << 16 | frag_shift;
	if (ioctl(hdl->fd, SNDCTL_DSP_SETFRAGMENT, &frag) < 0) {
		DPERROR("sio_oss_setpar: SETFRAGMENT");
		hdl->sio.eof = 1;
		return 0;
	}

	return 1;
}

static int
sio_oss_getpar(struct sio_hdl *sh, struct sio_par *par)
{
	struct sio_oss_hdl *hdl = (struct sio_oss_hdl *)sh;
	unsigned int i, found = 0;
	audio_buf_info pbi, rbi;

	for (i = 0; i < sizeof(formats)/sizeof(formats[0]); i++) {
		if (formats[i].fmt == hdl->fmt) {
			par->sig = formats[i].sig;
			par->le = formats[i].le;
			par->bits = formats[i].bits;
			par->bps = formats[i].bps;
			par->msb = formats[i].msb;
			found = 1;
			break;
		}
	}
	if (!found) {
		DPRINTF("sio_oss_getpar: unknown format %d\n", hdl->fmt);
		hdl->sio.eof = 1;
		return 0;
	}

	par->rate = hdl->rate;
	par->pchan = hdl->chan;
	par->rchan = hdl->chan;
	par->xrun = SIO_IGNORE;

	if (hdl->sio.mode & SIO_PLAY) {
		if (ioctl(hdl->fd, SNDCTL_DSP_GETOSPACE, &pbi) < 0) {
			DPERROR("sio_oss_getpar: SNDCTL_DSP_GETOSPACE");
			hdl->sio.eof = 1;
			return 0;
		}
		par->round = pbi.fragsize / (par->pchan * par->bps);
		par->bufsz = pbi.fragstotal * par->round;
	}
	if (hdl->sio.mode & SIO_REC) {
		if (ioctl(hdl->fd, SNDCTL_DSP_GETISPACE, &rbi) < 0) {
			DPERROR("sio_oss_getpar: SNDCTL_DSP_GETISPACE");
			hdl->sio.eof = 1;
			return 0;
		}
		if (!(hdl->sio.mode & SIO_PLAY)) {
			par->round = rbi.fragsize / (par->rchan * par->bps);
			par->bufsz = rbi.fragstotal * par->round;
		}
	}
	par->appbufsz = par->bufsz;
#ifdef DEBUG
	if ((hdl->sio.mode & (SIO_REC | SIO_PLAY)) == (SIO_REC | SIO_PLAY)) {
		if (pbi.fragsize != rbi.fragsize) {
			DPRINTF("sio_oss_getpar: frag size/count mismatch\n"
			    "play: count = %d, size = %d\n"
			    "rec:  count = %d, size = %d\n",
			    pbi.fragstotal, pbi.fragsize,
			    rbi.fragstotal, rbi.fragsize);
			hdl->sio.eof = 1;
			return 0;
		}
	}
#endif
	return 1;
}

static size_t
sio_oss_read(struct sio_hdl *sh, void *buf, size_t len)
{
	struct sio_oss_hdl *hdl = (struct sio_oss_hdl *)sh;
	ssize_t n;

	while ((n = read(hdl->fd, buf, len)) < 0) {
		if (errno == EINTR)
			continue;
		if (errno != EAGAIN) {
			DPERROR("sio_oss_read: read");
			hdl->sio.eof = 1;
		}
		return 0;
	}
	if (n == 0) {
		DPRINTF("sio_oss_read: eof\n");
		hdl->sio.eof = 1;
		return 0;
	}

	hdl->idelta += n;
	return n;
}

static size_t
sio_oss_write(struct sio_hdl *sh, const void *buf, size_t len)
{
	struct sio_oss_hdl *hdl = (struct sio_oss_hdl *)sh;
	const unsigned char *data = buf;
	ssize_t n, todo;

	todo = len;
	while ((n = write(hdl->fd, data, todo)) < 0) {
		if (errno == EINTR)
			continue;
		if (errno != EAGAIN) {
			DPERROR("sio_oss_write: write");
			hdl->sio.eof = 1;
		}
		return 0;
	}

	hdl->odelta += n;
	return n;
}

static int
sio_oss_nfds(struct sio_hdl *hdl)
{
	return 1;
}

static int
sio_oss_pollfd(struct sio_hdl *sh, struct pollfd *pfd, int events)
{
	struct sio_oss_hdl *hdl = (struct sio_oss_hdl *)sh;
	int trig;

	pfd->fd = hdl->fd;
	pfd->events = events;
	if (hdl->filling && hdl->sio.wused == hdl->sio.par.bufsz *
		hdl->sio.par.pchan * hdl->sio.par.bps) {
		hdl->filling = 0;
		trig = 0;
		if (hdl->sio.mode & SIO_PLAY)
			trig |= PCM_ENABLE_OUTPUT;
		if (hdl->sio.mode & SIO_REC)
			trig |= PCM_ENABLE_INPUT;
		if (ioctl(hdl->fd, SNDCTL_DSP_SETTRIGGER, &trig) < 0) {
			DPERROR("sio_oss_pollfd: SETTRIGGER");
			hdl->sio.eof = 1;
			return 0;
		}
		_sio_onmove_cb(&hdl->sio, 0);
	}
	return 1;
}

static int
sio_oss_xrun(struct sio_oss_hdl *hdl)
{
	int clk;
	int wsil, rdrop, cmove;
	int rbpf, rround;
	int wbpf;

	DPRINTFN(2, "sio_oss_xrun:\n");
	if (_sndio_debug >= 2)
		_sio_printpos(&hdl->sio);

	/*
	 * we assume rused/wused are zero if rec/play modes are not
	 * selected. This allows us to keep the same formula for all
	 * modes, provided we set rbpf/wbpf to 1 to avoid division by
	 * zero.
	 *
	 * to understand the formula, draw a picture :)
	 */
	rbpf = (hdl->sio.mode & SIO_REC) ?
	    hdl->sio.par.bps * hdl->sio.par.rchan : 1;
	wbpf = (hdl->sio.mode & SIO_PLAY) ?
	    hdl->sio.par.bps * hdl->sio.par.pchan : 1;
	rround = hdl->sio.par.round * rbpf;

	clk = hdl->sio.cpos % hdl->sio.par.round;
	rdrop = (clk * rbpf - hdl->sio.rused) % rround;
	if (rdrop < 0)
		rdrop += rround;
	cmove = (rdrop + hdl->sio.rused) / rbpf;
	wsil = cmove * wbpf + hdl->sio.wused;

	DPRINTFN(2, "wsil = %d, cmove = %d, rdrop = %d\n", wsil, cmove, rdrop);

	if (!sio_oss_stop(&hdl->sio))
		return 0;
	if (!sio_oss_start(&hdl->sio))
		return 0;
	if (hdl->sio.mode & SIO_PLAY) {
		hdl->odelta -= cmove * hdl->bpf;
		hdl->sio.wsil = wsil;
	}
	if (hdl->sio.mode & SIO_REC) {
		hdl->idelta -= cmove * hdl->bpf;
		hdl->sio.rdrop = rdrop;
	}
	DPRINTFN(2, "xrun: corrected\n");
	DPRINTFN(2, "wsil = %d, rdrop = %d, odelta = %d, idelta = %d\n",
	    wsil, rdrop, hdl->odelta, hdl->idelta);
	return 1;
}

static int
sio_oss_revents(struct sio_hdl *sh, struct pollfd *pfd)
{
	struct sio_oss_hdl *hdl = (struct sio_oss_hdl *)sh;
	audio_errinfo ei;
	int delta, iused, oused;
	int revents = pfd->revents;
	oss_count_t optr, iptr;

	if ((pfd->revents & POLLHUP) ||
	    (pfd->revents & (POLLIN | POLLOUT)) == 0)
		return pfd->revents;

	/* Hide xruns from clients */
	if (ioctl(hdl->fd, SNDCTL_DSP_GETERROR, &ei) < 0) {
		DPERROR("sio_oss_revents: GETERROR");
		hdl->sio.eof = 1;
		return POLLHUP;
	}
	if (ei.play_underruns > 0 || ei.rec_overruns > 0) {
		if (!sio_oss_xrun(hdl))
			return POLLHUP;
		return 0;
	}

	if (hdl->sio.mode & SIO_PLAY) {
		if (ioctl(hdl->fd, SNDCTL_DSP_CURRENT_OPTR, &optr) < 0) {
			DPERROR("sio_oss_revents: CURRENT_OPTR");
			hdl->sio.eof = 1;
			return POLLHUP;
		}
		oused = optr.fifo_samples * hdl->bpf;
		hdl->odelta -= oused - hdl->oused;
		hdl->oused = oused;
		if (!(hdl->sio.mode & SIO_REC)) {
			hdl->idelta = hdl->odelta;
		}
	}
	if (hdl->sio.mode & SIO_REC) {
		if (ioctl(hdl->fd, SNDCTL_DSP_CURRENT_IPTR, &iptr) < 0) {
			DPERROR("sio_oss_revents: CURRENT_IPTR");
			hdl->sio.eof = 1;
			return POLLHUP;
		}
		iused = iptr.fifo_samples * hdl->bpf;
		hdl->idelta += iused - hdl->iused;
		hdl->iused = iused;
		if (!(hdl->sio.mode & SIO_PLAY)) {
			hdl->odelta = hdl->idelta;
		}
	}

	delta = (hdl->idelta > hdl->odelta) ? hdl->idelta : hdl->odelta;
	if (delta > 0) {
		_sio_onmove_cb(&hdl->sio, delta / hdl->bpf);
		hdl->idelta -= delta;
		hdl->odelta -= delta;
	}
	return revents;
}

static int
sio_oss_setvol(struct sio_hdl *sh, unsigned int vol)
{
	struct sio_oss_hdl *hdl = (struct sio_oss_hdl *)sh;
	int newvol;

	/* Scale to 0..100 */
	newvol = (100 * vol + SIO_MAXVOL / 2) / SIO_MAXVOL;
	newvol = newvol | (newvol << 8);

	if (ioctl(hdl->fd, SNDCTL_DSP_SETPLAYVOL, &newvol) < 0) {
		DPERROR("sio_oss_setvol");
		hdl->sio.eof = 1;
		return 0;
	}

	return 1;
}

static void
sio_oss_getvol(struct sio_hdl *sh)
{
	struct sio_oss_hdl *hdl = (struct sio_oss_hdl *)sh;
	int vol;

	if (ioctl(hdl->fd, SNDCTL_DSP_GETPLAYVOL, &vol) < 0) {
		DPERROR("sio_oss_getvol");
		hdl->sio.eof = 1;
		return;
	}

	/* Use left channel volume and scale to SIO_MAXVOL */
	vol = (SIO_MAXVOL * (vol & 0x7f) + 50) / 100;
	_sio_onvol_cb(&hdl->sio, vol);
}

#endif /* defined USE_OSS */
