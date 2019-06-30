/* SaveGIF.c */

/* GIF compressor code
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 *
 * History:
 *      18-Aug-95 pdh Took BSD compress(1) to pieces to provide a C version
 *                    of SWI Squash_Compress (cf 3PRM v4 p102)
 *      10-Dec-95 pdh 8 bit GIF algorithm
 *      13-Dec-95 pdh Adapted even more for 4 bit and interlaced GIFs
 *      14-Dec-95 *** Release 1.00
 *      03-Apr-96 pdh Add compression of multiple sprites into Netscape 2
 *                    animated GIF format
 *      03-Apr-96 pdh Added allowance for LH wastage in sprites
 *      04-Apr-96 pdh Add bLoop, csDelay
 *      04-Apr-96 *** Release 2.00
 *      21-May-96 pdh Fix csDelay so it actually works
 *      21-May-96 *** Release 2.01
 *      11-Aug-96 pdh Allow for 2-bit and 1-bit GIFs
 *      11-Aug-96 pdh Reduce #colours in dest if not all used in src
 *      11-Aug-96 *** Release 2.02
 *      23-Aug-96 pdh Modularise, remove all RiscOS specific bits
 *      23-Aug-96 pdh Changed-rectangle support
 *      25-Aug-96 pdh Write GIF straight to a file
 *      25-Aug-96 *** Release 3.00
 *      01-Sep-96 pdh Add 'one frame per file' option (-split)
 *      01-Sep-96 *** Release 3.01
 *      25-Sep-96 pdh Change loop count to 0x7FFF (from 0)
 *      26-Oct-96 pdh Groovy new transparency optimisation
 *      27-Oct-96 *** Release 4beta1
 *      29-Oct-96 *** Release 4beta2
 *      07-Nov-96 *** Release 4
 *      17-Nov-96 pdh Fixed disposal of last frame of looped animation
 *      15-Dec-96 *** Release 5beta1
 *      01-Jan-97 pdh Rewrote transparency optimiser again
 *      27-Jan-97 *** Release 5beta2
 *      29-Jan-97 *** Release 5beta3
 *      02-Feb-97 pdh Modified handling of bForceTrans (cannonspr bug)
 *      02-Feb-97 pdh Call to Anim_Percent
 *      03-Feb-97 *** Release 5
 *      07-Feb-97 *** Release 5.01
 *      02-Mar-97 pdh Frob a lot for new anim library
 *      07-Apr-97 *** Release 6beta1
 *      20-May-97 pdh Fix bug with -split
 *      20-May-97 *** Release 6beta2
 *      27-May-97 pdh Remove bForceTrans/bUseTrans
 *      01-Jun-97 pdh Split up some big allocate() buffers
 *      21-Aug-97 pdh Reinstate transparent borders on first frame (see below)
 *      24-Aug-97 *** Release 6
 *      27-Sep-97 pdh Add bForceDelay field to anim_GIFflags
 *      27-Sep-97 *** Release 6.01
 *      07-Nov-97 pdh Changed transparent borders behaviour again (see below)
 *      08-Nov-97 *** Release 6.02
 *      21-Feb-98 *** Release 6.03
 *      07-Jun-98 *** Release 6.04
 *      21-Aug-98 *** Release 6.05
 *      19-Feb-99 *** Release 6.07
 *
 *
 * GIF sizes
 * ---------
 *
 * Since at least version 2.02, InterGif has optimised out any transparent
 * border on the first (or only) frame of transparent GIFs. It does this by
 * setting the size in the Logical Screen Descriptor to the size of the whole
 * GIF, and the size in the first Frame Descriptor to the smaller rectangle
 * which bounds the first frame. This is all completely as per GIF spec, and
 * is what happens for the second and subsequent frames of animated GIFs
 * anyway.
 *      However, some programs which read GIFs (usually those which either
 * don't understand animations, or don't understand transparency) incorrectly
 * ignore the LSD size and use the FD size. These programs include ChangeFSI,
 * Claris HomePage, and early versions of Fresco (before 1.60). This is a
 * problem as it can lead to Web authors specifying the wrong width= and
 * height= attributes in Web pages. All versions of Netscape and MSIE use
 * the LSD size (at least for GIF89's).
 *      Early versions of Creator only set the FD size and not the LSD size;
 * such images look wrong in MSIE. Netscape cheats! and uses only the FD size
 * for GIF87 images and (correctly) the LSD size for GIF89 images. As of
 * version 1.63, this is Fresco's behaviour too.
 *
 *      Newsflash, InterGif 6.02... Netscape Communicator (Netscape 4) gets it
 * wrong if an image is (a) transparent (b) interlaced and (c) has a border
 * optimised out on the first frame. The symptom is that black, non-transparent
 * lines appear every fourth pixel down "transparent" areas of the image. This
 * is *unquestionably* a bug in Communicator rather than InterGif (especially
 * as Netscape 3 gets it right), but, powerless as I am in the face of Netscape
 * Corporation, I've stopped InterGif from optimising out the border if an
 * interlaced GIF is being made.
 *      This means that such GIFs end up being compressed less optimally than
 * they might. If this is a problem (and it may not be, as interlaced GIFs
 * usually end up compressed less well than non-interlaced ones anyway) you can
 * use the new -trim option to *remove*, rather than just avoid compressing,
 * the transparent border. When using -trim, InterGif's output will *not* be
 * the same size in pixels as the input image (it is in all other cases).
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "animlib.h"
#include "gifencode.h"
#include "frame.h"
#include "count.h"
#include "utils.h"
#include "workspace.h"

#if 0
#define debugf printf
#define DEBUG 1
#else
#define debugf 1?0:printf
#define DEBUG 0
#endif


/*--------------------*
 * Parts of GIF files *
 *--------------------*/

static char GIFheader[6] = { 'G','I','F','8','7','a' };
#define gifheader_VERSION 4

static char GIFscreen[7];
#define gifscreen_XLO 0
#define gifscreen_XHI 1
#define gifscreen_YLO 2
#define gifscreen_YHI 3
#define gifscreen_FLAGS 4

static char GIFtrans[8] = { '!', (char)0xF9, 4, 1, 0, 0, 0, 0 };
#define giftrans_FLAGS 3
#define giftrans_DELAYLO 4
#define giftrans_DELAYHI 5
#define giftrans_PIXEL 6

static char GIFimage[10] = { ',', 0 };
#define gifimage_XOFFLO 1
#define gifimage_XOFFHI 2
#define gifimage_YOFFLO 3
#define gifimage_YOFFHI 4
#define gifimage_XLO 5
#define gifimage_XHI 6
#define gifimage_YLO 7
#define gifimage_YHI 8
#define gifimage_FLAGS 9

static char GIFloop[19] = {
    '!', (char)0xFF, 11,
    'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0',
    3, 1,
    0, 0, /* 2 byte loop count == 0 (infinite) */
    0
};


/*--------------------*
 * Forward references *
 *--------------------*/

static BOOL GIF__GetFrame( anim a, int i, pixel *image, pixel *mask,
                           anim_GIFflags flags );

static int GIFCompressFrame( anim a, pixel *img, int bpp,
                             char *linebuf, BOOL bInterlace, void *pTo,
                             rect *pRect );


/*---------------------------------------------------------------------------*
 * Anim_SaveGIF                                                              *
 * Send a GIF to the given output stream                                     *
 *---------------------------------------------------------------------------*/

BOOL Anim_SaveGIF( anim a, anim_GIFflags flags, FILE *f, int nFrom, int nTo )
{
    char *to;
    unsigned int sizeafter;
    int nBytes;
    int x, y;
    unsigned int *GCT;
    int nGCTcol;
    char *ptr;
    char *linebuf;
    int i;
    rect r,r2;
    rect lastrect;
    BOOL bFrameTrans;
    int nFrameTransPixel;
    int disposal;
    int lastdisposal;
    BOOL ok = TRUE;
    pixel *imgbuffer[3];
    pixel *maskbuffer[3];
    int prevframe = 0, thisframe = 1, nextframe = 2;
    palette palGlobal = a->pFrames->pal;        /* first frame's palette */
    int nFrames = nTo - nFrom + 1;
    int nTotalCol;

    x = a->nWidth;
    y = a->nHeight;
    nBytes = x*y;

    for ( i=0; i<3; i++ )
    {
        imgbuffer[i] = Anim_Allocate( nBytes );
        maskbuffer[i] = Anim_Allocate( nBytes );
        if ( !imgbuffer[i] || !maskbuffer[i] )
            ok = FALSE;
    }

    to =      Anim_Allocate( nBytes*2 );
    linebuf = Anim_Allocate( x );

    if ( !to || !ok || !linebuf )
    {
        Anim_Free( &to );
        for ( i=0; i<3; i++ )
        {
            Anim_Free( &imgbuffer[i] );
            Anim_Free( &maskbuffer[i] );
        }
        Anim_Free( &linebuf );
        Anim_NoMemory( "gifcomp" );
        return FALSE;
    }

    /* Write header */

    GIFheader[gifheader_VERSION] = ( a->pFrames[0].pMaskData
                                     || nFrames>0 ) ? '9' : '7';
    fwrite( GIFheader, 1, 6, f );

    /* Write Logical Screen Descriptor ... */

    nGCTcol = palGlobal->nColours;
    GCT = palGlobal->pColours;

    nTotalCol = 1 << MinBpp( nGCTcol );

    debugf( "amg: nGCTcol=%d nTotalCol=%d choosing %dbpp\n",
            nGCTcol, nTotalCol, MinBpp(nTotalCol) );

    GIFscreen[gifscreen_XLO] = x & 0xFF;
    GIFscreen[gifscreen_XHI] = x >> 8;
    GIFscreen[gifscreen_YLO] = y & 0xFF;
    GIFscreen[gifscreen_YHI] = y >> 8;
    GIFscreen[gifscreen_FLAGS] = 0xF0 + MinBpp(nTotalCol) - 1;
    fwrite( GIFscreen, 1, 7, f );

    /* ... including palette */

    ptr = to;
    memset( to, 0, nGCTcol*3 );
    for ( i=0; i < nGCTcol; i++ )
    {
        *ptr++ = GCT[i]>>8;
        *ptr++ = GCT[i]>>16;
        *ptr++ = GCT[i]>>24;
    }

    fwrite( to, 1, nGCTcol*3, f );

    for ( i = nGCTcol; i < nTotalCol; i++ )
    {
        fwrite( "ig\x67", 1, 3, f );    /* 6.07 in BCD... */
    }

    if ( nFrames > 1 && flags.bLoop )
    {
        /* Write Netscape extension */
        fwrite( GIFloop, 1, 19, f );
    }

    /* Write the frames */

    lastdisposal = 0;

    Workspace_Claim(0);

    for ( i = nFrom; i <= nTo && ok; i++ )
    {
        /* Trans5 algorithm */
        frame fr = a->pFrames + i;
        int next = i+1;

        if ( next > nTo )
            next = nFrom;

        if ( i )
            Anim_Percent( i*100 / nFrames );

        if ( i == nFrom )
        {
            debugf( "amg: opening frame %d as thisframe\n", i );

            if ( !GIF__GetFrame( a, i, imgbuffer[thisframe],
                                 maskbuffer[thisframe], flags ) )
            {
                ok = FALSE;
                break;
            }
        }

        bFrameTrans = FALSE;
        nFrameTransPixel = -1;

        /* Must be done FIRST to get the arnie case right */
        if ( ( nFrames > 1 )
              && ( i < nTo || flags.bLoop ) )
        {
            debugf( "amg: opening frame %d as nextframe\n", next );

            if ( !GIF__GetFrame( a, next, imgbuffer[nextframe],
                                 maskbuffer[nextframe], flags ) )
            {
                ok = FALSE;
                break;
            }

            if ( fr->pal == a->pFrames[next].pal )
            {
                /* Find bounding box of all pixels which are solid in thisframe
                 * but transparent in nextframe */

                BitMaskFindTransRect( a, maskbuffer[thisframe],
                                      maskbuffer[nextframe], &r2 );

                debugf( "amg: transrect is (%d,%d)-(%d,%d)\n",
                        r2.xoff, r2.yoff, r2.xoff+r2.xsize, r2.yoff+r2.ysize );

                if ( r2.xsize == 0 )
                    disposal = 1;
                else
                    disposal = 2;
            }
            else
            {
                debugf( "amg: palettes incompatible, compressing whole frame\n" );
                disposal = 2;
                r2.xsize = 0;
                r2.ysize = 0;
            }
        }
        else
        {
            disposal = 1;
            r2.xsize = 0;
            r2.ysize = 0;
        }

        if ( i > nFrom
             && fr->pal == a->pFrames[i-1].pal )
        {
            debugf( "amg: opening frame %d as prevframe\n", i-1 );

            if ( !GIF__GetFrame( a, i-1, imgbuffer[prevframe],
                                 maskbuffer[prevframe], flags ) )
            {
                ok = FALSE;
                break;
            }

            if ( lastdisposal == 2 )
                BitMaskClearRectangle( a, maskbuffer[prevframe], &lastrect );

            ChangedRect( a, imgbuffer[thisframe], maskbuffer[thisframe],
                            imgbuffer[prevframe], maskbuffer[prevframe], &r );

            debugf( "amg: changed rect is (%d,%d)-(%d,%d)\n",
                    r.xoff, r.yoff, r.xoff+r.xsize, r.yoff+r.ysize );

            if ( BitMaskAnyTransparent( a, maskbuffer[thisframe], &r ) )
            {
                bFrameTrans = TRUE;
                BitMaskEqualPixels( a, imgbuffer[thisframe],
                                       maskbuffer[thisframe],
                                       imgbuffer[prevframe],
                                       maskbuffer[prevframe] );
                nFrameTransPixel =
                    BitMaskFindTransPixel( a, imgbuffer[thisframe],
                                           maskbuffer[thisframe], TRUE );
            }
            else
            {
                nFrameTransPixel =
                    BitMaskFindTransPixel( a, imgbuffer[thisframe],
                                           maskbuffer[thisframe], FALSE );

                if ( nFrameTransPixel != -1 )
                {
                    bFrameTrans = TRUE;
                    BitMaskEqualPixels( a, imgbuffer[thisframe],
                                           maskbuffer[thisframe],
                                           imgbuffer[prevframe],
                                           maskbuffer[prevframe] );
                }
            }

            if ( bFrameTrans )
            {
                BitMaskToPixMask( a, imgbuffer[thisframe],
                                  maskbuffer[thisframe], nFrameTransPixel );
                PixMaskOptimiseRectangle( a, imgbuffer[thisframe],
                                             maskbuffer[thisframe],
                                             imgbuffer[prevframe],
                                             maskbuffer[prevframe],
                                          nFrameTransPixel, &r );
            }
        }
        else
        {
            /* First frame, or incompatible palette */
            r.xoff =
                r.yoff = 0;
            r.xsize = x;
            r.ysize = y;

            if ( BitMaskAnyTransparent( a, maskbuffer[thisframe], &r ) )
            {
                bFrameTrans = TRUE;
                nFrameTransPixel =
                    BitMaskFindTransPixel( a, imgbuffer[thisframe],
                                           maskbuffer[thisframe], TRUE );

                /* pdh: this stops InterGif from optimising out transparent
                 * transparent borders on the first frame of interlaced GIFs.
                 * See "GIF sizes" above for more information.
                 */
                if ( i>nFrom || !flags.bInterlace )
                    BitMaskTrimTransparentBorders( a, maskbuffer[thisframe],
                                                   &r );

                BitMaskToPixMask( a, imgbuffer[thisframe],
                                  maskbuffer[thisframe], nFrameTransPixel );
            }
        }


        Rect_Union( &r, &r2 );

        /* r is now the rectangle of this frame that wants compressing */

        if ( r.xsize == 0 )
            r.xsize = 1;
        if ( r.ysize == 0 )
            r.ysize = 1;

        if ( fr->pal == palGlobal )
        {
            ptr = to;
            if ( bFrameTrans || nFrames > 1 )
            {
                /* Write Graphic Format Extension */

                int csDelay = fr->csDelay;

                if ( flags.bForceDelay )
                    csDelay = flags.nDefaultDelay;

                GIFtrans[giftrans_FLAGS] = (disposal << 2)
                                            | (bFrameTrans ? 1 : 0);
                GIFtrans[giftrans_DELAYLO] = csDelay & 0xFF;
                GIFtrans[giftrans_DELAYHI] = csDelay >> 8;
                GIFtrans[giftrans_PIXEL] = nFrameTransPixel;
                memcpy( ptr, GIFtrans, 8 );
                ptr += 8;
            }

            /* Write frame data */

            GIFimage[gifimage_XOFFLO] = r.xoff & 0xFF;
            GIFimage[gifimage_XOFFHI] = r.xoff >> 8;
            GIFimage[gifimage_YOFFLO] = r.yoff & 0xFF;
            GIFimage[gifimage_YOFFHI] = r.yoff >> 8;
            GIFimage[gifimage_XLO] = r.xsize & 0xFF;
            GIFimage[gifimage_XHI] = r.xsize >> 8;
            GIFimage[gifimage_YLO] = r.ysize & 0xFF;
            GIFimage[gifimage_YHI] = r.ysize >> 8;
            GIFimage[gifimage_FLAGS] = (flags.bInterlace && i==nFrom) ? 0x40
                                                                      : 0;
            memcpy( ptr, GIFimage, 10 );
            ptr += 10;

            debugf( "%03d: %dx%d at (%d,%d) dis=%d",
                    i, r.xsize, r.ysize, r.xoff, r.yoff, disposal );

            if ( bFrameTrans )
                debugf( " trans=%d", nFrameTransPixel );

            sizeafter = GIFCompressFrame( a, imgbuffer[thisframe],
                                          MinBpp(nTotalCol),
                                          linebuf, flags.bInterlace && i==nFrom,
                                          ptr, &r );
            ptr += sizeafter;

            debugf( " size=%4d\n", sizeafter );

            fwrite( to, 1, ptr-to, f );
        }
        else
        {
            Anim_SetError( "Please choose a palette-reduction option when using animations with more than 256 colours" );
            ok = FALSE;
        }

        lastrect = r;
        lastdisposal = disposal;

        debugf( "Finished writing frame\n" );

        thisframe = (thisframe+1) % 3;
        prevframe = (prevframe+1) % 3;
        nextframe = (nextframe+1) % 3;

        /* if !ok then fall out */
    }

    Workspace_Release();

    fwrite( ";", 1,1, f );

    Anim_Free( &to );
    for ( i=0; i<3; i++ )
    {
        Anim_Free( &imgbuffer[i] );
        Anim_Free( &maskbuffer[i] );
    }
    Anim_Free( &linebuf );
    return ok;
}


/*---------------------------------------------------------------------------*
 * GIF__GetFrame()                                                           *
 * Decompresses a frame into the image and mask buffers, setting the mask as *
 * appropriate depending on the flags.                                       *
 *---------------------------------------------------------------------------*/

static BOOL GIF__GetFrame( anim a, int i, pixel *image, pixel *mask,
                           anim_GIFflags flags )
{
    frame f = a->pFrames + i;
    unsigned int nBytes = a->nWidth * a->nHeight;
    BOOL ok;

    if ( f->pMaskData )
        ok = Anim_Decompress( f->pMaskData, f->nMaskSize, nBytes, mask );
    else
    {
        memset( mask, 1, nBytes );     /* all solid */
        ok = TRUE;
    }

    if ( ok )
        ok = Anim_Decompress( f->pImageData, f->nImageSize, nBytes, image );

    return ok;
}


/*---------------------------------------------------------------------------*
 * Compressing one frame into a GIF image block                              *
 * Uses GIFLZW compression -- the compression routine (in giflzw.c) takes    *
 * a "fetcher" function (and handle) to supply it with its source bytestream *
 * The fetcher function is getnextline, and the handle is a GIF_linedata*    *
 *---------------------------------------------------------------------------*/

typedef struct {
    BOOL bDone;         /* Finished yet? */
    BOOL bInterlace;    /* Interlaced? (affects the order we supply lines) */
    int pass;           /* Interlacing pass */
    unsigned int ypos;           /* Next line to give */
    char *buffer;       /* Line buffer */
    anim a;             /* Animation */
    pixel *img;         /* And frame */
    rect r;             /* Rectangle of frame to compress */
} GIF_linedata;


/* Data for interlacing */

#define GIFPASSES 4
static const char starts[GIFPASSES]  = { 0,4,2,1 };    /* starts[0] not used */
static const char offsets[GIFPASSES] = { 8,8,4,2 };


/* Supply another lump of data (one line of bitmap) to the compressor */

#if DEBUG
static void debugstream( const char *p, int n )
{
    int j=0;
    while ( n-- )
    {
        debugf( j ? " %d" : "[%d", (int)*p );
        p++;
        j=1;
    }
    debugf( "]" );
}
#else
#define debugstream(p,n) /* skip */
#endif

static unsigned char *getnextline( void *handle, int *pSize )
{
    GIF_linedata *pLine = (GIF_linedata*) handle;
    int w = pLine->a->nWidth;
    pixel *pImage = pLine->img;
    int offset = (pLine->ypos + pLine->r.yoff)*w;

    pImage += offset;

    if ( pLine->bDone )
    {
        *pSize = 0;
        return NULL;
    }

    if ( pLine->bInterlace )
    {
        pLine->ypos += offsets[pLine->pass];
        while ( pLine->ypos >= pLine->r.ysize )
        {
            pLine->pass++;
            if ( pLine->pass == GIFPASSES )
            {
                pLine->bDone = TRUE;
                break;
            }
            pLine->ypos = starts[pLine->pass];
        }
    }
    else
    {
        pLine->ypos++;
        if ( pLine->ypos == pLine->r.ysize )
            pLine->bDone = TRUE;
    }

    *pSize = pLine->r.xsize;

#if 0
    if ( pLine->ypos == 1 )
    {
        debugf( "Sending " );
        debugstream( pImage + pLine->r.xoff, 10 );
    }
#endif

    return (unsigned char*)(pImage + pLine->r.xoff);
}

/* Compress one frame */

static int GIFCompressFrame( anim a, pixel *img, int bpp,
                             char *linebuf, BOOL bInterlace, void *pTo,
                             rect *r )
{
    GIF_linedata line;
    int n_bits;

    line.bDone = FALSE;
    line.bInterlace = bInterlace;
    line.pass = 0;
    line.ypos = 0;
    line.buffer = linebuf;
    line.a = a;
    line.img = img;
    line.r = *r;

    n_bits = bpp+1;
    if ( n_bits == 2 )
        n_bits = 3;     /* avoid degenerate case */

    debugf( "compressing, n_bits=%d\n", n_bits );

    return LZWCompress( n_bits, pTo, &getnextline, (void*)&line, NULL );
}
