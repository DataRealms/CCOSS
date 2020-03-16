//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUIBanner.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUIBanner class
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUIBanner.h"
#include "ContentFile.h"

namespace RTE {

map<string, GUIBanner::FontChar *> GUIBanner::m_sFontCache;
map<string, int> GUIBanner::m_sCharCapCache;


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIBanner
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIBanner object in system
//                  memory.

GUIBanner::GUIBanner()
{
    m_pFontImage[REGULAR] = 0;
    m_pFontImage[BLURRED] = 0;
    memset(m_aaFontChars[REGULAR], 0, sizeof(FontChar) * MAXBANNERFONTCHARS);
    memset(m_aaFontChars[BLURRED], 0, sizeof(FontChar) * MAXBANNERFONTCHARS);
    m_CharIndexCap = MAXBANNERFONTCHARS;
    m_FontHeight = 0;
    m_Kerning = 0;
    m_BannerText.clear();
    m_BannerChars.clear();
    m_TargetSize.Reset();
    m_BannerPosY = 240;
    m_FlySpeed = 1500;
    m_FlySpacing = 100;
    m_BannerChars.clear();
    m_AnimMode = BLINKING;
    m_AnimState = NOTSTARTED;
    m_TotalAnimTimer.Reset();
    m_DisplayTimer.Reset();
    m_SpacingTimer.Reset();
    m_FrameTimer.Reset();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Create the font from an image file.

bool GUIBanner::Create(const std::string fontFilePath, const std::string fontBlurFilePath, int bitDepth)
{
    // Package the font bitmap paths o they are more easily processed below
    string filePaths[2] = {fontFilePath, fontBlurFilePath};

    // Now process them and extract the character data from each
    ContentFile fontFile;
    map<string, FontChar *>::iterator fontItr;
    map<string, int>::iterator indexItr;
    int y, dotColor;
    for (int mode = REGULAR; mode < FONTMODECOUNT; ++mode)
    {
        // Load the font images
        fontFile.SetDataPath(filePaths[mode].c_str());
        m_pFontImage[mode] = fontFile.GetAsBitmap(bitDepth == 8 ? COLORCONV_REDUCE_TO_256 : COLORCONV_8_TO_32);
        RTEAssert(m_pFontImage[mode], "Couldn't load font bitmap for banner font from this file:\n" + fontFilePath);

        // Check the color key to be the same color as the Bottom-Right hand corner pixel
        int keyColor = getpixel(m_pFontImage[mode], m_pFontImage[mode]->w - 1, m_pFontImage[mode]->h - 1);
// No need (or way!) to actually set it; it's assumed to be bright pink (255, 255, 0)
//        m_pFontImage[mode]->SetColorKey(keyColor);

        // Check if these files have already been read and loaded from disk before.
        fontItr = m_sFontCache.find(filePaths[mode]);

        if (fontItr != m_sFontCache.end())
        {
            // Yes, has been loaded previously, then use that data from memory.
            memcpy(m_aaFontChars[mode], (*fontItr).second, sizeof(FontChar) * MAXBANNERFONTCHARS);
            // Also retrieve the font max number of characters if we can
            indexItr = m_sCharCapCache.find(filePaths[mode]);
            if (indexItr != m_sCharCapCache.end())
                m_CharIndexCap = (*indexItr).second;
            else
                m_CharIndexCap = MAXBANNERFONTCHARS;

            // Calc the font height - the horizontally blurred one should be the same
            if (mode == REGULAR)
            {
                // The red separator MUST be on the Top-Left hand corner
                dotColor = getpixel(m_pFontImage[mode], 0, 0);
                m_FontHeight = 0;
                for (y = 1; y < m_pFontImage[mode]->h; y++)
                {
                    if (getpixel(m_pFontImage[mode], 0, y) == dotColor)
                    {
                        m_FontHeight = y;
                        break;
                    }
                }
            }
        }
        // Hasn't been loaded previously, so go ahead and do so now.
        else
        {
            // The red separator MUST be on the Top-Left hand corner
            dotColor = getpixel(m_pFontImage[mode], 0, 0);
            // Find the separating gap of the font lines
            // Calc the font height - the horizontally blurred one should be the same
            if (mode == REGULAR)
            {
                m_FontHeight = 0;
                for (y = 1; y < m_pFontImage[mode]->h; y++)
                {
                    if (getpixel(m_pFontImage[mode], 0, y) == dotColor)
                    {
                        m_FontHeight = y;
                        break;
                    }
                }
            }

            // Pre-calculate all the font character details
            memset(m_aaFontChars[mode], 0, sizeof(FontChar) * MAXBANNERFONTCHARS);

            int x = 1;
            y = 0;
            int charOnLine = 0;
            for (int chr = 32; chr < MAXBANNERFONTCHARS; chr++)
            {
                m_aaFontChars[mode][chr].m_Offset = x;

                // Find the next red pixel
                int w = 0;
                int n;
                for (n = x; n < m_pFontImage[mode]->w; n++, w++)
                {
                    if (getpixel(m_pFontImage[mode], n, y) == dotColor)
                    {
                        break;
                    }
                }

                // Calculate the maximum height of the character
                int Height = 0;
                for (int j = y; j < y + m_FontHeight; j++)
                {
                    for (int i = x; i < x + w; i++)
                    {
                        int Pixel = getpixel(m_pFontImage[mode], i, j);
                        if (Pixel != dotColor && Pixel != keyColor)
                            Height = MAX(Height, j - y);
                    }
                }
                
                m_aaFontChars[mode][chr].m_Height = Height;
                m_aaFontChars[mode][chr].m_Width = w - 1;
                x += w + 1;

                m_CharIndexCap = chr + 1;

                charOnLine++;
                if (charOnLine >= 16)
                {
                    charOnLine = 0;
                    x = 1;
                    y += m_FontHeight;
                    // Stop if we run out of bitmap
                    if ((y + m_FontHeight) > m_pFontImage[mode]->h)
                        break;
                }
            }
        }

        // Add the calculated charIndexcap to the cache so we can use it in other banner instances
        // that use the same font bitmap files.
        m_sCharCapCache.insert(pair<string, int>(filePaths[mode], m_CharIndexCap));
        // Also add the now calculated font char data to the cache
        // Allocate a dynamic array to throw into the map.. probably until app close
        FontChar *aNewCache = new FontChar[MAXBANNERFONTCHARS];
        // Copy the font data into the cache
        memcpy(aNewCache, m_aaFontChars[mode], sizeof(FontChar) * MAXBANNERFONTCHARS);
        // Now put it into the cache map
        m_sFontCache.insert(pair<string, FontChar *>(filePaths[mode], aNewCache));
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys the font data

void GUIBanner::Destroy()
{
    m_BannerText.clear();
    m_BannerChars.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SpaceBetween
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells how much space, in pixels, currently exists between two flying
//                  characters.

int GUIBanner::SpaceBetween(const FlyingChar &first, FontMode firstMode, const FlyingChar &second, FontMode secondMode) const
{
    if (first.m_PosX < second.m_PosX)
        return second.m_PosX - first.m_PosX - CalculateWidth(first.m_Character, firstMode);
    else
        return first.m_PosX - second.m_PosX - CalculateWidth(second.m_Character, secondMode);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ShowText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Starts the display animation of a text string in this banner's font.

void GUIBanner::ShowText(const string text, AnimMode mode, long duration, Vector targetSize, float yOnTarget, int flySpeed, int flySpacing)
{
    m_BannerText = text;
    m_AnimMode = mode;
    m_AnimState = SHOWING;
    m_TotalAnimTimer.Reset();
    m_SpacingTimer.Reset();
    m_DisplayTimer.SetRealTimeLimitMS(duration);
    m_DisplayTimer.Reset();
    m_TargetSize = targetSize;
    m_BannerPosY = (m_TargetSize.m_Y * yOnTarget) - (m_FontHeight / 2);
    m_FlySpeed = flySpeed;
    m_FlySpacing = flySpacing;

    // Clear out the old text string so we can reload it with the new text string
    m_BannerChars.clear();
    // Starting position of the first character
    int startPosX = m_AnimMode == FLYBYLEFTWARD ? m_TargetSize.m_X : -(CalculateWidth(text, BLURRED) + ((int)(text.length() - 1) * m_FlySpacing));
    // The showing position for the first character
    int showPosX = (m_TargetSize.m_X / 2) - (CalculateWidth(text, REGULAR) / 2);
    int whichChar = 0;
    for (string::const_iterator tItr = text.begin(); tItr != text.end(); ++tItr)
    {
        whichChar++;
        // Create the flying character entry
        m_BannerChars.push_back(FlyingChar((*tItr), NOTSTARTED, showPosX, m_FlySpeed));

        if (m_AnimMode == FLYBYLEFTWARD)
        {
            // Set up the starting and end positions
            m_BannerChars.back().m_StartPosX = startPosX;
            m_BannerChars.back().m_HidePosX = -CalculateWidth((*tItr), BLURRED);
        }
        // Now the characters are going reverse
        else if (m_AnimMode == FLYBYRIGHTWARD)
        {
            // Set up the starting and end positions - again, opposite of above
            m_BannerChars.back().m_StartPosX = startPosX;
            m_BannerChars.back().m_HidePosX = m_TargetSize.m_X;
        }
        // Blinking, they stay put, so no different start/end positions
        else
        {

        }

        // Set the current position to match the starting pos
        m_BannerChars.back().m_PosX = m_BannerChars.back().m_StartPosX;
        // Set up the next character's starting position
        startPosX += CalculateWidth((*tItr), BLURRED) + m_FlySpacing;
        // Set up the next character's showing position
        showPosX += CalculateWidth((*tItr), REGULAR) + GetKerning();
    }

    // If we're flying to the right we need to reverse the order of the characters
    if (m_AnimMode == FLYBYRIGHTWARD)
        m_BannerChars.reverse();

    m_FrameTimer.Reset();
    m_SpacingTimer.Reset();
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HideText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells the banner to animate away elegantly. Especially useful when
//                  a ShowText is waiting with a negative duration.

void GUIBanner::HideText()
{
    
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the position of the flying characters of this banner.

void GUIBanner::Update()
{
    double deltaS = m_FrameTimer.GetElapsedRealTimeS();

    // Only bother updating if things are visible at all
    if (m_AnimState < SHOWING || m_AnimState > HIDING)
        return;

    // Time out the showing as set
    if (m_AnimState == SHOW && m_DisplayTimer.IsPastRealTimeLimit())
        m_AnimState = HIDING;

    int flyDirection = m_AnimMode == FLYBYLEFTWARD ? -1 : 1;
    int whichChar = 0;
    // Go through every character, updating their positions and states
    list<FlyingChar>::iterator prevItr = m_BannerChars.end();
    for (list<FlyingChar>::iterator cItr = m_BannerChars.begin(); cItr != m_BannerChars.end(); ++cItr)
    {
        whichChar++;
        // Start off each character's motion at the appropriate order and timing
        if ((*cItr).m_MoveState == NOTSTARTED)
        {
//            // Get the very first character going, or any consecutive one after the previous has cleared off enough
//            if (cItr == m_BannerChars.begin() || ((*prevItr).m_MoveState >= SHOWING && SpaceBetween(*prevItr, BLURRED, *cItr, BLURRED) >= m_FlySpacing))
            // All the spacings are now set up in the starting positions, so just let them all fly at the same time
            {
// TODO: Play a starting sound?
                (*cItr).m_MoveState = SHOWING;
                (*cItr).m_Speed = m_FlySpeed * flyDirection;
            }
        }
        // Move each character along that is currently in flight
        if ((*cItr).m_MoveState == SHOWING)
        {
            (*cItr).m_PosX += (*cItr).m_Speed * deltaS;

            // Stop each character that is at or past its showing destination OR
            // before going too far into its stopped predecessor, taking blurring into account
            if ((m_AnimMode == FLYBYLEFTWARD && (*cItr).m_PosX <= (*cItr).m_ShowPosX) ||
                (m_AnimMode == FLYBYRIGHTWARD && (*cItr).m_PosX >= (*cItr).m_ShowPosX) ||
                (cItr != m_BannerChars.begin() && (*prevItr).m_MoveState == SHOW && SpaceBetween(*prevItr, REGULAR, *cItr, BLURRED) < m_Kerning))
            {
// TODO: Play a stopping sound?
                (*cItr).m_PosX = (*cItr).m_ShowPosX;
                (*cItr).m_MoveState = SHOW;
                (*cItr).m_Speed = 0;
                // If all characters are now showing, update the overall animation state of the banner to reflect that
                if (whichChar == m_BannerChars.size() && m_BannerChars.front().m_MoveState == SHOW)
                {
                    m_AnimState = SHOW;
                    m_DisplayTimer.Reset();
                }
            }
        }
        // If we're supposed to be hiding the shown chars, then do so in an orderly fashion
        if ((*cItr).m_MoveState == SHOW && m_AnimState >= HIDING)
        {
            // Get the very first character going, or any consecutive one after the previous has cleared off enough
            if (cItr == m_BannerChars.begin() || ((*prevItr).m_MoveState >= HIDING && SpaceBetween(*prevItr, BLURRED, *cItr, BLURRED) >= m_FlySpacing))
            {
                (*cItr).m_MoveState = HIDING;
                (*cItr).m_Speed = m_FlySpeed * flyDirection;
            }
        }
        // If we're hiding chars, check if they have gone off the screen completely and can now be retired
        if ((*cItr).m_MoveState == HIDING)
        {
            (*cItr).m_PosX += (*cItr).m_Speed * deltaS;

            // Stop each character that has now traveled off the screen
            if ((m_AnimMode == FLYBYLEFTWARD && (*cItr).m_PosX <= (*cItr).m_HidePosX) ||
                (m_AnimMode == FLYBYRIGHTWARD && (*cItr).m_PosX >= (*cItr).m_HidePosX))
            {
                (*cItr).m_PosX = (*cItr).m_HidePosX;
                (*cItr).m_MoveState = OVER;
// Let them travel off screen
//                (*cItr).m_Speed = 0;
                // If this was the last character to fly off screen, the the banner showing is over
                if (whichChar == m_BannerChars.size())
                    m_AnimState = OVER;
            }
        }
        // Keep traveling these suckers even off screen, because char spacings larger than half the screen will cause problems otherwise
        if ((*cItr).m_MoveState == OVER)
        {
            (*cItr).m_PosX += (*cItr).m_Speed * deltaS;
        }

        // Update the iterator we'll use on the next char to check distances to etc
        prevItr = cItr;
    }

    m_FrameTimer.Reset();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws text to a bitmap.

void GUIBanner::Draw(BITMAP *pTargetBitmap)
{
    // Only bother drawing if things are visible at all
    if (m_AnimState < SHOWING || m_AnimState > HIDING)
        return;

    // Go through every character in the banner, drawing the ones that are showing
    unsigned char c;
    int mode, charWidth, offX, offY;
    for (list<FlyingChar>::iterator cItr = m_BannerChars.begin(); cItr != m_BannerChars.end(); ++cItr)
    {
        // Only draw anything if the character is even visible
        if ((*cItr).m_MoveState >= SHOWING && (*cItr).m_MoveState <= HIDING)
        {
            // Validate the character
            c = (*cItr).m_Character;
            if (c == '\n')
            {
                
            }
            if (c == '\t')
            {
                
            }
            if (c < 0)
                c += m_CharIndexCap;
            if (c < 32 || c >= m_CharIndexCap)
                continue;

            // Figure out where on the font bitmap we're cutting out the character from
            mode = (*cItr).m_MoveState == SHOW ? REGULAR : BLURRED;
            charWidth = m_aaFontChars[mode][c].m_Width;
            offX = m_aaFontChars[mode][c].m_Offset;
            offY = ((c - 32) / 16) * m_FontHeight;

            // Draw the character onto the target
            masked_blit(m_pFontImage[mode], pTargetBitmap, offX, offY, (*cItr).m_PosX, m_BannerPosY, charWidth, m_FontHeight);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the width of a piece of text.

int GUIBanner::CalculateWidth(const std::string text, FontMode mode) const
{
    unsigned char c;
    int Width = 0;

    // Go through every character
    for(int i = 0; i < text.length(); i++)
    {
        c = text.at(i);

        // Reset line counting if newline encountered
        if (c == '\n')
        {
/* No newlines allowed in banners
            if (Width > WidestLine)
                WidestLine = Width;
            Width = 0;
*/
            continue;
        }
        if (c < 0)
            c += m_CharIndexCap;
        if (c < 32 || c >= m_CharIndexCap)
            continue;

        Width += m_aaFontChars[mode][c].m_Width;
        
        // Add kerning, IF NOT BLURRED
        //if (i < Text.length() - 1)
        if (mode != BLURRED)
            Width += m_Kerning;
    }

    return Width;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the width of a piece of text.

int GUIBanner::CalculateWidth(const char Character, FontMode mode) const
{
    unsigned char c = Character;
    if (c >= 32 && c < m_CharIndexCap)
        return m_aaFontChars[mode][c].m_Width + m_Kerning;

    return 0;
}

} // namespace RTE
