/* InterGif.h */

/* Header file for central InterGif routine
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 */

#ifndef intergif_intergif_h
#define intergif_intergif_h

#ifndef animlib_h
#include "animlib.h"
#endif

#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif

/* Turn a file or files into a GIF. bSprite and palfile only works on RiscOS.
 * bDither doesn't work at all.
 */

BOOL InterGif( const char *infile, BOOL bJoin, BOOL bSplit, BOOL bSprite,
               anim_GIFflags flags, BOOL bForceTrans, pixel nForceTransPixel,
               BOOL bUseTrans, BOOL bTrim, BOOL b216, BOOL b256, BOOL bDither,
               const char *palfile, BOOL bSame, int nBest, const char *cfsi,
               const char *outfile );

#endif
