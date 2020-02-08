#ifndef _GUIBANNER_
#define _GUIBANNER_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUIBanner.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUIBanner class
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


#include "Vector.h"
#include "Timer.h"
#include "allegro.h"

struct BITMAP;

#define MAXBANNERFONTCHARS 256

namespace RTE
{


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           GUIBanner
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A class to handle the drawing of LARGE text banners that fly across
//                  the screen, grabbing the player's attention.
// Parent(s):       None.
// Class history:   5/7/2011 GUIBanner Created.

class GUIBanner {


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

    enum FontMode
    {
        REGULAR = 0,
        BLURRED,
        FONTMODECOUNT
    };

    enum AnimMode
    {
        BLINKING = 0,
        FLYBYLEFTWARD,
        FLYBYRIGHTWARD,
        ANIMMODECOUNT
    };
    
    enum AnimState
    {
        NOTSTARTED = 0,
        SHOWING,
        SHOW,
        HIDING,
        OVER,
        ANIMSTATECOUNT
    };

    // Font character
    struct FontChar {
        int m_Width;
        int m_Height;
        int m_Offset;
    };

    // Flying characters
    struct FlyingChar {
        FlyingChar(char character, AnimState state, int showPosX, float speed) {m_Character = character; m_MoveState = state; m_PosX = m_StartPosX = m_ShowPosX = m_HidePosX = showPosX; m_Speed = speed; }
        char m_Character;
        AnimState m_MoveState;
        int m_PosX;
        int m_StartPosX;
        int m_ShowPosX;
        int m_HidePosX;
        float m_Speed;
    };


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIBanner
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIBanner object in system
//                  memory.
// Arguments:       None.
// Return value:    None.

    GUIBanner();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Create the font from an image file.
// Arguments:       Path to the font bitmap file.
//                  Path to the blurred font bitmap file.
//                  At which color bit depth to load the font files as.
// Return value:    None.

    bool Create(const std::string fontFilePath, const std::string fontBlurFilePath, int bitDepth);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys the font data
// Arguments:       None.
// Return value:    None.

    void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBannerText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the currently displayed text string.
// Arguments:       None.
// Return value:    The currently displayed text string.

    std::string GetBannerText() const { return m_BannerText; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAnimState
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current state of the overall animation of this banner.
// Arguments:       None.
// Return value:    The current state of the animation.

    AnimState GetAnimState() const { return m_AnimState; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsVisible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether this banner is currently showing anything on screen.
// Arguments:       None.
// Return value:    Whether this is showing anything presently.

    bool IsVisible() const { return m_AnimState >= SHOWING && m_AnimState <= HIDING; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the width of a piece of text.
// Arguments:       Text.
// Return value:    None.

    int CalculateWidth(const std::string Text, FontMode mode) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the width of a piece of text.
// Arguments:       Character.
// Return value:    None.

    int CalculateWidth(const char Character, FontMode mode) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFontHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the font height.
// Arguments:       None.
// Return value:    The font height in pixels.

    int GetFontHeight() const { return m_FontHeight; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetKerning
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get the character kerning (spacing)
// Arguments:       None.
// Return value:    Spacing between characters, in pixels. 1 = one empty pixel
//                  between chars, 0 = chars are touching.

    int GetKerning() const { return m_Kerning; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetKerning
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Set the character kerning (spacing), in pixels. 1 = one empty pixel
//                  between chars, 0 = chars are touching.
// Arguments:       The new kerning value.
// Return value:    None.

    void SetKerning(int newKerning = 1) { m_Kerning = newKerning; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SpaceBetween
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells how much space, in pixels, currently exists between two flying
//                  characters.
// Arguments:       The first FlyingChar.
//                  The font mode of the first character.
//                  The second FlyingChar.
//                  The font mode of the second character.
// Return value:    The space, in pixels.

    int SpaceBetween(const FlyingChar &first, FontMode firstMode, const FlyingChar &second, FontMode secondMode) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ShowText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Starts the display animation of a text string in this banner's font.
//                  This only needs to be called once per display animation. Any currently
//                  played animation will be interrupted and the banner restarted.
// Arguments:       The text to display.
//                  The animation mode to display the text in.
//                  The duration of the animation the text is displayed in. Negative value
//                  means the text pauses at the display/center until HideText is called.
//                  The width and height of the bitmap target this will be displayed on.
//                  The Y position the banner should appear on the target, in normalized
//                  value. 0.5 = banner midline is centered on halfway down the target.
//                  The speed at which the characters will fly, in pixels per second.
//                  The spacing between the flying characters, in pixels.
// Return value:    None.

    void ShowText(const std::string text, AnimMode mode, long duration, Vector targetSize, float yOnTarget, int flySpeed = 1500, int flySpacing = 100);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HideText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells the banner to animate away elegantly. Especially useful when
//                  a ShowText is waiting with a negative duration.
// Arguments:       The speed at which the characters will fly, in pixels per second.
//                  The spacing between the flying characters, in pixels.
// Return value:    None.

    void HideText(int flySpeed = 1500, int flySpacing = 100) { if (m_AnimState <= SHOW) { m_AnimState = HIDING; } m_FlySpeed = flySpeed; m_FlySpacing = flySpacing; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Abruptly clears any text without animating it away. Resets this thing.
// Arguments:       None.
// Return value:    None.

    void ClearText() { m_BannerText.clear(); m_BannerChars.clear(); m_AnimState = NOTSTARTED; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the position of the flying characters of this banner.
// Arguments:       None.
// Return value:    None.

    void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws text to a bitmap.
// Arguments:       The target bitmap to draw to.
// Return value:    None.

    void Draw(BITMAP *pTargetBitmap);


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

    // Font bitmap files - not owned
    BITMAP *m_pFontImage[FONTMODECOUNT];

    // The loaded font information for each filepath to a font bitmap
    static std::map<std::string, FontChar *> m_sFontCache;
    // Cache of the highest indices of valid characters that was read in from the file
    static std::map<std::string, int> m_sCharCapCache;

    // The actual character info for this specific banner font
    FontChar m_aaFontChars[FONTMODECOUNT][MAXBANNERFONTCHARS];
    // The highest index of valid characters that was read in from the file
    int m_CharIndexCap;

    // Height of the font
    int m_FontHeight;
    // Spacing between characters, in pixels
    int m_Kerning;

    // The text string currently being displayed
    std::string m_BannerText;
    // The actual characters of this banner and their positions etc
    std::list<FlyingChar> m_BannerChars;
    // The dimensions of the screen area that this banner is displayed on
    Vector m_TargetSize;
    // The pixel Y position that the banner has on the target
    int m_BannerPosY;
    // The speed at which the characters will fly, in pixels per second
    int m_FlySpeed;
    // The spacing between characters when they fly, in pixels
    int m_FlySpacing;
    // The mode of animation to show the above text in
    AnimMode m_AnimMode;
    // The current state of the animation
    AnimState m_AnimState;
    // The timer that keeps track of the total display animation.
    Timer m_TotalAnimTimer;
    // The timer that keeps track of how long to wait with the banner displaying before animating it away.
    Timer m_DisplayTimer;
    // Timer for keeping track of how long between individual characters are shown
    Timer m_SpacingTimer;
    // Timer for keeping track of how long passed since last call to Update
    Timer m_FrameTimer;
};

}; // namespace RTE

#endif  //  _GUIBanner_