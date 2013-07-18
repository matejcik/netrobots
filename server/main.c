/*
 * netrobots, main program
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
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "toolkit.h"
#include "field.h"
#include "net_defines.h"

int main(int argc, char **argv)
{
	unsigned int i = 0;
	struct timeval start_ticks, end_ticks;
	int phase = 0;
	event_t event;

	srandom(time(NULL) + getpid());
	server_init(argc, argv);
	init_cairo(&argc, &argv);

	gettimeofday(&start_ticks, NULL);

	/* enter event-loop */
	for (;;) {
		i++;
		event = process_cairo();
		if (event == EVENT_QUIT)
			break;

		/* Call functions here to parse event and render on cairo_context...  */
		switch (phase) {
		case 0:
			/* connecting */
			if (server_process_connections(event))
				phase++;
			break;
		case 1:
			/* game in progress */
			if (server_cycle(event)) {
				complete_ranking();
				phase++;
			}
			break;
		case 2:
			/* display ranking */
			server_finished_cycle(event);
			break;
		}
	}

	gettimeofday(&end_ticks, NULL);
	printf("%.2f fps\n",
	       (i * 1000.0) / (1000 * (end_ticks.tv_sec - start_ticks.tv_sec) +
			       (end_ticks.tv_usec - start_ticks.tv_usec) / 1000));

	/* clear resources before exit */
	destroy_cairo();

	return 0;
}
