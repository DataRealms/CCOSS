/* Count.h */

/* Header file for colour counting
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 */

#ifndef animlib_h
#include "animlib.h"
#endif

/* These now return the number of colours ie candidate transparent colour
int CountColours( anim a, char *used, char *map, BOOL usemask );
int CountColoursRect( anim a, frameptr f, rect *r, char *used, char *map );
 */

int MinBpp( int nColours );
