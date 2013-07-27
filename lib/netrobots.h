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

#ifdef WIN32
// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the LIBNETROBOTS_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// LIBNETROBOTS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef LIBNETROBOTS_EXPORTS
#define NETROBOTS_API __declspec(dllexport)
#else
#define NETROBOTS_API __declspec(dllimport)
#endif
#else
#define NETROBOTS_API
#endif

NETROBOTS_API void init(int argc, char **argv);
NETROBOTS_API void init_custom(char *img_path, int argc, char **argv);

NETROBOTS_API int scan(int degree, int resolution);
NETROBOTS_API int cannon(int degree, int range);
NETROBOTS_API void drive(int degree, int speed);
NETROBOTS_API void cycle(void);

NETROBOTS_API int damage(void);
NETROBOTS_API int speed(void);
NETROBOTS_API int loc_x(void);
NETROBOTS_API int loc_y(void);
NETROBOTS_API int elapsed(void);
NETROBOTS_API void get_all(int *_loc_x, int *_loc_y, int *_damage, int *_speed, int *_elapsed);

/* server config */
extern NETROBOTS_API int game_type;
extern NETROBOTS_API int shot_speed;
extern NETROBOTS_API int max_cycles;

#endif
