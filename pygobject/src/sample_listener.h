/* SPDX-License-Identifier: LGPL-2.1-or-later */
#ifndef __SAMPLE_LISTENER_H__
#define __SAMPLE_LISTENER_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define SAMPLE_TYPE_LISTENER	(sample_listener_get_type())

#define SAMPLE_LISTENER(obj)					\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    SAMPLE_TYPE_LISTENER,	\
				    SampleListener))
#define SAMPLE_IS_LISTENER(obj)				\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    SAMPLE_TYPE_LISTENER))

#define SAMPLE_LISTENER_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 SAMPLE_TYPE_LISTENER,	\
				 SampleListenerClass))
#define SAMPLE_IS_LISTENER_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 SAMPLE_TYPE_LISTENER))
#define SAMPLE_LISTENER_GET_CLASS(obj)			\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   SAMPLE_TYPE_LISTENER,	\
				   SampleListenerClass))

typedef struct _SampleListener		SampleListener;
typedef struct _SampleListenerClass	SampleListenerClass;
typedef struct _SampleListenerPrivate	SampleListenerPrivate;

struct _SampleListener {
	GObject parent_instance;

	SampleListenerPrivate *priv;
};

struct _SampleListenerClass {
	GObjectClass parent_class;
};

GType sample_listener_get_type(void) G_GNUC_CONST;

void sample_listener_listen(SampleListener *self, GError **exception);
void sample_listener_unlisten(SampleListener *self);

G_END_DECLS

#endif
