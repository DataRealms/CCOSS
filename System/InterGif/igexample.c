/* IGExample.c */

/* Example maker of GIF files (draws a red square on a blue background)
 * (K) All Rites Reserved -- Copy What You Like
 *
 * Authors:
 *      Peter Hartley  <peter@ant.co.uk>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __acorn
#include "DeskLib:File.h"   /* for File_SetType() */
#endif

#include "animlib.h"


/* How big we want the image to be.
 */
#define XSIZE 64
#define YSIZE 64


/* The pixel data for the image (a 'pixel' is just an unsigned char).
 */
pixel *image = NULL;


/* The palette data for the image (0xBBGGRR00, the same format as RiscOS sprite
 * palettes).
 */
unsigned int myColours[2];

/* A helpful macro for defining palette entries.
 */
#define RGB(r,g,b) ((b<<24)+(g<<16)+(r<<8))


/* Dead simple square drawing... does no clipping and will crash if asked to
 * plot outside the image. Draws in colour 1.
 */
void drawSquare( int x, int y, int w, int h )
{
    while ( h )
    {
        pixel *pos = image + y*XSIZE + x;
        memset( pos, 1, w );
        h--;
        y++;
    }
}


int main( int argc, char *argv[] )
{
    /* Create the anim structure (which represents an entire animation ie.
     * potentially several frames).
     */
    anim a = Anim_Create();
    char *outputFileName;

    if ( !a )
    {
        fprintf(stderr, Anim_Error );
        return 1;
    }


    /* Create a buffer for the image we're going to build; clear it to
     * colour zero.
     */
    image = malloc( XSIZE*YSIZE );
    if ( !image )
    {
        fprintf( stderr, "Can't allocate %d-byte image buffer\n",
                         XSIZE*YSIZE );
        Anim_Destroy( &a );  /* not strictly necessary as we're exiting */
        return 1;
    }

    memset( image, 0, XSIZE*YSIZE );


    /* Do our plotting into the image (here, just a simple square).
     */
    drawSquare( XSIZE/4, YSIZE/4, XSIZE/2, YSIZE/2 );


    /* Create the palette which will be used for the GIF
     */
    myColours[0] = RGB( 0, 128, 255 );    /* colour 0 is turquoise */
    myColours[1] = RGB( 255, 0, 0 );      /* colour 1 is full red */


    /* Add this frame to the anim we've created (adding the first frame
     * sets the overall size of the animation).
     */
    if ( !Anim_AddFrame( a,             /* anim to add to */
                         XSIZE, YSIZE,  /* size of frame */
                         image,         /* pixels making up frame */
                         NULL,          /* mask, or NULL if no mask */
                         2,             /* number of colours used */
                         myColours ) )  /* colour palette */
    {
        fprintf( stderr, Anim_Error );
        Anim_Destroy( &a );  /* again, not strictly necessary */
        return 1;
    }


    /* We could add more frames if we wanted.
     */


    /* Save the completed (1-frame) animation as a GIF */
    if ( argc > 1 )
        outputFileName = argv[1];
    else
        outputFileName = "result";

    {
        anim_GIFflags agf = { 0 };
        FILE *f = fopen( outputFileName, "wb" );

        if ( !f )
        {
            fprintf( stderr, "Couldn't open output file '%s'\n",
                             outputFileName );
            Anim_Destroy( &a );  /* again, not strictly necessary */
            return 1;
        }


        /* Set any flags we need in agf. (As we've only got a 1-frame anim,
         * only the interlace one is relevant.)
         */
        agf.bInterlace = TRUE;


        /* Actually do the saving. The last two arguments are the first and
         * last frames to save, counting from zero -- i.e. this only saves
         * the first frame.
         */
        if ( !Anim_SaveGIF( a, agf, f, 0, 0 ) )
        {
            fprintf( stderr, Anim_Error );
            Anim_Destroy( &a );
            fclose( f );
            return 1;
        }

        fclose( f );


#ifdef __acorn
        /* Set the file's type to "GIF" (this isn't relevant on Win32 or Unix).
         */
        File_SetType( outputFileName, 0x695 );
#endif
    }

    Anim_Destroy( &a );

    return 0;
}

/* eof */
