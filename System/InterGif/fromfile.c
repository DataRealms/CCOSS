/* FromFile.c */

/* General constructor for loading a file into an anim
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 *
 * History:
 *      10-Nov-96 pdh Split off from InterGif itself
 *      15-Dec-96 *** Release 5beta1
 *      27-Jan-97 *** Release 5beta2
 *      29-Jan-97 *** Release 5beta3
 *      02-Feb-97 pdh Modify FromData so the individual Creates return errors
 *      03-Feb-97 *** Release 5
 *      05-Feb-97 pdh Add call to Draw_CreateAnim
 *      07-Feb-97 *** Release 5.01
 *      10-Mar-97 pdh Frob a lot for new anim library
 *      07-Apr-97 *** Release 6beta1
 *      20-May-97 *** Release 6beta2
 *      24-Aug-97 *** Release 6
 *      27-Sep-97 *** Release 6.01
 *      08-Nov-97 *** Release 6.02
 *      21-Feb-98 *** Release 6.03
 *      07-Jun-98 *** Release 6.04
 *      21-Aug-98 pdh Rename to ConvertFile/ConvertData; add wrappers in from.c
 *      21-Aug-98 *** Release 6.05
 *      05-Oct-98 *** Release 6.06
 *      19-Feb-99 *** Release 6.07
 *
 */

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"
#include "animlib.h"

BOOL Anim_ConvertFile( const char *file,
                       anim_animfn animfn, anim_imagefn fn, void *handle )
{
    void *pData;
    int insize;
    BOOL result;

    insize = Anim_FileSize( file );

    if ( insize <= 0 )
    {
        Anim_SetError( "File %s not found", file );
        return FALSE;
    }
    pData = Anim_Allocate( insize );

    if ( !pData )
    {
        Anim_NoMemory( "pdata" );
        return FALSE;
    }

    if ( !Anim_LoadFile( file, pData ) )
    {
        Anim_Free( &pData );
        Anim_SetError( "Cannot load %s", file );
        return FALSE;
    }

    result = Anim_ConvertData( pData, insize, animfn, fn, handle );

    Anim_Free( &pData );

    return result;
}

BOOL Anim_ConvertData( const void *pData, size_t insize,
                       anim_animfn animfn, anim_imagefn fn, void *handle )
{
    BOOL result;

    Anim_Error = NULL;

    result = Anim_ConvertGIF( pData, insize, animfn, fn, handle );

    if ( !result && !Anim_Error )
        result = Anim_ConvertTCA( pData, insize, animfn, fn, handle );

    if ( !result && !Anim_Error )
        result = Anim_ConvertSprite( pData, insize, animfn, fn, handle );

#ifdef __acorn
    if ( !result && !Anim_Error )
        result = Anim_ConvertDraw( pData, insize, animfn, fn, handle );
#endif

    if ( !result && !Anim_Error )
        Anim_SetError( "File format not recognised" );

    return result;
}

/* eof */
