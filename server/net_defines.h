/*
 * netrobots
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

#ifndef NET_DEFINES_H
#define NET_DEFINES_H

#include <stdbool.h>

#include "robotserver.h"
#include "toolkit.h"
#include "net_command_list.h"

typedef enum cmd_type_t {
	CMD_TYPE_NONE,
	CMD_TYPE_INT,
	CMD_TYPE_STR,
} cmd_type_t;

#define CMD_FLAG_CYCLE		(1 << 0)
#define CMD_FLAG_PRESTART	(1 << 1)
#define CMD_FLAG_DATA		(1 << 2)

typedef int (*cmd_f)(struct robot *robot, void *args);

typedef struct cmd_t {
	char cmd;
	cmd_type_t type;
	int args;
	cmd_f func;
	unsigned int flags;
} cmd_t;

#define RES_FLAG_ERROR		(1 << 0)
#define RES_FLAG_CYCLE		(1 << 1)
#define RES_FLAG_DATA		(1 << 2)
#define RES_FLAG_BLOCK		(1 << 3)

typedef enum result_error_t {
	ERR_ERR,	/* unspecified error */
	ERR_PARSE,	/* unparsable command */
	ERR_UNKNOWN,	/* unknown command */
	ERR_ARGS,	/* wrong argument type or count */
	ERR_DENIED,	/* command not allowed */
	__ERR_MAX
} result_error_t;

typedef struct result_t {
	int result;
	unsigned int flags;
} result_t;

int server_init(int argc, char *argv[]);
int server_process_connections(event_t event);
int server_cycle(event_t event);
int server_finished_cycle(event_t event);

result_t execute_cmd(struct robot *robot, char *input, int phase);

#endif
