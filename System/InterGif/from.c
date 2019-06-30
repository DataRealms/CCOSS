/* From.c */

/* Whole-animation constructors for InterGif
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 *
 * History:
 *      21-Aug-98 pdh Split off from intergif.c
 *      21-Aug-98 *** Release 6.05
 *      05-Oct-98 pdh Fix pixmasks
 *      05-Oct-98 *** Release 6.06
 *      19-Feb-99 *** Release 6.07
 *
 */

#include <stdio.h>

#include "animlib.h"
#include "utils.h"

#if 0
#define debugf printf
#define DEBUG 1
#else
#define debugf 1?0:printf
#define DEBUG 0
#endif

/* Convert %0bbbbbgggggrrrrr to &BBGGRR00 */
unsigned int Anim_Pix16to32(unsigned int x)
{
    unsigned int b = (x>>(10-3)) & 0xF8;
    unsigned int g = (x>>(5-3)) & 0xF8;
    unsigned int r = (x<<3) & 0xF8;
    b += (b>>5);
    g += (g>>5);
    r += (r>>5);
    return (b<<24) + (g<<16) + (r<<8);
}

static void MakeByteWide( const anim_imageinfo *aii, pixel *pOutput,
                          palettemapper pmap )
{
    int bpp = aii->nBPP;
    int x,y;
    const char *pBits = (const char*) aii->pBits;
    int h = aii->nHeight, w = aii->nWidth;

    if ( bpp > 8 )
    {
        /* Direct colour (must be mapped) */
        for ( y=0; y<h; y++ )
        {
            if ( bpp == 16 )
            {
                const unsigned short *pShorts = (unsigned short*) pBits;

                for ( x=0; x<w; x++ )
                {
                    unsigned int pix = *pShorts++;
                    pix = Anim_Pix16to32(pix);
                    *pOutput++ = PaletteMapper_MapIndex( pmap, pix );
                }
            }
            else /* bpp == 32 */
            {
                const unsigned int *pWords = (unsigned int*) pBits;

                for ( x=0; x<w; x++ )
                {
                    unsigned int pix = *pWords++;
                    *pOutput++ = PaletteMapper_MapIndex( pmap, pix<<8 );
                }
            }
            pBits += aii->nLineWidthBytes;
        }
    }
    else
    {
        /* Palettised colour (no mapping needed) */
        for ( y=0; y<h; y++ )
        {
            unsigned int imagewant = (1<<bpp) - 1;
            unsigned char *imgptr = (unsigned char*) pBits;
            unsigned int imageshift = 0;

            for ( x=0; x<w; x++ )
            {
                unsigned int pix = (*imgptr >> imageshift) & imagewant;

                *pOutput++ = pix;

                imageshift += bpp;
                if ( imageshift == 8 )
                {
                    imgptr++;
                    imageshift = 0;
                }
            }
            pBits += aii->nLineWidthBytes;
        }
    }
}

typedef struct {
    anim a;
    palettemapper pmap;
} animmaker_args;

static BOOL Animmaker( void *handle, unsigned int xs, unsigned int ys,
                       unsigned int flags )
{
    animmaker_args *pArgs = (animmaker_args*) handle;
    anim a;

    if ( pArgs->a )
        return TRUE;

    a = Anim_Create();

    if ( !a )
    {
        debugf( "animmaker: Anim_Create failed\n" );
        return FALSE;
    }
    pArgs->a = a;
    a->nWidth = xs;
    a->nHeight = ys;
    a->flags = flags;

    debugf( "animmaker: %dx%d, flags 0x%x\n", xs, ys, flags );

    return TRUE;
}

static BOOL Animframe( void *handle, const anim_imageinfo *pixels,
                       int pixmask, const anim_imageinfo *bitmask,
                       const unsigned int *pColours )
{
    animmaker_args *pArgs = (animmaker_args*) handle;
    anim a = pArgs->a;
    int bpp = pixels->nBPP;
    pixel *image = NULL, *mask = NULL;
    unsigned int xs, ys;
    BOOL bMask = ( pixmask != -1 || bitmask );
    unsigned int mappedpal[256];
    const unsigned int *pPalette = pColours;
    int ncol = 1<<bpp;
    BOOL result = TRUE;

    xs = pixels->nWidth;
    ys = pixels->nHeight;

    if ( xs != a->nWidth || ys != a->nHeight )
    {
        Anim_SetError( "All images must be the same size" );
        return FALSE;
    }

    if ( bpp > 8 && !pArgs->pmap )
    {
        Anim_SetError( "Please choose a palette-reduction option when using images with more than 256 colours" );
        return FALSE;
    }

    image = Anim_Allocate( xs*ys );

    if ( bMask )
        mask = Anim_Allocate( xs*ys );

    if ( !image || (bMask && !mask ) )
    {
        Anim_NoMemory("animmaker");
        return FALSE;
    }

    if ( bitmask )
    {
        debugf( "animmaker: calling mbw(mask)\n" );
        MakeByteWide( bitmask, mask, NULL );
    }

    debugf( "animmaker: calling mbw(image)\n" );

    MakeByteWide( pixels, image, pArgs->pmap );

    debugf( "animmaker: mbw ok\n" );

    if ( pixmask != -1 )
    {
        unsigned int i;

        debugf( "animmaker: making pixmask\n" );
        for ( i=0; i<xs*ys; i++ )
            mask[i] = (image[i] == pixmask) ? 0 : 255;
    }

    if ( pArgs->pmap )
    {
        if ( bpp > 8 )
            ncol = PaletteMapper_GetPalette( pArgs->pmap, mappedpal );
        else
        {
            int i;

            for ( i=0; i<ncol; i++ )
                mappedpal[i] = PaletteMapper_Map( pArgs->pmap, pColours[i] );
        }
        pPalette = mappedpal;
    }

    if ( Anim_AddFrame( a, xs, ys, image, bMask ? mask : NULL, ncol,
                         pPalette ) )
    {
        a->pFrames[a->nFrames-1].csDelay = pixels->csDelay;
    }
    else
    {
        result = FALSE;
        debugf( "animmaker: Anim_AddFrame failed\n" );
    }

    Anim_Free( &image );
    Anim_Free( &mask );

    return result;
}


        /*=======================*
         *   Exported routines   *
         *=======================*/


anim Anim_FromData( const void *data, unsigned int size,
                    palettemapper pmap )
{
    animmaker_args aa = { NULL, NULL };
    BOOL result;

    aa.pmap = pmap;
    result = Anim_ConvertData( data, size, &Animmaker, &Animframe, &aa );

    if ( !result )
        Anim_Destroy( &aa.a );

    return aa.a;
}

anim Anim_FromFile( const char *filename, palettemapper pmap )
{
    animmaker_args aa = { NULL, NULL };
    BOOL result;

    aa.pmap = pmap;
    result = Anim_ConvertFile( filename, &Animmaker, &Animframe, &aa );

    if ( !result )
        Anim_Destroy( &aa.a );

    return aa.a;
}

anim Anim_FromFiles( const char *firstname, palettemapper pmap )
{
    animmaker_args aa = { NULL, NULL };
    BOOL result;

    aa.pmap = pmap;
    result = Anim_ConvertFiles( firstname, &Animmaker, &Animframe, &aa );

    if ( !result )
        Anim_Destroy( &aa.a );

    return aa.a;
}

/* eof */
