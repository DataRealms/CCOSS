/* Riemann.h */

/* Header file for Animator decompressor
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 */

#define UInt unsigned int
#define UInt32 unsigned int
#define Int32 int
#define FAR

#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif

void Riemann_Decompress( void FAR *packed,
                         int linelen,
                         int lineskip,
                         int nlines,
                         unsigned char FAR *image,
                         void FAR *workspace,
                         BOOL delta );

UInt32 Riemann_DecompWspace( Int32 csize );
