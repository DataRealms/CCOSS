/* GIFencode.c */

/* Implementation of GIF-LZW compression algorithm
 * This code is based on Unix compress(1) -- the version in Slackware Linux
 * (where I got it) carries no copyright information, but the following author
 * credit:
 *

 > rcs_ident[] = "Based on compress.c,v 4.0 85/07/30 12:50:00 joe Release";
 >
 > Authors:	Spencer W. Thomas	(decvax!harpo!utah-cs!utah-gr!thomas)
 >		Jim McKie		(decvax!mcvax!jim)
 >		Steve Davies		(decvax!vax135!petsd!peora!srd)
 >		Ken Turkowski		(decvax!decwrl!turtlevax!ken)
 >		James A. Woods		(decvax!ihnp4!ames!jaw)
 >		Joe Orost		(decvax!vax135!petsd!joe)
 >
 > Algorithm from "A Technique for High Performance Data Compression",
 > Terry A. Welch, IEEE Computer Vol 17, No 6 (June 1984), pp 8-19.

 *
 * I was quite awed to realise I was the first person to touch this code for
 * over eleven years (but not so awed I didn't hack it to bits and back again)
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gifencode.h"
#include "workspace.h"
#include "utils.h"

#if 0
#define debugf printf
#define DEBUG 1
#else
#define debugf 1?0:printf
#define DEBUG 0
#endif

#define BITS 12
#define MAXCODE(__bits) ( (1<<(__bits)) -1 )

#define HSIZE  5003        /* 80% occupancy */

#define MAXMAXCODE ( 1 << BITS )

static unsigned char *pTo;
static int nPacketCount;
static int free_ent;
static int pdh_out;
static int pdh_bitoff;
static int clearcode;       /* 2^n      Clear dict */
static int eoicode;         /* 2^n+1    End of image */
static int firstcode;       /* 2^n+2    the first real code */
static int bDontPacket;

static void _output( int code );
static void _clearhash( int *htab );
static void _clearblock( int *htab );
static void checkpacket( void );

int LZWCompress( int n_bits, void *pTo2, LZWGiveMoreProc proc,
                 void *handle, void *pWork )
{
    int fcode;
    int c;
    int i=0, ent, disp, hshift;
    unsigned short *codetab;
    int *htab;
    int nFrom;
    unsigned char *pcFrom = (unsigned char*)(*proc)( handle, &nFrom );

    pWork = Workspace_Claim( HSIZE * 6 );

    if ( !pWork )
    {
        Anim_NoMemory( "lzwe" );
        return FALSE;
    }

    htab = (int*) pWork;
    codetab = (unsigned short*) ( htab + HSIZE );

    pTo = (unsigned char*) pTo2;

    if ( n_bits < 0 )
    {
        bDontPacket = 1;
        n_bits = -n_bits;
    }
    else
    {
        bDontPacket = 0;
        *pTo++ = n_bits-1;
        pTo++;
    }

    nPacketCount = 0;

    clearcode = 1 << (n_bits-1);
    eoicode = clearcode + 1;
    firstcode = eoicode + 1;

    free_ent = firstcode;

    pdh_out = clearcode;
    pdh_bitoff = 0;

    ent = *pcFrom++;
    nFrom--;

    hshift = 0;
    for ( fcode = HSIZE; fcode < 65536; fcode *=2 )
        hshift++;
    hshift = 8 - hshift;

    _clearhash( htab );

    _output( clearcode );

    debugf( "Compressing: " );

    while ( nFrom > 0 )
    {
        c = *pcFrom++;
        nFrom--;

        if ( nFrom == 0 )
        {
            pcFrom = (*proc)( handle, &nFrom );
        }

        fcode = (c << BITS) + ent;
        i = ( c << hshift ) ^ ent;

        if ( htab[i] == fcode )
        {
            ent = codetab[i];
            continue;
        }
        else if ( htab[i] < 0 )
            goto nomatch;

        disp = HSIZE - i;
        if ( i==0 )
            disp = 1;
probe:
        if ( ( i -= disp ) < 0 )
            i += HSIZE;

        if ( htab[i] == fcode )
        {
            ent = codetab[i];
            continue;
        }
        if ( htab[i] > 0 )
            goto probe;
nomatch:
        if ( ent >= free_ent )
            debugf( "Compress Error! ent=%d free_ent=%d\n", ent, free_ent );
        _output( ent );

        debugf( "%d ", ent );

        ent = c;
        if ( free_ent < MAXMAXCODE )
        {
            codetab[i] = free_ent++;
            htab[i] = fcode;
        }
        else
            _clearblock( htab );
    }
    _output( ent );
    _output( eoicode );     /* end-of-image */
    _output( -1 );          /* flush */

    if ( !bDontPacket )
    {
        pTo[-nPacketCount-1] = nPacketCount;

        if ( nPacketCount )
        {
            *pTo++ = 0;
        }
    }

    debugf( "\n\n" );

    Workspace_Release();

    return pTo - (unsigned char*)pTo2;
}

void _output( int code )
{
    int nbits;

/*     debugf( "-%02x", code ); */

    if ( code >= 0 )
    {
        nbits = 2;
        if ( pdh_out & 4 )
            nbits = 3;
        if ( pdh_out & 8 )
            nbits = 4;
        if ( pdh_out & 16 )
            nbits = 5;
        if ( pdh_out & 32 )
            nbits = 6;
        if ( pdh_out & 64 )
            nbits = 7;
        if ( pdh_out & 128 )
            nbits = 8;
        if ( pdh_out & 256 )
            nbits = 9;
        if ( pdh_out & 512 )
            nbits = 10;
        if ( pdh_out & 1024 )
            nbits = 11;
        if ( pdh_out > 2047 )
            nbits = 12;

        pdh_out++;
        if ( code == clearcode )
            pdh_out = eoicode;

        if ( pdh_bitoff )
        {
            code = (code << pdh_bitoff) | *pTo;
            nbits += pdh_bitoff;
        }
        if ( nbits >= 8 )
        {
            *pTo++ = code;
            code >>= 8;
            nbits -= 8;
            if ( !bDontPacket )
                checkpacket();
        }
        if ( nbits >= 8 )
        {
            *pTo++ = code;
            code >>= 8;
            nbits -= 8;
            if ( !bDontPacket )
                checkpacket();
        }
        pdh_bitoff = nbits;
        if ( nbits > 0 )
        {
            *pTo = code;
        }
    }
    else
    {
        if ( pdh_bitoff > 0 )
        {
            pTo++;
            if ( !bDontPacket )
                checkpacket();
        }
    }
}

static void checkpacket( void )
{
    nPacketCount++;
    if ( nPacketCount == 255 )
    {
        /* pPacketHeader = pTo */
        pTo[-256] = (unsigned char)'\xFF';
        pTo++;
        nPacketCount = 0;
    }
}

static void _clearhash( int *htab )
{
    memset( htab, -1, HSIZE*sizeof( int ) );
}

static void _clearblock( int *htab )
{
    _clearhash( htab );
    free_ent = firstcode;
    _output( clearcode );
}

