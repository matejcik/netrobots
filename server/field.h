/* Map drawing module interface.
   Paolo Bonzini, August 2008.

   This source code is released for free distribution under the terms
   of the GNU General Public License.  */

#ifndef MAP_H
#define MAP_H

/* Initialize the rendering of the map.  */
extern void init_cairo(int *argc, char ***argv);

/* Update the canvas */
extern void update_display(int finished);

#endif /* MAP_H */
