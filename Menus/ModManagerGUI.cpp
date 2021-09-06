#include "ModManagerGUI.h"

#include "SettingsMan.h"
#include "PresetMan.h"
#include "DataModule.h"
#include "GlobalScript.h"

#include "GUI.h"
#include "AllegroScreen.h"
#include "AllegroInput.h"
#include "GUICollectionBox.h"
#include "GUILabel.h"
#include "GUIButton.h"
#include "GUIListBox.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ModManagerGUI::ModManagerGUI(AllegroScreen *guiScreen, AllegroInput *guiInput) {
		m_GUIControlManager = std::make_unique<GUIControlManager>();
		RTEAssert(m_GUIControlManager->Create(guiScreen, guiInput, "Base.rte/GUIs/Skins/Menus", "MainMenuSubMenuSkin.ini"), "Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Menus/MainMenuSubMenuSkin.ini");
		m_GUIControlManager->Load("Base.rte/GUIs/ModManagerGUI.ini");

		GUICollectionBox *rootBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("root"));
		rootBox->Resize(g_FrameMan.GetResX(), g_FrameMan.GetResY());

		GUICollectionBox *modManagerMenuBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxModManager"));
		modManagerMenuBox->CenterInParent(true, true);
		modManagerMenuBox->SetPositionAbs(modManagerMenuBox->GetXPos(), (rootBox->GetHeight() < 540) ? modManagerMenuBox->GetYPos() - 15 : 140);

		m_BackToMainButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonBackToMainMenu"));
		m_BackToMainButton->SetPositionAbs((rootBox->GetWidth() - m_BackToMainButton->GetWidth()) / 2, modManagerMenuBox->GetYPos() + modManagerMenuBox->GetHeight() + 10);

		m_ModsListBox = dynamic_cast<GUIListBox *>(m_GUIControlManager->GetControl("ListBoxMods"));
		m_ModsListBox->SetMouseScrolling(true);
		m_ModsListBox->SetScrollBarThickness(15);
		m_ModsListBox->SetScrollBarPadding(2);

		m_ScriptsListBox = dynamic_cast<GUIListBox *>(m_GUIControlManager->GetControl("ListBoxScripts"));
		m_ScriptsListBox->SetMouseScrolling(true);
		m_ScriptsListBox->SetScrollBarThickness(15);
		m_ScriptsListBox->SetScrollBarPadding(2);

		m_ToggleModButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonToggleMod"));
		m_ToggleScriptButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonToggleScript"));
		m_ModOrScriptDescriptionLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelDescription"));

		m_ModsListFetched = false;
		m_ScriptsListFetched = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ModManagerGUI::PopulateKnownModsList() {
		for (int i = 0; i < g_PresetMan.GetTotalModuleCount(); ++i) {
			if (i >= g_PresetMan.GetOfficialModuleCount() && i < g_PresetMan.GetTotalModuleCount() - 2) {
				if (const DataModule *dataModule = g_PresetMan.GetDataModule(i)) {
					ModRecord modRecord = { dataModule->GetFileName(), dataModule->GetFriendlyName(), dataModule->GetDescription(), g_SettingsMan.IsModDisabled(dataModule->GetFileName()) };
					m_KnownMods.emplace_back(modRecord);
				}
			}
		}
		// Add missing data from disabled mods settings
		for (const auto &[modPath, modDisabled] : g_SettingsMan.GetDisabledModsMap()) {
			bool found = false;
			for (const ModRecord &knowModListEntry : m_KnownMods) {
				if (modPath == knowModListEntry.ModulePath) {
					found = true;
					break;
				}
			}
			if (!found) {
				ModRecord disabledModRecord = { modPath, "N/A, Module not loaded", "N/A, Module not loaded", modDisabled };
				m_KnownMods.emplace_back(disabledModRecord);
			}
		}
		std::sort(m_KnownMods.begin(), m_KnownMods.end());

		for (int i = 0; i < m_KnownMods.size(); i++) {
			m_ModsListBox->AddItem(m_KnownMods.at(i).GetDisplayString(), std::string(), nullptr, nullptr, i);
		}
		m_ModsListBox->ScrollToTop();
		m_ModsListFetched = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ModManagerGUI::PopulateKnownScriptsList() {
		std::list<Entity *> globalScriptList;
		g_PresetMan.GetAllOfType(globalScriptList, "GlobalScript");

		for (Entity *globalScriptListEntry : globalScriptList) {
			if (const GlobalScript *globalScript = dynamic_cast<GlobalScript *>(globalScriptListEntry)) {
				ScriptRecord scriptRecord = { globalScript->GetModuleAndPresetName(), globalScript->GetDescription(), g_SettingsMan.IsGlobalScriptEnabled(scriptRecord.PresetName) };
				m_KnownScripts.emplace_back(scriptRecord);
			}
		}
		std::sort(m_KnownScripts.begin(), m_KnownScripts.end());

		for (int i = 0; i < m_KnownScripts.size(); i++) {
			m_ScriptsListBox->AddItem(m_KnownScripts.at(i).GetDisplayString(), std::string(), nullptr, nullptr, i);
		}
		m_ScriptsListBox->ScrollToTop();
		m_ScriptsListFetched = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ModManagerGUI::ToggleMod() {
		int index = m_ModsListBox->GetSelectedIndex();
		if (index > -1) {
			std::map<std::string, bool> &disabledModsList = g_SettingsMan.GetDisabledModsMap();
			GUIListPanel::Item *selectedItem = m_ModsListBox->GetSelected();
			ModRecord &modRecord = m_KnownMods.at(selectedItem->m_ExtraIndex);

			modRecord.Disabled = !modRecord.Disabled;
			if (modRecord.Disabled) {
				m_ToggleModButton->SetText("Enable Mod");
				if (disabledModsList.find(modRecord.ModulePath) != disabledModsList.end()) {
					disabledModsList.at(modRecord.ModulePath) = true;
				} else {
					disabledModsList.try_emplace(modRecord.ModulePath, true);
				}
			} else {
				m_ToggleModButton->SetText("Disable Mod");
				disabledModsList.at(modRecord.ModulePath) = false;
			}
			selectedItem->m_Name = modRecord.GetDisplayString();
			m_ModsListBox->SetSelectedIndex(index);
			m_ModsListBox->Invalidate();
			g_GUISound.ItemChangeSound()->Play();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ModManagerGUI::ToggleScript() {
		int index = m_ScriptsListBox->GetSelectedIndex();
		if (index > -1) {
			std::map<std::string, bool> &enabledScriptList = g_SettingsMan.GetEnabledGlobalScriptMap();
			GUIListPanel::Item *selectedItem = m_ScriptsListBox->GetSelected();
			ScriptRecord &scriptRecord = m_KnownScripts.at(selectedItem->m_ExtraIndex);

			scriptRecord.Enabled = !scriptRecord.Enabled;
			if (scriptRecord.Enabled) {
				m_ToggleScriptButton->SetText("Disable Script");
				if (enabledScriptList.find(scriptRecord.PresetName) != enabledScriptList.end()) {
					enabledScriptList.at(scriptRecord.PresetName) = true;
				} else {
					enabledScriptList.try_emplace(scriptRecord.PresetName, true);
				}
			} else {
				m_ToggleScriptButton->SetText("Enable Script");
				enabledScriptList.at(scriptRecord.PresetName) = false;
			}
			selectedItem->m_Name = scriptRecord.GetDisplayString();
			m_ScriptsListBox->SetSelectedIndex(index);
			m_ScriptsListBox->Invalidate();
			g_GUISound.ItemChangeSound()->Play();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ModManagerGUI::HandleInputEvents() {
		if (!ListsFetched()) {
			PopulateKnownModsList();
			PopulateKnownScriptsList();
		}
		m_GUIControlManager->Update();

		GUIEvent guiEvent;
		while (m_GUIControlManager->GetEvent(&guiEvent)) {
			if (guiEvent.GetType() == GUIEvent::Command) {
				if (guiEvent.GetControl() == m_BackToMainButton) {
					return true;
				} else if (guiEvent.GetControl() == m_ToggleModButton) {
					ToggleMod();
				} else if (guiEvent.GetControl() == m_ToggleScriptButton) {
					ToggleScript();
				}
			} else if (guiEvent.GetType() == GUIEvent::Notification) {
				if (guiEvent.GetMsg() == GUIButton::Focused && dynamic_cast<GUIButton *>(guiEvent.GetControl())) { g_GUISound.SelectionChangeSound()->Play(); }

				if (guiEvent.GetControl() == m_ModsListBox && (guiEvent.GetMsg() == GUIListBox::Select && m_ModsListBox->GetSelectedIndex() > -1)) {
					const ModRecord &modRecord = m_KnownMods.at(m_ModsListBox->GetSelected()->m_ExtraIndex);
					m_ModOrScriptDescriptionLabel->SetText(modRecord.Description);
					m_ToggleModButton->SetText(modRecord.Disabled ? "Enable Mod" : "Disable Mod");
				} else if (guiEvent.GetControl() == m_ScriptsListBox && (guiEvent.GetMsg() == GUIListBox::Select && m_ScriptsListBox->GetSelectedIndex() > -1)) {
					const ScriptRecord &scriptRecord = m_KnownScripts.at(m_ScriptsListBox->GetSelected()->m_ExtraIndex);
					m_ModOrScriptDescriptionLabel->SetText(scriptRecord.Description);
					m_ToggleScriptButton->SetText(scriptRecord.Enabled ? "Disable Script" : "Enable Script");
				}
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ModManagerGUI::Draw() const {
		m_GUIControlManager->Draw();
	}
}