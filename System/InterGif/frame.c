/* Frame.c */

/* Operations on whole frames
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 *
 * History:
 *      23-Aug-96 pdh Created
 *      25-Aug-96 *** Release 3.00
 *      01-Sep-96 *** Release 3.01
 *      20-Oct-96 pdh PixMask and BitMask routines
 *      27-Oct-96 *** Release 4beta1
 *      29-Oct-96 *** Release 4beta2
 *      07-Nov-96 *** Release 4
 *      17-Nov-96 pdh Fix bmep and pmep to take account of prevframe's mask
 *      15-Dec-96 *** Release 5beta1
 *      01-Jan-97 pdh Add BitMaskClearRect, Rect_Union, others
 *      27-Jan-97 *** Release 5beta2
 *      29-Jan-97 *** Release 5beta3
 *      02-Feb-97 pdh Add PixMaskToBitMask, BitMaskClear
 *      03-Feb-97 *** Release 5
 *      07-Feb-97 *** Release 5.01
 *      08-Mar-97 pdh Frob for new anim library
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "animlib.h"
#include "utils.h"

#include "frame.h"

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

/*---------------------------------------------------------------------------*
 * slim                                                                      *
 * Given a buffer and size, return the offset and length of the portion      *
 * that's used (i.e. non-zero)                                               *
 *---------------------------------------------------------------------------*/

static void slim( unsigned int *pOff, unsigned int *pSize, char *buf, int max )
{
    int min = 0;

    while ( buf[max-1] == 0 && max>0 )
        max--;
    if ( max == 0 )
    {
        *pOff = 0;
        *pSize = 0;     /* was 1? */
        return;
    }
    while ( buf[min] == 0 )
        min++;
    *pOff = min;
    *pSize = max-min;
}


/*---------------------------------------------------------------------------*
 * Distance                                                                  *
 * Weighted distance between two colours (one already broken out)            *
 *---------------------------------------------------------------------------*/

#if 0
static int Distance( unsigned int col,
                     unsigned int r, unsigned int g, unsigned int b )
{
    unsigned int colr = (col>>8) & 0xFF;
    unsigned int colg = (col>>16) & 0xFF;
    unsigned int colb = (col>>24) & 0xFF;

    r = abs(colr-r);
    g = abs(colg-g);
    b = abs(colb-b);

    return (r*r*3)+(g*g*10)+(b*b);  /* see !ChangeFSI.FSIinfo */
}
#endif


/*---------------------------------------------------------------------------*
 * FindClosest                                                               *
 * Find closest match in a palette to a given colour                         *
 *---------------------------------------------------------------------------*/

#if 0
static int FindClosest( unsigned int *pPal, unsigned int col )
{
    unsigned int colr = (col>>8) & 0xFF;
    unsigned int colg = (col>>16) & 0xFF;
    unsigned int colb = (col>>24) & 0xFF;

    int result = 0;
    int closest = Distance( pPal[0], colr, colg, colb );
    int i, dist;

    for ( i=1; i<256; i++ )
    {
        dist = Distance( pPal[i], colr, colg, colb );
        if ( dist < closest )
        {
            closest = dist;
            result = i;
            if ( dist == 0 )
                break;
        }
    }
    return result;
}
#endif


/*---------------------------------------------------------------------------*
 * ChangedRect                                                               *
 * Given an anim structure and two frame pointers, return the smallest       *
 * rectangle which encloses all changes between the two frames               *
 *---------------------------------------------------------------------------*/

BOOL ChangedRect( anim a, const pixel *i1, const pixel *m1,
                  const pixel *i2, const pixel *m2, rect *pRect )
{
    char *xbuf, *ybuf;
    unsigned int x,y;

    xbuf = Anim_Allocate( a->nWidth );
    ybuf = Anim_Allocate( a->nHeight );

    if ( !xbuf || !ybuf )
    {
        Anim_Free( &xbuf );
        Anim_Free( &ybuf );
        Anim_NoMemory( "rect" );
        return FALSE;
    }

    /* Set xbuf[i] to 1 if anything in column i has changed,
     *     ybuf[j] to 1 if anything in column j has changed
     */

    memset( xbuf, 0, a->nWidth );
    memset( ybuf, 0, a->nHeight );

    for ( y=0; y < a->nHeight; y++ )
    {
        for ( x=0; x < a->nWidth; x++ )
        {
            if ( ( *i1 != *i2 )
                 || ( *m1 != *m2 ) )
            {
                xbuf[x] = 1;
                ybuf[y] = 1;
            }
            i1++;
            i2++;
            m1++;
            m2++;
        }
    }
    slim( &pRect->xoff, &pRect->xsize, xbuf, a->nWidth );
    slim( &pRect->yoff, &pRect->ysize, ybuf, a->nHeight );

    Anim_Free( &xbuf );
    Anim_Free( &ybuf );
    return TRUE;
}


/*---------------------------------------------------------------------------*
 * Rect_Union                                                                *
 * r1 := r1 {union} r2                                                       *
 *---------------------------------------------------------------------------*/

#ifndef MAX
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif
#ifndef MIN
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#endif

void Rect_Union( rect *r1, const rect *r2 )
{
    rect result;

    /* degenerate cases */
    if ( r2->xsize == 0 || r2->ysize == 0 )
        return;

    if ( r1->xsize == 0 || r1->ysize == 0 )
    {
        *r1 = *r2;
        return;
    }

    /* really do it */
    result.xoff = MIN( r1->xoff, r2->xoff );
    result.xsize = MAX( r1->xoff + r1->xsize, r2->xoff + r2->xsize )
                     - result.xoff;

    result.yoff = MIN( r1->yoff, r2->yoff );
    result.ysize = MAX( r1->yoff + r1->ysize, r2->yoff + r2->ysize )
                     - result.yoff;
    *r1 = result;
}


#if 0
/*---------------------------------------------------------------------------*
 * PixMaskCheck                                                              *
 * Given an anim structure and two frame pointers, return an indicator of    *
 * whether any pixels are solid in f2 but transparent in f1                  *
 *---------------------------------------------------------------------------*/

BOOL PixMaskCheck( anim a, frameptr f1, frameptr f2, int transpixel )
{
    int x,y;
    char *p1 = f1->pImage;
    char *p2 = f2->pImage;

    for ( y=0; y < a->nHeight; y++ )
    {
        for ( x=0; x < a->nWidth; x++ )
        {
            if ( *p1 == transpixel && *p2 != transpixel )
            {
                debugf( "bmc: trans at x=%d y=%d\n", x, y );
                return TRUE;
            }
            p1++;
            p2++;
        }
    }
    return FALSE;
}
#endif


#if 0
/*---------------------------------------------------------------------------*
 * BitMaskCheck                                                              *
 * Given an anim structure and two frame pointers, return an indicator of    *
 * whether any pixels are solid in f2 but transparent in f1                  *
 *---------------------------------------------------------------------------*/

BOOL BitMaskCheck( anim a, frameptr f1, frameptr f2, BOOL *pbMaskUsed )
{
    int x,y;
    char *p1 = f1->pMask;
    char *p2 = f2->pMask;
    BOOL bMaskUsed = FALSE;
    BOOL bBecomeTrans = FALSE;

    for ( y=0; y < a->nHeight; y++ )
    {
        for ( x=0; x < a->nWidth; x++ )
        {
            if ( *p1 == 0 && *p2 != 0 )
            {
                if ( !bBecomeTrans )
                    debugf( "bmc: trans at x=%d y=%d\n", x, y );
                bBecomeTrans = TRUE;
            }
            if ( *p1 == 0)
                bMaskUsed = TRUE;
            p1++;
            p2++;
        }
    }
    *pbMaskUsed = bMaskUsed;
    return bBecomeTrans;
}
#endif


#if 0
/*---------------------------------------------------------------------------*
 * PixMaskEqualPixels                                                        *
 * Given an anim structure and two frame pointers, make transparent any      *
 * pixels in f1 which are the same as the corresponding ones in f2           *
 *---------------------------------------------------------------------------*/

void PixMaskEqualPixels( anim a, frameptr f1,frameptr f2, int transpixel )
{
    int i;
    int len = a->nWidth * a->nHeight;
    char *p1 = f1->pImage;
    char *p2 = f2->pImage;
    char *m2 = f2->pMask;
    unsigned int *pal1 = f1->pPalette;
    unsigned int *pal2 = f2->pPalette;
    DBTOT;

    for ( i=0; i < len; i++ )
    {
        if ( *m2
             && pal1[*p1] == pal2[*p2] )
        {
            *p1 = transpixel;
            DBINC;
        }
        p1++;
        p2++;
        m2++;
    }
#if DEBUG
    debugf( "pmep: %d pixels cleared\n", total );
#endif
}
#endif


/*---------------------------------------------------------------------------*
 * BitMaskEqualPixels                                                        *
 * Given an anim structure and two frame pointers, make transparent any      *
 * pixels in f1 which are the same as the corresponding ones in f2           *
 *---------------------------------------------------------------------------*/

void BitMaskEqualPixels( anim a, const pixel *i1, pixel *m1,
                         const pixel *i2, const pixel *m2 )
{
    int i;
    int len = a->nWidth * a->nHeight;
    DBTOT;

    for ( i=0; i < len; i++ )
    {
        if ( *m2
             && (*i1 == *i2) )
        {
            *m1 = 0;
            DBINC;
        }
        i1++;
        i2++;
        m1++;
        m2++;
    }
#if DEBUG
    debugf( "bmep: %d pixels cleared\n", total );
#endif
}


/*---------------------------------------------------------------------------*
 * BitMaskToPixMask                                                          *
 * Set all masked-out pixels to the transparent value                        *
 *---------------------------------------------------------------------------*/

void BitMaskToPixMask( anim a, pixel *ptr, const pixel *pMask, int transpixel )
{
    int i;
    int len = a->nWidth * a->nHeight;
    DBTOT;

    for ( i=0; i<len; i++ )
    {
        if ( *pMask == 0 )
        {
            *ptr = transpixel;
            DBINC;
        }
        pMask++;
        ptr++;
    }
#if DEBUG
    debugf( "bmpm: %d pixels set to %d\n", total, transpixel );
#endif
}


/*---------------------------------------------------------------------------*
 * PixMaskToBitMask                                                          *
 * Mask out all pixels with the transparent value                            *
 *---------------------------------------------------------------------------*/

#if 0
void PixMaskToBitMask( anim a, frameptr f, int transpixel )
{
    int i;
    int len = a->nWidth * a->nHeight;
    char *ptr = f->pImage;
    char *pMask = f->pMask;
    DBTOT;

    for ( i=0; i<len; i++ )
    {
        if ( *ptr == transpixel )
        {
            *pMask = 0;
            DBINC;
        }
        pMask++;
        ptr++;
    }
#if DEBUG
    debugf( "pmbm: %d %d-pixels masked out\n", total, transpixel );
#endif
}
#endif


/*---------------------------------------------------------------------------*
 * BitMaskClear                                                              *
 * Make the frame completely opaque                                          *
 *---------------------------------------------------------------------------*/

#if 0
void BitMaskClear( anim a, frameptr f )
{
    int len = a->nWidth * a->nHeight;
    char *pMask = f->pMask;

    memset( pMask, 1, len );
}
#endif


/*---------------------------------------------------------------------------*
 * BitMaskFindTransPixel                                                     *
 * Return a pixel-value from map that's not used by any solid pixel in f     *
 * If bForce=TRUE then, on failure, find the least-used colour and map it    *
 * to something else, then return that.                                      *
 *---------------------------------------------------------------------------*/

int BitMaskFindTransPixel( anim a, pixel *ptr, const pixel *pMask,
                           BOOL bForce )
{
    int i;
    int len = a->nWidth * a->nHeight;
    unsigned int sum[256];
    DBTOT;

    memset( sum, 0, 256*sizeof(int) );

    for ( i=0; i<len; i++ )
    {
        if ( *pMask != 0 )
            sum[*ptr]++;
        pMask++;
        ptr++;
    }

#if DEBUG
    for ( i=0; i < 256; i++ )
        if ( sum[i] == 0 )
            DBINC;
#endif

    for ( i=0; i < 256; i++ )
        if ( sum[i] == 0 )
        {
#if DEBUG
            debugf( "bmft: %d choices, chose %d\n", total, i );
#endif
            return i;
        }

#if 0
    if ( bForce )
    {
        int result = 0;
        unsigned int fewest = sum[0];
        int alternative;

        for ( i=1; i<256; i++ )
        {
            if ( sum[i] < fewest )
            {
                fewest = sum[i];
                result = i;
            }
        }
        /* Now find its replacement */

        /* Reuse 'sum' array as palette */
        memcpy( sum, f->pPalette, 4 * f->nColours );
        sum[result] ^= 0xFFFFFF00;        /* invert */

        alternative = FindClosest( sum, sum[result] ^ 0xFFFFF00 );

        ptr = f->pImage;
        for ( i=0; i<len; i++ )
        {
            if ( *ptr == result )
                *ptr = alternative;
            ptr++;
        }

        debugf( "bmft: kicking out %d (used %d times), replacing with %d\n",
                result, fewest, alternative );


        return result;
    }
#endif

    debugf( "bmft: none found\n" );
    return -1;
}

#if 0
/*---------------------------------------------------------------------------*
 * BitMaskAllSolid                                                           *
 * Return whether the frame's mask is all solid                              *
 *---------------------------------------------------------------------------*/

BOOL BitMaskAllSolid( anim a, frameptr f )
{
    int len = a->nWidth * a->nHeight;
    char *ptr = f->pMask;

    while ( len )
    {
        if ( *ptr == 0 )
            return FALSE;
        ptr++;
        len--;
    }
    return TRUE;
}
#endif


/*---------------------------------------------------------------------------*
 * BitMaskTrimTransparentBorders                                             *
 * Return the smallest rectangle containing all the solid pixels in the mask *
 *---------------------------------------------------------------------------*/

BOOL BitMaskTrimTransparentBorders( anim a, const pixel *ptr, rect *pRect )
{
    char *xbuf, *ybuf;
    unsigned int x,y;

    xbuf = Anim_Allocate( a->nWidth );
    ybuf = Anim_Allocate( a->nHeight );

    if ( !xbuf || !ybuf )
    {
        Anim_Free( &xbuf );
        Anim_Free( &ybuf );
        Anim_NoMemory( "trimtrans" );
        return FALSE;
    }

    memset( xbuf, 0, a->nWidth );
    memset( ybuf, 0, a->nHeight );

    for ( y=0; y < a->nHeight; y++ )
    {
        for ( x=0; x< a->nWidth; x++ )
        {
            if ( *ptr != 0 )
            {
                xbuf[x] = 1;
                ybuf[y] = 1;
            }
            ptr++;
        }
    }
    slim( &pRect->xoff, &pRect->xsize, xbuf, a->nWidth );
    slim( &pRect->yoff, &pRect->ysize, ybuf, a->nHeight );

    Anim_Free( &xbuf );
    Anim_Free( &ybuf );
    return TRUE;
}


/*---------------------------------------------------------------------------*
 * BitMaskFindTransRect                                                      *
 * Returns the bounding rectangle of all pixels which are solid in f1 but    *
 * transparent in f2                                                         *
 *---------------------------------------------------------------------------*/

BOOL BitMaskFindTransRect( anim a, const pixel *p1, const pixel *p2,
                           rect *pRect )
{
    char *xbuf, *ybuf;
    unsigned int x,y;
    DBTOT;

    xbuf = Anim_Allocate( a->nWidth );
    ybuf = Anim_Allocate( a->nHeight );

    if ( !xbuf || !ybuf )
    {
        Anim_Free( &xbuf );
        Anim_Free( &ybuf );
        Anim_NoMemory( "findtransrect" );
        return FALSE;
    }

    memset( xbuf, 0, a->nWidth );
    memset( ybuf, 0, a->nHeight );

    for ( y=0; y < a->nHeight; y++ )
    {
        for ( x=0; x< a->nWidth; x++ )
        {
            if ( (*p1 != 0) && (*p2 == 0) )
            {
                xbuf[x] = 1;
                ybuf[y] = 1;
                DBINC;
            }
            p1++;
            p2++;
        }
    }
    slim( &pRect->xoff, &pRect->xsize, xbuf, a->nWidth );
    slim( &pRect->yoff, &pRect->ysize, ybuf, a->nHeight );

#if DEBUG
    debugf( "bmftr: %d pixels\n", total );
#endif

    debugf( "bmftr: %dx%d at (%d,%d)\n", pRect->xsize, pRect->ysize,
            pRect->xoff, pRect->yoff );

    Anim_Free( &xbuf );
    Anim_Free( &ybuf );
    return TRUE;
}


/*---------------------------------------------------------------------------*
 * BitMaskClearRectangle                                                     *
 * Set a rectangle's worth of f's bitmask to transparent                     *
 *---------------------------------------------------------------------------*/

void BitMaskClearRectangle( anim a, pixel *ptr, const rect *pRect )
{
    unsigned int y,w;

    w = a->nWidth;

    ptr += pRect->xoff + w*(pRect->yoff);

    for ( y=0; y < pRect->ysize; y++ )
    {
        memset( ptr, 0, pRect->xsize );
        ptr += w;
    }
    debugf( "  bmcr: cleared %dx%d at (%d,%d)\n", pRect->xsize, pRect->ysize,
            pRect->xoff, pRect->yoff );
}


/*---------------------------------------------------------------------------*
 * BitMaskAnyTransparent                                                     *
 * TRUE if any pixels in the specified rectangle are transparent             *
 *---------------------------------------------------------------------------*/

BOOL BitMaskAnyTransparent( anim a, const pixel *pMask, const rect *pRect )
{
    unsigned int x,y,w;
    const pixel *ptr;

    w = a->nWidth;

    ptr = pMask + pRect->xoff + w*(pRect->yoff);

    for ( y=0; y < pRect->ysize; y++ )
    {
        for ( x=0; x < pRect->xsize; x++ )
            if ( ptr[x] == 0 )
            {
                debugf( "bmat: Yes (%d,%d)\n", x + pRect->xoff,
                                               y + pRect->yoff );
                return TRUE;
            }
        ptr += w;
    }
    debugf( "bmat: No\n" );
    return FALSE;
}


/*---------------------------------------------------------------------------*
 * PixMaskOptimiseRectangle                                                  *
 * In cases where f1 and f2 have matching pixels, InterGif has the choice of *
 * making f1 transparent, or of putting in another copy of the pixel. Making *
 * f1 transparent wins most of the time, but not when putting in a solid     *
 * pixel continues a run of pixels the same colour, as long runs will always *
 * LZW compress better. This function extends runs of pixels the same colour *
 * into adjacent transparent areas where possible.                           *
 *     An alternative method is included in the code, but ifdef'd out. This  *
 * alternative removes transparency whenever the solid colour is already     *
 * present somewhere else in the rectangle. Some GIFs come out smaller using *
 * Plan A, some using Plan B ... it's my gut feeling that Plan B is better   *
 * more often, but the difference is tiny and either plan gives much better  *
 * results than not calling OptimiseRectangle at all.                        *
 *---------------------------------------------------------------------------*/


void PixMaskOptimiseRectangle( anim a, pixel *i1, pixel *mask1,
                               const pixel *i2, const pixel *mask2,
                               int transpixel, rect *pRect )
{
#if 0
    /* Plan A */
    int last;
    int x,y;
    unsigned char *p1, *p2;
    int offset;
    DBTOT;

    /* Optimise forwards */
    last = -1;

    for ( y=0; y<pRect->ysize; y++ )
    {
        offset = (y+pRect->yoff) * a->width + pRect->xoff;
        p1 = f1->pImage + offset;
        p2 = f2->pImage + offset;

        for ( x=0; x < pRect->xsize; x++ )
        {
            if ( last >= 0 )
            {
                if ( p1[x] == transpixel
                     && p2[x] == last )
                {
                    p1[x] = last;
                    DBINC;
                }
            }
            last = p1[x];
        }
    }

    /* Optimise backwards */
    last = -1;

    for ( y = pRect->ysize-1; y>=0; y-- )
    {
        offset = (y+pRect->yoff) * a->width + pRect->xoff;
        p1 = f1->pImage + offset;
        p2 = f2->pImage + offset;

        for ( x = pRect->xsize-1; x>=0; x-- )
        {
            if ( last >= 0 )
            {
                if ( p1[x] == transpixel
                     && p2[x] == last )
                {
                    p1[x] = last;
                    DBINC;
                }
            }
            last = p1[x];
        }
    }
#if DEBUG
    debugf( "pmor: %d pixels optimised\n", total );
#endif

#else /* 0 */
    /* Plan B */
    unsigned char used[256];
    pixel *p1, *m1;
    const pixel *p2, *m2;
    unsigned int x,y;
    int offset;
    DBTOT;

    memset( used, 0, 256 );

    for ( y=0; y<pRect->ysize; y++ )
    {
        offset = (y+pRect->yoff) * a->nWidth + pRect->xoff;
        p1 = i1 + offset;

        for ( x=0; x < pRect->xsize; x++ )
        {
            used[ p1[x] ] = 1;
        }
    }

    for ( y=0; y<pRect->ysize; y++ )
    {
        offset = (y+pRect->yoff) * a->nWidth + pRect->xoff;
        p1 = i1 + offset;
        p2 = i2 + offset;
        m1 = mask1 + offset;
        m2 = mask2 + offset;

        for ( x=0; x < pRect->xsize; x++ )
        {
                if ( p1[x] == transpixel
                     && m1[x]
                     && m2[x]
                     && used[ p2[x] ] )
                {
                    p1[x] = p2[x];
                    DBINC;
                }
        }
    }
#if DEBUG
    debugf( "pmor: %d pixels optimised\n", total );
#endif

#endif /* 0 */
}
