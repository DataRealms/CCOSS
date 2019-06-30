/* Win32.c */

/* Bits and pieces to compile AnimLib under Win32
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 *
 * History:
 *      27-Sep-97 *** Release 6.01
 *      08-Nov-97 *** Release 6.02
 *      21-Feb-98 *** Release 6.03
 *      07-Jun-98 *** Release 6.04
 *      21-Aug-98 *** Release 6.05
 *      19-Feb-99 *** Release 6.07
 */

#include <stdlib.h>

#include "utils.h"

void *Anim_Allocate( int nSize )
{
    return malloc( nSize );
}

void *Anim_Reallocate( void *pBlock, int nSize )
{
    return realloc( pBlock, nSize );
}

void Anim_Free( void *ppBlock )
{
    void *pBlock = *((void**)ppBlock);
    if ( pBlock )
    {
        free( pBlock );
    }
    *((void**)ppBlock) = NULL;
}

void Anim_SetFileType( const char *filename, int type ) { }

void Anim_Percent( int percent ) { }

int Anim_FileSize( const char *filename )
{
    FILE *f = fopen( filename, "rb" );
    long sz;

    if ( !f )
        return 0;

    fseek( f, 0, SEEK_END );

    sz = ftell(f);

    fclose( f );

    return (int) sz;
}

BOOL Anim_LoadFile( const char *filename, void *data )
{
    FILE *f = fopen( filename, "rb" );
    long sz;

    if ( !f )
        return FALSE;

    fseek( f, 0, SEEK_END );

    sz = ftell(f);

    fseek( f, 0, SEEK_SET );

    fread( data, 1, sz, f );

    fclose( f );

    return TRUE;
}
