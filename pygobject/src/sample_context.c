/* SPDX-License-Identifier: LGPL-2.1-or-later */
#include "sample_context.h"

static GMainContext *ctx;
static GThread *thread;

static gboolean running;

static gpointer run_main_loop(gpointer data)
{
	while (running)
		g_main_context_iteration(ctx, TRUE);

	g_thread_exit(NULL);

	return NULL;
}

static GMainContext *get_my_context(GError **exception)
{
	if (!ctx)
		ctx = g_main_context_new();

	if (!thread) {
		thread = g_thread_try_new("gmain", run_main_loop, NULL,
					  exception);
		if (*exception) {
			g_main_context_unref(ctx);
			ctx = NULL;
		}
	}

	return ctx;
}

gpointer sample_context_add_src(GSource *src, gint fd, GIOCondition event,
				GError **exception)
{
	GMainContext *ctx;

	ctx = get_my_context(exception);
	if (*exception)
		return NULL;
	running = TRUE;

	/* NOTE: The returned ID is never used. */
	g_source_attach(src, ctx);

	return NULL;
}
