/* SaveSprite.c */

/* Saving an anim as a RiscOS sprite file
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 *
 * History:
 *      23-Oct-96 pdh Started
 *      27-Oct-96 pdh Frob to cope with 3/5/6/7bpp anim's
 *      27-Oct-96 *** Release 4beta1
 *      29-Oct-96 pdh Fix mask code so it expands to whole pixels properly
 *      29-Oct-96 *** Release 4beta2
 *      07-Nov-96 *** Release 4
 *      15-Dec-96 *** Release 5beta1
 *      01-Jan-97 pdh Fix bug in saving 2bpp sprites
 *      27-Jan-97 *** Release 5beta2
 *      29-Jan-97 *** Release 5beta3
 *      03-Feb-97 *** Release 5
 *      07-Feb-97 *** Release 5.01
 *      01-Mar-97 pdh Lots of frobbing for new anim library
 *      23-Mar-97 pdh Fix 2bpp/1bpp bugs
 *      05-Apr-97 pdh Fix more 2bpp/1bpp bugs
 *      07-Apr-97 *** Release 6beta1
 *      20-May-97 *** Release 6beta2
 *      24-Aug-97 *** Release 6
 *      27-Sep-97 *** Release 6.01
 *      08-Nov-97 *** Release 6.02
 *      21-Feb-98 *** Release 6.03
 *      07-Jun-98 *** Release 6.04
 *      01-Aug-98 pdh Fix knit.gif problem (not flushing ends of 1bpp lines)
 *      21-Aug-98 *** Release 6.05
 *      19-Feb-99 *** Release 6.07
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "animlib.h"
#include "utils.h"
#include "workspace.h"
#include "sprite.h"

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

    Anim_CheckHeap( __FILE__, line );

    foo = Anim_Allocate(20000);
    if ( foo ) Anim_Free(&foo);
}
#else
#define HEAPTEST
#endif

/* Must be careful here because masked-out pixels may have way-out values */

static void CompressSpriteLine( pixel *dest, const pixel *src, int n, int bpp )
{
    int i;
    pixel j;

    switch ( bpp )
    {
    case 8:
        if ( src != dest )
            memmove( dest, src, n );
        break;

    case 4:
        for ( i=0; i< (n+1)/2; i++ )
            dest[i] = (src[i<<1] & 0xF) + ( src[(i<<1)+1] << 4 ) ;
        break;

    case 2:
        for ( i=0; i < (n+3)/4; i++ )
            dest[i] =    ( ( src[i<<2    ] ) & 3 )
                       | ( ( src[(i<<2)+1] << 2 ) & 0xC )
                       | ( ( src[(i<<2)+2] << 4 ) & 0x30 )
                       | ( src[(i<<2)+3] << 6 );
        break;

    case 1:
        j = 0;
        for ( i=0; i < (n|7)+1; i++ )
        {
            j += (src[i] & 1) << (i&7);
            if ( (i&7) == 7 )
            {
                dest[i>>3] = j;
                j = 0;
            }
        }
        break;
    }
}

static void CompressMaskLine( pixel *dest, const pixel *src, int n, int bpp )
{
    int i;

    switch ( bpp )
    {
    case 8:
        for ( i=0; i<n; i++ )
            dest[i] = ( src[i] ) ? 0xFF : 0;
        break;
    case 4:
        for ( i=0; i< (n+1)/2; i++ )
            dest[i] =     ( src[i<<1]     ? 0xF : 0 )
                      + ( ( src[(i<<1)+1] ? 0xF : 0 ) << 4 );
        break;
    case 2:
        for ( i=0; i < (n+3)/4; i++ )
            dest[i] =    ( src[i<<2    ] ?  0x3 : 0 )
                       + ( src[(i<<2)+1] ?  0xC : 0 )
                       + ( src[(i<<2)+2] ? 0x30 : 0 )
                       + ( src[(i<<2)+3] ? 0xC0 : 0 );
        break;
    case 1:
        CompressSpriteLine( dest, src, n, 1 );      /* It's the same! */
        break;
    }
}

BOOL Anim_SaveSprite( const anim a, FILE *output, int nFrom, int nTo )
{
    spriteareastr sai;
    spritestr *psh;
    int nSpriteSize;
    int i;
    unsigned int y;
    int abw;
    void *data;
    int nFrames = nTo - nFrom + 1;
    int nTotalSize = 16;

    HEAPTEST;

    /* Work out biggest a sprite from this anim can be */

    abw = ( a->nWidth + 3 ) & ~3;           /* aligned byte width for 8bpp */

    nSpriteSize = abw * a->nHeight;         /* bitmap data */
    nSpriteSize *= 2;                       /* times 2 for mask */
    nSpriteSize += 8*256;                   /* 256 palette entries */
    nSpriteSize += sizeof( spritestr );     /* header */

    debugf( "savesprite: asking for %d bytes\n", nSpriteSize );

    HEAPTEST;

    data = Anim_Allocate( nSpriteSize );
    if ( !data )
    {
        Anim_NoMemory( "savesprite" );
        return FALSE;
    }

    /* Some parts of sprite header independent of bpp */

    psh = (spritestr*) data;
    psh->height = a->nHeight-1;
    psh->leftbit = 0;

    fseek( output, 12, SEEK_SET );  /* we do the header last */

    Workspace_Claim(0);

    for ( i=nFrom; i <=nTo; i++ )
    {
        frame f = a->pFrames+i;
        pixel *src;
        pixel *dest;
        palette p = f->pal;
        const unsigned int *pSrcPal = p->pColours;
        unsigned int *pDestPal = (unsigned int*)(psh+1);
        int delay;
        unsigned int nColours = p->nColours;
        int bpp = 8;

        debugf( "Saving frame %d\n", i );

        if ( nColours <= 16 )
            bpp = 4;
        if ( nColours <= 4 )
            bpp = 2;
        if ( nColours <= 2 )
            bpp = 1;

        abw = ( ( ( a->nWidth * bpp ) + 31 ) & ~31) >> 3;

        /* Size of this particular sprite */

        nSpriteSize = abw * a->nHeight;
        if ( f->pMaskData )
            nSpriteSize *= 2;
        nSpriteSize += (1<<bpp)*8 + sizeof(spritestr);

        psh->nNextOffset = nSpriteSize;
        psh->width = (abw>>2)-1;
        psh->rightbit = (a->nWidth*bpp - 1 ) & 31;
        psh->imageoffset = sizeof(spritestr) + 8*(1<<bpp);
        if ( f->pMaskData )
            psh->maskoffset = psh->imageoffset + abw*a->nHeight;
        else
            psh->maskoffset = psh->imageoffset;

        switch ( bpp )
        {
        case 1: psh->screenmode = 18; break;
        case 2: psh->screenmode = 19; break;
        case 4: psh->screenmode = 20; break;
        case 8: psh->screenmode = 21; break;
        }

        memset( psh->name, 0, 12 );
        delay = f->csDelay;
        if ( delay > 0 )
            sprintf( psh->name, "%03ddelay%d", i, delay );
        else
            sprintf( psh->name, "%03d", i );

        memset( pDestPal, 0, 8*(1<<bpp) );

        for ( y=0; y < nColours; y++ )
        {
            *pDestPal++ = pSrcPal[y];
            *pDestPal++ = pSrcPal[y];
        }

        /* Excessive cheating here: we decompress the frame into the sprite
         * itself. This works because we've always got enough memory here.
         */

        dest = ((pixel*)psh) + psh->imageoffset;
        src = dest + a->nHeight*((4-a->nWidth) & 3); /* {0,1,2,3}->{0,3,2,1} */

        debugf( "ass: a->nWidth=%d abw=%d\n", a->nWidth, abw );

        HEAPTEST;

        if ( !Anim_Decompress( f->pImageData, f->nImageSize,
                               a->nWidth*a->nHeight, src ) )
        {
            Anim_Free( &data );
            Workspace_Release();
            return FALSE;
        }

        HEAPTEST;

        for ( y=0; y < a->nHeight; y++ )
        {
            CompressSpriteLine( dest, src, a->nWidth, bpp );
            dest += abw;
            src += a->nWidth;
        }

        HEAPTEST;

        if ( f->pMaskData )
        {
            dest = ((pixel*)psh) + psh->maskoffset;
            src = dest + a->nHeight * ((4-a->nWidth) & 3);

            if ( !Anim_Decompress( f->pMaskData, f->nMaskSize,
                                   a->nWidth*a->nHeight, src ))
            {
                Anim_Free( &data );
                Workspace_Release();
                return FALSE;
            }

            HEAPTEST;

            for ( y=0; y < a->nHeight; y++ )
            {
                CompressMaskLine( dest, src, a->nWidth, bpp );
                dest += abw;
                src += a->nWidth;
            }

            HEAPTEST;
        }

        fwrite( data, 1, nSpriteSize, output );

        nTotalSize += nSpriteSize;
    }

    Workspace_Release();

    /* Write the header */
    sai.nSprites = nFrames;
    sai.nFirstOffset = 16;
    sai.nFreeOffset = nTotalSize;
    fseek( output, 0, SEEK_SET );
    fwrite( &sai.nSprites, 1, 12, output );

    Anim_Free( &data );
    return TRUE;
}
