/* Count.c */

/* Count all the colours used in an animation
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 *
 * History:
 *      23-Aug-96 pdh Split off from gif.c, modularised
 *      25-Aug-96 *** Release 3.00
 *      01-Sep-96 *** Release 3.01
 *      07-Sep-96 pdh Allow for 3-, 5-, 6-, 7-bit GIFs
 *      21-Oct-96 pdh Allow for autotransparency by letting mask inc #colours
 *      27-Oct-96 *** Release 4beta1
 *      29-Oct-96 *** Release 4beta2
 *      07-Nov-96 *** Release 4
 *      15-Dec-96 *** Release 5beta1
 *      27-Jan-97 *** Release 5beta2
 *      29-Jan-97 *** Release 5beta3
 *      03-Feb-97 *** Release 5
 *      07-Feb-97 *** Release 5.01
 *      07-Apr-97 *** Release 6beta1
 *      20-May-97 *** Release 6beta2
 *      24-Aug-97 *** Release 6
 *      27-Sep-97 *** Release 6.01
 *      08-Nov-97 *** Release 6.02
 *      21-Feb-98 *** Release 6.03
 *      07-Jun-98 *** Release 6.04
 *      21-Aug-98 *** Release 6.05
 *      05-Oct-98 *** Release 6.06
 *      19-Feb-99 *** Release 6.07
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "animlib.h"

#include "count.h"

#if 0
#define debugf printf
#define DEBUG 1
#define DBTOT int total = 0
#define DBINC total++
#else
#define debugf 1?0:printf
#define DEBUG 0
#define DBTOT
#define DBINC
#endif

#if 0
static int scanmap( char *used, char *map );
#endif

#if 0
/*---------------------------------------------------------------------------*
 * CountColours                                                              *
 * Count the total number of colours used in the animation                   *
 *---------------------------------------------------------------------------*/

int CountColours( anim a, char *used, char *map, BOOL bUseMask )
{
    int i, j;
    const int len = a->nWidth * a->nHeight;
    char *ptr;
    char *pMask;
    char frameused[256];
    int nUsedOnFrame;
    int nMostOnFrame = 0;
    BOOL bFrameTrans;

    /* 1) set map entries to 1 for all colours present (0 otherwise)
     */

    memset( map, 0, 256 );
    memset( used, 0, 256 );

    for ( i=0; i < a->nFrames; i++ )
    {
        frameptr f;

        f = Anim_OpenFrame( a, i );

        memset( frameused, 0, 256 );

        bFrameTrans = FALSE;

        ptr = f->pImage;
        pMask = bUseMask ? f->pMask : NULL;

        if ( pMask )
        {
            for ( j=0; j < len; j++ )
            {
                if ( *pMask )
                {
                    used[*ptr] = 1;
                    frameused[*ptr] = 1;
                }
                else
                    bFrameTrans = TRUE;

                ptr++;
                pMask++;
            }
        }
        else
        {
            for ( j=0; j < len; j++ )
            {
                used[*ptr] = 1;
                ptr++;
            }
        }

        nUsedOnFrame = 0;

        for ( j=0; j<256; j++ )
            if ( frameused[j] )
                nUsedOnFrame++;

        if ( bFrameTrans || i>0 )
            nUsedOnFrame++;

        if ( nUsedOnFrame > nMostOnFrame )
            nMostOnFrame = nUsedOnFrame;

        Anim_CloseFrame( a, &f );
    }

    nUsedOnFrame = scanmap( used, map );

    if ( nUsedOnFrame > nMostOnFrame )
        nMostOnFrame = nUsedOnFrame;

    return nMostOnFrame;
}
#endif

#if 0
static int scanmap( char *used, char *map )
{
    int newncol, x, j;

    /* 2) then go through assigning destination colours in order
     */

    newncol = 0;
    for ( x=0; x<256; x++ )
    {
        if ( used[x] )
        {
            map[x] = newncol;
            newncol++;
        }
    }

    j = newncol;

    for ( x=0; x<256; x++ )
    {
        if ( used[x] == 0 )
            map[x] = j++;
    }

    return newncol;
}
#endif

int MinBpp( int newncol )
{
    int newbpp;

    /* 3) calculate minimum bpp that does it
     */

    newbpp = 0;
    newncol--;

    do
    {
        newbpp++;
        newncol >>= 1;
    } while ( newncol );

    return newbpp;
}


#if 0
int CountColoursRect( anim a, frameptr f, rect *r, char *used, char *map )
{
    char *pLine;
    int x,y;
    int xoff = r->xoff;
    int yoff = r->yoff;

    if ( map )
        memset( map, 0, 256 );
    memset( used, 0, 256 );

    for ( y=0; y < r->ysize; y++ )
    {
        pLine = f->pImage + ((y+yoff) * a->nWidth);
        for ( x=0; x < r->xsize; x++ )
        {
            used[pLine[x + xoff]] = 1;
        }
    }

#if DEBUG
    {
        int n=0;
        debugf( "Used: <" );
        for ( x=0; x<256; x++ )
        {
            if ( used[x] )
            {
                debugf( n ? " %d" : "%d", x );
                n++;
            }
        }
        debugf( ">=%d\n", n );
    }
#endif

    return map ? scanmap( used, map ) : 0;
}
#endif
