/*
 * netrobots, networking module
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

#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <assert.h>

#include "net_utils.h"
#include "net_defines.h"
#include "robotserver.h"
#include "toolkit.h"
#include "field.h"

#define STD_CYCLES	10000
#define STD_HOSTNAME	NULL
#define STD_PORT	"4300"

int autostart_robots = 0;
struct pollfd *fds;
int sockd;

extern int debug;

int save_results = 0;
double shot_speed = DEF_SHOT_SPEED;
int shot_reload = 0;

int arena_mode = 0;

int max_robots = 0;
int dead_robots;

game_type_t game_type = GAME_SCORE;

int max_cycles;
int current_cycles = 0;

int max_passive_cycles = 0;
int current_passive_cycles = 0;

struct robot **all_robots;
struct robot **ranking;

struct timeval game_start;
struct timeval game_end;

static volatile int timer;

static char *err_desc[__ERR_MAX] = {
	"Unspecified error",
	"Unparsable command",
	"Unknown command",
	"Wrong argument type or count",
	"Command not allowed",
};

float get_color_component(unsigned n, unsigned shade)
{
	return (float)((2 - shade) * n) / 2;
}

void generate_color(float *color)
{
	static unsigned mask = 0;
	static unsigned value = 0;
	unsigned m, v;
	int i;

	do {
		if (++value > 7) {
			value = 1;
			mask++;
		}
	} while ((value & mask) != mask || (value ^ mask) == 7);
	m = mask;
	v = value;
	for (i = 0; i < 3; i++) {
		color[i] = get_color_component(v & 1, m & 1);
		m >>= 1;
		v >>= 1;
	}
}

int create_client(int fd)
{
	struct robot *r;
	static int quad = 0;

	if (fd == -1)
		return 0;

	r = malloc(sizeof(struct robot));
	if (!r)
		return 0;
	memset (r, 0, sizeof (*r));

	/* place each robot in a different quadrant.  */
	r->x = ((quad & 1) ? 0 : 500) + 500 * (random() / (double)RAND_MAX);
	r->y = ((quad & 2) ? 0 : 500) + 500 * (random() / (double)RAND_MAX);
	quad++;

	generate_color(r->color);

	r->life_length.tv_sec = -1;

	fds[max_robots].fd = fd;
	all_robots[max_robots++] = r;
	return 1;
}

int try_accept(int allow)
{
	int fd;
	struct sockaddr *addr;
	socklen_t addrlen = sizeof(addr);

	fd = accept(sockd, (struct sockaddr *) &addr, &addrlen);
	if (fd >= 0) {
		if (max_robots == MAX_ROBOTS || !allow) {
			ndprintf(stdout, "[INFO] Ignoring excessive connection\n");
			close(fd);
		} else {
			if (!create_client(fd))
				sockwrite(fd, ERROR, "Internal error");
			else {
				sockwrite(fd, OK, "%d %d %d", game_type,
					  (int)(shot_speed / SPEED_RATIO), max_cycles);
				return 1;
			}

		}
	}
	return 0;
}

void raise_timer(int sig)
{
	timer = 1;
}

void close_kill_robot(struct pollfd *pfd, struct robot *robot)
{
	close(pfd->fd);
	pfd->fd = -1;
	kill_robot(robot);
}

int process_robots(int phase)
{
	int i, ret;
	struct pollfd *pfd;
	result_t result;
	char buf[STD_BUF];
	struct robot *robot;
	int to_talk;

	do {
		for (i = 0; i < max_robots; i++)
			fds[i].events = POLLIN | POLLOUT;

		fds[max_robots].fd = sockd;
		fds[max_robots].events = POLLIN;

		to_talk = 0;
		for (i = 0; i < max_robots; i++) {
			if (fds[i].fd != -1)
				to_talk++;
		}

		if (phase == 1) {
			if (to_talk <= 1)
				return 1;
		}

		poll(fds, max_robots + 1, 10);

		for (i = 0; i < max_robots; i++) {
			robot = all_robots[i];
			pfd = &fds[i];
			if (pfd->fd == -1)
				/* dead robot */
				continue;
			if (robot->waiting) {
				to_talk--;
				continue;
			}
			if (pfd->events == 0) {
				to_talk--;
				continue;
			}

			if (pfd->revents & (POLLERR | POLLHUP)) {
				/* error or disconnected robot -> kill */
				close_kill_robot(pfd, robot);
				continue;
			}
			if (robot->damage >= 100) {
				sockwrite(pfd->fd, DEAD, "You're dead");
				close_kill_robot(pfd, robot);
				continue;
			}

			if (!(pfd->revents & POLLIN))
				continue;

			if (!(pfd->revents & POLLOUT)) {
				close_kill_robot(pfd, robot);
				continue;
			}

			ret = read(pfd->fd, buf, STD_BUF);
			if (ret <= 0) {
				close_kill_robot(pfd, robot);
				continue;
			}
			if (robot->data) {
				/* read picture data */
				if (robot->data_ptr + ret > robot->data_len) {
					close_kill_robot(pfd, robot);
					continue;
				}
				memcpy(robot->data + robot->data_ptr, buf, ret);
				robot->data_ptr += ret;
				if (robot->data_ptr == robot->data_len)
					sockwrite(pfd->fd, OK, "%d", load_image(robot));
				continue;
			}
			buf[ret] = '\0';

			result = execute_cmd(robot, buf, phase);
			if (result.flags & RES_FLAG_ERROR) {
				assert(result.result >= 0 && result.result < __ERR_MAX);
				sockwrite(pfd->fd, ERROR, "%s", err_desc[result.result]);
				close_kill_robot(pfd, robot);
				continue;
			}
			if (result.flags & RES_FLAG_BLOCK) {
				robot->waiting = 1;
				continue;
			}

			if (result.flags & RES_FLAG_CYCLE)
				pfd->events = 0;

			if (result.flags & RES_FLAG_MULTI) {
				sockwrite_ints(pfd->fd, OK, multivalue, result.result);
				continue;
			}

			if (result.flags & RES_FLAG_DATA) {
				assert(!robot->data);
				robot->data_len = result.result;
				robot->data_ptr = 0;
				robot->data = malloc(robot->data_len);
				if (!robot->data) {
					sockwrite(pfd->fd, ERROR, "Internal error");
					close_kill_robot(pfd, robot);
					continue;
				}
			}
			sockwrite(pfd->fd, OK, "%d", result.result);
		}

		if (fds[max_robots].revents) try_accept(arena_mode || phase == 0);
	} while ((to_talk || phase != 1) && !timer);
	return 0;
}

void server_start(char *hostname, char *port)
{
	int ret, opt = 1;
	struct addrinfo *ai, *runp, hints;

	memset(&hints, 0x0, sizeof (hints));
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
	hints.ai_socktype = SOCK_STREAM;

	ndprintf(stdout, "[SERVER] Starting Server at %s:%s\n", hostname, port);
	if (autostart_robots > 0)
		ndprintf(stdout, "[INFO] Number of players: %d\n", autostart_robots);

	if ((ret = getaddrinfo(hostname, port, &hints, &ai)))
		ndprintf_die(stderr, "[ERROR] getaddrinfo('%s', '%s'): %s\n",
			     hostname, port, gai_strerror(ret));
	if (!ai)
		ndprintf_die(stderr, "[ERROR] getaddrinf(): couldn't fill the struct!\n");

	runp = ai;
	do {
		sockd = socket(runp->ai_family, runp->ai_socktype, runp->ai_protocol);
		if (sockd != -1)
			break;
		runp = runp->ai_next;
	} while (runp);
	if (sockd == -1)
		ndprintf_die(stderr, "[ERROR] socket(): Couldn't create socket!\n");

	/* To close the port after closing the socket */
	if (setsockopt(sockd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt)) == -1)
		ndprintf_die(stderr, "[ERROR] setsockopt(): %s\n", strerror(errno));
	if (fcntl(sockd, F_SETFL, O_NONBLOCK))
		ndprintf_die(stderr, "[ERROR] fcntl(): %s\n", strerror(errno));
	if (bind(sockd, runp->ai_addr, runp->ai_addrlen))
		ndprintf_die(stderr, "[ERROR] bind(): %s\n", strerror(errno));
	if (listen(sockd, MAX_ROBOTS))
		ndprintf_die(stderr, "[ERROR] listen(): %s\n", strerror(errno));
	if (!(fds = (struct pollfd *) malloc ((MAX_ROBOTS + 1) * sizeof(struct pollfd))))
		ndprintf_die(stderr, "[ERROR] Coulnd't malloc space for fds!\n");
}

void charge_timer(void)
{
	struct itimerval itv;
	itv.it_interval.tv_sec = 0;
	itv.it_interval.tv_usec = 0;
	itv.it_value.tv_sec = 0;
	itv.it_value.tv_usec = 10000;
	setitimer (ITIMER_REAL, &itv, NULL);
	timer = 0;
}

int server_process_connections(event_t event)
{
	int i;
	int res = 0;

	if (max_passive_cycles && current_passive_cycles >= max_passive_cycles)
		ndprintf_die(stdout, "[INFO] Terminated on client connection timeout\n");
	current_passive_cycles++;

	charge_timer();
	update_display();
	update_display_msg("Waiting for connections");
	process_robots(0);

	/* TODO: This is horrible. We should just poll for an incoming
	 * connection, data on existing fd, a keypress, or X event, not
	 * looping like hell. */
	//try_accept();
	if (autostart_robots > 0 && max_robots == autostart_robots)
		res = 1;

	if (event == EVENT_START)
		res = 1;

	if (res) {
		ndprintf(stdout, "[GAME] Starting. All clients connected!\n");
		/* notify all robots that are passively waiting for game start */
		for (i = 0; i < max_robots; i++) {
			if (!all_robots[i]->waiting)
				continue;
			all_robots[i]->waiting = 0;
			sockwrite(fds[i].fd, OK, "1");
		}

		gettimeofday(&game_start, NULL);
		current_passive_cycles = 0;
	}
	return res;
}

int server_cycle(event_t event)
{
	int res;

	if (current_cycles >= max_cycles) {
		ndprintf(stdout, "[GAME] Ended - Draw!\n");
		res = 1;
	} else {
		current_cycles++;
		charge_timer();
		cycle();
		update_display();
		res = process_robots(1);
	}
	if (res) {
		gettimeofday(&game_end, NULL);
		complete_ranking();
	}
	return res;
}

int server_finished_cycle(event_t event)
{
	if (max_passive_cycles && current_passive_cycles >= max_passive_cycles)
		return 1;
	current_passive_cycles++;
	charge_timer();
	cycle();
	update_display();
	update_display_results();
	process_robots(2);
	return 0;
}

void usage(char *prog, int retval)
{
	printf("Usage %s [OPTIONS]\n"
	       "\t-n <clients>\tNumber of clients to start the game, 0 for dynamic (Default: 0)\n"
	       "\t-H <hostname>\tSpecifies hostname (Default: 127.0.0.1)\n"
	       "\t-P <port>\tSpecifies port (Default: 4300)\n"
	       "\t-c <cycles>\tMaximum length of the game (Default: 10000)\n"
	       "\t-C <cycles>\tLimit init and results time (Default: 0 = unlimited)\n"
	       "\t-m <speed>\tMissiles speed, 0 for laser game (Default: 400)\n"
	       "\t-l\tLaser game, shorthand for -m 0\n"
	       "\t-t\tTime based game (Default: score based game)\n"
	       "\t-s\tSave results to ./results.txt\n"
	       "\t-d\tEnables debug mode\n"
	       "\t-a\tArena mode (clients can connect anytime, dead bots respawn)\n"
		, prog);
	exit(retval);
}

int server_init(int argc, char *argv[])
{
	int retval;

	char *port = STD_PORT;
	char *hostname = STD_HOSTNAME;

	while ((retval = getopt(argc, argv, "dn:hH:P:c:C:m:ltsa")) != -1) {
		switch (retval) {
		case 'a':
			arena_mode = 1;
			break;
		case 'c':
			max_cycles = atoi(optarg);
			break;
		case 'C':
			max_passive_cycles = atoi(optarg);
			break;
		case 'H':
			hostname = optarg;
			break;
		case 'P':
			port = optarg;
			break;
		case 'd':
			debug = 1;
			break;
		case 'n':
			autostart_robots = atoi(optarg);
			break;
		case 'm':
			shot_speed = atoi(optarg) * SPEED_RATIO;
			break;
		case 'l':
			shot_speed = 0;
			break;
		case 't':
			game_type = GAME_TIME;
			break;
		case 's':
			save_results = 1;
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

	if (autostart_robots > MAX_ROBOTS)
		autostart_robots = MAX_ROBOTS;

	if (max_cycles <= 1)
		max_cycles = STD_CYCLES;

	if (shot_speed == 0)
		shot_reload = SHOT_RELOAD;

	all_robots = malloc(MAX_ROBOTS * sizeof(struct robot *));
	ranking = malloc(MAX_ROBOTS * sizeof(struct robot *));
	dead_robots = 0;

	server_start(hostname, port);
	signal(SIGALRM, raise_timer);
	return 0;
}
