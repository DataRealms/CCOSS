#include "GUI.h"
#include "GUITextPanel.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUITextPanel::GUITextPanel(GUIManager *Manager) : GUIPanel(Manager) {
	m_Font = nullptr;
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

	m_MaxTextLength = 0;
	m_NumericOnly = false;
	m_MaxNumericValue = 0;
}

// TODO: Both constructors use a common clear function?? Same with other panels

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUITextPanel::GUITextPanel() : GUIPanel() {
	m_Font = nullptr;
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

	m_MaxTextLength = 0;
	m_NumericOnly = false;
	m_MaxNumericValue = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextPanel::Create(int X, int Y, int Width, int Height) {
	m_X = X;
	m_Y = Y;
	m_Width = Width;
	m_Height = Height;

	assert(m_Manager);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextPanel::ChangeSkin(GUISkin *Skin) {
	// Load the font
	std::string Filename;
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextPanel::Draw(GUIScreen *Screen) {
	if (!m_Font)
		return;

	int FontHeight = m_Font->GetFontHeight();
	int wSpacer = m_WidthMargin;
	int hSpacer = m_HeightMargin;

	// Clamp the cursor
	m_CursorX = std::max(m_CursorX, 0);

	// Setup the clipping
	Screen->GetBitmap()->SetClipRect(GetRect());

	std::string Text = m_Text.substr(m_StartIndex);

	// Draw the text
	m_Font->SetColor(m_FontColor);
	m_Font->SetKerning(m_FontKerning);
	m_Font->Draw(Screen->GetBitmap(), m_X + wSpacer, m_Y + hSpacer, Text, m_FontShadow);

	// Draw the right-justified extra text in the background
	m_Font->DrawAligned(Screen->GetBitmap(), m_X + m_Width - wSpacer, m_Y + hSpacer, m_RightText, GUIFont::Right, GUIFont::Top, m_Width, m_FontShadow);

	// Draw the selected text
	if (m_GotSelection && m_GotFocus && !m_Text.empty()) {
		// Draw selection mark
		Screen->GetBitmap()->DrawRectangle(m_X + wSpacer + m_SelectionX, m_Y + hSpacer + 2, m_SelectionWidth, FontHeight - 3, m_SelectedColorIndex, true);
		// Draw text with selection regions in different color
		m_Font->SetColor(m_FontSelectColor);
		int Start = std::min(m_StartSelection, m_EndSelection);
		int End = std::max(m_StartSelection, m_EndSelection);

		// Selection
		if (m_StartIndex > Start) { Start = m_StartIndex; }
		m_Font->Draw(Screen->GetBitmap(), m_X + wSpacer + m_SelectionX, m_Y + hSpacer, Text.substr(Start - m_StartIndex, End - Start));
	}


	// If we have focus, draw the cursor with hacky blink
	if (m_GotFocus && (m_CursorBlinkCount++ % 30 > 15)) { Screen->GetBitmap()->DrawRectangle(m_X + m_CursorX + 2, m_Y + hSpacer + m_CursorY + 2, 1, FontHeight - 3, m_CursorColor, true); }

	// Restore normal clipping
	Screen->GetBitmap()->SetClipRect(nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
		int newCursorIndex = ModKey ? GetStartOfNextCharacterGroup(m_Text, m_CursorIndex) : m_CursorIndex + 1;
		if (m_CursorIndex < m_Text.size()) {
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
			GUIUtil::SetClipboardText(GetSelectionText());
			RemoveSelectionText();
			SendSignal(Changed, 0);
		}
		return;
	}

	// ModKey-C (Copy)
	if (asciiChar == 'c' && ModKey) {
		if (m_GotSelection) { GUIUtil::SetClipboardText(GetSelectionText()); }
		return;
	}

	// ModKey-V (Paste)
	if (asciiChar == 'v' && ModKey) {
		RemoveSelectionText();
		std::string Text = "";
		GUIUtil::GetClipboardText(&Text);
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

	int minValidKeyCode = 32;
	int maxValidKeyCode = 126;
	if (m_NumericOnly) {
		minValidKeyCode = 48;
		maxValidKeyCode = 57;
	}
	// Add valid ASCII characters
	if (KeyCode >= minValidKeyCode && KeyCode <= maxValidKeyCode) {
		RemoveSelectionText();
		char buf[2] = { static_cast<char>(KeyCode), '\0' };
		if (m_MaxTextLength > 0 && m_Text.length() >= m_MaxTextLength) {
			return;
		}
		m_Text.insert(m_CursorIndex, buf);
		m_CursorIndex++;

		if (m_NumericOnly && m_MaxNumericValue > 0 && std::stoi(m_Text) > m_MaxNumericValue) { m_Text = std::to_string(m_MaxNumericValue); }

		SendSignal(Changed, 0);
		UpdateText(true);
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextPanel::OnMouseDown(int X, int Y, int Buttons, int Modifier) {
	SendSignal(MouseDown, Buttons);

	if (m_Locked) {
		return;
	}

	// Give me focus
	SetFocus();
	CaptureMouse();

	if (!(Buttons & MOUSE_LEFT)) {
		return;
	}

	int OldIndex = m_CursorIndex;

	// Set the cursor
	std::string Text = m_Text.substr(m_StartIndex, m_Text.size() - m_StartIndex);
	m_CursorIndex = m_Text.size();

	if (!(Modifier & MODI_SHIFT)) { m_GotSelection = false; }

	// Go through each character until we to the mouse point
	int TX = m_X;
	for (int i = 0; i < Text.size(); i++) {
		TX += m_Font->CalculateWidth(Text.at(i));
		if (TX > X) {
			m_CursorIndex = i + m_StartIndex;
			break;
		}
	}

	// Do a selection if holding the shift button
	if (Modifier & MODI_SHIFT)
		DoSelection(OldIndex, m_CursorIndex);

	// Update the text
	UpdateText(false, false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextPanel::OnMouseMove(int X, int Y, int Buttons, int Modifier) {
	if (!(Buttons & MOUSE_LEFT) || !IsCaptured()) {
		return;
	}

	// Select from the mouse down point to where the mouse is currently
	std::string Text = m_Text.substr(m_StartIndex, m_Text.size() - m_StartIndex);
	int TX = m_X;
	for (int i = 0; i < Text.size(); i++) {
		TX += m_Font->CalculateWidth(Text.at(i));
		if (TX >= X) {
			DoSelection(m_CursorIndex, i + m_StartIndex);
			m_CursorIndex = i + m_StartIndex;
			UpdateText(false, true);
			break;
		}
	}

	// Double check for the mouse at the end of the text
	if (X > TX) {
		DoSelection(m_CursorIndex, m_Text.size());
		m_CursorIndex = m_Text.size();
		UpdateText(false, true);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextPanel::OnMouseUp(int X, int Y, int Buttons, int Modifier) {
	ReleaseMouse();
	SendSignal(Clicked, Buttons);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextPanel::UpdateText(bool Typing, bool DoIncrement) {
	if (!m_Font) {
		return;
	}

	// Using increments of four characters to show a little extra of the text when
	// moving and typing
	// Only do this when NOT typing
	int Increment = 4;
	int Spacer = 2;

	if (Typing) { Increment = 1; }

	// Make sure the cursor is greater or equal to the start index
	if (m_CursorIndex <= m_StartIndex && DoIncrement) { m_StartIndex = m_CursorIndex - Increment; }

	// Clamp it
	m_StartIndex = std::max(m_StartIndex, 0);

	// If the cursor is greater than the length of text panel, adjust the start index
	std::string Sub = m_Text.substr(m_StartIndex, m_CursorIndex - m_StartIndex);
	while (m_Font->CalculateWidth(Sub) > m_Width - Spacer * 2 && DoIncrement) {
		m_StartIndex += Increment;
		Sub = m_Text.substr(m_StartIndex, m_CursorIndex - m_StartIndex);
	}

	// Clamp it
	m_StartIndex = std::max(0, std::min(m_StartIndex, static_cast<int>(m_Text.size() - 1)));

	// Adjust the cursor position
	m_CursorX = m_Font->CalculateWidth(m_Text.substr(m_StartIndex, m_CursorIndex - m_StartIndex));

	// Update the selection
	if (m_GotSelection) { DoSelection(m_StartSelection, m_EndSelection); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextPanel::DoSelection(int Start, int End) {
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
	if (m_GotSelection && m_StartSelection == m_EndSelection) {
		m_GotSelection = false;
		return;
	}

	// Update the selection coordinates
	int StartSel = std::min(m_StartSelection, m_EndSelection);
	int EndSel = std::max(m_StartSelection, m_EndSelection);

	m_SelectionX = StartSel - m_StartIndex;
	m_SelectionX = std::max(m_SelectionX, 0);
	int temp = m_SelectionX;

	m_SelectionWidth = (EndSel - m_StartIndex) - m_SelectionX;

	m_SelectionX = m_Font->CalculateWidth(m_Text.substr(m_StartIndex, m_SelectionX));
	m_SelectionWidth = m_Font->CalculateWidth(m_Text.substr(m_StartIndex + temp, m_SelectionWidth));

	m_SelectionX = std::max(m_SelectionX, 0);
	m_SelectionWidth = std::min(m_SelectionWidth, m_Width);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RTE::GUITextPanel::GetStartOfNextCharacterGroup(const std::string_view &stringToCheck, int currentIndex) const {
	auto isNormalCharacter = [](char charToCheck) {
		return (std::isalnum(charToCheck) || charToCheck == '_');
	};
	auto isNormalCharacterOrSpace = [](char charToCheck) {
		return (std::isalnum(charToCheck) || charToCheck == '_' || std::isspace(charToCheck));
	};
	auto isSpecialCharacterOrSpace = [](char charToCheck) {
		return !(std::isalnum(charToCheck) || charToCheck == '_');
	};

	std::string_view::const_iterator currentIterator = stringToCheck.cbegin() + currentIndex;
	currentIterator = isNormalCharacter(*currentIterator) ?
		std::find_if(currentIterator, stringToCheck.cend(), isSpecialCharacterOrSpace) :
		std::find_if(currentIterator, stringToCheck.cend(), isNormalCharacterOrSpace);

	if (currentIterator != stringToCheck.cend() && std::isspace(*currentIterator)) { currentIterator = std::find_if_not(currentIterator, stringToCheck.cend(), isspace); }
	return std::distance(stringToCheck.cbegin(), currentIterator);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RTE::GUITextPanel::GetStartOfPreviousCharacterGroup(const std::string_view &stringToCheck, int currentIndex) const {
	auto isNormalCharacter = [](char charToCheck) {
		return (std::isalnum(charToCheck) || charToCheck == '_');
	};
	auto isNormalCharacterOrSpace = [](char charToCheck) {
		return (std::isalnum(charToCheck) || charToCheck == '_' || std::isspace(charToCheck));
	};
	auto isSpecialCharacterOrSpace = [](char charToCheck) {
		return !(std::isalnum(charToCheck) || charToCheck == '_');
	};

	std::string_view::reverse_iterator currentIterator = stringToCheck.crbegin() + (m_Text.size() - currentIndex);
	if (std::isspace(*currentIterator)) { currentIterator = std::find_if_not(currentIterator, stringToCheck.crend(), isspace); }

	if (currentIterator != stringToCheck.crend()) {
		currentIterator = isNormalCharacter(*currentIterator) ?
			std::find_if(currentIterator, stringToCheck.crend(), isSpecialCharacterOrSpace) :
			std::find_if(currentIterator, stringToCheck.crend(), isNormalCharacterOrSpace);
	}
	return std::distance(stringToCheck.cbegin(), currentIterator.base());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextPanel::RemoveSelectionText() {
	if (!m_GotSelection) {
		return;
	}

	int Start = std::min(m_StartSelection, m_EndSelection);
	int End = std::max(m_StartSelection, m_EndSelection);

	if (Start == End) {
		return;
	}

	m_Text.erase(Start, End - Start);

	m_CursorIndex = Start;
	UpdateText(false);

	m_GotSelection = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextPanel::SetCursorPos(int cursorPos) {
	m_GotSelection = false;

	if (cursorPos <= 0) { cursorPos = 0; }
	if (cursorPos > m_Text.size()) { cursorPos = m_Text.size(); }

	m_CursorIndex = m_Text.size();

	UpdateText();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GUITextPanel::GetSelectionText() const {
	if (!m_GotSelection) {
		return "";
	}
	int Start = std::min(m_StartSelection, m_EndSelection);
	int End = std::max(m_StartSelection, m_EndSelection);

	if (Start == End) {
		return "";
	}
	return m_Text.substr(Start, End - Start);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextPanel::SetText(const std::string &Text) {
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextPanel::SetRightText(const std::string &rightText) {
	m_RightText = rightText;
	SendSignal(Changed, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextPanel::SetSelection(int Start, int End) {
	if (m_Locked) {
		return;
	}
	// Reset the selection
	m_GotSelection = false;

	DoSelection(Start, End);

	UpdateText(false, false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUITextPanel::GetSelectionStart() const {
	// No selection?
	if (!m_GotSelection) {
		return -1;
	}

	return m_StartSelection;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUITextPanel::GetSelectionEnd() const {
	if (!m_GotSelection) {
		return -1;
	}
	return m_EndSelection;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextPanel::ClearSelection() {
	m_GotSelection = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextPanel::SetLocked(bool Locked) {
	m_Locked = Locked;

	// Clear the selection if we are now locked
	if (m_Locked) { ClearSelection(); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUITextPanel::GetLocked() const {
	return m_Locked;
}