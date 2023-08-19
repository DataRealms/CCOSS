#include "SaveLoadMenuGUI.h"

#include "ActivityMan.h"
#include "PresetMan.h"
#include "WindowMan.h"

#include "PauseMenuGUI.h"
#include "SettingsGUI.h"
#include "ModManagerGUI.h"

#include "GUI.h"
#include "AllegroScreen.h"
#include "GAScripted.h"
#include "GUIInputWrapper.h"
#include "GUICollectionBox.h"
#include "GUILabel.h"
#include "GUIButton.h"
#include "GUIListBox.h"
#include "GUITextBox.h"
#include "GUIComboBox.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SaveLoadMenuGUI::SaveLoadMenuGUI(AllegroScreen *guiScreen, GUIInputWrapper *guiInput, bool createForPauseMenu) {
		m_GUIControlManager = std::make_unique<GUIControlManager>();
		RTEAssert(m_GUIControlManager->Create(guiScreen, guiInput, "Base.rte/GUIs/Skins/Menus", "MainMenuSubMenuSkin.ini"), "Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Menus/MainMenuSubMenuSkin.ini");
		m_GUIControlManager->Load("Base.rte/GUIs/SaveLoadMenuGUI.ini");

		int rootBoxMaxWidth = g_WindowMan.FullyCoversAllDisplays() ? g_WindowMan.GetPrimaryWindowDisplayWidth() / g_WindowMan.GetResMultiplier() : g_WindowMan.GetResX();

		GUICollectionBox *rootBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("root"));
		rootBox->Resize(rootBoxMaxWidth, g_WindowMan.GetResY());

		m_SaveGameMenuBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxSaveGameMenu"));
		m_SaveGameMenuBox->CenterInParent(true, true);
		m_SaveGameMenuBox->SetPositionAbs(m_SaveGameMenuBox->GetXPos(), (rootBox->GetHeight() < 540) ? m_SaveGameMenuBox->GetYPos() - 15 : 140);

		m_OrderByComboBox = dynamic_cast<GUIComboBox*>(m_GUIControlManager->GetControl("ComboOrderBy"));
		m_OrderByComboBox->AddItem("Name");
		m_OrderByComboBox->AddItem("Date");
		m_OrderByComboBox->AddItem("Activity");
		m_OrderByComboBox->SetSelectedIndex(1); //order by Date by default

		m_BackToMainButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonBackToMainMenu"));

		if (createForPauseMenu) {
			m_BackToMainButton->SetSize(120, 20);
			m_BackToMainButton->SetText("Back to Pause Menu");
		}
		m_BackToMainButton->SetPositionAbs((rootBox->GetWidth() - m_BackToMainButton->GetWidth()) / 2, m_SaveGameMenuBox->GetYPos() + m_SaveGameMenuBox->GetHeight() + 10);

		m_SaveGamesListBox = dynamic_cast<GUIListBox *>(m_GUIControlManager->GetControl("ListBoxSaveGames"));
		m_SaveGamesListBox->SetFont(m_GUIControlManager->GetSkin()->GetFont("FontConsoleMonospace.png"));
		m_SaveGamesListBox->SetMouseScrolling(true);
		m_SaveGamesListBox->SetScrollBarThickness(15);
		m_SaveGamesListBox->SetScrollBarPadding(2);

		m_SaveGameName = dynamic_cast<GUITextBox *>(m_GUIControlManager->GetControl("SaveGameName"));
		m_LoadButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonLoad"));
		m_CreateButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonCreate"));
		m_OverwriteButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonOverwrite"));
		m_DeleteButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonDelete"));
		m_ActivityCannotBeSavedLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("ActivityCannotBeSavedWarning"));

		m_ConfirmationBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("ConfirmDialog"));
		m_ConfirmationBox->CenterInParent(true, true);

		m_ConfirmationLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("ConfirmLabel"));
		m_ConfirmationButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ConfirmButton"));
		m_CancelButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("CancelButton"));

		SwitchToConfirmDialogMode(ConfirmDialogMode::None);

		m_SaveGamesFetched = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SaveLoadMenuGUI::PopulateSaveGamesList() {
		m_SaveGames.clear();
		m_SaveGameName->SetText("");
		
		m_GUIControlManager->GetManager()->SetFocus(nullptr);

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
		const std::string& currentOrder = m_OrderByComboBox->GetSelectedItem()->m_Name;
		if (currentOrder == "Name") {
			std::stable_sort(m_SaveGames.begin(), m_SaveGames.end(), [](const SaveRecord& lhs, const SaveRecord& rhs) { return lhs.SavePath.stem().string() < rhs.SavePath.stem().string(); });
		} else if (currentOrder == "Date") {
			std::stable_sort(m_SaveGames.begin(), m_SaveGames.end(), [](const SaveRecord& lhs, const SaveRecord& rhs) { return lhs.SaveDate > rhs.SaveDate; });
		} else if (currentOrder == "Activity") {
			std::stable_sort(m_SaveGames.begin(), m_SaveGames.end(), [](const SaveRecord& lhs, const SaveRecord& rhs) { return lhs.Activity < rhs.Activity; });
		}

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

	bool SaveLoadMenuGUI::LoadSave() {
		bool success = g_ActivityMan.LoadAndLaunchGame(m_SaveGameName->GetText());

		if (success) {
			g_GUISound.ConfirmSound()->Play();
		} else {
			g_GUISound.UserErrorSound()->Play();
		}

		return success;
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

	void SaveLoadMenuGUI::DeleteSave() {
		std::string saveFilePath = g_PresetMan.GetFullModulePath(c_UserScriptedSavesModuleName) + "/" + m_SaveGameName->GetText();

		// TODO - it'd be nice to have this all zipped up into one file...
		std::vector<std::filesystem::path> filePaths;
		filePaths.emplace_back(saveFilePath + ".ini");
		filePaths.emplace_back(saveFilePath + " BG.png");
		filePaths.emplace_back(saveFilePath + " FG.png");
		filePaths.emplace_back(saveFilePath + " Mat.png");
		filePaths.emplace_back(saveFilePath + " UST1.png");
		filePaths.emplace_back(saveFilePath + " UST2.png");
		filePaths.emplace_back(saveFilePath + " UST3.png");
		filePaths.emplace_back(saveFilePath + " UST4.png");

		std::for_each(std::execution::par_unseq,
			filePaths.begin(), filePaths.end(),
			[](const std::filesystem::path &path) {
				std::filesystem::remove(path);
			});

		g_GUISound.ConfirmSound()->Play();

		PopulateSaveGamesList();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SaveLoadMenuGUI::UpdateButtonEnabledStates() {
		bool allowSave = g_ActivityMan.GetActivityAllowsSaving() && m_SaveGameName->GetText() != "";

		int existingSaveItemIndex = -1;
		for (int i = 0; i < m_SaveGamesListBox->GetItemList()->size(); ++i) {
			SaveRecord& save = m_SaveGames[m_SaveGamesListBox->GetItem(i)->m_ExtraIndex];
			if (save.SavePath.stem().string() == m_SaveGameName->GetText()) {
				existingSaveItemIndex = i;
				break;
			}
		}

		// Select the item in the list - selecting -1 unselects all
		m_SaveGamesListBox->SetSelectedIndex(existingSaveItemIndex);

		bool saveExists = existingSaveItemIndex != -1;

		bool allowCreate = allowSave && !saveExists;
		m_CreateButton->SetVisible(allowCreate);
		m_CreateButton->SetEnabled(allowCreate);

		bool allowOverwrite = allowSave && saveExists;
		m_OverwriteButton->SetVisible(allowOverwrite);
		m_OverwriteButton->SetEnabled(allowOverwrite);

		m_LoadButton->SetEnabled(saveExists);
		m_DeleteButton->SetEnabled(saveExists);

		m_ActivityCannotBeSavedLabel->SetVisible(g_ActivityMan.GetActivity() && !g_ActivityMan.GetActivityAllowsSaving());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SaveLoadMenuGUI::SwitchToConfirmDialogMode(ConfirmDialogMode mode)
	{
		m_ConfirmDialogMode = mode;

		bool dialogOpen = m_ConfirmDialogMode != ConfirmDialogMode::None;
		m_SaveGameMenuBox->SetEnabled(!dialogOpen);
		m_ConfirmationBox->SetEnabled(dialogOpen);
		m_ConfirmationBox->SetVisible(dialogOpen);

		switch (m_ConfirmDialogMode) {
		case ConfirmDialogMode::ConfirmOverwrite:
			m_ConfirmationLabel->SetText("Are you sure you want to overwrite this savegame?");
			break;
		case ConfirmDialogMode::ConfirmDelete:
			m_ConfirmationLabel->SetText("Are you sure you want to delete this savegame?");
			break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SaveLoadMenuGUI::HandleInputEvents(PauseMenuGUI *pauseMenu) {
		if (!ListsFetched()) {
			PopulateSaveGamesList();
			UpdateButtonEnabledStates();
		}

		m_GUIControlManager->Update();

		GUIEvent guiEvent;
		while (m_GUIControlManager->GetEvent(&guiEvent)) {
			if (guiEvent.GetType() == GUIEvent::Command) {
				if (guiEvent.GetControl() == m_BackToMainButton) {
					return true;
				} else if (guiEvent.GetControl() == m_LoadButton) {
					bool gameLoaded = LoadSave();
					if (gameLoaded) {
						if (pauseMenu) {
							pauseMenu->ClearBackdrop();
						}
						return true;
					}
				} else if (guiEvent.GetControl() == m_CreateButton) {
					CreateSave();
				} else if (guiEvent.GetControl() == m_OverwriteButton) {
					SwitchToConfirmDialogMode(ConfirmDialogMode::ConfirmOverwrite);
				} else if (guiEvent.GetControl() == m_DeleteButton) {
					SwitchToConfirmDialogMode(ConfirmDialogMode::ConfirmDelete);
				} else if (guiEvent.GetControl() == m_ConfirmationButton) {
					switch (m_ConfirmDialogMode) {
					case ConfirmDialogMode::ConfirmOverwrite:
						CreateSave();
						break;
					case ConfirmDialogMode::ConfirmDelete:
						DeleteSave();
						break;
					}
					SwitchToConfirmDialogMode(ConfirmDialogMode::None);
				} else if (guiEvent.GetControl() == m_CancelButton) {
					SwitchToConfirmDialogMode(ConfirmDialogMode::None);
				}
			} else if (guiEvent.GetType() == GUIEvent::Notification) {
				if (guiEvent.GetMsg() == GUIButton::Focused && dynamic_cast<GUIButton *>(guiEvent.GetControl())) { 
					g_GUISound.SelectionChangeSound()->Play(); 
				}

				if (guiEvent.GetControl() == m_SaveGamesListBox && (guiEvent.GetMsg() == GUIListBox::Select && m_SaveGamesListBox->GetSelectedIndex() > -1)) {
					const SaveRecord &record = m_SaveGames[m_SaveGamesListBox->GetSelected()->m_ExtraIndex];
					m_SaveGameName->SetText(record.SavePath.stem().string());
				}

				if (guiEvent.GetControl() == m_OrderByComboBox && guiEvent.GetMsg() == GUIComboBox::Closed) {
					UpdateSaveGamesGUIList();
				}
			}
		}

		UpdateButtonEnabledStates();

		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SaveLoadMenuGUI::Draw() const {
		m_GUIControlManager->Draw();
	}
}