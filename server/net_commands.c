/*
 * netrobots, command evaluation module
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "robotserver.h"
#include "net_utils.h"
#include "net_defines.h"

int multivalue[5];

int cmd_start(struct robot *robot, int *args);
int cmd_cycle(struct robot *robot, int *args);
int cmd_cannon(struct robot *robot, int *args);
int cmd_scan(struct robot *robot, int *args);
int cmd_loc_x(struct robot *robot, int *args);
int cmd_loc_y(struct robot *robot, int *args);
int cmd_damage(struct robot *robot, int *args);
int cmd_speed(struct robot *robot, int *args);
int cmd_elapsed(struct robot *robot, int *args);
int cmd_get_all(struct robot *robot, int *args);
int cmd_drive(struct robot *robot, int *args);
int cmd_name(struct robot *robot, char **args);
int cmd_image(struct robot *robot, int *args);

#define MAX_ARGS	2

cmd_t cmds[] = {
	{ START,  CMD_TYPE_NONE, 0, (cmd_f)cmd_start,  CMD_FLAG_PRESTART },
	{ CYCLE,  CMD_TYPE_NONE, 0, (cmd_f)cmd_cycle,  CMD_FLAG_CYCLE | CMD_FLAG_PRESTART },
	{ CANNON, CMD_TYPE_INT,  2, (cmd_f)cmd_cannon, CMD_FLAG_CYCLE },
	{ SCAN,   CMD_TYPE_INT,  2, (cmd_f)cmd_scan,   CMD_FLAG_CYCLE },
	{ DRIVE,  CMD_TYPE_INT,  2, (cmd_f)cmd_drive,  CMD_FLAG_CYCLE },
	{ LOC_X,  CMD_TYPE_NONE, 0, (cmd_f)cmd_loc_x,  0 },
	{ LOC_Y,  CMD_TYPE_NONE, 0, (cmd_f)cmd_loc_y,  0 },
	{ DAMAGE, CMD_TYPE_NONE, 0, (cmd_f)cmd_damage, 0 },
	{ SPEED,  CMD_TYPE_NONE, 0, (cmd_f)cmd_speed,  0 },
	{ ELAPSED,CMD_TYPE_NONE, 0, (cmd_f)cmd_elapsed,0 },
	{ GET_ALL,CMD_TYPE_NONE, 0, (cmd_f)cmd_get_all,CMD_FLAG_MULTI },
	{ NAME,   CMD_TYPE_STR,  1, (cmd_f)cmd_name,   CMD_FLAG_PRESTART },
	{ IMAGE,  CMD_TYPE_INT,  1, (cmd_f)cmd_image,  CMD_FLAG_PRESTART | CMD_FLAG_DATA },
};

int cmdn = sizeof(cmds) / sizeof(cmd_t);

int cmd_start(struct robot *robot, int *args)
{
	return (timerisset(&game_start) ? 1 : -RES_FLAG_BLOCK);
}

int cmd_cycle(struct robot *robot, int *args)
{
	return !!timerisset(&game_start);
}

int cmd_scan(struct robot *robot, int *args)
{
	return scan(robot, args[0], args[1]);
}

int cmd_cannon(struct robot *robot, int *args)
{
	return cannon(robot, args[0], args[1]);
}

int cmd_loc_x(struct robot *robot, int *args)
{
	return loc_x(robot);
}

int cmd_loc_y(struct robot *robot, int *args)
{
	return loc_y(robot);
}

int cmd_damage(struct robot *robot, int *args)
{
	return damage(robot);
}

int cmd_speed(struct robot *robot, int *args)
{
	return speed(robot);
}

int cmd_elapsed(struct robot *robot, int *args)
{
	return current_cycles;
}

int cmd_get_all(struct robot *robot, int *args)
{
	multivalue[0] = loc_x(robot);
	multivalue[1] = loc_y(robot);
	multivalue[2] = damage(robot);
	multivalue[3] = speed(robot);
	multivalue[4] = current_cycles;
	return 5;
}

int cmd_drive(struct robot *robot, int *args)
{
	return drive(robot, args[0], args[1]);
}

int cmd_name(struct robot *robot, char **args)
{
	char *name;

	name = strndup(args[0], MAX_NAME_LEN);
	if (robot->name)
		free(robot->name);
	robot->name = name;
	return 1;
}

int cmd_image(struct robot *robot, int *args)
{
	if (*args <= 0 || *args > MAX_IMAGE_BYTES)
		return -1;
	return *args;
}

result_t execute_cmd(struct robot *robot, char *input, int phase)
{
	int i;
	int cmd_id;
	cmd_t *cmd = NULL;
	char *argv[MAX_ARGS];
	int args_int[MAX_ARGS];
	void *args_cmd;
	result_t res;

	res.flags = RES_FLAG_ERROR;

	input = get_command(input, &cmd_id);
	if (cmd_id < 0) {
		res.result = ERR_PARSE;
		return res;
	}
	for (i = 0; i < cmdn; i++)
		if (cmds[i].cmd == cmd_id) {
			cmd = &cmds[i];
			break;
		}
	if (!cmd) {
		res.result = ERR_UNKNOWN;
		return res;
	}

	assert(cmd->args <= MAX_ARGS);
	if (tokenize_args(input, cmd->args, argv) != cmd->args) {
		res.result = ERR_ARGS;
		return res;
	}

	if (phase == 0 && !(cmd->flags & CMD_FLAG_PRESTART)) {
		res.result = ERR_DENIED;
		return res;
	}

	switch (cmd->type) {
	case CMD_TYPE_INT:
		for (i = 0; i < cmd->args; i++) {
			if (!str_isnumber(argv[i])) {
				res.result = ERR_ARGS;
				return res;
			}
			args_int[i] = atoi(argv[i]);
		}
		args_cmd = args_int;
		break;
	case CMD_TYPE_STR:
		args_cmd = argv;
		break;
	case CMD_TYPE_NONE:
	default:
		args_cmd = NULL;
		break;
	}

	res.flags = 0;
	res.result = cmd->func(robot, args_cmd);
	if (res.result < 0) {
		res.flags = -res.result;
		res.result = 0;
	}
	if (cmd->flags & CMD_FLAG_CYCLE)
		res.flags |= RES_FLAG_CYCLE;
	if (cmd->flags & CMD_FLAG_DATA)
		res.flags |= RES_FLAG_DATA;
	if (cmd->flags & CMD_FLAG_MULTI)
		res.flags |= RES_FLAG_MULTI;
	return res;
}
