/*

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	clear {
		m_pModManagerBackButton = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	create {
		// Mod manager controls
		m_pModManagerBackButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonExitModManager"));
		m_pModManagerToggleModButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonToggleMod"));
		m_pModManagerToggleScriptButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonToggleScript"));
		m_pModManagerModsListBox = dynamic_cast<GUIListBox *>(m_pGUIController->GetControl("ModsLB"));
		m_pModManagerScriptsListBox = dynamic_cast<GUIListBox *>(m_pGUIController->GetControl("ScriptsLB"));
		m_pModManagerDescriptionLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelDescription"));

		///////////////////////////////////////////////////////////////////////////////////////////////
		// Load mod data and fill the lists
		for (int i = 0; i < g_PresetMan.GetTotalModuleCount(); ++i) {
			// Discard official modules
			if (i >= g_PresetMan.GetOfficialModuleCount() && i < g_PresetMan.GetTotalModuleCount() - 2) {
				const DataModule *pModule = g_PresetMan.GetDataModule(i);
				if (pModule) {
					ModRecord r;
					r.ModulePath = pModule->GetFileName();
					r.Description = pModule->GetDescription();
					r.ModuleName = pModule->GetFriendlyName();
					r.Disabled = g_SettingsMan.IsModDisabled(r.ModulePath);

					m_KnownMods.push_back(r);
				}
			}
		}

		// Now add missing data from disabled mods settings
		map<string, bool> disabledMods = g_SettingsMan.GetDisabledModsList();
		for (std::map<std::string, bool>::iterator itr = disabledMods.begin(); itr != disabledMods.end(); ++itr) {
			std::string modPath = itr->first;
			std::transform(modPath.begin(), modPath.end(), modPath.begin(), ::tolower);

			bool found = false;
			// Check if this mod is already in list
			for (vector<ModRecord>::iterator mItr = m_KnownMods.begin(); mItr != m_KnownMods.end(); ++mItr) {
				if (modPath == (*mItr).ModulePath) { found = true; }
			}
			if (!found) {
				ModRecord r;
				r.ModulePath = modPath;
				r.Description = "n/a, module not loaded";
				r.ModuleName = "n/a, module not loaded";
				r.Disabled = itr->second;

				m_KnownMods.push_back(r);
			}
		}

		// Sort the list
		std::sort(m_KnownMods.begin(), m_KnownMods.end());

		// Fill the GUI listbox with loaded mod data
		for (int i = 0; i < m_KnownMods.size(); i++) {
			ModRecord r = m_KnownMods.at(i);
			m_pModManagerModsListBox->AddItem(MakeModString(r), "", 0, 0, i);
		}


		///////////////////////////////////////////////////////////////////////////////////////////////
		// Load script data and fill the lists
		std::list<Entity *> globalScripts;
		g_PresetMan.GetAllOfType(globalScripts, "GlobalScript");

		for (std::list<Entity *>::iterator sItr = globalScripts.begin(); sItr != globalScripts.end(); ++sItr) {
			GlobalScript * script = dynamic_cast<GlobalScript *>(*sItr);
			if (script) {
				ScriptRecord r;
				r.PresetName = script->GetModuleAndPresetName();
				r.Description = script->GetDescription();
				r.Enabled = g_SettingsMan.IsScriptEnabled(r.PresetName);
				m_KnownScripts.push_back(r);
			}
		}

		// Sort the list
		std::sort(m_KnownScripts.begin(), m_KnownScripts.end());

		// Fill the GUI listbox with loaded mod data
		for (int i = 0; i < m_KnownScripts.size(); i++) {
			ScriptRecord r = m_KnownScripts.at(i);
			m_pModManagerScriptsListBox->AddItem(MakeScriptString(r), "", 0, 0, i);
		}

	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	update {
		GUIEvent anEvent;
		while (m_pGUIController->GetEvent(&anEvent)) {
			if (anEvent.GetType() == GUIEvent::Command) {
				/////////////////////////////////////////////
				// MOD MANAGER SCREEN BUTTONS

				if (m_MenuScreen == MODMANAGERSCREEN) {
					// Return to main menu button pressed
					if (anEvent.GetControl() == m_pModManagerBackButton) {
						// Save settings
						g_SettingsMan.UpdateSettingsFile();

						// Hide all screens, the appropriate screen will reappear on next update
						HideAllScreens();
						m_MenuScreen = MAINSCREEN;
						m_ScreenChange = true;
						g_GUISound.BackButtonPressSound()->Play();
					}

					// Disable/Enable mod pressed
					if (anEvent.GetControl() == m_pModManagerToggleModButton) {
						ToggleMod();
					}

					// Disable/Enable script pressed
					if (anEvent.GetControl() == m_pModManagerToggleScriptButton) {
						ToggleScript();
					}
				}

			// Notifications
			else if (anEvent.GetType() == GUIEvent::Notification) {
			// Button focus notification that we can play a sound to
			if (dynamic_cast<GUIButton *>(anEvent.GetControl())) {
				if (anEvent.GetMsg() == GUIButton::Focused) { g_GUISound.SelectionChangeSound()->Play(); }
			}

			// Mod list pressed
			if (anEvent.GetControl() == m_pModManagerModsListBox) {
				if (anEvent.GetMsg() == GUIListBox::Select) {
					int index = m_pModManagerModsListBox->GetSelectedIndex();
					if (index > -1) {
						GUIListPanel::Item *selectedItem = m_pModManagerModsListBox->GetSelected();
						ModRecord r = m_KnownMods.at(selectedItem->m_ExtraIndex);
						m_pModManagerDescriptionLabel->SetText(r.Description);

						if (r.Disabled) {
							m_pModManagerToggleModButton->SetText("Enable");
						} else {
							m_pModManagerToggleModButton->SetText("Disable");
						}
					}
				}

				// Not reliable at all!!
				//if (anEvent.GetMsg() == GUIListBox::DoubleClick)
				//{
				//	ToggleMod();
				//}
			}

			// Script list pressed
			if (anEvent.GetControl() == m_pModManagerScriptsListBox) {
				if (anEvent.GetMsg() == GUIListBox::Select) {
					int index = m_pModManagerScriptsListBox->GetSelectedIndex();
					if (index > -1) {
						GUIListPanel::Item *selectedItem = m_pModManagerScriptsListBox->GetSelected();
						ScriptRecord r = m_KnownScripts.at(selectedItem->m_ExtraIndex);
						m_pModManagerDescriptionLabel->SetText(r.Description);

						if (r.Enabled) {
							m_pModManagerToggleScriptButton->SetText("Disable");
						} else {
							m_pModManagerToggleScriptButton->SetText("Enable");
						}
					}
				}

				//if (anEvent.GetMsg() == GUIListBox::DoubleClick)
				//{
				//	ToggleScript();
				//}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MainMenuGUI::MakeModString(ModRecord r) {
	std::string s;

	if (r.Disabled) {
		s = "- ";
	} else {
		s = "+ ";
	}
	s = s + r.ModulePath + " - " + r.ModuleName;

	return s;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MainMenuGUI::MakeScriptString(ScriptRecord r) {
	std::string s;

	if (!r.Enabled) {
		s = "- ";
	} else {
		s = "+ ";
	}
	s = s + r.PresetName;

	return s;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainMenuGUI::ToggleMod() {
	int index = m_pModManagerModsListBox->GetSelectedIndex();
	if (index > -1) {
		GUIListPanel::Item *selectedItem = m_pModManagerModsListBox->GetSelected();
		ModRecord r = m_KnownMods.at(selectedItem->m_ExtraIndex);

		r.Disabled = !r.Disabled;

		if (r.Disabled) {
			m_pModManagerToggleModButton->SetText("Enable");
			g_SettingsMan.DisableMod(r.ModulePath);
		} else {
			m_pModManagerToggleModButton->SetText("Disable");
			g_SettingsMan.EnableMod(r.ModulePath);
		}
		selectedItem->m_Name = MakeModString(r);
		m_KnownMods[selectedItem->m_ExtraIndex] = r;
		m_pModManagerModsListBox->SetSelectedIndex(index);
		m_pModManagerModsListBox->Invalidate();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainMenuGUI::ToggleScript() {
	int index = m_pModManagerScriptsListBox->GetSelectedIndex();
	if (index > -1) {
		GUIListPanel::Item *selectedItem = m_pModManagerScriptsListBox->GetSelected();
		ScriptRecord r = m_KnownScripts.at(selectedItem->m_ExtraIndex);

		r.Enabled = !r.Enabled;

		if (r.Enabled) {
			m_pModManagerToggleScriptButton->SetText("Disable");
			g_SettingsMan.EnableScript(r.PresetName);
		} else {
			m_pModManagerToggleScriptButton->SetText("Enable");
			g_SettingsMan.DisableScript(r.PresetName);
		}
		selectedItem->m_Name = MakeScriptString(r);
		m_KnownScripts[selectedItem->m_ExtraIndex] = r;
		m_pModManagerScriptsListBox->SetSelectedIndex(index);
		m_pModManagerScriptsListBox->Invalidate();
	}
}
*/