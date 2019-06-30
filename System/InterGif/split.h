/* Split.h */

/* Header file for multi-file stuff
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 *
 * Authors:
 *      Peter Hartley       <pdh@chaos.org.uk>
 */

void NthName( const char *in, char *out, int n );


#ifdef __acorn
BOOL MultiChangeFSI( const char *infile, const char *outfile, BOOL bJoin,
                     const char *options );
void MultiChangeFSI_RemoveScrapFiles( const char *infile );
#endif
