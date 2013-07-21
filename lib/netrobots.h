/*
 * libnetrobots, client library for netrobots
 * Copyright (C) 2008 Paolo Bonzini and others
 * Copyright (C) 2013 Jiri Benc
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#ifndef NETROBOTS_H
#define NETROBOTS_H

void init(int argc, char *argv[]);
void init_custom(char *img_path, int argc, char *argv[]);

int scan(int degree, int resolution);
int cannon(int degree, int range);
void drive(int degree, int speed);
void cycle(void);

int damage(void);
int speed(void);
int loc_x(void);
int loc_y(void);
int elapsed(void);
void get_all(int *_loc_x, int *_loc_y, int *_damage, int *_speed, int *_elapsed);

/* server config */
extern int game_type;
extern int shot_speed;
extern int max_cycles;

#endif
