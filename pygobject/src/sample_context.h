/* SPDX-License-Identifier: LGPL-2.1-or-later */
#ifndef __SAMPLE_CONTEXT_H__
#define __SAMPLE_CONTEXT_H__

#include <glib.h>
#include <glib-object.h>

gpointer sample_context_add_src(GSource *src, gint fd, GIOCondition event,
				GError **exception);

#endif
