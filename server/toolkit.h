/*
 * netrobots, toolkit interface
 * Copyright (C) 2008 Paolo Bonzini and others
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

#ifndef DRAWING_H
#define DRAWING_H

#include <cairo.h>

#define WIN_HEIGHT	540
#define WIN_WIDTH	(WIN_HEIGHT + 140)
#define WIN_TITLE	"Netrobots Battlefield"

typedef enum event_t {
	EVENT_NONE,
	EVENT_QUIT,
	EVENT_START,
} event_t;

/* Functions used by main.c as a high-level interface with the toolkit.  */

cairo_t *init_toolkit(int *argc, char ***argv);
void free_toolkit(cairo_t *cr);
event_t process_toolkit(cairo_t **cr);

#endif
