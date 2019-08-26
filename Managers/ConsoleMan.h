#ifndef _RTECONSOLEMAN_
#define _RTECONSOLEMAN_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            ConsoleMan.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the ConsoleMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "DDTTools.h"
#include "Singleton.h"
#define g_ConsoleMan ConsoleMan::Instance()

#include "Serializable.h"
#include "Sound.h"

#include <deque>

namespace RTE
{

class GUIScreen;
class GUIInput;
class GUIControlManager;
class GUICollectionBox;
class GUITextBox;
class GUILabel;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           ConsoleMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The singleton manager of the lua console.
// Parent(s):       Singleton, Serializable?
// Class history:   5/7/2008 ConsoleMan created.

class ConsoleMan:
    public Singleton<ConsoleMan>//,
//    public Serializable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     ConsoleMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a ConsoleMan object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    ConsoleMan() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~ConsoleMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a ConsoleMan object before deletion
//                  from system memory.
// Arguments:       None.

    ~ConsoleMan() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ConsoleMan object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(bool logToCli = false);

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a Reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the Reader's position is untouched.
// Arguments:       The name of the property to be read.
//                  A Reader lined up to the value of the property to be read.
// Return value:    An error return value signaling whether the property was successfully
//                  read or not. 0 means it was read successfully, and any nonzero indicates
//                  that a property of that name could not be found in this or base classes.

    virtual int ReadProperty(std::string propName, Reader &reader);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this ConsoleMan to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the ConsoleMan will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire ConsoleMan, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ConsoleMan object.
// Arguments:       None.
// Return value:    None.

    void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_ClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetConsoleScreenSize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets how much of the screen that the console is covering when opened.
// Arguments:       None.
// Return value:    The ratio of the screen that is covered: 0 - 1.0.

    float GetConsoleScreenSize() const { return m_ConsoleScreenRatio; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetConsoleScreenSize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets how much of the screen that the console should cover when opened.
// Arguments:       The ratio of the screen to cover: 0 - 1.0.
// Return value:    None.

    void SetConsoleScreenSize(float screenRatio = 0.3);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Enables or disables the menu. This will animate it in and out of view.
// Arguments:       Whether to enable or disable the menu.
// Return value:    None.

    void SetEnabled(bool enable = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether the menu is enabled or not.
// Arguments:       None.
// Return value:    None.

    bool IsEnabled() const { return m_EnabledState == ENABLED || m_EnabledState == ENABLING; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsVisible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether the menu is at all visible or not.
// Arguments:       None.
// Return value:    None.

    bool IsVisible() const { return m_EnabledState != DISABLED; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ForceVisibility
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets console visible no matter what.
// Arguments:       None.
// Return value:    None.

	void ForceVisibility(bool visible);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsForceVisible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Report whether the console is forced to be always visible.
// Arguments:       None.
// Return value:    None.

	bool IsForceVisible() const { return m_ForceVisible; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PrintString
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Prints a string into the console.
// Arguments:       The string to print.
// Return value:    None.

    void PrintString(std::string toPrint);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveInputLog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Writes all the input strings to a log in the order they were entered.
// Arguments:       The filename of the file to write to.
// Return value:    None.

    void SaveInputLog(std::string filePath);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveAllText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Writes the entire console buffer to a file.
// Arguments:       The filename of the file to write to.
// Return value:    None.

    void SaveAllText(std::string filePath);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this ConsoleMan. Supposed to be done every frame
//                  before drawing.
// Arguments:       None.
// Return value:    None.

    void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this ConsoleMan's current graphical representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
// Return value:    None.

    void Draw(BITMAP *pTargetBitmap);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearLog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all previous input.
// Arguments:       None.
// Return value:    None.

    void ClearLog();


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    enum EnabledState
    {
        ENABLING = 0,
        ENABLED,
        DISABLING,
        DISABLED
    };

    // Member variables
    static const std::string m_ClassName;
    
    // GUI Screen for use by the in-game GUI
    GUIScreen *m_pGUIScreen;
    // GUI Input controller
    GUIInput *m_pGUIInput;
    // Manager of the console GUI elements
    GUIControlManager *m_pGUIController;
    // Current state of the console
    EnabledState m_EnabledState;

    // The ratio of the screen that the console should take up, from 0.1 to 1.0;
    float m_ConsoleScreenRatio;
    // Collection box of the console GUI
    GUICollectionBox *m_pParentBox;
    // The label which presents the console output
    GUILabel *m_pConsoleText;
    // The Textbox which the user types in the edited line
    GUITextBox *m_pInputTextBox;
    // Place to save the last worked on input string before deactivating the console
    std::string m_LastInputString;

    // Log of previously entered input strings
    std::deque<std::string> m_InputLog;
    // Iterator to the current position in the log
    std::deque<std::string>::iterator m_InputLogPosition;
    // The lat direction the log marker was moved. Needed so that changing directions dont need doouble tapping etc
    int m_LastLogMove;

    // Sound for enabling and disabling menu
    Sound m_EnterMenuSound;
    Sound m_ExitMenuSound;
    // Sound for erroneus input
    Sound m_UserErrorSound;

	// If true the console is drawn no matter what
	bool m_ForceVisible;

    bool m_LogToCli;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ConsoleMan, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    ConsoleMan(const ConsoleMan &reference);
    ConsoleMan & operator=(const ConsoleMan &rhs);

};

} // namespace RTE

#endif // File