/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <stdio.h>
#include <stdlib.h>

#include "sample_listener.h"

static GMainLoop *loop;

static GArray *handle_notice(SampleListener *listener, guint count,
			     gpointer user_data)
{
	GArray *ret;

	printf("  notified: %u\n", count);

	if (count % 2 == 0) {
		ret = NULL;
	} else {
		ret = g_array_sized_new(FALSE, TRUE, sizeof(guint8), 6);
		g_array_append_vals(ret,
			(gconstpointer)((guint8[]){0, 1, 2, 3, 4, 5}), 6);
	}

	return ret;
}

static void handle_unix_signal_for_finish(int sig)
{
	g_main_loop_quit(loop);
}

int main(void)
{
	SampleListener *listener;
	struct sigaction sa = {
		.sa_handler = handle_unix_signal_for_finish,
	};
	GError *exception = NULL;

	if (sigaction(SIGINT, &sa, NULL) < 0)
		return EXIT_FAILURE;

	loop = g_main_loop_new(NULL, FALSE);

	listener = g_object_new(SAMPLE_TYPE_LISTENER, NULL);
	if (!listener)
		return EXIT_FAILURE;

	g_signal_connect(listener, "notified", G_CALLBACK(handle_notice), NULL);

	sample_listener_listen(listener, &exception);
	if (exception)
		return EXIT_FAILURE;

	g_main_loop_run(loop);

	sample_listener_unlisten(listener);

	return EXIT_SUCCESS;
}
