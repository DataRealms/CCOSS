/* GIFdecode.h */

/* Header file for GIF-LZW decompression routine
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 */

#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif

#define gifdecompress_WORKSIZE ( (3<<12) + 8000 )

BOOL LZWDecompress( void *pFrom, int nFrom2, void *pTo2, void *pWork,
                    unsigned int nMaxOut, unsigned int *pResultSize,
                    int minbits2 );
/* TRUE => image was okay
 * FALSE => it was broken (some may still have decompressed okay)
 */
