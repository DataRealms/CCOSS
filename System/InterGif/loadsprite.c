/* LoadSprite.c */

/* RiscOS sprite code for InterGif
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 *
 * History:
 *      23-Aug-96 pdh Created
 *      25-Aug-96 *** Release 3.00
 *      01-Sep-96 pdh Added support for bLoop, nBackground fields
 *      01-Sep-96 *** Release 3.01
 *      27-Oct-96 *** Release 4beta1
 *      29-Oct-96 *** Release 4beta2
 *      07-Nov-96 *** Release 4
 *      15-Dec-96 *** Release 5beta1
 *      12-Jan-97 pdh Added support for pauses on individual frames
 *      27-Jan-97 *** Release 5beta2
 *      29-Jan-97 *** Release 5beta3
 *      03-Feb-97 *** Release 5
 *      07-Feb-97 *** Release 5.01
 *      01-Mar-97 pdh Frob a lot for new anim library
 *      07-Apr-97 *** Release 6beta1
 *      20-May-97 *** Release 6beta2
 *      24-Aug-97 *** Release 6
 *      27-Sep-97 *** Release 6.01
 *      08-Nov-97 *** Release 6.02
 *      15-Feb-98 pdh Support for palette-mappers
 *      21-Feb-98 *** Release 6.03
 *      26-May-98 pdh Fix Rob Davison's bug for "same size but different depth"
 *      07-Jun-98 *** Release 6.04
 *      30-Jul-98 pdh Stuff for anim_imagefn
 *      21-Aug-98 *** Release 6.05
 *      05-Oct-98 *** Release 6.06
 *      06-Oct-98 pdh Remove RiscOS dependencies
 *	19-Feb-99 *** Release 6.07
 *      17-Apr-99 pdh Fix VIDC1 palette (had R and B wrong way round)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#include "animlib.h"
#include "sprite.h"

#if 0
#define debugf printf
#define DEBUG 1
#else
#define debugf 1?0:printf
#define DEBUG 0
#endif


    /*========================*
     *   RiscOS SWI veneers   *
     *========================*/


void Sprite_RemoveLeftHandWastage( spritearea pArea, sprite spr );


    /*========================*
     *   Forward references   *
     *========================*/


static void Sprite__GetPalette( const sprite spr, int ncol,
                                unsigned int *pPalette );
static int Sprite__GetDelay( const sprite spr );
static int Sprite__ModeLog2BPP( int mode );


    /*=====================*
     *   Anim_FromSprite   *
     *=====================*/


BOOL Anim_ConvertSprite( const void *data, size_t nSize,
                         anim_animfn animfn, anim_imagefn fn, void *handle )
{
    spritearea pArea;
    sprite spr;
    int log2bpp;
    unsigned int i;
    unsigned int pal[256];
    int xs,ys,abw;
    anim_imageinfo pixels, mask;

    pArea = (spritearea) ((char*)data-4);

    /* Check it's a sprite file, if not return NULL */
    if ( pArea->nFirstOffset != 16
         || pArea->nFreeOffset != nSize+4 )
    {
        debugf( "Not a sprite file\n" );
        return FALSE;
    }

    spr = (sprite)(pArea+1);

    for ( i=0; i < pArea->nSprites; i++ )
    {
        int bpp;
        BOOL bMask;

        Sprite_RemoveLeftHandWastage( pArea, spr );

        log2bpp = Sprite__ModeLog2BPP( spr->screenmode );
        bpp = 1 << log2bpp;
        xs = (spr->width*32 + spr->rightbit-spr->leftbit+1 )
                >> log2bpp;
        ys = spr->height+1;
        abw = spr->width*4+4;

        if ( i==0 && animfn )
        {
            if ( !(*animfn)( handle, xs, ys, 0 ) )
            {
                debugf( "convertsprite: animfn returned FALSE, exiting\n" );
                return FALSE;
            }
        }

        bMask = FALSE;

        if ( spr->maskoffset != spr->imageoffset
             && spr->maskoffset != 0 )
        {
            BOOL newformat = ( spr->screenmode & 0xF8000000 ) != 0;

            bMask = TRUE;
            mask.nWidth = xs;
            mask.nLineWidthBytes = newformat ? ( (xs+31)>>5 ) << 2 : abw;
            mask.nHeight = ys;
            mask.nBPP = newformat ? 1 : bpp;
            mask.pBits = (char*)spr + spr->maskoffset;
        }

        if ( bpp <= 8 )
            Sprite__GetPalette( spr, 1<<bpp, pal );

        pixels.nWidth = xs;
        pixels.nLineWidthBytes = abw;
        pixels.nHeight = ys;
        pixels.nBPP = bpp;
        pixels.pBits = (char*)spr + spr->imageoffset;
        pixels.csDelay = Sprite__GetDelay( spr );

        if ( !(*fn)( handle, &pixels, -1, bMask ? &mask : NULL,
                     bpp<=8 ? pal : NULL ) )
        {
            debugf( "convertsprite: fn returned FALSE, exiting\n" );
            return FALSE;
        }

        spr = (sprite) ( (char*)spr + spr->nNextOffset );
    }

    return TRUE;
}


static const unsigned int mode0palette[] = { 0xFFFFFF00, 0 };
static const unsigned int mode1palette[] = {
    0xFFFFFF00, 0xAAAAAA00, 0x55555500, 0 };
static const unsigned int mode12palette[] = {
    0xFFFFFF00, 0xDDDDDD00, 0xBBBBBB00, 0x99999900,
    0x77777700, 0x55555500, 0x33333300, 0x00000000,
    0x99440000, 0x00EEEE00, 0x00CC0000, 0x0000DD00,
    0xBBEEEE00, 0x00885500, 0x00BBFF00, 0xFFBB0000 };

static void Sprite__GetPalette( const sprite spr, int ncol,
                                unsigned int *pPalette )
{
    const unsigned int *pal;

    if ( spr->imageoffset < (44+ncol*8) )
    {
        /* This sprite don't got no palette */

        if ( ncol == 256 )
        {
            int i;

            /* Construct 8bit VIDC1 palette */
            for ( i=0; i<256; i++ )
            {
                /* See 3PRM p1-539 (red and blue were the wrong way round in
                 * InterGif 6.07)
                 */
                pPalette[i] = ((i&  3) * 0x11111100U)      /* tint b1 b0 */
                            + ((i&  4) ? 0x00004400U : 0)  /* red b2 */
                            + ((i&  8) ? 0x44000000U : 0)  /* blue b2 */
                            + ((i& 16) ? 0x00008800U : 0)  /* red b3 */
                            + ((i& 32) ? 0x00440000U : 0)  /* green b2 */
                            + ((i& 64) ? 0x00880000U : 0)  /* green b3 */
                            + ((i&128) ? 0x88000000U : 0); /* blue b3 */
            }
        }
        else
        {
            switch ( ncol )
            {
            case 2:  pal = mode0palette; break;
            case 4:  pal = mode1palette; break;
            case 16: pal = mode12palette; break;
            default: return;
            }
            memcpy( pPalette, pal, ncol * 4 );
        }
    }
    else
    {
        int i;

        pal = (const unsigned int*)((const char*)spr + 44);

        for ( i=0; i<ncol; i++ )
            pPalette[i] = pal[i*2];
    }
}

static int Sprite__GetDelay( const sprite spr )
{
    const char *p;
    int i = 0;

    p = strstr( spr->name, "delay" );
    if ( p )
        i = atoi( p+5 );

    return i ? i : 8;
}

static const char spritebpp[] = {
    0,  /* MODE 0 1bpp */
    1,	/* MODE 1 2bpp */
    2,	/* MODE 2 4bpp */
    0,	/* MODE 3? */
    0,	/* MODE 4 1bpp */
    1,	/* MODE 5 2bpp */
    0,	/* MODE 6? */
    0,	/* MODE 7? */
    1,
    2,	/* MODE 9 4bpp */
    3,	/* MODE 10 8bpp */
    1,
    2,	/* MODE 12 4bpp */
    3,	/* MODE 13 8bpp */
    2,
    3,	/* MODE 15 8bpp */
    2,
    2,
    0,	/* MODE 18 1bpp */
    1,	/* MODE 19 2bpp */
    2,	/* MODE 20 4bpp */
    3,	/* MODE 21 8bpp */
    2,
    0,	/* MODE 23 1bpp */
    3,	/* MODE 24 8bpp */
    0,
    1,
    2,
    3,	/* MODE 28 8bpp */
    0,
    1,	/* MODE 30 2bpp */
    2,
    3,
    0,
    1,
    2,
    3,	/* MODE 36 8bpp */
    0,
    1,
    2,
    3,	/* MODE 40 8bpp */
    0,
    1,
    2,	/* MODE 43 4bpp */
    0,
    1,
    2,	/* MODE 46 4bpp */
    3,	/* MODE 47 8bpp */
    2,	/* MODE 48 4bpp */
    3	/* MODE 49 8bpp */
};

static int Sprite__ModeLog2BPP( int mode )
{
    int type = (mode>>27) & 63;

    if ( type == 0 )
        return mode < 50 ? (int)(spritebpp[mode]) : 4;

    return type-1;
}

#ifndef RISCOS
/* Remove any left-hand wastage from the sprite. There's a SWI to do
 * this in RiscOS :-) but not elsewhere :-(
 */
static void rotate( unsigned int *src, int srcwidth, int destwidth, int height,
                    int shift )
{
    int x, y;
    unsigned int w;
    unsigned int *dest;
    unsigned int shift2 = 32-shift;

    dest = src;

    for ( y=0; y< height; y++ )
    {
        w = *src;
        for ( x=1; x<srcwidth; x++ )
        {
            dest[x-1] = (w>>shift) | (src[x]<<shift2);
            w = src[x];
        }
        dest[x-1] = w>>shift;

        src += srcwidth;
        dest += destwidth;
    }
}

void Sprite_RemoveLeftHandWastage( spritearea a, sprite s )
{
    int srcwidth, destwidth;
    int height;
    unsigned int shift = s->leftbit;

    /* If nothing to do, or new style sprite, return */
    if ( shift == 0 || (s->screenmode & 0xF8000000) )
        return;

    height = s->height+1;
    srcwidth = s->width+1;
    destwidth = srcwidth;
    if ( s->rightbit < shift )
        destwidth--;

    rotate( (unsigned int*)((char*)s + s->imageoffset), srcwidth, destwidth,
            height, shift );

    if ( s->imageoffset != s->maskoffset )
    {
        /* if it's old enough to have lh wastage, it's too old to have
         * a 1bpp mask
         */
        rotate( (unsigned int*)((char*)s + s->maskoffset), srcwidth, destwidth,
                height, shift );
    }

    s->leftbit = 0;
    s->rightbit = (s->rightbit - shift) & 31;
    s->width = destwidth-1;

    /* This leaves the sprite "non-canonical" if s->width changes, but
     * for our purposes it's good enough.
     */
}
#endif

/* eof */
