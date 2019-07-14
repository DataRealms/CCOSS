/*	$OpenBSD$	*/
/*
 * Copyright (c) 2008-2011 Alexandre Ratchov <alex@caoua.org>
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
#ifdef USE_ALSA
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <alsa/asoundlib.h>

#include "debug.h"
#include "mio_priv.h"

#define DEVNAME_PREFIX "hw:"

#ifdef DEBUG
static snd_output_t *output = NULL;
#define DALSA(str, err) fprintf(stderr, "%s: %s\n", str, snd_strerror(err))
#else
#define DALSA(str, err) do {} while (0)
#endif

struct mio_alsa_hdl {
	struct mio_hdl mio;
	char *devname;
	snd_rawmidi_t *in, *out;
	int infds, onfds, nfds, events;
};

static void mio_alsa_close(struct mio_hdl *);
static size_t mio_alsa_read(struct mio_hdl *, void *, size_t);
static size_t mio_alsa_write(struct mio_hdl *, const void *, size_t);
static int mio_alsa_nfds(struct mio_hdl *);
static int mio_alsa_pollfd(struct mio_hdl *, struct pollfd *, int);
static int mio_alsa_revents(struct mio_hdl *, struct pollfd *);

static struct mio_ops mio_alsa_ops = {
	mio_alsa_close,
	mio_alsa_write,
	mio_alsa_read,
	mio_alsa_nfds,
	mio_alsa_pollfd,
	mio_alsa_revents
};

struct mio_hdl *
_mio_alsa_open(const char *_str, unsigned int mode, int nbio)
{
	const char *p;
	struct mio_alsa_hdl *hdl;
	size_t len;
	int rc;

	p = _sndio_parsetype(_str, "rmidi");
	if (p == NULL) {
		DPRINTF("_mio_alsa_open: %s: \"rsnd\" expected\n", _str);
		return NULL;
	}
	switch (*p) {
	case '/':
		p++;
		break;
	default:
		DPRINTF("_mio_alsa_open: %s: '/' expected\n", _str);
		return NULL;
	}
	hdl = malloc(sizeof(struct mio_alsa_hdl));
	if (hdl == NULL)
		return NULL;
	_mio_create(&hdl->mio, &mio_alsa_ops, mode, nbio);
#ifdef DEBUG
	rc = snd_output_stdio_attach(&output, stderr, 0);
	if (rc < 0)
		DALSA("couldn't attach to stderr", rc);
#endif
	len = strlen(p);
	hdl->devname = malloc(len + sizeof(DEVNAME_PREFIX));
	if (hdl->devname == NULL) {
		free(hdl);
		return NULL;
	}
	memcpy(hdl->devname, DEVNAME_PREFIX, sizeof(DEVNAME_PREFIX) - 1);
	memcpy(hdl->devname + sizeof(DEVNAME_PREFIX) - 1, p, len + 1);
	hdl->in = hdl->out = NULL;
	rc = snd_rawmidi_open(&hdl->in, &hdl->out,
	    hdl->devname, SND_RAWMIDI_NONBLOCK);
	if (rc) {
		DALSA("could't open port", rc);
		free(hdl->devname);
		free(hdl);
		return NULL;
	}
	hdl->nfds = 0;
	if (hdl->in)
		hdl->nfds += snd_rawmidi_poll_descriptors_count(hdl->in);
	if (hdl->out)
		hdl->nfds += snd_rawmidi_poll_descriptors_count(hdl->out);
	return (struct mio_hdl *)hdl;
}

static void
mio_alsa_close(struct mio_hdl *sh)
{
	struct mio_alsa_hdl *hdl = (struct mio_alsa_hdl *)sh;

	if (hdl->in)
		snd_rawmidi_close(hdl->in);
	if (hdl->out) {
		snd_rawmidi_drain(hdl->out);
		snd_rawmidi_close(hdl->out);
	}
	free(hdl->devname);
	free(hdl);
}

static size_t
mio_alsa_read(struct mio_hdl *sh, void *buf, size_t len)
{
	struct mio_alsa_hdl *hdl = (struct mio_alsa_hdl *)sh;
	ssize_t n;

	for (;;) {
		n = snd_rawmidi_read(hdl->in, buf, len);
		if (n > 0)
			return n;
		if (n == -EINTR)
			continue;
		if (n == -EAGAIN)
			return 0;
		if (n == 0)
			DPRINTF("mio_alsa_read: eof\n");
		if (n < 0)
			DALSA("mio_alsa_read", n);
		hdl->mio.eof = 1;
		return 0;
	}
}

static size_t
mio_alsa_write(struct mio_hdl *sh, const void *buf, size_t len)
{
	struct mio_alsa_hdl *hdl = (struct mio_alsa_hdl *)sh;
	ssize_t n;

	for (;;) {
		n = snd_rawmidi_write(hdl->out, buf, len);
		if (n > 0)
			return n;
		if (n == -EINTR)
			continue;
		if (n == -EAGAIN)
			return 0;
		if (n == 0)
			DPRINTF("mio_alsa_write: eof\n");
		if (n < 0)
			DALSA("mio_alsa_write", n);
		hdl->mio.eof = 1;
		return 0;
	}
}

static int
mio_alsa_nfds(struct mio_hdl *sh)
{
	struct mio_alsa_hdl *hdl = (struct mio_alsa_hdl *)sh;

	return hdl->nfds;
}

static int
mio_alsa_pollfd(struct mio_hdl *sh, struct pollfd *pfd, int events)
{
	struct mio_alsa_hdl *hdl = (struct mio_alsa_hdl *)sh;

	if (!hdl->in)
		events &= ~POLLIN;
	if (!hdl->out)
		events &= ~POLLOUT;
	hdl->events = events;
	if (events & POLLIN) {
		hdl->infds = snd_rawmidi_poll_descriptors(hdl->in,
		    pfd, hdl->nfds);
	} else
		hdl->infds = 0;
	if (events & POLLOUT) {
		hdl->onfds += snd_rawmidi_poll_descriptors(hdl->out,
		    pfd + hdl->infds, hdl->nfds - hdl->infds);
	} else
		hdl->onfds = 0;
	return hdl->infds + hdl->onfds;
}

static int
mio_alsa_revents(struct mio_hdl *sh, struct pollfd *pfd)
{
	struct mio_alsa_hdl *hdl = (struct mio_alsa_hdl *)sh;
	unsigned short r;
	int revents = 0, rc;

	if (hdl->events & POLLIN) {
		rc = snd_rawmidi_poll_descriptors_revents(hdl->in,
		    pfd, hdl->infds, &r);
		if (rc < 0) {
			DALSA("snd_rawmidi_poll_descriptors_revents", rc);
			hdl->mio.eof = 1;
			return POLLHUP;
		}
		revents |= r & (POLLIN | POLLHUP);
	}
	if (hdl->events & POLLOUT) {
		rc = snd_rawmidi_poll_descriptors_revents(hdl->in,
		    pfd + hdl->infds, hdl->onfds, &r);
		if (rc < 0) {
			DALSA("snd_rawmidi_poll_descriptors_revents", rc);
			hdl->mio.eof = 1;
			return POLLHUP;
		}
		revents |= r & (POLLOUT | POLLHUP);
	}
	return revents;
}
#endif /* defined USE_ALSA */
