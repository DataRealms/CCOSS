/*
 *  Mixer Simple Interface - local header file
 *  Copyright (c) 2005 by Jaroslav Kysela <perex@perex.cz>
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

#include "mixer_abst.h"

/* make local functions really local */
#define snd_mixer_simple_none_register \
	snd1_mixer_simple_none_register
#define snd_mixer_simple_basic_register \
	snd1_mixer_simple_basic_register

int snd_mixer_simple_none_register(snd_mixer_t *mixer, struct snd_mixer_selem_regopt *options, snd_mixer_class_t **classp);

#ifdef HAVE_LIBDL
int snd_mixer_simple_basic_register(snd_mixer_t *mixer, struct snd_mixer_selem_regopt *options, snd_mixer_class_t **classp);
#else
static inline int snd_mixer_simple_basic_register(snd_mixer_t *mixer ATTRIBUTE_UNUSED,
						  struct snd_mixer_selem_regopt *options ATTRIBUTE_UNUSED,
						  snd_mixer_class_t **classp ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}
#endif
