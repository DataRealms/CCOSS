//////////////////////////////////////////////////////////////////////////////////////////
// File:            AssemblyEditorGUI.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation file for the AssemblyEditorGUI class
// Project:         GUI Library
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "AssemblyEditorGUI.h"

#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "GameActivity.h"
#include "SceneEditor.h"
#include "UInputMan.h"

#include "Controller.h"
#include "SceneObject.h"
#include "MOSprite.h"
#include "HeldDevice.h"
#include "HDFirearm.h"
#include "TDExplosive.h"
#include "TerrainObject.h"
#include "AHuman.h"
#include "ACrab.h"
#include "SLTerrain.h"
#include "ObjectPickerGUI.h"
#include "PieMenuGUI.h"
#include "Scene.h"
#include "SettingsMan.h"

using namespace RTE;

#define MAXBRAINPATHCOST 10000
#define BLUEPRINTREVEALRATE 150
#define BLUEPRINTREVEALPAUSE 1500

BITMAP *AssemblyEditorGUI::s_pValidPathDot = 0;
BITMAP *AssemblyEditorGUI::s_pInvalidPathDot = 0;

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this AssemblyEditorGUI, effectively
//                  resetting the members of this abstraction level only.

void AssemblyEditorGUI::Clear()
{
    m_pController = 0;
    m_FeatureSet = ONLOADEDIT;
    m_EditMade = false;
    m_EditorGUIMode = PICKINGOBJECT;
    m_PreviousMode = ADDINGOBJECT;
    m_ModeChanged = true;
    m_BlinkTimer.Reset();
    m_BlinkMode = NOBLINK;
    m_RepeatStartTimer.Reset();
    m_RepeatTimer.Reset();
    m_RevealTimer.Reset();
    m_RevealIndex = 0;
	m_PieMenu = nullptr;
    m_pPicker = 0;
    m_NativeTechModule = 0;
    m_ForeignCostMult = 4.0;
    m_GridSnapping = true;
    m_CursorPos.Reset();
    m_CursorOffset.Reset();
    m_CursorInAir = true;
    m_FacingLeft = false;
    m_PlaceTeam = Activity::TeamOne;
    m_pCurrentObject = 0;
    m_ObjectListOrder = -1;
    m_DrawCurrentObject = true;
    m_pObjectToBlink = 0;
    m_BrainSkyPath.clear();
    m_BrainSkyPathCost = 0;
	m_RequireClearPathToOrbit = true;
	m_pCurrentScheme = 0;
	m_CurrentAssemblyName.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the AssemblyEditorGUI object ready for use.

int AssemblyEditorGUI::Create(Controller *pController, FeatureSets featureSet, int whichModuleSpace, int nativeTechModule, float foreignCostMult)
{
    RTEAssert(pController, "No controller sent to AssemblyEditorGUI on creation!");
    m_pController = pController;

    m_FeatureSet = featureSet;

	if (m_PieMenu) { m_PieMenu = nullptr; }
	m_PieMenu = std::unique_ptr<PieMenuGUI>(dynamic_cast<PieMenuGUI *>(g_PresetMan.GetEntityPreset("PieMenuGUI", "Assembly Editor Pie Menu")->Clone()));
	m_PieMenu->SetMenuController(pController);

    // Update the brain path
    UpdateBrainPath();

    // Allocate and (re)create the Editor GUIs
    if (!m_pPicker)
        m_pPicker = new ObjectPickerGUI();
    else
        m_pPicker->Reset();
    m_pPicker->Create(pController, whichModuleSpace);

    m_NativeTechModule = nativeTechModule;
    m_ForeignCostMult = foreignCostMult;
    // Also apply these to the picker
    m_pPicker->SetNativeTechModule(m_NativeTechModule);
    m_pPicker->SetForeignCostMultiplier(m_ForeignCostMult);

    // Cursor init
    m_CursorPos = g_SceneMan.GetSceneDim() / 2;

    // Set initial focus, category list, and label settings
    m_EditorGUIMode = PICKINGOBJECT;
    m_ModeChanged = true;
    m_pCurrentObject = 0;

    // Reset repeat timers
    m_RepeatStartTimer.Reset();
    m_RepeatTimer.Reset();
    m_RevealTimer.Reset();
    m_RevealTimer.SetRealTimeLimitMS(100);

	//Check if we need to check for a clear path to orbit
	m_RequireClearPathToOrbit = true;
	GameActivity * gameActivity = dynamic_cast<GameActivity *>(g_ActivityMan.GetActivity());
	if (gameActivity)
		m_RequireClearPathToOrbit = gameActivity->GetRequireClearPathToOrbit();
	// Always disable clear path requirement in scene editor
	SceneEditor * editorActivity = dynamic_cast<SceneEditor *>(g_ActivityMan.GetActivity());
	if (editorActivity)
		m_RequireClearPathToOrbit = false;

    // Only load the static dot bitmaps once
    if (!s_pValidPathDot)
    {
        ContentFile dotFile("Base.rte/GUIs/Indicators/PathDotValid.png");
        s_pValidPathDot = dotFile.GetAsBitmap();
        dotFile.SetDataPath("Base.rte/GUIs/Indicators/PathDotInvalid.png");
        s_pInvalidPathDot = dotFile.GetAsBitmap();
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the AssemblyEditorGUI object.

void AssemblyEditorGUI::Destroy()
{
    delete m_pPicker;
    delete m_pCurrentObject;
	delete m_pCurrentScheme;

    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetController
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the controller used by this. The ownership of the controller is
//                  NOT transferred!

void AssemblyEditorGUI::SetController(Controller *pController)
{
    m_pController = pController;
	m_PieMenu->SetMenuController(pController);
    m_pPicker->SetController(pController);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPosOnScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets where on the screen that this GUI is being drawn to. If upper
//                  left corner, then 0, 0. This will affect the way the mouse is positioned
//                  etc.

void AssemblyEditorGUI::SetPosOnScreen(int newPosX, int newPosY)
{
    m_pPicker->SetPosOnScreen(newPosX, newPosY);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetCurrentObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the new Object to be held at the cursor of this Editor. Ownership
//                  IS transferred!

bool AssemblyEditorGUI::SetCurrentObject(SceneObject *pNewObject)
{
    if (m_pCurrentObject == pNewObject)
        return true;

    // Replace the current object with the new one
    delete m_pCurrentObject;
    m_pCurrentObject = pNewObject;

    if (!m_pCurrentObject)
        return false;

    m_pCurrentObject->SetTeam(m_FeatureSet == ONLOADEDIT ? m_PlaceTeam : m_pController->GetTeam());
    m_pCurrentObject->SetPlacedByPlayer(m_pController->GetPlayer());

    // Disable any controller, if an actor
    if (Actor *pActor = dynamic_cast<Actor *>(m_pCurrentObject))
    {
        pActor->GetController()->SetDisabled(true);
        pActor->SetStatus(Actor::INACTIVE);
    }




    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetActivatedPieSlice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets any Pie menu slice command activated last update.

PieSlice::PieSliceIndex AssemblyEditorGUI::GetActivatedPieSlice()
{
    return m_PieMenu->GetPieCommand();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetModuleSpace
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which DataModule space to be picking objects from. If -1, then
//                  let the player pick from all loaded modules.

void AssemblyEditorGUI::SetModuleSpace(int moduleSpaceID)
{
    m_pPicker->SetModuleSpace(moduleSpaceID);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetNativeTechModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which DataModule ID should be treated as the native tech of the
//                  user of this menu.

void AssemblyEditorGUI::SetNativeTechModule(int whichModule)
{
    if (whichModule >= 0 && whichModule < g_PresetMan.GetTotalModuleCount())
    {
        m_NativeTechModule = whichModule;
        m_pPicker->SetNativeTechModule(m_NativeTechModule);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetForeignCostMultiplier
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the multiplier of the cost of any foreign Tech items.

void AssemblyEditorGUI::SetForeignCostMultiplier(float newMultiplier)
{
    m_ForeignCostMult = newMultiplier;
    m_pPicker->SetForeignCostMultiplier(m_ForeignCostMult);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TestBrainResidence
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether the resident brain is currently placed into a valid
//                  location in this scene, based on whether there is a clear path to the
//                  sky above it. This forces the editor into place brain mode with the
//                  current resident brain if the current placement is no bueno. It also
//                  removes the faulty brain from residence in the scene!

bool AssemblyEditorGUI::TestBrainResidence(bool noBrainIsOK)
{
	// Brain is fine, leave it be
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this Menu each frame

void AssemblyEditorGUI::Update()
{
    // Update the user controller
//    m_pController->Update();

	string selectedAssembly = "\nSelected scheme: ";

	if (m_pCurrentScheme)
	{
		std::list<Entity *> assemblies;
		g_PresetMan.GetAllOfGroup(assemblies, m_pCurrentScheme->GetPresetName(), "BunkerAssembly");

		std::stringstream assemblyName;
        assemblyName << m_pCurrentScheme->GetPresetName() << " [ " << assemblies.size() << " ]";
		selectedAssembly += assemblyName.str();
	}

	selectedAssembly += "\nCurrent assembly: " + m_CurrentAssemblyName;

    m_EditMade = false;
    m_pObjectToBlink = 0;
    // Which set of placed objects in the scene we're editing
    int editedSet = Scene::PLACEONLOAD;

    /////////////////////////////////////////////
    // Repeating input logic

    bool pressLeft = m_pController->IsState(PRESS_LEFT);
    bool pressRight = m_pController->IsState(PRESS_RIGHT);
    bool pressUp = m_pController->IsState(PRESS_UP);
    bool pressDown = m_pController->IsState(PRESS_DOWN);

    // If no direciton is held down, then cancel the repeating
    if (!(m_pController->IsState(MOVE_RIGHT) || m_pController->IsState(MOVE_LEFT) || m_pController->IsState(MOVE_UP) || m_pController->IsState(MOVE_DOWN)))
    {
        m_RepeatStartTimer.Reset();
        m_RepeatTimer.Reset();
    }

    // Check if any direction has been held for the starting amount of time to get into repeat mode
    if (m_RepeatStartTimer.IsPastRealMS(200))
    {
        // Check for the repeat interval
        if (m_RepeatTimer.IsPastRealMS(30))
        {
            if (m_pController->IsState(MOVE_RIGHT))
                pressRight = true;
            else if (m_pController->IsState(MOVE_LEFT))
                pressLeft = true;

            if (m_pController->IsState(MOVE_UP))
                pressUp = true;
            else if (m_pController->IsState(MOVE_DOWN))
                pressDown = true;

            m_RepeatTimer.Reset();
        }
    }

    ///////////////////////////////////////////////
    // Analog cursor input

    Vector analogInput;
    if (m_pController->GetAnalogMove().GetMagnitude() > 0.1)
        analogInput = m_pController->GetAnalogMove();
//    else if (m_pController->GetAnalogAim().GetMagnitude() > 0.1)
//        analogInput = m_pController->GetAnalogAim();

    /////////////////////////////////////////////
    // PIE MENU

	m_PieMenu->Update();

    // Show the pie menu only when the secondary button is held down
    if (m_pController->IsState(PRESS_SECONDARY) && m_EditorGUIMode != INACTIVE && m_EditorGUIMode != PICKINGOBJECT) {
		m_PieMenu->SetEnabled(true);
		m_PieMenu->SetPos(m_GridSnapping ? g_SceneMan.SnapPosition(m_CursorPos) : m_CursorPos);

		PieSlice *saveSlice = m_PieMenu->GetPieSliceByType(PieSlice::PieSliceIndex::PSI_SAVE);
		if (saveSlice) {
			saveSlice->SetEnabled(m_pCurrentScheme != nullptr);
			saveSlice->SetDescription(m_pCurrentScheme != nullptr ? "Save Assembly" : "Can't Save Assembly, Scheme Not Selected!");
		}
    }

	if (!m_pController->IsState(PIE_MENU_ACTIVE) || m_EditorGUIMode == INACTIVE || m_EditorGUIMode == PICKINGOBJECT) { m_PieMenu->SetEnabled(false); }

    ///////////////////////////////////////
    // Handle pie menu selections

    if (m_PieMenu->GetPieCommand() != PieSlice::PieSliceIndex::PSI_NONE) {
		if (m_PieMenu->GetPieCommand() == PieSlice::PieSliceIndex::PSI_PICK) {
			m_EditorGUIMode = PICKINGOBJECT;
		} else if (m_PieMenu->GetPieCommand() == PieSlice::PieSliceIndex::PSI_MOVE) {
			m_EditorGUIMode = MOVINGOBJECT;
		} else if (m_PieMenu->GetPieCommand() == PieSlice::PieSliceIndex::PSI_REMOVE) {
			m_EditorGUIMode = DELETINGOBJECT;
		} else if (m_PieMenu->GetPieCommand() == PieSlice::PieSliceIndex::PSI_DONE) {
			m_EditorGUIMode = DONEEDITING;
		}

        UpdateBrainPath();
        m_ModeChanged = true;
    }

    //////////////////////////////////////////
    // Picker logic

    // Enable or disable the picker
    m_pPicker->SetEnabled(m_EditorGUIMode == PICKINGOBJECT);

    // Update the picker GUI
    m_pPicker->Update();

    if (m_EditorGUIMode == PICKINGOBJECT && m_pPicker->ObjectPicked())
    {
        // Assign a copy of the picked object to be the currently held one.
        if (SetCurrentObject(dynamic_cast<SceneObject *>(m_pPicker->ObjectPicked()->Clone())))
        {
            // Set the team
            if (m_FeatureSet != ONLOADEDIT)
                m_pCurrentObject->SetTeam(m_pController->GetTeam());
            // Set the list order to be at the end so new objects are added there
            m_ObjectListOrder = -1;
            // Update the object
            m_pCurrentObject->Update();
            // Update the path to the brain, or clear it if there's none
            UpdateBrainPath();
                
            // If done picking, revert to moving object mode
            if (m_pPicker->DonePicking())
            {
                m_EditorGUIMode = ADDINGOBJECT;
                UpdateBrainPath();
                m_ModeChanged = true;
            }
        }
    }

    if (!m_pPicker->IsVisible())
        g_SceneMan.SetScreenOcclusion(Vector(), g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
    else
        g_FrameMan.SetScreenText("Pick what you want to place next" + selectedAssembly, g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

    /////////////////////////////////////
    // ADDING OBJECT MODE

    if (m_EditorGUIMode == ADDINGOBJECT && !m_PieMenu->IsEnabled())
    {
        if (m_ModeChanged)
        {
            m_ModeChanged = false;
        }
        g_FrameMan.SetScreenText("Click to ADD a new object - Drag for precision" + selectedAssembly, g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));


        m_DrawCurrentObject = true;

        // Trap the mouse cursor
        g_UInputMan.TrapMousePos(true, m_pController->GetPlayer());

        // Move the cursor according to analog or mouse input
        if (!analogInput.IsZero())
        {
            m_CursorPos += analogInput * 8;
            // Re-enable snapping only when the cursor is moved again
            m_GridSnapping = true;
        }
        else if (!m_pController->GetMouseMovement().IsZero())
        {
            m_CursorPos += m_pController->GetMouseMovement();
            // Re-enable snapping only when the cursor is moved again
            m_GridSnapping = true;
        }
        // Digital input?
        else
        {
            if (pressUp)
                m_CursorPos.m_Y -= SCENESNAPSIZE;
            if (pressRight)
                m_CursorPos.m_X += SCENESNAPSIZE;
            if (pressDown)
                m_CursorPos.m_Y += SCENESNAPSIZE;
            if (pressLeft)
                m_CursorPos.m_X -= SCENESNAPSIZE;
            // Re-enable snapping only when the cursor is moved again
            if (pressUp || pressRight || pressDown || pressLeft)
                m_GridSnapping = true;
        }

        // Detect whether the cursor is in the air, or if it's overlapping some terrain
        Vector snappedPos = g_SceneMan.SnapPosition(m_CursorPos, m_GridSnapping);
        m_CursorInAir = g_SceneMan.GetTerrMatter(snappedPos.GetFloorIntX(), snappedPos.GetFloorIntY()) == g_MaterialAir;

        // Mousewheel is used as shortcut for getting next and prev items in teh picker's object list
        if (m_pController->IsState(SCROLL_UP) || m_pController->IsState(ControlState::ACTOR_NEXT))
        {
            // Assign a copy of the next picked object to be the currently held one.
            const SceneObject *pNewObject = m_pPicker->GetPrevObject();
            if (pNewObject)
            {
                // Set and update the cursor object
                if (SetCurrentObject(dynamic_cast<SceneObject *>(pNewObject->Clone())))
                    m_pCurrentObject->Update();
            }
        }
        else if (m_pController->IsState(SCROLL_DOWN) || m_pController->IsState(ControlState::ACTOR_PREV))
        {
            // Assign a copy of the next picked object to be the currently held one.
            const SceneObject *pNewObject = m_pPicker->GetNextObject();
            if (pNewObject)
            {
                // Set and update the object
                if (SetCurrentObject(dynamic_cast<SceneObject *>(pNewObject->Clone())))
                    m_pCurrentObject->Update();
            }
        }

        // Start the timer when the button is first pressed, and when the picker has deactivated
        if (m_pController->IsState(PRESS_PRIMARY) && !m_pPicker->IsVisible())
        {
            m_BlinkTimer.Reset();
            m_EditorGUIMode = PLACINGOBJECT;
            m_PreviousMode = ADDINGOBJECT;
            m_ModeChanged = true;
            g_GUISound.PlacementBlip()->Play();
        }

        // Apply the team to the current actor, if applicable
        if (m_pCurrentObject && m_DrawCurrentObject)
        {
            // Set the team of SceneObject based on what's been selected
            // Only if full featured mode, otherwise it's based on the controller when placed
            if (m_FeatureSet == ONLOADEDIT)
                m_pCurrentObject->SetTeam(m_PlaceTeam);
        }
    }

    /////////////////////////////////////////////////////////////
    // PLACING MODE

    if (m_EditorGUIMode == PLACINGOBJECT)
    {
        if (m_ModeChanged)
        {
            m_ModeChanged = false;
        }

        if (m_PreviousMode == MOVINGOBJECT)
            g_FrameMan.SetScreenText("Click and drag on a placed object to MOVE it - Click quickly to DETACH" + selectedAssembly, g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
        else
            g_FrameMan.SetScreenText("Release to ADD the new object - Tap other button to cancel" + selectedAssembly, g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

        m_DrawCurrentObject = true;

        // Freeze when first pressing down and grid snapping is still engaged
        if (!(m_pController->IsState(PRIMARY_ACTION) && m_GridSnapping))
        {
            if (!analogInput.IsZero())
            {
                m_CursorPos += analogInput;
                m_FacingLeft = analogInput.m_X < 0 || (m_FacingLeft && analogInput.m_X == 0);
            }
            // Try the mouse
            else if (!m_pController->GetMouseMovement().IsZero())
            {
                m_CursorPos += m_pController->GetMouseMovement();
                m_FacingLeft = m_pController->GetMouseMovement().m_X < 0 || (m_FacingLeft && m_pController->GetMouseMovement().m_X == 0);
            }
            // Digital input?
            else
            {
                if (pressUp)
                    m_CursorPos.m_Y -= 1;
                if (pressRight)
                {
                    m_CursorPos.m_X += 1;
                    m_FacingLeft = false;
                }
                if (pressDown)
                    m_CursorPos.m_Y += 1;
                if (pressLeft)
                {
                    m_CursorPos.m_X -= 1;
                    m_FacingLeft = true;
                }
            }

            // Detect whether the cursor is in the air, or if it's overlapping some terrain
            Vector snappedPos = g_SceneMan.SnapPosition(m_CursorPos, m_GridSnapping);
            m_CursorInAir = g_SceneMan.GetTerrMatter(snappedPos.GetFloorIntX(), snappedPos.GetFloorIntY()) == g_MaterialAir;
            // Also check that it isn't over unseen areas, can't place there
            m_CursorInAir = m_CursorInAir && !g_SceneMan.IsUnseen(snappedPos.GetFloorIntX(), snappedPos.GetFloorIntY(), m_pController->GetTeam());
        }

        // Constrain the cursor to only be within specific scene areas
// TODO: THIS!!!

        // Disable snapping after a small interval of holding down the button, to avoid unintentional nudges when just placing on the grid
        if (m_pController->IsState(PRIMARY_ACTION) && m_BlinkTimer.IsPastRealMS(333) && m_GridSnapping)
        {
            m_GridSnapping = false;
            m_CursorPos = g_SceneMan.SnapPosition(m_CursorPos);
        }

        // Cancel placing if secondary button is pressed
        if (m_pController->IsState(PRESS_SECONDARY) || m_pController->IsState(PIE_MENU_ACTIVE))
        {
            m_EditorGUIMode = m_PreviousMode;
            m_ModeChanged = true;
        }
        // If previous mode was moving, tear the gib loose if the button is released to soo
        else if (m_PreviousMode == MOVINGOBJECT && m_pController->IsState(RELEASE_PRIMARY) && !m_BlinkTimer.IsPastRealMS(150))
        {
            m_EditorGUIMode = ADDINGOBJECT;
            m_ModeChanged = true;
        }
        // Only place if the picker and pie menus are completely out of view, to avoid immediate placing after picking
        else if (m_pCurrentObject && m_pController->IsState(RELEASE_PRIMARY) && !m_pPicker->IsVisible())
        {
            m_pCurrentObject->Update();
			//If true we need to place object in the end, if false, then it was already given to an actor
			bool toPlace = true;

			//If we're placing an item then give that item to actor instead of dropping it nearby
			HeldDevice *pHeldDevice = dynamic_cast<HeldDevice *>(m_pCurrentObject);
	        if (pHeldDevice)
			{
				if (dynamic_cast<HDFirearm *>(pHeldDevice) || dynamic_cast<TDExplosive *>(pHeldDevice))
				{
					int objectListPosition = -1;

					//Find out if we have AHuman under the cursor
					const SceneObject *pPickedSceneObject = g_SceneMan.GetScene()->PickPlacedObject(editedSet, m_CursorPos, &objectListPosition);
					const Actor *pAHuman = 0;

					// Looks like we got nothing, search for actors in range then
					if (!pPickedSceneObject)
						pPickedSceneObject = g_SceneMan.GetScene()->PickPlacedActorInRange(editedSet, m_CursorPos, 20,  &objectListPosition);

					if (pPickedSceneObject)
					{
						pAHuman = dynamic_cast<const AHuman *>(pPickedSceneObject);
						
						if (!pAHuman)
						{
							// Maybe we clicked the underlying bunker module, search for actor in range
							pPickedSceneObject = g_SceneMan.GetScene()->PickPlacedActorInRange(editedSet, m_CursorPos, 20,  &objectListPosition);

							if (pPickedSceneObject)
								pAHuman = dynamic_cast<const AHuman *>(pPickedSceneObject);
						}

						if (pAHuman)
						{
							//Create a new AHuman instead of old one, give him an item, and delete old AHuman
							SceneObject * pNewObject = dynamic_cast<SceneObject *>(pPickedSceneObject->Clone());
							g_SceneMan.GetScene()->RemovePlacedObject(editedSet, objectListPosition);

							AHuman *pAHuman = dynamic_cast<AHuman *>(pNewObject);
							if (pAHuman)
							{
								pAHuman->AddInventoryItem(dynamic_cast<MovableObject *>(m_pCurrentObject->Clone()));
								pAHuman->FlashWhite(150);
							}

							g_SceneMan.GetScene()->AddPlacedObject(editedSet, pNewObject, objectListPosition);

							m_EditMade = true;
							g_GUISound.PlacementThud()->Play();
							toPlace = false;
						}
					}
					else
					{
						//No human? Look for brains robots then
						SceneObject *pBrain = g_SceneMan.GetScene()->GetResidentBrain(m_pController->GetPlayer());
						if (pBrain)
						{
							if (g_SceneMan.ShortestDistance(pBrain->GetPos(), m_CursorPos,true).GetMagnitude() < 20)
							{
								AHuman * pBrainAHuman = dynamic_cast<AHuman *>(pBrain);
								if (pBrainAHuman)
								{
									pBrainAHuman->AddInventoryItem(dynamic_cast<MovableObject *>(m_pCurrentObject->Clone()));
									pBrainAHuman->FlashWhite(150);
									m_EditMade = true;
									g_GUISound.PlacementThud()->Play();
									toPlace = false;
								}
							}
						}
					}
				}
			}

			BunkerAssemblyScheme *pBAS = dynamic_cast<BunkerAssemblyScheme *>(m_pCurrentObject);
	        if (pBAS)
			{
				//Clear current scheme and assign new one
				delete m_pCurrentScheme;
				m_pCurrentScheme = dynamic_cast<BunkerAssemblyScheme *>(m_pCurrentObject->Clone());
				m_CurrentAssemblyName = pBAS->GetPresetName();

				// Look through available assemblies and set the name appropriately
				int number = 1;
				list<Entity *> assemblies;
				g_PresetMan.GetAllOfGroup(assemblies, pBAS->GetPresetName(), "BunkerAssembly", -1);
				for (int i = 1; i < 256; i++)
				{	
					number = i;
					char currentName[256];

					std::snprintf(currentName, sizeof(currentName), "%s - %d", m_CurrentAssemblyName.c_str(), 1);

					for (list<Entity *>::iterator itr = assemblies.begin(); itr != assemblies.end(); itr++)
					{
						std::snprintf(currentName, sizeof(currentName), "%s - %d", m_CurrentAssemblyName.c_str(), number);
						if ((*itr)->GetPresetName() == currentName)
						{
							number = 0;
							break;
						}
					}

					if (number > 0)
					{
						m_CurrentAssemblyName = currentName;
						break;
					}
				}

				g_GUISound.PlacementThud()->Play();
				m_EditMade = true;
				toPlace = false;
			}

			// When we're trying to place bunker assembly it replaces current Scheme with it's parent Scheme
			// and places all included objects
			BunkerAssembly *pBA = dynamic_cast<BunkerAssembly *>(m_pCurrentObject);
	        if (pBA)
			{
				//Clear current scheme and assign new one
				delete m_pCurrentScheme;
				
				const Entity *pPreset = g_PresetMan.GetEntityPreset("BunkerAssemblyScheme", pBA->GetParentAssemblySchemeName(), -1);
				if (pPreset)
				{
					m_pCurrentScheme = dynamic_cast<BunkerAssemblyScheme *>(pPreset->Clone());
					m_pCurrentScheme->SetPos(m_pCurrentObject->GetPos());
				}

				//Place objects inlcuded in bunker assembly
				const std::list<SceneObject *> *objects = pBA->GetPlacedObjects();
				
		        for (list<SceneObject *>::const_iterator oItr = objects->begin(); oItr != objects->end(); ++oItr)
				{
					SceneObject *pSO = dynamic_cast<SceneObject *>((*oItr)->Clone());

					// Convert relative coordinates to scene coordintaes
					Vector pos = pBA->GetPos() + pSO->GetPos() + pBA->GetBitmapOffset();

					//Wrap over seam
					if (g_SceneMan.GetScene()->WrapsX())
					{
						if (pos.m_X < 0)
							pos.m_X += g_SceneMan.GetScene()->GetWidth();
						
						if (pos.m_X >= g_SceneMan.GetScene()->GetWidth())
							pos.m_X -= g_SceneMan.GetScene()->GetWidth();
					}

					if (g_SceneMan.GetScene()->WrapsY())
					{
						if (pos.m_Y < 0)
							pos.m_Y += g_SceneMan.GetScene()->GetHeight();
						
						if (pos.m_Y >= g_SceneMan.GetScene()->GetHeight())
							pos.m_Y -= g_SceneMan.GetScene()->GetHeight();
					}
					pSO->SetPos(pos);

					g_SceneMan.GetScene()->AddPlacedObject(editedSet, pSO, m_ObjectListOrder);
					// Increment the list order so we place over last placed item
					if (m_ObjectListOrder >= 0)
						m_ObjectListOrder++;
				}
				m_CurrentAssemblyName = pBA->GetPresetName();
				g_GUISound.PlacementThud()->Play();
				m_EditMade = true;
				toPlace = false;
			}

			//Finally place an object if still necessary
			if (toPlace)
			{
				g_SceneMan.GetScene()->AddPlacedObject(editedSet, dynamic_cast<SceneObject *>(m_pCurrentObject->Clone()), m_ObjectListOrder);
				// Increment the list order so we place over last placed item
				if (m_ObjectListOrder >= 0)
					m_ObjectListOrder++;
				g_GUISound.PlacementThud()->Play();
				m_EditMade = true;
            }
// TEMP REMOVE WEHN YOU CLEAN UP THE ABOVE HARDCODED BRAIN PLACEMENT
            if (m_EditorGUIMode != PICKINGOBJECT)
// TEMP REMOVE ABOVE
            // Go back to previous mode
            m_EditorGUIMode = m_PreviousMode;
            m_ModeChanged = true;
        }

        // Set the facing of AHumans based on right/left cursor movements
// TODO: Improve
        Actor *pActor = dynamic_cast<Actor *>(m_pCurrentObject);
        if (pActor && dynamic_cast<AHuman *>(pActor) || dynamic_cast<ACrab *>(pActor))
            pActor->SetHFlipped(m_FacingLeft);

		Deployment *pDeployment = dynamic_cast<Deployment *>(m_pCurrentObject);
		if (pDeployment)
			pDeployment->SetHFlipped(m_FacingLeft);
    }

    /////////////////////////////////////////////////////////////
    // POINTING AT MODES

    else if ((m_EditorGUIMode == MOVINGOBJECT || m_EditorGUIMode == DELETINGOBJECT) && !m_PieMenu->IsEnabled())
    {
        m_DrawCurrentObject = false;

        // Trap the mouse cursor
        g_UInputMan.TrapMousePos(true, m_pController->GetPlayer());

        // Move the cursor according to analog or mouse input
        if (!analogInput.IsZero())
            m_CursorPos += analogInput * 4;
        else if (!m_pController->GetMouseMovement().IsZero())
            m_CursorPos += m_pController->GetMouseMovement() / 2;
        // Digital input?
        else
        {
            if (pressUp)
                m_CursorPos.m_Y -= 1;
            if (pressRight)
                m_CursorPos.m_X += 1;
            if (pressDown)
                m_CursorPos.m_Y += 1;
            if (pressLeft)
                m_CursorPos.m_X -= 1;
        }

        /////////////////////////////////
        // MOVING OBJECT MODE

        if (m_EditorGUIMode == MOVINGOBJECT)
        {
            if (m_ModeChanged)
            {

                m_ModeChanged = false;
            }
            g_FrameMan.SetScreenText("Click and drag on a placed object to MOVE it - Click quickly to DETACH" + selectedAssembly, g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

            // Pick an object under the cursor and start moving it
            if (m_pController->IsState(PRESS_PRIMARY) && !m_pPicker->IsVisible())
            {
                const SceneObject *pPicked = g_SceneMan.GetScene()->PickPlacedObject(editedSet, m_CursorPos, &m_ObjectListOrder);
                if (pPicked)
                {
                    // Grab the position and a copy of the the object itself before killing it from the scene
                    SetCurrentObject(dynamic_cast<SceneObject *>(pPicked->Clone()));
                    m_CursorOffset = m_CursorPos - m_pCurrentObject->GetPos();
                    g_SceneMan.GetScene()->RemovePlacedObject(editedSet, m_ObjectListOrder);
                    m_EditMade = true;

                    // Go to placing mode to move it around
                    m_EditorGUIMode = PLACINGOBJECT;
                    m_PreviousMode = MOVINGOBJECT;
                    m_ModeChanged = true;
                    m_BlinkTimer.Reset();
                    g_GUISound.PlacementBlip()->Play();
                    g_GUISound.PlacementGravel()->Play();
                }
                else
                    g_GUISound.UserErrorSound()->Play();
            }
        }

        ////////////////////////////
        // REMOVING OBJECT MODE

        else if (m_EditorGUIMode == DELETINGOBJECT)
        {
            if (m_ModeChanged)
            {

                m_ModeChanged = false;
            }
            g_FrameMan.SetScreenText("Click and hold to select an object - release to DELETE it" + selectedAssembly, g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

            // When primary is held down, pick object and show which one will be nuked if released
            if (m_pController->IsState(PRIMARY_ACTION) && !m_pPicker->IsVisible())
            {
                m_pObjectToBlink = g_SceneMan.GetScene()->PickPlacedObject(editedSet, m_CursorPos);
            }
            else if (m_pController->IsState(RELEASE_PRIMARY))
            {
                if (g_SceneMan.GetScene()->PickPlacedObject(editedSet, m_CursorPos, &m_ObjectListOrder))
                {
                    // Nuke it!
                    g_SceneMan.GetScene()->RemovePlacedObject(editedSet, m_ObjectListOrder);
                    m_EditMade = true;
// TODO: Add awesome destruction sound here
                }
                else
                    g_GUISound.UserErrorSound()->Play();
            }
        }
    }
    else if (m_EditorGUIMode == DONEEDITING)
    {
        // Check first that the brain is in a good spot if finishing up a base edit
        if (m_FeatureSet != ONLOADEDIT)
            TestBrainResidence();
//        if (m_FeatureSet != ONLOADEDIT)
//            g_FrameMan.SetScreenText("DONE editing, wait for all other players to finish too...", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
    }

    // Remove cursor offset if not applicable anymore
    if (m_EditorGUIMode != PLACINGOBJECT)
        m_CursorOffset.Reset();

    // Keep the cursor position within the world
    bool cursorWrapped = g_SceneMan.ForceBounds(m_CursorPos);
// TODO: make setscrolltarget with 'sloppy' target
    // Scroll to the cursor's scene position
    g_SceneMan.SetScrollTarget(m_CursorPos, 0.3, cursorWrapped, g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
    // Apply the cursor position to the currently held object
    if (m_pCurrentObject && m_DrawCurrentObject)
    {
        m_pCurrentObject->SetPos(g_SceneMan.SnapPosition(m_CursorPos - m_CursorOffset, m_GridSnapping));
        // If an actor, set it to be inactive so it doesn't scan around and reveal unseen areas
        if (Actor *pCurrentActor = dynamic_cast<Actor *>(m_pCurrentObject))
        {
            pCurrentActor->SetStatus(Actor::INACTIVE);
            pCurrentActor->GetController()->SetDisabled(true);
        }
        m_pCurrentObject->Update();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the menu

void AssemblyEditorGUI::Draw(BITMAP *pTargetBitmap, const Vector &targetPos) const
{
    // Done, so don't draw the UI
    if (m_EditorGUIMode == DONEEDITING)
        return;

    // The get a list of the currently edited set of placed objects in the Scene
    const std::list<SceneObject *> *pSceneObjectList = 0;
    if (m_FeatureSet == ONLOADEDIT)
        pSceneObjectList = g_SceneMan.GetScene()->GetPlacedObjects(Scene::PLACEONLOAD);

    // Draw the set of currently edited objects already placed in the Scene
    if (pSceneObjectList)
    {
        // Draw all already placed Objects, and the currently held one in the order it is about to be placed in the scene
        int i = 0;
        Actor *pActor = 0;
//        HeldDevice *pDevice = 0;
        for (list<SceneObject *>::const_iterator itr = pSceneObjectList->begin(); itr != pSceneObjectList->end(); ++itr, ++i)
        {
            // Draw the currently held object into the order of the list if it is to be placed inside
            if (m_pCurrentObject && m_DrawCurrentObject && i == m_ObjectListOrder)
            {
                g_FrameMan.SetTransTable(m_BlinkTimer.AlternateReal(333) || m_EditorGUIMode == PLACINGOBJECT ? LessTrans : HalfTrans);
                m_pCurrentObject->Draw(pTargetBitmap, targetPos, g_DrawTrans);
                pActor = dynamic_cast<Actor *>(m_pCurrentObject);
                if (pActor)
                    pActor->DrawHUD(pTargetBitmap, targetPos);
            }

            // Is the placed object an actor?
            pActor = dynamic_cast<Actor *>(*itr);
//            pItem = dynamic_cast<MovableObject *>(*itr);            

            // Blink trans if we are supposed to blink this one
            if ((*itr) == m_pObjectToBlink)
            {
                g_FrameMan.SetTransTable(m_BlinkTimer.AlternateReal(333) ? LessTrans : HalfTrans);
                (*itr)->Draw(pTargetBitmap, targetPos, g_DrawTrans);
            }
            // Drawing of already placed objects that aren't highlighted or anything
            else
            {
				(*itr)->Draw(pTargetBitmap, targetPos);
            }

            // Draw basic HUD if an actor - don't do this for blueprints.. it is confusing
            if (pActor)
                pActor->DrawHUD(pTargetBitmap, targetPos);
        }
    }

    // Draw picking object crosshairs and not the selected object
    if (!m_DrawCurrentObject)
    {
        Vector center = m_CursorPos - targetPos;
        putpixel(pTargetBitmap, center.m_X, center.m_Y, g_YellowGlowColor);
        hline(pTargetBitmap, center.m_X - 5, center.m_Y, center.m_X - 2, g_YellowGlowColor);
        hline(pTargetBitmap, center.m_X + 5, center.m_Y, center.m_X + 2, g_YellowGlowColor);
        vline(pTargetBitmap, center.m_X, center.m_Y - 5, center.m_Y - 2, g_YellowGlowColor);
        vline(pTargetBitmap, center.m_X, center.m_Y + 5, center.m_Y + 2, g_YellowGlowColor);
    }
    // If the held object will be placed at the end of the list, draw it last to the scene, transperent blinking
	else if (m_pCurrentObject && (m_ObjectListOrder < 0 || (pSceneObjectList && m_ObjectListOrder == pSceneObjectList->size())))
    {
        g_FrameMan.SetTransTable(m_BlinkTimer.AlternateReal(333) || m_EditorGUIMode == PLACINGOBJECT ? LessTrans : HalfTrans);
        m_pCurrentObject->Draw(pTargetBitmap, targetPos, g_DrawTrans);
        Actor *pActor = dynamic_cast<Actor *>(m_pCurrentObject);
        if (pActor)
            pActor->DrawHUD(pTargetBitmap, targetPos);
    }

	if (m_pCurrentScheme)
		m_pCurrentScheme->Draw(pTargetBitmap, targetPos);

    m_pPicker->Draw(pTargetBitmap);

    // Draw the pie menu
	m_PieMenu->Draw(pTargetBitmap, targetPos);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateBrainPath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the brain path to the current resident brain, if any. If
//                  there's none, the path is cleared.

bool AssemblyEditorGUI::UpdateBrainPath()
{
    // First see if we have a brain in hand
    if (m_pCurrentObject && m_pCurrentObject->IsInGroup("Brains"))
    {
        m_BrainSkyPathCost = g_SceneMan.GetScene()->CalculatePath(m_CursorPos, Vector(m_CursorPos.m_X, 0), m_BrainSkyPath);
        return true;
    }

    // If not, then do we have a resident?
    SceneObject *pBrain = g_SceneMan.GetScene()->GetResidentBrain(m_pController->GetPlayer());
    if (pBrain)
        m_BrainSkyPathCost = g_SceneMan.GetScene()->CalculatePath(pBrain->GetPos(), Vector(pBrain->GetPos().m_X, 0), m_BrainSkyPath);
    else
    {
        m_BrainSkyPath.clear();
        m_BrainSkyPathCost = 0;
        return false;
    }
    return true;
}