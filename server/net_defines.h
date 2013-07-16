#ifndef NET_DEFINES_H
#define NET_DEFINES_H

#include <stdbool.h>

#include "robotserver.h"
#include "net_command_list.h"

typedef enum cmd_type_t {
	CMD_TYPE_INT,
	CMD_TYPE_STR,
} cmd_type_t;

typedef (*cmd_f)(struct robot *robot, void *args);

typedef struct cmd_t {
	cmd_f func;
	int args;
	cmd_type_t type;
	bool cycle;
	bool prestart;
} cmd_t;

typedef struct result_t {
	int result;
	bool error;
	bool cycle;
} result_t;

void init_server(char *hostname, char *port);
result_t execute_cmd(struct robot *robot, char *input, int phase);

#endif
