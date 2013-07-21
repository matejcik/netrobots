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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "net_utils.h"

int debug = 0;

/* Returns pointer to the rest of the string (i.e., start of the arguments). */
char *get_command(char *str, int *command)
{
	*command = -1;
	if (!*str)
		return str;
	*command = *str++;
	while (isspace(*str))
		str++;
	return str;
}

/* Expects argv to be allocated to hold argc pointers. Returns number of
 * found arguments. Note that this function rewrites the source string. */
int tokenize_args(char *str, int argc, char *argv[])
{
	int cnt = 0;
	char *ptr = str;

	if (!argc)
		return 0;
	/* strtok is not thread safe, strtok_r is POSIX only, we have to
	 * tokenize ourselves */
	while (*ptr && cnt < argc - 1) {
		if (isspace(*ptr)) {
			*ptr = '\0';
			argv[cnt++] = str;
			while (isspace(*++ptr))
				;
			str = ptr;
			continue;
		}
		ptr++;
	}
	if (*str)
		argv[cnt++] = str;
	return cnt;
}

void ndprintf(FILE *fd, char *fmt, ...)
{
	va_list vp;

	if (debug) {
		va_start (vp, fmt);
		vfprintf(fd, fmt, vp);
		va_end(vp);
	}
}

void ndprintf_die(FILE *fd, char *fmt, ...)
{
	va_list vp;

	if (debug) {
		va_start(vp, fmt);
		vfprintf(fd, fmt, vp);
		va_end(vp);
	}
	exit(EXIT_FAILURE);
}

void printf_die(FILE *fd, char *fmt, int err, ...)
{
	va_list vp;

	va_start(vp, err);
	vfprintf(fd, fmt, vp);
	va_end(vp);
	exit(err);
}

int sockwrite(int fd, char command, char *fmt, ...)
{
	char *str;
	int ret;
	va_list vp;

	str = malloc(STD_BUF);
	if (!str)
		return -1;
	str[0] = command;
	str[1] = '\0';
	if (fmt) {
		str[1] = ' ';
		va_start(vp, fmt);
		vsnprintf(str + 2, STD_BUF - 2, fmt, vp);
		va_end(vp);
	}
	ret = write(fd, str, strlen(str));
	free(str);
	return ret;
}

int sockwrite_ints(int fd, char command, int *data, int count)
{
	char *str;
	int ret, pos, i;

	str = malloc(STD_BUF);
	if (!str)
		return -1;
	str[0] = command;
	pos = 1;
	for (i = 0; i < count && pos < STD_BUF; i++)
		pos += snprintf(str + pos, STD_BUF - pos, " %d", data[i]);
	ret = write(fd, str, strlen(str));
	free(str);
	return ret;
}

int str_isnumber(char *str)
{
	const int len = strlen(str);
	int i;

	for (i = 0; i < len; i++) {
		if (i == 0 && str[i] == '-')
			continue;
		if (!isdigit(str[i]))
			return 0;
	}
	return 1;
}
