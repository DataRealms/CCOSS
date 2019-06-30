/* CommonPal.c */

/* Routines for optimising animations' palettes
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 *
 * History:
 *      01-Mar-97 pdh Created
 *      07-Apr-97 *** Release 6beta1
 *      20-May-97 *** Release 6beta2
 *      24-Aug-97 pdh Fixed problem with blank frame 1, 2-colour frame 2
 *                    Made palette entry for transparent colour black :-((((
 *      24-Aug-97 *** Release 6
 *      27-Sep-97 *** Release 6.01
 *      07-Nov-97 pdh Anim_Trim
 *      08-Nov-97 *** Release 6.02
 *      15-Feb-98 pdh Move palette mapping stuff to palettemap.c
 *      21-Feb-98 *** Release 6.03
 *      07-Jun-98 *** Release 6.04
 *      21-Aug-98 *** Release 6.05
 *      05-Oct-98 *** Release 6.06
 *      19-Feb-99 *** Release 6.07
 *
 */

#include <string.h>
#include <stdarg.h>

#include "animlib.h"
#include "utils.h"
#include "gifencode.h"
#include "workspace.h"
#include "frame.h"

#if 0
#define debugf printf
#define DEBUG 1
#else
#define debugf 1?0:printf
#define DEBUG 0
#endif

#if 0
#define HEAPTEST HeapTest(__LINE__)

static void HeapTest( int line )
{
    void *foo;

    Anim_CheckHeap(__FILE__,line);

    foo = Anim_Allocate(20000);
    if ( foo ) Anim_Free(&foo);
}
#else
#define HEAPTEST
#endif

static int FindColour( unsigned int *pColours, unsigned int *pnColours,
                       unsigned int colour );

BOOL Anim_CommonPalette( anim a )
{
    unsigned int colours[256];
    unsigned int nColours = 0;
    unsigned int nThisFrame;
    pixel *image, *mask;
    pixel map[256];
    unsigned char used[256];
    unsigned char dstused[256];
    unsigned int i, j;
    unsigned int len = a->nWidth * a->nHeight;
    BOOL result = TRUE;
    BOOL compatible;
    frame f;
    unsigned int mincolours = 0;
    unsigned int nUsedOnFrame;

    image = Anim_Allocate( len );
    mask  = Anim_Allocate( len );
    if ( !image || !mask )
    {
        Anim_NoMemory( "commonpal" );
        result = FALSE;
    }
    else
    {
        Workspace_Claim( gifcompress_WORKSIZE );

        for ( i=0; i < a->nFrames; i++ )
        {
            f = a->pFrames + i;

            if ( !Anim_Decompress( f->pImageData, f->nImageSize, len, image ) )
            {
                result = FALSE;
                break;
            }

            if ( f->pMaskData )
            {
                if ( !Anim_Decompress( f->pMaskData, f->nMaskSize, len, mask ) )
                {
                    result = FALSE;
                    break;
                }
            }
            else
                memset( mask, 1, len );     /* all solid */

            memset( used, 0, 256 );

            for ( j=0; j<len; j++ )
                if ( mask[j] )
                    used[ image[j] ] = 1;

            compatible = TRUE;

            nThisFrame = nColours;

            memset( dstused, 0, 256 );

            for ( j=0; j<256; j++ )
            {
                if ( used[j] )
                {
                    int n = FindColour( colours, &nColours,
                                        f->pal->pColours[j] );

                    if ( n<0 )
                    {
                        compatible = FALSE;
                        break;
                    }
                    map[j] = (pixel) n;
                    dstused[n] = 1;
                }
            }

            if ( compatible )
            {
                debugf( "commonpal: frame %d compatible\n", i );

#if DEBUG
                debugf( "map" );
                for ( j=0; j<f->pal->nColours; j++ )
                {
                    if ( used[j] )
                        debugf( " %d", map[j] );
                    else
                        debugf( " -" );
                }
                debugf( "\n" );
#endif

                Palette_Destroy( &f->pal );     /* sets to NULL */

                /* Remap the frame to the new palette */
                for ( j=0; j<len; j++ )
                    image[j] = map[image[j]];
                Anim_Free( &f->pImageData );

                f->pImageData = Anim_Compress( image, len, &f->nImageSize );

                if ( !f->pImageData )
                {
                    result = FALSE;
                    break;
                }

                nUsedOnFrame = 0;

                for ( j=0; j<256; j++ )
                    if ( dstused[j] )
                        nUsedOnFrame++;

                if ( nUsedOnFrame >= mincolours && (i || f->pMaskData) )
                {
                    mincolours = nUsedOnFrame + 1;
                }
            }
            else
            {
                debugf( "commonpal: frame %d INcompatible!\n", i );

                nColours = nThisFrame;
            }
        }

        Workspace_Release();
    }

    Anim_Free( &image );
    Anim_Free( &mask );

    if ( result )
    {
        /* Allow an extra colour for transparency if possible */
        if ( mincolours > nColours && nColours < 256 )
        {
            /* And make it black, to avoid BUGS in the Microsoft Java VM */
            colours[nColours] = 0;
            nColours++;
        }

        debugf( "mincolours=%d nColours=%d\n", mincolours, nColours );

        for ( i=0; i < a->nFrames; i++ )
        {
            f = a->pFrames+i;

            if ( !f->pal )
            {
                f->pal = Palette_Create( colours, nColours );
                if ( !f->pal )
                {
                    result = FALSE;
                    break;
                }
            }
        }
    }
    return result;
}

static int FindColour( unsigned int *pColours, unsigned int *pnColours,
                       unsigned int colour )
{
    int i;

    for ( i=0; i < (int)*pnColours; i++ )
    {
        if ( pColours[i] == colour )
        {
            return i;
        }
    }

    if ( *pnColours == 256 )
        return -1;

    pColours[ *pnColours ] = colour;
    return (*pnColours)++;
}


        /*===========================================*
         *   Stuff that doesn't really belong here   *
         *===========================================*/

#if 0
BOOL Anim_AddToHistogram( anim a )
{
    pixel *buffer;
    pixel *mask;
    unsigned int size = a->nWidth * a->nHeight;
    unsigned int i, j;

    debugf( "in Anim_AddToHistogram(%d)\n", a->nFrames );

    buffer = Anim_Allocate( size );
    mask   = Anim_Allocate( size );

    if ( !buffer
         || !mask )
    {
        Anim_NoMemory( "addhist" );
        Anim_Free( &buffer );
        Anim_Free( &mask );
        return FALSE;
    }

    Workspace_Claim(0);

    for ( i=0; i<a->nFrames; i++ )
    {
        frame f = a->pFrames + i;
        unsigned int *pal = f->pal->pColours;

        memset( mask, 1, size );

        if ( !Anim_Decompress( f->pImageData, f->nImageSize, size, buffer )
             || ( f->pMaskData
                  && !Anim_Decompress( f->pMaskData, f->nMaskSize, size, mask )
                )
            )
        {
            Anim_Free( &buffer );
            Anim_Free( &mask );
            Workspace_Release();
            return FALSE;
        }

        for ( j=0; j<size; j++ )
            if ( mask[j] )
                if ( !Histogram_Pixel( pal[ buffer[j] ] ) )
                {
                    Anim_Free( &buffer );
                    Anim_Free( &mask );
                    Workspace_Release();
                    return FALSE;
                }
    }

    Workspace_Release();

    Anim_Free( &buffer );
    Anim_Free( &mask );

    debugf( "Anim_AddToHistogram exits\n" );

    return TRUE;
}
#endif

BOOL Anim_Trim( anim a )
{
    pixel *buffer;
    pixel *mask;
    unsigned int size = a->nWidth * a->nHeight;
    unsigned int i;
    rect rcFrame, rcMin;

    /* quick exit if all solid */
    for ( i=0; i < a->nFrames; i++ )
        if ( a->pFrames[i].pMaskData == NULL )
            return TRUE;

    buffer = Anim_Allocate( size );
    mask   = Anim_Allocate( size );

    if ( !buffer || !mask )
    {
        Anim_NoMemory( "trim" );
        Anim_Free( &buffer );
        Anim_Free( &mask );
        return FALSE;
    }

    Workspace_Claim(0);

    /* discover smallest rect */
    for ( i=0; i<a->nFrames; i++ )
    {
        framestr *f = a->pFrames + i;

        if ( !Anim_Decompress( f->pMaskData, f->nMaskSize, size, mask )
             || !BitMaskTrimTransparentBorders( a, mask, &rcFrame ) )
        {
            Anim_Free( &buffer );
            Anim_Free( &mask );
            Workspace_Release();
            return FALSE;
        }

        if ( i==0 )
            rcMin = rcFrame;    /* structure copy */
        else
            Rect_Union( &rcMin, &rcFrame );
    }

    if ( rcMin.xoff > 0 || rcMin.yoff > 0
         || rcMin.xsize < a->nWidth || rcMin.ysize < a->nHeight )
    {
        /* Scissors out, Penfold */
        pixel *imgstart = buffer + rcMin.xoff + rcMin.yoff*a->nWidth;
        pixel *maskstart = mask + rcMin.xoff + rcMin.yoff*a->nWidth;

        for ( i=0; i<a->nFrames; i++ )
        {
            framestr *f = a->pFrames + i;

            if ( !Anim_Decompress( f->pImageData, f->nImageSize, size, buffer )
                 || !Anim_Decompress( f->pMaskData, f->nMaskSize, size, mask ))
            {
                Anim_Free( &buffer );
                Anim_Free( &mask );
                Workspace_Release();
                return FALSE;
            }
            Anim_Free( &f->pImageData );
            Anim_Free( &f->pMaskData );

            f->pImageData = Anim_CompressAligned( imgstart, rcMin.xsize,
                                                  rcMin.ysize, a->nWidth,
                                                  &f->nImageSize );
            f->pMaskData = Anim_CompressAligned( maskstart, rcMin.xsize,
                                                  rcMin.ysize, a->nWidth,
                                                  &f->nMaskSize );
            if ( !f->pImageData || !f->pMaskData )
            {
                Anim_Free( &buffer );
                Anim_Free( &mask );
                Workspace_Release();
                return FALSE;
            }
        }
        a->nWidth = rcMin.xsize;
        a->nHeight = rcMin.ysize;
    }
    Anim_Free( &buffer );
    Anim_Free( &mask );
    Workspace_Release();
    return TRUE;
}

/* eof */
