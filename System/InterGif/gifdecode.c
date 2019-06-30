/* GIFdecode.c */

/* Implementation of GIF-LZW decompression algorithm
 * This code is based on Unix compress(1) -- see comments in GIFencode.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "gifdecode.h"
#include "workspace.h"
#include "utils.h"

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


#define BITS 12     /* 3PRM p4-103 */

#define MAXCODE(__bits) ( (1<<(__bits)) -1 )

#define MAXMAXCODE ( 1 << BITS )


static unsigned char *pcFrom;
static int nFrom;
static int clear_flg;
static int free_ent;
static int maxcode;
static int n_bits;
static int minbits;
static unsigned int finchar;

static int clearcode;       /* 2^n      Clear dict */
static int eoicode;         /* 2^n+1    End of image */
static int firstcode;       /* 2^n+2    the first real code */


static int _getcode( void );

/*
static char magic[] = "\0x1F\0x9D";
static unsigned char lmask[9] = {0xFF,0xFE,0xFC,0xF8,0xF0,0xE0,0xC0,0x80,0x00 };
static unsigned char rmask[9] = {0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F,0x7F,0xFF };
*/

static int offset = 0, size = 0;

BOOL LZWDecompress( void *pFrom, int nFrom2, void *pTo2, void *pWork,
                    unsigned int nMaxOut, unsigned int *pResultSize,
                    int minbits2 )
{
    int code, oldcode, incode;
    unsigned int nWritten = 0;
    unsigned short *prefixes;
    unsigned char  *suffixes;
    unsigned char  *de_stack;

    unsigned char  *stackp;
    unsigned char  *pTo = (unsigned char*) pTo2;

    offset = 0;
    size = 0;

    pWork = Workspace_Claim( (3<<12) + 8000 );

    if ( !pWork )
    {
        Anim_NoMemory( "lzwd" );
        return FALSE;
    }

    HEAPTEST;

    prefixes = (unsigned short*) ( pWork );
    suffixes = (unsigned char*)  ( prefixes + (1<<BITS) );
    de_stack = (unsigned char*)  ( suffixes + (1<<BITS) );

    pcFrom = (unsigned char*) pFrom;
    nFrom = nFrom2;

    minbits = minbits2;

    if ( minbits > 0 )
    {
        /* GIF style */
        n_bits = minbits;

        clearcode = 1 << (n_bits-1);
        eoicode = clearcode + 1;
        firstcode = eoicode + 1;
    }
    else
    {
        /* Animator style */
        n_bits = 9;
        clearcode = -1;     /* doesn't happen */
        eoicode = 256;
        firstcode = 257;
    }

    maxcode = MAXCODE( n_bits );
    clear_flg = 0;

    for ( code=clearcode-1; code >= 0; code-- )
    {
        prefixes[code] = 0;
        suffixes[code] = (unsigned char) code;
    }
    free_ent = firstcode;

    finchar = oldcode = _getcode();
    debugf("%d ", finchar);

    if ( oldcode == clearcode )
    {
        finchar = oldcode = _getcode();
    	debugf("%d ", finchar);
    }

    if ( pTo )
        *pTo++ = finchar;
    nWritten++;

    stackp = de_stack;

    HEAPTEST;

    debugf( "Decompressing:" );

    while ( (code = _getcode()) > -1 )
    {
        debugf( " %d", code );

        if ( code == eoicode )
        {
            debugf( "E" );
            break;
        }

        if ( code == clearcode )
        {
            debugf( "C" );
            for ( code=clearcode-1; code>=0; code-- )
                prefixes[code] = 0;
            clear_flg = 1;
            free_ent = firstcode-1;
            if ( (code = _getcode()) == -1 )
                break;

            debugf( " %d", code );
        }
        incode = code;

        if ( code >= free_ent )
        {
            if ( code > free_ent )
            {
                debugf( "Error! code=%d, free_ent=%d\n", code, free_ent );
                Anim_SetError( "Bad compressed data" );
                return FALSE;
            }
            if ( pTo )
            {
                *stackp++ = finchar;
            }
            else
                nWritten++;
            code = oldcode;
        }

        if ( pTo )
        {
            while ( code > clearcode )
            {
                int oldcode = code;
                *stackp++ = suffixes[ code ];
                code = prefixes[ code ];
                if ( oldcode <= code )
                {
                    debugf( "Error! oldcode=%d code=%d incode=%d, free_ent=%d\n", oldcode, code, incode, free_ent );
                    if ( pResultSize )
                        *pResultSize = nWritten;
                    Anim_SetError( "Bad compressed data" );
                    return FALSE;
                }
            }
            *stackp++ = finchar = suffixes[ code ];

            nWritten += stackp - de_stack;

            debugf( "<%d>", stackp - de_stack );

            if ( nWritten > nMaxOut )
            {
                printf( "Warning: too much compressed data (%d>%d)\n",nWritten,nMaxOut );

                nWritten -= (stackp - de_stack);
                while ( nWritten < nMaxOut )
                {
                    *pTo++ = *--stackp;
                    nWritten++;
                }

                if ( pResultSize )
                    *pResultSize = nMaxOut;

                /* not an error any more, there's too many broken GIFs out
                 * there
                 */
/*                 Anim_SetError( "Too much compressed data" ); */
                return TRUE;
            }

/*             debugf( "[" ); */
            do {
                *pTo++ = *--stackp;
/*                 debugf( " %02x", pTo[-1] ); */
            } while ( stackp > de_stack );
/*             debugf( " ]\n" ); */
        }
        else
        {
            while ( code > clearcode )
            {
                nWritten++;
                code = prefixes[ code ];
            }
            finchar = suffixes[ code ];
            nWritten++;
            stackp = de_stack;
        }

        if ( (code=free_ent) < MAXMAXCODE )
        {
            prefixes[ code ] = (unsigned short) oldcode;
            suffixes[ code ] = finchar;
            free_ent = code+1;
        }

        oldcode = incode;
    }

    HEAPTEST;

    debugf( "\n\nnWritten=%d\n", nWritten );

    if ( code != eoicode )
        printf( "Warning: last code wasn't EOI\n" );

    Workspace_Release();

    HEAPTEST;

    if ( pResultSize )
        *pResultSize = nWritten;

    return TRUE;
}

static unsigned int rmask[14] = {0x00,0x01,0x03,0x07 ,0x0F,0x1F,0x3F,0x7F,0xFF,
                                 0x1FF,0x3FF,0x7FF, 0xFFF,0x1FFF };

static int _getcode( void )
{
    int code;
    unsigned int r_off;
    int bits;
    unsigned char *bp = (unsigned char*)pcFrom;

    if ( free_ent > maxcode )
    {
        n_bits++;
        if ( n_bits == BITS )
            maxcode = MAXMAXCODE;
        else
            maxcode = MAXCODE( n_bits );
    }

    if ( clear_flg > 0 )
    {
        maxcode = MAXCODE( n_bits = minbits );
        clear_flg = 0;
    }

    if ( (offset + n_bits) > nFrom*8 )
    {
        return -1;
    }

    r_off = offset; /*0*/
    bits = n_bits;  /*9*/

    bp += ( r_off >> 3 );
    r_off &= 7;

    code = bp[0] + 256*bp[1] + 65536*bp[2];
    code = code >> r_off;
    code = code & rmask[n_bits];
    offset += n_bits;

/*     debugf( "code = %d\n", code ); */

    return code;
}
