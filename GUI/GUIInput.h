#ifndef _GUIINPUT_
#define _GUIINPUT_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUIInput.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUIInput class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


namespace RTE
{


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           GUIInput
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An interface class inherited by the different types of input methods
// Parent(s):       None.
// Class history:   12/30/2003 GUIInput Created.

class GUIInput {

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

    // Mouse & Key events
    enum {
        None,
        Released,        // Has just been released
        Pushed,            // Has just been pushed down
        Repeat            // Is repeating
    } Event;

    // Mouse & Key states
    enum {
        Up,
        Down
    } State;

    // Modifiers
    enum {
        ModNone = 0x00,
        ModShift = 0x01,
        ModCtrl = 0x02,
        ModAlt = 0x04,
		ModCommand = 0x08
    } Modifier;

    // Extra keys
    enum {
        Key_None        = 0,
        Key_Backspace   = 0x00000008,
        Key_Tab         = 0x00000009,
        Key_Enter       = 0x0000000D,
        Key_Escape      = 0x0000001B,        
        Key_LeftArrow   = 0x00000086,
        Key_RightArrow  = 0x00000087,
        Key_UpArrow     = 0x00000088,
        Key_DownArrow   = 0x00000089,
        Key_Insert      = 0x00000095,
        Key_Delete      = 0x00000096,
        Key_Home        = 0x00000097,
        Key_End         = 0x00000098,
        Key_PageUp      = 0x00000099,
        Key_PageDown    = 0x0000009A
    } Keys;
        

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIInput
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIInput object in system
//                  memory.
// Arguments:       Whether the keyboard and joysticks also can control the mouse crsr.

    GUIInput(int whichPlayer, bool keyJoyMouseCursor = false);
   
    virtual ~GUIInput() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:    Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroy the screen
// Arguments:       None.

    virtual void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMouseOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the offset for the mouse input to be adjusted by. This shuold
//                  be used when the GUI is being drawn somewhere else on the screen than
//                  the upper left corner. These values shuold be from the GUI to the upper
//                  left corner.
// Arguments:       The new offset.

    void SetMouseOffset(int mouseOffsetX, int mouseOffsetY) { m_MouseOffsetX = mouseOffsetX; m_MouseOffsetY = mouseOffsetY; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMouseOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the offset for the mouse input to be adjusted by. This shuold
//                  These values shuold be from the GUI to the upper of the screen.
//                  left corner.
// Arguments:       The new offset.

    void GetMouseOffset(int &mouseOffsetX, int &mouseOffsetY) { mouseOffsetX = m_MouseOffsetX; mouseOffsetY = m_MouseOffsetY; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:    Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Input.
// Arguments:       None.

    virtual void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetKeyboard
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copies the keyboard buffer into an array. The keyboard buffer is
//					ordered by ascii code and each entry contains a GUInput::Event enum 
//					state.
// Arguments:       Buffer array.

    void GetKeyboard(unsigned char *Buffer);
	
	unsigned char GetAsciiState(unsigned char ascii);
	
	unsigned char GetScanCodeState(unsigned char scancode);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMouseButtons
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copies the mouse button states into an array
// Arguments:       State array.

    void GetMouseButtons(int *Events, int *States);


	static void SetNetworkMouseButton(int whichPlayer, int state1, int state2, int state3);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMousePosition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the mouse position
// Arguments:       Pointers to store the X and Y coordinates in

    void GetMousePosition(int *X, int *Y);


	static void SetNetworkMouseMovement(int whichPlayer, int x, int y);



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetModifier
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the key modifiers.
// Arguments:       None.

    int GetModifier();


	/// <summary>
	/// This function returns how much the mouse scroll wheel has moved. Positive integer is scroll up, negative is scroll down.
	/// </summary>
	/// <returns>Mouse scroll wheel movement in integer value.</returns>
	int GetMouseWheelChange() {
		return m_MouseWheelChange;
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

protected:

	
	enum Constants
	{
		KEYBOARD_BUFFER_SIZE = 256
	};

    // Keyboard buffer holding the key states
	unsigned char		   m_KeyboardBuffer[KEYBOARD_BUFFER_SIZE];
	unsigned char		   m_ScanCodeState[KEYBOARD_BUFFER_SIZE];

    // Mouse button states
    // Order:    Left, Middle, Right
	int            m_MouseButtonsEvents[3];
	int            m_MouseButtonsStates[3];

	static int            m_NetworkMouseButtonsEvents[4][3];
	static int            m_NetworkMouseButtonsStates[4][3];
	static int            m_PrevNetworkMouseButtonsStates[4][3];

	static bool			  m_OverrideInput;

    int            m_MouseX, m_MouseY;
	int            m_LastFrameMouseX, m_LastFrameMouseY;

	static int			   m_NetworkMouseX[4], m_NetworkMouseY[4];

	int				m_Player;

	int				m_MouseWheelChange; //!< the amount and direction that the mouse wheel has moved.

    // These offset the mouse positions so that the cursor is shifted for all events
    int            m_MouseOffsetX, m_MouseOffsetY;

    int            m_Modifier;

    // Whether the keyboard and joysticks also control the mouse
    bool           m_KeyJoyMouseCursor;
};


}; // namespace RTE


#endif  //  _GUIINPUT_