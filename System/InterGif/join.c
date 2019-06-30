/* Join.c */

/* Making an anim that takes one frame each from lots of files (-join option)
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 *
 * History:
 *      27-Nov-96 pdh Created (as joinanim.c)
 *      15-Dec-96 *** Release 5beta1
 *      27-Jan-97 *** Release 5beta2
 *      29-Jan-97 *** Release 5beta3
 *      03-Feb-97 *** Release 5
 *      07-Feb-97 *** Release 5.01
 *      10-Mar-97 pdh Frob for new anim library
 *      07-Apr-97 *** Release 6beta1
 *      20-May-97 pdh Fix very dumb realloc bug
 *      20-May-97 *** Release 6beta2
 *      24-Aug-97 *** Release 6
 *      27-Sep-97 *** Release 6.01
 *      08-Nov-97 *** Release 6.02
 *      21-Feb-98 *** Release 6.03
 *      07-Jun-98 pdh Fix bug with joining deep sprites
 *      07-Jun-98 *** Release 6.04
 *      21-Aug-98 *** Release 6.05
 *      05-Oct-98 *** Release 6.06
 *      19-Feb-99 *** Release 6.07
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "animlib.h"

#include "utils.h"


#if 0
#define debugf printf
#define DEBUG 1
#else
#define debugf 1?0:printf
#define DEBUG 0
#endif

BOOL Anim_ConvertFiles( const char *first,
                        anim_animfn animfn, anim_imagefn fn, void *handle )
{
    char buffer[256];
    int n = 1;
    BOOL result;

    result = Anim_ConvertFile( first, animfn, fn, handle );

    while ( result )
    {
        Anim_NthName( first, buffer, n++ );
        if ( Anim_FileSize( buffer ) <= 0 )
            break;

        debugf( "anim_fromfiles loads %s\n", buffer );

        result = Anim_ConvertFile( buffer, animfn, fn, handle );
    }

    return result;
}

#ifdef __acorn
BOOL MultiChangeFSI( const char *infile, const char *outfile, BOOL bJoin,
                     const char *options )
{
    char inbuf[256];
    char outbuf[256];
    int n = 0;

    if ( Anim_FileSize(infile) <= 0 )
    {
        Anim_SetError( "File %s not found", infile );
        return FALSE;
    }

    for (;;)
    {
        Anim_NthName( infile, inbuf, n );
        Anim_NthName( outfile, outbuf, n );
        n++;

        if ( Anim_FileSize(inbuf) <= 0 )
            break;

        if ( !ChangeFSI( inbuf, outbuf, options ) )
            return FALSE;

        if ( !bJoin )
            break;
    }
    return TRUE;
}

void MultiChangeFSI_RemoveScrapFiles( const char *infile )
{
    char inbuf[256];
    int n = 0;

    for (;;)
    {
        Anim_NthName( infile, inbuf, n );
        n++;

        if ( Anim_FileSize(inbuf) <= 0 )
            break;

        remove(inbuf);
    }
}
#endif

/* eof */
