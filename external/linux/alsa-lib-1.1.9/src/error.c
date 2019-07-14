/**
 * \file error.c
 * \brief Error code handling routines
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 1998-2001
 *
 * Error code handling routines.
 */
/*
 *  Copyright (c) 1998 by Jaroslav Kysela <perex@perex.cz>
 *
 *  snd_strerror routine needs to be recoded for the locale support
 *
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "local.h"

/**
 * Array of error codes in US ASCII.
 */
static const char *snd_error_codes[] =
{
	"Sound protocol is not compatible"
};

/**
 * \brief Returns the message for an error code.
 * \param errnum The error code number, which must be a system error code
 *               or an ALSA error code.
 * \return The ASCII description of the given numeric error code.
 */
const char *snd_strerror(int errnum)
{
	if (errnum < 0)
		errnum = -errnum;
	if (errnum < SND_ERROR_BEGIN)
		return (const char *) strerror(errnum);
	errnum -= SND_ERROR_BEGIN;
	if ((unsigned int) errnum >= sizeof(snd_error_codes) / sizeof(const char *))
		 return "Unknown error";
	return snd_error_codes[errnum];
}

#ifndef DOC_HIDDEN
#ifdef HAVE___THREAD
#define TLS_PFX		__thread
#else
#define TLS_PFX		/* NOP */
#endif
#endif

static TLS_PFX snd_local_error_handler_t local_error = NULL;

/**
 * \brief Install local error handler
 * \param func The local error handler function
 * \retval Previous local error handler function
 */
snd_local_error_handler_t snd_lib_error_set_local(snd_local_error_handler_t func)
{
	snd_local_error_handler_t old = local_error;
	local_error = func;
	return old;
}

/**
 * \brief The default error handler function.
 * \param file The filename where the error was hit.
 * \param line The line number.
 * \param function The function name.
 * \param err The error code.
 * \param fmt The message (including the format characters).
 * \param ... Optional arguments.
 *
 * If a local error function has been installed for the current thread by
 * \ref snd_lib_error_set_local, it is called. Otherwise, prints the error
 * message including location to \c stderr.
 */
static void snd_lib_error_default(const char *file, int line, const char *function, int err, const char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	if (local_error) {
		local_error(file, line, function, err, fmt, arg);
		va_end(arg);
		return;
	}
	fprintf(stderr, "ALSA lib %s:%i:(%s) ", file, line, function);
	vfprintf(stderr, fmt, arg);
	if (err)
		fprintf(stderr, ": %s", snd_strerror(err));
	putc('\n', stderr);
	va_end(arg);
}

/**
 * \ingroup Error
 * Pointer to the error handler function.
 * For internal use only.
 */
snd_lib_error_handler_t snd_lib_error = snd_lib_error_default;

/**
 * \brief Sets the error handler.
 * \param handler The pointer to the new error handler function.
 *
 * This function sets a new error handler, or (if \c handler is \c NULL)
 * the default one which prints the error messages to \c stderr.
 */
int snd_lib_error_set_handler(snd_lib_error_handler_t handler)
{
	snd_lib_error = handler == NULL ? snd_lib_error_default : handler;
#ifndef NDEBUG
	if (snd_lib_error != snd_lib_error_default)
		snd_err_msg = snd_lib_error;
#endif
	return 0;
}

/**
 * \brief Returns the ALSA sound library version in ASCII format
 * \return The ASCII description of the used ALSA sound library.
 */
const char *snd_asoundlib_version(void)
{
	return SND_LIB_VERSION_STR;
}

#ifndef NDEBUG
/*
 * internal error handling
 */
static void snd_err_msg_default(const char *file, int line, const char *function, int err, const char *fmt, ...)
{
	va_list arg;
	const char *verbose;
	
	verbose = getenv("LIBASOUND_DEBUG");
	if (! verbose || ! *verbose)
		return;
	va_start(arg, fmt);
	fprintf(stderr, "ALSA lib %s:%i:(%s) ", file, line, function);
	vfprintf(stderr, fmt, arg);
	if (err)
		fprintf(stderr, ": %s", snd_strerror(err));
	putc('\n', stderr);
	va_end(arg);
#ifdef ALSA_DEBUG_ASSERT
	verbose = getenv("LIBASOUND_DEBUG_ASSERT");
	if (verbose && *verbose)
		assert(0);
#endif
}

/**
 * The ALSA error message handler
 */
snd_lib_error_handler_t snd_err_msg = snd_err_msg_default;

#endif

/**
 * \brief Copy a C-string into a sized buffer
 * \param dst Where to copy the string to
 * \param src Where to copy the string from
 * \param size Size of destination buffer
 * \retval The source string length
 *
 * The result is always a valid NUL-terminated string that fits
 * in the buffer (unless, of course, the buffer size is zero).
 * It does not pad out the result like strncpy() does.
 */
size_t snd_strlcpy(char *dst, const char *src, size_t size)
{
  size_t ret = strlen(src);
  if (size) {
    size_t len = ret >= size ? size - 1 : ret;
    memcpy(dst, src, len);
    dst[len] = '\0';
  }
  return ret;
}
