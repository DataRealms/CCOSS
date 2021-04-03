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
		m_GUIControlManager = nullptr;
		m_ParentBox = nullptr;
		m_PopupBox = nullptr;
		m_PopupText = nullptr;
		m_GroupsList = nullptr;
		m_ObjectsList = nullptr;
		m_Controller = nullptr;
		m_PickerState = PickerState::Disabled;
		m_PickerFocus = PickerFocus::GroupList;
		m_OpenCloseSpeed = 0.3F;
		m_ModuleSpaceID = -1;
		m_ShowType.clear();
		m_NativeTechModuleID = 0;
		m_ForeignCostMult = 4.0F;
		m_ShownGroupIndex = 0;
		m_SelectedGroupIndex = 0;
		m_SelectedObjectIndex = 0;
		m_PickedObject = nullptr;
		m_RepeatStartTimer.Reset();
		m_RepeatTimer.Reset();

		m_ExpandedModules.resize(g_PresetMan.GetTotalModuleCount());
		std::fill(m_ExpandedModules.begin(), m_ExpandedModules.end(), false);
		m_ExpandedModules.at(0) = true; // Base.rte is always expanded
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ObjectPickerGUI::Create(Controller *controller, int whichModuleSpace, const std::string_view &onlyOfType) {
		RTEAssert(controller, "No controller sent to ObjectPickerGUI on creation!");
		m_Controller = controller;

		if (!m_GUIScreen) { m_GUIScreen = std::make_unique<AllegroScreen>(g_FrameMan.GetBackBuffer8()); }
		if (!m_GUIInput) { m_GUIInput = std::make_unique<AllegroInput>(controller->GetPlayer()); }
		if (!m_GUIControlManager) { m_GUIControlManager = std::make_unique<GUIControlManager>(); }
		RTEAssert(m_GUIControlManager->Create(m_GUIScreen.get(), m_GUIInput.get(), "Base.rte/GUIs/Skins/Base"), "Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Base");

		m_GUIControlManager->Load("Base.rte/GUIs/ObjectPickerGUI.ini");
		m_GUIControlManager->EnableMouse(controller->IsMouseControlled());

		if (!s_Cursor) {
			ContentFile cursorFile("Base.rte/GUIs/Skins/Cursor.png");
			s_Cursor = cursorFile.GetAsBitmap();
		}

		if (g_FrameMan.IsInMultiplayerMode()) {
			dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("base"))->SetSize(g_FrameMan.GetPlayerFrameBufferWidth(controller->GetPlayer()), g_FrameMan.GetPlayerFrameBufferHeight(controller->GetPlayer()));
		} else {
			dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("base"))->SetSize(g_FrameMan.GetResX(), g_FrameMan.GetResY());
		}

		if (!m_ParentBox) { m_ParentBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("PickerGUIBox")); }
		m_ParentBox->SetPositionAbs(g_FrameMan.GetPlayerFrameBufferWidth(m_Controller->GetPlayer()), 0);
		m_ParentBox->SetEnabled(false);
		m_ParentBox->SetVisible(false);
		m_GroupsList = dynamic_cast<GUIListBox *>(m_GUIControlManager->GetControl("GroupsLB"));
		m_GroupsList->SetAlternateDrawMode(false);
		m_GroupsList->SetMultiSelect(false);
		m_ObjectsList = dynamic_cast<GUIListBox *>(m_GUIControlManager->GetControl("ObjectsLB"));
		m_ObjectsList->SetAlternateDrawMode(true);
		m_ObjectsList->SetMultiSelect(false);

		int stretchAmount = g_FrameMan.IsInMultiplayerMode() ? (g_FrameMan.GetPlayerFrameBufferHeight(m_Controller->GetPlayer()) - m_ParentBox->GetHeight()) : (g_FrameMan.GetPlayerScreenHeight() - m_ParentBox->GetHeight());
		if (stretchAmount != 0) {
			m_ParentBox->SetSize(m_ParentBox->GetWidth(), m_ParentBox->GetHeight() + stretchAmount);
			m_GroupsList->SetSize(m_GroupsList->GetWidth(), m_GroupsList->GetHeight() + stretchAmount);
			m_ObjectsList->SetSize(m_ObjectsList->GetWidth(), m_ObjectsList->GetHeight() + stretchAmount);
		}

		if (!m_PopupBox) {
			m_PopupBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BuyGUIPopup"));
			m_PopupText = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("PopupText"));
			m_PopupText->SetFont(m_GUIControlManager->GetSkin()->GetFont("smallfont.png"));

			// Never enable the popup box because it steals focus and causes other windows to think the cursor left them
			m_PopupBox->SetEnabled(false);
			m_PopupBox->SetVisible(false);
		}

		SetModuleSpace(whichModuleSpace);
		ShowOnlyType(onlyOfType);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::SetEnabled(bool enable) {
		if (enable && m_PickerState != PickerState::Enabled && m_PickerState != PickerState::Enabling) {
			m_PickerState = PickerState::Enabling;
			g_UInputMan.TrapMousePos(false, m_Controller->GetPlayer());
			m_CursorPos.SetXY(static_cast<float>(g_FrameMan.GetPlayerFrameBufferWidth(m_Controller->GetPlayer()) / 2), static_cast<float>(g_FrameMan.GetPlayerFrameBufferHeight(m_Controller->GetPlayer()) / 2));
			g_UInputMan.SetMousePos(m_CursorPos, m_Controller->GetPlayer());

			SetListFocus(m_ObjectsList->GetItemList()->empty() ? PickerFocus::GroupList : PickerFocus::ObjectList);

			m_RepeatStartTimer.Reset();
			m_RepeatTimer.Reset();

			g_GUISound.EnterMenuSound()->Play(m_Controller->GetPlayer());
		} else if (!enable && m_PickerState != PickerState::Disabled && m_PickerState != PickerState::Disabling) {
			m_PickerState = PickerState::Disabling;
			g_UInputMan.TrapMousePos(true, m_Controller->GetPlayer());
			g_GUISound.ExitMenuSound()->Play(m_Controller->GetPlayer());
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::SetNativeTechModule(int whichModule) {
		if (whichModule >= 0 && whichModule < g_PresetMan.GetTotalModuleCount()) {
			m_NativeTechModuleID = whichModule;
			if (m_NativeTechModuleID > 0) { SetObjectsListModuleGroupExpanded(m_NativeTechModuleID); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ObjectPickerGUI::SetListFocus(PickerFocus listToFocusOn) {
		if (listToFocusOn == m_PickerFocus) {
			return false;
		}
		if (listToFocusOn == PickerFocus::GroupList) {
			m_PickerFocus = PickerFocus::GroupList;
			m_GroupsList->SetFocus();
			m_GroupsList->ScrollToSelected();
		} else if (listToFocusOn == PickerFocus::ObjectList) {
			m_PickerFocus = PickerFocus::ObjectList;
			m_ObjectsList->SetFocus();
			SelectObjectByIndex((!m_ObjectsList->GetItemList()->empty() && m_ObjectsList->GetSelectedIndex() > 0) ? m_ObjectsList->GetSelectedIndex() : 0);
			m_ObjectsList->ScrollToSelected();
		}
		g_GUISound.FocusChangeSound()->Play(m_Controller->GetPlayer());
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ObjectPickerGUI::SelectGroupByName(const std::string_view &groupName) {
		for (const GUIListPanel::Item *groupListItem : *m_GroupsList->GetItemList()) {
			if (groupListItem->m_Name == groupName) {
				SelectGroupByIndex(groupListItem->m_ID);
				SetListFocus(PickerFocus::ObjectList);
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::SelectGroupByIndex(int groupIndex, bool updateObjectsList) {
		m_SelectedGroupIndex = (groupIndex < 0) ? m_ShownGroupIndex : groupIndex;
		m_GroupsList->SetSelectedIndex(m_SelectedGroupIndex);

		if (updateObjectsList) {
			m_ShownGroupIndex = m_SelectedGroupIndex;
			UpdateObjectsList();
			g_GUISound.ItemChangeSound()->Play(m_Controller->GetPlayer());
		} else {
			g_GUISound.SelectionChangeSound()->Play(m_Controller->GetPlayer());
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::SelectNextOrPrevGroup(bool selectPrev) {
		int groupIndex = m_SelectedGroupIndex;
		if (selectPrev) {
			groupIndex--;
			if (groupIndex < 0) { groupIndex = m_GroupsList->GetItemList()->size() - 1; }
		} else {
			groupIndex++;
			if (groupIndex >= m_GroupsList->GetItemList()->size()) { groupIndex = 0; }
		}
		SelectGroupByIndex(groupIndex);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::UpdateGroupsList() {
		m_GroupsList->ClearList();
		bool showAssemblySchemes = dynamic_cast<EditorActivity *>(g_ActivityMan.GetActivity());

		std::list<std::string> groupList;
		g_PresetMan.GetModuleSpaceGroups(groupList, m_ModuleSpaceID, m_ShowType);

		for (const std::string &groupListEntry : groupList) {
			std::list<Entity *> objectList;
			g_PresetMan.GetAllOfGroupInModuleSpace(objectList, groupListEntry, m_ShowType, m_ModuleSpaceID);

			bool onlyAssembliesInGroup = true;
			bool onlyAssemblySchemesInGroup = true;
			bool hasObjectsToShow = false;

			for (Entity *objectListEntry : objectList) {
				if (!dynamic_cast<BunkerAssembly *>(objectListEntry)) { onlyAssembliesInGroup = false; }
				if (!dynamic_cast<BunkerAssemblyScheme *>(objectListEntry)) { onlyAssemblySchemesInGroup = false; }

				const SceneObject *sceneObject = dynamic_cast<SceneObject *>(objectListEntry);
				if (sceneObject && sceneObject->IsBuyable()) {
					hasObjectsToShow = true;
					break;
				}
			}
			// If this group is in the SettingsMan list of always visible assembly groups, then force the onlyAssembliesInGroup flag off so this group is always shown
			if (onlyAssembliesInGroup) {
				for (const std::string &assemblyGroup : g_SettingsMan.GetVisibleAssemblyGroupsList()) {
					if (groupListEntry == assemblyGroup) {
						onlyAssembliesInGroup = false;
						break;
					}
				}
			}
			if (!objectList.empty() && hasObjectsToShow && (!onlyAssembliesInGroup || groupListEntry == "Assemblies") && (!onlyAssemblySchemesInGroup || showAssemblySchemes)) { m_GroupsList->AddItem(groupListEntry); }
		}

		if (const GUIListPanel::Item *listItem = m_GroupsList->GetItem(0)) {
			m_GroupsList->ScrollToTop();
			m_SelectedGroupIndex = 0;
			m_GroupsList->SetSelectedIndex(m_SelectedGroupIndex);
			UpdateObjectsList();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const SceneObject * ObjectPickerGUI::GetSelectedObject() {
		if (const GUIListPanel::Item *selectedItem = m_ObjectsList->GetSelected()) {
			return dynamic_cast<const SceneObject *>(selectedItem->m_pEntity);
		}
		return nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::SelectObjectByIndex(int objectIndex, bool playSelectionSound) {
		m_SelectedObjectIndex = objectIndex;
		m_ObjectsList->SetSelectedIndex(m_SelectedObjectIndex);
		if (playSelectionSound) { g_GUISound.SelectionChangeSound()->Play(m_Controller->GetPlayer()); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::SelectNextOrPrevObject(bool getPrev) {
		int objectIndex = m_SelectedObjectIndex;
		if (getPrev) {
			objectIndex--;
			if (objectIndex < 0) { objectIndex = m_ObjectsList->GetItemList()->size() - 1; }
		} else {
			objectIndex++;
			if (objectIndex >= m_ObjectsList->GetItemList()->size()) { objectIndex = 0; }
		}
		SelectObjectByIndex(objectIndex);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::AddObjectsListModuleGroup(int moduleID) {
		const DataModule *dataModule = g_PresetMan.GetDataModule(moduleID);
		std::string moduleName = dataModule->GetFriendlyName();
		std::transform(moduleName.begin(), moduleName.end(), moduleName.begin(), ::toupper);
		GUIBitmap *dataModuleIcon = dataModule->GetIcon() ? new AllegroBitmap(dataModule->GetIcon()) : nullptr;
		m_ObjectsList->AddItem(moduleName, m_ExpandedModules.at(moduleID) ? "-" : "+", dataModuleIcon, nullptr, moduleID);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::SetObjectsListModuleGroupExpanded(int moduleID, bool expanded) {
		if (moduleID > 0 && moduleID < m_ExpandedModules.size()) {
			m_ExpandedModules.at(moduleID) = expanded;
			UpdateObjectsList(false);
		} else {
			std::fill(m_ExpandedModules.begin(), m_ExpandedModules.end(), expanded);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::ToggleObjectsListModuleGroupExpansion(int moduleID) {
		if (moduleID > 0 && moduleID < m_ExpandedModules.size()) {
			m_ExpandedModules.at(moduleID) = !m_ExpandedModules.at(moduleID);
			UpdateObjectsList(false);
			g_GUISound.ItemChangeSound()->Play(m_Controller->GetPlayer());
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::ShowDescriptionPopupBox() {
		std::string description = "";
		GUIListPanel::Item *objectListItem = m_ObjectsList->GetSelected();
		if (objectListItem && objectListItem->m_pEntity && !objectListItem->m_pEntity->GetDescription().empty()) {
			description = objectListItem->m_pEntity->GetDescription();
		} else if (objectListItem && objectListItem->m_ExtraIndex >= 0) {
			const DataModule *dataModule = g_PresetMan.GetDataModule(objectListItem->m_ExtraIndex);
			if (dataModule && !dataModule->GetDescription().empty()) { description = dataModule->GetDescription(); }
		}
		if (!description.empty()) {
			m_PopupBox->SetEnabled(false);
			m_PopupBox->SetVisible(true);
			m_PopupBox->SetPositionAbs(m_ObjectsList->GetXPos() - m_PopupBox->GetWidth() + 4, m_ObjectsList->GetYPos() + m_ObjectsList->GetStackHeight(objectListItem) - m_ObjectsList->GetScrollVerticalValue());
			if (m_PopupBox->GetYPos() + m_PopupBox->GetHeight() > m_ParentBox->GetHeight()) { m_PopupBox->SetPositionAbs(m_PopupBox->GetXPos(), m_ParentBox->GetHeight() - m_PopupBox->GetHeight()); }

			m_PopupText->SetHAlignment(GUIFont::Left);
			m_PopupText->SetText(description);
			m_PopupBox->Resize(m_PopupBox->GetWidth(), m_PopupText->ResizeHeightToFit() + 10);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::UpdateObjectsList(bool selectTop) {
		m_ObjectsList->ClearList();
		std::vector<std::list<Entity *>> moduleList(g_PresetMan.GetTotalModuleCount(), std::list<Entity *>());

		if (const GUIListPanel::Item *groupListItem = m_GroupsList->GetSelected()) {
			if (m_ModuleSpaceID < 0) {
				if (g_SettingsMan.ShowForeignItems() || m_NativeTechModuleID <= 0) {
					for (int moduleID = 0; moduleID < moduleList.size(); ++moduleID) {
						g_PresetMan.GetAllOfGroup(moduleList.at(moduleID), groupListItem->m_Name, m_ShowType, moduleID);
					}
				} else {
					for (int moduleID = 0; moduleID < moduleList.size(); ++moduleID) {
						if (moduleID == 0 || moduleID == m_NativeTechModuleID) { g_PresetMan.GetAllOfGroup(moduleList.at(moduleID), groupListItem->m_Name, m_ShowType, moduleID); }
					}
				}
			} else {
				for (int moduleID = 0; moduleID < g_PresetMan.GetOfficialModuleCount() && moduleID < m_ModuleSpaceID; ++moduleID) {
					g_PresetMan.GetAllOfGroup(moduleList.at(moduleID), groupListItem->m_Name, m_ShowType, moduleID);
				}
				g_PresetMan.GetAllOfGroup(moduleList.at(m_ModuleSpaceID), groupListItem->m_Name, m_ShowType, m_ModuleSpaceID);
			}
		}

		for (int moduleID = 0; moduleID < moduleList.size(); ++moduleID) {
			if (moduleList.at(moduleID).empty()) {
				continue;
			}
			std::list<SceneObject *> objectList;
			for (Entity *moduleListEntryEntity : moduleList.at(moduleID)) {
				SceneObject *sceneObject = dynamic_cast<SceneObject *>(moduleListEntryEntity);
				if (sceneObject && sceneObject->IsBuyable()) { objectList.emplace_back(sceneObject); }
			}
			if (!objectList.empty()) {
				if (moduleID != 0) { AddObjectsListModuleGroup(moduleID); }
				if (moduleID == 0 || m_ExpandedModules.at(moduleID)) {
					for (SceneObject *objectListEntry : objectList) {
						GUIBitmap *objectIcon = new AllegroBitmap(objectListEntry->GetGraphicalIcon());
						m_ObjectsList->AddItem(objectListEntry->GetPresetName(), objectListEntry->GetGoldValueString(m_NativeTechModuleID, m_ForeignCostMult), objectIcon, objectListEntry);
					}
				}
			}
		}
		if (selectTop) {
			m_ObjectsList->ScrollToTop();
			SelectObjectByIndex(0, false);
		} else {
			SelectObjectByIndex(m_SelectedObjectIndex, false);
			m_ObjectsList->ScrollToSelected();
		}
		if (const GUIListPanel::Item *selectedItem = m_ObjectsList->GetSelected()) { m_PickedObject = dynamic_cast<const SceneObject *>(selectedItem->m_pEntity); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::Update() {
		m_PopupBox->SetVisible(false);

		if (m_PickerState != PickerState::Enabled && m_PickerState != PickerState::Disabled) { AnimateOpenClose(); }
		if (m_PickerState == PickerState::Enabled || m_PickerState == PickerState::Enabling) { m_GUIControlManager->Update(true); }
		if (m_PickerState == PickerState::Enabled) {
			m_GUIControlManager->EnableMouse(m_Controller->IsMouseControlled());

			m_PickedObject = nullptr;
			if (HandleInput()) {
				if (const GUIListPanel::Item *selectedItem = m_ObjectsList->GetSelected()) {
					m_PickedObject = dynamic_cast<const SceneObject *>(selectedItem->m_pEntity);
					if (m_PickedObject) {
						g_GUISound.ObjectPickedSound()->Play(m_Controller->GetPlayer());
						SetEnabled(false);
					}
				}
				return;
			}
			if (m_PickerFocus == PickerFocus::ObjectList) { ShowDescriptionPopupBox(); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ObjectPickerGUI::HandleInput() {
		bool objectPickedOrPickerClosed = false;
		if (m_Controller->IsMouseControlled()) { objectPickedOrPickerClosed = HandleMouseEvents(); }

		if (!objectPickedOrPickerClosed && !m_Controller->IsState(ControlState::PRESS_SECONDARY)) {
			bool pressUp = m_Controller->IsState(ControlState::PRESS_UP) || m_Controller->IsState(ControlState::SCROLL_UP);
			bool pressDown = m_Controller->IsState(ControlState::PRESS_DOWN) || m_Controller->IsState(ControlState::SCROLL_DOWN);

			if (!(m_Controller->IsState(ControlState::MOVE_UP) || m_Controller->IsState(ControlState::MOVE_DOWN))) {
				m_RepeatStartTimer.Reset();
				m_RepeatTimer.Reset();
			}
			if (m_RepeatStartTimer.IsPastRealMS(200) && m_RepeatTimer.IsPastRealMS(70)) {
				if (m_Controller->IsState(ControlState::MOVE_UP)) {
					pressUp = true;
				} else if (m_Controller->IsState(ControlState::MOVE_DOWN)) {
					pressDown = true;
				}
				m_RepeatTimer.Reset();
			}

			if ((m_Controller->IsState(ControlState::PRESS_LEFT) && !SetListFocus(PickerFocus::GroupList)) || (m_Controller->IsState(ControlState::PRESS_RIGHT) && !SetListFocus(PickerFocus::ObjectList))) {
				g_GUISound.UserErrorSound()->Play(m_Controller->GetPlayer());
			}

			if (m_PickerFocus == PickerFocus::GroupList) {
				if (pressDown) {
					SelectNextOrPrevGroup(false);
				} else if (pressUp) {
					SelectNextOrPrevGroup(true);
				} else if (m_Controller->IsState(ControlState::PRESS_FACEBUTTON) && m_GroupsList->GetItem(m_SelectedGroupIndex)) {
					SetListFocus(PickerFocus::ObjectList);
				}
			} else if (m_PickerFocus == PickerFocus::ObjectList) {
				if (pressDown) {
					SelectNextOrPrevObject(false);
					m_PickedObject = GetSelectedObject();
				} else if (pressUp) {
					SelectNextOrPrevObject(true);
					m_PickedObject = GetSelectedObject();
				} else if (m_Controller->IsState(ControlState::PRESS_FACEBUTTON)) {
					if (const GUIListPanel::Item *objectListItem = m_ObjectsList->GetSelected()) {
						if (objectListItem->m_ExtraIndex >= 0) {
							ToggleObjectsListModuleGroupExpansion(objectListItem->m_ExtraIndex);
						} else {
							objectPickedOrPickerClosed = true;
						}
					}
				}
			}
		}

		if (objectPickedOrPickerClosed || m_Controller->IsState(ControlState::PRESS_SECONDARY)) {
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ObjectPickerGUI::HandleMouseEvents() {
		int mouseX;
		int mouseY;
		m_GUIInput->GetMousePosition(&mouseX, &mouseY);
		m_CursorPos.SetXY(static_cast<float>(mouseX), static_cast<float>(mouseY));

		GUIEvent guiEvent;
		while (m_GUIControlManager->GetEvent(&guiEvent)) {
			if (guiEvent.GetType() == GUIEvent::Notification) {
				if (guiEvent.GetControl() == m_GroupsList) {
					if (guiEvent.GetMsg() == GUIListBox::MouseDown) {
						SelectGroupByIndex(m_GroupsList->GetSelectedIndex());
					} else if (guiEvent.GetMsg() == GUIListBox::MouseMove) {
						const GUIListPanel::Item *groupListItem = m_GroupsList->GetItem(m_CursorPos.GetFloorIntX(), m_CursorPos.GetFloorIntY());
						if (groupListItem && m_SelectedGroupIndex != groupListItem->m_ID) { SelectGroupByIndex(groupListItem->m_ID, false); }
					} else if (guiEvent.GetMsg() == GUIListBox::MouseEnter) {
						SetListFocus(PickerFocus::GroupList);
					} else if (guiEvent.GetMsg() == GUIListBox::MouseLeave && m_SelectedGroupIndex != m_ShownGroupIndex) {
						SelectGroupByIndex(m_ShownGroupIndex, false);
					}
				} else if (guiEvent.GetControl() == m_ObjectsList) {
					if (guiEvent.GetMsg() == GUIListBox::MouseDown) {
						m_ObjectsList->ScrollToSelected();
						if (const GUIListPanel::Item *objectListItem = m_ObjectsList->GetSelected()) {
							if (objectListItem->m_ExtraIndex >= 0) {
								ToggleObjectsListModuleGroupExpansion(objectListItem->m_ExtraIndex);
							} else if (objectListItem->m_pEntity) {
								SelectObjectByIndex(m_ObjectsList->GetSelectedIndex());
								return true;
							}
						}
					} else if (guiEvent.GetMsg() == GUIListBox::MouseMove) {
						const GUIListPanel::Item *objectListItem = m_ObjectsList->GetItem(m_CursorPos.GetFloorIntX(), m_CursorPos.GetFloorIntY());
						if (objectListItem && m_SelectedObjectIndex != objectListItem->m_ID) { SelectObjectByIndex(objectListItem->m_ID); }
					} else if (guiEvent.GetMsg() == GUIListBox::MouseEnter) {
						SetListFocus(PickerFocus::ObjectList);
					}
				} else {
					if (guiEvent.GetMsg() == GUIListBox::Click && m_CursorPos.GetFloorIntX() < m_ParentBox->GetXPos()) {
						return true;
					}
				}
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::AnimateOpenClose() {
		if (m_PickerState == PickerState::Enabling) {
			m_ParentBox->SetVisible(true);

			int enabledPos = g_FrameMan.GetPlayerFrameBufferWidth(m_Controller->GetPlayer()) - m_ParentBox->GetWidth();
			float travelCompletionDistance = std::floor(static_cast<float>(enabledPos - m_ParentBox->GetXPos()) * m_OpenCloseSpeed);

			m_ParentBox->SetPositionAbs(m_ParentBox->GetXPos() + static_cast<int>(travelCompletionDistance), 0);
			g_SceneMan.SetScreenOcclusion(Vector(static_cast<float>(m_ParentBox->GetXPos() - g_FrameMan.GetPlayerFrameBufferWidth(m_Controller->GetPlayer())), 0), g_ActivityMan.GetActivity()->ScreenOfPlayer(m_Controller->GetPlayer()));

			if (m_ParentBox->GetXPos() <= enabledPos) {
				m_ParentBox->SetEnabled(true);
				m_PickerState = PickerState::Enabled;
			}
		} else if (m_PickerState == PickerState::Disabling) {
			m_ParentBox->SetEnabled(false);
			m_PopupBox->SetVisible(false);

			int disabledPos = g_FrameMan.GetPlayerFrameBufferWidth(m_Controller->GetPlayer());
			float travelCompletionDistance = std::ceil(static_cast<float>(disabledPos - m_ParentBox->GetXPos()) * m_OpenCloseSpeed);

			m_ParentBox->SetPositionAbs(m_ParentBox->GetXPos() + static_cast<int>(travelCompletionDistance), 0);
			g_SceneMan.SetScreenOcclusion(Vector(static_cast<float>(m_ParentBox->GetXPos() - g_FrameMan.GetPlayerFrameBufferWidth(m_Controller->GetPlayer())), 0), g_ActivityMan.GetActivity()->ScreenOfPlayer(m_Controller->GetPlayer()));

			if (m_ParentBox->GetXPos() >= g_FrameMan.GetPlayerFrameBufferWidth(m_Controller->GetPlayer())) {
				m_ParentBox->SetVisible(false);
				m_PickerState = PickerState::Disabled;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ObjectPickerGUI::Draw(BITMAP *drawBitmap) const {
		AllegroScreen drawScreen(drawBitmap);
		m_GUIControlManager->Draw(&drawScreen);
		if (IsEnabled() && m_Controller->IsMouseControlled()) { draw_sprite(drawBitmap, s_Cursor, m_CursorPos.GetFloorIntX(), m_CursorPos.GetFloorIntY()); }
	}
}