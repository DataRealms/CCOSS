#include "SaveLoadMenuGUI.h"

#include "ActivityMan.h"
#include "PresetMan.h"
#include "WindowMan.h"

#include "GUI.h"
#include "AllegroScreen.h"
#include "GAScripted.h"
#include "GUIInputWrapper.h"
#include "GUICollectionBox.h"
#include "GUILabel.h"
#include "GUIButton.h"
#include "GUIListBox.h"
#include "GUITextBox.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SaveLoadMenuGUI::SaveLoadMenuGUI(AllegroScreen *guiScreen, GUIInputWrapper *guiInput, bool createForPauseMenu) {
		m_GUIControlManager = std::make_unique<GUIControlManager>();
		RTEAssert(m_GUIControlManager->Create(guiScreen, guiInput, "Base.rte/GUIs/Skins/Menus", "MainMenuSubMenuSkin.ini"), "Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Menus/MainMenuSubMenuSkin.ini");
		m_GUIControlManager->Load("Base.rte/GUIs/SaveLoadMenuGUI.ini");

		int rootBoxMaxWidth = g_WindowMan.FullyCoversAllDisplays() ? g_WindowMan.GetPrimaryWindowDisplayWidth() / g_WindowMan.GetResMultiplier() : g_WindowMan.GetResX();

		GUICollectionBox *rootBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("root"));
		rootBox->Resize(rootBoxMaxWidth, g_WindowMan.GetResY());

		GUICollectionBox *saveGameMenuBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxSaveGameMenu"));
		saveGameMenuBox->CenterInParent(true, true);
		saveGameMenuBox->SetPositionAbs(saveGameMenuBox->GetXPos(), (rootBox->GetHeight() < 540) ? saveGameMenuBox->GetYPos() - 15 : 140);

		m_BackToMainButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonBackToMainMenu"));

		if (createForPauseMenu) {
			m_BackToMainButton->SetSize(120, 20);
			m_BackToMainButton->SetText("Back to Pause Menu");
		}
		m_BackToMainButton->SetPositionAbs((rootBox->GetWidth() - m_BackToMainButton->GetWidth()) / 2, saveGameMenuBox->GetYPos() + saveGameMenuBox->GetHeight() + 10);

		m_SaveGamesListBox = dynamic_cast<GUIListBox *>(m_GUIControlManager->GetControl("ListBoxSaveGames"));
		m_SaveGamesListBox->SetFont(m_GUIControlManager->GetSkin()->GetFont("FontConsoleMonospace.png"));
		m_SaveGamesListBox->SetMouseScrolling(true);
		m_SaveGamesListBox->SetScrollBarThickness(15);
		m_SaveGamesListBox->SetScrollBarPadding(2);

		m_SaveGameName = dynamic_cast<GUITextBox *>(m_GUIControlManager->GetControl("SaveGameName"));
		m_LoadButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonLoad"));
		m_CreateButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonCreate"));
		m_DescriptionLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelDescription"));

		m_SaveGamesFetched = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SaveLoadMenuGUI::PopulateSaveGamesList() {
		m_SaveGames.clear();

		std::string saveFilePath = g_PresetMan.GetFullModulePath(c_UserScriptedSavesModuleName) + "/";
		for (const auto &entry : std::filesystem::directory_iterator(saveFilePath)) {
			if (entry.path().extension() == ".ini" && entry.path().filename() != "Index.ini") {
				SaveRecord record;
				record.SavePath = entry.path();
				record.SaveDate = entry.last_write_time();
				m_SaveGames.push_back(record);
			}
		}

		std::for_each(std::execution::par_unseq,
			m_SaveGames.begin(), m_SaveGames.end(),
			[](SaveRecord &record) {
				Reader reader(record.SavePath.string(), true, nullptr, true);

				bool readActivity = false;
				bool readSceneName = false;

				GAScripted activity;

				std::string originalScenePresetName;
				while (reader.NextProperty()) {
					std::string propName = reader.ReadPropName();
					if (propName == "Activity") {
						reader >> activity;
						readActivity = true;
					} else if (propName == "OriginalScenePresetName") {
						reader >> originalScenePresetName;
						readSceneName = true;
					}

					if (readActivity && readSceneName) {
						break;
					}
				}

				record.Activity = activity.GetPresetName();
				record.Scene = originalScenePresetName;
			});

		m_SaveGamesFetched = true;
		UpdateSaveGamesGUIList();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SaveLoadMenuGUI::UpdateSaveGamesGUIList()
	{
		std::sort(m_SaveGames.begin(), m_SaveGames.end());

		m_SaveGamesListBox->ClearList();
		for (int i = 0; i < m_SaveGames.size(); i++) {
			const SaveRecord &save = m_SaveGames[i];

			std::stringstream saveNameText;
			saveNameText << std::left << std::setfill(' ') << std::setw(32) << save.SavePath.stem().string();

			// This is so much more fucking difficult than it has any right to be
			const auto saveFsTime = std::chrono::clock_cast<std::chrono::system_clock>(save.SaveDate);
			const auto saveTime = std::chrono::system_clock::to_time_t(saveFsTime);
			const auto saveTimeLocal = std::localtime(&saveTime);

			std::stringstream saveDateTimeText;
			saveDateTimeText << std::put_time(saveTimeLocal, "%Y-%m-%d %X");

			m_SaveGamesListBox->AddItem(" " + saveNameText.str() + "" + save.Scene + " - " + save.Activity + "", saveDateTimeText.str() + " ", nullptr, nullptr, i);
		}

		m_SaveGamesListBox->ScrollToTop();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SaveLoadMenuGUI::LoadSave() {
		bool success = g_ActivityMan.LoadAndLaunchGame(m_SaveGameName->GetText());
		if (success) {
			g_GUISound.ConfirmSound()->Play();
		} else {
			g_GUISound.UserErrorSound()->Play();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SaveLoadMenuGUI::CreateSave() {
		bool success = g_ActivityMan.SaveCurrentGame(m_SaveGameName->GetText());
		if (success) {
			g_GUISound.ConfirmSound()->Play();
		} else {
			g_GUISound.UserErrorSound()->Play();
		}
		PopulateSaveGamesList();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SaveLoadMenuGUI::HandleInputEvents() {
		if (!ListsFetched()) {
			PopulateSaveGamesList();
		}
		m_GUIControlManager->Update();

		GUIEvent guiEvent;
		while (m_GUIControlManager->GetEvent(&guiEvent)) {
			if (guiEvent.GetType() == GUIEvent::Command) {
				if (guiEvent.GetControl() == m_BackToMainButton) {
					return true;
				} else if (guiEvent.GetControl() == m_LoadButton) {
					LoadSave();
				} else if (guiEvent.GetControl() == m_CreateButton) {
					CreateSave();
				}
			} else if (guiEvent.GetType() == GUIEvent::Notification) {
				if (guiEvent.GetMsg() == GUIButton::Focused && dynamic_cast<GUIButton *>(guiEvent.GetControl())) { 
					g_GUISound.SelectionChangeSound()->Play(); 
				}

				if (guiEvent.GetControl() == m_SaveGamesListBox && (guiEvent.GetMsg() == GUIListBox::Select && m_SaveGamesListBox->GetSelectedIndex() > -1)) {
					const SaveRecord &record = m_SaveGames.at(m_SaveGamesListBox->GetSelected()->m_ExtraIndex);
					//m_DescriptionLabel->SetText(record.GetDisplayString());
					m_SaveGameName->SetText(record.SavePath.stem().string());
				}
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SaveLoadMenuGUI::Draw() const {
		m_GUIControlManager->Draw();
	}
}