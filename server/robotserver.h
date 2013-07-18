/*
 * netrobots, server logic interface
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

#ifndef ROBOTSERVER_H
#define ROBOTSERVER_H

#include <sys/time.h>
#include <cairo/cairo.h>

#define RELOAD_RATIO	50
#define SPEED_RATIO	0.04
#define BREAK_DISTANCE	(0.7 / SPEED_RATIO)

#define MAX_ROBOTS	21
#define MAX_NAME_LEN	14

struct cannon {
	int timeToReload;
	int x, y;
};

struct robot {
	/* int fd; Should not be needed as it is synchronized with the array of fds */
	char *name;
	double x, y;
	int damage;
	int speed;
	int break_distance;
	int target_speed;
	int degree;
	int cannon_degree;
	int radar_degree;
	int score;
	int waiting;
	struct timeval life_length;	/* valid only after death */
	struct cannon cannon[2];
	float color[3];
	cairo_surface_t *img;
	void *data;
	unsigned int data_len, data_ptr;
};

extern struct robot **all_robots;
extern int max_robots;

extern struct robot **ranking;
extern int dead_robots;
extern struct timeval game_start;
extern struct timeval game_end;
extern int save_results;

typedef enum game_type_t {
	GAME_SCORE,
	GAME_TIME,
} game_type_t;

extern game_type_t game_type;

/* Interface from networking code to game logic. */

int scan(struct robot *r, int degree, int resolution);
int cannon(struct robot *r, int degree, int range);
void drive(struct robot *r, int degree, int speed);
void cycle(void);
int loc_x(struct robot *r);
int loc_y(struct robot *r);
int speed(struct robot *r);
int damage(struct robot *r);
void kill_robot(struct robot *r);
void complete_ranking(void);

#endif
