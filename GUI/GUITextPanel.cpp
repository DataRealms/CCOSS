//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUITextPanel.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUITextPanel class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"
#include "GUITextPanel.h"

#ifdef _WIN32
#include "WinUtil.h"
#elif defined(__unix__)
#include "LinuxUtil.h"
#endif

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUITextPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUITextPanel object in
//                  system memory.

GUITextPanel::GUITextPanel(GUIManager *Manager)
: GUIPanel(Manager)
{
    m_Font = 0;
    m_CursorX = m_CursorY = 0;
    m_CursorIndex = 0;
    m_CursorColor = 0;
    m_CursorBlinkCount = 0;

    m_FontColor = 0;
    m_FontSelectColor = 0;
    m_StartIndex = 0;
    m_GotSelection = false;
    m_SelectedColorIndex = 0;
    m_Locked = false;
    m_WidthMargin = 3;
    m_HeightMargin = 0;
}


// TODO: Both constructors use a common clear function??
// Same with other panels


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUITextPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUITextPanel object in
//                  system memory.

GUITextPanel::GUITextPanel()
: GUIPanel()
{
    m_Font = 0;
    m_Text = "";
    m_CursorX = m_CursorY = 0;
    m_CursorIndex = 0;
    m_CursorColor = 0;
    m_CursorBlinkCount = 0;
    m_FontColor = 0;
    m_FontSelectColor = 0;
    m_StartIndex = 0;
    m_GotSelection = false;
    m_SelectedColorIndex = 0;
    m_Locked = false;
    m_WidthMargin = 3;
    m_HeightMargin = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Create the panel.

void GUITextPanel::Create(int X, int Y, int Width, int Height)
{
    m_X = X;
    m_Y = Y;
    m_Width = Width;
    m_Height = Height;
    
    assert(m_Manager);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeSkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the skin has been changed.

void GUITextPanel::ChangeSkin(GUISkin *Skin)
{
    // Load the font
    string Filename;
    Skin->GetValue("TextBox", "Font", &Filename);
    m_Font = Skin->GetFont(Filename);
    Skin->GetValue("TextBox", "FontColor", &m_FontColor);
    Skin->GetValue("TextBox", "FontShadow", &m_FontShadow);
    Skin->GetValue("TextBox", "FontKerning", &m_FontKerning);
    Skin->GetValue("TextBox", "FontSelectColor", &m_FontSelectColor);
    Skin->GetValue("TextBox", "SelectedColorIndex", &m_SelectedColorIndex);

    Skin->GetValue("TextBox", "WidthMargin", &m_WidthMargin);
    Skin->GetValue("TextBox", "HeightMargin", &m_HeightMargin);

    // Convert
    m_FontColor = Skin->ConvertColor(m_FontColor);
    m_FontSelectColor = Skin->ConvertColor(m_FontSelectColor);
    m_SelectedColorIndex = Skin->ConvertColor(m_SelectedColorIndex);

    // Pre-cache the font colors
    m_Font->CacheColor(m_FontColor);
    m_Font->CacheColor(m_FontSelectColor);

    // Get the cursor color
    Skin->GetValue("TextBox", "CursorColorIndex", &m_CursorColor);
    m_CursorColor = Skin->ConvertColor(m_CursorColor);

}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the panel

void GUITextPanel::Draw(GUIScreen *Screen)
{
    if (!m_Font)
        return;

    int FontHeight = m_Font->GetFontHeight();
    int wSpacer = m_WidthMargin;
    int hSpacer = m_HeightMargin;

    // Clamp the cursor
    m_CursorX = MAX(m_CursorX, 0);
    
    // Setup the clipping
    Screen->GetBitmap()->SetClipRect(GetRect());
    
    string Text = m_Text.substr(m_StartIndex);

    // Draw the text    
    m_Font->SetColor(m_FontColor);
    m_Font->SetKerning(m_FontKerning);
    m_Font->Draw(Screen->GetBitmap(), m_X+wSpacer, m_Y+hSpacer, Text, m_FontShadow);

    // Draw the right-justified extra text in the background
    m_Font->DrawAligned(Screen->GetBitmap(), m_X+m_Width-wSpacer, m_Y+hSpacer, m_RightText, GUIFont::Right, GUIFont::Top, m_Width, m_FontShadow);

    // Draw the selected text
    if (m_GotSelection && m_GotFocus && !m_Text.empty()) {
        // Draw selection mark
        Screen->GetBitmap()->DrawRectangle(m_X+wSpacer + m_SelectionX, m_Y + hSpacer + 2, m_SelectionWidth, FontHeight - 3, m_SelectedColorIndex, true);
        // Draw text with selection regions in different colour
        m_Font->SetColor(m_FontSelectColor);
        int Start = MIN(m_StartSelection, m_EndSelection);
        int End = MAX(m_StartSelection, m_EndSelection);
        
        // Selection
        if (m_StartIndex > Start)
            Start = m_StartIndex;
        m_Font->Draw(Screen->GetBitmap(), m_X+wSpacer+m_SelectionX, m_Y+hSpacer, Text.substr(Start-m_StartIndex, End-Start));
    }


    // If we have focus, draw the cursor with hacky blink
    if (m_GotFocus && (m_CursorBlinkCount++ % 30 > 15))
    {
        Screen->GetBitmap()->DrawRectangle(m_X + m_CursorX + 2, m_Y + hSpacer + m_CursorY + 2, 1, FontHeight - 3, m_CursorColor, true);
    }

    // Restore normal clipping
    Screen->GetBitmap()->SetClipRect(0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnKeyPress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when a key is pressed (OnDown & repeating).

void GUITextPanel::OnKeyPress(int KeyCode, int Modifier) {

	// TODO: Figure out what the "performance bitching" is.
	// Condition here to stop the compiler bitching about performance
	bool Shift = ((Modifier & MODI_SHIFT) != 0); 
	bool ModKey = ((Modifier & MODI_CTRL) != 0);

	// To convert to allegro's crazy scheme with their keyboard function returning the order of the letter when ctrl is pressed
	int asciiChar = ModKey ? KeyCode + 96 : KeyCode;

	if (m_Locked) {
		return;
	}

	// Backspace
	if (KeyCode == GUIInput::Key_Backspace) {
		if (m_GotSelection) {
			RemoveSelectionText();
		} else {
			if (m_CursorIndex > 0) {
                int newCursorIndex = ModKey ? GetStartOfPreviousCharacterGroup(m_Text, m_CursorIndex) : m_CursorIndex - 1;
                m_Text.erase(newCursorIndex, m_CursorIndex - newCursorIndex);
                m_CursorIndex = newCursorIndex;
			}
		}
		UpdateText();
		SendSignal(Changed, 0);
		return;
	}

	// Delete
	if (KeyCode == GUIInput::Key_Delete) {
		if (m_GotSelection) {
			RemoveSelectionText();
		} else {
			if (m_CursorIndex < m_Text.size()) {
                int nextCursorIndex = ModKey ? GetStartOfNextCharacterGroup(m_Text, m_CursorIndex) : m_CursorIndex + 1;
                m_Text.erase(m_CursorIndex, nextCursorIndex - m_CursorIndex);
			}
		}
		UpdateText();
		SendSignal(Changed, 0);
		return;
	}

	// Left Arrow
	if (KeyCode == GUIInput::Key_LeftArrow) {
		if (m_CursorIndex > 0) {
            int newCursorIndex = ModKey ? GetStartOfPreviousCharacterGroup(m_Text, m_CursorIndex) : m_CursorIndex - 1;
			if (Shift) {
				DoSelection(m_CursorIndex, newCursorIndex);
			} else {
				m_GotSelection = false;
			}
			m_CursorIndex = newCursorIndex;
			UpdateText();
		}
		return;
	}

	// Right Arrow
	if (KeyCode == GUIInput::Key_RightArrow) {
		if (m_CursorIndex < m_Text.size()) {
            int newCursorIndex = ModKey ? GetStartOfNextCharacterGroup(m_Text, m_CursorIndex) : m_CursorIndex + 1;
			if (Shift) {
				DoSelection(m_CursorIndex, newCursorIndex);
			} else {
				m_GotSelection = false;
			}
			m_CursorIndex = newCursorIndex;
			UpdateText();
		}
		return;
	}

	// Home
	if (KeyCode == GUIInput::Key_Home) {
		if (Shift) {
			DoSelection(m_CursorIndex, 0);
		} else {
			m_GotSelection = false;
		}
		m_CursorIndex = 0;
		UpdateText();
		return;
	}

	// End
	if (KeyCode == GUIInput::Key_End) {
		if (Shift) {
			DoSelection(m_CursorIndex, m_Text.size());
		} else {
			m_GotSelection = false;
		}
		m_CursorIndex = m_Text.size();
		UpdateText();
		return;
	}

	// ModKey-X (Cut)
	if (asciiChar == 'x' && ModKey) {
		if (m_GotSelection) {
#ifdef _WIN32
			WinUtil::SetClipboardText(GetSelectionText());
#elif defined(__unix__)
			LinuxUtil::SetClipboardText(GetSelectionText());
#endif
			RemoveSelectionText();
			SendSignal(Changed, 0);
		}
		return;
	}

	// ModKey-C (Copy)
	if (asciiChar == 'c' && ModKey) {
		if (m_GotSelection) {
#ifdef _WIN32
			WinUtil::SetClipboardText(GetSelectionText());
#elif defined(__unix__)
			LinuxUtil::SetClipboardText(GetSelectionText());
#endif
		}
		return;
	}

	// ModKey-V (Paste)
	if (asciiChar == 'v' && ModKey) {
		RemoveSelectionText();
		string Text = "";
#ifdef _WIN32
		WinUtil::GetClipboardText(&Text);
#elif defined(__unix__)
		LinuxUtil::GetClipboardText(&Text);
#endif
		m_Text.insert(m_CursorIndex, Text);
		m_CursorIndex += Text.size();
		UpdateText(true, true);
		SendSignal(Changed, 0);
		return;
	}

	// ModKey-A (Select All)
	if (asciiChar == 'a' && ModKey) {
		DoSelection(0, m_Text.size());
		UpdateText();
		return;
	}

	// Enter key
	if (KeyCode == '\n' || KeyCode == '\r') {
		SendSignal(Enter, 0);
		return;
	}

	// Add valid ASCII characters
	if (KeyCode >= 32 && KeyCode < 128 && KeyCode != 127) {
		RemoveSelectionText();
		char buf[2] = { static_cast<char>(KeyCode), '\0' };
		m_Text.insert(m_CursorIndex, buf);
		m_CursorIndex++;
		SendSignal(Changed, 0);
		UpdateText(true);
		return;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseDown
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes down on the panel

void GUITextPanel::OnMouseDown(int X, int Y, int Buttons, int Modifier)
{
    SendSignal(MouseDown, Buttons);

    if (m_Locked)
        return;

    // Give me focus
    SetFocus();
    CaptureMouse();

    if (!(Buttons & MOUSE_LEFT))
        return;

    int OldIndex = m_CursorIndex;

    // Set the cursor
    string Text = m_Text.substr(m_StartIndex, m_Text.size()-m_StartIndex);
    m_CursorIndex = m_Text.size();

    if (!(Modifier & MODI_SHIFT))
        m_GotSelection = false;

    // Go through each character until we to the mouse point
    int TX = m_X;
    for(int i=0; i<Text.size(); i++) {
        TX += m_Font->CalculateWidth(Text.at(i));
        if (TX > X) {
            m_CursorIndex = i+m_StartIndex;
            break;
        }
    }

    // Do a selection if holding the shift button
    if (Modifier & MODI_SHIFT)
        DoSelection(OldIndex, m_CursorIndex);

    // Update the text
    UpdateText(false, false);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseMove
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse moves (over the panel, or when captured).

void GUITextPanel::OnMouseMove(int X, int Y, int Buttons, int Modifier)
{
    if (!(Buttons & MOUSE_LEFT) || !IsCaptured())
        return;
    
    // Select from the mouse down point to where the mouse is currently
    string Text = m_Text.substr(m_StartIndex, m_Text.size() - m_StartIndex);
    int TX = m_X;
    for(int i=0; i<Text.size(); i++)
    {
        TX += m_Font->CalculateWidth(Text.at(i));
        if (TX >= X)
        {
            DoSelection(m_CursorIndex, i+m_StartIndex);
            m_CursorIndex = i+m_StartIndex;
            UpdateText(false, true);
            break;
        }
    }

    // Double check for the mouse at the end of the text
    if (X > TX)
    {
        DoSelection(m_CursorIndex, m_Text.size());
        m_CursorIndex = m_Text.size();
        UpdateText(false, true);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  OnMouseUp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes up on the panel

void GUITextPanel::OnMouseUp(int X, int Y, int Buttons, int Modifier)
{
    ReleaseMouse();

    SendSignal(Clicked, Buttons);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the cursor and start positions.

void GUITextPanel::UpdateText(bool Typing, bool DoIncrement)
{
    if (!m_Font)
        return;

    // Using increments of four characters to show a little extra of the text when
    // moving and typing
    // Only do this when NOT typing
    int Increment = 4;
    int Spacer = 2;

    if (Typing)
        Increment = 1;
    
    // Make sure the cursor is greater or equal to the start index
    if (m_CursorIndex <= m_StartIndex && DoIncrement)
        m_StartIndex = m_CursorIndex-Increment;

    // Clamp it
    m_StartIndex = MAX(m_StartIndex, 0);
    
    // If the cursor is greater than the length of text panel, adjust the start index
    string Sub = m_Text.substr(m_StartIndex,m_CursorIndex-m_StartIndex);
    while(m_Font->CalculateWidth(Sub) > m_Width-Spacer*2 && DoIncrement) {
        m_StartIndex += Increment;
        Sub = m_Text.substr(m_StartIndex,m_CursorIndex-m_StartIndex);
    }

    // Clamp it
    m_StartIndex = MIN(m_StartIndex, m_Text.size()-1);

    // Adjust the cursor position
    m_CursorX = m_Font->CalculateWidth(m_Text.substr(m_StartIndex, m_CursorIndex-m_StartIndex));

    // Update the selection
    if (m_GotSelection)
        DoSelection(m_StartSelection, m_EndSelection);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DoSelection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Update the selection.

void GUITextPanel::DoSelection(int Start, int End)
{
    // Start a selection
    if (!m_GotSelection) {
        m_GotSelection = true;
        m_StartSelection = Start;
        m_EndSelection = End;
    } else {
        // Update the selection
        m_EndSelection = End;
    }

    // Avoid zero char selections
    if (m_GotSelection && m_StartSelection == m_EndSelection)
    {
        m_GotSelection = false;
        return;
    }

    // Update the selection coords
    int StartSel = MIN(m_StartSelection, m_EndSelection);
    int EndSel = MAX(m_StartSelection, m_EndSelection);

    m_SelectionX = StartSel - m_StartIndex;
    m_SelectionX = MAX(m_SelectionX, 0);
    int temp = m_SelectionX;

    m_SelectionWidth = (EndSel - m_StartIndex) - m_SelectionX;

    m_SelectionX = m_Font->CalculateWidth(m_Text.substr(m_StartIndex, m_SelectionX));
    m_SelectionWidth = m_Font->CalculateWidth(m_Text.substr(m_StartIndex+temp, m_SelectionWidth));

    m_SelectionX = MAX(m_SelectionX, 0);
    m_SelectionWidth = MIN(m_SelectionWidth, m_Width);
}

int RTE::GUITextPanel::GetStartOfNextCharacterGroup(const std::string_view &stringToCheck, int currentIndex) const {
    auto isNormalCharacter = [](char charToCheck) { return (std::isalnum(charToCheck) || charToCheck == '_'); };
    auto isNormalCharacterOrSpace = [](char charToCheck) { return (std::isalnum(charToCheck) || charToCheck == '_' || std::isspace(charToCheck)); };
    auto isSpecialCharacterOrSpace = [](char charToCheck) { return !(std::isalnum(charToCheck) || charToCheck == '_'); };

    std::string_view::const_iterator currentIterator = stringToCheck.cbegin() + currentIndex;
    currentIterator = isNormalCharacter(*currentIterator) ?
        std::find_if(currentIterator, stringToCheck.cend(), isSpecialCharacterOrSpace) :
        std::find_if(currentIterator, stringToCheck.cend(), isNormalCharacterOrSpace);

    if (currentIterator != stringToCheck.cend() && std::isspace(*currentIterator)) {
        currentIterator = std::find_if_not(currentIterator, stringToCheck.cend(), isspace);
    }
    return std::distance(stringToCheck.cbegin(), currentIterator);
}

int RTE::GUITextPanel::GetStartOfPreviousCharacterGroup(const std::string_view &stringToCheck, int currentIndex) const {
    auto isNormalCharacter = [](char charToCheck) { return (std::isalnum(charToCheck) || charToCheck == '_'); };
    auto isNormalCharacterOrSpace = [](char charToCheck) { return (std::isalnum(charToCheck) || charToCheck == '_' || std::isspace(charToCheck)); };
    auto isSpecialCharacterOrSpace = [](char charToCheck) { return !(std::isalnum(charToCheck) || charToCheck == '_'); };

    std::string_view::reverse_iterator currentIterator = stringToCheck.crbegin() + (m_Text.size() - currentIndex);
    if (std::isspace(*(currentIterator))) {
        currentIterator = std::find_if_not(currentIterator, stringToCheck.crend(), isspace);
    }

    if (currentIterator != stringToCheck.crend()) {
        currentIterator = isNormalCharacter(*(currentIterator)) ?
            std::find_if(currentIterator, stringToCheck.crend(), isSpecialCharacterOrSpace) :
            std::find_if(currentIterator, stringToCheck.crend(), isNormalCharacterOrSpace);
    }
    return std::distance(stringToCheck.cbegin(), currentIterator.base());
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveSelectionTet
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes the characters in the selection.

void GUITextPanel::RemoveSelectionText(void)
{
    if (!m_GotSelection)
        return;

    int Start = MIN(m_StartSelection, m_EndSelection);
    int End = MAX(m_StartSelection, m_EndSelection);

    if (Start == End)
        return;

    m_Text.erase(Start, End-Start);

    m_CursorIndex = Start;
    UpdateText(false);

    m_GotSelection = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetCursorPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets where the cursor should be. This will clear any selection.
// Arguments:       The index of the new cursor position.

void GUITextPanel::SetCursorPos(int cursorPos)
{
    m_GotSelection = false;

    if (cursorPos <= 0)
        cursorPos = 0;
    if (cursorPos > m_Text.size())
        cursorPos = m_Text.size();

    m_CursorIndex = m_Text.size();

    UpdateText();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSelectionText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the selection text.

string GUITextPanel::GetSelectionText(void)
{
    if (!m_GotSelection)
        return "";

    int Start = MIN(m_StartSelection, m_EndSelection);
    int End = MAX(m_StartSelection, m_EndSelection);

    if (Start == End)
        return "";

    return m_Text.substr(Start, End-Start);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the text in the textpanel.

void GUITextPanel::SetText(const std::string Text)
{
    m_Text = Text;

    // Clear the selection
    ClearSelection();
    
    // Clear the cursor position
    m_CursorIndex = 0;
    m_StartIndex = 0;
    m_CursorX = 0;

    UpdateText(false, false);

    SendSignal(Changed, 0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetRightText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the extra text which appears right-justified in the textpanel.

void GUITextPanel::SetRightText(const std::string rightText)
{
    m_RightText = rightText;

//    UpdateText(false, false);

    SendSignal(Changed, 0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSelection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the start and end indexes of the selection text.

void GUITextPanel::SetSelection(int Start, int End)
{
    if (m_Locked)
        return;

    // Reset the selection
    m_GotSelection = false;

    DoSelection(Start, End);

    UpdateText(false, false);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSelectionStart
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the start index of the selection.

int GUITextPanel::GetSelectionStart(void)
{
    // No selection?
    if (!m_GotSelection)
        return -1;

    return m_StartSelection;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSelectionEnd
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the end index of the selection.

int GUITextPanel::GetSelectionEnd(void)
{
    // No selection?
    if (!m_GotSelection)
        return -1;

    return m_EndSelection;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearSelection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the selection. Does NOT remove the selection text though.

void GUITextPanel::ClearSelection(void)
{
    m_GotSelection = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLocked
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the locked state on the textbox.

void GUITextPanel::SetLocked(bool Locked)
{
    m_Locked = Locked;

    // Clear the selection if we are now locked
    if (m_Locked)
        ClearSelection();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLocked
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the locked state on the textbox.

bool GUITextPanel::GetLocked(void)
{
    return m_Locked;
}
