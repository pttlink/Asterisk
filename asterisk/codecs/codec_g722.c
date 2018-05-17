/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 1999 - 2008, Digium, Inc.
 *
 * Matthew Fredrickson <creslin@digium.com>
 * Russell Bryant <russell@digium.com>
 *
 * Special thanks to Steve Underwood for the implementation
 * and for doing the 8khz<->g.722 direct translation code.
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 */

/*! \file
 *
 * \brief codec_g722.c - translate between signed linear and ITU G.722-64kbps
 *
 * \author Matthew Fredrickson <creslin@digium.com>
 * \author Russell Bryant <russell@digium.com>
 *
 * \arg http://soft-switch.org/downloads/non-gpl-bits.tgz
 * \arg http://lists.digium.com/pipermail/asterisk-dev/2006-September/022866.html
 *
 * \ingroup codecs
 */

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision: 130129 $")

#include "asterisk/linkedlists.h"
#include "asterisk/module.h"
#include "asterisk/config.h"
#include "asterisk/options.h"
#include "asterisk/translate.h"
#include "asterisk/utils.h"

#define BUFFER_SAMPLES   8096	/* size for the translation buffers */
#define BUF_SHIFT	5

/* Sample frame data */

#include "g722/g722.h"
#include "slin_g722_ex.h"
#include "g722_slin_ex.h"

struct g722_encoder_pvt {
	g722_encode_state_t g722;
};

struct g722_decoder_pvt {
	g722_decode_state_t g722;
};

/*! \brief init a new instance of g722_encoder_pvt. */
static int lintog722_new(struct ast_trans_pvt *pvt)
{
	struct g722_encoder_pvt *tmp = pvt->pvt;

	g722_encode_init(&tmp->g722, 64000, G722_SAMPLE_RATE_8000);

	return 0;
}

/*! \brief init a new instance of g722_encoder_pvt. */
static int g722tolin_new(struct ast_trans_pvt *pvt)
{
	struct g722_decoder_pvt *tmp = pvt->pvt;

	g722_decode_init(&tmp->g722, 64000, G722_SAMPLE_RATE_8000);

	return 0;
}

static int g722tolin_framein(struct ast_trans_pvt *pvt, struct ast_frame *f)
{
	struct g722_decoder_pvt *tmp = pvt->pvt;
	int out_samples;
	int in_samples;

	/* g722_decode expects the samples to be in the invalid samples / 2 format */
	in_samples = f->samples / 2;

	out_samples = g722_decode(&tmp->g722, (int16_t *) &pvt->outbuf[pvt->samples * sizeof(int16_t)], (uint8_t *) f->data, in_samples);

	pvt->samples += out_samples;

	pvt->datalen += (out_samples * sizeof(int16_t));

	return 0;
}

static int lintog722_framein(struct ast_trans_pvt *pvt, struct ast_frame *f)
{
	struct g722_encoder_pvt *tmp = pvt->pvt;
	int outlen;

	outlen = g722_encode(&tmp->g722, (uint8_t *) (&pvt->outbuf[pvt->datalen]), (int16_t *) f->data, f->samples);


	pvt->samples += outlen * 2;

	pvt->datalen += outlen;

	return 0;
}

static struct ast_frame *g722tolin_sample(void)
{
	static struct ast_frame f = {
		.frametype = AST_FRAME_VOICE,
		.subclass = AST_FORMAT_G722,
		.datalen = sizeof(g722_slin_ex),
		.samples = sizeof(g722_slin_ex) * 2,
		.src = __PRETTY_FUNCTION__,
		.data = g722_slin_ex,
	};

	return &f;
}

static struct ast_frame *lintog722_sample (void)
{
	static struct ast_frame f = {
		.frametype = AST_FRAME_VOICE,
		.subclass = AST_FORMAT_SLINEAR,
		.datalen = sizeof(slin_g722_ex),
		.samples = ARRAY_LEN(slin_g722_ex),
		.src = __PRETTY_FUNCTION__,
		.data = slin_g722_ex,
	};

	return &f;
}

static struct ast_translator g722tolin = {
	.name = "g722tolin",
	.srcfmt = AST_FORMAT_G722,
	.dstfmt = AST_FORMAT_SLINEAR,
	.newpvt = g722tolin_new,	/* same for both directions */
	.framein = g722tolin_framein,
	.sample = g722tolin_sample,
	.desc_size = sizeof(struct g722_decoder_pvt),
	.buffer_samples = BUFFER_SAMPLES / sizeof(int16_t),
	.buf_size = BUFFER_SAMPLES,
	.plc_samples = 160,
};

static struct ast_translator lintog722 = {
	.name = "lintog722",
	.srcfmt = AST_FORMAT_SLINEAR,
	.dstfmt = AST_FORMAT_G722,
	.newpvt = lintog722_new,	/* same for both directions */
	.framein = lintog722_framein,
	.sample = lintog722_sample,
	.desc_size = sizeof(struct g722_encoder_pvt),
	.buffer_samples = BUFFER_SAMPLES * 2,
	.buf_size = BUFFER_SAMPLES,
};

static int parse_config(int reload)
{
	struct ast_variable *var;
	struct ast_config *cfg = ast_config_load("codecs.conf");

	if (!cfg)
		return 0;
	for (var = ast_variable_browse(cfg, "plc"); var; var = var->next) {
		if (!strcasecmp(var->name, "genericplc")) {
			g722tolin.useplc = ast_true(var->value) ? 1 : 0;
			if (option_verbose > 2)
				 ast_verbose(VERBOSE_PREFIX_3 "codec_g722: %susing generic PLC\n", g722tolin.useplc ? "" : "not ");
		}
	}
	ast_config_destroy(cfg);
	return 0;
}

static int reload(void)
{
	if (parse_config(1))
		return AST_MODULE_LOAD_DECLINE;
	return AST_MODULE_LOAD_SUCCESS;
}

static int unload_module(void)
{
	int res = 0;

	res |= ast_unregister_translator(&g722tolin);
	res |= ast_unregister_translator(&lintog722);

	return res;
}

static int load_module(void)
{
	int res = 0;

	if (parse_config(0))
		return AST_MODULE_LOAD_DECLINE;

	res |= ast_register_translator(&g722tolin);
	res |= ast_register_translator(&lintog722);

	if (res) {
		unload_module();
		return AST_MODULE_LOAD_FAILURE;
	}	

	return AST_MODULE_LOAD_SUCCESS;
}

AST_MODULE_INFO(ASTERISK_GPL_KEY, AST_MODFLAG_DEFAULT, "ITU G.722-64kbps G722 Transcoder",
		.load = load_module,
		.unload = unload_module,
		.reload = reload,
	       );
