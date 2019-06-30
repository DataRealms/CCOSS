/* PaletteMap.c */

/* Routines for finding closest colours
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 *
 * History:
 *      15-Feb-97 pdh Created
 *      21-Feb-98 *** Release 6.03
 *      07-Jun-98 *** Release 6.04
 *      21-Aug-98 *** Release 6.05
 *      19-Feb-99 *** Release 6.07
 *
 */

#include <string.h>
#include <stdarg.h>

#include "animlib.h"
#include "utils.h"

#if 0
#define debugf printf
#define DEBUG 1
#else
#define debugf 1?0:printf
#define DEBUG 0
#endif

typedef unsigned int (*mapfn)(palettemapper p, unsigned int c);
typedef pixel (*mapindexfn)(palettemapper p, unsigned int c);
typedef unsigned int (*getpalfn)(palettemapper p, unsigned int *pal );
typedef void (*destroyfn)(palettemapper *pMapper );

typedef struct palettemapperstr {
    const mapfn map;
    const mapindexfn mapindex;
    const getpalfn getpal;
    const destroyfn destroy;
} palettemapperstr;


        /*=======================*
         *   Map to 216 (easy)   *
         *=======================*/


static unsigned int Find216( unsigned int c, BOOL bIndex )
{
    unsigned int b = (c >> 24) & 0xFF;
    unsigned int g = (c >> 16) & 0xFF;
    unsigned int r = (c >>  8) & 0xFF;

    /* Component values available are 0, 0x33, 0x66, 0x99, 0xCC, 0xFF
     * i.e. multiples of 0x33 = 51
     *
     *   (0+25)/51 = 0
     * (255+25)/51 = 280/51 = 5
     */

    b = (b+25)/51;
    g = (g+25)/51;
/*     (int)(g*6.0/256.0); */
    r = (r+25)/51;

    if ( bIndex )
        return b*36 + g*6 + r;

    return ((b*51)<<24) + ((g*51)<<16) + ((r*51)<<8);
}

static unsigned int To216_Map( palettemapper p, unsigned int c )
{
    return Find216( c, FALSE );
}

static pixel To216_MapIndex( palettemapper p, unsigned int c )
{
    return (pixel) Find216( c, TRUE );
}

static unsigned int To216_GetPal( palettemapper p, unsigned int *pal )
{
    int r,g,b;

    for ( b=0;b<6;b++ )
        for ( g=0;g<6;g++ )
            for ( r=0;r<6;r++ )
                *pal++ = ((b*51)<<24) + ((g*51)<<16) + ((r*51)<<8);

    return 216;
}

static const palettemapperstr map216 = {
    &To216_Map,
    &To216_MapIndex,
    &To216_GetPal,
    NULL
};

palettemapper PaletteMapper_To216( void )
{
    return (palettemapper) &map216;
}


        /*==============================*
         *   Map to 256 (not so easy)   *
         *==============================*/


static int tinterror( unsigned int comp, unsigned int tint )
{
    int te = (comp & 0x30) - tint;

    if ( te<0 )
        te = -te;

    if ( te > 0x20 )
        te = 0x40 - te;

    return te;
}

static unsigned int tintadd( unsigned int comp, unsigned int tint )
{
    int te = ( comp & 0x30 ) - tint;

    if ( te > 0x20 && comp < 0xC0 )
    {
        comp += 0x40;
    }
    else if ( te <= -0x20 && comp >= 0x40 )
    {
        comp -= 0x40;
    }
    return (( ( comp & 0xC0 ) + tint ) * 0x11)>>4;
}

static unsigned int Find256( unsigned int c, BOOL bIndex )
{
    unsigned int b = (c >> 24) & 0xFF;
    unsigned int g = (c >> 16) & 0xFF;
    unsigned int r = (c >>  8) & 0xFF;

    unsigned int tint;
    unsigned int best = 0;
    int besterror = 0x4000000;

/*     debugf( "find256: %x\n  ", c ); */

    for ( tint=0; tint < 0x3F; tint += 0x10 )
    {
        int thiserror = tinterror( r, tint ) * 3
                        + tinterror( g, tint ) * 10
                        + tinterror( b, tint );

/*         debugf(" %x:%x", tint, thiserror ); */

        if ( thiserror < besterror )
        {
            besterror = thiserror;
            best = tint;
        }
    }

/*     debugf( " -> %x\n", best ); */

    /* now best is value required ( 0, 0x10, 0x20, 0x30 ) */

    r = tintadd( r, best );
    g = tintadd( g, best );
    b = tintadd( b, best );

    if ( bIndex )
    {
        tint = best>>4; /* 0..3 */
        r = r>>6;       /* 0..3 */
        g = g>>6;
        b = b>>6;

        return ((b&2)<<6) + (g<<5) + ((r&2)<<3) + ((b&1)<<3)
             + ((r&1)<<2) + tint;
    }

    return (b<<24) + (g<<16) + (r<<8);
}

static unsigned int To256_Map( palettemapper p, unsigned int c )
{
    return Find256( c, FALSE );
}

static pixel To256_MapIndex( palettemapper p, unsigned int c )
{
    return (pixel) Find256( c, TRUE );
}

static unsigned int To256_GetPal( palettemapper p, unsigned int *pal )
{
    int i, tint, r, g, b;

    /* palette indices are %bggrbrtt (PRM 3-339) */

    for ( i=0; i<256; i++ )
    {
        tint = (i&3);
        r = ((i&16)>>3) | ((i&4)>>2);
        g = (i&0x60) >> 5;
        b = ((i&128)>>6) | ((i&8)>>3);
        pal[i] =   ( ((r*4+tint)*0x11) << 8)
                 + ( ((g*4+tint)*0x11) << 16)
                 + ( ((b*4+tint)*0x11) << 24);
    }

    return 256;
}

static const palettemapperstr map256 = {
    &To256_Map,
    &To256_MapIndex,
    &To256_GetPal,
    NULL
};

palettemapper PaletteMapper_To256( void )
{
    return (palettemapper) &map256;
}


        /*==================*
         *   Palette file   *
         *==================*/


palettemapper PaletteMapper_ToFile( const char *filename )
{
    unsigned int nCol = 0;
    unsigned int pal[256];
    char bytes[6];
    FILE *f;

    f = fopen( filename, "rb" );
    if ( !f )
    {
        Anim_SetError( "Palette file %s won't open", filename );
        return NULL;
    }

    while ( fread( bytes, 6, 1, f ) > 0 && nCol < 256 )
    {
        unsigned int word = (bytes[5]<<24) + (bytes[4]<<16) + (bytes[3]<<8);
        pal[nCol++] = word;
    }
    fclose( f );

    debugf( "Built %d-entry palette\n", nCol );

    return PaletteMapper_ToData( nCol, pal );
}


        /*=====================================*
         *   Arbitrary map (not at all easy)   *
         *=====================================*/


typedef struct {
    palettemapperstr p;
    colourmatch cm;
    int nCol;
    unsigned int pal[1]; /* or more */
} pmdata;

static unsigned int ToData_Map( palettemapper p, unsigned int c )
{
    pmdata *pmd = (pmdata*)p;

#if DEBUG
    if ( pmd->pal[0] == pmd->pal[1] )
        debugf( "todatamap: pal[0]=%x pal[1]=%x\n", pmd->pal[0], pmd->pal[1] );
#endif

    return ColourMatch_Match( pmd->cm, c, FALSE );
}

static pixel ToData_MapIndex( palettemapper p, unsigned int c )
{
    pixel result = (pixel) ColourMatch_Match( ((pmdata*)p)->cm, c, TRUE );

    return result;
}

static unsigned int ToData_GetPal( palettemapper p, unsigned int *pal )
{
    pmdata *pmd = (pmdata*)p;
    memcpy( pal, pmd->pal, pmd->nCol*sizeof(unsigned int) );
    return pmd->nCol;
}

static void ToData_Destroy( palettemapper *pMapper )
{
    if ( *pMapper )
    {
        pmdata *pmd = (pmdata*)*pMapper;
        debugf( "todata_d calls cm_d\n" );
        ColourMatch_Destroy( &pmd->cm );
        debugf( "todata_d calls free\n" );
        Anim_Free(&pmd);
        *pMapper = NULL;
    }
}

static const palettemapperstr mapdata = {
    &ToData_Map,
    &ToData_MapIndex,
    &ToData_GetPal,
    &ToData_Destroy
};

palettemapper PaletteMapper_ToData( unsigned int n, unsigned int *p )
{
    pmdata *result = Anim_Allocate( sizeof(pmdata)
                                    +(n-1)*sizeof(unsigned int) );

    if ( !result )
    {
        Anim_NoMemory( "pmapnew" );
        return NULL;
    }

    debugf( "pmap_todata(%d)\n", n );

    /* Used to say
     * result->p = mapdata;
     * but MS VC++ versions 4 and 5 both barf on it */
    memcpy( &result->p, &mapdata, sizeof(mapdata) );

    result->nCol = n;
    memcpy( result->pal, p, n*sizeof(unsigned int) );

    result->cm = ColourMatch_Create( n, result->pal );
    if ( !result->cm )
    {
        Anim_Free(&result);
        return NULL;
    }

    return (palettemapper)result;
}


        /*============*
         *   Driver   *
         *============*/


unsigned int PaletteMapper_Map( palettemapper p, unsigned int c )
{
    if ( !p )
        return c;

    return (p->map)(p,c);
}

pixel PaletteMapper_MapIndex( palettemapper p, unsigned int c )
{
    return (p->mapindex)(p,c);
}

unsigned int PaletteMapper_GetPalette( palettemapper p, unsigned int *pal )
{
    return (p->getpal)(p,pal);
}

void PaletteMapper_Destroy( palettemapper *p )
{
    if ( *p )
    {
        if ( (*p)->destroy )
            ((*p)->destroy)(p);
        *p = NULL;
    }
}

/* eof */
