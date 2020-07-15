#include "ConsoleMan.h"
#include "System.h"

#include "LuaMan.h"
#include "UInputMan.h"
#include "FrameMan.h"

#include "GUI/GUI.h"
#include "GUI/AllegroBitmap.h"
#include "GUI/AllegroScreen.h"
#include "GUI/AllegroInput.h"
#include "GUI/GUICollectionBox.h"
#include "GUI/GUITextBox.h"
#include "GUI/GUILabel.h"

namespace RTE {

	const std::string ConsoleMan::c_ClassName = "ConsoleMan";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::Clear() {
		m_ConsoleState = DISABLED;
		m_ReadOnly = false;
		m_ConsoleScreenRatio = 0.3F;
		m_GUIScreen = 0;
		m_GUIInput = 0;
		m_GUIControlManager = 0;
		m_ParentBox = 0;
		m_ConsoleText = 0;
		m_InputTextBox = 0;
		m_InputLog.clear();
		m_InputLogPosition = m_InputLog.begin();
		m_LastInputString.clear();
		m_LastLogMove = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ConsoleMan::Create() {
		if (!m_GUIScreen) { m_GUIScreen = new AllegroScreen(g_FrameMan.GetBackBuffer32()); }
		if (!m_GUIInput) { m_GUIInput = new AllegroInput(-1); }
		if (!m_GUIControlManager) { m_GUIControlManager = new GUIControlManager(); }

		if (!m_GUIControlManager->Create(m_GUIScreen, m_GUIInput, "Base.rte/GUIs/Skins/MainMenu", "ConsoleSkin.ini")) {
			RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/MainMenu/ConsoleSkin.ini");
		}

		m_GUIControlManager->Load("Base.rte/GUIs/ConsoleGUI.ini");
		m_GUIControlManager->EnableMouse(false);

		// Stretch the invisible root box to fill the screen
		dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("base"))->SetSize(g_FrameMan.GetResX(), g_FrameMan.GetResY());

		if (!m_ParentBox) {
			m_ParentBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("ConsoleGUIBox"));
			m_ParentBox->SetDrawType(GUICollectionBox::Color);
		}
		m_ConsoleText = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("ConsoleLabel"));
		m_InputTextBox = dynamic_cast<GUITextBox *>(m_GUIControlManager->GetControl("InputTB"));

		SetConsoleScreenSize(m_ConsoleScreenRatio);

		m_ParentBox->SetPositionAbs(0, -m_ParentBox->GetHeight());
		m_ParentBox->SetEnabled(false);
		m_ParentBox->SetVisible(false);

		m_ConsoleText->SetText("- RTE Lua Console -\nSee the Data Realms Wiki for commands: http://www.datarealms.com/wiki/\nPress F1 for a list of helpful shortcuts\n-------------------------------------");

		m_InputLogPosition = m_InputLog.begin();
		m_LastLogMove = 0;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::Destroy() {
		SaveAllText("LogConsole.txt");

		delete m_GUIControlManager;
		delete m_GUIInput;
		delete m_GUIScreen;

		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::SetEnabled(bool enable) {
		if (enable && m_ConsoleState != ENABLED && m_ConsoleState != ENABLING) {
			m_ConsoleState = ENABLING;
			g_GUISound.EnterMenuSound()->Play();
		} else if (!enable && m_ConsoleState != DISABLED && m_ConsoleState != DISABLING) {
			m_ConsoleState = DISABLING;
			g_GUISound.ExitMenuSound()->Play();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void ConsoleMan::SetReadOnly() {
		if (!m_ReadOnly) {
			// Save the current input string before changing to the read-only notice so we can restore it when switching back
			m_LastInputString = m_InputTextBox->GetText();
			m_InputTextBox->SetText("- CONSOLE IN READ-ONLY MODE! -");
			m_ReadOnly = true;
			SetEnabled(true);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::SetConsoleScreenSize(float screenRatio) {
		m_ConsoleScreenRatio = Limit(screenRatio, 1.0F, 0.1F);

		if (!m_ParentBox || !m_ConsoleText || !m_InputTextBox) {
			return;
		}

		m_ParentBox->SetSize(g_FrameMan.GetResX(), g_FrameMan.GetResY() * m_ConsoleScreenRatio);
		m_ConsoleText->SetSize(m_ParentBox->GetWidth() - 4, m_ParentBox->GetHeight() - m_InputTextBox->GetHeight() - 2);
		m_InputTextBox->SetPositionRel(m_InputTextBox->GetRelXPos(), m_ConsoleText->GetHeight());
		m_InputTextBox->Resize(m_ParentBox->GetWidth() - 3, m_InputTextBox->GetHeight());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::SaveInputLog(std::string filePath) {
		Writer logWriter(filePath.c_str());
		if (logWriter.WriterOK()) {
			for (std::deque<std::string>::reverse_iterator logItr = m_InputLog.rbegin(); logItr != m_InputLog.rend(); ++logItr) {
				logWriter << *logItr;
				// Add semicolon so the line input becomes a statement
				if (!logItr->empty() && (*logItr)[logItr->length() - 1] != ';') { logWriter << ";"; }
				logWriter << "\n";
			}
			PrintString("SYSTEM: Console input log saved to " + filePath);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::SaveAllText(std::string filePath) {
		Writer logWriter(filePath.c_str());
		if (logWriter.WriterOK()) {
			logWriter << m_ConsoleText->GetText();
			PrintString("SYSTEM: Entire console contents saved to " + filePath);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::ClearLog() {
		m_InputLog.clear();
		m_InputLogPosition = m_InputLog.begin();
		m_ConsoleText->SetText("");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::PrintString(std::string stringToPrint) {
		m_ConsoleText->SetText(m_ConsoleText->GetText() + "\n" + stringToPrint);
		if (g_System.GetLogToCLI()) { g_System.PrintToCLI(stringToPrint); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::ShowShortcuts() {
		if (!IsEnabled()) { SetEnabled(); }

		PrintString("--- SHORTCUTS ---");
		PrintString("CTRL + ~ - Console in read-only mode without input capture");
		PrintString("CTRL + DOWN / UP - Increase/decrease console size (Only while console is open)");
		PrintString("CTRL + S / PrintScrn - Make a screenshot");
		PrintString("CTRL + W - Make a screenshot of the entire level");
		PrintString("ALT  + W - Make a miniature preview image of the entire level");
		PrintString("CTRL + P - Show performance stats");
		PrintString("CTRL + R - Reset activity");
		PrintString("CTRL + M - Switch display mode: Draw -> Material -> MO");
		PrintString("CTRL + O - Toggle one sim update per frame");
		PrintString("----------------");
		PrintString("F2 - Reload all Lua scripts");
		PrintString("F3 - Save console log");
		PrintString("F4 - Save console user input log");
		PrintString("F5 - Clear console log ");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::Update() {
		if (g_UInputMan.FlagCtrlState() && g_UInputMan.KeyPressed(KEY_TILDE)) {
			SetReadOnly();		
		}

		if (!g_UInputMan.FlagShiftState() && (!g_UInputMan.FlagCtrlState() && g_UInputMan.KeyPressed(KEY_TILDE))) {
			if (IsEnabled()) {
				if (!m_ReadOnly) {
					m_InputTextBox->SetEnabled(false);
					m_GUIControlManager->GetManager()->SetFocus(0);
					// Save any text being worked on in the input, as the box keeps getting junk added to it
					m_LastInputString = m_InputTextBox->GetText();
					SetEnabled(false);
				} else {
					// Restore any text being worked on in the input and set the focus and cursor position of the input line
					m_InputTextBox->SetText(m_LastInputString);
					m_InputTextBox->SetCursorPos(m_InputTextBox->GetText().length());
					m_ReadOnly = false;
				}
			} else {
				m_InputTextBox->SetText(m_LastInputString);
				m_InputTextBox->SetCursorPos(m_InputTextBox->GetText().length());
				SetEnabled(true);
			}
		}

		if (m_ConsoleState != ENABLED && m_ConsoleState != DISABLED) { ConsoleOpenClose(); }

		if (m_ConsoleState != ENABLED) {
			return;
		}

		m_GUIControlManager->Update();

		if (g_UInputMan.FlagCtrlState() && g_UInputMan.KeyPressed(KEY_DOWN)) {
			SetConsoleScreenSize(m_ConsoleScreenRatio + 0.05F);
		} else if (g_UInputMan.FlagCtrlState() && g_UInputMan.KeyPressed(KEY_UP)) {
			SetConsoleScreenSize(m_ConsoleScreenRatio - 0.05F);
		}

		if (!m_ReadOnly) {
			m_InputTextBox->SetEnabled(true);
			m_InputTextBox->SetFocus();

			if (!m_InputLog.empty() && !g_UInputMan.FlagCtrlState()) {
				if (g_UInputMan.KeyPressed(KEY_UP)) {
					LoadLoggedInput(false);
				} else if (g_UInputMan.KeyPressed(KEY_DOWN)) {
					LoadLoggedInput(true);
				}
			}
			RemoveGraveAccents();
		} else {
			m_InputTextBox->SetEnabled(false);
			m_GUIControlManager->GetManager()->SetFocus(0);
			return;
		}	

		// Execute string when Enter is pressed, or execute immediately if a newline character is found, meaning multiple strings were pasted in.
		if ((g_UInputMan.KeyPressed(KEY_ENTER) || g_UInputMan.KeyPressed(KEY_ENTER_PAD)) || (m_InputTextBox->GetText().find_last_of('\n') != std::string::npos)) { FeedString(); }

		// TODO: Get this working and see if it actually makes any difference.
		/*
		// Cut off the text in the text label at a reasonable height so it doesn't get really slow to draw
		if (m_ConsoleText->GetTextHeight() > g_FrameMan.GetResY() {
		}
		*/
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::ConsoleOpenClose() {
		float travelCompletionDistance = 0;

		if (m_ConsoleState == ENABLING) {
			m_ParentBox->SetEnabled(true);
			m_ParentBox->SetVisible(true);

			travelCompletionDistance = std::floorf(static_cast<float>(m_ParentBox->GetYPos()) * 0.5F);
			m_ParentBox->SetPositionAbs(0, m_ParentBox->GetYPos() - static_cast<int>(travelCompletionDistance));

			if (m_ParentBox->GetYPos() >= 0) { m_ConsoleState = ENABLED; }
		} else if (m_ConsoleState == DISABLING) {
			travelCompletionDistance = std::ceilf((static_cast<float>(m_ParentBox->GetHeight()) + static_cast<float>(m_ParentBox->GetYPos())) * 0.5F);
			m_ParentBox->SetPositionAbs(0, m_ParentBox->GetYPos() - static_cast<int>(travelCompletionDistance));

			if (m_ParentBox->GetYPos() <= -m_ParentBox->GetHeight()) {
				m_ParentBox->SetEnabled(false);
				m_ParentBox->SetVisible(false);
				m_InputTextBox->SetEnabled(false);
				m_GUIControlManager->GetManager()->SetFocus(0);
				m_ConsoleState = DISABLED;
			}
		}

		// If supposed to be enabled or disabled but appears to be the opposite, enable or disable accordingly.
		if (m_ConsoleState == ENABLED && (m_ParentBox->GetYPos() < 0)) {
			m_ConsoleState = ENABLING;
		} else if (m_ConsoleState == DISABLED && (m_ParentBox->GetYPos() > -m_ParentBox->GetHeight())) {
			m_ConsoleState = DISABLING;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::FeedString() {
		char strLine[1024];
		std::stringstream inputStream(m_InputTextBox->GetText());

		while (!inputStream.fail()) {
			inputStream.getline(strLine, 1024, '\n');
			std::string line = strLine;

			if (line != "\r" && !line.empty()) {
				g_LuaMan.ClearErrors();
				m_ConsoleText->SetText(m_ConsoleText->GetText() + "\n" + line);
				g_LuaMan.RunScriptString(line, false);

				if (g_LuaMan.ErrorExists()) { m_ConsoleText->SetText(m_ConsoleText->GetText() + "\n" + "ERROR: " + g_LuaMan.GetLastError()); }
				if (m_InputLog.empty() || m_InputLog.front().compare(line) != 0) { m_InputLog.push_front(line); }

				m_InputLogPosition = m_InputLog.begin();
				m_LastLogMove = 0;
			}
		}
		m_InputTextBox->SetText("");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::LoadLoggedInput(bool nextEntry) {
		switch (nextEntry) {
			case true:
				// See if we should decrement doubly because the last move was in the opposite direction
				if (m_LastLogMove > 0 && m_InputLogPosition != m_InputLog.begin()) { m_InputLogPosition--; }

				if (m_InputLogPosition == m_InputLog.begin()) {
					m_InputTextBox->SetText("");
					m_LastLogMove = 0;
				} else {
					m_InputLogPosition--;
					m_InputTextBox->SetText(*m_InputLogPosition);
					m_InputTextBox->SetCursorPos(m_InputTextBox->GetText().length());
					m_LastLogMove = -1;
				}
				break;
			case false:
				// See if we should increment doubly because the last move was in the opposite direction
				if (m_LastLogMove < 0 && m_InputLogPosition != m_InputLog.end() - 1) { m_InputLogPosition++; }

				m_InputTextBox->SetText(*m_InputLogPosition);
				m_InputTextBox->SetCursorPos(m_InputTextBox->GetText().length());
				m_InputLogPosition++;
				m_LastLogMove = 1;

				// Avoid falling off the end
				if (m_InputLogPosition == m_InputLog.end()) {
					m_InputLogPosition--;
					m_LastLogMove = 0;
				}
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::RemoveGraveAccents() {					
		std::string textBoxString = m_InputTextBox->GetText();
		if (std::find(textBoxString.begin(), textBoxString.end(), '`') != textBoxString.end()) {
			textBoxString.erase(std::remove(textBoxString.begin(), textBoxString.end(), '`'), textBoxString.end());
			m_InputTextBox->SetText(textBoxString);
			m_InputTextBox->SetCursorPos(textBoxString.length());
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::Draw(BITMAP *targetBitmap) {
		if (m_ConsoleState != DISABLED) {
			AllegroScreen drawScreen(targetBitmap);
			m_GUIControlManager->Draw(&drawScreen);
		}
	}
}