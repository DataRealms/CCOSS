#ifndef _DDTTOOLS_
#define _DDTTOOLS_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            DDTTools.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header to include useful development tools and preprocessor tricks.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com

#include "DDTError.h"
#include "time.h"
#include "Constants.h"
#include <cmath>

struct TexMapTable;

#define DMax(a, b) (((a) > (b)) ? (a) : (b))
#define DMin(a, b) (((a) < (b)) ? (a) : (b))

namespace RTE {

class Vector;


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: SeedRand
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Seeds the rand with the current runtime time.

void SeedRand();


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: PosRand
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A good rand function that return a float between 0.0 and 0.999

double PosRand();


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: NormalRand
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A good rand function that returns a floating point value between -1.0
//                  and 1.0, both inclusive.

double NormalRand()/* { return (static_cast<double>(rand()) / (RAND_MAX / 2)) - 1.0; }*/;


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: RangeRand
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A good rand function that returns a floating point value between two
//                  given thresholds, the min being inclusive, but the max not.

double RangeRand(float min, float max)/* { return min + ((max - min) * PosRand()); }*/;


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: SelectRand
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A rand function that returns an int between min and max, both inclusive.

int SelectRand(int min, int max);


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: LERP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Simple Linear Interpolation, returns y for the last x passed in.

float LERP(float xStart, float xEnd, float yStart, float yEnd, float xPos);


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: EaseIn
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Nonlinear ease-in interpolation. Starts slow.
// Arguments:       Start, end values, and the normalized positive progress scalar (0 - 1.0).

float EaseIn(float x1, float x2, float y);


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: EaseOut
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Nonlinear ease-out interpolation. Slows down toward the end.
// Arguments:       Start, end values, and the normalized positive progress scalar (0 - 1.0).

float EaseOut(float x1, float x2, float y);


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: EaseInOut
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Nonlinear ease-in-out interpolation. Slows down in the start and end.
// Arguments:       Start, end values, and the normalized positive progress scalar (0 - 1.0).

float EaseInOut(float x1, float x2, float y);


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: Clamp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clamps a value between two limit values.
// Arguments:       Value to clamp, the upper and the lower limit values.
// Return value:    Whether the value was clamped or not.

bool Clamp(float &value, float upperLimit, float lowerLimit);


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: Limit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clamps a value between two limit values.
// Arguments:       Value to clamp, the upper and the lower limit values.
// Return value:    The clamped value.

float Limit(float value, float upperLimit, float lowerLimit);


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: WithinBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a point is within a specified box.

bool WithinBox(Vector &point, Vector &topLeft, float width, float height);


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: WithinBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a point is within a specified box.

bool WithinBox(Vector &point, float left, float top, float right, float bottom);


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: XORStrings
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Encrypts/decrypts one string with another with XOR encryption.
//                  Returns the result. Inputs don't have to be the same length.
//                  Won't encrypt any chars in the toEncrypt string that match the exception.

std::string XORStrings(std::string toEncrypt, std::string keyString, char exception = ' ');


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: OpenBrowserToURL
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fires up the default browser for the current OS on a specific URL.
// Arguments:       A string with the URL to send the browser to.
// Return value:    None.

void OpenBrowserToURL(std::string goToURL);


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: XTEAEncrypt
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Encrypts one string with a key with XTEA encryption.
//                  Returns the result.
//                  http://en.wikipedia.org/wiki/XTEA#Implementations

std::string XTEAEncrypt(std::string toEncrypt, std::string keyString, int numRounds = 32);


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: XTEADecrypt
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Decrypts one string with a key with XTEA encryption.
//                  Returns the result.
//                  http://en.wikipedia.org/wiki/XTEA#Implementations

std::string XTEADecrypt(std::string encrypted, std::string keyString, int numRounds = 32);


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: ASCIIFileContainsString
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Searches through an ascii file on disk for a specific string and tells
//                  whether it was found or not.
// Arguments:       The path to the ascii file to search.
//                  The exact string to look for; case sensitive.
// Return value:    Whether the file was found, AND that string was found in that that file.

bool ASCIIFileContainsString(std::string filePath, std::string findString);

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Global function: DrawMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a regular (non rotozoomed)BITMAP's silhuette as a solid
//                  color/material index to a BITMAP of choice.
// Arguments:       A pointer to the BITMAP whose silhuette will be drawn.
//                  A pointer to a BITMAP to draw on.
//                  A color/material index that the silhuette will be filled with.
// Return value:    None.

void DrawMaterial(BITMAP *pSprite, BITMAP *pTargetBitmap, unsigned char fillByte);


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: DrawMaterialRotoZoomed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a rotated BITMAP's silhuette as a solid color/material index
//                  to a BITMAP of choice.
// Arguments:       A pointer to the BITMAP whose silhuette will be drawn.
//                  A pointer to a BITMAP to draw on.
//                  A color/material index that the silhuette will be filled with.
// Return value:    None.

void DrawMaterialRotoZoomed(BITMAP *pSprite, BITMAP *pTargetBitmap, unsigned char fillByte);
*/

} // Namespace RTE

#endif // File
