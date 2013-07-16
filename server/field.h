/* Map drawing module interface.
   Paolo Bonzini, August 2008.

   This source code is released for free distribution under the terms
   of the GNU General Public License.  */

#ifndef MAP_H
#define MAP_H

#include "toolkit.h"

/* Initialize the rendering of the map.  */
void init_cairo(int *argc, char ***argv);
event_t process_cairo(void);

/* Update the canvas */
void update_display(int finished);

#endif
