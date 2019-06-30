/* Sprite.h */

/* RiscOS sprite format definition for animlib/InterGif
 * (K) All Rites Reversed - Copy What You Like (see file Copying)
 */

/* Parts of a sprite file (like Desklib:Sprite.h, except that some releases
 * of DeskLib get this wrong, and define a sprite as a sprite_info* rather
 * than a sprite_header*). Also we can't exactly require Windows and Linux
 * users to install DeskLib just to compile InterGif...
 */

#ifndef animlib_sprite_h
#define animlib_sprite_h

typedef struct {
    unsigned int nSize;
    unsigned int nSprites;
    unsigned int nFirstOffset;
    unsigned int nFreeOffset;
} spriteareastr;

typedef spriteareastr *spritearea;

typedef struct
{
  int  nNextOffset;
  char name[12];
  int  width;
  int  height;
  unsigned int  leftbit;
  unsigned int  rightbit;
  int  imageoffset;
  int  maskoffset;
  int  screenmode;
} spritestr;

typedef spritestr *sprite;

#endif
