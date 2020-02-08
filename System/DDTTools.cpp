//////////////////////////////////////////////////////////////////////////////////////////
// File:            DDTTools.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the DDTTools function.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "DDTTools.h"

#include "FrameMan.h"
#include "SceneMan.h"
#include "Vector.h"

namespace RTE { 

//////////////////////////////////////////////////////////////////////////////////////////
// Global function: SeedRand
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Seeds the rand with the current runtime time.

void SeedRand() { srand(time(0)); }


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: PosRand
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A good rand function that return a float between 0.0 and 0.999

double PosRand()
{
    return (rand() / (RAND_MAX / 1000 + 1)) / 1000.0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: NormalRand
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A good rand function that returns a floating point value between -1.0
//                  and 1.0, both inclusive.

double NormalRand()
{
    return (static_cast<double>(rand()) / (RAND_MAX / 2)) - 1.0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: RangeRand
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A good rand function that returns a floating point value between two
//                  given thresholds, the min being inclusive, but the max not.

double RangeRand(float min, float max)
{
    return min + ((max - min) * PosRand());
}


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: SelectRand
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A rand function that returns an int between min and max, both inclusive.

int SelectRand(int min, int max)
{
    return min + (int)((max - min) * PosRand() + 0.5);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: LERP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Simple Linear Interpolation, returns y for the last x passed in.

float LERP(float xStart, float xEnd, float yStart, float yEnd, float xPos)
{
    // Clamp
    if (xPos <= xStart)
        return yStart;
    else if (xPos >= xEnd)
        return yEnd;

    return yStart + ((xPos - xStart) * ((yEnd - yStart) / (xEnd - xStart)));
}


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: EaseIn
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Nonlinear ease-in interpolation. Starts slow.

float EaseIn(float x1, float x2, float y)
{
    // Clamp
    if (y <= 0)
        return x1;
    else if (y >= 1.0)
        return x2;

    float t = 1 - y;
    return (x2 - x1) * (sinf(-t * c_HalfPI) + 1) + x1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: EaseOut
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Nonlinear ease-out interpolation. Slows down toward the end.

float EaseOut(float x1, float x2, float y)
{
    // Clamp
    if (y <= 0)
        return x1;
    else if (y >= 1.0)
        return x2;

    return (x2 - x1) * -sinf(-y * c_HalfPI) + x1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: EaseInOut
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Nonlinear ease-in-out interpolation. Slows down in the start and end.

float EaseInOut(float x1, float x2, float y)
{
    return x1 * (2 * powf(y, 3) - 3 * powf(y, 2) + 1) + x2 * (3 * powf(y, 2) - 2 * powf(y, 3));
}


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: Clamp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clamps a value between two limit values.

bool Clamp(float &value, float upperLimit, float lowerLimit)
{
    // Straighten out the limits
    if (upperLimit < lowerLimit)
    {
        float temp = upperLimit;
        upperLimit = lowerLimit;
        lowerLimit = temp;
    }

    // Do the clamping
    if (value > upperLimit)
    {
        value = upperLimit;
        return true;
    }
    else if (value < lowerLimit)
    {
        value = lowerLimit;
        return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: Limit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clamps a value between two limit values.

float Limit(float value, float upperLimit, float lowerLimit)
{
    // Straighten out the limits
    if (upperLimit < lowerLimit)
    {
        float temp = upperLimit;
        upperLimit = lowerLimit;
        lowerLimit = temp;
    }

    // Do the clamping
    if (value > upperLimit)
    {
        return upperLimit;
    }
    else if (value < lowerLimit)
    {
        return lowerLimit;
    }

    return value;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: WithinBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a point is within a specified box.

bool WithinBox(Vector &point, float left, float top, float right, float bottom)
{
    return point.m_X >= left && point.m_X < right && point.m_Y >= top && point.m_Y < bottom;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: WithinBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a point is within a specified box.

bool WithinBox(Vector &point, Vector &topLeft, float width, float height)
{
    return point.m_X >= topLeft.m_X && point.m_X < (topLeft.m_X + width) &&
           point.m_Y >= topLeft.m_Y && point.m_Y < (topLeft.m_Y + height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: XORStrings
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Encrypts/decrypts one string with another with XOR encryption.
//                  Returns the result. Inputs don't have to be the same length.

string XORStrings(string toEncrypt, string keyString, char exception)
{
/* TOO SCARY! hack for build 20 to avoid having huge problem with people's old keys being read as invalid
    if (keyString.empty())
        return toEncrypt;

    char encryptChar;
    char encryptKey;
    string encryptedString = toEncrypt;

    for (int i = 0; i < toEncrypt.size(); ++i)
    {
        // Don't encrypt exceptions
        if (toEncrypt[i] == exception)
            continue;

        encryptChar = toEncrypt[i];
        // MNC suggests this change: get a rolling xor you'll never get a 0 and the encryption is easy to reverse but strong enough to hide the machineid
//        encryptKey = keyString[i % keyString.size()];
        encryptKey =  i % 16;
        // XOR only if the chars are different, or you will end up with 0 which will terminate the string prematurely!
        if (encryptChar != encryptKey)
            encryptedString[i] = encryptChar ^ encryptKey;
    }

    return encryptedString;
*/
    return toEncrypt;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: OpenBrowserToURL
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fires up the default browser for the current OS on a specific URL.

void OpenBrowserToURL(string goToURL)
{
#ifdef WIN32
    ShellExecute(NULL, "open", goToURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else // WIN32
    system(("open" + goToURL).c_str());
#endif // WIN32
}


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: ASCIIFileContainsString
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Searches through an ascii file on disk for a specific string and tells
//                  whether it was found or not.

bool ASCIIFileContainsString(string filePath, string findString)
{
    // Open the script file so we can check it out
    std::ifstream *pFile = new std::ifstream(filePath.c_str());
    if (!pFile->good())
        return false;

    char rawLine[1024];
    string line;
    string::size_type pos = 0;
    string::size_type endPos = 0;
    string::size_type commentPos = string::npos;
    bool blockCommented = false;

    while (!pFile->eof())
    {
        // Go through the script file, line by line
        pFile->getline(rawLine, 1024);
        line = rawLine;
        pos = endPos = 0;
        commentPos = string::npos;

        // Check for block comments
        if (!blockCommented && (commentPos = line.find("/*", 0)) != string::npos)
            blockCommented = true;

        // Find the end of the block comment
        if (blockCommented)
        {
            if ((commentPos = line.find("*/", commentPos == string::npos ? 0 : commentPos)) != string::npos)
            {
                blockCommented = false;
                pos = commentPos;
            }
        }

        // Process the line as usual
        if (!blockCommented)
        {
            // See if this line is commented out anywhere
            commentPos = line.find("//", 0);
            do
            {
                // Find the string
                pos = line.find(findString.c_str(), pos);
                if (pos != string::npos && pos < commentPos)
                {
                    // Found it!
                    delete pFile;
                    pFile = 0;
                    return true;
                }
            }
            while(pos != string::npos && pos < commentPos);
        }
    }

    // Didn't find the search string
    delete pFile;
    pFile = 0;
    return false;
}


///////////////////////////
// Commence ugly hacking

///////////////////////////////////
// Begin total hacking

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Global function: DrawMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a regular (non rotozoomed)BITMAP's silhuette as a solid
//                  color/material index to a BITMAP of choice.

void DrawMaterial(BITMAP *pSprite, BITMAP *pTargetBitmap, unsigned char fillByte)
{
//HRESULT BITMAP::Draw(BITMAP* pTargetBitmap, unsigned long ScrnWorldX, unsigned long ScrnWorldY, WORD BltType)

    RECT SpriteDestRect;    // Sprite RECT is defined in world space
    RECT SpriteSrcRect;        // RECT defining the source area for the BLT
    unsigned long TilesInWidth;        // Number of tiles in width of sprite tile bitmap
    unsigned long SpriteSrcX;
    unsigned long SpriteSrcY;
    int blockWidth, blockHeight;
//    HRESULT rval;

    // Get block width and height
    blockWidth  = pSprite->GetTile()->m_BlockWidth;
    blockHeight = pSprite->GetTile()->m_BlockHeight;

    // Validate the screen world space coordinates.
//    if ( (ScrnWorldX == 0) || (ScrnWorldX < 0) ) 
      unsigned long ScrnWorldX = 0;
//    if ( (ScrnWorldY == 0) || (ScrnWorldY < 0) ) 
      unsigned long ScrnWorldY = 0;

    // RECT defining the sprite in world space.
    SpriteDestRect.top    = pSprite->GetPosY();
    SpriteDestRect.left   = pSprite->GetPosX();
    SpriteDestRect.bottom = pSprite->GetPosY() + blockHeight;
    SpriteDestRect.right  = pSprite->GetPosX() + blockWidth;

    // Get the number of tiles in the sprite tile bitmap width
    TilesInWidth = pSprite->GetTile()->GetWidth() / blockWidth;

    // Calc the upper left corner of the current frame of animation
    SpriteSrcX = (pSprite->GetFrame() % TilesInWidth) * blockWidth;
    SpriteSrcY = (pSprite->GetFrame() / TilesInWidth) * blockHeight;
    
    // Define the source RECT for the BLT.
    SpriteSrcRect.top    = SpriteSrcY;
    SpriteSrcRect.left   = SpriteSrcX;
    SpriteSrcRect.bottom = SpriteSrcY + blockHeight;
    SpriteSrcRect.right  = SpriteSrcX + blockWidth;

//  rval = TransBlt(pTargetBitmap, SpriteSrcRect, SpriteDestRect);
//  HRESULT BITMAP::TransBlt(BITMAP* pDrawBitmapDest, RECT SrcRect, RECT DestRect)

//    HRESULT rval;

//  rval = m_Tile->BITMAP::DrawTrans(pDrawBitmapDest, DestRect.left, DestRect.top, &SrcRect);
//  void MOSParticle::DrawMaterial(BITMAP *pTargetBitmap, LONG lDestX, LONG lDestY, RECT* srcRect)

//    HRESULT rval = 0;
    LONG i,j;
    LONG width, height;
    BYTE* src;
    BYTE* dest;
    unsigned long sTemp;
    unsigned long sPitch, dPitch;
    unsigned long sbuf,dbuf;
    unsigned long sColorKey;
    RECT tRect = SpriteSrcRect;

    if (pSprite->GetTile()->ValidateBlt(pTargetBitmap, &SpriteDestRect.left, &SpriteDestRect.top, &tRect) == FALSE)
        return;

    // Blt using memcpy
    width  = tRect.right - tRect.left;
    height = tRect.bottom - tRect.top;

    // Lock down both bitmaps for read and write
    pSprite->GetTile()->Lock();
    pTargetBitmap->Lock();

    LPDDbitmapDESC2 ddsd = new _DDbitmapDESC2;
    pSprite->GetTile()->GetBitmapDescriptor(ddsd);
    LPDDbitmapDESC2 ddsd2 = new _DDbitmapDESC2;
    pTargetBitmap->GetBitmapDescriptor(ddsd2);

    // Set the pitch for both bitmaps
    sPitch = ddsd->lPitch;
    dPitch = ddsd2->lPitch;

    // Initialize the pointers to the upper left hand corner of the bitmap
    src  = (BYTE*)ddsd->lpBitmap;
    dest = (BYTE*)ddsd2->lpBitmap;

    delete ddsd;
    ddsd = 0;
    delete ddsd2;
    ddsd2 = 0;

    // ColorKey
    sColorKey = (unsigned long)pSprite->GetTile()->GetColorKey();

    // Initialize the pointers to the first pixel in the rectangle
    src  += (tRect.top * sPitch) + tRect.left;
    dest += (SpriteDestRect.top * dPitch) + SpriteDestRect.left;

    // Set the horizontal padding
    sbuf = (WORD)(sPitch - width);
    dbuf = (WORD)(dPitch - width);

    i = height;
    do
    {
        j = width;
        do
        {
            sTemp = *src;

            if (sTemp != sColorKey)
                *dest = fillByte;

            src++;
            dest++;

        }while (--j > 0);

        src  += sbuf;
        dest += dbuf;

    }while (--i > 0);

    // UnLock both bitmaps 
    pSprite->GetTile()->UnLock();
    pTargetBitmap->UnLock();
}


//////////////////////////////////////////////////////////////////////////////////
// NAME: Scanrightside
// 
// PURPOSE:          
//
// INPUT: none
//
// RETURNS: 0 or > - successful
//          < 0    - failed
// 
//////////////////////////////////////////////////////////////////////////////////
void Scanrightside(int x1, int x2, int ytop, int lineheight, char side, int TexWidth, int TexHeight,RECT* dClip, TexMapTable *rightTable)
{
    int x,px,py,xadd,pxadd,pyadd,y;

    if (++lineheight < 1)lineheight=1;

    xadd = ((x2-x1)<<16)/lineheight;

    if (side==1)
    {
        px    = 0;
        py    = 0;
        pxadd = (TexWidth << 16) / lineheight;
        pyadd = 0;
    }
    if (side==2)
    {
        px    = (TexWidth-0) << 16;
        py    = 0;
        pxadd = 0;
        pyadd = (TexHeight << 16) / lineheight;
    }
    if (side==3)
    {
        px    = (TexWidth - 0) << 16;
        py    = (TexHeight - 0) << 16;
        pxadd = (-TexWidth << 16) / lineheight;
        pyadd = 0;
    }
    if (side==4)
    {
        px    = 0;
        py    = (TexHeight - 0) << 16;
        pxadd = 0;
        pyadd = (-TexHeight << 16) / lineheight;
    }

    x = x1 << 16;

    for(y=0; y<=lineheight; y++)
    {
        int yp=ytop+y;
        if ( (yp >= dClip->top) && (yp < dClip->bottom) )
        {
            rightTable[yp].x  = x;
            rightTable[yp].px = px;
            rightTable[yp].py = py;
        }
        x  = x + xadd;
        px = px + pxadd;
        py = py + pyadd;
    }
}


//////////////////////////////////////////////////////////////////////////////////
// NAME: Scanleftside
// 
// PURPOSE:          
//
// INPUT: none
//
// RETURNS: 0 or > - successful
//          < 0    - failed
// 
//////////////////////////////////////////////////////////////////////////////////
void Scanleftside(int x1,int x2,int ytop,int lineheight,char side,int TexWidth,int TexHeight,RECT* dClip, TexMapTable *leftTable)
{
    int x,px,py,xadd,pxadd,pyadd,y;

    if (++lineheight<1) lineheight = 1;

    xadd = ((x2-x1) << 16) / lineheight;

    if (side == 1)
    {
        px    = (TexWidth - 0) << 16;
        py    = 0;
        pxadd = (-TexWidth << 16) / lineheight;
        pyadd = 0;
    }
    if (side == 2)
    {
        px    = (TexWidth - 0) << 16;
        py    = (TexHeight - 0) << 16;
        pxadd = 0;
        pyadd = (-TexHeight << 16) / lineheight;
    }
    if (side == 3)
    {
        px    = 0;
        py    = (TexHeight - 0) << 16;
        pxadd = (TexWidth << 16) / lineheight;
        pyadd = 0;
    }
    if (side == 4)
    {
        px    = 0;
        py    = 0;
        pxadd = 0;
        pyadd = (TexHeight << 16) / lineheight;
    }

    x = x1 << 16;

    for(y=0; y<=lineheight; y++)
    {
        int yp = ytop + y;

        if ( (yp >= dClip->top) &&( yp < dClip->bottom) )
        {
            leftTable[yp].x  = x;
            leftTable[yp].px = px;
            leftTable[yp].py = py;
        }
        x  = x + xadd;
        px = px + pxadd;
        py = py + pyadd;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: DrawMaterialRotoZoomed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a rotated BITMAP's silhuette as a solid color/material index
//                  to a BITMAP of choice.

void DrawMaterialRotoZoomed(BITMAP *pSprite, BITMAP *pTargetBitmap, unsigned char fillByte)
{
//  HRESULT BITMAP::Draw(BITMAP* pTargetBitmap, unsigned long ScrnWorldX, unsigned long ScrnWorldY, WORD BltType)

    RECT SpriteDestRect;    // Sprite RECT is defined in world space
    RECT SpriteSrcRect;        // RECT defining the source area for the BLT
    unsigned long TilesInWidth;        // Number of tiles in width of sprite tile bitmap
    unsigned long SpriteSrcX;
    unsigned long SpriteSrcY;
    int blockWidth, blockHeight;
//    HRESULT rval;

    // Get block width and height
    blockWidth  = pSprite->GetTile()->m_BlockWidth;
    blockHeight = pSprite->GetTile()->m_BlockHeight;

    // Validate the screen world space coordinates.
//    if ((ScrnWorldX == 0) || (ScrnWorldX < 0))
    unsigned long ScrnWorldX = 0;
//    if ((ScrnWorldY == 0) || (ScrnWorldY < 0))
    unsigned long ScrnWorldY = 0;

    // RECT defining the sprite in world space.
    SpriteDestRect.top    = pSprite->GetPosY();
    SpriteDestRect.left   = pSprite->GetPosX();
    SpriteDestRect.bottom = pSprite->GetPosY() + blockHeight;
    SpriteDestRect.right  = pSprite->GetPosX() + blockWidth;

    // Get the number of tiles in the sprite tile bitmap width
    TilesInWidth = pSprite->GetTile()->GetWidth() / blockWidth;

    // Calc the upper left corner of the current frame of animation
    SpriteSrcX = (pSprite->GetFrame() % TilesInWidth) * blockWidth;
    SpriteSrcY = (pSprite->GetFrame() / TilesInWidth) * blockHeight;
    
    // Define the source RECT for the BLT.
    SpriteSrcRect.top    = SpriteSrcY;
    SpriteSrcRect.left   = SpriteSrcX;
    SpriteSrcRect.bottom = SpriteSrcY + blockHeight;
    SpriteSrcRect.right  = SpriteSrcX + blockWidth;

//  TransRotoZoom(pTargetBitmap, SpriteSrcRect, SpriteDestRect);
//  HRESULT BITMAP::TransRotoZoom(BITMAP* pTargetBitmap, RECT SpriteSrcRect, RECT SpriteDestRect)

    unsigned long w,h;

    w = SpriteSrcRect.right - SpriteSrcRect.left;
    h = SpriteSrcRect.bottom - SpriteSrcRect.top;

//  m_Tile->BITMAP::DrawTransRotoZoom(pTargetBitmap, SpriteDestRect.left + w/2, SpriteDestRect.top + h/2, &SpriteSrcRect, m_Angle, m_ScaleFactor);
//  HRESULT BITMAP::DrawTransRotoZoom(BITMAP* pTargetBitmap, int midX, int midY, RECT* area, double m_Rotation, double m_Scale)

    double SinA,CosA;
    BOOL bTrans = TRUE;

    // Clip the source rect to the extents of the bitmap.
    pSprite->GetTile()->ClipRect(&SpriteSrcRect);

    int HalfWidth  = int(double((SpriteSrcRect.right  - SpriteSrcRect.left) * pSprite->GetScale() / 2));
    int HalfHeight = int(double((SpriteSrcRect.bottom - SpriteSrcRect.top)  * pSprite->GetScale() / 2));

    SinA = sin(-pSprite->GetAngle());
    CosA = cos(-pSprite->GetAngle());

    int x1 = (int)(CosA * -HalfWidth - SinA * -HalfHeight);
    int y1 = (int)(SinA * -HalfWidth + CosA * -HalfHeight);
    int x2 = (int)(CosA * HalfWidth - SinA * -HalfHeight);
    int y2 = (int)(SinA * HalfWidth + CosA * -HalfHeight);
    int x3 = (int)(CosA * HalfWidth - SinA * HalfHeight);
    int y3 = (int)(SinA * HalfWidth + CosA * HalfHeight);
    int x4 = (int)(CosA * -HalfWidth - SinA * HalfHeight);
    int y4 = (int)(SinA * -HalfWidth + CosA * HalfHeight);

    x1 += SpriteDestRect.left + w/2;
    y1 += SpriteDestRect.top + h/2;
    x2 += SpriteDestRect.left + w/2;
    y2 += SpriteDestRect.top + h/2;
    x3 += SpriteDestRect.left + w/2;
    y3 += SpriteDestRect.top + h/2;
    x4 += SpriteDestRect.left + w/2;
    y4 += SpriteDestRect.top + h/2;

//  TextureMap(pTargetBitmap, x1+midX, y1+midY, x2+midX, y2+midY, x3+midX, y3+midY, x4+midX, y4+midY, area, bTrans);
//  void BITMAP::TextureMap(BITMAP *pTargetBitmap, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, RECT* area, BOOL bTrans)

    BYTE* lpSrc;
    BYTE* lpDest;
    unsigned long dPitch, sPitch;
    unsigned long colorKey;
    RECT* dClipRect;

    // Get the ClipRect for the destination bitmap.
    dClipRect = pTargetBitmap->GetClipRect();

    TexMapTable* leftTable = 0;
    TexMapTable* rightTable = 0;

    //Screen->TexMapTableHeight = m_ClipRect.bottom;
    leftTable  = new TexMapTable[dClipRect->bottom];
    rightTable = new TexMapTable[dClipRect->bottom];

    int TexWidth  = SpriteSrcRect.right  - SpriteSrcRect.left;
    int TexHeight = SpriteSrcRect.bottom - SpriteSrcRect.top;

    int miny = y1;
    int maxy = y1;
    if (y2<miny) miny = y2;
    if (y2>maxy) maxy = y2;
    if (y3<miny) miny = y3;
    if (y3>maxy) maxy = y3;
    if (y4<miny) miny = y4;
    if (y4>maxy) maxy = y4;

    if (miny >= maxy) return;
    if (maxy < dClipRect->top) return;
    if (miny >= dClipRect->bottom) return;
    if (miny < dClipRect->top) miny = dClipRect->top;
    if (maxy > dClipRect->bottom) maxy = dClipRect->bottom;
    if (maxy - miny<1)return;

    if (y2 < y1) {Scanleftside(x2,x1,y2,y1-y2,1,TexWidth,TexHeight, dClipRect, leftTable);}
    else        {Scanrightside(x1,x2,y1,y2-y1,1,TexWidth,TexHeight, dClipRect, rightTable);}

    if (y3 < y2) {Scanleftside (x3,x2,y3,y2-y3,2,TexWidth,TexHeight, dClipRect, leftTable);}
    else        {Scanrightside (x2,x3,y2,y3-y2,2,TexWidth,TexHeight, dClipRect, rightTable);}

    if (y4 < y3) {Scanleftside (x4,x3,y4,y3-y4,3,TexWidth,TexHeight, dClipRect, leftTable);}
    else        {Scanrightside (x3,x4,y3,y4-y3,3,TexWidth,TexHeight, dClipRect, rightTable);}

    if (y1 < y4) {Scanleftside (x1,x4,y1,y4-y1,4,TexWidth,TexHeight, dClipRect, leftTable);}
    else        {Scanrightside (x4,x1,y4,y1-y4,4,TexWidth,TexHeight, dClipRect, rightTable);}

    int polyx1,polyx2,y,linewidth,pxadd,pyadd;
    int texX,texY;

    // Lock down both bitmaps for read and write
    pSprite->GetTile()->Lock();
    pTargetBitmap->Lock();

    LPDDbitmapDESC2 ddsd = new _DDbitmapDESC2;
    pSprite->GetTile()->GetBitmapDescriptor(ddsd);
    LPDDbitmapDESC2 ddsd2 = new _DDbitmapDESC2;
    pTargetBitmap->GetBitmapDescriptor(ddsd2);

    // Set the pitch for both bitmaps
    sPitch = ddsd->lPitch;
    dPitch = ddsd2->lPitch;

    // Initialize the pointers to the upper left hand corner of the bitmap
    lpSrc  = (BYTE*)ddsd->lpBitmap;
    lpDest = (BYTE*)ddsd2->lpBitmap;

    delete ddsd;
    ddsd = 0;
    delete ddsd2;
    ddsd2 = 0;

    // Get the color key for sprite bitmap
    colorKey = (unsigned long)pSprite->GetTile()->GetColorKey();

    for(y=miny; y<=maxy; y++) {
        polyx1 = leftTable[y].x >> 16;
        polyx2 = rightTable[y].x >> 16;
        linewidth = polyx2 - polyx1;
        if (linewidth < 1) linewidth = 1;
        pxadd = ((rightTable[y].px)-(leftTable[y].px)) / linewidth;
        pyadd = ((rightTable[y].py)-(leftTable[y].py)) / linewidth;

        texX = leftTable[y].px;
        texY = leftTable[y].py;

        if (polyx1 < dClipRect->left)
        {
            texX  += pxadd * (dClipRect->left - polyx1);
            texY  += pyadd * (dClipRect->left - polyx1);
            polyx1 = dClipRect->left;
        }
        if (polyx2 > dClipRect->right - 0) {polyx2 = dClipRect->right - 0;}

        texX += SpriteSrcRect.left << 16;
        texY += SpriteSrcRect.top << 16;
        unsigned char* dst = (lpDest + (y * dPitch) + polyx1 );

        for(int x=polyx1+1; x<polyx2; x++) {
            texX += pxadd;
            texY += pyadd;
            unsigned char c = *(lpSrc + ((texY >> 16) * sPitch) + (texX >> 16) );

            if (c != colorKey)
                *dst++ = fillByte;
            else
                dst++;
        }
    }

    delete leftTable; leftTable = 0;
    delete rightTable; rightTable = 0;

    pSprite->GetTile()->UnLock();
    pTargetBitmap->UnLock();
}
*/

} // Namespace RTE