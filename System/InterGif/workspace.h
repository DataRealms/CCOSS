/* workspace.h */

/* Workspace for LZW routines (so we don't have to keep mallocing and freeing)
 */

void *Workspace_Claim( unsigned int nSize );

void Workspace_Release( void );
