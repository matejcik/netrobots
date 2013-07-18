/*
 * netrobots, map drawing module interface
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

#ifndef MAP_H
#define MAP_H

#include "toolkit.h"
#include "robotserver.h"

/* Initialize the rendering of the map.  */
void init_cairo(int *argc, char ***argv);
void destroy_cairo();
event_t process_cairo(void);

/* Update the canvas */
void update_display(int finished);

int load_image(struct robot *robot);

#endif
