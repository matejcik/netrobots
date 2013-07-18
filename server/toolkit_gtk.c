/*
 * netrobots, gtk interface
 * Copyright (C) 2013 Jiri Benc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "toolkit.h"

static GtkWidget *window;
static cairo_surface_t *(surface[2]);
static cairo_t *(context[2]);
static int current;
static volatile event_t last_event;

static void close_window(GtkWidget *widget, gpointer data)
{
	last_event = EVENT_QUIT;
}

static gboolean key_pressed(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	switch (event->keyval) {
	case GDK_KEY_q:
		last_event = EVENT_QUIT;
		return TRUE;
	case GDK_KEY_s:
	case GDK_KEY_Return:
		last_event = EVENT_START;
		return TRUE;
	}
	return FALSE;
}

static gboolean redraw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	cairo_set_source_surface(cr, surface[1 - current], 0, 0);
	cairo_paint(cr);
}

cairo_t *init_toolkit(int *argc, char ***argv)
{
	cairo_t *cairo_context;
	cairo_rectangle_int_t rect = { 0, 0, WIN_WIDTH, WIN_HEIGHT };
	int i;

	gtk_init(argc, argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request(window, WIN_WIDTH, WIN_HEIGHT);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_window_set_title(GTK_WINDOW(window), WIN_TITLE);
	gtk_widget_set_app_paintable(window, TRUE);
	gtk_widget_set_double_buffered(window, FALSE);
	gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);

	g_signal_connect(window, "destroy", G_CALLBACK(close_window), NULL);
	g_signal_connect(window, "key-press-event", G_CALLBACK(key_pressed), NULL);
	g_signal_connect(window, "draw", G_CALLBACK(redraw), NULL);

	for (i = 0; i < 2; i++) {
		surface[i] = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, WIN_WIDTH, WIN_HEIGHT);
		context[i] = cairo_create(surface[i]);
	}
	current = 0;

	gtk_widget_show(window);

	return context[0];
}

void free_toolkit(cairo_t *cr)
{
	int i;

	for (i = 0; i < 2; i++) {
		cairo_destroy(context[i]);
		cairo_surface_destroy(surface[i]);
	}
}

event_t process_toolkit(cairo_t **cr)
{
	cairo_t *lcr;
	event_t res;
	static int skip = 0;

	current = 1 - current;
	*cr = context[current];
	/* experimental: render every 5th frame (for Broadway) */
	if (!skip)
		gtk_widget_queue_draw_area(window, 0, 0, WIN_WIDTH, WIN_HEIGHT);
	if (++skip > 4)
		skip = 0;
	while (gtk_events_pending())
		gtk_main_iteration();
	res = last_event;
	last_event = EVENT_NONE;
	return res;
}
