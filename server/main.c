/* Main program.
   Paolo Bonzini, August 2008.

   This source code is released for free distribution under the terms
   of the GNU General Public License.  */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#include "toolkit.h"
#include "field.h"
#include "net_defines.h"

int
main (int argc, char **argv)
{
  unsigned int i = 0;
  struct timeval start_ticks, end_ticks;
  int phase = 0;
  event_t event;

  cairo_t *cairo_context;
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
  printf ("%.2f fps\n", (i * 1000.0) / (1000 * (end_ticks.tv_sec - start_ticks.tv_sec) +
			(end_ticks.tv_usec - start_ticks.tv_usec) / 1000));

  /* clear resources before exit */
  destroy_cairo();

  return 0;
}
