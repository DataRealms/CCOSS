/* Frame.h */

/* Header file for operations on frames
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 */

#ifndef animlib_h
#include "animlib.h"
#endif

void Rect_Union( rect *r1, const rect *r2 );
/* r1 := r1 {union} r2 */

BOOL ChangedRect( anim a, const pixel *i1, const pixel *m1,
                  const pixel *i2, const pixel *m2, rect *pRect );

/*
void PixMaskClearRect( anim a, frameptr fpp, rect *pRect );
 */

/* InterGif deals with two types of transparency: a PixMask, where one of the
 * pixel-values of the bitmap is a "knock-out" colour that's not plotted
 * (like in a GIF itself), and a BitMask, where the bitmap has a second bitmap
 * associated with it which defines which pixels are to be plotted (like in a
 * RiscOS sprite file).
 *      All the anim types generate BitMasks, but the compressor uses a
 * PixMask (of course)... all the cleverness is done in the conversion from
 * one to the other.
 */

/*
BOOL PixMaskCheck( anim a, frameptr f1, frameptr f2, int transpixel );
BOOL BitMaskCheck( anim a, frameptr f1, frameptr f2, BOOL *pbMaskUsed );
 * TRUE if any pixels are solid in f2 but transparent in f1 */

/*
void PixMaskEqualPixels( anim a, frameptr f1, frameptr f2, int transpixel );
 */
void BitMaskEqualPixels( anim a, const pixel *i1, pixel *m1,
                         const pixel *i2, const pixel *m2 );
/* Makes transparent any pixels in f1 which are the same as the corresponding
 * ones in f2
 */

int BitMaskFindTransPixel( anim a, pixel *img, const pixel *mask,
                           BOOL bForce );
/* Returns a pixel-value not used by any solid pixel in f */

void BitMaskToPixMask( anim a, pixel *img, const pixel *mask, int nTransPixel );
/* Turns a bitmasked image into a pixmasked one, by setting all masked-out
 * pixels to nTransPixel
 */

/*
void PixMaskToBitMask( anim a, frameptr f, int nTransPixel );
 * Turns a pixmasked image into a bitmasked one, by masking out all nTransPixel
 * pixels
 */

/*
void BitMaskClear( anim a, frameptr f );
 * Make the frame completely opaque */

/*
BOOL BitMaskAllSolid( anim a, frameptr f );
 */

BOOL BitMaskTrimTransparentBorders( anim a, const pixel *mask, rect *pRect );
/* Returns the bounding rectangle of all solid pixels in f */

void PixMaskOptimiseRectangle( anim a, pixel *i1, pixel *m1,
                               const pixel *i2, const pixel *m2,
                               int transpixel, rect *pRect );
/* See frame.c for explanation of what this rather complex function does
 * and why it's needed
 */

void BitMaskClearRectangle( anim a, pixel *mask, const rect *pRect );
/* Make transparent a rectangle in f's pixel mask */

BOOL BitMaskAnyTransparent( anim a, const pixel *mask, const rect *pRect );
/* TRUE if any pixels are transparent in the specified rectangle of f */

BOOL BitMaskFindTransRect( anim a, const pixel *m1, const pixel *m2,
                           rect *pRect );
/* Returns the bounding rectangle of all pixels which are solid in f1 but
 * transparent in f2
 */
