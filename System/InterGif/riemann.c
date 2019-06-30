/* riemann.c */

/* The Complete Animator decompressor code for InterGif
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 *
 * History:
 *      Aeons ago pdh Adapted from the public-domain EuclidX module
 *      23-Aug-96 pdh Created in intergif project
 *      25-Aug-96 pdh Frob to cope with delta films
 *      25-Aug-96 *** Release 3.00
 *      01-Sep-96 *** Release 3.01
 *      27-Oct-96 *** Release 4beta1
 *      29-Oct-96 *** Release 4beta2
 *      07-Nov-96 *** Release 4
 *      15-Dec-96 *** Release 5beta1
 *      27-Jan-97 *** Release 5beta2
 *      29-Jan-97 *** Release 5beta3
 *      03-Feb-97 *** Release 5
 *      07-Feb-97 *** Release 5.01
 *      07-Apr-97 *** Release 6beta1
 *      20-May-97 *** Release 6beta2
 *      24-Aug-97 *** Release 6
 *      27-Sep-97 *** Release 6.01
 *      08-Nov-97 *** Release 6.02
 *      21-Feb-98 *** Release 6.03
 *      07-Jun-98 *** Release 6.04
 *      21-Aug-98 *** Release 6.05
 *      19-Feb-99 *** Release 6.07
 *
 */

#include <stdlib.h>

#include "riemann.h"

UInt32 Riemann_DecompWspace( Int32 csize )
{
    UInt32 dsize = 257;
    int bits = 9;
    while ( csize > 0 )
    {
        dsize += (1<<bits);
        csize -= ( bits<<bits )/8;
    }
    return dsize*4;
}

#define DecompressByte                                  \
    {                                                   \
        if ( (nextentry >> 16) == 0x0100 )              \
            nextentry = *(--dictp);                     \
        nextentry = dict[ nextentry >> 16 ];            \
        *(--dest) = (unsigned char)(nextentry & 0xFF);  \
    }

#define DecompressDeltaByte                             \
    {                                                   \
        if ( (nextentry >> 16) == 0x0100 )              \
            nextentry = *(--dictp);                     \
        nextentry = dict[ nextentry >> 16 ];            \
        *(--dest) ^= (unsigned char)(nextentry & 0xFF); \
    }

void Riemann_Decompress( void FAR *packed,
                         int linelen,
                         int lineskip,
                         int nlines,
                         unsigned char FAR *image,
                         void FAR *workspace,
                         BOOL bDelta )
{
    UInt FAR *dict = (UInt FAR*) workspace;
    UInt FAR *src = (UInt FAR*) packed;
    UInt dmask = 0x1FF;
    UInt FAR *dictp;
    UInt nextentry;

    int entrysize = 9,
        entrypos = 23;
    UInt dictend;
    unsigned char FAR *linestart;
    unsigned char FAR *dest, FAR *rounded;
    UInt loword, dbyte;
    UInt word;

    for ( dictend = 0; dictend < 0x100; dictend++ )
    {
        dict[dictend] = 0x01000000 | dictend;
    }
    dictend = 0x101;

    src++;      /* skip size word */
    dict--;     /* temporarily */

    nextentry = 0x01000000;

    /* type1_dodecrypt
       ... build lzw dictionary */

    do {

        if ( 0 == entrypos )
            loword = src[1];
        else
            loword = ( src[0] >> ( 32-entrypos ) ) | ( src[1] << entrypos );

        loword = ( loword & dmask) << 16;
        entrypos -= entrysize;
        if ( entrypos < 0 )
        {
            entrypos += 32;
            src++;
        }
        if ( dictend >> entrysize )
        {
            entrysize++;
            dmask |= dictend;
        }
        if ( loword > 0x01000000 )
            dbyte = dict[ loword >> 16 ] & 0xFF;
        else if ( loword == 0x01000000 )
            dbyte = 0;
        else
            dbyte = loword >> 16;

/*
        if ( dictend*4 > worksize )
            return Riemann_OutOfMem;
*/

        dict[ dictend ] = nextentry | dbyte;
        nextentry = loword;

        dictend++;
    } while ( nextentry != 0x01000000 );
    dictend--;


    dict++;

    /* type1_startimage */

    dictp = dict + dictend;

    dest = linestart = image + nlines*linelen;

    nextentry = 0x01000000;

    if ( bDelta )
    {
        while ( linestart != image )
        {
            int others;
            /* go round this loop once per line */

            linestart -= linelen;
            dest = linestart + abs(linelen) - lineskip;

            rounded = linestart + 4;

            others = ( abs(linelen) - lineskip ) & 3;

            if ( dest >= rounded )
            {
                /* some words to do */
                if ( others )
                {
                    DecompressDeltaByte;
                    others--;
                    if ( others )
                    {
                        DecompressDeltaByte;
                        others--;
                        if ( others )
                        {
                            DecompressDeltaByte;
                        }
                    }
                }

                /* words! */
                while ( dest >= rounded )
                {
                    if ( (nextentry >> 16) == 0x0100 )
                        nextentry = *(--dictp);
                    nextentry = dict[ nextentry >> 16 ];

                    word = nextentry << 24;

                    if ( (nextentry >> 16) == 0x0100 )
                        nextentry = *(--dictp);
                    nextentry = dict[ nextentry >> 16 ];

                    word |= (nextentry & 0xFF) << 16;

                    if ( (nextentry >> 16) == 0x0100 )
                        nextentry = *(--dictp);
                    nextentry = dict[ nextentry >> 16 ];

                    word |= (nextentry & 0xFF) << 8;

                    if ( (nextentry >> 16) == 0x0100 )
                        nextentry = *(--dictp);
                    nextentry = dict[ nextentry >> 16 ];

                    word |= nextentry & 0xFF;

                    dest -= 4;
                    *((UInt FAR*) dest) ^= word;
                }
            }
        }
    }
    else /* not delta */
    {
        while ( linestart != image )
        {
            int others;
            /* go round this loop once per line */

            linestart -= linelen;
            dest = linestart + abs(linelen) - lineskip;

            rounded = linestart + 4;

            others = ( abs(linelen) - lineskip ) & 3;

            if ( dest >= rounded )
            {
                /* some words to do */
                if ( others )
                {
                    DecompressByte;
                    others--;
                    if ( others )
                    {
                        DecompressByte;
                        others--;
                        if ( others )
                        {
                            DecompressByte;
                        }
                    }
                }

                /* words! */
                while ( dest >= rounded )
                {
                    if ( (nextentry >> 16) == 0x0100 )
                        nextentry = *(--dictp);
                    nextentry = dict[ nextentry >> 16 ];

                    word = nextentry << 24;

                    if ( (nextentry >> 16) == 0x0100 )
                        nextentry = *(--dictp);
                    nextentry = dict[ nextentry >> 16 ];

                    word |= (nextentry & 0xFF) << 16;

                    if ( (nextentry >> 16) == 0x0100 )
                        nextentry = *(--dictp);
                    nextentry = dict[ nextentry >> 16 ];

                    word |= (nextentry & 0xFF) << 8;

                    if ( (nextentry >> 16) == 0x0100 )
                        nextentry = *(--dictp);
                    nextentry = dict[ nextentry >> 16 ];

                    word |= nextentry & 0xFF;

                    dest -= 4;
                    *((UInt FAR*) dest) = word;
                }
            }
        }
    }
}

