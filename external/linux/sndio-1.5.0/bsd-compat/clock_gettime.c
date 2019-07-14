/*
 * Copyright (c) 2017 Alexandre Ratchov <alex@coua.org>
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
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <stddef.h>
#include "bsd-compat.h"

#ifndef HAVE_CLOCK_GETTIME

int
_sndio_clock_gettime(int timer, struct timespec *ts)
{
	struct timeval tv;

	if (timer != CLOCK_MONOTONIC) {
		errno = EINVAL;
		return -1;
	}

	if (gettimeofday(&tv, NULL) < 0)
		return -1;

	ts->tv_sec = tv.tv_sec;
	ts->tv_nsec = tv.tv_usec * 1000;
	return 0;
}

#endif
