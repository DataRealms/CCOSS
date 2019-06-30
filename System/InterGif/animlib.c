/* animlib.c */

/* Core of Anim library
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 *
 * History:
 *      25-Feb-97 pdh Created
 *      07-Apr-97 *** Release 6beta1
 *      20-May-97 *** Release 6beta2
 *      27-May-97 pdh Anim_TransparentPixel/Anim_RemoveMasks
 *      01-Jun-97 pdh Split up some big allocate() buffers
 *      24-Aug-97 *** Release 6
 *      27-Sep-97 *** Release 6.01
 *      08-Nov-97 *** Release 6.02
 *      21-Feb-98 *** Release 6.03
 *      07-Jun-98 *** Release 6.04
 *      21-Aug-98 *** Release 6.05
 *      05-Oct-98 *** Release 6.06
 *	19-Feb-99 pdh Fix Neil's bug in Anim_Compress
 *      19-Feb-99 *** Release 6.07
 *
 */

#include <string.h>
#include <stdarg.h>

#include "utils.h"
#include "gifdecode.h"
#include "gifencode.h"
#include "animlib.h"

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


/* Forward references */

static void Anim__FreeFrame( framestr *f );


        /*======*/


anim Anim_Create( void )
{
    anim result = (anim) Anim_Allocate( sizeof(animationstr) );

    if ( result )
        memset( result, 0, sizeof(animationstr) );
    else
        Anim_NoMemory( "animcreate" );

    return result;
}

BOOL Anim_AddFrame( anim a, unsigned int xs, unsigned int ys,
                    const pixel *pImage, const pixel *pMask,
                    unsigned int nColours, const unsigned int *pal )
{
    int n = a->nFrames;
    framestr f;
    framestr *pFrames;
    unsigned int i;

    if ( ( a->nWidth && xs != a->nWidth )
         || ( a->nHeight && ys != a->nHeight ) )
    {
        Anim_SetError( "Frame %d incompatible: wrong size", n );
        return FALSE;
    }
    a->nWidth = xs;
    a->nHeight = ys;

    HEAPTEST;

    f.csDelay = 0;
    f.pal = Palette_Create( pal, nColours );
    f.pImageData = Anim_Compress( pImage, xs*ys, &f.nImageSize );

    HEAPTEST;

    /* Only compress the mask if there is one */

    f.pMaskData = NULL;
    f.nMaskSize = 0;
    if ( pMask )
        for ( i=0; i < xs*ys; i++ )
            if ( pMask[i] == 0 )
            {
                f.pMaskData = Anim_Compress( pMask, xs*ys, &f.nMaskSize );
                if ( !f.pMaskData )
                {
                    debugf( "aaf: Anim_Compress(mask) failed\n" );
                    Anim__FreeFrame( &f );
                    return FALSE;
                }
                break;
            }

    HEAPTEST;

    debugf( "Compressed data: %d bytes of image, %d bytes of mask\n",
            f.nImageSize, f.nMaskSize );

    if ( !f.pal || !f.pImageData )
    {
        debugf( "aaf: pal=%p, imagedata=%p, failing\n", f.pal, f.pImageData );
        Anim__FreeFrame( &f );
        return FALSE;
    }

    pFrames = Anim_Reallocate( a->pFrames, (n+1)*sizeof(framestr) );

    HEAPTEST;

    if ( !pFrames )
    {
        Anim_NoMemory( "addframe" );
        Anim__FreeFrame( &f );
        return FALSE;
    }

    pFrames[n] = f;

    HEAPTEST;

    a->pFrames = pFrames;
    a->nFrames = n+1;

    debugf( "aaf: frames now %d\n", n+1 );

    return TRUE;
}

void Anim_Destroy( anim *pAnim )
{
    if ( *pAnim )
    {
        unsigned int i;
        anim a = *pAnim;

        for ( i=0; i < a->nFrames; i++ )
            Anim__FreeFrame( a->pFrames+i );

        Anim_Free( &a->pFrames );
        Anim_Free( pAnim );
    }
}

static void Anim__FreeFrame( framestr *f )
{
    if ( f->pal )
        Palette_Destroy( &f->pal );
    Anim_Free( &f->pImageData );
    Anim_Free( &f->pMaskData );
}


/*------------------------------------------------------------*
 * Frame compression: like GIF compression but not packetised *
 *------------------------------------------------------------*/

typedef struct {
    const pixel *data;
    unsigned int len;
} rgmpargs;

static unsigned char *RawGiveMoreProc( void *handle, int *pSize )
{
    rgmpargs *pArgs = (rgmpargs*) handle;
    *pSize = (int) pArgs->len;
    pArgs->len = 0;
    return (unsigned char*) pArgs->data;
}

static void *Anim__Compress( LZWGiveMoreProc proc, void *handle,
                             unsigned int len, unsigned int *pSize )
{
    char *result;
    unsigned int outsize;

    result = Anim_Allocate( (len*3)/2 + 32 );	/* add some slop, like nraine says */
    if ( !result )
    {
        Anim_NoMemory( "animcompress" );
        return NULL;
    }

    outsize = LZWCompress( -9, result, proc, handle, NULL );

    debugf( "Compressed %d bytes of data to %d\n", len, outsize );

    *pSize = outsize;

    /* block is getting smaller, so we suspect Reallocate never fails */
    return Anim_Reallocate( result, outsize );
}

void *Anim_Compress( const pixel *data, unsigned int len,
                     unsigned int *pCompSize )
{
    rgmpargs args;

    if ( !data )
    {
        *pCompSize = 0;
        return NULL;
    }

    args.data = data;
    args.len = len;

    return Anim__Compress( &RawGiveMoreProc, (void*)&args, len, pCompSize );
}

typedef struct {
    const pixel *data;
    unsigned int xs, ys, rowlen;
} agmpargs;

static unsigned char *AlignedGiveMoreProc( void *handle, int *pSize )
{
    agmpargs *pArgs = (agmpargs*) handle;
    const pixel *data = pArgs->data;

    *pSize = pArgs->ys ? pArgs->xs : 0;
    pArgs->ys--;
    pArgs->data += pArgs->rowlen;
    return (unsigned char*)data;
}


void *Anim_CompressAligned( const pixel *data, unsigned int xs, unsigned int ys,
                            unsigned int rowlen,
                            unsigned int *pCompSize )
{
    agmpargs args;

    if ( !data )
    {
        *pCompSize = 0;
        return NULL;
    }

    args.data   = data;
    args.xs     = xs;
    args.rowlen = rowlen;  /*(xs+3)&~3;*/
    args.ys     = ys;

    return Anim__Compress( &AlignedGiveMoreProc, (void*)&args, xs*ys,
                           pCompSize );
}

/*---------------*
 * Mask handling *
 *---------------*/

BOOL Anim_TransparentPixel( anim a, pixel p )
{
    unsigned int i, j;
    pixel *imgbuffer, *maskbuffer;
    unsigned int bytes = a->nWidth * a->nHeight;
    BOOL ok = TRUE;

    imgbuffer = (pixel*) Anim_Allocate( bytes );
    maskbuffer = (pixel*) Anim_Allocate( bytes );

    if ( !imgbuffer || !maskbuffer )
    {
        Anim_NoMemory( "atpixel" );
        ok = FALSE;
    }
    else
    {

        for ( i=0; i < a->nFrames && ok; i++ )
        {
            frame f = a->pFrames+i;

            ok = Anim_Decompress( f->pImageData, f->nImageSize, bytes,
                                  imgbuffer );
            if (ok)
            {
                BOOL allsolid = TRUE;
                pixel p2;

                for ( j=0; j<bytes; j++ )
                {
                    p2 = (imgbuffer[j] != p);    /* 1 = solid */
                    maskbuffer[j] = p2;
                    allsolid &= p2;
                }

                Anim_Free( &f->pMaskData );

                if ( !allsolid )
                {
                    f->pMaskData = Anim_Compress( maskbuffer,bytes,
                                                  &f->nMaskSize );
                    if ( !f->pMaskData )
                        ok = FALSE;
                }
            }
        }
    }

    Anim_Free( &imgbuffer );
    Anim_Free( &maskbuffer );
    return ok;
}

BOOL Anim_RemoveMasks( anim a )
{
    unsigned int i;

    for ( i=0; i < a->nFrames; i++ )
    {
        Anim_Free( &((a->pFrames+i)->pMaskData) );
    }
    return TRUE;
}


/*---------------------*
 * Frame decompression *
 *---------------------*/

BOOL Anim_Decompress( const void *data, unsigned int len, unsigned int outlen,
                      pixel *dest )
{
    BOOL result;
    unsigned int outsize;

    HEAPTEST;

    debugf( "decompressing %d bytes of data (expect %d)\n", len, outlen );

    result = LZWDecompress( (void*) data, len, dest, NULL, outlen, &outsize,
                            9 );

    HEAPTEST;

    debugf( "decompressed size %d (expected %d)\n", outsize, outlen );

    return result;
}

BOOL Anim_DecompressAligned( const void *data, unsigned int len,
                             unsigned int xs, unsigned int ys, pixel *dest )
{
    unsigned int abw = (xs+3) & ~3;
    pixel *src = dest + (abw-xs)*ys;

    debugf( "animda: decompressing %dx%d pixels into %dx%d\n", xs,ys, abw,ys );

    HEAPTEST;

    if ( !Anim_Decompress( data, len, xs*ys, src ) )
        return FALSE;

    HEAPTEST;

    if ( abw == xs )
        return TRUE;        /* was aligned anyway */

    while ( ys )
    {
        debugf( "memcpy( %p, %p, %d )\n", dest, src, xs );
        memmove( dest, src, xs );
        dest += abw;
        src += xs;
        ys--;
    }

    return TRUE;
}


        /*====================*
         *   Error handling   *
         *====================*/


static char memoryerror[] = "Out of memory (01234567890123456789)";

char *Anim_Error = NULL;
static char *errorbuffer = NULL;

void Anim_NoMemory( const char *where )
{
    char *to = strchr( memoryerror, '(' );
    strcpy( to+1, where );
    strcat( to, ")" );
    Anim_Error = memoryerror;
}

void Anim_SetError( const char *report, ... )
{
    va_list ap;

    va_start( ap, report );

    if ( !errorbuffer )
        errorbuffer = Anim_Allocate(256);

    vsprintf(errorbuffer, report, ap);
    va_end(ap);
    Anim_Error = errorbuffer;
}

void Anim_ClearError( void )
{
    Anim_Free( &errorbuffer );
    Anim_Error = NULL;
}


        /*==================*
         *   Palette pool   *
         *==================*/


palettestr *palettepool = NULL;

palette Palette_Create( const unsigned int *pColours, unsigned int nColours )
{
    palettestr *ptr = palettepool;

    while ( ptr )
    {
        if ( ptr->nColours == nColours
             && !memcmp(pColours,ptr->pColours,nColours*sizeof(unsigned int)) )
        {
            ptr->nUsage++;

            debugf( "pal%p: usage becomes %d\n", ptr, ptr->nUsage );

            return ptr;
        }

        ptr = ptr->pNext;
    }

    ptr = (palettestr*) Anim_Allocate( sizeof(palettestr)
                                       + (nColours-1)*sizeof(unsigned int) );
    if ( ptr )
    {
        ptr->nColours = nColours;
        ptr->nUsage = 1;
        memcpy( ptr->pColours, pColours, nColours*sizeof(unsigned int) );
        ptr->pNext = palettepool;
        palettepool = ptr;

        debugf( "pal%p: new, %d colours\n", ptr, nColours );
    }
    else
        Anim_NoMemory( "palcreate" );

    return ptr;
}

void Palette_Destroy( palette *p )
{
    if ( *p )
    {
        palettestr **ppPal = &palettepool;

        while ( *ppPal )
        {
            if ( *ppPal == *p )
                break;
            ppPal = &((*ppPal)->pNext);
        }

        (*p)->nUsage--;

        debugf( "pal%p: usage becomes %d\n", *p, (*p)->nUsage );

        if ( (*p)->nUsage == 0 )
        {
             *ppPal = (*p)->pNext;
             Anim_Free( p );
        }

        *p = NULL;
    }
}

/* eof */
