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

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>

#include "netrobots.h"
#include "net_utils.h"
#include "net_command_list.h"

#define STD_RESP_LEN		64
#define DEFAULT_REMOTEHOST	"localhost"
#define DEFAULT_PORT		"4300"

/* server config */
int game_type = -1;
int shot_speed = -1;
int max_cycles = -1;

static int serverfd;

void start(void);
void set_name(char *name);
int image(char *path);

static int get_resp(int *resp, int count)
{
	char buf[STD_RESP_LEN + 1], *tmp;
	char **argv;
	int len, cmd, i;

	len = read(serverfd, buf, STD_RESP_LEN);

	if (len > 0) {
		buf[len] = '\0';
		tmp = get_command(buf, &cmd);
		switch (cmd) {
		case DEAD:
			printf_die(stdout, "Killed: %s\n", 1, *tmp ? tmp : "unknown reason");
			break;
		case OK:
			break;
		case ERROR:
			printf_die(stderr, "Error: %s\n", 2, *tmp ? tmp : "unknown error");
			break;
		default:
			printf_die(stderr, "Error: unexpected response from the server\n", 2);
			break;
		}

		argv = malloc(count * sizeof(char *));
		count = tokenize_args(tmp, count, argv);
		for (i = 0; i < count; i++) {
			if (str_isnumber(argv[i]))
				resp[i] = atoi(argv[i]);
			else
				resp[i] = -1;
		}
		free(argv);
	} else
		count = 0;
	return count;
}

static int get_resp_value(int ret)
{
	int value = -1;

	get_resp(&value, 1);
	if (ret == -1)
		printf_die(stdout, "Server dead or you have been killed\n", 1);
	return value;
}

static void receive_config(void)
{
	int config[4], count;

	count = get_resp(config, 4);
	if (count >= 1)
		game_type = config[0];
	if (count >= 2)
		shot_speed = config[1];
	if (count >= 3)
		max_cycles = config[2];
}

static int client_init(char *remotehost, char *port)
{
	int ret, sock = -1;
	struct addrinfo *ai, *runp;
	struct addrinfo hints;

	memset(&hints, '\0', sizeof(hints));
	hints.ai_flags = AI_ADDRCONFIG;
	hints.ai_socktype = SOCK_STREAM;

	ret = getaddrinfo(remotehost, port, &hints, &ai);
	if (ret)
		printf_die(stderr, "[ERROR] getaddrinfo('%s', '%s'): %s\n",
			   EXIT_FAILURE, remotehost, port, gai_strerror(ret));
	if (!ai)
		printf_die(stderr, "[ERROR] getaddrinfo(): no address returned\n",
			   EXIT_FAILURE);

	runp = ai;
	while (runp) {
		sock = socket(runp->ai_family, runp->ai_socktype, runp->ai_protocol);
		if (sock != -1) {
			ndprintf(stdout, "[NETWORK] Connecting to server...\n");
			if (connect(sock, runp->ai_addr, runp->ai_addrlen) == 0) {
				ndprintf(stdout, "[NETWORK] Connected.\n");
				serverfd = sock;
				receive_config();
				break;
			}
			close(sock);
			sock = -1;
		}
		runp = runp->ai_next;
	}
	freeaddrinfo(ai);
	return (sock < 0);
}

static void set_default_name(char *argv0)
{
	char *start, *name;
	int len;

	start = strrchr(argv0, '/');
	if (!start)
		start = argv0;
	else
		start++;
	len = strlen(start);
	name = malloc(len);
	if (!name)
		return;
	strcpy(name, start);
	if (*name)
		*name = toupper(*name);
	start = strrchr(name, '.');
	if (start && (len - (start - name)) <= 4)
		/* delete extension if there is any */
		*start = '\0';
	set_name(name);
	free(name);
}

void usage(char *prog, int retval)
{
	printf("Usage: %s [-H <hostname>] [-P <port>] [-d]\n"
	       "\t-H <remotehost>\tSpecifies hostname (Default: 127.0.0.1)\n"
	       "\t-P <port>\tSpecifies port (Default: 4300)\n"
	       "\t-d\tEnables debug mode\n", prog);
	exit(retval);
}

void init_custom(char *img_path, int argc, char **argv)
{
	int retval;
	char *remotehost = DEFAULT_REMOTEHOST;
	char *port = DEFAULT_PORT;

	while ((retval = getopt(argc, argv, "dn:hH:P:")) != -1) {
		switch (retval) {
		case 'H':
			remotehost = optarg;
			break;
		case 'P':
			port = optarg;
			break;
		case 'd':
			debug = 1;
			break;
		case 'h':
			usage(argv[0], EXIT_SUCCESS);
			break;
		default:
			break;
		}
	}

	if (argc > optind)
		usage(argv[0], EXIT_FAILURE);

	signal(SIGPIPE, SIG_IGN);
	if (client_init(remotehost, port))
		printf_die(stderr, "Could not connect to %s:%s\n",
			   EXIT_FAILURE, remotehost, port);

	srandom(time(NULL) + getpid());
	srand(time(NULL) + getpid());

	set_default_name(argv[0]);
	if (img_path)
		image(img_path);
	start();
}

void init(int argc, char **argv)
{
	init_custom(NULL, argc, argv);
}

void start(void)
{
	int ret;

	ret = sockwrite(serverfd, START, NULL);
	get_resp_value(ret);
}

int scan(int degree, int resolution)
{
	int ret;

	ret = sockwrite(serverfd, SCAN, "%d %d", degree, resolution);
	return get_resp_value(ret);
}

int cannon(int degree, int range)
{
	int ret;

	ret = sockwrite(serverfd, CANNON, "%d %d", degree, range);
	return get_resp_value(ret);
}

void drive(int degree, int speed)
{
	int ret;

	ret = sockwrite(serverfd, DRIVE,  "%d %d", degree, speed);
	get_resp_value(ret);
}

int damage(void)
{
	int ret;

	ret = sockwrite(serverfd, DAMAGE, NULL);
	return get_resp_value(ret);
}

void cycle(void)
{
	int ret;

	ret = sockwrite(serverfd, CYCLE, NULL);
	get_resp_value(ret);
}

int speed(void)
{
	int ret;

	ret = sockwrite(serverfd, SPEED, NULL);
	return get_resp_value(ret);
}

int loc_x(void)
{
	int ret;

	ret = sockwrite(serverfd, LOC_X, NULL);
	return get_resp_value(ret);
}

int loc_y(void)
{
	int ret;

	ret = sockwrite(serverfd, LOC_Y, NULL);
	return get_resp_value(ret);
}

int elapsed(void)
{
	int ret;

	ret = sockwrite(serverfd, ELAPSED, NULL);
	return get_resp_value(ret);
}

void get_all(int *_loc_x, int *_loc_y, int *_damage, int *_speed, int *_elapsed)
{
	int ret, count;
	int values[5];

	ret = sockwrite(serverfd, GET_ALL, NULL);
	count = get_resp(values, 5);
	if (ret == -1 || count != 5)
		printf_die(stdout, "Server dead or you have been killed\n", 1);
	if (_loc_x)
		*_loc_x = values[0];
	if (_loc_y)
		*_loc_y = values[1];
	if (_damage)
		*_damage = values[2];
	if (_speed)
		*_speed = values[3];
	if (_elapsed)
		*_elapsed= values[4];
}

void set_name(char *name)
{
	int ret;

	ret = sockwrite(serverfd, NAME, "%s", name);
	get_resp_value(ret);
}

int image(char *path)
{
	FILE *f;
	void *data;
	long fsize;
	int ret = 0;

	f = fopen(path, "rb");
	if (!f) {
		ndprintf(stderr, "[WARNING] Cannot find file %s\n", path);
		return 0;
	}
	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	if (fsize < 0 || fsize > MAX_IMAGE_BYTES) {
		ndprintf(stderr, "[WARNING] File %s longer than %d bytes\n",
			 path, MAX_IMAGE_BYTES);
		goto out_close;
	}
	rewind(f);
	data = malloc(fsize);
	if (!data) {
		ndprintf(stderr, "[WARNING] Cannot allocate memory\n");
		goto out_close;
	}
	if (fread(data, 1, fsize, f) != (size_t)fsize) {
		ndprintf(stderr, "[WARNING] Error reading from %s\n", path);
		goto out_free;
	}
	ret = sockwrite(serverfd, IMAGE, "%ld", fsize);
	get_resp_value(ret);
	ret = write(serverfd, data, fsize);
	get_resp_value(ret);
out_free:
	free(data);
out_close:
	fclose(f);
	return ret;
}
