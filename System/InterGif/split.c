/* Split.c */

/* Multi-file routines for InterGif
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 *
 * History:
 *      01-Sep-96 pdh Created
 *      01-Sep-96 *** Release 3.01
 *      27-Oct-96 *** Release 4beta1
 *      29-Oct-96 pdh Fix "ADFS::4.sprite" -> "ADFS::5.sprite" problem
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"

#ifdef __acorn
#define DIRSEP '.'
#define SUFFIXSEP '/'
#else
#define DIRSEP '/'
#define SUFFIXSEP '.'
#endif

void Anim_NthName( const char *original, char *output, int n )
{
    int presize;

    const char *numptr;
    int numsize;    /* numeric part of string */

    const char *postptr;

    int nStart;

    const char *ptr;

    if ( n == 0 )
    {
        strcpy( output, original );
        return;
    }

    ptr = original + strlen(original) - 1;  /* point to last character */

    while ( ptr > original && !isdigit(*ptr) )
        ptr--;

    /* ptr points at last digit (or first character) */

    if ( isdigit( *ptr ) && ptr > strrchr( original, DIRSEP ) )
    {
        postptr = ptr + 1;
        ptr--;
        while ( ptr >= original && isdigit(*ptr) )
            ptr--;

        presize = ptr - original + 1;
        numptr = ptr+1;

        numsize = postptr - numptr;

        strncpy( output, numptr, numsize );
        output[numsize] = 0;
        nStart = atoi( output );

        sprintf( output, "%.*s%0*d%s", presize, original, numsize, n + nStart,
                 postptr );
    }
    else
    {
        /* no digits in leafname */
        ptr = strrchr( original, SUFFIXSEP );
        if ( ptr )
            sprintf( output, "%.*s%d%s", ptr-original, original, n, ptr );
        else
            sprintf( output, "%s%d", original, n );
    }
}

#ifdef TEST

void test( char *s )
{
    char buffer[80];
    int i;

    for ( i=0; i < 5; i++ )
    {
        NthName( s, buffer, i );
        printf( "%d @ %s = %s\n", i, s, buffer );
    }
}

int main( int argc, char *argv[] )
{
    test( "frame0/gif" );
    test( "frame/gif" );
    test( "frame97/gif" );
    test( "frame" );
    test( "13" );

    return 0;
}
#endif
