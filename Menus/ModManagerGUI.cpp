#include "ModManagerGUI.h"

#include "SettingsMan.h"
#include "PresetMan.h"
#include "DataModule.h"
#include "GlobalScript.h"

#include "GUI.h"
#include "GUILabel.h"
#include "GUIButton.h"
#include "GUIListBox.h"
#include "GUICollectionBox.h"
#include "AllegroScreen.h"
#include "AllegroInput.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ModManagerGUI::ModManagerGUI(AllegroScreen *guiScreen, AllegroInput *guiInput) {
		if (!m_GUIControlManager) { m_GUIControlManager = std::make_unique<GUIControlManager>(); }
		if (!m_GUIControlManager->Create(guiScreen, guiInput, "Base.rte/GUIs/Skins/MainMenu")) { RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/MainMenu"); }
		m_GUIControlManager->Load("Base.rte/GUIs/ModManagerGUI.ini");

		m_RootBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("root"));
		m_RootBox->Resize(g_FrameMan.GetResX(), g_FrameMan.GetResY());

		m_ModManagerScreen = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("ModManagerScreen"));
		if (m_RootBox->GetHeight() < 540) {
			m_ModManagerScreen->CenterInParent(true, true);
		} else {
			m_ModManagerScreen->SetPositionAbs((m_RootBox->GetWidth() - m_ModManagerScreen->GetWidth()) / 2, 140);
		}

		m_BackToMainButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonExitModManager"));
		m_ToggleModButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonToggleMod"));
		m_ToggleScriptButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonToggleScript"));
		m_ModsListBox = dynamic_cast<GUIListBox *>(m_GUIControlManager->GetControl("ModsLB"));
		m_ScriptsListBox = dynamic_cast<GUIListBox *>(m_GUIControlManager->GetControl("ScriptsLB"));
		m_ModOrScriptDescriptionLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelDescription"));

		FillKnownModsList();
		FillKnownScriptsList();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ModManagerGUI::SetEnabled() const {
		m_RootBox->SetVisible(true);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ModManagerGUI::ToggleMod() {
		int index = m_ModsListBox->GetSelectedIndex();
		if (index > -1) {
			GUIListPanel::Item *selectedItem = m_ModsListBox->GetSelected();
			ModRecord modRecord = m_KnownMods.at(selectedItem->m_ExtraIndex);
			modRecord.Disabled = !modRecord.Disabled;
			if (modRecord.Disabled) {
				m_ToggleModButton->SetText("Enable Mod");
				g_SettingsMan.DisableMod(modRecord.ModulePath);
			} else {
				m_ToggleModButton->SetText("Disable Mod");
				g_SettingsMan.EnableMod(modRecord.ModulePath);
			}
			selectedItem->m_Name = MakeModString(modRecord);
			m_KnownMods.at(selectedItem->m_ExtraIndex) = modRecord;
			m_ModsListBox->SetSelectedIndex(index);
			m_ModsListBox->Invalidate();
			g_GUISound.ItemChangeSound()->Play();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ModManagerGUI::ToggleScript() {
		int index = m_ScriptsListBox->GetSelectedIndex();
		if (index > -1) {
			GUIListPanel::Item *selectedItem = m_ScriptsListBox->GetSelected();
			ScriptRecord scriptRecord = m_KnownScripts.at(selectedItem->m_ExtraIndex);
			scriptRecord.Enabled = !scriptRecord.Enabled;
			if (scriptRecord.Enabled) {
				m_ToggleScriptButton->SetText("Disable Script");
				g_SettingsMan.EnableScript(scriptRecord.PresetName);
			} else {
				m_ToggleScriptButton->SetText("Enable Script");
				g_SettingsMan.DisableScript(scriptRecord.PresetName);
			}
			selectedItem->m_Name = MakeScriptString(scriptRecord);
			m_KnownScripts.at(selectedItem->m_ExtraIndex) = scriptRecord;
			m_ScriptsListBox->SetSelectedIndex(index);
			m_ScriptsListBox->Invalidate();
			g_GUISound.ItemChangeSound()->Play();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ModManagerGUI::FillKnownModsList() {
		for (int i = 0; i < g_PresetMan.GetTotalModuleCount(); ++i) {
			// Discard official modules
			if (i >= g_PresetMan.GetOfficialModuleCount() && i < g_PresetMan.GetTotalModuleCount() - 2) {
				const DataModule *dataModule = g_PresetMan.GetDataModule(i);
				if (dataModule) {
					ModRecord modRecord;
					modRecord.ModulePath = dataModule->GetFileName();
					modRecord.Description = dataModule->GetDescription();
					modRecord.ModuleName = dataModule->GetFriendlyName();
					modRecord.Disabled = g_SettingsMan.IsModDisabled(modRecord.ModulePath);
					m_KnownMods.emplace_back(modRecord);
				}
			}
		}
		// Add missing data from disabled mods settings
		std::map<std::string, bool> disabledMods = g_SettingsMan.GetDisabledModsList();
		for (const std::pair<std::string, bool> &disabledModEntry : disabledMods) {
			std::string modPath = disabledModEntry.first;
			std::transform(modPath.begin(), modPath.end(), modPath.begin(), ::tolower);

			bool found = false;
			for (const ModRecord &knowModListEntry : m_KnownMods) {
				if (modPath == knowModListEntry.ModulePath) {
					found = true;
					break;
				}
			}
			if (!found) {
				ModRecord modRecord;
				modRecord.ModulePath = modPath;
				modRecord.Description = "N/A, Module not loaded";
				modRecord.ModuleName = "N/A, Module not loaded";
				modRecord.Disabled = disabledModEntry.second;
				m_KnownMods.emplace_back(modRecord);
			}
		}
		std::sort(m_KnownMods.begin(), m_KnownMods.end());

		for (int i = 0; i < m_KnownMods.size(); i++) {
			ModRecord modRecord = m_KnownMods.at(i);
			m_ModsListBox->AddItem(MakeModString(modRecord), std::string(), nullptr, nullptr, i);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ModManagerGUI::FillKnownScriptsList() {
		std::list<Entity *> globalScriptList;
		g_PresetMan.GetAllOfType(globalScriptList, "GlobalScript");
		for (Entity *globalScriptListEntry : globalScriptList) {
			const GlobalScript *globalScript = dynamic_cast<GlobalScript *>(globalScriptListEntry);
			if (globalScript) {
				ScriptRecord scriptRecord;
				scriptRecord.PresetName = globalScript->GetModuleAndPresetName();
				scriptRecord.Description = globalScript->GetDescription();
				scriptRecord.Enabled = g_SettingsMan.IsScriptEnabled(scriptRecord.PresetName);
				m_KnownScripts.emplace_back(scriptRecord);
			}
		}
		std::sort(m_KnownScripts.begin(), m_KnownScripts.end());

		for (int i = 0; i < m_KnownScripts.size(); i++) {
			ScriptRecord scriptRecord = m_KnownScripts.at(i);
			m_ScriptsListBox->AddItem(MakeScriptString(scriptRecord), std::string(), nullptr, nullptr, i);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ModManagerGUI::HandleInputEvents() {
		m_GUIControlManager->Update();

		GUIEvent guiEvent;
		while (m_GUIControlManager->GetEvent(&guiEvent)) {
			if (guiEvent.GetType() == GUIEvent::Command) {
				if (guiEvent.GetControl() == m_BackToMainButton) {
					g_SettingsMan.UpdateSettingsFile();
					m_RootBox->SetVisible(false);
					return true;
				} else if (guiEvent.GetControl() == m_ToggleModButton) {
					ToggleMod();
				} else if (guiEvent.GetControl() == m_ToggleScriptButton) {
					ToggleScript();
				}
			} else if (guiEvent.GetType() == GUIEvent::Notification) {
				if (dynamic_cast<GUIButton *>(guiEvent.GetControl()) && (guiEvent.GetMsg() == GUIButton::Focused)) { g_GUISound.SelectionChangeSound()->Play(); }

				if (guiEvent.GetControl() == m_ModsListBox && (guiEvent.GetMsg() == GUIListBox::Select && m_ModsListBox->GetSelectedIndex() > -1)) {
					ModRecord modRecord = m_KnownMods.at(m_ModsListBox->GetSelected()->m_ExtraIndex);
					m_ModOrScriptDescriptionLabel->SetText(modRecord.Description);
					m_ToggleModButton->SetText(modRecord.Disabled ? "Enable Mod" : "Disable Mod");
				} else if (guiEvent.GetControl() == m_ScriptsListBox && (guiEvent.GetMsg() == GUIListBox::Select && m_ScriptsListBox->GetSelectedIndex() > -1)) {
					ScriptRecord scriptRecord = m_KnownScripts.at(m_ScriptsListBox->GetSelected()->m_ExtraIndex);
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