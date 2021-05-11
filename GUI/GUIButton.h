#ifndef _GUIBUTTON_
#define _GUIBUTTON_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUIButton.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUIButton class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


namespace RTE
{

    class GUILabel;

//////////////////////////////////////////////////////////////////////////////////////////
// Class:           GUIButton
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A button control class.
// Parent(s):       GUIControl, Panel.
// Class history:   1/6/2004 GUIButton Created.

class GUIButton :
    public GUIControl,
    public GUIPanel
{

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

    // Button Notifications
    enum {
        Pushed = 0,
        UnPushed,
        Clicked,
        Focused
    } Notification;


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIButton
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIButton object in
//                  system memory.
// Arguments:       GUIManager, GUIControlManager.

    GUIButton(GUIManager *Manager, GUIControlManager *ControlManager);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.
// Arguments:       Name, Position.

    void Create(const std::string Name, int X, int Y, int Width = -1, int Height = -1) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been destroyed.
// Arguments:       None.

    void Destroy() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.
// Arguments:       Properties.

    void Create(GUIProperties *Props) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeSkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the skin has been changed.
// Arguments:       New skin pointer.

    void ChangeSkin(GUISkin *Skin) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the panel
// Arguments:       Screen class

    void Draw(GUIScreen *Screen) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  OnMouseDown
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes down on the panel
// Arguments:       Mouse Position, Mouse Buttons, Modifier.

    void OnMouseDown(int X, int Y, int Buttons, int Modifier) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  OnMouseUp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes up on the panel
// Arguments:       Mouse Position, Mouse Buttons, Modifier.

    void OnMouseUp(int X, int Y, int Buttons, int Modifier) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseMove
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse moves (over the panel, or when captured).
// Arguments:       Mouse Position, Mouse Buttons, Modifier.

    void OnMouseMove(int X, int Y, int Buttons, int Modifier) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseEnter
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse enters the panel.
// Arguments:       Mouse Position, Mouse Buttons, Modifier.

    void OnMouseEnter(int X, int Y, int Buttons, int Modifier) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseLeave
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse leaves the panel.
// Arguments:       Mouse Position, Mouse Buttons, Modifier.

    void OnMouseLeave(int X, int Y, int Buttons, int Modifier) override;

 //////////////////////////////////////////////////////////////////////////////////////////
// Method:  OnGainFocus
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the panel gains focus.
// Arguments:       None.

    void OnGainFocus() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  OnLoseFocus
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the panel looses focus.
// Arguments:       None.

    void OnLoseFocus() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  OnKeyDown
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when a key goes down.
// Arguments:       KeyCode, Modifier.

	void OnKeyDown(int KeyCode, int Modifier) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the panel of the control.
// Arguments:       None.
// Returns:         0 if the control does not have a panel, otherwise the topmost panel.

    GUIPanel * GetPanel() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetControlID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a string representing the control's ID
// Arguments:       None.

    static std::string GetControlID()    { return "BUTTON"; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetControlRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the rectangle of the control.
// Arguments:       Position, Size.

    void GetControlRect(int *X, int *Y, int *Width, int *Height) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StoreProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the control to store the values into properties.
// Arguments:       None.

    void StoreProperties() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Move
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be moved.
// Arguments:       New position.

    void Move(int X, int Y) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Resize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be resized.
// Arguments:       New size.

    void Resize(int Width, int Height) override;

    /// <summary>
    /// Gets whether or not this button is currently pushed.
    /// </summary>
    /// <returns>Whether or not this button is currently pushed.</returns>
    bool IsPushed() const { return m_Pushed; }

    /// <summary>
    /// Sets whether or not this button is currently pushed.
    /// </summary>
    /// <param name="pushed">Whether or not this button should be pushed.</param>
    void SetPushed(bool pushed = false);

    /// <summary>
    /// Gets whether or not this button is currently being moused over.
    /// </summary>
    /// <returns>Whether or not this button is currently being moused over.</returns>
    bool IsMousedOver() const { return m_Over; }

    /// <summary>
    /// Gets the text of this GUIButton's GUILabel.
    /// </summary>
    /// <returns>The text of this GUIButton's GUILabel.</returns>
    const std::string &GetText() const;

    /// <summary>
    /// Sets the text of this GUIButton's GUILabel.
    /// </summary>
    /// <param name="newText">The new text for this GUIButton's GUILabel.</param>
    /// <param name="noBitmapRebuild">Lets this method NOT rebuild the button bitmap, even if the icon has changed. Defaults to false and should almost always stay that way.</param>
    void SetText(const std::string_view &newText, bool noBitmapRebuild = false);

    /// <summary>
    /// Sets whether or not this GUIButton's text should scroll horizontally (right) when it overflows the button.
    /// </summary>
    /// <param name="newOverflowScroll">Whether or not this GUIButton's text should scroll horizontally when it overflows.</param>
    void SetHorizontalOverflowScroll(bool newOverflowScroll);

    /// <summary>
    /// Sets whether or not this GUIButton's text should scroll vertically (down) when it overflows the button.
    /// </summary>
    /// <param name="newOverflowScroll">Whether or not this GUIButton's text should scroll vertically when it overflows.</param>
    void SetVerticalOverflowScroll(bool newOverflowScroll);

    /// <summary>
    /// Gets whether or not this GUIButton has an icon with a Bitmap.
    /// </summary>
    bool HasIcon() const { return m_Icon->HasBitmap(); }

    /// <summary>
    /// Sets the icon for this GUIButton. Ownership is NOT transferred.
    /// </summary>
    /// <param name="newIcon">A pointer to the new icon BITMAP for this GUIButton.</param>
    /// <param name="noBitmapRebuild">Lets this method NOT rebuild the button bitmap, even if the icon has changed. Defaults to false and should almost always stay that way.</param>
    void SetIcon(BITMAP *newIcon, bool noBitmapRebuild = false);

    /// <summary>
    /// Helper method to set both text and icon for this GUIButton at the same time.
    /// </summary>
    /// <param name="newIcon">A pointer to the new icon BITMAP for this GUIButton.</param>
    /// <param name="newText">The new text for this GUIButton's GUILabel.</param>
    void SetIconAndText(BITMAP *newIcon, const std::string_view &newText);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Applies new properties to the control.
// Arguments:       GUIProperties.

    void ApplyProperties(GUIProperties *Props) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BuildBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Create the button bitmap to draw.
// Arguments:       None.

    void BuildBitmap();


// Members

    GUIBitmap *m_DrawBitmap;

    bool            m_Pushed;
    bool            m_Over;
    std::unique_ptr<GUILabel> m_Text;
    std::unique_ptr<GUIBitmap> m_Icon;
    std::unique_ptr<GUIRect> m_BorderSizes;
};


}; // namespace RTE


#endif  //  _GUIBUTTON_