/* Interface between Cairo and a toolkit.
   Paolo Bonzini, August 2008.

   This source code is released for free distribution under the terms
   of the GNU General Public License.  */

#ifndef DRAWING_H
#define DRAWING_H

#include <cairo.h>

#define WIN_HEIGHT	540
#define WIN_WIDTH	(WIN_HEIGHT + 140)
#define WIN_TITLE	"Netrobots Battlefield"

typedef enum event_t {
	EVENT_NONE,
	EVENT_QUIT,
	EVENT_START,
} event_t;

/* Functions used by main.c as a high-level interface with the toolkit.  */

cairo_t *init_toolkit(int *argc, char ***argv);
void free_toolkit(cairo_t *cr);
event_t process_toolkit(cairo_t **cr);
cairo_surface_t *toolkit_load_image(char *path);

#endif
