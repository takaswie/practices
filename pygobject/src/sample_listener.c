/* SPDX-License-Identifier: LGPL-2.1-or-later */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "sample_listener.h"
#include "sample_context.h"
#include "sample_sigs_marshal.h"

/* For error handling. */
G_DEFINE_QUARK("SampleListener", sample_listener)
#define raise(exception, errno)					\
	g_set_error(exception, sample_listener_quark(), errno,	\
		    "%d: %s", __LINE__, strerror(errno))

typedef struct {
	GSource src;
	SampleListener *self;
	gpointer tag;
} SampleListenerSource;

struct _SampleListenerPrivate {
	unsigned int len;
	void *buf;
	SampleListenerSource *src;
	unsigned int count;
};
G_DEFINE_TYPE_WITH_PRIVATE(SampleListener, sample_listener, G_TYPE_OBJECT)

/* This object has no properties. */

/* This object has one signal. */
enum sample_listener_sig_type {
	SAMPLE_LISTENER_SIG_TYPE_NOTIFIED = 0,
	SAMPLE_LISTENER_SIG_TYPE_NOTIFIED32,
	SAMPLE_LISTENER_SIG_TYPE_COUNT,
};
static guint sample_listener_sigs[SAMPLE_LISTENER_SIG_TYPE_COUNT];

static void sample_listener_class_init(SampleListenerClass *klass)
{
	/**
	 * SampleListener::notified:
	 * @self: A #SampleListener
	 * @count: the number for index of this notification
	 *
	 * Returns: (element-type guint8)(array)(nullable)(transfer full): An
	 * 	    array with 8bit elements.
	 */
	sample_listener_sigs[SAMPLE_LISTENER_SIG_TYPE_NOTIFIED] =
		g_signal_new("notified",
			     G_OBJECT_CLASS_TYPE(klass),
			     G_SIGNAL_RUN_LAST,
			     0,
			     NULL, NULL,
			     sample_sigs_marshal_BOXED__UINT,
			     G_TYPE_ARRAY, 1, G_TYPE_UINT);

	/**
	 * SampleListener::notified32:
	 * @self: A #SampleListener
	 * @count: the number for index of this notification
	 *
	 * Returns: (element-type guint32)(array)(nullable)(transfer full): An
	 * 	    array with 32bit elements.
	 */
	sample_listener_sigs[SAMPLE_LISTENER_SIG_TYPE_NOTIFIED32] =
		g_signal_new("notified32",
			     G_OBJECT_CLASS_TYPE(klass),
			     G_SIGNAL_RUN_LAST,
			     0,
			     NULL, NULL,
			     sample_sigs_marshal_BOXED__UINT,
			     G_TYPE_ARRAY, 1, G_TYPE_UINT);
}

static void sample_listener_init(SampleListener *self)
{
	return;
}

static gboolean prepare_src(GSource *src, gint *timeout)
{
	/* Use poll timeout to generate periodical events. */
	*timeout = 1000;

	/* This source is not ready, let's poll(2) */
	return FALSE;
}

static gboolean check_src(GSource *gsrc)
{
	SampleListenerSource *src = (SampleListenerSource *)gsrc;
	SampleListener *self = src->self;
	SampleListenerPrivate *priv = sample_listener_get_instance_private(self);
	GArray *resp;

	/* 8bit array. */
	printf("Notify:\n");
	resp = NULL;
	g_signal_emit(self,
		      sample_listener_sigs[SAMPLE_LISTENER_SIG_TYPE_NOTIFIED],
		      0, priv->count, &resp);

	if (resp) {
		for (int i = 0; i < resp->len; ++i) {
			printf("%8u: [%02u] %02x\n",
			       priv->count, i, g_array_index(resp, guint8, i));
		}
		g_array_free(resp, TRUE);
	} else {
		printf("%8u: None\n", priv->count);
	}

	/* 32bit array. */
	resp = NULL;
	printf("Notify32:\n");
	g_signal_emit(self,
		      sample_listener_sigs[SAMPLE_LISTENER_SIG_TYPE_NOTIFIED32],
		      0, priv->count, &resp);

	if (resp) {
		for (int i = 0; i < resp->len; ++i) {
			printf("%8u: [%02u] %08x\n",
			       priv->count, i, g_array_index(resp, guint32, i));
		}
		g_array_free(resp, TRUE);
	} else {
		printf("%8u: None\n", priv->count);
	}

	if (++priv->count == G_MAXUINT)
		priv->count = 0;

	/* Don't go to dispatch, then continue to process this source. */
	return FALSE;
}

static gboolean dispatch_src(GSource *src, GSourceFunc callback,
			     gpointer user_data)
{
	/* Just be sure to continue to process this source. */
	return TRUE;
}

/**
 * sample_listener_listen:
 * @self: A #SampleListener
 * @exception: A #GError
 */
void sample_listener_listen(SampleListener *self, GError **exception)
{
	static GSourceFuncs funcs = {
		.prepare	= prepare_src,
		.check		= check_src,
		.dispatch	= dispatch_src,
		.finalize	= NULL,
	};
	SampleListenerPrivate *priv;
	GSource *src;

	g_return_if_fail(SAMPLE_IS_LISTENER(self));
	priv = sample_listener_get_instance_private(self);

	src = g_source_new(&funcs, sizeof(SampleListenerSource));
	if (!src) {
		raise(exception, ENOMEM);
		return;
	}

	g_source_set_name(src, "SampleListener");
	g_source_set_priority(src, G_PRIORITY_HIGH_IDLE);
	g_source_set_can_recurse(src, TRUE);

	((SampleListenerSource *)src)->self = self;
	priv->src = (SampleListenerSource *)src;

	((SampleListenerSource *)src)->tag =
		sample_context_add_src(src, 0, G_IO_IN, exception);
	if (*exception) {
		g_source_destroy(src);
		priv->src = NULL;
		return;
	}

	priv->count = 0;
}

/**
 * sample_listener_unlisten:
 * @self: A #SampleListener
 */
void sample_listener_unlisten(SampleListener *self)
{
	SampleListenerPrivate *priv;

	g_return_if_fail(SAMPLE_IS_LISTENER(self));
	priv = sample_listener_get_instance_private(self);

	if (priv->src) {
		g_source_destroy((GSource *)priv->src);
		g_free(priv->src);
		priv->src = NULL;
	}
}
