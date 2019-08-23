//////////////////////////////////////////////////////////////////////////////////////////
// File:            ConsoleMan.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the ConsoleMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "ConsoleMan.h"
#include "RTEManagers.h"
#include "Writer.h"

#include "GUI/GUI.h"
#include "GUI/AllegroBitmap.h"
#include "GUI/AllegroScreen.h"
#include "GUI/AllegroInput.h"
#include "GUI/GUIControlManager.h"
#include "GUI/GUICollectionBox.h"
#include "GUI/GUITextBox.h"
#include "GUI/GUILabel.h"

#include <regex>
#include <string>
#include <sstream>
using namespace std;

namespace RTE
{

#define MENUSPEED 0.5

const string ConsoleMan::m_ClassName = "ConsoleMan";


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ConsoleMan, effectively
//                  resetting the members of this abstraction level only.

void ConsoleMan::Clear()
{
    m_pGUIScreen = 0;
    m_pGUIInput = 0;
    m_pGUIController = 0;
    m_EnabledState = DISABLED;
    m_ConsoleScreenRatio = 0.333;
    m_pParentBox = 0;
    m_pConsoleText = 0;
    m_pInputTextBox = 0;
    m_LastInputString.clear();
    m_InputLog.clear();
    m_InputLogPosition = m_InputLog.begin();
    m_LastLogMove = 0;
    m_EnterMenuSound.Reset();
    m_ExitMenuSound.Reset();
    m_UserErrorSound.Reset();
	m_ForceVisible = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ConsoleMan object ready for use.

int ConsoleMan::Create(bool logToCli)
{
    m_LogToCli = logToCli;
    if (!m_pGUIScreen)
        m_pGUIScreen = new AllegroScreen(g_FrameMan.GetBackBuffer32());
    if (!m_pGUIInput)
        m_pGUIInput = new AllegroInput(-1);
    if (!m_pGUIController)
        m_pGUIController = new GUIControlManager();
    // Use the MainMenu skin because it uses 32bpp images and will work since we only draw to 32bpp bitmaps
    if(!m_pGUIController->Create(m_pGUIScreen, m_pGUIInput, "Base.rte/GUIs/Skins/MainMenu", "ConsoleSkin.ini"))
        DDTAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/MainMenu/ConsoleSkin.ini");
    m_pGUIController->Load("Base.rte/GUIs/ConsoleGUI.ini");
    m_pGUIController->EnableMouse(false);

    // Stretch the invisible root box to fill the screen
    dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("base"))->SetSize(g_FrameMan.GetResX(), g_FrameMan.GetResY());

    // Make sure we have convenient points to teh containing GUI colleciton boxes that we will manipulate the positions of
    if (!m_pParentBox)
    {
        m_pParentBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("ConsoleGUIBox"));

        // Set the background image of the parent collection box
//        ContentFile backgroundFile("Base.rte/GUIs/BuyMenuHeader.bmp");
//        m_pParentBox->SetDrawImage(new AllegroBitmap(backgroundFile.GetAsBitmap()));
//        m_pParentBox->SetDrawBackground(true);
//        m_pParentBox->SetDrawType(GUICollectionBox::Image);
        m_pParentBox->SetDrawType(GUICollectionBox::Color);
    }
    m_pConsoleText = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("ConsoleLabel"));
    m_pInputTextBox = dynamic_cast<GUITextBox *>(m_pGUIController->GetControl("InputTB"));

    // Stretch the parent box to fill the top of the screen widthwise, and as far down as the setting in settings.ini says
    SetConsoleScreenSize(m_ConsoleScreenRatio);

    // Hide the parent box out of view
    m_pParentBox->SetPositionAbs(0, -m_pParentBox->GetHeight());
    m_pParentBox->SetEnabled(false);
    m_pParentBox->SetVisible(false);

    // Set a nice intro text in the console
    m_pConsoleText->SetText("- RTE Lua Console -\nSee the Data Realms Wiki for commands:\nhttp://www.datarealms.com/wiki/\n-------------------------------------");

    // Reset the input log
    m_InputLogPosition = m_InputLog.begin();
    m_LastLogMove = 0;

    m_EnterMenuSound.Create("Base.rte/GUIs/Sounds/MenuEnter.wav");
    m_ExitMenuSound.Create("Base.rte/GUIs/Sounds/MenuExit.wav");
    m_UserErrorSound.Create("Base.rte/GUIs/Sounds/Error.wav");

    return 0;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int ConsoleMan::ReadProperty(std::string propName, Reader &reader)
{
//    if (propName == "AddEffect")
//        g_PresetMan.GetEntityPreset(reader);
//    else
        // See if the base class(es) can find a match instead
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this ConsoleMan with a Writer for
//                  later recreation with Create(Reader &reader);

int ConsoleMan::Save(Writer &writer) const
{

// TODO: "Do this!")

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ConsoleMan object.

void ConsoleMan::Destroy()
{
//#ifdef _DEBUG
    // Dump out the text in the console
    SaveAllText("LogConsole.txt");
//#endif // _DEBUG

    delete m_pGUIController;
    delete m_pGUIInput;
    delete m_pGUIScreen;

    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetConsoleScreenSize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets how much of the screen that the console should cover when opened.

void ConsoleMan::SetConsoleScreenSize(float screenRatio)
{
    // Clamp to reasonable value
    if (screenRatio < 0.1)
        screenRatio = 0.1;
    if (screenRatio > 1.0)
        screenRatio = 1.0;

    m_ConsoleScreenRatio = screenRatio;

    // If we're not created yet, then don't do the actual adjusting yet.
    if (!m_pParentBox || !m_pConsoleText || !m_pInputTextBox)
        return;

    // Stretch the parent box to fill the top of the screen widthwise, and as far down as the parameter says
    m_pParentBox->SetSize(g_FrameMan.GetResX(), g_FrameMan.GetResY() * m_ConsoleScreenRatio);

    // Adjust the other controls to match the screen ratio
    m_pConsoleText->SetSize(m_pParentBox->GetWidth() - 4, m_pParentBox->GetHeight() - m_pInputTextBox->GetHeight() - 2);
    m_pInputTextBox->SetPositionRel(m_pInputTextBox->GetRelXPos(), m_pConsoleText->GetHeight());
    m_pInputTextBox->Resize(m_pParentBox->GetWidth() - 3, m_pInputTextBox->GetHeight());
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Enables or disables the menu. This will animate it in and out of view.

void ConsoleMan::SetEnabled(bool enable)
{
    if (enable && m_EnabledState != ENABLED && m_EnabledState != ENABLING)
    {
		if (m_ForceVisible)
			ForceVisibility(false);
        m_EnabledState = ENABLING;
/*
        // Reset repeat timers
        m_RepeatStartTimer.Reset();
        m_RepeatTimer.Reset();
        // Set the mouse cursor free
        g_UInputMan.TrapMousePos(false, m_pController->GetPlayer());
        // Move the mouse cursor to the middle of the player's screen
        int mouseOffX, mouseOffY;
        m_pGUIInput->GetMouseOffset(mouseOffX, mouseOffY);
        Vector mousePos(-mouseOffX + (g_FrameMan.GetPlayerScreenWidth() / 2), -mouseOffY + (g_FrameMan.GetPlayerScreenHeight() / 2));
        g_UInputMan.SetMousePos(mousePos, m_pController->GetPlayer());
*/
        m_EnterMenuSound.Play();
    }
    else if (!enable && m_EnabledState != DISABLED && m_EnabledState != DISABLING)
    {
        m_EnabledState = DISABLING;
        // Trap the mouse cursor again
//        g_UInputMan.TrapMousePos(true, m_pController->GetPlayer());
        // Only play switching away sound
        m_ExitMenuSound.Play();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PrintString
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Prints a string into the console.

void ConsoleMan::PrintString(string toPrint)
{
    // Add the input line to the console
    m_pConsoleText->SetText(m_pConsoleText->GetText() + "\n" + toPrint);
    if(m_LogToCli)
    {
        std::regex re_error("(ERROR|SYSTEM):");
        toPrint = std::regex_replace(toPrint, re_error, "\033[1;31m$&\033[0;0m");//red
        std::regex re_path("\\w*\\.rte\\/(\\w| |\\.|\\/)*(\\/|\\.bmp|\\.wav|\\.lua|\\.ini)");
        toPrint = std::regex_replace(toPrint, re_path, "\033[1;32m$&\033[0;0m");//green
        std::regex re_name("(\"[A-Z].*\"|\'[A-Z].*\')");
        toPrint = std::regex_replace(toPrint, re_name, "\033[1;33m$&\033[0;0m");//yellow
        std::cout << "\r" << toPrint << std::endl;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveInputLog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Writes all the input strings to a log in the order they were entered.

void ConsoleMan::SaveInputLog(std::string filePath)
{
    Writer writer(filePath.c_str());

    if (writer.WriterOK())
    {
        for (deque<string>::reverse_iterator itr = m_InputLog.rbegin(); itr != m_InputLog.rend(); ++itr)
        {
            writer << *itr;
            // Add semicolon so the line input becomes a statement
            if (!itr->empty() && (*itr)[itr->length() - 1] != ';')
                writer << ";";
            writer << "\n";
        }

        PrintString("SYSTEM: Console input log saved to " + filePath);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveAllText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Writes the entire console buffer to a file.

void ConsoleMan::SaveAllText(std::string filePath)
{
    Writer writer(filePath.c_str());

    if (writer.WriterOK())
    {
        writer << m_pConsoleText->GetText();
        PrintString("SYSTEM: Entire console contents saved to " + filePath);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ForceVisibility
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets console visible no matter what.
// Arguments:       None.
// Return value:    None.

void ConsoleMan::ForceVisibility(bool visible)
{
	m_ForceVisible = visible; 
	if (m_ForceVisible)
	{
		m_pParentBox->SetPositionAbs(0, 0);
	}
	else
	{
		m_pParentBox->SetPositionAbs(0, -m_pParentBox->GetHeight());
	}
	m_pParentBox->SetEnabled(visible);
	m_pParentBox->SetVisible(visible);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this ConsoleMan. Supposed to be done every frame
//                  before drawing.

void ConsoleMan::Update()
{
    SLICK_PROFILE(0xFF856541);

	bool inputConsumed = false;

	if ((g_UInputMan.FlagCtrlState() && g_UInputMan.KeyPressed(KEY_TILDE)) || (g_UInputMan.FlagCtrlState() && g_UInputMan.KeyPressed(KEY_SLASH)))
	{
		inputConsumed = true;
		if (IsForceVisible())
			ForceVisibility(false);
		else
			ForceVisibility(true);
	}

    ////////////////////////////////////////////////////////////////////////
    // Animate the menu into and out of view if enabled or disabled

    if (m_EnabledState == ENABLING)
    {
        m_pParentBox->SetEnabled(true);
        m_pParentBox->SetVisible(true);

        float toGo = -floorf((float)m_pParentBox->GetYPos() * MENUSPEED);
        m_pParentBox->SetPositionAbs(0, m_pParentBox->GetYPos() + toGo);

        if (m_pParentBox->GetYPos() >= 0)
            m_EnabledState = ENABLED;
    }
    // Animate the menu out of view
    else if (m_EnabledState == DISABLING)
    {
        float toGo = -ceilf(((float)m_pParentBox->GetHeight() + (float)m_pParentBox->GetYPos()) * MENUSPEED);
        m_pParentBox->SetPositionAbs(0, m_pParentBox->GetYPos() + toGo);

        if (m_pParentBox->GetYPos() <= -m_pParentBox->GetHeight())
        {
            m_pParentBox->SetEnabled(false);
            m_pParentBox->SetVisible(false);
            m_EnabledState = DISABLED;
        }
    }
    else if (m_EnabledState == ENABLED)
    {
        // If supposed to be enabled but appears to be disabled, enable it gracefully
        if (m_pParentBox->GetYPos() < 0)
            m_EnabledState = ENABLING;

        m_pParentBox->SetEnabled(true);
        m_pParentBox->SetVisible(true);
        m_pInputTextBox->SetEnabled(true);
        m_pInputTextBox->SetFocus();
    }
    else if (m_EnabledState == DISABLED)
    {
		if (!m_ForceVisible)
		{
			// If supposed to be disabled but appears to be out anyway, disable it gracefully
			if (m_pParentBox->GetYPos() > -m_pParentBox->GetHeight())
			{
				m_pParentBox->SetEnabled(true);
				m_pParentBox->SetVisible(true);
				m_EnabledState = DISABLING;
			}
			else
			{
				m_pParentBox->SetEnabled(false);
				m_pParentBox->SetVisible(false);
				m_pInputTextBox->SetEnabled(false);
				m_pGUIController->GetManager()->SetFocus(0);
			}
		}
    }

    // Toggle enabling with traditional tilde key
    if (!inputConsumed && (g_UInputMan.KeyPressed(KEY_TILDE) || g_UInputMan.KeyPressed(KEY_SLASH)))
    {
        if (IsEnabled())
        {
            m_pInputTextBox->SetEnabled(false);
            m_pGUIController->GetManager()->SetFocus(0);
            // Save any text being worked on in the input, as the box keeps getting junk added to it
            m_LastInputString = m_pInputTextBox->GetText();

            m_EnabledState = DISABLING;
            m_ExitMenuSound.Play();
        }
        else
        {
            // Set the focus and cursor position of the input line
            m_pInputTextBox->SetEnabled(true);
            m_pInputTextBox->SetFocus();
            // Restore any text being worked on in the input, as the box keeps getting junk added to it
            m_pInputTextBox->SetText(m_LastInputString);
            m_pInputTextBox->SetCursorPos(m_pInputTextBox->GetText().length());

            m_EnabledState = ENABLING;
            m_EnterMenuSound.Play();
        }
    }

    // Quit now if we aren't enabled
    if (m_EnabledState != ENABLED &&  m_EnabledState != ENABLING)
        return;

    // Update the GUI components
    m_pGUIController->Update();

    // Remove any junk input that will have been entered by the opening/closing of the console
    if (g_UInputMan.KeyPressed(KEY_TILDE) || g_UInputMan.KeyPressed(KEY_SLASH) ||
        g_UInputMan.KeyHeld(KEY_TILDE) || g_UInputMan.KeyHeld(KEY_SLASH))
    {
        // Restore any text being worked on in the input, as the box keeps getting junk added to it
        m_pInputTextBox->SetText(m_LastInputString);
        m_pInputTextBox->SetCursorPos(m_pInputTextBox->GetText().length());
    }

    // If a multi-line string was pasted into the box, we need to check for and submit all but the last line
    if (m_pInputTextBox->GetText().find_last_of('\n') != string::npos)
    {
        char strLine[1024];
        stringstream inputSStream(m_pInputTextBox->GetText());
        do
        {
            inputSStream.getline(strLine, 1024, '\n');

            // See if we have hit the end of the string, and if so, don't execute this last line, but put it back into the input line
            if (!inputSStream.fail() && !inputSStream.eof())
            {
                // Clear the errors from the VM so we will get fresh feedback
                g_LuaMan.ClearErrors();
                // Add the input line to the console
                m_pConsoleText->SetText(m_pConsoleText->GetText() + "\n" + string(strLine));
                // Run the script string
                g_LuaMan.RunScriptString(string(strLine), false);
                // Add any error feedback from the lua VM
                if (!g_LuaMan.ErrorExists())
                    m_pConsoleText->SetText(m_pConsoleText->GetText() + "\n" + "ERROR: " + g_LuaMan.GetLastError());

                // Save the input made to the log, but only if it's different from the last one
                if (strLine[0] != '\r' && (m_InputLog.empty() || m_InputLog.front().compare(strLine) != 0))
                    m_InputLog.push_front(strLine);
                // Reset the log marker
                m_InputLogPosition = m_InputLog.begin();
                m_LastLogMove = 0;
            }
        }
        while (!inputSStream.fail() && !inputSStream.eof());

        // Set the input box to contain the last line of the input string
        m_pInputTextBox->SetText(string(strLine));
        m_pInputTextBox->SetCursorPos(m_pInputTextBox->GetText().length());
    }

    // Enter pressed! Feed line input to console and lua script engine
    if (g_UInputMan.KeyPressed(KEY_ENTER) || g_UInputMan.KeyPressed(KEY_ENTER_PAD))
    {
        // Clear the errors from the VM so we will get fresh feedback
        g_LuaMan.ClearErrors();
        // Add the input line to the console
        m_pConsoleText->SetText(m_pConsoleText->GetText() + "\n" + m_pInputTextBox->GetText());
        // Run the script string
        g_LuaMan.RunScriptString(m_pInputTextBox->GetText(), false);

        // Add any error feedback from the lua VM
        if (!g_LuaMan.ErrorExists())
            m_pConsoleText->SetText(m_pConsoleText->GetText() + "\n" + "ERROR: " + g_LuaMan.GetLastError());

        // Save the input made to the log, but only if it's different from the last one
        if (!m_pInputTextBox->GetText().empty() && (m_InputLog.empty() || m_InputLog.front().compare(m_pInputTextBox->GetText()) != 0))
            m_InputLog.push_front(m_pInputTextBox->GetText());

        // Reset the log marker
        m_InputLogPosition = m_InputLog.begin();
        m_LastLogMove = 0;

        // Clear the input line
        m_pInputTextBox->SetText("");
    }

    // Can't load input log entries if the log is empty!
    if (!m_InputLog.empty())
    {
        // Up pressed! Load previous input string into the input line
        if (g_UInputMan.KeyPressed(KEY_UP))
        {
            // See if we should increment doubly because the last move was in the opposite direction
            if (m_LastLogMove < 0 && m_InputLogPosition != --(m_InputLog.end()))
                m_InputLogPosition++;

            // Revive logged input and put cursor at the end of it
            m_pInputTextBox->SetText(*m_InputLogPosition);
            m_pInputTextBox->SetCursorPos(m_pInputTextBox->GetText().length());
            m_InputLogPosition++;
            m_LastLogMove = 1;

            // Avoid falling off the end
            if (m_InputLogPosition == m_InputLog.end())
            {
                m_InputLogPosition--;
                m_LastLogMove = 0;
            }
        }
        // Down pressed! Load next logged input string into the input line
        else if (g_UInputMan.KeyPressed(KEY_DOWN))
        {
            // See if we should decrement doubly because the last move was in the opposite direction
            if (m_LastLogMove > 0 && m_InputLogPosition != m_InputLog.begin())
                m_InputLogPosition--;

            // If already at the beginning, then only put in empty string
            if (m_InputLogPosition == m_InputLog.begin())
            {
                m_pInputTextBox->SetText("");
                m_LastLogMove = 0;
            }
            else
            {
                m_InputLogPosition--;
                // Revive logged input and put cursor at the end of it
                m_pInputTextBox->SetText(*m_InputLogPosition);
                m_pInputTextBox->SetCursorPos(m_pInputTextBox->GetText().length());
                m_LastLogMove = -1;
            }
        }
    }

// TODO: This sometime.. bleh
    // Cut off the text in the text label at a reasonable height so it doesn't get really slow to draw
//    if (m_pConsoleText->GetTextHeight() > g_FrameMan.)
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this ConsoleMan's current graphical representation to a
//                  BITMAP of choice.

void ConsoleMan::Draw(BITMAP *pTargetBitmap)
{
    if (IsVisible() || m_ForceVisible)
    {
        AllegroScreen drawScreen(pTargetBitmap);
        m_pGUIController->Draw(&drawScreen);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearLog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all previous input.

void ConsoleMan::ClearLog()
{
    m_InputLog.clear();
    m_InputLogPosition = m_InputLog.begin();
    m_pConsoleText->SetText("");
}


} // namespace RTE
