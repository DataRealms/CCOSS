/* AnimLib.h */

/* Header file for object-oriented bitmap animation routines
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 */

#ifndef animlib_h
#define animlib_h

#ifndef __stdio_h
#include <stdio.h>
#endif

#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif

typedef unsigned char pixel;


/* Palettes */

typedef struct palettestr {
    struct palettestr *pNext;
    unsigned int nUsage;
    unsigned int nColours;
    unsigned int pColours[1];   /* or more. Colours are 0xBBGGRR00 */
} palettestr;

typedef palettestr *palette;

palette Palette_Create( const unsigned int *pColours, unsigned int nColours );
void    Palette_Destroy( palette *pPal );


/* Histograms (palettes with usage information) */

typedef struct {
    unsigned int value;
    unsigned int n;
} histogramentrystr;

typedef struct {
    int nEntries;
    histogramentrystr *e;
} histogramstr;

typedef histogramstr *histogram;

BOOL Histogram_Open( void );
BOOL Histogram_Pixel( unsigned int pixel );
BOOL Histogram_PixelsFromFile( const char *fname );
histogram Histogram_Close( void );
void Histogram_Destroy( histogram *pHist );

histogram Histogram_ReduceMC( histogram input, int nColours ); /* median-cut */


/* Palette mappers */

typedef struct palettemapperstr *palettemapper;

palettemapper PaletteMapper_To256( void );
palettemapper PaletteMapper_To216( void );
palettemapper PaletteMapper_ToFile( const char *filename );
palettemapper PaletteMapper_ToData( unsigned int nCols, unsigned int *pCols );

unsigned int PaletteMapper_Map( palettemapper p, unsigned int c );
pixel   PaletteMapper_MapIndex( palettemapper p, unsigned int c );
unsigned int PaletteMapper_GetPalette( palettemapper p, unsigned int *pal );

void PaletteMapper_Destroy( palettemapper *p );


/* Frames (images) */

typedef struct framestr {
    palette       pal;
    void         *pImageData;
    unsigned int  nImageSize;
    void         *pMaskData;    /* May be NULL. 0=transparent, 1=solid. */
    unsigned int  nMaskSize;
    unsigned int  csDelay;      /* Frame delay (centiseconds) */
} framestr;

typedef framestr *frame;

void *Anim_Compress( const pixel *data, unsigned int len,
                     unsigned int *pCompSize );
void *Anim_CompressAligned( const pixel *data, unsigned int xs, unsigned int ys,
                            unsigned int rowlen,
                            unsigned int *pCompSize );
BOOL  Anim_Decompress( const void *data, unsigned int len, unsigned int outlen,
                       pixel *dest );
BOOL  Anim_DecompressAligned( const void *data, unsigned int len,
                              unsigned int xs, unsigned int ys, pixel *dest );


/* Animations (collections of images all the same size) */

#define animflag_LOOP       0x1

typedef struct animationstr {
    unsigned int  nWidth;
    unsigned int  nHeight;      /* pixels */
    unsigned int  nFrames;
    framestr     *pFrames;
    unsigned int  flags;
} animationstr;

typedef animationstr *anim;     /* not "animation" for historical reasons */

anim Anim_Create( void );
BOOL Anim_AddFrame( anim a, unsigned int xs, unsigned int ys,
                    const pixel *pImage, const pixel *pMask,
                    unsigned int nColours, const unsigned int *pColours );
void Anim_Destroy( anim *pAnim );

typedef struct {
    int nWidth;
    int nLineWidthBytes;
    int nHeight;
    int nBPP;
    int csDelay;
    void *pBits;
} anim_imageinfo;

typedef BOOL (*anim_animfn)( void *handle, unsigned int nWidth,
                             unsigned int nHeight, unsigned int flags );

/* Pass pixmask==-1 if no pixmask, bitmask==NULL if no bitmask */
typedef BOOL (*anim_imagefn)( void *handle,
                              const anim_imageinfo *pixels, int pixmask,
                              const anim_imageinfo *bitmask,
                              const unsigned int *pColours );

/* Higher-level constructors (Draw and Sprite on RiscOS only) */

BOOL Anim_ConvertSprite( const void *spritedata, size_t size,
                         anim_animfn animfn, anim_imagefn fn, void *handle );
BOOL Anim_ConvertTCA( const void *tcadata, size_t size,
                      anim_animfn animfn, anim_imagefn fn, void *handle );
BOOL Anim_ConvertDraw( const void *drawdata, size_t size,
                       anim_animfn animfn, anim_imagefn fn, void *handle );
BOOL Anim_ConvertGIF( const void *gifdata, size_t size,
                      anim_animfn animfn, anim_imagefn fn, void *handle );

/* Very high-level constructors (calling these will link in *all* the above
 * constructors)
 */

BOOL Anim_ConvertData( const void *data, unsigned int size,
                    anim_animfn animfn, anim_imagefn fn, void *handle );
BOOL Anim_ConvertFile( const char *filename,
                    anim_animfn animfn, anim_imagefn fn, void *handle );
BOOL Anim_ConvertFiles( const char *firstname,
                     anim_animfn animfn, anim_imagefn fn, void *handle );
extern char *Anim_ChangeFSI;

anim Anim_FromData( const void *data, unsigned int size,
                    palettemapper pmap );
anim Anim_FromFile( const char *filename, palettemapper pmap );
anim Anim_FromFiles( const char *firstname, palettemapper pmap );

/* Things to do with an anim when you've got one */

BOOL Anim_MapTo216( anim a, BOOL bDither );
BOOL Anim_MapTo256( anim a, BOOL bDither );
BOOL Anim_MapToFile( anim a, const char *palfile, BOOL bDither );
BOOL Anim_MapToGiven( anim a, unsigned int nColours, unsigned int *pColours,
                      BOOL bDither );
BOOL Anim_MapToBest( anim a, unsigned int nColours, BOOL bDither );
BOOL Anim_CommonPalette( anim a );
BOOL Anim_TransparentPixel( anim a, pixel p );
BOOL Anim_Trim( anim a );           /* commonpal.c */
BOOL Anim_RemoveMasks( anim a );
BOOL Anim_AddToHistogram( anim a ); /* commonpal.c */


/* Saving animations (nFrom and nTo are inclusive and count from 0) */

BOOL Anim_SaveSprite( const anim a, FILE *output, int nFrom, int nTo );

typedef struct {
    unsigned int nDefaultDelay    : 16;
    BOOL         bInterlace       : 1;
    BOOL         bLoop            : 1;
    BOOL         bForceDelay      : 1;
} anim_GIFflags;

BOOL Anim_SaveGIF( anim a, anim_GIFflags agf, FILE *f, int nFrom, int nTo );


/* Colour matching */

typedef struct colourmatchstr *colourmatch;

colourmatch ColourMatch_Create( unsigned int nCols, unsigned int *pCols );
unsigned int ColourMatch_Match( colourmatch cm, unsigned int c, BOOL bIndex );
void ColourMatch_Destroy( colourmatch *pcm );


/* Other global routines and data */

typedef void* (anim_allocproc)( int nSize );
typedef void  (anim_freeproc)( void *pBlock );

void Anim_RegisterAllocator( anim_allocproc m, anim_freeproc f );

typedef void (anim_flexallocproc)( void *pPtr, int nSize );
typedef BOOL (anim_flexreallocproc)( void *pPtr, int nNewSize );
typedef void (anim_flexfreeproc)( void *pPtr );

void Anim_RegisterFlexAllocator( anim_flexallocproc fa,
                                 anim_flexreallocproc fr,
                                 anim_flexfreeproc ff );

extern char *Anim_Error;
void Anim_ClearError( void );

typedef struct {
    unsigned int xoff, yoff, xsize, ysize;
} rect;

#endif
