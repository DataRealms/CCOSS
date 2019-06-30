/* LoadGIF.c */

/* GIF decompressor code
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 *
 * History:
 *      25-Sep-96 pdh Started
 *      27-Oct-96 *** Release 4beta1
 *      29-Oct-96 *** Release 4beta2
 *      07-Nov-96 pdh Fix bug with transparent GIF input
 *      07-Nov-96 *** Release 4
 *      15-Dec-96 *** Release 5beta1
 *      27-Jan-97 *** Release 5beta2
 *      29-Jan-97 *** Release 5beta2
 *      02-Feb-97 pdh Improve error for >256 colours
 *      03-Feb-97 *** Release 5
 *      07-Feb-97 *** Release 5.01
 *      27-Feb-97 pdh Lots of frobbing for new anim library
 *      04-Apr-97 pdh Sort out "altavista.gif" problems
 *      07-Apr-97 *** Release 6beta1
 *      20-May-97 pdh Stop "too much data" being an error (in gifdecode.c)
 *      20-May-97 pdh Cleverer GIF__Image copes with more >256 GIFs.
 *      20-May-97 *** Release 6beta2
 *      01-Jun-97 pdh Split up some big allocate() buffers
 *      24-Aug-97 *** Release 6
 *      27-Sep-97 *** Release 6.01
 *      06-Oct-97 pdh Fix GIFs where header lies about anim size, "nsnow.gif"
 *      08-Nov-97 *** Release 6.02
 *      15-Feb-98 pdh Support palette-mappers, bHistogramOnly
 *      19-Feb-98 pdh Fix "Be a movie star" bug, see GIF__Image()
 *      21-Feb-98 *** Release 6.03
 *      07-Jun-98 *** Release 6.04
 *      01-Aug-98 pdh anim_imagefn stuff
 *      21-Aug-98 pdh Reinstate >256 support
 *      21-Aug-98 *** Release 6.05
 *      05-Oct-98 *** Release 6.06
 *	15-Oct-98 pdh Fix GIF__Write for systems where char is signed
 *      19-Feb-99 *** Release 6.07
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include "animlib.h"
#include "utils.h"
#include "gifdecode.h"
#include "gifencode.h"
#include "workspace.h"

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

    Anim_CheckHeap(__FILE__, line);

    foo = Anim_Allocate(20000);
    if ( foo ) Anim_Free(&foo);
}
#else
#define HEAPTEST
#endif

    /*================================*
     *   State kept during decoding   *
     *================================*/


typedef struct {
    const unsigned char *pGCX;  /* Current Graphic Control Extension, or NULL */
    const unsigned char *pLastGCX;   /* Previous, or NULL */
    const unsigned char *pLastImage;
    BOOL bLoop;
    BOOL bDeep;                 /* >256 distinct colours found? */
    int nFrames;
    pixel bgpixel;
    unsigned int bgcolour;
    unsigned int xs, ys;
    unsigned int CT[256];
    const unsigned char *pGCT;
    unsigned int nGCTcolours;
    pixel *image;
    unsigned int *image32;
    pixel *mask;
    pixel *raw;
    const unsigned char *pGif;
} GIFinfo;


    /*========================*
     *   Forward references   *
     *========================*/


static void GIF__Header( unsigned char **ptr, GIFinfo *gif,
                         unsigned char *end );
static void GIF__Extension( unsigned char **pptr, GIFinfo *gif );
static void GIF__SkipPackets( unsigned char **pptr, int nSkip );
static BOOL GIF__Image( anim_imagefn fn, void *handle, unsigned char **pptr,
                        GIFinfo *gif );
static void GIF__FrameRectangle( const unsigned char *img, rect *r );
static BOOL GIF__DecodeFrame( const unsigned char *img, pixel *out,
                              int outsize );
static void GIF__Write( pixel *out, pixel *mask, const pixel *in,
                        int ow, int oh, const rect *framerect,
                        int transpixel, BOOL bInterlace, pixel *lctmap,
                        BOOL bRealThing );
static void GIF__Write32( unsigned int *out, pixel *mask, const pixel *in,
                          int ow, int oh, const rect *framerect,
                          int transpixel, BOOL bInterlace, unsigned int *ct );
static void GIF__Unwrite( pixel *out, pixel *mask, int ow, int oh,
                          const rect *framerect, int disposal,
                          int bg, BOOL trans );
static void GIF__Unwrite32( unsigned int *out, pixel *mask, int ow, int oh,
                            const rect *framerect, int disposal,
                            unsigned int bg, BOOL trans );


    /*=====================*
     *   Anim_ConvertGIF   *
     *=====================*/


BOOL Anim_ConvertGIF( const void *pvGif, unsigned int size,
                      anim_animfn animfn, anim_imagefn fn, void *handle )
{
    unsigned char *ptr = (unsigned char*) pvGif;
    unsigned char *end = ptr + size;
    GIFinfo gif;
    int bufsize;
    BOOL doneanimfn = FALSE;
    BOOL result = FALSE;

    /* Check it's a GIF file, if not return FALSE */
    if ( strncmp( pvGif, "GIF", 3 ) )
    {
        debugf( "It's not a GIF\n" );
        return FALSE;
    }

    memset( &gif, 0, sizeof(gif) );

    GIF__Header( &ptr, &gif, end );

    bufsize = gif.xs * gif.ys;

    gif.image = Anim_Allocate( bufsize );
    gif.mask = Anim_Allocate( bufsize );
    gif.raw = Anim_Allocate( bufsize + 32 );    /* the "+32" is slop */

    gif.pGif = pvGif;

    if ( !gif.image || !gif.mask || !gif.raw )
    {
        Anim_NoMemory( "animfromgif" );
        goto err;
    }

    Workspace_Claim( gifcompress_WORKSIZE );

    while ( ptr < end && *ptr != ';' )
    {
        switch ( *ptr )
        {
        case '!':
            GIF__Extension( &ptr, &gif );
            break;
        case ',':
            if ( !doneanimfn )
            {
                if ( animfn && !(*animfn)( handle, gif.xs, gif.ys, gif.bLoop ) )
                {
                    debugf( "convertgif: animfn returned FALSE\n" );
                    goto err2;
                }
                doneanimfn = TRUE;
            }

            if ( !GIF__Image( fn, handle, &ptr, &gif ) )
                goto err2;

            HEAPTEST;

            break;
        default:
            /* Bad gif file, but try and recover */
            debugf( "%08x: *** Something dodgy\n", ptr - gif.pGif );

            ptr++;
        }
    }

    result = TRUE;

err2:
    Workspace_Release();

err:
    Anim_Free( &gif.image32 );
    Anim_Free( &gif.image );
    Anim_Free( &gif.mask );
    Anim_Free( &gif.raw );

    HEAPTEST;

    return result;
}

static void GIF__CheckSize( unsigned char *stream, unsigned char *end,
                            unsigned int *pWidth, unsigned int *pHeight )
{
    rect framerect;
    BOOL bLCT;
    int nLCTcolours;

    while ( stream < end && *stream != ';' )
    {
        switch ( *stream )
        {
        case '!':
            GIF__SkipPackets( &stream, 2 );
            break;
        case ',':
            GIF__FrameRectangle( stream+1, &framerect );

            if ( framerect.xoff + framerect.xsize > *pWidth )
            {
                *pWidth = framerect.xoff + framerect.xsize;
                debugf( "gifchecksize: adjusted width to %d\n", *pWidth );
            }

            if ( framerect.yoff + framerect.ysize > *pHeight )
            {
                *pHeight = framerect.yoff + framerect.ysize;
                debugf( "gifchecksize: adjusted height to %d\n", *pHeight );
            }

            bLCT = (stream[9] & 0x80) != 0;   /* local colour table? */
            nLCTcolours = bLCT ? ( 2 << ( stream[9] & 7 ) ) : 0;
            stream += 10 + nLCTcolours*3;
            GIF__SkipPackets( &stream, 1 );
            break;
        default:
            stream++;
            break;
        }
    }
}

/*
 * GIF__Header()
 * Decode the Logical Screen Descriptor
 */

static void GIF__Header( unsigned char **ptr, GIFinfo *gif,
                         unsigned char *end )
{
    unsigned char *head = *ptr;
    BOOL bGCT;
    int cres;
    int bpp;

    debugf( "%08X GIF header version %3.3s\n", 0, head+3 );

    head += 6;  /* points to Logical Screen Descriptor */
    gif->xs = head[0] + 256*head[1];
    gif->ys = head[2] + 256*head[3];

    bpp = (head[4] & 7)+1;
    cres = ((head[4] & 0x70)>>4) + 1;
    bGCT = ( head[4] & 0x80 );

    gif->nGCTcolours = bGCT ? (1<<bpp) : 0;

    debugf( "%08X   Screen size %dx%d gctsize=%dbpp color-res=%dbpp %s\n",
            0, gif->xs, gif->ys, bpp, cres, bGCT ? "gct-present" : "" );
    debugf( "%08X   Background %d, aspect ratio %d\n", 0, head[5],
            head[6] );

    gif->bgpixel = head[5];

    if ( bGCT )
    {
        unsigned char *src;

        gif->pGCT = head+7;

        src = head+7 + 3*gif->bgpixel;
        gif->bgcolour = ( src[2] << 24 ) + (src[1] << 16) + (*src << 8 );
    }

    *ptr = head + 7 + (gif->nGCTcolours)*3;

    /* Aaaaaaaarrrrrrrrgh, the header may have LIED to us (hmm, the "Netscape
     * now" animation does this: a deliberate plot to blow up other browsers?)
     */

    GIF__CheckSize( *ptr, end, &gif->xs, &gif->ys );
}

static void GIF__ExpandColourTable( unsigned int *dest, const pixel *src,
                                    int n )
{
    int i;

    for ( i=0; i<n; i++ )
    {
        dest[i] = ( src[2] << 24 ) + (src[1] << 16) + (*src << 8 );
        src += 3;
    }
}

static BOOL GIF__MergeColourTable( unsigned int *dest, pixel *destused,
                                   const unsigned char *src,
                                   const pixel *srcused,
                                   unsigned int *pnColours, pixel *pixmap )
{
    unsigned int n = *pnColours;
    unsigned int i, j;
#if DEBUG
    int found=0, added=0;
#endif

    for ( i=0; i<n; i++ )
    {
        unsigned int col = ( src[2] << 24 ) + (src[1] << 16) + (*src << 8 );
        src += 3;

        if ( srcused[i] )
        {
            for ( j=0; j<256; j++ )
            {
                if ( dest[j] == col )
                {
                    pixmap[i] = (pixel) j;
                    destused[j] = 1;
#if DEBUG
                    found++;
/*                     debugf( "gmct: found 0x%08x at %d\n", col, j ); */
#endif
                    break;
                }
            }
            if ( j==256 )
            {
                for ( j=0; j<256; j++ )
                {
                    if ( !destused[j] )
                    {
                        pixmap[i] = (pixel) j;
                        destused[j] = 1;
                        dest[j] = col;
#if DEBUG
                        added++;
/*                         debugf( "gmct: added 0x%08x at %d\n", col, j ); */
#endif
                        break;
                    }
                }
            }
            if ( j==256 )
            {
                /* Oh Dear */
                debugf( "gmct: can't merge 0x%08x!\n", col );
                return FALSE;
            }
        }
    }

#if DEBUG
    debugf( "gmct: found: %d, added: %d, ignored (unused): %d\n",
            found, added, n-found-added );
    debugf( "gmct: <%d", pixmap[0] );
    for ( i=1; i<n; i++ )
        debugf( " %d", pixmap[i] );
    debugf( ">\n" );
#endif

    i = 256;
    do {
        i--;
        if ( destused[i] )
        {
            *pnColours = i+1;
            break;
        }
    } while ( i );

    return TRUE;
}


static void GIF__Extension( unsigned char **pptr, GIFinfo *gif )
{
    unsigned char *gfx = *pptr;
    unsigned char type = gfx[1];

    switch( type )
    {
    case 0xF9: /* Graphic control extension */
        debugf( "%08X: Graphic control extension\n", gfx - gif->pGif );
        debugf( "%08X:   dispose=%d trans=%s\n", gfx - gif->pGif,
                (gfx[3] >> 2) & 7,
                (gfx[3] & 1) ? "yes" : "no" );
        gif->pGCX = gfx;
        break;

    case 0xFF: /* Application extension */
        debugf( "%08X: Application extension\n", gfx - gif->pGif );
        if ( !strncmp( (char*)gfx+3, "NETSCAPE2.0", 11 ) )
        {
            if ( gfx[14] == 3 && gfx[15] == 1 )
            {
                debugf( "%08X:   Netscape looping extension\n",
                        gfx - gif->pGif );

                gif->bLoop = TRUE;
            }
        }
        break;
    }

    GIF__SkipPackets( pptr, 2 );
}


static void GIF__SkipPackets( unsigned char **pptr, int nSkip )
{
    unsigned char *ptr = *pptr;
    unsigned int packlen;

    ptr += nSkip;
    do {
        packlen = *ptr;
        ptr += packlen+1;
    } while ( packlen );
    *pptr = ptr;
}


static BOOL GIF__Image( anim_imagefn fn, void *handle, unsigned char **pptr,
                        GIFinfo *gif )
{
    const unsigned char *gcx = gif->pGCX;
    unsigned char *lctptr;
    unsigned char *img = *pptr;
    unsigned int aw = gif->xs;
    unsigned int ah = gif->ys;
    int transpixel = -1;
    unsigned int i;
    BOOL bInterlace;
    BOOL bTrans = FALSE;
    BOOL bLCT = FALSE;
    int lastdispose = 0;
    rect framerect;
    unsigned int nLCTcolours;
    unsigned int nColours = gif->nGCTcolours;
    int n = gif->nFrames;
    pixel pixmap[256];
    anim_imageinfo pixels, mask;

    debugf( "%08X: Image\n", img - gif->pGif );

    if ( gcx )
    {
        if ( gcx[3] & 1 )
            transpixel = gcx[6];
    }

    if ( n > 0 )
    {
        const unsigned char *lastgcx = gif->pLastGCX;

        if ( lastgcx )
        {
            lastdispose = ( lastgcx[3] >> 2 ) & 7;
            bTrans = ( lastgcx[3] & 1 );
            if ( !bTrans )
                lastdispose = 0;
        }
    }

    if ( n == 0 )
    {
        memset( gif->image, 0, aw*ah );
        memset( gif->mask, 0, aw*ah );    /* transparent */
    }
    else
    {
        const unsigned char *lastimg = gif->pLastImage;
        GIF__FrameRectangle( lastimg+1, &framerect );

        if ( gif->bDeep )
            GIF__Unwrite32( gif->image32, gif->mask, aw, ah, &framerect,
                            lastdispose, gif->bgpixel, bTrans );
        else
            GIF__Unwrite( gif->image, gif->mask, aw, ah, &framerect,
                          lastdispose, gif->bgpixel, bTrans );
    }

    GIF__FrameRectangle( img+1, &framerect );

    debugf( "%08X:   (%d,%d)-(%d,%d)\n", img - gif->pGif,
            framerect.xoff, framerect.yoff,
            framerect.xoff+framerect.xsize, framerect.yoff+framerect.ysize );

    bLCT = (img[9] & 0x80) != 0;   /* local colour table? */
    nLCTcolours = 2 << ( img[9] & 7 );
    bInterlace = (img[9] & 0x40) != 0;

    img += 10;

    for ( i=0; i < 256; i++ )
        pixmap[i] = i;

    lctptr = img;

    if ( bLCT )
    {
        debugf( "%08X:   lct, %d entries\n", img - gif->pGif, nLCTcolours );

        img += nLCTcolours*3;
    }

    debugf( "%08X:   image data\n", img - gif->pGif );

    if ( !GIF__DecodeFrame( img, gif->raw, framerect.xsize*framerect.ysize ) )
        return FALSE;

    if ( !gif->bDeep )
    {
        pixel alreadyused[256];
        pixel newused[256];
        unsigned int x,y;

        /* Determine which of the pixels currently in the image will still be
         * visible when this one is drawn on top.
         */

        memset( alreadyused, 0, 256 );
        memset( newused, 0, 256 );

        if ( n > 0 )
        {
            i = 0;
            for ( y=0; y<ah; y++ )
                for ( x=0; x<aw; x++ )
                {
                    if ( x < framerect.xoff || y < framerect.yoff
                         || (x >= (framerect.xoff+framerect.xsize) )
                         || (y >= (framerect.yoff+framerect.ysize) ) )
                    {
                        if ( gif->mask[i] )
                            alreadyused[ gif->image[i] ] = 1;
                    }
                    i++;
                }

#if DEBUG
            debugf( "gi: alreadyused (rect)" );
            for ( i=0; i < 256; i++ )
                if ( alreadyused[i] )
                    debugf( " %d", i );
            debugf( "\n" );
#endif

            GIF__Write( gif->image, gif->mask, gif->raw, aw, ah, &framerect,
                        transpixel, bInterlace, alreadyused, FALSE );

#if DEBUG
            debugf( "gi: alreadyused (mask)" );
            for ( i=0; i < 256; i++ )
                if ( alreadyused[i] )
                    debugf( " %d", i );
            debugf( "\n" );
#endif
        }

        for ( i=0; i<framerect.xsize*framerect.ysize; i++ )
        {
            pixel p = gif->raw[i];
            if ( p != transpixel )
                newused[p] = 1;
        }

        nColours = bLCT ? nLCTcolours : gif->nGCTcolours;

        debugf( "merging %d colours from %s colour table\n", nColours,
                bLCT ? "local" : "global" );

        if ( GIF__MergeColourTable( gif->CT, alreadyused,
                                    bLCT ? lctptr : gif->pGCT, newused,
                                    &nColours, pixmap ) )
        {
            HEAPTEST;

            GIF__Write( gif->image, gif->mask, gif->raw, aw, ah, &framerect,
                        transpixel, bInterlace, pixmap, TRUE );

            HEAPTEST;
        }
        else
        {
            gif->bDeep = TRUE;
            debugf( "gi: >256 colours! setting bDeep\n" );

            gif->image32 = Anim_Allocate(aw*ah*4);

            if ( !gif->image32 )
            {
                Anim_NoMemory("gifimage32");
                return FALSE;
            }

            /* Expand current 8bpp image into a 32bpp one */
            for ( i=0; i<aw*ah; i++ )
                gif->image32[i] = gif->CT[gif->image[i]] >> 8;

            Anim_Free( &gif->image );
        }

        pixels.nBPP = 8;
        pixels.pBits = gif->image;
        pixels.nLineWidthBytes = aw;
    }

    if ( gif->bDeep )
    {
        unsigned int ct[256];

        if ( bLCT )
            GIF__ExpandColourTable( ct, lctptr, nLCTcolours );
        else
            GIF__ExpandColourTable( ct, gif->pGCT, gif->nGCTcolours );

        GIF__Write32( gif->image32, gif->mask, gif->raw, aw, ah, &framerect,
                      transpixel, bInterlace, ct );
        pixels.nBPP = 32;
        pixels.pBits = gif->image32;
        pixels.nLineWidthBytes = aw*4;
    }

    pixels.nWidth = aw;
    pixels.nHeight = ah;
    pixels.csDelay = gcx ? ( gcx[4] + 256*gcx[5] ) : 8;

    mask.nWidth = aw;
    mask.nLineWidthBytes = aw;
    mask.nHeight = ah;
    mask.nBPP = 8;
    mask.pBits = gif->mask;

    if ( !(*fn)( handle, &pixels, -1, &mask, gif->CT ) )
    {
        debugf( "convertgif: fn returned FALSE, exiting\n" );
        return FALSE;
    }

    HEAPTEST;

    gif->pLastGCX = gcx;
    gif->pGCX = NULL;
    gif->pLastImage = *pptr;
    gif->nFrames++;

    GIF__SkipPackets( &img, 1 );

/*     debugf( "Image at 0x%X next thing at 0x%X [%d]\n", *pptr - gif->pGif, */
/*             img - gif->pGif, *img ); */

    *pptr = img;

    return TRUE;
}

static void GIF__FrameRectangle( const unsigned char *img, rect *r )
{
    r->xoff  = img[0] + 256*img[1];
    r->yoff  = img[2] + 256*img[3];
    r->xsize = img[4] + 256*img[5];
    r->ysize = img[6] + 256*img[7];
}

static BOOL GIF__DecodeFrame( const unsigned char *img, pixel *out,
                              int outsize )
{
    int nBits;
    const unsigned char *ptr;
    int packlen;
    int size;
    unsigned char *data, *dptr;
    BOOL result;

    nBits = *img++;

    size = 0;
    ptr = img;

    debugf( "Packets: <" );

    do {
        packlen = *ptr;
        ptr++;
        ptr += packlen;
        size += packlen;
#if DEBUG
        if ( packlen ) debugf( "%d ", packlen );
#endif
    } while ( packlen );

    debugf( "0> total=%d bits=%d outsize=%d\n", size, nBits,
            outsize );

    HEAPTEST;

    data = Anim_Allocate( size );

    if ( !data )
    {
        Anim_Free( &data );
        Anim_NoMemory( "gifdecodeframe" );
        return FALSE;
    }

    dptr = data;

    ptr = img;

    do {
        packlen = *ptr;
        ptr++;
        memcpy( dptr, ptr, packlen );
        dptr += packlen;
        ptr += packlen;
    } while ( packlen );

    result = LZWDecompress( data, size, out, NULL, outsize, NULL, nBits+1 );

    HEAPTEST;

    Anim_Free( &data );

    debugf("gif__df returns\n");

    return result;
}


/*---------------------------------------------------------------------------*
 * GIF_Write()                                                               *
 * Having decompressed the image, render it (taking account of interlacing   *
 * and disposal method) into the final image buffer                          *
 *-------------------------------------------------------------------------- */

#define GIFPASSES 4
static const char starts[GIFPASSES]  = { 0,4,2,1 };    /* starts[0] not used */
static const char offsets[GIFPASSES] = { 8,8,4,2 };

static void GIF__Write( pixel *out, pixel *mask, const pixel *in,
                        int ow, int oh, const rect *framerect,
                        int transpixel, BOOL bInterlace, pixel *lctmap,
                        BOOL forreal )
{
    int x,y;
    int fxoff,fyoff,fx,fy;
    pixel *ptr, *ptr2;
    pixel pix;
    int ypos = 0;
    int pass = 0;

    fxoff = framerect->xoff;
    fyoff = framerect->yoff;
    fx = framerect->xsize;
    fy = framerect->ysize;

    debugf( "gif__write: (%d,%d)-(%d,%d) ow=%d oh=%d, real=%d\n", fxoff, fyoff, fx,fy ,
            ow, oh, forreal );

    ptr = out + ow*fyoff;
    ptr2 = mask + ow*fyoff;
    for ( y=0; y < fy; y++ )
    {
        for ( x=0; x < fx; x++ )
        {
            pix = *in;
            if ( pix != transpixel )
            {
                /* pixel overwritten */
                if ( forreal )
                {
                    ptr[x+fxoff] = lctmap[pix];
                    ptr2[x+fxoff] = 1;
                }
            }
            else if ( !forreal )
            {
                /* pixel not overwritten */
                lctmap[ ptr[x+fxoff] ] = 1;
            }
            in++;
        }
        if ( bInterlace )
        {
            ypos += offsets[pass];
            while ( ypos >= fy && pass < GIFPASSES )
            {
                pass++;
                ypos = starts[pass];
            }
            ptr = out + ow*(fyoff+ypos);
            ptr2 = mask + ow*(fyoff+ypos);
        }
        else
        {
            ptr += ow;
            ptr2 += ow;
        }
    }
    debugf("gifwrite returns\n");
}

static void GIF__Write32( unsigned int *out, pixel *mask, const pixel *in,
                          int ow, int oh, const rect *framerect,
                          int transpixel, BOOL bInterlace, unsigned int *ct )
{
    int x,y;
    int fxoff,fyoff,fx,fy;
    unsigned int *ptr;
    pixel *ptr2;
    pixel pix;
    int ypos = 0;
    int pass = 0;

    fxoff = framerect->xoff;
    fyoff = framerect->yoff;
    fx = framerect->xsize;
    fy = framerect->ysize;

    debugf( "gif__write32: (%d,%d)-(%d,%d) ow=%d oh=%d\n", fxoff, fyoff, fx,fy ,
            ow, oh );

    ptr = out + ow*fyoff;
    ptr2 = mask + ow*fyoff;
    for ( y=0; y < fy; y++ )
    {
        for ( x=0; x < fx; x++ )
        {
            pix = *in;
            if ( pix != transpixel )
            {
                /* pixel overwritten */
                ptr[x+fxoff] = ct[pix] >> 8;
                ptr2[x+fxoff] = 1;
            }
            in++;
        }
        if ( bInterlace )
        {
            ypos += offsets[pass];
            while ( ypos >= fy && pass < GIFPASSES )
            {
                pass++;
                ypos = starts[pass];
            }
            ptr = out + ow*(fyoff+ypos);
            ptr2 = mask + ow*(fyoff+ypos);
        }
        else
        {
            ptr += ow;
            ptr2 += ow;
        }
    }
    debugf("gifwrite32 returns\n");
}

static void GIF__Unwrite( pixel *out, pixel *mask, int ow, int oh,
                          const rect *framerect, int disposal,
                          int bg, BOOL trans )
{
    pixel *ptr, *ptr2;
    int x,y;
    int fxoff,fyoff,fx,fy;

/*     debugf( "guw: unwriting, disposal=%d\n", disposal ); */

    if ( disposal < 2 )
        return;     /* nothing to do */

    fxoff = framerect->xoff;
    fyoff = framerect->yoff;
    fx = framerect->xsize;
    fy = framerect->ysize;

/*     debugf( "     (%d,%d)-(%d,%d)\n", fxoff, fyoff, fxoff+fx, fyoff+fy ); */

    ptr = out + ow*fyoff;
    ptr2 = mask + ow*fyoff;
    for ( y=0; y<fy; y++ )
    {
        for ( x=0; x < fx; x++ )
        {
            ptr [ x + fxoff ] = bg >> 8;
            ptr2[ x + fxoff ] = trans ? 0 : 1;
        }
        ptr += ow;
        ptr2 += ow;
    }
}

static void GIF__Unwrite32( unsigned int *out, pixel *mask, int ow, int oh,
                            const rect *framerect, int disposal,
                            unsigned int bg, BOOL trans )
{
    unsigned int *ptr;
    pixel *ptr2;
    int x,y;
    int fxoff,fyoff,fx,fy;

/*     debugf( "guw: unwriting, disposal=%d\n", disposal ); */

    if ( disposal < 2 )
        return;     /* nothing to do */

    fxoff = framerect->xoff;
    fyoff = framerect->yoff;
    fx = framerect->xsize;
    fy = framerect->ysize;

/*     debugf( "     (%d,%d)-(%d,%d)\n", fxoff, fyoff, fxoff+fx, fyoff+fy ); */

    ptr = out + ow*fyoff;
    ptr2 = mask + ow*fyoff;
    for ( y=0; y<fy; y++ )
    {
        for ( x=0; x < fx; x++ )
        {
            ptr [ x + fxoff ] = bg;
            ptr2[ x + fxoff ] = trans ? 0 : 1;
        }
        ptr += ow;
        ptr2 += ow;
    }
}

/* eof */
