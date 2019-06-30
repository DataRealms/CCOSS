/* Hist.c */

/* Histogram generation for AnimLib
 *
 */

#include <stdlib.h>
#include <string.h>

#include "animlib.h"
#include "utils.h"

typedef struct colour {
  unsigned int  value;
  unsigned int  nPixels;
  struct colour *pNext;
} colour;


#define HASHSIZE 20023
#define HASH(c) ((( ( (c>>8) & 0xFF ) * 33023 + \
                    ( (c>>16)& 0xFF ) * 30013 + \
                    ( (c>>24)& 0xFF ) * 27011 ) & 0x7fffffff ) % HASHSIZE )

static colour **hashtable;
static int nEntries;

BOOL Histogram_Open( void )
{
    hashtable = (colour**) Anim_Allocate( HASHSIZE * sizeof(colour*) );
    if ( !hashtable )
    {
        Anim_NoMemory( "histopen" );
        return FALSE;
    }
    memset( hashtable, 0, HASHSIZE*sizeof(colour*) );
    nEntries = 0;
    return TRUE;
}

BOOL Histogram_Pixel( unsigned int pixel )
{
    int hash;
    colour *entry;

    hash = HASH(pixel);
    for ( entry = hashtable[hash]; ; entry = entry->pNext )
    {
        if ( !entry )
	{
	    entry = (colour*) Anim_Allocate( sizeof(colour) );
	    if ( !entry )
	    {
	        Anim_NoMemory( "histpixel" );
	        return FALSE;
	    }

	    entry->value = pixel;
	    entry->nPixels = 1;
	    entry->pNext = hashtable[hash];
	    hashtable[hash] = entry;
	    nEntries++;
	    break; /* we've inserted it */
	}
	else if ( entry->value == pixel )
        {
	    entry->nPixels++;
	    break; /* we've found it */
	}
    }
    return TRUE;
}

histogram Histogram_Close( void )
{
    int i;
    colour *ptr, *next;
    histogram res = (histogram) Anim_Allocate( sizeof(histogramstr) );
    histogramentrystr *entry;

    entry = Anim_Allocate( nEntries * sizeof(histogramentrystr) );

    if ( !res || !entry )
    {
        Anim_Free( &res );
        Anim_Free( &entry );
        Anim_NoMemory( "histclose" );
        return NULL;
    }

    res->nEntries = nEntries;
    res->e = entry;

    for ( i=0; i < HASHSIZE; i++)
    {
        ptr = hashtable[i];
        while ( ptr )
        {
            next = ptr->pNext;
            entry->n = ptr->nPixels;
            entry->value = ptr->value;
            entry++;
            Anim_Free( &ptr );
            ptr = next;
        }
    }

    Anim_Free( &hashtable );

    return res;
}

void Histogram_Destroy( histogram *pHist )
{
    if ( pHist && *pHist )
    {
        Anim_Free( &(*pHist)->e );
        Anim_Free( pHist );
    }
    if ( hashtable )
    {
        int i;
        colour *ptr, *next;

        for ( i=0; i < HASHSIZE; i++ )
        {
            ptr = hashtable[i];
            while ( ptr )
            {
                next = ptr->pNext;
                Anim_Free( &ptr );
                ptr = next;
            }
        }
        Anim_Free( &hashtable );
    }
}

/* eof */
