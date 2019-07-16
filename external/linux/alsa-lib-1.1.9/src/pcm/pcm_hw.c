/**
 * \file pcm/pcm_hw.c
 * \ingroup PCM_Plugins
 * \brief PCM HW Plugin Interface
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 2000-2001
 */
/*
 *  PCM - Hardware
 *  Copyright (c) 2000 by Abramo Bagnara <abramo@alsa-project.org>
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
#include <stddef.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "pcm_local.h"
#include "../control/control_local.h"
#include "../timer/timer_local.h"

//#define DEBUG_RW		/* use to debug readi/writei/readn/writen */
//#define DEBUG_MMAP		/* debug mmap_commit */

#ifndef PIC
/* entry for static linking */
const char *_snd_module_pcm_hw = "";
#endif

#ifndef DOC_HIDDEN

#ifndef F_SETSIG
#define F_SETSIG 10
#endif

/*
 *  Compatibility
 */

struct sndrv_pcm_hw_params_old {
	unsigned int flags;
	unsigned int masks[SNDRV_PCM_HW_PARAM_SUBFORMAT -
			   SNDRV_PCM_HW_PARAM_ACCESS + 1];
	struct snd_interval intervals[SNDRV_PCM_HW_PARAM_TICK_TIME -
					SNDRV_PCM_HW_PARAM_SAMPLE_BITS + 1];
	unsigned int rmask;
	unsigned int cmask;
	unsigned int info;
	unsigned int msbits;
	unsigned int rate_num;
	unsigned int rate_den;
	sndrv_pcm_uframes_t fifo_size;
	unsigned char reserved[64];
};

#define SND_PCM_IOCTL_HW_REFINE_OLD _IOWR('A', 0x10, struct sndrv_pcm_hw_params_old)
#define SND_PCM_IOCTL_HW_PARAMS_OLD _IOWR('A', 0x11, struct sndrv_pcm_hw_params_old)

static int use_old_hw_params_ioctl(int fd, unsigned int cmd, snd_pcm_hw_params_t *params);
static snd_pcm_sframes_t snd_pcm_hw_avail_update(snd_pcm_t *pcm);
static const snd_pcm_fast_ops_t snd_pcm_hw_fast_ops;
static const snd_pcm_fast_ops_t snd_pcm_hw_fast_ops_timer;

/*
 *
 */

typedef struct {
	int version;
	int fd;
	int card, device, subdevice;

	volatile struct snd_pcm_mmap_status * mmap_status;
	struct snd_pcm_mmap_control *mmap_control;
	bool mmap_status_fallbacked;
	bool mmap_control_fallbacked;
	struct snd_pcm_sync_ptr *sync_ptr;

	int period_event;
	snd_timer_t *period_timer;
	struct pollfd period_timer_pfd;
	int period_timer_need_poll;
	/* restricted parameters */
	snd_pcm_format_t format;
	int rate;
	int channels;
	/* for chmap */
	unsigned int chmap_caps;
	snd_pcm_chmap_query_t **chmap_override;
} snd_pcm_hw_t;

#define SNDRV_FILE_PCM_STREAM_PLAYBACK		ALSA_DEVICE_DIRECTORY "pcmC%iD%ip"
#define SNDRV_FILE_PCM_STREAM_CAPTURE		ALSA_DEVICE_DIRECTORY "pcmC%iD%ic"
#define SNDRV_PCM_VERSION_MAX			SNDRV_PROTOCOL_VERSION(2, 0, 9)

/* update appl_ptr with driver */
#define FAST_PCM_STATE(hw) \
	((snd_pcm_state_t) (hw)->mmap_status->state)
#define FAST_PCM_TSTAMP(hw) \
	((hw)->mmap_status->tstamp)

struct timespec snd_pcm_hw_fast_tstamp(snd_pcm_t *pcm)
{
	struct timespec res;
	snd_pcm_hw_t *hw = pcm->private_data;
	res = FAST_PCM_TSTAMP(hw);
	if (SNDRV_PROTOCOL_VERSION(2, 0, 5) > hw->version)
		res.tv_nsec *= 1000L;
	return res;
}
#endif /* DOC_HIDDEN */

static int sync_ptr1(snd_pcm_hw_t *hw, unsigned int flags)
{
	int err;
	hw->sync_ptr->flags = flags;
	if (ioctl(hw->fd, SNDRV_PCM_IOCTL_SYNC_PTR, hw->sync_ptr) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_SYNC_PTR failed (%i)", err);
		return err;
	}
	return 0;
}

static int issue_avail_min(snd_pcm_hw_t *hw)
{
	if (!hw->mmap_control_fallbacked)
		return 0;

	/* Avoid unexpected change of applptr in kernel space. */
	return sync_ptr1(hw, SNDRV_PCM_SYNC_PTR_APPL);
}

static int issue_applptr(snd_pcm_hw_t *hw)
{
	if (!hw->mmap_control_fallbacked)
		return 0;

	/* Avoid unexpected change of avail_min in kernel space. */
	return sync_ptr1(hw, SNDRV_PCM_SYNC_PTR_AVAIL_MIN);
}

static int request_hwsync(snd_pcm_hw_t *hw)
{
	if (!hw->mmap_status_fallbacked)
		return 0;

	/*
	 * Query both of control/status data to avoid unexpected change of
	 * control data in kernel space.
	 */
	return sync_ptr1(hw,
			 SNDRV_PCM_SYNC_PTR_HWSYNC |
			 SNDRV_PCM_SYNC_PTR_APPL |
			 SNDRV_PCM_SYNC_PTR_AVAIL_MIN);
}

static int query_status_and_control_data(snd_pcm_hw_t *hw)
{
	if (!hw->mmap_control_fallbacked)
		return 0;

	/*
	 * Query both of control/status data to avoid unexpected change of
	 * control data in kernel space.
	 */
	return sync_ptr1(hw,
			 SNDRV_PCM_SYNC_PTR_APPL |
			 SNDRV_PCM_SYNC_PTR_AVAIL_MIN);
}

static int query_status_data(snd_pcm_hw_t *hw)
{
	if (!hw->mmap_status_fallbacked)
		return 0;

	/*
	 * Query both of control/status data to avoid unexpected change of
	 * control data in kernel space.
	 */
	return sync_ptr1(hw,
			 SNDRV_PCM_SYNC_PTR_APPL |
			 SNDRV_PCM_SYNC_PTR_AVAIL_MIN);
}

static int snd_pcm_hw_clear_timer_queue(snd_pcm_hw_t *hw)
{
	if (hw->period_timer_need_poll) {
		while (poll(&hw->period_timer_pfd, 1, 0) > 0) {
			snd_timer_tread_t rbuf[4];
			snd_timer_read(hw->period_timer, rbuf, sizeof(rbuf));
		}
	} else {
		snd_timer_tread_t rbuf[4];
		snd_timer_read(hw->period_timer, rbuf, sizeof(rbuf));
	}
	return 0;
}

static int snd_pcm_hw_poll_descriptors_count(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 2;
}

static int snd_pcm_hw_poll_descriptors(snd_pcm_t *pcm, struct pollfd *pfds, unsigned int space)
{
	snd_pcm_hw_t *hw = pcm->private_data;

	if (space < 2)
		return -ENOMEM;
	pfds[0].fd = hw->fd;
	pfds[0].events = pcm->poll_events | POLLERR | POLLNVAL;
	pfds[1].fd = hw->period_timer_pfd.fd;
	pfds[1].events = POLLIN | POLLERR | POLLNVAL;
	return 2;
}

static int snd_pcm_hw_poll_revents(snd_pcm_t *pcm, struct pollfd *pfds, unsigned nfds, unsigned short *revents)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	unsigned int events;

	if (nfds != 2 || pfds[0].fd != hw->fd || pfds[1].fd != hw->period_timer_pfd.fd)
		return -EINVAL;
	events = pfds[0].revents;
	if (pfds[1].revents & POLLIN) {
		snd_pcm_hw_clear_timer_queue(hw);
		events |= pcm->poll_events & ~(POLLERR|POLLNVAL);
	}
	*revents = events;
	return 0;
}

static int snd_pcm_hw_nonblock(snd_pcm_t *pcm, int nonblock)
{
	long flags;
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;

	if ((flags = fcntl(fd, F_GETFL)) < 0) {
		err = -errno;
		SYSMSG("F_GETFL failed (%i)", err);
		return err;
	}
	if (nonblock)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) {
		err = -errno;
		SYSMSG("F_SETFL for O_NONBLOCK failed (%i)", err);
		return err;
	}
	return 0;
}

static int snd_pcm_hw_async(snd_pcm_t *pcm, int sig, pid_t pid)
{
	long flags;
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;

	if ((flags = fcntl(fd, F_GETFL)) < 0) {
		err = -errno;
		SYSMSG("F_GETFL failed (%i)", err);
		return err;
	}
	if (sig >= 0)
		flags |= O_ASYNC;
	else
		flags &= ~O_ASYNC;
	if (fcntl(fd, F_SETFL, flags) < 0) {
		err = -errno;
		SYSMSG("F_SETFL for O_ASYNC failed (%i)", err);
		return err;
	}
	if (sig < 0)
		return 0;
	if (fcntl(fd, F_SETSIG, (long)sig) < 0) {
		err = -errno;
		SYSMSG("F_SETSIG failed (%i)", err);
		return err;
	}
	if (fcntl(fd, F_SETOWN, (long)pid) < 0) {
		err = -errno;
		SYSMSG("F_SETOWN failed (%i)", err);
		return err;
	}
	return 0;
}

static int snd_pcm_hw_info(snd_pcm_t *pcm, snd_pcm_info_t * info)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;
	if (ioctl(fd, SNDRV_PCM_IOCTL_INFO, info) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_INFO failed (%i)", err);
		return err;
	}
	return 0;
}

static inline int hw_refine_call(snd_pcm_hw_t *pcm_hw, snd_pcm_hw_params_t *params)
{
	/* check for new hw_params structure; it's available from 2.0.2 version of PCM API */
	if (SNDRV_PROTOCOL_VERSION(2, 0, 2) <= pcm_hw->version)
		return ioctl(pcm_hw->fd, SNDRV_PCM_IOCTL_HW_REFINE, params);
	return use_old_hw_params_ioctl(pcm_hw->fd, SND_PCM_IOCTL_HW_REFINE_OLD, params);
}

static int snd_pcm_hw_hw_refine(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err;

	if (hw->format != SND_PCM_FORMAT_UNKNOWN) {
		err = _snd_pcm_hw_params_set_format(params, hw->format);
		if (err < 0)
			return err;
	}
	if (hw->channels > 0) {
		err = _snd_pcm_hw_param_set(params, SND_PCM_HW_PARAM_CHANNELS,
					    hw->channels, 0);
		if (err < 0)
			return err;
	}
	if (hw->rate > 0) {
		err = _snd_pcm_hw_param_set_minmax(params, SND_PCM_HW_PARAM_RATE,
						   hw->rate, 0, hw->rate + 1, -1);
		if (err < 0)
			return err;
	}

	if (hw_refine_call(hw, params) < 0) {
		err = -errno;
		// SYSMSG("SNDRV_PCM_IOCTL_HW_REFINE failed");
		return err;
	}

	if (params->info != ~0U) {
		params->info &= ~0xf0000000;
		if (pcm->tstamp_type != SND_PCM_TSTAMP_TYPE_GETTIMEOFDAY)
			params->info |= SND_PCM_INFO_MONOTONIC;
	}
	
	return 0;
}

static inline int hw_params_call(snd_pcm_hw_t *pcm_hw, snd_pcm_hw_params_t *params)
{
	/* check for new hw_params structure; it's available from 2.0.2 version of PCM API */
	if (SNDRV_PROTOCOL_VERSION(2, 0, 2) <= pcm_hw->version)
		return ioctl(pcm_hw->fd, SNDRV_PCM_IOCTL_HW_PARAMS, params);
	return use_old_hw_params_ioctl(pcm_hw->fd, SND_PCM_IOCTL_HW_PARAMS_OLD, params);
}

static int snd_pcm_hw_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t * params)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err;
	if (hw_params_call(hw, params) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_HW_PARAMS failed (%i)", err);
		return err;
	}
	params->info &= ~0xf0000000;
	if (pcm->tstamp_type != SND_PCM_TSTAMP_TYPE_GETTIMEOFDAY)
		params->info |= SND_PCM_INFO_MONOTONIC;
	return query_status_data(hw);
}

static void snd_pcm_hw_close_timer(snd_pcm_hw_t *hw)
{
	if (hw->period_timer) {
		snd_timer_close(hw->period_timer);
		hw->period_timer = NULL;
	}
}

static int snd_pcm_hw_change_timer(snd_pcm_t *pcm, int enable)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	snd_timer_params_t params = {0};
	unsigned int suspend, resume;
	int err;
	
	if (enable) {
		err = snd_timer_hw_open(&hw->period_timer,
				"hw-pcm-period-event",
				SND_TIMER_CLASS_PCM, SND_TIMER_SCLASS_NONE,
				hw->card, hw->device,
				(hw->subdevice << 1) | (pcm->stream & 1),
				SND_TIMER_OPEN_NONBLOCK | SND_TIMER_OPEN_TREAD);
		if (err < 0) {
			err = snd_timer_hw_open(&hw->period_timer,
				"hw-pcm-period-event",
				SND_TIMER_CLASS_PCM, SND_TIMER_SCLASS_NONE,
				hw->card, hw->device,
				(hw->subdevice << 1) | (pcm->stream & 1),
				SND_TIMER_OPEN_NONBLOCK);
			return err;
		}
		if (snd_timer_poll_descriptors_count(hw->period_timer) != 1) {
			snd_pcm_hw_close_timer(hw);
			return -EINVAL;
		}
		hw->period_timer_pfd.events = POLLIN;
 		hw->period_timer_pfd.revents = 0;
		snd_timer_poll_descriptors(hw->period_timer,
					   &hw->period_timer_pfd, 1);
		hw->period_timer_need_poll = 0;
		suspend = 1<<SND_TIMER_EVENT_MSUSPEND;
		resume = 1<<SND_TIMER_EVENT_MRESUME;
		/*
		 * hacks for older kernel drivers
		 */
		{
			int ver = 0;
			ioctl(hw->period_timer_pfd.fd,
			      SNDRV_TIMER_IOCTL_PVERSION, &ver);
			/*
			 * In older versions, check via poll before read() is
			 * needed because of the confliction between
			 * TIMER_START and FIONBIO ioctls.
                         */
			if (ver < SNDRV_PROTOCOL_VERSION(2, 0, 4))
				hw->period_timer_need_poll = 1;
			/*
			 * In older versions, timer uses pause events instead
			 * suspend/resume events.
			 */
			if (ver < SNDRV_PROTOCOL_VERSION(2, 0, 5)) {
				suspend = 1<<SND_TIMER_EVENT_MPAUSE;
				resume = 1<<SND_TIMER_EVENT_MCONTINUE;
			}
		}
		snd_timer_params_set_auto_start(&params, 1);
		snd_timer_params_set_ticks(&params, 1);
		INTERNAL(snd_timer_params_set_filter)(&params, (1<<SND_TIMER_EVENT_TICK) |
					    suspend | resume);
		err = snd_timer_params(hw->period_timer, &params);
		if (err < 0) {
			snd_pcm_hw_close_timer(hw);
			return err;
		}
		err = snd_timer_start(hw->period_timer);
		if (err < 0) {
			snd_pcm_hw_close_timer(hw);
			return err;
		}
		pcm->fast_ops = &snd_pcm_hw_fast_ops_timer;
	} else {
		snd_pcm_hw_close_timer(hw);
		pcm->fast_ops = &snd_pcm_hw_fast_ops;
		hw->period_event = 0;
	}
	return 0;
}

static int snd_pcm_hw_hw_free(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;
	snd_pcm_hw_change_timer(pcm, 0);
	if (ioctl(fd, SNDRV_PCM_IOCTL_HW_FREE) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_HW_FREE failed (%i)", err);
		return err;
	}
	return 0;
}

static int snd_pcm_hw_sw_params(snd_pcm_t *pcm, snd_pcm_sw_params_t * params)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err = 0;
	int old_period_event = sw_get_period_event(params);
	sw_set_period_event(params, 0);
	if ((snd_pcm_tstamp_t) params->tstamp_mode == pcm->tstamp_mode &&
	    (snd_pcm_tstamp_type_t) params->tstamp_type == pcm->tstamp_type &&
	    params->period_step == pcm->period_step &&
	    params->start_threshold == pcm->start_threshold &&
	    params->stop_threshold == pcm->stop_threshold &&
	    params->silence_threshold == pcm->silence_threshold &&
	    params->silence_size == pcm->silence_size &&
	    old_period_event == hw->period_event) {
		hw->mmap_control->avail_min = params->avail_min;
		err = issue_avail_min(hw);
		goto out;
	}
	if (params->tstamp_type == SND_PCM_TSTAMP_TYPE_MONOTONIC_RAW &&
	    hw->version < SNDRV_PROTOCOL_VERSION(2, 0, 12)) {
		SYSMSG("Kernel doesn't support SND_PCM_TSTAMP_TYPE_MONOTONIC_RAW");
		err = -EINVAL;
		goto out;
	}
	if (params->tstamp_type == SND_PCM_TSTAMP_TYPE_MONOTONIC &&
	    hw->version < SNDRV_PROTOCOL_VERSION(2, 0, 5)) {
		SYSMSG("Kernel doesn't support SND_PCM_TSTAMP_TYPE_MONOTONIC");
		err = -EINVAL;
		goto out;
	}
	if (ioctl(fd, SNDRV_PCM_IOCTL_SW_PARAMS, params) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_SW_PARAMS failed (%i)", err);
		goto out;
	}
	if ((snd_pcm_tstamp_type_t) params->tstamp_type != pcm->tstamp_type) {
		if (hw->version < SNDRV_PROTOCOL_VERSION(2, 0, 12)) {
			int on = (snd_pcm_tstamp_type_t) params->tstamp_type ==
				SND_PCM_TSTAMP_TYPE_MONOTONIC;
			if (ioctl(fd, SNDRV_PCM_IOCTL_TSTAMP, &on) < 0) {
				err = -errno;
				SNDMSG("TSTAMP failed\n");
				goto out;
			}
		}
		pcm->tstamp_type = params->tstamp_type;
	}
	hw->mmap_control->avail_min = params->avail_min;
	if (hw->period_event != old_period_event) {
		err = snd_pcm_hw_change_timer(pcm, old_period_event);
		if (err < 0)
			goto out;
		hw->period_event = old_period_event;
	}
 out:
	sw_set_period_event(params, old_period_event);
	return err;
}

static int snd_pcm_hw_channel_info(snd_pcm_t *pcm, snd_pcm_channel_info_t * info)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	struct snd_pcm_channel_info i;
	int fd = hw->fd, err;
	i.channel = info->channel;
	if (ioctl(fd, SNDRV_PCM_IOCTL_CHANNEL_INFO, &i) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_CHANNEL_INFO failed (%i)", err);
		return err;
	}
	info->channel = i.channel;
	info->addr = 0;
	info->first = i.first;
	info->step = i.step;
	info->type = SND_PCM_AREA_MMAP;
	info->u.mmap.fd = fd;
	info->u.mmap.offset = i.offset;
	return 0;
}

static int snd_pcm_hw_status(snd_pcm_t *pcm, snd_pcm_status_t * status)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;
	if (SNDRV_PROTOCOL_VERSION(2, 0, 13) > hw->version) {
		if (ioctl(fd, SNDRV_PCM_IOCTL_STATUS, status) < 0) {
			err = -errno;
			SYSMSG("SNDRV_PCM_IOCTL_STATUS failed (%i)", err);
			return err;
		}
	} else {
		if (ioctl(fd, SNDRV_PCM_IOCTL_STATUS_EXT, status) < 0) {
			err = -errno;
			SYSMSG("SNDRV_PCM_IOCTL_STATUS_EXT failed (%i)", err);
			return err;
		}
	}
	if (SNDRV_PROTOCOL_VERSION(2, 0, 5) > hw->version) {
		status->tstamp.tv_nsec *= 1000L;
		status->trigger_tstamp.tv_nsec *= 1000L;
	}
	return 0;
}

static snd_pcm_state_t snd_pcm_hw_state(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err = query_status_data(hw);
	if (err < 0)
		return err;
	return (snd_pcm_state_t) hw->mmap_status->state;
}

static int snd_pcm_hw_delay(snd_pcm_t *pcm, snd_pcm_sframes_t *delayp)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;
	if (ioctl(fd, SNDRV_PCM_IOCTL_DELAY, delayp) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_DELAY failed (%i)", err);
		return err;
	}
	return 0;
}

static int snd_pcm_hw_hwsync(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;
	if (SNDRV_PROTOCOL_VERSION(2, 0, 3) <= hw->version) {
		if (hw->mmap_status_fallbacked) {
			err = request_hwsync(hw);
			if (err < 0)
				return err;
		} else {
			if (ioctl(fd, SNDRV_PCM_IOCTL_HWSYNC) < 0) {
				err = -errno;
				SYSMSG("SNDRV_PCM_IOCTL_HWSYNC failed (%i)", err);
				return err;
			}
		}
	} else {
		snd_pcm_sframes_t delay;
		int err = snd_pcm_hw_delay(pcm, &delay);
		if (err < 0) {
			switch (FAST_PCM_STATE(hw)) {
			case SND_PCM_STATE_PREPARED:
			case SND_PCM_STATE_SUSPENDED:
				return 0;
			default:
				return err;
			}
		}
	}
	return 0;
}

static int snd_pcm_hw_prepare(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;
	if (ioctl(fd, SNDRV_PCM_IOCTL_PREPARE) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_PREPARE failed (%i)", err);
		return err;
	}
	return query_status_and_control_data(hw);
}

static int snd_pcm_hw_reset(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;
	if (ioctl(fd, SNDRV_PCM_IOCTL_RESET) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_RESET failed (%i)", err);
		return err;
	}
	return query_status_and_control_data(hw);
}

static int snd_pcm_hw_start(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err;
#if 0
	assert(pcm->stream != SND_PCM_STREAM_PLAYBACK ||
	       snd_pcm_mmap_playback_hw_avail(pcm) > 0);
#endif
	issue_applptr(hw);
	if (ioctl(hw->fd, SNDRV_PCM_IOCTL_START) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_START failed (%i)", err);
#if 0
		if (err == -EBADFD)
			SNDERR("PCM state = %s", snd_pcm_state_name(snd_pcm_hw_state(pcm)));
#endif
		return err;
	}
	return 0;
}

static int snd_pcm_hw_drop(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err;
	if (ioctl(hw->fd, SNDRV_PCM_IOCTL_DROP) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_DROP failed (%i)", err);
		return err;
	} else {
	}
	return 0;
}

static int snd_pcm_hw_drain(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err;
	if (ioctl(hw->fd, SNDRV_PCM_IOCTL_DRAIN) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_DRAIN failed (%i)", err);
		return err;
	}
	return 0;
}

static int snd_pcm_hw_pause(snd_pcm_t *pcm, int enable)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err;
	if (ioctl(hw->fd, SNDRV_PCM_IOCTL_PAUSE, enable) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_PAUSE failed (%i)", err);
		return err;
	}
	return 0;
}

static snd_pcm_sframes_t snd_pcm_hw_rewindable(snd_pcm_t *pcm)
{
	return snd_pcm_mmap_hw_rewindable(pcm);
}

static snd_pcm_sframes_t snd_pcm_hw_rewind(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err;
	if (ioctl(hw->fd, SNDRV_PCM_IOCTL_REWIND, &frames) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_REWIND failed (%i)", err);
		return err;
	}
	err = query_status_and_control_data(hw);
	if (err < 0)
		return err;
	return frames;
}

static snd_pcm_sframes_t snd_pcm_hw_forwardable(snd_pcm_t *pcm)
{
	return snd_pcm_mmap_avail(pcm);
}

static snd_pcm_sframes_t snd_pcm_hw_forward(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err;
	if (SNDRV_PROTOCOL_VERSION(2, 0, 4) <= hw->version) {
		if (ioctl(hw->fd, SNDRV_PCM_IOCTL_FORWARD, &frames) < 0) {
			err = -errno;
			SYSMSG("SNDRV_PCM_IOCTL_FORWARD failed (%i)", err);
			return err;
		}
		err = query_status_and_control_data(hw);
		if (err < 0)
			return err;
		return frames;
	} else {
		snd_pcm_sframes_t avail;

		switch (FAST_PCM_STATE(hw)) {
		case SNDRV_PCM_STATE_RUNNING:
		case SNDRV_PCM_STATE_DRAINING:
		case SNDRV_PCM_STATE_PAUSED:
		case SNDRV_PCM_STATE_PREPARED:
			break;
		case SNDRV_PCM_STATE_XRUN:
			return -EPIPE;
		default:
			return -EBADFD;
		}
		avail = snd_pcm_mmap_avail(pcm);
		if (avail < 0)
			return 0;
		if (frames > (snd_pcm_uframes_t)avail)
			frames = avail;
		snd_pcm_mmap_appl_forward(pcm, frames);
		return frames;
	}
}

static int snd_pcm_hw_resume(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd, err;
	if (ioctl(fd, SNDRV_PCM_IOCTL_RESUME) < 0) {
		err = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_RESUME failed (%i)", err);
		return err;
	}
	return 0;
}

static int hw_link(snd_pcm_t *pcm1, snd_pcm_t *pcm2)
{
	snd_pcm_hw_t *hw1 = pcm1->private_data;
	snd_pcm_hw_t *hw2 = pcm2->private_data;
	if (ioctl(hw1->fd, SNDRV_PCM_IOCTL_LINK, hw2->fd) < 0) {
		SYSMSG("SNDRV_PCM_IOCTL_LINK failed (%i)", -errno);
		return -errno;
	}
	return 0;
}

static int snd_pcm_hw_link_slaves(snd_pcm_t *pcm, snd_pcm_t *master)
{
	if (master->type != SND_PCM_TYPE_HW) {
		SYSMSG("Invalid type for SNDRV_PCM_IOCTL_LINK (%i)", master->type);
		return -EINVAL;
	}
	return hw_link(master, pcm);
}

static int snd_pcm_hw_link(snd_pcm_t *pcm1, snd_pcm_t *pcm2)
{
	if (pcm2->type != SND_PCM_TYPE_HW) {
		if (pcm2->fast_ops->link_slaves)
			return pcm2->fast_ops->link_slaves(pcm2, pcm1);
		return -ENOSYS;
	}
	return hw_link(pcm1, pcm2);
 }

static int snd_pcm_hw_unlink(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;

	if (ioctl(hw->fd, SNDRV_PCM_IOCTL_UNLINK) < 0) {
		SYSMSG("SNDRV_PCM_IOCTL_UNLINK failed (%i)", -errno);
		return -errno;
	}
	return 0;
}

static snd_pcm_sframes_t snd_pcm_hw_writei(snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size)
{
	int err;
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd;
	struct snd_xferi xferi;
	xferi.buf = (char*) buffer;
	xferi.frames = size;
	xferi.result = 0; /* make valgrind happy */
	if (ioctl(fd, SNDRV_PCM_IOCTL_WRITEI_FRAMES, &xferi) < 0)
		err = -errno;
	else
		err = query_status_and_control_data(hw);
#ifdef DEBUG_RW
	fprintf(stderr, "hw_writei: frames = %li, xferi.result = %li, err = %i\n", size, xferi.result, err);
#endif
	if (err < 0)
		return snd_pcm_check_error(pcm, err);
	return xferi.result;
}

static snd_pcm_sframes_t snd_pcm_hw_writen(snd_pcm_t *pcm, void **bufs, snd_pcm_uframes_t size)
{
	int err;
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd;
	struct snd_xfern xfern;
	memset(&xfern, 0, sizeof(xfern)); /* make valgrind happy */
	xfern.bufs = bufs;
	xfern.frames = size;
	if (ioctl(fd, SNDRV_PCM_IOCTL_WRITEN_FRAMES, &xfern) < 0)
		err = -errno;
	else
		err = query_status_and_control_data(hw);
#ifdef DEBUG_RW
	fprintf(stderr, "hw_writen: frames = %li, result = %li, err = %i\n", size, xfern.result, err);
#endif
	if (err < 0)
		return snd_pcm_check_error(pcm, err);
	return xfern.result;
}

static snd_pcm_sframes_t snd_pcm_hw_readi(snd_pcm_t *pcm, void *buffer, snd_pcm_uframes_t size)
{
	int err;
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd;
	struct snd_xferi xferi;
	xferi.buf = buffer;
	xferi.frames = size;
	xferi.result = 0; /* make valgrind happy */
	if (ioctl(fd, SNDRV_PCM_IOCTL_READI_FRAMES, &xferi) < 0)
		err = -errno;
	else
		err = query_status_and_control_data(hw);
#ifdef DEBUG_RW
	fprintf(stderr, "hw_readi: frames = %li, result = %li, err = %i\n", size, xferi.result, err);
#endif
	if (err < 0)
		return snd_pcm_check_error(pcm, err);
	return xferi.result;
}

static snd_pcm_sframes_t snd_pcm_hw_readn(snd_pcm_t *pcm, void **bufs, snd_pcm_uframes_t size)
{
	int err;
	snd_pcm_hw_t *hw = pcm->private_data;
	int fd = hw->fd;
	struct snd_xfern xfern;
	memset(&xfern, 0, sizeof(xfern)); /* make valgrind happy */
	xfern.bufs = bufs;
	xfern.frames = size;
	if (ioctl(fd, SNDRV_PCM_IOCTL_READN_FRAMES, &xfern) < 0)
		err = -errno;
	else
		err = query_status_and_control_data(hw);
#ifdef DEBUG_RW
	fprintf(stderr, "hw_readn: frames = %li, result = %li, err = %i\n", size, xfern.result, err);
#endif
	if (err < 0)
		return snd_pcm_check_error(pcm, err);
	return xfern.result;
}

static bool map_status_data(snd_pcm_hw_t *hw, struct snd_pcm_sync_ptr *sync_ptr,
			    bool force_fallback)
{
	struct snd_pcm_mmap_status *mmap_status;
	bool fallbacked;

	mmap_status = MAP_FAILED;
	if (!force_fallback) {
		mmap_status = mmap(NULL, page_align(sizeof(*mmap_status)),
				   PROT_READ, MAP_FILE|MAP_SHARED,
				   hw->fd, SNDRV_PCM_MMAP_OFFSET_STATUS);
	}

	if (mmap_status == MAP_FAILED || mmap_status == NULL) {
		mmap_status = &sync_ptr->s.status;
		fallbacked = true;
	} else {
		fallbacked = false;
	}

	hw->mmap_status = mmap_status;

	return fallbacked;
}

static bool map_control_data(snd_pcm_hw_t *hw,
			     struct snd_pcm_sync_ptr *sync_ptr,
			     bool force_fallback)
{
	struct snd_pcm_mmap_control *mmap_control;
	bool fallbacked;

	mmap_control = MAP_FAILED;
	if (!force_fallback) {
		mmap_control = mmap(NULL, page_align(sizeof(*mmap_control)),
				    PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED,
				    hw->fd, SNDRV_PCM_MMAP_OFFSET_CONTROL);
	}

	if (mmap_control == MAP_FAILED || mmap_control == NULL) {
		mmap_control = &sync_ptr->c.control;
		fallbacked = true;
	} else {
		fallbacked = false;
	}

	hw->mmap_control = mmap_control;

	return fallbacked;
}

static int map_status_and_control_data(snd_pcm_t *pcm, bool force_fallback)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	struct snd_pcm_sync_ptr *sync_ptr;
	int err;

	/* Preparation for fallback to failure of mmap(2). */
	sync_ptr = malloc(sizeof(*sync_ptr));
	if (sync_ptr == NULL)
		return -ENOMEM;
	memset(sync_ptr, 0, sizeof(*sync_ptr));

	hw->mmap_status_fallbacked =
			map_status_data(hw, sync_ptr, force_fallback);
	hw->mmap_control_fallbacked =
			map_control_data(hw, sync_ptr, force_fallback);

	/* Any fallback mode needs to keep the buffer. */
	if (hw->mmap_status_fallbacked || hw->mmap_control_fallbacked) {
		hw->sync_ptr = sync_ptr;
	} else {
		free(sync_ptr);
		hw->sync_ptr = NULL;
	}

	/* do not initialize in case of append and keep the values from the
	 * kernel
	 */
	if (!(pcm->mode & SND_PCM_APPEND)) {
		/* Initialize the data. */
		hw->mmap_control->appl_ptr = 0;
		hw->mmap_control->avail_min = 1;
	}
	snd_pcm_set_hw_ptr(pcm, &hw->mmap_status->hw_ptr, hw->fd,
			   SNDRV_PCM_MMAP_OFFSET_STATUS +
				offsetof(struct snd_pcm_mmap_status, hw_ptr));
	snd_pcm_set_appl_ptr(pcm, &hw->mmap_control->appl_ptr, hw->fd,
			     SNDRV_PCM_MMAP_OFFSET_CONTROL);
	if (hw->mmap_control_fallbacked) {
		unsigned int flags = 0;
		/* read appl_ptr and avail_min from kernel when device opened
		 * with SND_PCM_APPEND flag
		 */
		if (pcm->mode & SND_PCM_APPEND)
			flags = SNDRV_PCM_SYNC_PTR_APPL |
				SNDRV_PCM_SYNC_PTR_AVAIL_MIN;
		err = sync_ptr1(hw, flags);
		if (err < 0)
			return err;
	}

	return 0;
}

static void unmap_status_data(snd_pcm_hw_t *hw)
{
	if (!hw->mmap_status_fallbacked) {
		if (munmap((void *)hw->mmap_status,
			   page_align(sizeof(*hw->mmap_status))) < 0)
			SYSMSG("status munmap failed (%u)", errno);
	}
}

static void unmap_control_data(snd_pcm_hw_t *hw)
{
	if (!hw->mmap_control_fallbacked) {
		if (munmap((void *)hw->mmap_control,
			   page_align(sizeof(*hw->mmap_control))) < 0)
			SYSMSG("control munmap failed (%u)", errno);
	}
}

static void unmap_status_and_control_data(snd_pcm_hw_t *hw)
{
	unmap_status_data(hw);
	unmap_control_data(hw);

	if (hw->mmap_status_fallbacked || hw->mmap_control_fallbacked)
		free(hw->sync_ptr);

	hw->mmap_status = NULL;
	hw->mmap_control = NULL;
	hw->mmap_status_fallbacked = false;
	hw->mmap_control_fallbacked = false;
	hw->sync_ptr = NULL;
}

static int snd_pcm_hw_mmap(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_pcm_hw_munmap(snd_pcm_t *pcm ATTRIBUTE_UNUSED)
{
	return 0;
}

static int snd_pcm_hw_close(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	int err = 0;
	if (close(hw->fd)) {
		err = -errno;
		SYSMSG("close failed (%i)\n", err);
	}

	unmap_status_and_control_data(hw);

	free(hw);
	return err;
}

static snd_pcm_sframes_t snd_pcm_hw_mmap_commit(snd_pcm_t *pcm,
						snd_pcm_uframes_t offset ATTRIBUTE_UNUSED,
						snd_pcm_uframes_t size)
{
	snd_pcm_hw_t *hw = pcm->private_data;

	snd_pcm_mmap_appl_forward(pcm, size);
	issue_applptr(hw);
#ifdef DEBUG_MMAP
	fprintf(stderr, "appl_forward: hw_ptr = %li, appl_ptr = %li, size = %li\n", *pcm->hw.ptr, *pcm->appl.ptr, size);
#endif
	return size;
}

static snd_pcm_sframes_t snd_pcm_hw_avail_update(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	snd_pcm_uframes_t avail;

	query_status_data(hw);
	avail = snd_pcm_mmap_avail(pcm);
	switch (FAST_PCM_STATE(hw)) {
	case SNDRV_PCM_STATE_RUNNING:
		if (avail >= pcm->stop_threshold) {
			/* SNDRV_PCM_IOCTL_XRUN ioctl has been implemented since PCM kernel API 2.0.1 */
			if (SNDRV_PROTOCOL_VERSION(2, 0, 1) <= hw->version) {
				if (ioctl(hw->fd, SNDRV_PCM_IOCTL_XRUN) < 0)
					return -errno;
			}
			/* everything is ok, state == SND_PCM_STATE_XRUN at the moment */
			return -EPIPE;
		}
		break;
	case SNDRV_PCM_STATE_XRUN:
		return -EPIPE;
	default:
		break;
	}
	return avail;
}

static int snd_pcm_hw_htimestamp(snd_pcm_t *pcm, snd_pcm_uframes_t *avail,
				 snd_htimestamp_t *tstamp)
{
	snd_pcm_sframes_t avail1;
	int ok = 0;

	/* unfortunately, loop is necessary to ensure valid timestamp */
	while (1) {
		avail1 = snd_pcm_hw_avail_update(pcm);
		if (avail1 < 0)
			return avail1;
		if (ok && (snd_pcm_uframes_t)avail1 == *avail)
			break;
		*avail = avail1;
		*tstamp = snd_pcm_hw_fast_tstamp(pcm);
		ok = 1;
	}
	return 0;
}

static void __fill_chmap_ctl_id(snd_ctl_elem_id_t *id, int dev, int subdev,
				int stream)
{
	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_PCM);
	if (stream == SND_PCM_STREAM_PLAYBACK)
		snd_ctl_elem_id_set_name(id, "Playback Channel Map");
	else
		snd_ctl_elem_id_set_name(id, "Capture Channel Map");
	snd_ctl_elem_id_set_device(id, dev);
	snd_ctl_elem_id_set_index(id, subdev);
}

static void fill_chmap_ctl_id(snd_pcm_t *pcm, snd_ctl_elem_id_t *id)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	return __fill_chmap_ctl_id(id, hw->device, hw->subdevice, pcm->stream);
}

static int is_chmap_type(int type)
{
	return (type >= SND_CTL_TLVT_CHMAP_FIXED &&
		type <= SND_CTL_TLVT_CHMAP_PAIRED);
}

/**
 * \!brief Query the available channel maps
 * \param card the card number
 * \param dev the PCM device number
 * \param subdev the PCM substream index
 * \param stream the direction of PCM stream
 * \return the NULL-terminated array of integer pointers, or NULL at error.
 *
 * This function works like snd_pcm_query_chmaps() but it takes the card,
 * device, substream and stream numbers instead of the already opened
 * snd_pcm_t instance, so that you can query available channel maps of
 * a PCM before actually opening it.
 *
 * As the parameters stand, the query is performed only to the hw PCM
 * devices, not the abstracted PCM object in alsa-lib.
 */
snd_pcm_chmap_query_t **
snd_pcm_query_chmaps_from_hw(int card, int dev, int subdev,
			     snd_pcm_stream_t stream)
{
	snd_ctl_t *ctl;
	snd_ctl_elem_id_t id = {0};
	unsigned int tlv[2048], *start;
	unsigned int type;
	snd_pcm_chmap_query_t **map;
	int i, ret, nums;

	ret = snd_ctl_hw_open(&ctl, NULL, card, 0);
	if (ret < 0) {
		SYSMSG("Cannot open the associated CTL\n");
		return NULL;
	}

	__fill_chmap_ctl_id(&id, dev, subdev, stream);
	ret = snd_ctl_elem_tlv_read(ctl, &id, tlv, sizeof(tlv));
	snd_ctl_close(ctl);
	if (ret < 0) {
		SYSMSG("Cannot read Channel Map TLV\n");
		return NULL;
	}

#if 0
	for (i = 0; i < 32; i++)
		fprintf(stderr, "%02x: %08x\n", i, tlv[i]);
#endif
	/* FIXME: the parser below assumes that the TLV only contains
	 * chmap-related blocks
	 */
	type = tlv[SNDRV_CTL_TLVO_TYPE];
	if (type != SND_CTL_TLVT_CONTAINER) {
		if (!is_chmap_type(type)) {
			SYSMSG("Invalid TLV type %d\n", type);
			return NULL;
		}
		start = tlv;
		nums = 1;
	} else {
		unsigned int *p;
		int size;
		start = tlv + 2;
		size = tlv[SNDRV_CTL_TLVO_LEN];
		nums = 0;
		for (p = start; size > 0; ) {
			if (!is_chmap_type(p[0])) {
				SYSMSG("Invalid TLV type %d\n", p[0]);
				return NULL;
			}
			nums++;
			size -= p[1] + 8;
			p += p[1] / 4 + 2;
		}
	}
	map = calloc(nums + 1, sizeof(int *));
	if (!map)
		return NULL;
	for (i = 0; i < nums; i++) {
		map[i] = malloc(start[1] + 8);
		if (!map[i]) {
			snd_pcm_free_chmaps(map);
			return NULL;
		}
		map[i]->type = start[0] - 0x100;
		map[i]->map.channels = start[1] / 4;
		memcpy(map[i]->map.pos, start + 2, start[1]);
		start += start[1] / 4 + 2;
	}
	return map;
}

enum { CHMAP_CTL_QUERY, CHMAP_CTL_GET, CHMAP_CTL_SET };

static int chmap_caps(snd_pcm_hw_t *hw, int type)
{
	if (hw->chmap_caps & (1 << type))
		return 1;
	if (hw->chmap_caps & (1 << (type + 8)))
		return 0;
	return 1;
}

static void chmap_caps_set_ok(snd_pcm_hw_t *hw, int type)
{
	hw->chmap_caps |= (1 << type);
}

static void chmap_caps_set_error(snd_pcm_hw_t *hw, int type)
{
	hw->chmap_caps |= (1 << (type + 8));
}

static snd_pcm_chmap_query_t **snd_pcm_hw_query_chmaps(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	snd_pcm_chmap_query_t **map;

	if (hw->chmap_override)
		return _snd_pcm_copy_chmap_query(hw->chmap_override);

	if (!chmap_caps(hw, CHMAP_CTL_QUERY))
		return NULL;

	map = snd_pcm_query_chmaps_from_hw(hw->card, hw->device,
					   hw->subdevice, pcm->stream);
	if (map)
		chmap_caps_set_ok(hw, CHMAP_CTL_QUERY);
	else
		chmap_caps_set_error(hw, CHMAP_CTL_QUERY);
	return map;
}

static snd_pcm_chmap_t *snd_pcm_hw_get_chmap(snd_pcm_t *pcm)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	snd_pcm_chmap_t *map;
	snd_ctl_t *ctl;
	snd_ctl_elem_id_t id = {0};
	snd_ctl_elem_value_t val = {0};
	unsigned int i;
	int ret;

	if (hw->chmap_override)
		return _snd_pcm_choose_fixed_chmap(pcm, hw->chmap_override);

	if (!chmap_caps(hw, CHMAP_CTL_GET))
		return NULL;

	switch (FAST_PCM_STATE(hw)) {
	case SNDRV_PCM_STATE_PREPARED:
	case SNDRV_PCM_STATE_RUNNING:
	case SNDRV_PCM_STATE_XRUN:
	case SNDRV_PCM_STATE_DRAINING:
	case SNDRV_PCM_STATE_PAUSED:
	case SNDRV_PCM_STATE_SUSPENDED:
		break;
	default:
		SYSMSG("Invalid PCM state for chmap_get: %s\n",
		       snd_pcm_state_name(FAST_PCM_STATE(hw)));
		return NULL;
	}
	map = malloc(pcm->channels * sizeof(map->pos[0]) + sizeof(*map));
	if (!map)
		return NULL;
	map->channels = pcm->channels;
	ret = snd_ctl_hw_open(&ctl, NULL, hw->card, 0);
	if (ret < 0) {
		free(map);
		SYSMSG("Cannot open the associated CTL\n");
		chmap_caps_set_error(hw, CHMAP_CTL_GET);
		return NULL;
	}
	fill_chmap_ctl_id(pcm, &id);
	snd_ctl_elem_value_set_id(&val, &id);
	ret = snd_ctl_elem_read(ctl, &val);
	snd_ctl_close(ctl);
	if (ret < 0) {
		free(map);
		SYSMSG("Cannot read Channel Map ctl\n");
		chmap_caps_set_error(hw, CHMAP_CTL_GET);
		return NULL;
	}
	for (i = 0; i < pcm->channels; i++)
		map->pos[i] = snd_ctl_elem_value_get_integer(&val, i);
	chmap_caps_set_ok(hw, CHMAP_CTL_GET);
	return map;
}

static int snd_pcm_hw_set_chmap(snd_pcm_t *pcm, const snd_pcm_chmap_t *map)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	snd_ctl_t *ctl;
	snd_ctl_elem_id_t id = {0};
	snd_ctl_elem_value_t val = {0};
	unsigned int i;
	int ret;

	if (hw->chmap_override)
		return -ENXIO;

	if (!chmap_caps(hw, CHMAP_CTL_SET))
		return -ENXIO;

	if (map->channels > 128) {
		SYSMSG("Invalid number of channels %d\n", map->channels);
		return -EINVAL;
	}
	if (FAST_PCM_STATE(hw) != SNDRV_PCM_STATE_PREPARED) {
		SYSMSG("Invalid PCM state for chmap_set: %s\n",
		       snd_pcm_state_name(FAST_PCM_STATE(hw)));
		return -EBADFD;
	}
	ret = snd_ctl_hw_open(&ctl, NULL, hw->card, 0);
	if (ret < 0) {
		SYSMSG("Cannot open the associated CTL\n");
		chmap_caps_set_error(hw, CHMAP_CTL_SET);
		return ret;
	}

	fill_chmap_ctl_id(pcm, &id);
	snd_ctl_elem_value_set_id(&val, &id);
	for (i = 0; i < map->channels; i++)
		snd_ctl_elem_value_set_integer(&val, i, map->pos[i]);
	ret = snd_ctl_elem_write(ctl, &val);
	snd_ctl_close(ctl);
	if (ret >= 0)
		chmap_caps_set_ok(hw, CHMAP_CTL_SET);
	else if (ret == -ENOENT || ret == -EPERM || ret == -ENXIO) {
		chmap_caps_set_error(hw, CHMAP_CTL_SET);
		ret = -ENXIO;
	}
	if (ret < 0)
		SYSMSG("Cannot write Channel Map ctl\n");
	return ret;
}

static void snd_pcm_hw_dump(snd_pcm_t *pcm, snd_output_t *out)
{
	snd_pcm_hw_t *hw = pcm->private_data;
	char *name;
	int err = snd_card_get_name(hw->card, &name);
	if (err < 0) {
		SNDERR("cannot get card name");
		return;
	}
	snd_output_printf(out, "Hardware PCM card %d '%s' device %d subdevice %d\n",
			  hw->card, name, hw->device, hw->subdevice);
	free(name);
	if (pcm->setup) {
		snd_output_printf(out, "Its setup is:\n");
		snd_pcm_dump_setup(pcm, out);
		snd_output_printf(out, "  appl_ptr     : %li\n", hw->mmap_control->appl_ptr);
		snd_output_printf(out, "  hw_ptr       : %li\n", hw->mmap_status->hw_ptr);
	}
}

static const snd_pcm_ops_t snd_pcm_hw_ops = {
	.close = snd_pcm_hw_close,
	.info = snd_pcm_hw_info,
	.hw_refine = snd_pcm_hw_hw_refine,
	.hw_params = snd_pcm_hw_hw_params,
	.hw_free = snd_pcm_hw_hw_free,
	.sw_params = snd_pcm_hw_sw_params,
	.channel_info = snd_pcm_hw_channel_info,
	.dump = snd_pcm_hw_dump,
	.nonblock = snd_pcm_hw_nonblock,
	.async = snd_pcm_hw_async,
	.mmap = snd_pcm_hw_mmap,
	.munmap = snd_pcm_hw_munmap,
	.query_chmaps = snd_pcm_hw_query_chmaps,
	.get_chmap = snd_pcm_hw_get_chmap,
	.set_chmap = snd_pcm_hw_set_chmap,
};

static const snd_pcm_fast_ops_t snd_pcm_hw_fast_ops = {
	.status = snd_pcm_hw_status,
	.state = snd_pcm_hw_state,
	.hwsync = snd_pcm_hw_hwsync,
	.delay = snd_pcm_hw_delay,
	.prepare = snd_pcm_hw_prepare,
	.reset = snd_pcm_hw_reset,
	.start = snd_pcm_hw_start,
	.drop = snd_pcm_hw_drop,
	.drain = snd_pcm_hw_drain,
	.pause = snd_pcm_hw_pause,
	.rewindable = snd_pcm_hw_rewindable,
	.rewind = snd_pcm_hw_rewind,
	.forwardable = snd_pcm_hw_forwardable,
	.forward = snd_pcm_hw_forward,
	.resume = snd_pcm_hw_resume,
	.link = snd_pcm_hw_link,
	.link_slaves = snd_pcm_hw_link_slaves,
	.unlink = snd_pcm_hw_unlink,
	.writei = snd_pcm_hw_writei,
	.writen = snd_pcm_hw_writen,
	.readi = snd_pcm_hw_readi,
	.readn = snd_pcm_hw_readn,
	.avail_update = snd_pcm_hw_avail_update,
	.mmap_commit = snd_pcm_hw_mmap_commit,
	.htimestamp = snd_pcm_hw_htimestamp,
	.poll_descriptors = NULL,
	.poll_descriptors_count = NULL,
	.poll_revents = NULL,
};

static const snd_pcm_fast_ops_t snd_pcm_hw_fast_ops_timer = {
	.status = snd_pcm_hw_status,
	.state = snd_pcm_hw_state,
	.hwsync = snd_pcm_hw_hwsync,
	.delay = snd_pcm_hw_delay,
	.prepare = snd_pcm_hw_prepare,
	.reset = snd_pcm_hw_reset,
	.start = snd_pcm_hw_start,
	.drop = snd_pcm_hw_drop,
	.drain = snd_pcm_hw_drain,
	.pause = snd_pcm_hw_pause,
	.rewindable = snd_pcm_hw_rewindable,
	.rewind = snd_pcm_hw_rewind,
	.forwardable = snd_pcm_hw_forwardable,
	.forward = snd_pcm_hw_forward,
	.resume = snd_pcm_hw_resume,
	.link = snd_pcm_hw_link,
	.link_slaves = snd_pcm_hw_link_slaves,
	.unlink = snd_pcm_hw_unlink,
	.writei = snd_pcm_hw_writei,
	.writen = snd_pcm_hw_writen,
	.readi = snd_pcm_hw_readi,
	.readn = snd_pcm_hw_readn,
	.avail_update = snd_pcm_hw_avail_update,
	.mmap_commit = snd_pcm_hw_mmap_commit,
	.htimestamp = snd_pcm_hw_htimestamp,
	.poll_descriptors = snd_pcm_hw_poll_descriptors,
	.poll_descriptors_count = snd_pcm_hw_poll_descriptors_count,
	.poll_revents = snd_pcm_hw_poll_revents,
};

/**
 * \brief Creates a new hw PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param fd File descriptor
 * \param sync_ptr_ioctl Boolean flag for sync_ptr ioctl
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int snd_pcm_hw_open_fd(snd_pcm_t **pcmp, const char *name, int fd,
		       int sync_ptr_ioctl)
{
	int ver, mode;
	snd_pcm_tstamp_type_t tstamp_type = SND_PCM_TSTAMP_TYPE_GETTIMEOFDAY;
	long fmode;
	snd_pcm_t *pcm = NULL;
	snd_pcm_hw_t *hw = NULL;
	snd_pcm_info_t info;
	int ret;

	assert(pcmp);

	memset(&info, 0, sizeof(info));
	if (ioctl(fd, SNDRV_PCM_IOCTL_INFO, &info) < 0) {
		ret = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_INFO failed (%i)", ret);
		close(fd);
		return ret;

	}

	if ((fmode = fcntl(fd, F_GETFL)) < 0) {
		ret = -errno;
		close(fd);
		return ret;
	}
	mode = 0;
	if (fmode & O_NONBLOCK)
		mode |= SND_PCM_NONBLOCK;
	if (fmode & O_ASYNC)
		mode |= SND_PCM_ASYNC;
	if (fmode & O_APPEND)
		mode |= SND_PCM_APPEND;

	if (ioctl(fd, SNDRV_PCM_IOCTL_PVERSION, &ver) < 0) {
		ret = -errno;
		SYSMSG("SNDRV_PCM_IOCTL_PVERSION failed (%i)", ret);
		close(fd);
		return ret;
	}
	if (SNDRV_PROTOCOL_INCOMPATIBLE(ver, SNDRV_PCM_VERSION_MAX))
		return -SND_ERROR_INCOMPATIBLE_VERSION;

	if (SNDRV_PROTOCOL_VERSION(2, 0, 14) <= ver) {
		/* inform the protocol version we're supporting */
		unsigned int user_ver = SNDRV_PCM_VERSION;
		if (ioctl(fd, SNDRV_PCM_IOCTL_USER_PVERSION, &user_ver) < 0) {
			ret = -errno;
			SNDMSG("USER_PVERSION failed\n");
			return ret;
		}
	}

#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_MONOTONIC)
	if (SNDRV_PROTOCOL_VERSION(2, 0, 9) <= ver) {
		struct timespec timespec;
		if (clock_gettime(CLOCK_MONOTONIC, &timespec) == 0) {
			int on = SNDRV_PCM_TSTAMP_TYPE_MONOTONIC;
			if (ioctl(fd, SNDRV_PCM_IOCTL_TTSTAMP, &on) < 0) {
				ret = -errno;
				SNDMSG("TTSTAMP failed\n");
				return ret;
			}
			tstamp_type = SND_PCM_TSTAMP_TYPE_MONOTONIC;
		}
	} else
#endif
	  if (SNDRV_PROTOCOL_VERSION(2, 0, 5) <= ver) {
		int on = 1;
		if (ioctl(fd, SNDRV_PCM_IOCTL_TSTAMP, &on) < 0) {
			ret = -errno;
			SNDMSG("TSTAMP failed\n");
			return ret;
		}
	}
	
	hw = calloc(1, sizeof(snd_pcm_hw_t));
	if (!hw) {
		close(fd);
		return -ENOMEM;
	}

	hw->version = ver;
	hw->card = info.card;
	hw->device = info.device;
	hw->subdevice = info.subdevice;
	hw->fd = fd;
	/* no restriction */
	hw->format = SND_PCM_FORMAT_UNKNOWN;
	hw->rate = 0;
	hw->channels = 0;

	ret = snd_pcm_new(&pcm, SND_PCM_TYPE_HW, name, info.stream, mode);
	if (ret < 0) {
		free(hw);
		close(fd);
		return ret;
	}

	pcm->ops = &snd_pcm_hw_ops;
	pcm->fast_ops = &snd_pcm_hw_fast_ops;
	pcm->private_data = hw;
	pcm->poll_fd = fd;
	pcm->poll_events = info.stream == SND_PCM_STREAM_PLAYBACK ? POLLOUT : POLLIN;
	pcm->tstamp_type = tstamp_type;
#ifdef THREAD_SAFE_API
	pcm->need_lock = 0;	/* hw plugin is thread-safe */
#endif
	pcm->own_state_check = 1; /* skip the common state check */

	ret = map_status_and_control_data(pcm, !!sync_ptr_ioctl);
	if (ret < 0) {
		snd_pcm_close(pcm);
		return ret;
	}

	*pcmp = pcm;
	return 0;
}

/**
 * \brief Creates a new hw PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param card Number of card
 * \param device Number of device
 * \param subdevice Number of subdevice
 * \param stream PCM Stream
 * \param mode PCM Mode
 * \param mmap_emulation Obsoleted parameter
 * \param sync_ptr_ioctl Use SYNC_PTR ioctl rather than mmap for control structures
 * \retval zero on success otherwise a negative error code
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int snd_pcm_hw_open(snd_pcm_t **pcmp, const char *name,
		    int card, int device, int subdevice,
		    snd_pcm_stream_t stream, int mode,
		    int mmap_emulation ATTRIBUTE_UNUSED,
		    int sync_ptr_ioctl)
{
	char filename[sizeof(SNDRV_FILE_PCM_STREAM_PLAYBACK) + 20];
	const char *filefmt;
	int ret = 0, fd = -1;
	int attempt = 0;
	snd_pcm_info_t info;
	int fmode;
	snd_ctl_t *ctl;

	assert(pcmp);

	if ((ret = snd_ctl_hw_open(&ctl, NULL, card, 0)) < 0)
		return ret;

	switch (stream) {
	case SND_PCM_STREAM_PLAYBACK:
		filefmt = SNDRV_FILE_PCM_STREAM_PLAYBACK;
		break;
	case SND_PCM_STREAM_CAPTURE:
		filefmt = SNDRV_FILE_PCM_STREAM_CAPTURE;
		break;
	default:
		SNDERR("invalid stream %d", stream);
		return -EINVAL;
	}
	sprintf(filename, filefmt, card, device);

      __again:
      	if (attempt++ > 3) {
		ret = -EBUSY;
		goto _err;
	}
	ret = snd_ctl_pcm_prefer_subdevice(ctl, subdevice);
	if (ret < 0)
		goto _err;
	fmode = O_RDWR;
	if (mode & SND_PCM_NONBLOCK)
		fmode |= O_NONBLOCK;
	if (mode & SND_PCM_ASYNC)
		fmode |= O_ASYNC;
	if (mode & SND_PCM_APPEND)
		fmode |= O_APPEND;
	fd = snd_open_device(filename, fmode);
	if (fd < 0) {
		ret = -errno;
		SYSMSG("open '%s' failed (%i)", filename, ret);
		goto _err;
	}
	if (subdevice >= 0) {
		memset(&info, 0, sizeof(info));
		if (ioctl(fd, SNDRV_PCM_IOCTL_INFO, &info) < 0) {
			ret = -errno;
			SYSMSG("SNDRV_PCM_IOCTL_INFO failed (%i)", ret);
			goto _err;
		}
		if (info.subdevice != (unsigned int) subdevice) {
			close(fd);
			goto __again;
		}
	}
	snd_ctl_close(ctl);
	return snd_pcm_hw_open_fd(pcmp, name, fd, sync_ptr_ioctl);
       _err:
	snd_ctl_close(ctl);
	return ret;
}

/*! \page pcm_plugins

\section pcm_plugins_hw Plugin: hw

This plugin communicates directly with the ALSA kernel driver. It is a raw
communication without any conversions. The emulation of mmap access can be
optionally enabled, but expect worse latency in the case.

The nonblock option specifies whether the device is opened in a non-blocking
manner.  Note that the blocking behavior for read/write access won't be
changed by this option.  This influences only on the blocking behavior at
opening the device.  If you would like to keep the compatibility with the
older ALSA stuff, turn this option off.

\code
pcm.name {
	type hw			# Kernel PCM
	card INT/STR		# Card name (string) or number (integer)
	[device INT]		# Device number (default 0)
	[subdevice INT]		# Subdevice number (default -1: first available)
	[sync_ptr_ioctl BOOL]	# Use SYNC_PTR ioctl rather than the direct mmap access for control structures
	[nonblock BOOL]		# Force non-blocking open mode
	[format STR]		# Restrict only to the given format
	[channels INT]		# Restrict only to the given channels
	[rate INT]		# Restrict only to the given rate
	[chmap MAP]		# Override channel maps; MAP is a string array
}
\endcode

\subsection pcm_plugins_hw_funcref Function reference

<UL>
  <LI>snd_pcm_hw_open()
  <LI>_snd_pcm_hw_open()
</UL>

*/

/**
 * \brief Creates a new hw PCM
 * \param pcmp Returns created PCM handle
 * \param name Name of PCM
 * \param root Root configuration node
 * \param conf Configuration node with hw PCM description
 * \param stream PCM Stream
 * \param mode PCM Mode
 * \warning Using of this function might be dangerous in the sense
 *          of compatibility reasons. The prototype might be freely
 *          changed in future.
 */
int _snd_pcm_hw_open(snd_pcm_t **pcmp, const char *name,
		     snd_config_t *root ATTRIBUTE_UNUSED, snd_config_t *conf,
		     snd_pcm_stream_t stream, int mode)
{
	snd_config_iterator_t i, next;
	long card = -1, device = 0, subdevice = -1;
	const char *str;
	int err, sync_ptr_ioctl = 0;
	int rate = 0, channels = 0;
	snd_pcm_format_t format = SND_PCM_FORMAT_UNKNOWN;
	snd_config_t *n;
	int nonblock = 1; /* non-block per default */
	snd_pcm_chmap_query_t **chmap = NULL;
	snd_pcm_hw_t *hw;

	/* look for defaults.pcm.nonblock definition */
	if (snd_config_search(root, "defaults.pcm.nonblock", &n) >= 0) {
		err = snd_config_get_bool(n);
		if (err >= 0)
			nonblock = err;
	}
	snd_config_for_each(i, next, conf) {
		const char *id;
		n = snd_config_iterator_entry(i);
		if (snd_config_get_id(n, &id) < 0)
			continue;
		if (snd_pcm_conf_generic_id(id))
			continue;
		if (strcmp(id, "card") == 0) {
			err = snd_config_get_integer(n, &card);
			if (err < 0) {
				err = snd_config_get_string(n, &str);
				if (err < 0) {
					SNDERR("Invalid type for %s", id);
					err = -EINVAL;
					goto fail;
				}
				card = snd_card_get_index(str);
				if (card < 0) {
					SNDERR("Invalid value for %s", id);
					err = card;
					goto fail;
				}
			}
			continue;
		}
		if (strcmp(id, "device") == 0) {
			err = snd_config_get_integer(n, &device);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				goto fail;
			}
			continue;
		}
		if (strcmp(id, "subdevice") == 0) {
			err = snd_config_get_integer(n, &subdevice);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				goto fail;
			}
			continue;
		}
		if (strcmp(id, "sync_ptr_ioctl") == 0) {
			err = snd_config_get_bool(n);
			if (err < 0)
				continue;
			sync_ptr_ioctl = err;
			continue;
		}
		if (strcmp(id, "nonblock") == 0) {
			err = snd_config_get_bool(n);
			if (err < 0)
				continue;
			nonblock = err;
			continue;
		}
		if (strcmp(id, "rate") == 0) {
			long val;
			err = snd_config_get_integer(n, &val);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				goto fail;
			}
			rate = val;
			continue;
		}
		if (strcmp(id, "format") == 0) {
			err = snd_config_get_string(n, &str);
			if (err < 0) {
				SNDERR("invalid type for %s", id);
				goto fail;
			}
			format = snd_pcm_format_value(str);
			continue;
		}
		if (strcmp(id, "channels") == 0) {
			long val;
			err = snd_config_get_integer(n, &val);
			if (err < 0) {
				SNDERR("Invalid type for %s", id);
				goto fail;
			}
			channels = val;
			continue;
		}
		if (strcmp(id, "chmap") == 0) {
			snd_pcm_free_chmaps(chmap);
			chmap = _snd_pcm_parse_config_chmaps(n);
			if (!chmap) {
				SNDERR("Invalid channel map for %s", id);
				err = -EINVAL;
				goto fail;
			}
			continue;
		}
		SNDERR("Unknown field %s", id);
		err = -EINVAL;
		goto fail;
	}
	if (card < 0) {
		SNDERR("card is not defined");
		err = -EINVAL;
		goto fail;
	}
	err = snd_pcm_hw_open(pcmp, name, card, device, subdevice, stream,
			      mode | (nonblock ? SND_PCM_NONBLOCK : 0),
			      0, sync_ptr_ioctl);
	if (err < 0)
		goto fail;
	if (nonblock && ! (mode & SND_PCM_NONBLOCK)) {
		/* revert to blocking mode for read/write access */
		snd_pcm_hw_nonblock(*pcmp, 0);
		(*pcmp)->mode = mode;
	} else
		/* make sure the SND_PCM_NO_xxx flags don't get lost on the
		 * way */
		(*pcmp)->mode |= mode & (SND_PCM_NO_AUTO_RESAMPLE|
					 SND_PCM_NO_AUTO_CHANNELS|
					 SND_PCM_NO_AUTO_FORMAT|
					 SND_PCM_NO_SOFTVOL);

	hw = (*pcmp)->private_data;
	if (format != SND_PCM_FORMAT_UNKNOWN)
		hw->format = format;
	if (channels > 0)
		hw->channels = channels;
	if (rate > 0)
		hw->rate = rate;
	if (chmap)
		hw->chmap_override = chmap;

	return 0;

fail:
        snd_pcm_free_chmaps(chmap);
        return err;
}

#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(_snd_pcm_hw_open, SND_PCM_DLSYM_VERSION);
#endif

/*
 *  To be removed helpers, but keep binary compatibility at the time
 */

#ifndef DOC_HIDDEN
#define __OLD_TO_NEW_MASK(x) ((x&7)|((x&0x07fffff8)<<5))
#define __NEW_TO_OLD_MASK(x) ((x&7)|((x&0xffffff00)>>5))
#endif

static void snd_pcm_hw_convert_from_old_params(snd_pcm_hw_params_t *params,
					       struct sndrv_pcm_hw_params_old *oparams)
{
	unsigned int i;

	memset(params, 0, sizeof(*params));
	params->flags = oparams->flags;
	for (i = 0; i < sizeof(oparams->masks) / sizeof(unsigned int); i++)
		params->masks[i].bits[0] = oparams->masks[i];
	memcpy(params->intervals, oparams->intervals, sizeof(oparams->intervals));
	params->rmask = __OLD_TO_NEW_MASK(oparams->rmask);
	params->cmask = __OLD_TO_NEW_MASK(oparams->cmask);
	params->info = oparams->info;
	params->msbits = oparams->msbits;
	params->rate_num = oparams->rate_num;
	params->rate_den = oparams->rate_den;
	params->fifo_size = oparams->fifo_size;
}

static void snd_pcm_hw_convert_to_old_params(struct sndrv_pcm_hw_params_old *oparams,
					     snd_pcm_hw_params_t *params,
					     unsigned int *cmask)
{
	unsigned int i, j;

	memset(oparams, 0, sizeof(*oparams));
	oparams->flags = params->flags;
	for (i = 0; i < sizeof(oparams->masks) / sizeof(unsigned int); i++) {
		oparams->masks[i] = params->masks[i].bits[0];
		for (j = 1; j < sizeof(params->masks[i].bits) / sizeof(unsigned int); j++)
			if (params->masks[i].bits[j]) {
				*cmask |= 1 << i;
				break;
			}
	}
	memcpy(oparams->intervals, params->intervals, sizeof(oparams->intervals));
	oparams->rmask = __NEW_TO_OLD_MASK(params->rmask);
	oparams->cmask = __NEW_TO_OLD_MASK(params->cmask);
	oparams->info = params->info;
	oparams->msbits = params->msbits;
	oparams->rate_num = params->rate_num;
	oparams->rate_den = params->rate_den;
	oparams->fifo_size = params->fifo_size;
}

static int use_old_hw_params_ioctl(int fd, unsigned int cmd, snd_pcm_hw_params_t *params)
{
	struct sndrv_pcm_hw_params_old oparams;
	unsigned int cmask = 0;
	int res;
	
	snd_pcm_hw_convert_to_old_params(&oparams, params, &cmask);
	res = ioctl(fd, cmd, &oparams);
	snd_pcm_hw_convert_from_old_params(params, &oparams);
	params->cmask |= cmask;
	return res;
}
