/* MedianCut.c */

/* Median-cut quantiser algorithm for AnimLib
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 *      This software is based in part on the work of the Independent JPEG Group
 *
 * History:
 *      17-Feb-97 pdh Ported jquant2.c out of jpeglib to RiscOS
 *      23-Feb-97 pdh Optimise for new histogram code
 *      19-Mar-97 pdh Shift into AnimLib
 *      07-Apr-97 *** Release 6beta1
 *      20-May-97 *** Release 6beta2
 *      24-Aug-97 *** Release 6
 *      27-Sep-97 *** Release 6.01
 *      08-Nov-97 *** Release 6.02
 *      21-Feb-98 *** Release 6.03
 *      07-Jun-98 *** Release 6.04
 *      21-Aug-98 *** Release 6.05
 *      19-Feb-99 *** Release 6.07
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "animlib.h"
#include "utils.h"

#if 0
#define debugf printf
#else
#define debugf 1?0:printf
#endif

#define C0_SCALE 2		/* scale R distances by this much */
#define C1_SCALE 3		/* scale G distances by this much */
#define C2_SCALE 1		/* and B by this much */

#define scale_RED 3
#define scale_GREEN 10
#define scale_BLUE 1

typedef struct {
    unsigned int min[3], max[3];

    /* The volume (actually 2-norm) of the box */
    int volume;

    int first, last;  /* indices into histogram */
} box;

typedef box * boxptr;

static const box firstbox = { {0,0,0},{255,255,255},0,0,0 };

/*---------------------------------------------------------------------------*
 * Partition()                                                               *
 * Split the histogram so that these two newly-split boxes each have         *
 * continuous ranges                                                         *
 *---------------------------------------------------------------------------*/

static void Partition( histogramentrystr *h, int axis, box *b1, box *b2 )
{
    histogramentrystr *first = h + b1->first;
    histogramentrystr *last = h + b1->last;
    unsigned int mask = 0xFF << (axis*8 + 8);
    histogramentrystr t;
    unsigned int val = (b1->max[axis] + b1->min[axis]) / 2;

    val = val << (axis*8 + 8);

    for (;;)
    {
        while ( ( first->value & mask ) <= val ) first++;
        while ( ( last->value & mask ) > val ) last--;

        /* first cannot equal last here */
        if ( first > last )
            break;

        t = *first;
        *first = *last;
        *last = t;
    }

    b2->last = b1->last;

    /* Here (we rather hope) first=last+1 */
    b1->last = last - h;
    b2->first = first - h;

    debugf( "Done partitioning: split %d-%d into %d-%d and %d-%d on 0x%x\n",
            b1->first, b2->last,
            b1->first, b1->last, b2->first, b2->last, val );

    b1->max[axis] = val;
    b2->min[axis] = val+1;
}


/*---------------------------------------------------------------------------*
 * ShrinkBox                                                                 *
 * Shrink the min/max bounds of a box to enclose only nonzero elements,      *
 * and recompute its volume and population                                   *
 *---------------------------------------------------------------------------*/

static void ShrinkBox( histogramentrystr *h, boxptr boxp )
{
    histogramentrystr *ptr;
    unsigned int min[3], max[3];
    int dist0, dist1, dist2;

    memset( min, -1, 3*sizeof(unsigned int) );
    memset( max, 0, 3*sizeof(unsigned int) );

    for ( ptr = h + boxp->first; ptr <= h + boxp->last; ptr++ )
    {
        unsigned int v, vv = ptr->value;

        v = (vv >> 8) & 0xFF;
        if ( v > max[0] ) max[0] = v;
        if ( v < min[0] ) min[0] = v;

        v = (vv >> 16) & 0xFF;
        if ( v > max[1] ) max[1] = v;
        if ( v < min[1] ) min[1] = v;

        v = (vv >> 24) & 0xFF;
        if ( v > max[2] ) max[2] = v;
        if ( v < min[2] ) min[2] = v;
    }

    memcpy( boxp->min, min, 3*sizeof(unsigned int) );
    memcpy( boxp->max, max, 3*sizeof(unsigned int) );

  /* Update box volume.
   * We use 2-norm rather than real volume here; this biases the method
   * against making long narrow boxes, and it has the side benefit that
   * a box is splittable iff norm > 0.
   */

  dist0 = max[0]-min[0];
  dist1 = max[1]-min[1];
  dist2 = max[2]-min[2];
  boxp->volume = dist0*dist0*scale_RED
               + dist1*dist1*scale_GREEN
               + dist2*dist2*scale_BLUE;

    debugf( "Shrunk box: now %d-%d, %d-%d, %d-%d\n",
            min[0], max[0], min[1], max[1], min[2], max[2] );
}


/*---------------------------------------------------------------------------*
 * MaxPopulation()                                                           *
 * Find the splittable box with the largest color population                 *
 * Returns NULL if no splittable boxes remain                                *
 *---------------------------------------------------------------------------*/

static boxptr MaxPopulation( boxptr b, int n )
{
    int i;
    int maxc = 0;
    boxptr which = NULL;

    for ( i=0; i < n; i++ )
    {
        int count = b->last - b->first;

        if ( count > maxc && b->volume > 0)
        {
            which = b;
            maxc = count;
        }
        b++;
    }
    return which;
}


/*---------------------------------------------------------------------------*
 * MaxVolume()                                                               *
 * Find the splittable box with the largest (scaled) volume                  *
 * Returns NULL if no splittable boxes remain                                *
 *---------------------------------------------------------------------------*/

static boxptr MaxVolume( boxptr b, int n )
{
    int i;
    int maxv = 0;
    boxptr which = NULL;

    for ( i=0; i < n; i++ )
    {
        /* Don't worry about the absence of a (b->last > b->first) condition
         * here ... if there's only one entry, b->volume will be 0
         */
        if ( b->volume > maxv )
        {
            which = b;
            maxv = b->volume;
        }
        b++;
    }
    return which;
}


/*---------------------------------------------------------------------------*
 * Median()                                                                  *
 * Repeatedly select and split the largest box until we have enough boxes    *
 *---------------------------------------------------------------------------*/

static int Median( histogramentrystr *h, boxptr boxlist, int nBoxes,
                   int nColours )
{
    int n;
    int c0, c1, c2, cmax;
    boxptr b1, b2;

    while ( nBoxes < nColours)
    {
        /* Select box to split.
         * Current algorithm: by population for first half, then by volume.
         */
        if ( nBoxes*2 <= nColours)
            b1 = MaxPopulation( boxlist, nBoxes );
        else
            b1 = MaxVolume( boxlist, nBoxes );

        if (b1 == NULL)		/* no splittable boxes left! */
            break;

        b2 = &boxlist[nBoxes];	/* where new box will go */

        /* Copy the color bounds to the new box. */
        memcpy( b2->max, b1->max, 3*sizeof(unsigned int) );
        memcpy( b2->min, b1->min, 3*sizeof(unsigned int) );

        /* Choose which axis to split the box on.
         * Current algorithm: longest scaled axis.
         */
        c0 = (b1->max[0] - b1->min[0]);
        c1 = (b1->max[1] - b1->min[1]);
        c2 = (b1->max[2] - b1->min[2]);

        /* We want to break any ties in favor of green, then red, blue last.
         * This code does the right thing for R,G,B or B,G,R color orders only.
         */
        cmax = c1*c1*scale_GREEN;
        n = 1;
        c0 = c0*c0*scale_RED;
        if (c0 > cmax)
        {
            cmax = c0;
            n = 0;
        }
        if (c2*c2*scale_BLUE > cmax)
            n = 2;

        Partition( h, n, b1, b2 );

        /* Update stats for boxes */
        ShrinkBox( h, b1 );
        ShrinkBox( h, b2 );
        nBoxes++;
    }
    return nBoxes;
}


histogram Histogram_ReduceMC( histogram input, int nColours )
{
    boxptr boxlist;
    int nBoxes;
    int i;
    histogram result = (histogram) Anim_Allocate( sizeof(histogramstr) );

    boxlist = (boxptr) Anim_Allocate( nColours*sizeof(box) );

    if ( !result || !boxlist )
    {
        Anim_Free( &result );
        Anim_Free( &boxlist );
        Anim_NoMemory( "mediancut" );
        return NULL;
    }

    nBoxes = 1;

    boxlist[0] = firstbox;
    boxlist[0].last = input->nEntries - 1;

    ShrinkBox( input->e, &boxlist[0] );

    nBoxes = Median( input->e, boxlist, nBoxes, nColours );

    result->nEntries = nBoxes;
    result->e =
        (histogramentrystr*) Anim_Allocate( nBoxes*sizeof(histogramentrystr) );

    if ( !result->e )
    {
        Anim_Free( &result );
        Anim_Free( &boxlist );
        Anim_NoMemory( "mediancut2" );
        return NULL;
    }

    for ( i=0; i < nBoxes; i++ )
    {
        /* ComputeColour() */
        histogramentrystr *ptr;
        int total = 0;
        int c0total = 0;
        int c1total = 0;
        int c2total = 0;
        unsigned int colour;
        box *boxp = boxlist+i;

        for ( ptr = input->e + boxp->first; ptr <= input->e + boxp->last; ptr++ )
        {
            unsigned int v = ptr->value;
            unsigned int n = ptr->n;

            c0total += ((v>>8) & 0xFF) * n;
            c1total += ((v>>16) & 0xFF) * n;
            c2total += ((v>>24) & 0xFF) * n;
            total += n;
        }

        debugf( "%03d total=%d\n", i, total );

        colour  = ((c0total + (total>>1)) / total) << 8;
        colour |= ((c1total + (total>>1)) / total) << 16;
        colour |= ((c2total + (total>>1)) / total) << 24;

        result->e[i].value = colour;
        result->e[i].n = total;
    }

    Anim_Free( &boxlist );

    return result;
}
