#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "robotserver.h"
#include "toolkit.h"
#include "net_utils.h"
#include "net_defines.h"

int cmd_start(struct robot *robot, int *args);
int cmd_cycle(struct robot *robot, int *args);
int cmd_cannon(struct robot *robot, int *args);
int cmd_scan(struct robot *robot, int *args);
int cmd_loc_x(struct robot *robot, int *args);
int cmd_loc_y(struct robot *robot, int *args);
int cmd_damage(struct robot *robot, int *args);
int cmd_speed(struct robot *robot, int *args);
int cmd_drive(struct robot *robot, int *args);
int cmd_name(struct robot *robot, char **args);
int cmd_image(struct robot *robot, char **args);

cmd_t cmds[] = {
	{ (cmd_f)cmd_start, 0, CMD_TYPE_INT, false, true },
	{ (cmd_f)cmd_cycle, 0, CMD_TYPE_INT, true, true },
	{ (cmd_f)cmd_cannon, 2, CMD_TYPE_INT, true, false },
	{ (cmd_f)cmd_scan, 2, CMD_TYPE_INT, true, false },
	{ (cmd_f)cmd_loc_x, 0, CMD_TYPE_INT, false, false },
	{ (cmd_f)cmd_loc_y, 0, CMD_TYPE_INT, false, false },
	{ (cmd_f)cmd_damage, 0, CMD_TYPE_INT, false, false },
	{ (cmd_f)cmd_speed, 0, CMD_TYPE_INT, false, false },
	{ (cmd_f)cmd_drive, 2, CMD_TYPE_INT, true, false },
	{ (cmd_f)cmd_name, 1, CMD_TYPE_STR, false, true },
	{ (cmd_f)cmd_image, 1, CMD_TYPE_STR, false, true },
};

int cmdn = sizeof(cmds) / sizeof(cmd_t);

result_t error_res = { -1, true, false };
result_t block_res = { 0, true, false };

int cmd_start(struct robot *robot, int *args)
{
	return (timerisset(&game_start) ? 1 : -2);
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

int cmd_drive(struct robot *robot, int *args)
{
	drive(robot, args[0], args[1]);
	return 1;
}

int cmd_name(struct robot *robot, char **args)
{
	char *name, *p;

	name = strndup(args[0], MAX_NAME_LEN);
	for (p = name; *p; p++)
		if (*p == '_')
			*p = ' ';
	if (robot->name)
		free(robot->name);
	robot->name = name;
	return 1;
}

int cmd_image(struct robot *robot, char **args)
{
	if (strchr(args[0], '/'))
		return -1;
	if (robot->img)
		cairo_surface_destroy(robot->img);
	robot->img = toolkit_load_image(args[0]);
	return !!robot->img;
}

result_t execute_cmd(struct robot *robot, char *input, int phase)
{
	char **argv;
	int argc, ret, i;
	void *args = NULL;
	cmd_t cmd;
	result_t res = error_res;

	argc = str_to_argv(input, &argv);
	if (argc == -1)
		return error_res;
	if (!argc || !str_isnumber(argv[0]))
		goto out;

	i = atoi(argv[0]);
	if (i < 0 || i >= cmdn)
		goto out;
	cmd = cmds[i];

	if (cmd.args != argc - 1)
		goto out;

	if (phase == 0 && !cmd.prestart)
		goto out;

	switch (cmd.type) {
	case CMD_TYPE_INT:
		if (!(args = malloc(cmd.args * sizeof(int))))
			goto out;

		for (i = 1; i < argc; i++) {
			if (!str_isnumber(argv[i]))
				goto out;
			((int *)args)[i - 1] = atoi(argv[i]);
		}
		break;
	case CMD_TYPE_STR:
		if (!(args = malloc(cmd.args * sizeof(char *))))
			goto out;
		for (i = 1; i < argc; i++)
			((char **)args)[i - 1] = argv[i];
		break;
	}

	ret = cmd.func(robot, args);
	ndprintf(stdout, "[COMMAND] %s -> %d recived - %g %g %d\n",
		 argv[0], ret, robot->x, robot->y, robot->damage);
	if (ret == -1)
		goto out;
	if (ret == -2) {
		/* special case: block the caller */
		res = block_res;
		goto out;
	}

	res.result = ret;
	res.cycle = cmd.cycle;
	res.error = false;

out:
	free(args);
	free(argv);
	return res;
}
