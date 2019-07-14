/*
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 */

#include <limits.h>
#include <stdint.h>
#include <stdbool.h>

#include "local.h"
#include "list.h"
#include "topology.h"

#include <sound/asound.h>
#include <sound/asoc.h>
#include <sound/tlv.h>

#ifdef TPLG_DEBUG
#define tplg_dbg SNDERR
#else
#define tplg_dbg(fmt, arg...) do { } while (0)
#endif

#define MAX_FILE		256
#define TPLG_MAX_PRIV_SIZE	(1024 * 128)

/** The name of the environment variable containing the tplg directory */
#define ALSA_CONFIG_TPLG_VAR "ALSA_CONFIG_TPLG"

struct tplg_ref;
struct tplg_elem;

typedef enum _snd_pcm_rates {
	SND_PCM_RATE_UNKNOWN = -1,
	SND_PCM_RATE_5512 = 0,
	SND_PCM_RATE_8000,
	SND_PCM_RATE_11025,
	SND_PCM_RATE_16000,
	SND_PCM_RATE_22050,
	SND_PCM_RATE_32000,
	SND_PCM_RATE_44100,
	SND_PCM_RATE_48000,
	SND_PCM_RATE_64000,
	SND_PCM_RATE_88200,
	SND_PCM_RATE_96000,
	SND_PCM_RATE_176400,
	SND_PCM_RATE_192000,
	SND_PCM_RATE_CONTINUOUS = 30,
	SND_PCM_RATE_KNOT = 31,
	SND_PCM_RATE_LAST = SND_PCM_RATE_KNOT,
} snd_pcm_rates_t;

struct snd_tplg {

	/* opaque vendor data */
	int vendor_fd;
	char *vendor_name;

	/* out file */
	int out_fd;

	int verbose;
	unsigned int version;

	/* runtime state */
	unsigned int next_hdr_pos;
	int index;
	int channel_idx;

	/* manifest */
	struct snd_soc_tplg_manifest manifest;
	void *manifest_pdata;	/* copied by builder at file write */

	/* list of each element type */
	struct list_head tlv_list;
	struct list_head widget_list;
	struct list_head pcm_list;
	struct list_head dai_list;
	struct list_head be_list;
	struct list_head cc_list;
	struct list_head route_list;
	struct list_head text_list;
	struct list_head pdata_list;
	struct list_head token_list;
	struct list_head tuple_list;
	struct list_head manifest_list;
	struct list_head pcm_config_list;
	struct list_head pcm_caps_list;
	struct list_head hw_cfg_list;

	/* type-specific control lists */
	struct list_head mixer_list;
	struct list_head enum_list;
	struct list_head bytes_ext_list;
};

/* object text references */
struct tplg_ref {
	unsigned int type;
	struct tplg_elem *elem;
	char id[SNDRV_CTL_ELEM_ID_NAME_MAXLEN];
	struct list_head list;
};

struct tplg_texts {
	unsigned int num_items;
	char items[SND_SOC_TPLG_NUM_TEXTS][SNDRV_CTL_ELEM_ID_NAME_MAXLEN];
};

/* element for vendor tokens */
struct tplg_token {
	char id[SNDRV_CTL_ELEM_ID_NAME_MAXLEN];
	unsigned int value;
};

struct tplg_vendor_tokens {
	unsigned int num_tokens;
	struct tplg_token token[0];
};

/* element for vendor tuples */
struct tplg_tuple {
	char token[SNDRV_CTL_ELEM_ID_NAME_MAXLEN];
	union {
		char string[SNDRV_CTL_ELEM_ID_NAME_MAXLEN];
		unsigned char uuid[16];
		unsigned int value;
	};
};

struct tplg_tuple_set {
	unsigned int  type; /* uuid, bool, byte, short, word, string*/
	unsigned int  num_tuples;
	struct tplg_tuple tuple[0];
};

struct tplg_vendor_tuples {
	unsigned int  num_sets;
	struct tplg_tuple_set **set;
};

/* topology element */
struct tplg_elem {

	char id[SNDRV_CTL_ELEM_ID_NAME_MAXLEN];

	int index;
	enum snd_tplg_type type;

	int size; /* total size of this object inc pdata and ref objects */
	int compound_elem; /* dont write this element as individual elem */
	int vendor_type; /* vendor type for private data */

	/* UAPI object for this elem */
	union {
		void *obj;
		struct snd_soc_tplg_mixer_control *mixer_ctrl;
		struct snd_soc_tplg_enum_control *enum_ctrl;
		struct snd_soc_tplg_bytes_control *bytes_ext;
		struct snd_soc_tplg_dapm_widget *widget;
		struct snd_soc_tplg_pcm *pcm;
		struct snd_soc_tplg_dai *dai;
		struct snd_soc_tplg_link_config *link;/* physical link */
		struct snd_soc_tplg_dapm_graph_elem *route;
		struct snd_soc_tplg_stream *stream_cfg;
		struct snd_soc_tplg_stream_caps *stream_caps;
		struct snd_soc_tplg_hw_config *hw_cfg;

		/* these do not map to UAPI structs but are internal only */
		struct snd_soc_tplg_ctl_tlv *tlv;
		struct tplg_texts *texts;
		struct snd_soc_tplg_private *data;
		struct tplg_vendor_tokens *tokens;
		struct tplg_vendor_tuples *tuples;
		struct snd_soc_tplg_manifest *manifest;
	};

	/* an element may refer to other elements:
	 * a mixer control may refer to a tlv,
	 * a widget may refer to a mixer control array,
	 * a graph may refer to some widgets.
	 */
	struct list_head ref_list;
	struct list_head list; /* list of all elements with same type */

	void (*free)(void *obj);
};

struct map_elem {
	const char *name;
	int id;
};

int tplg_parse_compound(snd_tplg_t *tplg, snd_config_t *cfg,
	int (*fcn)(snd_tplg_t *, snd_config_t *, void *),
	void *private);

int tplg_write_data(snd_tplg_t *tplg);

int tplg_parse_tlv(snd_tplg_t *tplg, snd_config_t *cfg,
	void *private ATTRIBUTE_UNUSED);

int tplg_parse_text(snd_tplg_t *tplg, snd_config_t *cfg,
	void *private ATTRIBUTE_UNUSED);

int tplg_parse_data(snd_tplg_t *tplg, snd_config_t *cfg,
	void *private ATTRIBUTE_UNUSED);

int tplg_parse_tokens(snd_tplg_t *tplg, snd_config_t *cfg,
	void *private ATTRIBUTE_UNUSED);

int tplg_parse_tuples(snd_tplg_t *tplg, snd_config_t *cfg,
	void *private ATTRIBUTE_UNUSED);

void tplg_free_tuples(void *obj);

int tplg_parse_manifest_data(snd_tplg_t *tplg, snd_config_t *cfg,
	void *private ATTRIBUTE_UNUSED);

int tplg_parse_control_bytes(snd_tplg_t *tplg,
	snd_config_t *cfg, void *private ATTRIBUTE_UNUSED);

int tplg_parse_control_enum(snd_tplg_t *tplg, snd_config_t *cfg,
	void *private ATTRIBUTE_UNUSED);

int tplg_parse_control_mixer(snd_tplg_t *tplg,
	snd_config_t *cfg, void *private ATTRIBUTE_UNUSED);

int tplg_parse_dapm_graph(snd_tplg_t *tplg, snd_config_t *cfg,
	void *private ATTRIBUTE_UNUSED);

int tplg_parse_dapm_widget(snd_tplg_t *tplg,
	snd_config_t *cfg, void *private ATTRIBUTE_UNUSED);

int tplg_parse_stream_caps(snd_tplg_t *tplg,
	snd_config_t *cfg, void *private ATTRIBUTE_UNUSED);

int tplg_parse_pcm(snd_tplg_t *tplg,
	snd_config_t *cfg, void *private ATTRIBUTE_UNUSED);

int tplg_parse_dai(snd_tplg_t *tplg, snd_config_t *cfg,
		   void *private ATTRIBUTE_UNUSED);

int tplg_parse_link(snd_tplg_t *tplg,
	snd_config_t *cfg, void *private ATTRIBUTE_UNUSED);

int tplg_parse_cc(snd_tplg_t *tplg,
	snd_config_t *cfg, void *private ATTRIBUTE_UNUSED);

int tplg_parse_hw_config(snd_tplg_t *tplg, snd_config_t *cfg,
			 void *private ATTRIBUTE_UNUSED);

int tplg_build_data(snd_tplg_t *tplg);
int tplg_build_manifest_data(snd_tplg_t *tplg);
int tplg_build_controls(snd_tplg_t *tplg);
int tplg_build_widgets(snd_tplg_t *tplg);
int tplg_build_routes(snd_tplg_t *tplg);
int tplg_build_pcm_dai(snd_tplg_t *tplg, unsigned int type);

int tplg_copy_data(snd_tplg_t *tplg, struct tplg_elem *elem,
		   struct tplg_ref *ref);

int tplg_parse_data_refs(snd_config_t *cfg, struct tplg_elem *elem);

int tplg_ref_add(struct tplg_elem *elem, int type, const char* id);
int tplg_ref_add_elem(struct tplg_elem *elem, struct tplg_elem *elem_ref);

struct tplg_elem *tplg_elem_new(void);
void tplg_elem_free(struct tplg_elem *elem);
void tplg_elem_free_list(struct list_head *base);
struct tplg_elem *tplg_elem_lookup(struct list_head *base,
				const char* id,
				unsigned int type,
				int index);
struct tplg_elem* tplg_elem_new_common(snd_tplg_t *tplg,
	snd_config_t *cfg, const char *name, enum snd_tplg_type type);

int tplg_parse_channel(snd_tplg_t *tplg ATTRIBUTE_UNUSED,
	snd_config_t *cfg, void *private);

int tplg_parse_ops(snd_tplg_t *tplg ATTRIBUTE_UNUSED,
	snd_config_t *cfg, void *private);
int tplg_parse_ext_ops(snd_tplg_t *tplg ATTRIBUTE_UNUSED,
	snd_config_t *cfg, void *private);

struct tplg_elem *lookup_pcm_dai_stream(struct list_head *base,
	const char* id);

int tplg_add_mixer_object(snd_tplg_t *tplg, snd_tplg_obj_template_t *t);
int tplg_add_enum_object(snd_tplg_t *tplg, snd_tplg_obj_template_t *t);
int tplg_add_bytes_object(snd_tplg_t *tplg, snd_tplg_obj_template_t *t);
int tplg_add_widget_object(snd_tplg_t *tplg, snd_tplg_obj_template_t *t);
int tplg_add_graph_object(snd_tplg_t *tplg, snd_tplg_obj_template_t *t);

int tplg_add_mixer(snd_tplg_t *tplg, struct snd_tplg_mixer_template *mixer,
		   struct tplg_elem **e);
int tplg_add_enum(snd_tplg_t *tplg, struct snd_tplg_enum_template *enum_ctl,
		  struct tplg_elem **e);
int tplg_add_bytes(snd_tplg_t *tplg, struct snd_tplg_bytes_template *bytes_ctl,
		   struct tplg_elem **e);

int tplg_build_pcms(snd_tplg_t *tplg, unsigned int type);
int tplg_build_dais(snd_tplg_t *tplg, unsigned int type);
int tplg_build_links(snd_tplg_t *tplg, unsigned int type);
int tplg_add_link_object(snd_tplg_t *tplg, snd_tplg_obj_template_t *t);
int tplg_add_pcm_object(snd_tplg_t *tplg, snd_tplg_obj_template_t *t);
int tplg_add_dai_object(snd_tplg_t *tplg, snd_tplg_obj_template_t *t);
