#include "ConsoleMan.h"

#include "LuaMan.h"
#include "UInputMan.h"
#include "FrameMan.h"

#include "GUI.h"
#include "AllegroBitmap.h"
#include "AllegroScreen.h"
#include "AllegroInput.h"
#include "GUICollectionBox.h"
#include "GUITextBox.h"
#include "GUILabel.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::Clear() {
		m_ConsoleState = ConsoleState::Disabled;
		m_ReadOnly = false;
		m_ConsoleScreenRatio = 0.3F;
		m_GUIScreen = nullptr;
		m_GUIInput = nullptr;
		m_GUIControlManager = nullptr;
		m_ParentBox = nullptr;
		m_ConsoleText = nullptr;
		m_InputTextBox = nullptr;
		m_ConsoleTextMaxNumLines = 10;
		m_OutputLog.clear();
		m_InputLog.clear();
		m_InputLogPosition = m_InputLog.begin();
		m_LastInputString.clear();
		m_LastLogMove = 0;

		m_ConsoleUseMonospaceFont = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ConsoleMan::Initialize() {
		if (!m_GUIScreen) { m_GUIScreen = new AllegroScreen(g_FrameMan.GetBackBuffer32()); }
		if (!m_GUIInput) { m_GUIInput = new AllegroInput(-1); }
		if (!m_GUIControlManager) { m_GUIControlManager = new GUIControlManager(); }

		if (!m_GUIControlManager->Create(m_GUIScreen, m_GUIInput, "Base.rte/GUIs/Skins/Menus", m_ConsoleUseMonospaceFont ? "ConsoleMonospaceSkin.ini" : "ConsoleSkin.ini")) {
			RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Menus/ConsoleSkin.ini");
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

		if (!g_FrameMan.ResolutionChanged()) { m_OutputLog.emplace_back("- RTE Lua Console -\nSee the Data Realms Wiki for commands: http://www.datarealms.com/wiki/\nPress F1 for a list of helpful shortcuts\n-------------------------------------"); }

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::Destroy() {
		if (!g_FrameMan.ResolutionChanged()) { SaveAllText("LogConsole.txt"); }

		delete m_GUIControlManager;
		delete m_GUIInput;
		delete m_GUIScreen;

		if (g_FrameMan.ResolutionChanged()) {
			m_GUIScreen = nullptr;
			m_GUIInput = nullptr;
			m_GUIControlManager = nullptr;
			m_ParentBox = nullptr;
		} else {
			Clear();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::SetEnabled(bool enable) {
		if (enable && m_ConsoleState != ConsoleState::Enabled && m_ConsoleState != ConsoleState::Enabling) {
			m_ConsoleState = ConsoleState::Enabling;
			g_GUISound.EnterMenuSound()->Play();
		} else if (!enable && m_ConsoleState != ConsoleState::Disabled && m_ConsoleState != ConsoleState::Disabling) {
			m_ConsoleState = ConsoleState::Disabling;
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
		m_ConsoleTextMaxNumLines = 5 + (m_ConsoleText->GetHeight() / m_GUIControlManager->GetSkin()->GetFont("FontSmall.png")->GetFontHeight());
		m_InputTextBox->SetPositionRel(m_InputTextBox->GetRelXPos(), m_ConsoleText->GetHeight());
		m_InputTextBox->Resize(m_ParentBox->GetWidth() - 3, m_InputTextBox->GetHeight());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::SetConsoleUseMonospaceFont(bool useFont) {
		m_ConsoleUseMonospaceFont = useFont;
		if (m_GUIControlManager) { m_GUIControlManager->ChangeSkin("Base.rte/GUIs/Skins/Menus", m_ConsoleUseMonospaceFont ? "ConsoleMonospaceSkin.ini" : "ConsoleSkin.ini"); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::AddLoadWarningLogEntry(const std::string &pathToLog, const std::string &readerPosition, const std::string &altFileExtension) {
		const std::string pathAndAccessLocation = "\"" + pathToLog + "\" referenced " + readerPosition + ". ";
		std::string newEntry = pathAndAccessLocation + (!altFileExtension.empty() ? "Found and loaded a file with \"" + altFileExtension + "\" extension." : "The file was not loaded.");
		std::transform(newEntry.begin(), newEntry.end(), newEntry.begin(), ::tolower);
		if (m_LoadWarningLog.find(newEntry) == m_LoadWarningLog.end()) { m_LoadWarningLog.insert(newEntry); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::SaveLoadWarningLog(const std::string &filePath) {
		Writer logWriter(filePath.c_str());
		if (logWriter.WriterOK()) {
			logWriter << "// Warnings produced during loading:";
			logWriter.NewLine(false);
			for (const std::string &logEntry : m_LoadWarningLog) {
				logWriter.NewLineString(logEntry, false);
			}
			PrintString("SYSTEM: Loading warning log saved to " + filePath);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::SaveInputLog(const std::string &filePath) {
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

	void ConsoleMan::SaveAllText(const std::string &filePath) {
		Writer logWriter(filePath.c_str());
		if (logWriter.WriterOK()) {
			for (const std::string &loggedString : m_OutputLog) {
				logWriter << loggedString;
			}
			PrintString("SYSTEM: Entire console contents saved to " + filePath);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::ClearLog() {
		m_InputLog.clear();
		m_InputLogPosition = m_InputLog.begin();
		m_OutputLog.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::PrintString(const std::string &stringToPrint) {
		m_OutputLog.emplace_back("\n" + stringToPrint);
		if (System::IsLoggingToCLI()) { System::PrintToCLI(stringToPrint); }
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
					m_GUIControlManager->GetManager()->SetFocus(nullptr);
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

		if (m_ConsoleState != ConsoleState::Enabled && m_ConsoleState != ConsoleState::Disabled) { ConsoleOpenClose(); }

		std::stringstream consoleText;
		for (std::deque<std::string>::iterator logIterator = (m_OutputLog.size() < m_ConsoleTextMaxNumLines) ? m_OutputLog.begin() : m_OutputLog.end() - m_ConsoleTextMaxNumLines; logIterator != m_OutputLog.end(); ++logIterator) {
			consoleText << *logIterator;
		}
		m_ConsoleText->SetText(consoleText.str());

		if (m_ConsoleState != ConsoleState::Enabled) {
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
			m_GUIControlManager->GetManager()->SetFocus(nullptr);
			return;
		}

		// Execute string when Enter is pressed, or execute immediately if a newline character is found, meaning multiple strings were pasted in.
		if ((g_UInputMan.KeyPressed(KEY_ENTER) || g_UInputMan.KeyPressed(KEY_ENTER_PAD)) || (m_InputTextBox->GetText().find_last_of('\n') != std::string::npos)) {
			FeedString(m_InputTextBox->GetText().empty() ? true : false);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::ConsoleOpenClose() {
		float travelCompletionDistance;

		if (m_ConsoleState == ConsoleState::Enabling) {
			m_ParentBox->SetEnabled(true);
			m_ParentBox->SetVisible(true);

			travelCompletionDistance = std::floor(static_cast<float>(m_ParentBox->GetYPos()) * 0.5F);
			m_ParentBox->SetPositionAbs(0, m_ParentBox->GetYPos() - static_cast<int>(travelCompletionDistance));

			if (m_ParentBox->GetYPos() >= 0) { m_ConsoleState = ConsoleState::Enabled; }
		} else if (m_ConsoleState == ConsoleState::Disabling) {
			travelCompletionDistance = std::ceil((static_cast<float>(m_ParentBox->GetHeight()) + static_cast<float>(m_ParentBox->GetYPos())) * 0.5F);
			m_ParentBox->SetPositionAbs(0, m_ParentBox->GetYPos() - static_cast<int>(travelCompletionDistance));

			if (m_ParentBox->GetYPos() <= -m_ParentBox->GetHeight()) {
				m_ParentBox->SetEnabled(false);
				m_ParentBox->SetVisible(false);
				m_InputTextBox->SetEnabled(false);
				m_GUIControlManager->GetManager()->SetFocus(nullptr);
				m_ConsoleState = ConsoleState::Disabled;
			}
		}

		// If supposed to be enabled or disabled but appears to be the opposite, enable or disable accordingly.
		if (m_ConsoleState == ConsoleState::Enabled && (m_ParentBox->GetYPos() < 0)) {
			m_ConsoleState = ConsoleState::Enabling;
		} else if (m_ConsoleState == ConsoleState::Disabled && (m_ParentBox->GetYPos() > -m_ParentBox->GetHeight())) {
			m_ConsoleState = ConsoleState::Disabling;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::FeedString(bool feedEmptyString) {
		char strLine[1024];
		std::stringstream inputStream(m_InputTextBox->GetText());

		while (!inputStream.fail()) {
			inputStream.getline(strLine, 1024, '\n');
			std::string line = strLine;

			if (!feedEmptyString) {
				if (!line.empty() && line != "\r") {
					g_LuaMan.ClearErrors();
					m_OutputLog.emplace_back("\n" + line);
					g_LuaMan.RunScriptString(line, false);

					if (g_LuaMan.ErrorExists()) { m_OutputLog.emplace_back("\nERROR: " + g_LuaMan.GetLastError()); }
					if (m_InputLog.empty() || m_InputLog.front() != line) { m_InputLog.push_front(line); }

					m_InputLogPosition = m_InputLog.begin();
					m_LastLogMove = 0;
				}
			} else {
				m_OutputLog.emplace_back("\n");
				break;
			}
		}
		m_InputTextBox->SetText("");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::LoadLoggedInput(bool nextEntry) {
		if (nextEntry) {
			// See if we should decrement doubly because the last move was in the opposite direction
			if (m_LastLogMove > 0 && m_InputLogPosition != m_InputLog.begin()) { --m_InputLogPosition; }

			if (m_InputLogPosition == m_InputLog.begin()) {
				m_InputTextBox->SetText("");
				m_LastLogMove = 0;
			} else {
				--m_InputLogPosition;
				m_InputTextBox->SetText(*m_InputLogPosition);
				m_InputTextBox->SetCursorPos(m_InputTextBox->GetText().length());
				m_LastLogMove = -1;
			}
		} else {
			// See if we should increment doubly because the last move was in the opposite direction
			if (m_LastLogMove < 0 && m_InputLogPosition != m_InputLog.end() - 1) { ++m_InputLogPosition; }

			m_InputTextBox->SetText(*m_InputLogPosition);
			m_InputTextBox->SetCursorPos(m_InputTextBox->GetText().length());
			++m_InputLogPosition;
			m_LastLogMove = 1;

			// Avoid falling off the end
			if (m_InputLogPosition == m_InputLog.end()) {
				--m_InputLogPosition;
				m_LastLogMove = 0;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::RemoveGraveAccents() const {
		std::string textBoxString = m_InputTextBox->GetText();
		if (std::find(textBoxString.begin(), textBoxString.end(), '`') != textBoxString.end()) {
			textBoxString.erase(std::remove(textBoxString.begin(), textBoxString.end(), '`'), textBoxString.end());
			m_InputTextBox->SetText(textBoxString);
			m_InputTextBox->SetCursorPos(textBoxString.length());
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ConsoleMan::Draw(BITMAP *targetBitmap) const {
		if (m_ConsoleState != ConsoleState::Disabled) {
			AllegroScreen drawScreen(targetBitmap);
			m_GUIControlManager->Draw(&drawScreen);
		}
	}
}