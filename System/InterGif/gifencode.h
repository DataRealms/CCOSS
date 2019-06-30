/* GIFencode.h */

/* Header file for GIF-LZW compression routine
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 */

#define gifcompress_WORKSIZE (5003*6)

typedef unsigned char* (*LZWGiveMoreProc)( void *handle, int *pSize );

/* GIF-LZW compressor -- returns compressed size
 * set n_bits negative to do no packetising
 */
extern int LZWCompress( int n_bits, void *pTo2, LZWGiveMoreProc proc,
                        void *handle, void *pWork );
