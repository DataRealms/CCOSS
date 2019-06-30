/* workspace.c */

/* Workspace for LZW routines (so we don't have to keep mallocing and freeing)
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 *
 * History:
 *      01-Mar-97 pdh Created
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

#include <string.h>
#include <stdarg.h>

#include "utils.h"
#include "workspace.h"

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


static unsigned int workusage = 0;
static unsigned int worksize = 0;
static void *workspace = NULL;

void *Workspace_Claim( unsigned int nSize )
{
    if ( ( workusage == 0 && nSize>0 ) || worksize < nSize )
    {
        debugf( "asked for %d, worksize is %d, reallocating\n",
                nSize, worksize );

	if ( workspace )
	{
	    void *ptr = Anim_Reallocate( workspace, nSize );
	    if ( !ptr )
	    	return NULL;
	    workspace = ptr;
	}
	else
	{
	    workspace = Anim_Allocate( nSize );
	    if ( !workspace )
	    	return NULL;
	}
        worksize = nSize;
    }

    workusage++;

    /*debugf( "wsc(%d): asked for %d returning %d at %p\n", workusage, nSize, worksize, workspace );*/

    return workspace;
}

void Workspace_Release( void )
{
    HEAPTEST;

    workusage--;
    if ( workusage == 0 )
    {
        debugf( "wsr: really releasing workspace\n" );
        Anim_Free( &workspace );
        worksize = 0;
    }

    HEAPTEST;
}
