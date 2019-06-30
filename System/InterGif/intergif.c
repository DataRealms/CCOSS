/* InterGif.c */

/* Central routine for InterGif
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 *
 * History:
 *      17-Nov-96 pdh Split off from main.c
 *      15-Dec-96 *** Release 5beta1
 *      27-Jan-97 *** Release 5beta2
 *      29-Jan-97 *** Release 5beta3
 *      02-Feb-97 pdh Call to Anim_Percent
 *      03-Feb-97 *** Release 5
 *      07-Feb-97 *** Release 5.01
 *      11-Mar-97 pdh Frob for new anim library
 *      07-Apr-97 *** Release 6beta1
 *      20-May-97 *** Release 6beta2
 *      02-Jun-97 pdh Add more arguments to InterGif
 *      24-Aug-97 *** Release 6
 *      27-Sep-97 *** Release 6.01
 *      07-Nov-97 pdh Add bTrim
 *      08-Nov-97 *** Release 6.02
 *      15-Feb-98 pdh Add palette-mapper stuff
 *      21-Feb-98 *** Release 6.03
 *      07-Jun-98 *** Release 6.04
 *      01-Aug-98 pdh anim_imagefn stuff
 *      21-Aug-98 *** Release 6.05
 *      05-Oct-98 *** Release 6.06
 *      19-Feb-99 *** Release 6.07
 *
 * References:
 *      http://utter.chaos.org.uk/~pdh/software/intergif.htm
 *          pdh's InterGif home page
 *      http://www.n-vision.com/panda/gifanim/
 *          A site all about animated GIFs for other platforms
 *      http://asterix.seas.upenn.edu/~mayer/lzw_gif/gif89.html
 *          The GIF89a spec
 *      http://www.iota.co.uk/info/animfile/
 *          The Complete Animator file format
 *
 */

#include <stdio.h>

#include "animlib.h"
#include "utils.h"
#include "split.h"

#include "intergif.h"

#if 0
#define debugf printf
#define DEBUG 1
#else
#define debugf 1?0:printf
#define DEBUG 0
#endif

/*---------------------------------------------------------------------------*
 * Histogrammer is an anim_imagefn passed to Anim_FromFile[s] which adds the *
 * pixels in each frame to the current histogram.                            *
 *---------------------------------------------------------------------------*/

static BOOL Histogrammer( void *handle, const anim_imageinfo *pixels,
                          int pixmask, const anim_imageinfo *bitmask,
                          const unsigned int *pColours )
{
    int x,y;
    int bpp = pixels->nBPP;
    int maskbpp = bitmask ? bitmask->nBPP : bpp;
    const char *pBits = (const char*) pixels->pBits;
    const char *pMask = (const char*) (bitmask ? bitmask->pBits : NULL);
    int h = pixels->nHeight, w = pixels->nWidth;

    if ( bpp > 8 )
    {
        /* Direct colour case */

        for ( y=0; y<h; y++ )
        {
            unsigned int maskwant = (1<<maskbpp) - 1;
            unsigned char *maskptr = (unsigned char*) pMask;

            if ( bpp == 16 )
            {
                const unsigned short *pShorts = (unsigned short*) pBits;

                for ( x=0; x<w; x++ )
                {
                    unsigned int pix = *pShorts++;

                    if ( maskptr ? (*maskptr & maskwant)
                    	       : ( pix != pixmask ) )
                        if ( !Histogram_Pixel( Anim_Pix16to32(pix) ) )
                            return FALSE;

                    if ( maskptr )
                    {
                        maskwant = maskwant << maskbpp;
                        if ( maskwant > 256 )
                        {
                            maskptr++;
                            maskwant = (1<<maskbpp) - 1;
                        }
                    }
                }
            }
            else    /* bpp == 32 */
            {
                const unsigned int *pWords = (unsigned int*) pBits;

                for ( x=0; x<w; x++ )
                {
                    unsigned int pix = (*pWords++)<<8;

	            if ( maskptr ? (*maskptr & maskwant)
                               : ( pix != pixmask ) )
                        if ( !Histogram_Pixel( pix ) )
                            return FALSE;

                    if ( maskptr )
                    {
                        maskwant = maskwant << maskbpp;
                        if ( maskwant > 256 )
                        {
                            maskptr++;
                            maskwant = (1<<maskbpp) - 1;
                        }
                    }
                }
            }

            pBits += pixels->nLineWidthBytes;
            if ( pMask )
                pMask += bitmask->nLineWidthBytes;
        }
    }
    else
    {
        /* Palettised colour case */
        for ( y=0; y<h; y++ )
        {
            unsigned int imagewant = (1<<bpp) - 1;
            unsigned int maskwant = (1<<maskbpp) - 1;
            unsigned char *imgptr = (unsigned char*) pBits;
            unsigned char *maskptr = (unsigned char*) pMask;
            unsigned int imageshift = 0;

            for ( x=0; x<w; x++ )
            {
                unsigned int pix = (*imgptr >> imageshift) & imagewant;

                if ( maskptr ? (*maskptr & maskwant)
                             : (pix != pixmask) )
                    if ( !Histogram_Pixel( pColours[pix] ) )
                        return FALSE;

                imageshift += bpp;
                if ( imageshift == 8 )
                {
                    imgptr++;
                    imageshift = 0;
                }

                if ( maskptr )
                {
                    maskwant = maskwant << maskbpp;
                    if ( maskwant > 256 )
                    {
                        maskptr++;
                        maskwant = (1<<maskbpp) - 1;
                    }
                }
            }

            pBits += pixels->nLineWidthBytes;
            if ( pMask )
                pMask += bitmask->nLineWidthBytes;
        }
    }

    return TRUE;
}

static BOOL ChoosePaletteMapper( const char *infile, BOOL bJoin,
                                 BOOL b216, BOOL b256, const char *palfile,
                                 int nBest, palettemapper *pMapper )
{
    palettemapper pmap = NULL;

    if ( b216 )
    {
        pmap = PaletteMapper_To216();
    }
    else if ( b256 )
    {
        pmap = PaletteMapper_To256();
    }
    else if ( palfile )
    {
        pmap = PaletteMapper_ToFile( palfile );
    }
    else if ( nBest > 0 )
    {
        histogram h = NULL, h2 = NULL;
        unsigned int cols[256];
        BOOL result;

        /* Start up the histogram stuff */
        if ( !Histogram_Open() )
            return FALSE;

        if ( bJoin )
            result = Anim_ConvertFiles( infile, NULL, &Histogrammer, NULL );
        else
            result = Anim_ConvertFile( infile, NULL, &Histogrammer, NULL );

        if ( !result )
        {
            debugf( "Couldn't convert files in choosepm\n" );
            Histogram_Destroy(NULL);
            return FALSE;
        }

        h = Histogram_Close();

        if ( h )
        {
            debugf( "choosepm: histogram has %d entries\n", h->nEntries );

            h2 = Histogram_ReduceMC( h, nBest );

            if ( h2 )
            {
                int i;

                debugf( "reducemc(%d) returns", h2->nEntries );

                for ( i=0; i < h2->nEntries; i++ )
                {
                    cols[i] = h2->e[i].value;
                    debugf( " <%08x>", cols[i] );
                }
                debugf( "\n" );

                pmap = PaletteMapper_ToData( h2->nEntries, cols );

                debugf( "pmtodata returns %p\n", pmap );
            }
            else
                debugf( "Couldn't create h2 in choosepm\n" );
        }
        else
            debugf( "Couldn't create histogram in choosepm\n" );

        Histogram_Destroy( &h );
        Histogram_Destroy( &h2 );
    }
    else
    {
        *pMapper = NULL;
        return TRUE;
    }

    *pMapper = pmap;

    return pmap ? TRUE : FALSE;
}

BOOL InterGif( const char *infile, BOOL bJoin, BOOL bSplit, BOOL bSprite,
               anim_GIFflags flags, BOOL bForceTrans, pixel nForceTransPixel,
               BOOL bUseTrans, BOOL bTrim, BOOL b216, BOOL b256, BOOL bDither,
               const char *palfile, BOOL bSame, int nBest, const char *cfsi,
               const char *outfile )
{
    unsigned int i;
    char buffer[256];
    int nFramesPerFile;
    FILE *output;
    BOOL result = FALSE;
    anim a = NULL;
    palettemapper pmap = NULL;

#ifdef __acorn
    if ( cfsi )
    {
        char *file2 = "<Wimp$ScrapDir>.IGTemp000";

        if ( !MultiChangeFSI( infile, file2, bJoin, cfsi ) )
            return FALSE;

        infile = file2;
    }
#endif

    if ( !ChoosePaletteMapper( infile, bJoin, b216, b256, palfile, nBest,
                               &pmap ) )
        return FALSE;

    a = bJoin ? Anim_FromFiles( infile, pmap )
              : Anim_FromFile( infile, pmap );

    if ( !a )
        goto err;

    /* Sort mask out */

    if ( bForceTrans )
    {
        if ( !Anim_TransparentPixel( a, nForceTransPixel ) )
            goto err;
    }
    else if ( !bUseTrans )
    {
        if ( !Anim_RemoveMasks( a ) )
            goto err;
    }

    if ( bTrim )
    {
        if ( !Anim_Trim( a ) )
            goto err;
    }

    if ( !bSame )
        if ( !Anim_CommonPalette( a ) )
            goto err;

    debugf( "Saving %d-frame animation as %s%s\n", a->nFrames,
            bSprite ? "sprite" : "gif",
            bSplit ? "s" : "" );

    result = TRUE;
    nFramesPerFile = bSplit ? 1 : a->nFrames;

    for ( i=0; i < a->nFrames; i+= nFramesPerFile )
    {
        if ( i )
            Anim_Percent( i*100 / a->nFrames );

        Anim_NthName( outfile, buffer, i );

        output = fopen( buffer, "wb" );
        if ( !output )
        {
            Anim_SetError( "Cannot open '%s' for output", buffer );
            result = FALSE;
            break;
        }
        debugf( "Opening %s\n", buffer );

        if ( bSprite )
            result = Anim_SaveSprite( a, output, i, i + nFramesPerFile-1 );
        else
            result = Anim_SaveGIF( a, flags, output, i, i + nFramesPerFile-1 );

        fclose( output );

        if ( result )
            Anim_SetFileType( buffer, bSprite ? filetype_SPRITE
                                              : filetype_GIF );
        else
            break;
    }

err:
    Anim_Destroy( &a );
    PaletteMapper_Destroy( &pmap );

#ifdef __acorn
    if ( cfsi )
        MultiChangeFSI_RemoveScrapFiles( infile );
#endif

    return result;
}

/* eof */
