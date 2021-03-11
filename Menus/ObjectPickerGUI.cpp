#include "ObjectPickerGUI.h"

#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "UInputMan.h"
#include "SettingsMan.h"

#include "GUI/GUI.h"
#include "GUI/AllegroBitmap.h"
#include "GUI/AllegroScreen.h"
#include "GUI/AllegroInput.h"
#include "GUI/GUIControlManager.h"
#include "GUI/GUICollectionBox.h"
#include "GUI/GUIListBox.h"
#include "GUI/GUILabel.h"

#include "DataModule.h"
#include "SceneObject.h"
#include "EditorActivity.h"
#include "BunkerAssembly.h"
#include "BunkerAssemblyScheme.h"

namespace RTE {

	BITMAP *ObjectPickerGUI::s_Cursor = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::Clear() {
		m_CursorPos.Reset();
		m_GUIScreen = nullptr;
		m_GUIInput = nullptr;
		m_GUIController = nullptr;
		m_ParentBox = nullptr;
		m_PopupBox = nullptr;
		m_PopupText = nullptr;
		m_GroupsList = nullptr;
		m_ObjectsList = nullptr;
		m_Controller = nullptr;
		m_PickerEnabled = PickerState::Disabled;
		m_PickerFocus = PickerFocus::Groups;
		m_FocusChange = false;
		m_MenuSpeed = 0.3F;
		m_ModuleSpaceID = -1;
		m_ShowType.clear();
		m_SelectedGroupIndex = 0;
		m_SelectedObjectIndex = 0;
		m_LastHoveredMouseIndex = 0;
		m_NativeTechModule = 0;
		m_ForeignCostMult = 4.0F;
		m_PickedObject = nullptr;
		m_RepeatStartTimer.Reset();
		m_RepeatTimer.Reset();

		int moduleCount = g_PresetMan.GetTotalModuleCount();
		m_ExpandedModules = new bool[moduleCount];
		for (int i = 0; i < moduleCount; ++i) {
			m_ExpandedModules[i] = i == 0 ? true : false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ObjectPickerGUI::Create(Controller *controller, int whichModuleSpace, const std::string_view &onlyOfType) {
		RTEAssert(controller, "No controller sent to ObjectPickerGUI on creation!");
		m_Controller = controller;

		if (!m_GUIScreen) { m_GUIScreen = new AllegroScreen(g_FrameMan.GetBackBuffer8()); }
		if (!m_GUIInput) { m_GUIInput = new AllegroInput(controller->GetPlayer()); }
		if (!m_GUIController) { m_GUIController = new GUIControlManager(); }
		if (!m_GUIController->Create(m_GUIScreen, m_GUIInput, "Base.rte/GUIs/Skins/Base")) { RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Base"); }

		m_GUIController->Load("Base.rte/GUIs/ObjectPickerGUI.ini");
		m_GUIController->EnableMouse(controller->IsMouseControlled());

		if (!s_Cursor) {
			ContentFile cursorFile("Base.rte/GUIs/Skins/Cursor.png");
			s_Cursor = cursorFile.GetAsBitmap();
		}

		// Stretch the invisible root box to fill the screen
		if (g_FrameMan.IsInMultiplayerMode()) {
			dynamic_cast<GUICollectionBox *>(m_GUIController->GetControl("base"))->SetSize(g_FrameMan.GetPlayerFrameBufferWidth(controller->GetPlayer()), g_FrameMan.GetPlayerFrameBufferHeight(controller->GetPlayer()));
		} else {
			dynamic_cast<GUICollectionBox *>(m_GUIController->GetControl("base"))->SetSize(g_FrameMan.GetResX(), g_FrameMan.GetResY());
		}

		// Make sure we have convenient points to the containing GUI collection boxes that we will manipulate the positions of
		if (!m_ParentBox) {
			m_ParentBox = dynamic_cast<GUICollectionBox *>(m_GUIController->GetControl("PickerGUIBox"));

			// Set the background image of the parent collection box
			//ContentFile backgroundFile("Base.rte/GUIs/BuyMenuBackground.png");
			//m_ParentBox->SetDrawImage(new AllegroBitmap(backgroundFile.GetAsBitmap()));
			//m_ParentBox->SetDrawBackground(true);
			//m_ParentBox->SetDrawType(GUICollectionBox::Image);
			m_ParentBox->SetDrawType(GUICollectionBox::Color);
		}
		m_ParentBox->SetPositionAbs(g_FrameMan.GetPlayerFrameBufferWidth(m_Controller->GetPlayer()), 0);
		m_ParentBox->SetEnabled(false);
		m_ParentBox->SetVisible(false);

		if (!m_PopupBox) {
			m_PopupBox = dynamic_cast<GUICollectionBox *>(m_GUIController->GetControl("BuyGUIPopup"));
			m_PopupText = dynamic_cast<GUILabel *>(m_GUIController->GetControl("PopupText"));

			m_PopupBox->SetDrawType(GUICollectionBox::Panel);
			m_PopupBox->SetDrawBackground(true);
			// Never enable the popup, because it steals focus and causes other windows to think the cursor left them
			m_PopupBox->SetEnabled(false);
			m_PopupBox->SetVisible(false);
			// Set the font
			m_PopupText->SetFont(m_GUIController->GetSkin()->GetFont("smallfont.png"));
		}

		m_GroupsList = dynamic_cast<GUIListBox *>(m_GUIController->GetControl("GroupsLB"));
		m_ObjectsList = dynamic_cast<GUIListBox *>(m_GUIController->GetControl("ObjectsLB"));

		// If we're not split screen horizontally, then stretch out the layout for all the relevant controls
		if (g_FrameMan.IsInMultiplayerMode()) {
			int stretchAmount = g_FrameMan.GetPlayerFrameBufferHeight(controller->GetPlayer()) / 2;

			m_ParentBox->SetSize(m_ParentBox->GetWidth(), m_ParentBox->GetHeight() + stretchAmount);
			m_GroupsList->SetSize(m_GroupsList->GetWidth(), m_GroupsList->GetHeight() + stretchAmount);
			m_ObjectsList->SetSize(m_ObjectsList->GetWidth(), m_ObjectsList->GetHeight() + stretchAmount);
		} else {
			if (!g_FrameMan.GetHSplit()) {
				int stretchAmount = g_FrameMan.GetResY() / 2;

				m_ParentBox->SetSize(m_ParentBox->GetWidth(), m_ParentBox->GetHeight() + stretchAmount);
				m_GroupsList->SetSize(m_GroupsList->GetWidth(), m_GroupsList->GetHeight() + stretchAmount);
				m_ObjectsList->SetSize(m_ObjectsList->GetWidth(), m_ObjectsList->GetHeight() + stretchAmount);
			}
		}

		m_GroupsList->SetAlternateDrawMode(false);
		m_ObjectsList->SetAlternateDrawMode(true);
		m_GroupsList->SetMultiSelect(false);
		m_ObjectsList->SetMultiSelect(false);

		// Set initial focus, category list, and label settings
		m_PickerFocus = PickerFocus::Groups;
		m_FocusChange = true;

		// Reset repeat timers
		m_RepeatStartTimer.Reset();
		m_RepeatTimer.Reset();

		// Set up the groups to show from the module space we are working within. This also updates the Objects list
		SetModuleSpace(whichModuleSpace);

		// Show only objects of this specific type
		ShowOnlyType(onlyOfType);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::Destroy() {
		delete m_GUIController;
		delete m_GUIInput;
		delete m_GUIScreen;

		delete[] m_ExpandedModules;

		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::SetEnabled(bool enable) {
		if (enable && m_PickerEnabled != PickerState::Enabled && m_PickerEnabled != PickerState::Enabling) {
			if (g_FrameMan.IsInMultiplayerMode()) {
				int stretchAmount = g_FrameMan.GetPlayerFrameBufferHeight(m_Controller->GetPlayer()) - m_ParentBox->GetHeight();
				if (stretchAmount != 0) {
					m_ParentBox->SetSize(m_ParentBox->GetWidth(), m_ParentBox->GetHeight() + stretchAmount);
					m_GroupsList->SetSize(m_GroupsList->GetWidth(), m_GroupsList->GetHeight() + stretchAmount);
					m_ObjectsList->SetSize(m_ObjectsList->GetWidth(), m_ObjectsList->GetHeight() + stretchAmount);
				}
			} else {
				// If we're not split screen horizontally, then stretch out the layout for all the relevant controls
				int stretchAmount = g_FrameMan.GetPlayerScreenHeight() - m_ParentBox->GetHeight();
				if (stretchAmount != 0) {
					m_ParentBox->SetSize(m_ParentBox->GetWidth(), m_ParentBox->GetHeight() + stretchAmount);
					m_GroupsList->SetSize(m_GroupsList->GetWidth(), m_GroupsList->GetHeight() + stretchAmount);
					m_ObjectsList->SetSize(m_ObjectsList->GetWidth(), m_ObjectsList->GetHeight() + stretchAmount);
				}
			}

			m_PickerEnabled = PickerState::Enabling;
			// Reset repeat timers
			m_RepeatStartTimer.Reset();
			m_RepeatTimer.Reset();
			// Set the mouse cursor free
			g_UInputMan.TrapMousePos(false, m_Controller->GetPlayer());

			// Move the mouse cursor to the middle of the player's screen
			int mouseOffX;
			int mouseOffY;
			m_GUIInput->GetMouseOffset(mouseOffX, mouseOffY);
			Vector mousePos(-mouseOffX + (g_FrameMan.GetPlayerFrameBufferWidth(m_Controller->GetPlayer()) / 2), -mouseOffY + (g_FrameMan.GetPlayerFrameBufferHeight(m_Controller->GetPlayer()) / 2));
			g_UInputMan.SetMousePos(mousePos, m_Controller->GetPlayer());
			// Default focus to the groups list if the objects are empty
			if (m_ObjectsList->GetItemList()->empty()) {
				m_PickerFocus = PickerFocus::Groups;
			} else {
				m_PickerFocus = PickerFocus::Objects;
			}
			m_FocusChange = true;
			g_GUISound.EnterMenuSound()->Play(m_Controller->GetPlayer());
		} else if (!enable && m_PickerEnabled != PickerState::Disabled && m_PickerEnabled != PickerState::Disabling) {
			m_PickerEnabled = PickerState::Disabling;
			// Trap the mouse cursor again
			g_UInputMan.TrapMousePos(true, m_Controller->GetPlayer());
			// Only play switching away sound
			//if (!m_PickedObject)
			g_GUISound.ExitMenuSound()->Play(m_Controller->GetPlayer());
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::SetPosOnScreen(int newPosX, int newPosY) {
		m_GUIController->SetPosOnScreen(newPosX, newPosY);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::SetModuleSpace(int moduleSpaceID) {
		bool updateLists = m_ModuleSpaceID != moduleSpaceID;

		m_ModuleSpaceID = moduleSpaceID;

		// This updates the objects list too
		if (updateLists) { UpdateGroupsList(); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ObjectPickerGUI::ShowSpecificGroup(const std::string_view &groupName) {
		bool found = false;
		int index = 0;

		for (const GUIListPanel::Item *groupListItem : *m_GroupsList->GetItemList()) {
			if (groupListItem->m_Name == groupName) {
				found = true;
				m_SelectedGroupIndex = index;
				m_GroupsList->SetSelectedIndex(m_SelectedGroupIndex);
				UpdateObjectsList();
				m_PickerFocus = PickerFocus::Objects;
				m_FocusChange = 1;
			}
			index++;
		}
		return found;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::SetNativeTechModule(int whichModule) {
		if (whichModule >= 0 && whichModule < g_PresetMan.GetTotalModuleCount()) {
			// Set the multipliers and refresh everything that needs refreshing to reflect the change
			m_NativeTechModule = whichModule;
			// Object picker is first created with nativeTech = 0, which expands all modules but does not display them as expanded.
			// When visually collapsed module is first clicked, it internally turns itself into collapsed while the others display themselves as expanded.
			// To avoid that, just don't expand anything if we pass Base.rte as a native tech as Base.rte is always expanded by default anyway.
			if (whichModule > 0) { SetModuleExpanded(m_NativeTechModule); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::SetModuleExpanded(int whichModule, bool expanded) {
		int moduleCount = g_PresetMan.GetTotalModuleCount();
		if (whichModule > 0 && whichModule < moduleCount) {
			m_ExpandedModules[whichModule] = expanded;
			UpdateObjectsList(false);
		} else {
			// If base module (0), or out of range module, then affect all
			for (int m = 0; m < moduleCount; ++m) {
				m_ExpandedModules[m] = expanded;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const SceneObject * ObjectPickerGUI::GetNextObject() {
		m_SelectedObjectIndex++;
		// Loop around
		if (m_SelectedObjectIndex >= m_ObjectsList->GetItemList()->size()) { m_SelectedObjectIndex = 0; }

		m_ObjectsList->SetSelectedIndex(m_SelectedObjectIndex);
		// Report the newly selected item as being 'picked', but don't close the picker
		const GUIListPanel::Item *pItem = m_ObjectsList->GetSelected();
		if (pItem) {
			g_GUISound.SelectionChangeSound()->Play(m_Controller->GetPlayer());
			return dynamic_cast<const SceneObject *>(pItem->m_pEntity);
		}
		return nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const SceneObject * ObjectPickerGUI::GetPrevObject() {
		m_SelectedObjectIndex--;
		// Loop around
		if (m_SelectedObjectIndex < 0) { m_SelectedObjectIndex = m_ObjectsList->GetItemList()->size() - 1; }

		m_ObjectsList->SetSelectedIndex(m_SelectedObjectIndex);
		// Report the newly selected item as being 'picked', but don't close the picker
		const GUIListPanel::Item *pItem = m_ObjectsList->GetSelected();
		if (pItem) {
			g_GUISound.SelectionChangeSound()->Play(m_Controller->GetPlayer());
			return dynamic_cast<const SceneObject *>(pItem->m_pEntity);
		}
		return nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::UpdateGroupsList() {
		// Load in all the groups that exist in the given module space
		m_GroupsList->ClearList();

		//    m_GroupsList->AddItem("Bunker Modules");
		/*
			m_GroupsList->AddItem("Brains");
			m_GroupsList->AddItem("Weapons");
			m_GroupsList->AddItem("Clones");
			m_GroupsList->AddItem("Robots");
			m_GroupsList->AddItem("Brains");
		*/
		// Get the registered groups of all official modules loaded before this + the specific module (official or not) one we're picking from
		std::list<string> groupList;
		g_PresetMan.GetModuleSpaceGroups(groupList, m_ModuleSpaceID, m_ShowType);
		std::list<Entity *> objectList;
		const SceneObject *pSObject = 0;
		bool hasObjectsToShow = false;
		bool showSchemes = false;

		if (dynamic_cast<EditorActivity *>(g_ActivityMan.GetActivity())) { showSchemes = true; }

		for (std::list<std::string>::const_iterator gItr = groupList.begin(); gItr != groupList.end(); ++gItr) {
			bool onlyAssembliesInGroup = true;
			bool onlySchemesInGroup = true;

			// Get the actual object list for each group so we can check if they're empty or not
			objectList.clear();
			g_PresetMan.GetAllOfGroupInModuleSpace(objectList, *gItr, m_ShowType, m_ModuleSpaceID);

			// Go through the object list of this group and see if it contains any items we actually want to show
			hasObjectsToShow = false;
			for (std::list<Entity *>::iterator oItr = objectList.begin(); oItr != objectList.end(); ++oItr) {
				// Check if we have any other objects than assemblies to skip assembly groups.
				if (!dynamic_cast<BunkerAssembly *>(*oItr)) { onlyAssembliesInGroup = false; }

				// Check if we have any other objects than schemes to skip schemes group.
				if (!dynamic_cast<BunkerAssemblyScheme *>(*oItr)) { onlySchemesInGroup = false; }

				pSObject = dynamic_cast<SceneObject *>(*oItr);
				// Buyable?
				if (pSObject && pSObject->IsBuyable()) {
					hasObjectsToShow = true;
					break;
				}
			}

			// If we have this assembly group in the list of visible assembly groups, then show it no matter what
			if (onlyAssembliesInGroup) {
				std::list<std::string> visibleAssemblyGroups = g_SettingsMan.GetVisibleAssemblyGroupsList();

				for (std::list<std::string>::const_iterator aItr = visibleAssemblyGroups.begin(); aItr != visibleAssemblyGroups.end(); ++aItr)
					if (*gItr == *aItr) {
						onlyAssembliesInGroup = false;
						break;
					}
			}

			// Only add the group if it has something in it!
			if (!objectList.empty() && hasObjectsToShow && (!onlyAssembliesInGroup || *gItr == "Assemblies") && (!onlySchemesInGroup || showSchemes)) {// && *gItr != "Bunker Modules")
				m_GroupsList->AddItem(*gItr);
			}
		}

		// Select and load the first group
		const GUIListPanel::Item *pItem = m_GroupsList->GetItem(0);
		if (pItem) {
			m_GroupsList->SetSelectedIndex(m_SelectedGroupIndex = 0);
			UpdateObjectsList();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::UpdateObjectsList(bool selectTop) {
		m_ObjectsList->ClearList();

		// The vector of lists which will be filled with objects, grouped by which data module they were read from
		std::vector<std::list<Entity *> > moduleList;
		int moduleID = 0;

		// Make as many DataModule entries as necessary in the vector
		while (moduleList.size() < g_PresetMan.GetTotalModuleCount()) {
			moduleList.push_back(std::list<Entity *>());
		}

		// Get the name of the selected group
		const GUIListPanel::Item *pItem = m_GroupsList->GetSelected();
		if (pItem) {
			// Show objects from ALL modules
			if (m_ModuleSpaceID < 0) {
				if (g_SettingsMan.ShowForeignItems() || m_NativeTechModule <= 0) {
					// Go through ALL the data modules, gathering the objects that match the criteria in each one
					for (moduleID = 0; moduleID < g_PresetMan.GetTotalModuleCount(); ++moduleID) {
						g_PresetMan.GetAllOfGroup(moduleList[moduleID], pItem->m_Name, m_ShowType, moduleID);
					}
				} else {
					for (moduleID = 0; moduleID < g_PresetMan.GetTotalModuleCount(); ++moduleID) {
						if (moduleID == 0 || moduleID == m_NativeTechModule) { g_PresetMan.GetAllOfGroup(moduleList[moduleID], pItem->m_Name, m_ShowType, moduleID); }
					}

				}
			} else {
				// Only show objects from specific module space
				// Go through all the official data modules, gathering the objects that match the criteria in each one
				for (moduleID = 0; moduleID < g_PresetMan.GetOfficialModuleCount() && moduleID < m_ModuleSpaceID; ++moduleID) {
					g_PresetMan.GetAllOfGroup(moduleList[moduleID], pItem->m_Name, m_ShowType, moduleID);
				}

				// Now the stuff from the current module, official or not
				g_PresetMan.GetAllOfGroup(moduleList[m_ModuleSpaceID], pItem->m_Name, m_ShowType, m_ModuleSpaceID);
			}
		}

		// Now fill the item list with the module-separated items of the currently selected group
		SceneObject *pSObject = nullptr;
		const DataModule *pModule = nullptr;
		GUIBitmap *pItemBitmap = nullptr;
		std::list<SceneObject *> tempList;
		// Go through all modules we've gathered from
		for (moduleID = 0; moduleID < moduleList.size(); ++moduleID) {
			// Don't add an empty module grouping
			if (!moduleList[moduleID].empty()) {
				tempList.clear();

				// Move all valid/desired entities from the module list to the intermediate list
				for (std::list<Entity *>::iterator oItr = moduleList[moduleID].begin(); oItr != moduleList[moduleID].end(); ++oItr) {
					pSObject = dynamic_cast<SceneObject *>(*oItr);
					// Buyable?
					if (pSObject && pSObject->IsBuyable()) { tempList.push_back(pSObject); }
				}

				// Don't add anything to the real buy item list if the current module didn't yield any valid items
				if (!tempList.empty()) {
					// Add the DataModule separator in the item list, with appropriate name and perhaps icon? Don't add for first base module
					if (moduleID != 0 && (pModule = g_PresetMan.GetDataModule(moduleID))) {
						// Get the module icon so we can put it in the list separator
						pItemBitmap = pModule->GetIcon() ? new AllegroBitmap(pModule->GetIcon()) : 0;
						// Passing in ownership of the bitmap, making uppercase the name
						std::string name = pModule->GetFriendlyName();
						transform(name.begin(), name.end(), name.begin(), ::toupper);
						m_ObjectsList->AddItem(name, m_ExpandedModules[moduleID] ? "-" : "+", pItemBitmap, 0, moduleID);
					}

					// If the module is expanded, add all the items within it below
					if (moduleID == 0 || m_ExpandedModules[moduleID]) {
						// Transfer from the temp intermediate list to the real GUI list
						for (std::list<SceneObject *>::iterator tItr = tempList.begin(); tItr != tempList.end(); ++tItr) {
							pItemBitmap = new AllegroBitmap((*tItr)->GetGraphicalIcon());
							m_ObjectsList->AddItem((*tItr)->GetPresetName(), (*tItr)->GetGoldValueString(m_NativeTechModule, m_ForeignCostMult), pItemBitmap, *tItr);
						}
					}
				}
			}
		}

		if (selectTop) {
			// Scroll the item list to the top
			m_ObjectsList->ScrollToTop();
			m_ObjectsList->SetSelectedIndex(m_SelectedObjectIndex = 0);
			// Set the picked object to be the one now selected at the top
			pItem = m_ObjectsList->GetSelected();
			if (pItem) { m_PickedObject = dynamic_cast<const SceneObject *>(pItem->m_pEntity); }
		} else {
			// Attempt to leave the selection where it was
			m_ObjectsList->SetSelectedIndex(m_SelectedObjectIndex);
			m_ObjectsList->ScrollToSelected();
			// Set the picked object to be the one now selected
			pItem = m_ObjectsList->GetSelected();
			if (pItem) { m_PickedObject = dynamic_cast<const SceneObject *>(pItem->m_pEntity); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::Update() {
		// Enable mouse input if the controller allows it
		m_GUIController->EnableMouse(m_Controller->IsMouseControlled());

		// Reset the purchasing indicator
		m_PickedObject = nullptr;

		// Popup box is hidden by default
		m_PopupBox->SetVisible(false);

		////////////////////////////////////////////////////////////////////////
		// Animate the menu into and out of view if enabled or disabled

		if (m_PickerEnabled == PickerState::Enabling) {
			m_ParentBox->SetEnabled(true);
			m_ParentBox->SetVisible(true);

			Vector position;
			Vector occlusion;

			float enabledPos = g_FrameMan.GetPlayerFrameBufferWidth(m_Controller->GetPlayer()) - m_ParentBox->GetWidth();

			float toGo = std::floor((enabledPos - (float)m_ParentBox->GetXPos()) * m_MenuSpeed);
			position.m_X = m_ParentBox->GetXPos() + toGo;
			occlusion.m_X = m_ParentBox->GetXPos() - g_FrameMan.GetPlayerFrameBufferWidth(m_Controller->GetPlayer());

			m_ParentBox->SetPositionAbs(position.m_X, position.m_Y);
			g_SceneMan.SetScreenOcclusion(occlusion, g_ActivityMan.GetActivity()->ScreenOfPlayer(m_Controller->GetPlayer()));

			if (m_ParentBox->GetXPos() <= enabledPos) { m_PickerEnabled = PickerState::Enabled; }
		} else if (m_PickerEnabled == PickerState::Disabling) {
			// Animate the menu out of view
			float disabledPos = g_FrameMan.GetPlayerFrameBufferWidth(m_Controller->GetPlayer());

			float toGo = std::ceil((disabledPos - (float)m_ParentBox->GetXPos()) * m_MenuSpeed);
			m_ParentBox->SetPositionAbs(m_ParentBox->GetXPos() + toGo, 0);
			g_SceneMan.SetScreenOcclusion(Vector(m_ParentBox->GetXPos() - g_FrameMan.GetPlayerFrameBufferWidth(m_Controller->GetPlayer()), 0), g_ActivityMan.GetActivity()->ScreenOfPlayer(m_Controller->GetPlayer()));
			m_PopupBox->SetVisible(false);

			if (m_ParentBox->GetXPos() >= g_FrameMan.GetPlayerFrameBufferWidth(m_Controller->GetPlayer())) {
				m_ParentBox->SetEnabled(false);
				m_ParentBox->SetVisible(false);
				m_PickerEnabled = PickerState::Disabled;
			}
		} else if (m_PickerEnabled == PickerState::Enabled) {
			m_ParentBox->SetEnabled(true);
			m_ParentBox->SetVisible(true);
		} else if (m_PickerEnabled == PickerState::Disabled) {
			m_ParentBox->SetEnabled(false);
			m_ParentBox->SetVisible(false);
			m_PopupBox->SetVisible(false);
		}

		// Quit now if we aren't enabled
		if (m_PickerEnabled != PickerState::Enabled && m_PickerEnabled != PickerState::Enabling) {
			return;
		}

		/////////////////////////////////////////////////////
		// Mouse cursor logic

		int mouseX;
		int mouseY;
		m_GUIInput->GetMousePosition(&mouseX, &mouseY);
		m_CursorPos.SetXY(mouseX, mouseY);

		/////////////////////////////////////////////
		// Repeating input logic

		bool pressLeft = m_Controller->IsState(PRESS_LEFT);
		bool pressRight = m_Controller->IsState(PRESS_RIGHT);
		bool pressUp = m_Controller->IsState(PRESS_UP) || m_Controller->IsState(SCROLL_UP);
		bool pressDown = m_Controller->IsState(PRESS_DOWN) || m_Controller->IsState(SCROLL_DOWN);

		// If no direction is held down, then cancel the repeating
		if (!(/*m_Controller->IsState(MOVE_RIGHT) || m_Controller->IsState(MOVE_LEFT) || */m_Controller->IsState(MOVE_UP) || m_Controller->IsState(MOVE_DOWN))) {
			m_RepeatStartTimer.Reset();
			m_RepeatTimer.Reset();
		}

		// Check if any direction has been held for the starting amount of time to get into repeat mode
		if (m_RepeatStartTimer.IsPastRealMS(200)) {
			// Check for the repeat interval
			if (m_RepeatTimer.IsPastRealMS(75)) {
				/* L-R Not needed for picker
				if (m_Controller->IsState(MOVE_RIGHT))
					pressRight = true;
				else if (m_Controller->IsState(MOVE_LEFT))
					pressLeft = true;
				*/
				if (m_Controller->IsState(MOVE_UP)) {
					pressUp = true;
				} else if (m_Controller->IsState(MOVE_DOWN)) {
					pressDown = true;
				}

				m_RepeatTimer.Reset();
			}
		}

		/////////////////////////////////////////////
		// Change focus as the user directs

		if (pressRight) {
			m_PickerFocus++;

			// Went too far
			if (m_PickerFocus >= PickerFocus::FocusCount) {
				m_PickerFocus = PickerFocus::FocusCount - 1;
				// Only play sound when the picker is completely deployed
				if (m_PickerEnabled == PickerState::Enabled) { g_GUISound.UserErrorSound()->Play(m_Controller->GetPlayer()); }
			} else {
				m_FocusChange = 1;
			}
		} else if (pressLeft) {
			m_PickerFocus--;

			// Went too far
			if (m_PickerFocus < 0) {
				m_PickerFocus = 0;
				// Only play sound when the picker is completely deployed
				if (m_PickerEnabled == PickerState::Enabled) { g_GUISound.UserErrorSound()->Play(m_Controller->GetPlayer()); }
			} else {
				m_FocusChange = -1;
			}
		}

		// Play focus change sound, if applicable
		if (m_FocusChange && m_PickerEnabled != PickerState::Enabling) { g_GUISound.FocusChangeSound()->Play(m_Controller->GetPlayer()); }

		/////////////////////////////////////////
		// GROUPS LIST focus

		if (m_PickerFocus == PickerFocus::Groups) {
			if (m_FocusChange) {
				m_GroupsList->SetFocus();
				// Select the top one in the item list if none is already selected
				if (!m_GroupsList->GetItemList()->empty() && m_GroupsList->GetSelectedIndex() < 0) {
					m_GroupsList->SetSelectedIndex(m_SelectedGroupIndex = 0);
				} else {
					// Sync our index with the one already selected in the list
					m_SelectedGroupIndex = m_GroupsList->GetSelectedIndex();
					m_GroupsList->ScrollToSelected();
				}
				m_FocusChange = 0;
			}

			int listSize = m_GroupsList->GetItemList()->size();
			if (pressDown) {
				m_SelectedGroupIndex++;
				// Loop around
				if (m_SelectedGroupIndex >= listSize) { m_SelectedGroupIndex = 0; }

				m_GroupsList->SetSelectedIndex(m_SelectedGroupIndex);

				const GUIListPanel::Item *pItem = m_GroupsList->GetSelected();
				if (pItem) {
					UpdateObjectsList();
					g_GUISound.ItemChangeSound()->Play(m_Controller->GetPlayer());
				}

				//            g_GUISound.SelectionChangeSound()->Play(m_Controller->GetPlayer());
			} else if (pressUp) {
				m_SelectedGroupIndex--;
				// Loop around
				if (m_SelectedGroupIndex < 0) { m_SelectedGroupIndex = listSize - 1; }

				m_GroupsList->SetSelectedIndex(m_SelectedGroupIndex);

				const GUIListPanel::Item *pItem = m_GroupsList->GetSelected();
				if (pItem) {
					UpdateObjectsList();
					g_GUISound.ItemChangeSound()->Play(m_Controller->GetPlayer());
				}

				//            g_GUISound.SelectionChangeSound()->Play(m_Controller->GetPlayer());
			}

			// Move cursor to the object list if button is pressed on a groups list item
			if (m_Controller->IsState(PRESS_FACEBUTTON)) {
				const GUIListPanel::Item *pItem = m_GroupsList->GetItem(m_SelectedGroupIndex);
				if (pItem) {
					UpdateObjectsList();
					m_PickerFocus = PickerFocus::Objects;
					m_FocusChange = 1;
				}
			}
		}

		/////////////////////////////////////////
		// OBJECTS LIST focus

		else if (m_PickerFocus == PickerFocus::Objects) {
			// Changed to the list, so select the top one in the item list
			if (m_FocusChange) {
				m_ObjectsList->SetFocus();
				if (!m_ObjectsList->GetItemList()->empty() && m_ObjectsList->GetSelectedIndex() < 0) {
					m_ObjectsList->SetSelectedIndex(m_SelectedObjectIndex = 0);
				} else {
					// Sync our index with the one already selected in the list
					m_SelectedObjectIndex = m_ObjectsList->GetSelectedIndex();
					m_ObjectsList->ScrollToSelected();
				}

				// Set the picked object to be the one now selected
				const GUIListPanel::Item *pItem = m_ObjectsList->GetSelected();
				if (pItem) { m_PickedObject = dynamic_cast<const SceneObject *>(pItem->m_pEntity); }

				m_FocusChange = 0;
			}

			int listSize = m_ObjectsList->GetItemList()->size();
			if (pressDown) {
				m_SelectedObjectIndex++;
				// Loop around
				if (m_SelectedObjectIndex >= listSize) { m_SelectedObjectIndex = 0; }

				m_ObjectsList->SetSelectedIndex(m_SelectedObjectIndex);
				// Report the newly selected item as being 'picked', but don't close the picker
				const GUIListPanel::Item *pItem = m_ObjectsList->GetSelected();
				if (pItem) { m_PickedObject = dynamic_cast<const SceneObject *>(pItem->m_pEntity); }
				g_GUISound.SelectionChangeSound()->Play(m_Controller->GetPlayer());
			} else if (pressUp) {
				m_SelectedObjectIndex--;
				// Loop around
				if (m_SelectedObjectIndex < 0) { m_SelectedObjectIndex = listSize - 1; }

				m_ObjectsList->SetSelectedIndex(m_SelectedObjectIndex);
				// Report the newly selected item as being 'picked', but don't close the picker
				const GUIListPanel::Item *pItem = m_ObjectsList->GetSelected();
				if (pItem) { m_PickedObject = dynamic_cast<const SceneObject *>(pItem->m_pEntity); }

				g_GUISound.SelectionChangeSound()->Play(m_Controller->GetPlayer());
			}

			// Get handle to the currently selected item, if any
			GUIListPanel::Item *pItem = m_ObjectsList->GetSelected();

			// Show popup info box next to selected item, but only if it has a description
			string description = "";
			// Get it from the regular Entity preset
			if (pItem && pItem->m_pEntity && !pItem->m_pEntity->GetDescription().empty()) {
				description = pItem->m_pEntity->GetDescription();
			} else if (pItem && pItem->m_ExtraIndex >= 0) {
				// Show popup info box next to selected module item, but only if it has a description
				const DataModule *pModule = g_PresetMan.GetDataModule(pItem->m_ExtraIndex);
				if (pModule && !pModule->GetDescription().empty()) { description = pModule->GetDescription(); }
			}

			// Now show the description, if we have any
			if (!description.empty())
			{
				// Show the popup box with the hovered item's description
				m_PopupBox->SetVisible(true);
				// Need to add an offset to make it look better and not have the cursor obscure text
				m_PopupBox->SetPositionAbs(m_ObjectsList->GetXPos() - m_PopupBox->GetWidth() + 4, m_ObjectsList->GetYPos() + m_ObjectsList->GetStackHeight(pItem) - m_ObjectsList->GetScrollVerticalValue());
				// Make sure the popup box doesn't drop out of sight
				if (m_PopupBox->GetYPos() + m_PopupBox->GetHeight() > m_ParentBox->GetHeight()) { m_PopupBox->SetPositionAbs(m_PopupBox->GetXPos(), m_ParentBox->GetHeight() - m_PopupBox->GetHeight()); }

				m_PopupText->SetHAlignment(GUIFont::Right);
				m_PopupText->SetText(description);
				// Resize the box height to fit the text
				int newHeight = m_PopupText->ResizeHeightToFit();
				m_PopupBox->Resize(m_PopupBox->GetWidth(), newHeight + 10);
			}

			// Fire button picks the object and deactivates the picker GUI
			if (m_PickerEnabled == PickerState::Enabled && m_Controller->IsState(PRESS_FACEBUTTON)) {
				pItem = m_ObjectsList->GetSelected();
				// User pressed on a module group item; toggle its expansion!
				if (pItem && pItem->m_ExtraIndex >= 0) {
					// Make appropriate sound
					if (!m_ExpandedModules[pItem->m_ExtraIndex]) {
						g_GUISound.ItemChangeSound()->Play(m_Controller->GetPlayer());
					} else {
						// Different, maybe?
						g_GUISound.ItemChangeSound()->Play(m_Controller->GetPlayer());
					}
					// Toggle the expansion of the module group item's items below
					m_ExpandedModules[pItem->m_ExtraIndex] = !m_ExpandedModules[pItem->m_ExtraIndex];
					// Re-populate the item list with the new module expansion configuration
					UpdateObjectsList(false);
				} else if (pItem) {
					// Selection of a regular item in the list
					// User has made final selection, so close the Picker
					if (m_PickedObject = dynamic_cast<const SceneObject *>(pItem->m_pEntity)) {
						g_GUISound.ObjectPickedSound()->Play(m_Controller->GetPlayer());
						SetEnabled(false);
					}
				}
			}
		}

		// Right click, or pie menu press close the menu
		if (m_Controller->IsState(PRESS_SECONDARY)) {
			const GUIListPanel::Item *pItem = m_ObjectsList->GetSelected();
			if (pItem) {
				// User has made final selection, so close the Picker
				if (m_PickedObject = dynamic_cast<const SceneObject *>(pItem->m_pEntity)) {
					g_GUISound.ObjectPickedSound()->Play(m_Controller->GetPlayer());
					SetEnabled(false);
				}
			}
		}

		//////////////////////////////////////////
		// Update the ControlManager

		m_GUIController->Update();

		////////////////////////////////////////////////////////
		// Handle events for mouse input on the controls

		GUIEvent anEvent;
		while (m_GUIController->GetEvent(&anEvent)) {
			// If we're not supposed to have mouse control, then ignore these messages
			if (!m_Controller->IsMouseControlled()) {
				break;
			}

			if (anEvent.GetType() == GUIEvent::Command) {
				//
			} else if (anEvent.GetType() == GUIEvent::Notification) {

				//////////////////////////////////////////
				// Clicks on the Groups List

				if(anEvent.GetControl() == m_GroupsList) {
					// Regular click
					if(anEvent.GetMsg() == GUIListBox::MouseDown) {
						const GUIListPanel::Item *pItem = m_GroupsList->GetSelected();
						if (pItem) {
							m_SelectedGroupIndex = m_GroupsList->GetSelectedIndex();
							UpdateObjectsList();
							g_GUISound.ItemChangeSound()->Play(m_Controller->GetPlayer());
						} else {
							// Undo the click deselection if nothing was selected
							m_GroupsList->SetSelectedIndex(m_SelectedGroupIndex);
						}
						m_PickerFocus = PickerFocus::Groups;
					}
					// Mouse moved over the panel, show the popup with item description
					else if(anEvent.GetMsg() == GUIListBox::MouseMove) {
						// Mouse is moving within the list's items, so make it focus on the list
	//                    if (m_GroupsList->GetItem(m_CursorPos.m_X, m_CursorPos.m_Y))
	//                    {
							m_GroupsList->SetFocus();
							m_PickerFocus = PickerFocus::Groups;
	//                    }
						// Deselect the selection if we're fooling around with the mouse cursor outside any items, but still int he list area
	//                    else if (m_PickerFocus == GROUPS)
	//                        m_SelectedGroupIndex = -1;
					}
				}

				///////////////////////////////////////////////
				// Clicks on the Objects List

				else if (anEvent.GetControl() == m_ObjectsList) {
					/*
					// Something was just selected/highlighted, so update the selection index to the new selected index
					if(anEvent.GetMsg() == GUIListBox::Select)
					{
						if (m_SelectedObjectIndex != m_ObjectsList->GetSelectedIndex())
							g_GUISound.SelectionChangeSound()->Play(m_Controller->GetPlayer());
						m_SelectedObjectIndex = m_ObjectsList->GetSelectedIndex();
					}
					// MouseDown, picked something
					else*/ if (anEvent.GetMsg() == GUIListBox::MouseDown) {
						const GUIListPanel::Item *pItem = m_ObjectsList->GetSelected();
						m_ObjectsList->ScrollToSelected();

						// If a module group list item, toggle its expansion and update the list
						if (pItem && pItem->m_ExtraIndex >= 0) {
							// Make appropriate sound
							if (!m_ExpandedModules[pItem->m_ExtraIndex]) {
								g_GUISound.ItemChangeSound()->Play(m_Controller->GetPlayer());
								// Different, maybe?
							} else {
								g_GUISound.ItemChangeSound()->Play(m_Controller->GetPlayer());
							}
							// Toggle the expansion of the module group item's items below
							m_ExpandedModules[pItem->m_ExtraIndex] = !m_ExpandedModules[pItem->m_ExtraIndex];
							// Re-populate the item list with the new module expansion configuration
							UpdateObjectsList(false);
						}
						// Only add if there's an entity attached to the list item
						else if (pItem && pItem->m_pEntity) {
							m_SelectedObjectIndex = m_ObjectsList->GetSelectedIndex();
							// User has made a pick, so close the Picker
							if (m_PickedObject = dynamic_cast<const SceneObject *>(pItem->m_pEntity)) {
								g_GUISound.ObjectPickedSound()->Play(m_Controller->GetPlayer());
								SetEnabled(false);
							}
						}
						// Undo the click deselection if nothing was selected
	//                    else
	//                        m_ObjectsList->SetSelectedIndex(m_SelectedObjectIndex);

						m_PickerFocus = PickerFocus::Objects;
					}
					// Mouse moved over the panel, show the popup with item description
					else if (anEvent.GetMsg() == GUIListBox::MouseMove) {
						// Mouse is moving within the list, so make it focus on the list
						m_ObjectsList->SetFocus();
						m_PickerFocus = PickerFocus::Objects;

						// See if it's hovering over any item
						const GUIListPanel::Item *pItem = m_ObjectsList->GetItem(m_CursorPos.m_X, m_CursorPos.m_Y);
						if (pItem) {
							// Don't let mouse movement change the index if it's still hovering inside the same item.
							// This is to avoid erratic selection cursor if using both mouse and keyboard to work the menu
							if (m_LastHoveredMouseIndex != pItem->m_ID) {
								m_LastHoveredMouseIndex = pItem->m_ID;

								// Play select sound if new index
								if (m_SelectedObjectIndex != pItem->m_ID) { g_GUISound.SelectionChangeSound()->Play(m_Controller->GetPlayer()); }
								// Update the selection in both the GUI control and our menu
								m_ObjectsList->SetSelectedIndex(m_SelectedObjectIndex = pItem->m_ID);
							}
						}
						// Deselect the selection if we're fooling around with the mouse cursor outside any items, but still int he list area
	//                    else if (m_PickerFocus == OBJECTS)
	//                        m_SelectedObjectIndex = -1;
	//                        m_pShopList->SetSelectedIndex(m_ListItemIndex);
	//                        m_CategoryItemIndex[m_MenuCategory] = m_ListItemIndex = -1;
					}
				}
			}

			// If clicked outside the picker, then close the picker GUI
			if (anEvent.GetMsg() == GUIListBox::Click && m_PickerEnabled == PickerState::Enabled && m_CursorPos.m_X < m_ParentBox->GetXPos()) {
				const GUIListPanel::Item *pItem = m_ObjectsList->GetSelected();
				if (pItem) {
					// User has made final selection, so close the Picker
					if (m_PickedObject = dynamic_cast<const SceneObject *>(pItem->m_pEntity)) {
						g_GUISound.ObjectPickedSound()->Play(m_Controller->GetPlayer());
						SetEnabled(false);
					}
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::Draw(BITMAP *drawBitmap) const {
		AllegroScreen drawScreen(drawBitmap);
		m_GUIController->Draw(&drawScreen);

		// Draw the cursor on top of everything
		if (IsEnabled() && m_Controller->IsMouseControlled()) { draw_sprite(drawBitmap, s_Cursor, m_CursorPos.GetFloorIntX(), m_CursorPos.GetFloorIntY()); }
	}
}