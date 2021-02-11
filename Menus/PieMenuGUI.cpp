//////////////////////////////////////////////////////////////////////////////////////////
// File:            PieMenuGUI.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation file for the PieMenuGUI class
// Project:         GUI Library
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "PieMenuGUI.h"

#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "UInputMan.h"

#include "Controller.h"
#include "SceneObject.h"
#include "MOSprite.h"
#include "HeldDevice.h"
#include "AHuman.h"
#include "ACrab.h"
#include "ContentFile.h"
#include "AEmitter.h"

#include "GUI/GUI.h"
#include "GUI/GUIFont.h"
#include "GUI/AllegroBitmap.h"

using namespace RTE;

const string PieMenuGUI::Slice::c_ClassName = "Slice";
BITMAP *PieMenuGUI::s_pCursor;
//BITMAP *PieMenuGUI::s_pCursorGlow;

std::map<string, PieMenuGUI::Slice> PieMenuGUI::m_AllAvailableSlices;
const int PieMenuGUI::s_EnablingDelay = 50;

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Slice, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

void PieMenuGUI::Slice::Clear()
{
    m_Description.clear();
    m_SliceType = PieMenuGUI::PSI_NONE;
    m_Icon.Reset();
    m_Enabled = true;
    m_Highlighted = false;
    m_Direction = UP;
    m_GroupID = -1;
    m_AreaStart = 0;
    m_AreaArc = c_QuarterPI;
    m_MidAngle = m_AreaStart + (m_AreaArc / 2);
    m_ScriptPath.clear();
    m_FunctionName.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Slice to be identical to another, by deep copy.

int PieMenuGUI::Slice::Create(const Slice &reference)
{
    m_Description = reference.m_Description;
    m_SliceType = reference.m_SliceType;
    m_Icon = reference.m_Icon;
    m_Enabled = reference.m_Enabled;
    m_Highlighted = reference.m_Highlighted;
    m_Direction = reference.m_Direction;
    m_GroupID = reference.m_GroupID;
    m_AreaStart = reference.m_AreaStart;
    m_AreaArc = reference.m_AreaArc;
    m_MidAngle = reference.m_MidAngle;
    m_ScriptPath = reference.m_ScriptPath;
    m_FunctionName = reference.m_FunctionName;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Slice object ready for use.

int PieMenuGUI::Slice::Create()
{
    if (Serializable::Create() < 0)
        return -1;

    // Find and assign the proper Icon to correspond to this Slice's index
    if (m_SliceType == PSI_NONE)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Blank"));
    else if (m_SliceType == PSI_PICKUP)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Pick Up"));
    else if (m_SliceType == PSI_DROP)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Drop"));
    else if (m_SliceType == PSI_NEXTITEM)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Next"));
    else if (m_SliceType == PSI_PREVITEM)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Previous"));
    else if (m_SliceType == PSI_RELOAD)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Reload"));
    else if (m_SliceType == PSI_BUYMENU)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Trade Star"));
    else if (m_SliceType == PSI_STATS)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Stats"));
    else if (m_SliceType == PSI_MINIMAP)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Minimap"));
    else if (m_SliceType == PSI_FORMSQUAD)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Follow"));
    else if (m_SliceType == PSI_CEASEFIRE)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "White Flag"));
    else if (m_SliceType == PSI_SENTRY)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Eye"));
    else if (m_SliceType == PSI_PATROL)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Cycle"));
    else if (m_SliceType == PSI_BRAINHUNT)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Brain"));
    else if (m_SliceType == PSI_GOLDDIG)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Dig"));
    else if (m_SliceType == PSI_GOTO)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Go To"));
    else if (m_SliceType == PSI_RETURN)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Launch"));
    else if (m_SliceType == PSI_STAY)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Land"));
    else if (m_SliceType == PSI_DELIVER)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Return"));
    else if (m_SliceType == PSI_SCUTTLE)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Death"));
    else if (m_SliceType == PSI_DONE)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Check"));
    else if (m_SliceType == PSI_LOAD)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Load"));
    else if (m_SliceType == PSI_SAVE)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Save"));
    else if (m_SliceType == PSI_NEW)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Clear"));
    else if (m_SliceType == PSI_PICK)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Menu"));
    else if (m_SliceType == PSI_MOVE)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Move"));
    else if (m_SliceType == PSI_REMOVE)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Remove"));
    else if (m_SliceType == PSI_INFRONT)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "In Front"));
    else if (m_SliceType == PSI_BEHIND)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Behind"));
    else if (m_SliceType == PSI_ZOOMIN)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Zoom In"));
    else if (m_SliceType == PSI_ZOOMOUT)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Zoom Out"));
    else if (m_SliceType == PSI_TEAM1)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 1"));
    else if (m_SliceType == PSI_TEAM2)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 2"));
    else if (m_SliceType == PSI_TEAM3)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 3"));
    else if (m_SliceType == PSI_TEAM4)
        m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 4"));

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int PieMenuGUI::Slice::ReadProperty(const std::string_view &propName, Reader &reader)
{
    if (propName == "Description")
        reader >> m_Description;
    else if (propName == "Icon")
        reader >> m_Icon;
    else if (propName == "Direction")
        reader >> m_Direction;
	else if (propName == "ScriptPath") {
		m_ScriptPath = CorrectBackslashesInPath(reader.ReadPropValue());
	} else if (propName == "FunctionName")
        reader >> m_FunctionName;
    else
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Slice with a Writer for
//                  later recreation with Create(Reader &reader);

int PieMenuGUI::Slice::Save(Writer &writer) const {
	Serializable::Save(writer);

	writer.NewPropertyWithValue("Description", m_Description);
	writer.NewPropertyWithValue("Icon", m_Icon);
	writer.NewPropertyWithValue("Direction", m_Direction);
	writer.NewPropertyWithValue("ScriptPath", m_ScriptPath);
	writer.NewPropertyWithValue("FunctionName", m_FunctionName);

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this PieMenuGUI, effectively
//                  resetting the members of this abstraction level only.

void PieMenuGUI::Clear()
{
    m_pController = 0;
    m_pActor = 0;
    m_CenterPos.Reset();
    m_PieEnabled = DISABLED;
    m_pHoveredSlice = 0;
    m_pActivatedSlice = 0;
    m_PieCommand = PSI_NONE;
    m_AlreadyActivated = PSI_NONE;
    m_HoverTimer.Reset();
    m_BlinkTimer.Reset();
    m_BlinkMode = NOBLINK;

    m_UpSlice.Reset();
    m_LeftSlice.Reset();
    m_DownSlice.Reset();
    m_RightSlice.Reset();

    m_UpRightSlices.clear();
    m_UpLeftSlices.clear();
    m_DownLeftSlices.clear();
    m_DownRightSlices.clear();

    m_AllSlices.clear();
    m_SliceGroupCount = 0;

    m_EnablingTimer.Reset();
    m_Wobbling = false;
    m_Freeze = false;
    m_InnerRadius = 0;
    m_EnabledRadius = 58;
    m_Thickness = 16;
    m_pBGBitmap = 0;
    m_RedrawBG = true;
    m_CursorAng = 0;
    m_EnoughInput = false;
    m_DInputHoldTimer.Reset();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the PieMenuGUI object ready for use.

int PieMenuGUI::Create(Controller *pController, Actor *pFocusActor)
{
    RTEAssert(pController, "No controller sent to PieMenuGUI on creation!");
    m_pController = pController;

    m_pActor = pFocusActor;
    if (m_pActor)
	{
        m_CenterPos = m_pActor->GetCPUPos();
		m_pLastKnownActor = m_pActor;
	}

    // Load the static pie icons and cursor if havne't been done yet
    if (!s_pCursor)
    {
        ContentFile cursorFile("Base.rte/GUIs/Skins/PieCursor.png");
        s_pCursor = cursorFile.GetAsBitmap();
//        cursorFile.SetDataPath("Base.rte/Effects/Glows/YellowSmall.png");
//        s_pCursorGlow = cursorFile.GetAsBitmap();
    }

    if (!m_pBGBitmap)
    {
        int diameter = (m_EnabledRadius + m_Thickness + 2) * 2;
        m_pBGBitmap = create_bitmap_ex(8, diameter, diameter);
        clear_to_color(m_pBGBitmap, g_MaskColor);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the PieMenuGUI object.

void PieMenuGUI::Destroy()
{
    destroy_bitmap(m_pBGBitmap);

    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Enables or disables the menu. This will animate it in and out of view.

void PieMenuGUI::SetEnabled(bool enable)
{
    m_Wobbling = false;
    m_Freeze = false;

    if (enable && m_PieEnabled != ENABLED && m_PieEnabled != ENABLING)
    {
        // Flash the actor when starting to enable.
        if (m_pActor)
            m_pActor->FlashWhite();
        m_PieEnabled = ENABLING;
        m_EnablingTimer.Reset();
        m_AlreadyActivated = PSI_NONE;
        // Reset mouse position in center of its trap so pre-activation direction doesn't affect the menu
        if (m_pController->IsMouseControlled())
            g_UInputMan.SetMouseValueMagnitude(0);
		g_GUISound.PieMenuEnterSound()->Play(m_pController->GetPlayer());
    }
    else if (!enable && m_PieEnabled != DISABLED && m_PieEnabled != DISABLING)
    {
        m_PieEnabled = DISABLING;
        m_EnablingTimer.Reset();
        // Reset mouse position in center of its trap so menu direction doesn't affect an actor's aim
        if (m_pController->IsMouseControlled())
            g_UInputMan.SetMouseValueMagnitude(0);
        // Only play regular exit sound if the special sounds for selected slices won't play
//        if (!m_pHoveredSlice && !m_pActivatedSlice)
		g_GUISound.PieMenuExitSound()->Play(m_pController->GetPlayer());
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResetSlices
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets and removes all slices from the menu, so that new configuration
//                  of them can be added.

void PieMenuGUI::ResetSlices()
{
    m_pHoveredSlice = 0;
    m_pActivatedSlice = 0;

    m_UpSlice.Reset();
    m_LeftSlice.Reset();
    m_DownSlice.Reset();
    m_RightSlice.Reset();

    m_UpRightSlices.clear();
    m_UpLeftSlices.clear();
    m_DownLeftSlices.clear();
    m_DownRightSlices.clear();

    m_AllSlices.clear();
    m_SliceGroupCount = 0;
    m_RedrawBG = true;
}



bool PieMenuGUI::AddSliceLua(string description, string functionName, PieMenuGUI::Slice::SliceDirection direction, bool isEnabled)
{
	// Look for existing available slice item to use a reference
	int foundSliceIndex = -1;
	int sliceIndex = 0;

	if (m_AllAvailableSlices.find(description + "::" + functionName) != m_AllAvailableSlices.end())
	{
		Slice s = m_AllAvailableSlices[description + "::" + functionName];
		s.m_Direction = direction;
		s.m_Enabled = isEnabled;
		AddSlice(s);
	}

	// Remove the copy of slice since after Add we now have two duplicates of the same slice
	if (foundSliceIndex > -1)
	{
		//m_AllAddedSlices.erase(m_AllAddedSlices.begin() + foundSliceIndex);
		return true;
	}

	return false;
}

void PieMenuGUI::AlterSliceLua(string description, string functionName, PieMenuGUI::Slice::SliceDirection direction, bool isEnabled)
{
	Slice foundSlice;

	// Look for matching slice to alter
	vector<Slice *>::iterator sSliceItr;
	for (sSliceItr = m_AllSlices.begin(); sSliceItr != m_AllSlices.end(); ++sSliceItr)
	{
		if ((*sSliceItr)->m_Description == description && (*sSliceItr)->m_FunctionName == functionName)
		{
			foundSlice = *(*sSliceItr);
		}
	}

	if (m_UpSlice.m_Description == description && m_UpSlice.m_FunctionName == functionName)
		foundSlice = m_UpSlice;
	if (m_DownSlice.m_Description == description && m_DownSlice.m_FunctionName == functionName)
		foundSlice = m_DownSlice;
	if (m_LeftSlice.m_Description == description && m_LeftSlice.m_FunctionName == functionName)
		foundSlice = m_LeftSlice;
	if (m_RightSlice.m_Description == description && m_RightSlice.m_FunctionName == functionName)
		foundSlice = m_RightSlice;

	if (foundSlice.m_SliceType != PieSliceIndex::PSI_NONE)
	{
		foundSlice.m_Direction = direction;
		foundSlice.m_Enabled = isEnabled;

		RemoveSliceLua(description, functionName);
		AddSlice(foundSlice);
	}

	// Reallign to update 
	RealignSlices();
}


PieMenuGUI::Slice PieMenuGUI::RemoveSliceLua(string description, string functionName)
{
	Slice retValue;

	// Look for slices across all the collection to remove
	std::vector< std::list<Slice> * > sliceCollections;
	sliceCollections.push_back(&m_UpRightSlices);
	sliceCollections.push_back(&m_UpLeftSlices);
	sliceCollections.push_back(&m_DownLeftSlices);
	sliceCollections.push_back(&m_DownRightSlices);

	std::vector< std::list<Slice> * >::iterator sItr;
	for (sItr = sliceCollections.begin(); sItr != sliceCollections.end(); ++sItr)
	{
		list<Slice>::iterator sSliceItr;
		for (sSliceItr = (*sItr)->begin(); sSliceItr != (*sItr)->end(); ++sSliceItr)
		{
			if ((*sSliceItr).m_Description == description && (*sSliceItr).m_FunctionName == functionName)
			{
				retValue = (*sSliceItr);

				(*sItr)->erase(sSliceItr);
				break;
			}
		}
	}

	if (m_UpSlice.m_Description == description && m_UpSlice.m_FunctionName == functionName)
		m_UpSlice.Reset();
	if (m_DownSlice.m_Description == description && m_DownSlice.m_FunctionName == functionName)
		m_DownSlice.Reset();
	if (m_LeftSlice.m_Description == description && m_LeftSlice.m_FunctionName == functionName)
		m_LeftSlice.Reset();
	if (m_RightSlice.m_Description == description && m_RightSlice.m_FunctionName == functionName)
		m_RightSlice.Reset();

	// Reallign to update 
	RealignSlices();

	return retValue;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddSlice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a Slice to the menu. It will be placed according to what's already
//                  in there, and what placement apriority parameters it has.
bool PieMenuGUI::AddSlice(Slice &newSlice, bool takeAnyFreeCardinal)
{
	//m_AllAddedSlices.push_back(newSlice);

    // Make sure the slice is created and has a corresponding Icon assigned
    if (newSlice.m_Icon.GetFrameCount() <= 0)
        newSlice.Create();

    // Trying to place in the UP direction
    if (newSlice.m_Direction == Slice::UP)
    {
        // If available, take it!
        if (m_UpSlice.m_SliceType == PSI_NONE)
            m_UpSlice = newSlice;
        // If not, check first adjacent cardinal direction
        else if (takeAnyFreeCardinal && m_LeftSlice.m_SliceType == PSI_NONE)
            m_LeftSlice = newSlice;
        // If not, check second adjacent cardinal direction
        else if (takeAnyFreeCardinal && m_RightSlice.m_SliceType == PSI_NONE)
            m_RightSlice = newSlice;
        // If not, check opposite cardinal direction
        else if (takeAnyFreeCardinal && m_DownSlice.m_SliceType == PSI_NONE)
            m_DownSlice = newSlice;
        // If none of these are available, then we have to put into the least crowded adjacent corner
        else
        {
            // Decide which is least crowded and put it there, in the appropriate end
            if (m_UpRightSlices.size() <= m_UpLeftSlices.size())
                m_UpRightSlices.push_front(newSlice);
            else
                m_UpLeftSlices.push_front(newSlice);
        }
    }
    // Trying to place in the LEFT direction
    else if (newSlice.m_Direction == Slice::LEFT)
    {
        // If available, take it!
        if (m_LeftSlice.m_SliceType == PSI_NONE)
            m_LeftSlice = newSlice;
        // If not, check first adjacent cardinal direction
        else if (takeAnyFreeCardinal && m_DownSlice.m_SliceType == PSI_NONE)
            m_DownSlice = newSlice;
        // If not, check second adjacent cardinal direction
        else if (takeAnyFreeCardinal && m_UpSlice.m_SliceType == PSI_NONE)
            m_UpSlice = newSlice;
        // If not, check opposite cardinal direction
        else if (takeAnyFreeCardinal && m_RightSlice.m_SliceType == PSI_NONE)
            m_RightSlice = newSlice;
        // If none of these are available, then we have to put into the least crowded adjacent corner
        else
        {
            // Decide which is least crowded and put it there
            if (m_UpLeftSlices.size() <= m_DownLeftSlices.size())
                m_UpLeftSlices.push_back(newSlice);
            else
                m_DownLeftSlices.push_front(newSlice);
        }
    }
    // Trying to place in the DOWN direction
    else if (newSlice.m_Direction == Slice::DOWN)
    {
        // If available, take it!
        if (m_DownSlice.m_SliceType == PSI_NONE)
            m_DownSlice = newSlice;
        // If not, check first adjacent cardinal direction
        else if (takeAnyFreeCardinal && m_RightSlice.m_SliceType == PSI_NONE)
            m_RightSlice = newSlice;
        // If not, check second adjacent cardinal direction
        else if (takeAnyFreeCardinal && m_LeftSlice.m_SliceType == PSI_NONE)
            m_LeftSlice = newSlice;
        // If not, check opposite cardinal direction
        else if (takeAnyFreeCardinal && m_UpSlice.m_SliceType == PSI_NONE)
            m_UpSlice = newSlice;
        // If none of these are available, then we have to put into the least crowded adjacent corner
        else
        {
            // Decide which is least crowded and put it there
            if (m_DownLeftSlices.size() <= m_DownRightSlices.size())
                m_DownLeftSlices.push_back(newSlice);
            else
                m_DownRightSlices.push_back(newSlice);
        }
    }
    // Trying to place in the RIGHT direction
    else if (newSlice.m_Direction == Slice::RIGHT)
    {
        // If available, take it!
        if (m_RightSlice.m_SliceType == PSI_NONE)
            m_RightSlice = newSlice;
        // If not, check first adjacent cardinal direction
        else if (takeAnyFreeCardinal && m_UpSlice.m_SliceType == PSI_NONE)
            m_UpSlice = newSlice;
        // If not, check second adjacent cardinal direction
        else if (takeAnyFreeCardinal && m_DownSlice.m_SliceType == PSI_NONE)
            m_DownSlice = newSlice;
        // If not, check opposite cardinal direction
        else if (takeAnyFreeCardinal && m_LeftSlice.m_SliceType == PSI_NONE)
            m_LeftSlice = newSlice;
        // If none of these are available, then we have to put into the least crowded adjacent corner
        else
        {
            // Decide which is least crowded and put it there
            if (m_DownRightSlices.size() <= m_UpRightSlices.size())
                m_DownRightSlices.push_front(newSlice);
            else
                m_UpRightSlices.push_back(newSlice);
        }
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddSliceToGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a Slice to the menu, as part of a group. It will be assigned a
//                  group ID, which will also be returned. If a new group is wanted, pass
//                  -1 as groupID.

int PieMenuGUI::AddSliceToGroup(Slice &newSlice, int groupID)
{
    if (groupID < 0)
    {
        newSlice.m_GroupID = m_SliceGroupCount++;
        return AddSlice(newSlice) ? newSlice.m_GroupID : -1;
    }
    else if (groupID < m_SliceGroupCount)
    {
        newSlice.m_GroupID = groupID;
        return AddSlice(newSlice) ? newSlice.m_GroupID : -1;
    }

    return -1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RealignSlices
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes sure all currently added slices' areas are set up correctly
//                  position and sizewise on the pie menu circle.

void PieMenuGUI::RealignSlices()
{
    m_AllSlices.clear();
    list<Slice>::iterator sItr;
    // For the corner lists we need to traverse backwards to get the right order on the circle
    list<Slice>::reverse_iterator srItr;

    // Start straight out right and go CCW, aligning all encountered slcies as we go

    // UPPER RIGHT QUADRANT
    if (!m_UpRightSlices.empty())
    {
        int which = 0;
        for (srItr = m_UpRightSlices.rbegin(); srItr != m_UpRightSlices.rend(); ++srItr)
        {
            (*srItr).m_AreaArc = c_QuarterPI / m_UpRightSlices.size();
            (*srItr).m_AreaStart = c_EighthPI + which * (*srItr).m_AreaArc;
            // Add the corner slice to the ordered list of all
            m_AllSlices.push_back(&(*srItr));
            ++which;
        }
        m_UpSlice.m_AreaStart = c_QuarterPI + c_EighthPI;
        // Assume next quadrant won't have any corner slices
        m_UpSlice.m_AreaArc = c_QuarterPI + c_EighthPI;
    }
    else
    {
        m_UpSlice.m_AreaStart = c_QuarterPI;
        m_UpSlice.m_AreaArc = c_HalfPI;
    }

    // Add the next cardinal slice to the ordered list of all
    m_AllSlices.push_back(&m_UpSlice);

    // UPPER LEFT QUADRANT
    if (!m_UpLeftSlices.empty())
    {
        int which = 0;
        for (sItr = m_UpLeftSlices.begin(); sItr != m_UpLeftSlices.end(); ++sItr)
        {
            (*sItr).m_AreaArc = c_QuarterPI / m_UpLeftSlices.size();
            (*sItr).m_AreaStart = c_HalfPI + c_EighthPI + which * (*sItr).m_AreaArc;
            // Add the corner slice to the ordered list of all
            m_AllSlices.push_back(&(*sItr));
            ++which;
        }
        m_LeftSlice.m_AreaStart = c_HalfPI + c_QuarterPI + c_EighthPI;
        // Assume next quadrant won't have any corner slices
        m_LeftSlice.m_AreaArc =  c_QuarterPI + c_EighthPI;
        // Adjust previous cardinal slice since this quadrant is encroaching on it
        m_UpSlice.m_AreaArc -= c_EighthPI;
    }
    else
    {
        m_LeftSlice.m_AreaStart = c_HalfPI + c_QuarterPI;
        m_LeftSlice.m_AreaArc = c_HalfPI;
    }

    // Add the next cardinal slice to the ordered list of all
    m_AllSlices.push_back(&m_LeftSlice);

    // LOWER LEFT QUADRANT
    if (!m_DownLeftSlices.empty())
    {
        int which = 0;
        for (sItr = m_DownLeftSlices.begin(); sItr != m_DownLeftSlices.end(); ++sItr)
        {
            (*sItr).m_AreaArc = c_QuarterPI / m_DownLeftSlices.size();
            (*sItr).m_AreaStart = c_PI + c_EighthPI + which * (*sItr).m_AreaArc;
            // Add the corner slice to the ordered list of all
            m_AllSlices.push_back(&(*sItr));
            ++which;
        }
        m_DownSlice.m_AreaStart = c_PI + c_QuarterPI + c_EighthPI;
        // Assume next quadrant won't have any corner slices
        m_DownSlice.m_AreaArc = c_QuarterPI + c_EighthPI;
        // Adjust previous cardinal slice since this quadrant is encroaching on it
        m_LeftSlice.m_AreaArc -= c_EighthPI;
    }
    else
    {
        m_DownSlice.m_AreaStart = c_PI + c_QuarterPI;
        m_DownSlice.m_AreaArc = c_HalfPI;
    }

    // Add the next cardinal slice to the ordered list of all
    m_AllSlices.push_back(&m_DownSlice);

    // LOWER RIGHT QUADRANT
    if (!m_DownRightSlices.empty())
    {
        int which = 0;
        for (srItr = m_DownRightSlices.rbegin(); srItr != m_DownRightSlices.rend(); ++srItr)
        {
            (*srItr).m_AreaArc = c_QuarterPI / m_DownRightSlices.size();
            (*srItr).m_AreaStart = c_PI + c_HalfPI + c_EighthPI + which * (*srItr).m_AreaArc;
            // Add the corner slice to the ordered list of all
            m_AllSlices.push_back(&(*srItr));
            ++which;
        }
        m_RightSlice.m_AreaStart = c_PI + c_HalfPI + c_QuarterPI + c_EighthPI;
        // Check if next quadrant has any corner slices and set size accordingly
        m_RightSlice.m_AreaArc = c_QuarterPI + (m_UpRightSlices.empty() ? c_EighthPI : 0);
        // Adjust previous cardinal slice since this quadrant is encroaching on it
        m_DownSlice.m_AreaArc -= c_EighthPI;
    }
    else
    {
        m_RightSlice.m_AreaStart = c_PI + c_HalfPI + c_QuarterPI;
        m_RightSlice.m_AreaArc = c_HalfPI - (m_UpRightSlices.empty() ? 0 : c_EighthPI);
    }

    // Add the next cardinal slice to the ordered list of all
    m_AllSlices.push_back(&m_RightSlice);

    // Precalculate the mid angles for all slices
    for (vector<Slice *>::const_iterator sItr = m_AllSlices.begin(); sItr != m_AllSlices.end(); ++sItr)
        (*sItr)->m_MidAngle = (*sItr)->m_AreaStart + ((*sItr)->m_AreaArc / 2);

    m_RedrawBG = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSliceOnAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific slice based on an angle on the pie menu.

const PieMenuGUI::Slice * PieMenuGUI::GetSliceOnAngle(float angle)
{
    float areaEnd;

    for (vector<Slice *>::const_iterator sItr = m_AllSlices.begin(); sItr != m_AllSlices.end(); ++sItr)
    {
        areaEnd = (*sItr)->m_AreaStart + (*sItr)->m_AreaArc;
        // If the angle is found to be between 
        if ((*sItr)->m_AreaStart <= angle && areaEnd > angle)
            return *sItr;

        // Check for if the target slice overlaps the 0 point on the arc
        if (areaEnd > c_PI * 2 && angle >= 0 && angle < (areaEnd - c_PI * 2))
            return *sItr;
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this Menu each frame

void PieMenuGUI::Update()
{
    m_pActivatedSlice = 0;
    m_PieCommand = PSI_NONE;

    // Special wobble animation, menu is effectively deactivated
    if (m_Wobbling)
    {
        // Get it started
        if (m_PieEnabled != ENABLING && m_PieEnabled != DISABLING)
            m_PieEnabled = ENABLING;

        m_RedrawBG = true;
        // Growing
        if (m_PieEnabled == ENABLING)
        {
            m_InnerRadius += m_EnablingTimer.GetElapsedRealTimeMS() / 6;
            if (m_InnerRadius > m_EnabledRadius / 2)
            {
                m_InnerRadius = m_EnabledRadius / 2;
                m_PieEnabled = DISABLING;
            }
        }
        // Shrinking
        else if (m_PieEnabled == DISABLING)
        {
            m_InnerRadius -= m_EnablingTimer.GetElapsedRealTimeMS() / 6;
            if (m_InnerRadius < 0)
            {
                m_InnerRadius = 0;
                m_PieEnabled = ENABLING;
            }
        }
        m_EnablingTimer.Reset();
    }
    // Special frozen radius circle stage
    else if (m_Freeze)
    {
        // Don't do shit
        m_PieEnabled = ENABLING;
    }
    // Animate the menu into and out of view if enabled or disabled
    else
    {
        if (m_PieEnabled == ENABLING)
        {
            m_RedrawBG = true;
            m_InnerRadius = LERP(0, s_EnablingDelay, 0, m_EnabledRadius, m_EnablingTimer.GetElapsedRealTimeMS()); 
            if (m_EnablingTimer.IsPastRealMS(s_EnablingDelay))
            {
                m_InnerRadius = m_EnabledRadius;
                m_PieEnabled = ENABLED;
            }
        }
        // Animate the menu out of view
        else if (m_PieEnabled == DISABLING)
        {
            m_RedrawBG = true;
            m_InnerRadius = LERP(0, s_EnablingDelay, m_EnabledRadius, 0, m_EnablingTimer.GetElapsedRealTimeMS()); 
            if (m_EnablingTimer.IsPastRealMS(s_EnablingDelay))
            {
                // Flash the actor when done disabling.
                if (m_pActor)
                    m_pActor->FlashWhite();
                m_InnerRadius = 0;
                m_PieEnabled = DISABLED;
            }
        }
    }

    // Quit if no controller
    if (m_pController->IsDisabled())
        return;

    // Update the user controller - NO should be done by whomever owns this isntead
//    m_pController->Update();

    ////////////////////////////////////////////
    // Notification blinking logic
/*
    if (m_BlinkMode == NOFUNDS)
    {
        m_pCostLabel->SetVisible(m_BlinkTimer.AlternateSim(250));
    }
    else if (m_BlinkMode == NOCRAFT)
    {
        bool blink = m_BlinkTimer.AlternateSim(250);
        m_pCraftLabel->SetVisible(blink);
        m_pCraftBox->SetVisible(blink);
    }

    // Time out the blinker
    if (m_BlinkMode != NOBLINK && m_BlinkTimer.IsPastSimMS(1500))
    {
        m_pCostLabel->SetVisible(true);
        m_pCraftLabel->SetVisible(true);
        m_pCraftBox->SetVisible(true);
        m_BlinkMode = NOBLINK;
    }
*/
    /////////////////////////////////////////////////////
    // Analog input logic

    m_EnoughInput = false;

    // If not enabled, then don't read cursor input, and don't change the hovered slice
    if (IsEnabled())
    {
        // Get cursor angle input and see that it's enough to register as a direction on the pie
        if (m_pController->GetAnalogCursor().GetLargest() > 0.45)
        {
            m_CursorAng = m_pController->GetAnalogCursor().GetAbsRadAngle();
            m_EnoughInput = true;
        }

        // Always make cursor pos positive
        while (m_CursorAng < 0)
            m_CursorAng += c_PI * 2;

        // If enough input to give an angle, and a slice is present there, highlight it
        const Slice *pNewSlice = GetSliceOnAngle(m_CursorAng);
        // Only highlight if the currently ohvered slice is different from the previous one
        if (m_EnoughInput && pNewSlice && pNewSlice != m_pHoveredSlice)
        {
            m_pHoveredSlice = pNewSlice;
            m_RedrawBG = true;
            // Play the change sound
            if (m_pHoveredSlice->m_Enabled)
                g_GUISound.HoverChangeSound()->Play(m_pController->GetPlayer());
            else
                g_GUISound.HoverDisabledSound()->Play(m_pController->GetPlayer());
        }
    }

    //////////////////////////////////////////
    // Digital input logic
    
    // If not enabled, then don't read input, and don't change the hovered slice
    if (IsEnabled() && !m_EnoughInput)
    {
        // If we haven't already started to point at something, then pressing a d-pad entry will cause cardianl slice to be hovered
        if (!m_pHoveredSlice)
        {
            if (m_pController->IsState(PRESS_UP))
                m_EnoughInput = SelectSlice(&m_UpSlice, true);
            else if (m_pController->IsState(PRESS_LEFT))
                m_EnoughInput = SelectSlice(&m_LeftSlice, true);
            else if (m_pController->IsState(PRESS_DOWN))
                m_EnoughInput = SelectSlice(&m_DownSlice, true);
            else if (m_pController->IsState(PRESS_RIGHT))
                m_EnoughInput = SelectSlice(&m_RightSlice, true);

            // If something was initially selected here, reset the hold timer
            if (m_pHoveredSlice)
                m_DInputHoldTimer.Reset();
        }
        // Already pointing toward a slice; new d-pad input will move to next slice over
        else
        {
            // Check if all directions were let go of.. will stop pointing after delay
            if (m_DInputHoldTimer.IsPastRealMS(1500) && !m_pController->IsState(MOVE_UP) && !m_pController->IsState(MOVE_LEFT) && !m_pController->IsState(MOVE_DOWN) && !m_pController->IsState(MOVE_RIGHT))
                m_EnoughInput = false;
            else
            {
                // Find the currently hovered slice in the order of them all
                vector<Slice *>::iterator sItr = m_AllSlices.begin();
                int which = 0;
                int step = 0;
                for (sItr = m_AllSlices.begin(); sItr != m_AllSlices.end(); ++sItr, ++which)
                {
                    if ((*sItr) == m_pHoveredSlice)
                        break;
                }

                // Check for presses, and determine if the cursor position is opposite enough to warrant a jump to the cardinal axis
                // or if just a single step toward it should be made
                if (m_pController->IsState(PRESS_UP))
                {
                    if (m_CursorAng < m_RightSlice.m_MidAngle && m_CursorAng > m_LeftSlice.m_MidAngle)
                        SelectSlice(&m_UpSlice, true);
                    else if (m_pHoveredSlice != &m_UpSlice)
                        step = m_CursorAng > m_UpSlice.m_MidAngle && m_CursorAng < m_DownSlice.m_MidAngle ? -1 : 1;
                    m_DInputHoldTimer.Reset();
                }
                else if (m_pController->IsState(PRESS_LEFT))
                {
                    if (m_CursorAng < m_UpSlice.m_MidAngle || m_CursorAng > m_DownSlice.m_MidAngle)
                        SelectSlice(&m_LeftSlice, true);
                    else if (m_pHoveredSlice != &m_LeftSlice)
                        step = m_CursorAng > m_LeftSlice.m_MidAngle ? -1 : 1;
                    m_DInputHoldTimer.Reset();
                }
                else if (m_pController->IsState(PRESS_DOWN))
                {
                    if (m_CursorAng < m_LeftSlice.m_MidAngle || m_CursorAng > m_RightSlice.m_MidAngle)
                        SelectSlice(&m_DownSlice, true);
                    else if (m_pHoveredSlice != &m_DownSlice)
                        step = m_CursorAng > m_DownSlice.m_MidAngle || m_CursorAng < m_UpSlice.m_MidAngle ? -1 : 1;
                    m_DInputHoldTimer.Reset();
                }
                else if (m_pController->IsState(PRESS_RIGHT))
                {
                    if (m_CursorAng < m_DownSlice.m_MidAngle && m_CursorAng > m_UpSlice.m_MidAngle)
                        SelectSlice(&m_RightSlice, true);
                    else if (m_pHoveredSlice != &m_RightSlice)
                        step = m_CursorAng > m_RightSlice.m_MidAngle || m_CursorAng < (m_RightSlice.m_MidAngle - c_PI) ? -1 : 1;
                    m_DInputHoldTimer.Reset();
                }

                // We didn't jump to a cardinal axis, just moved a step, so make that happen
                if (step != 0)
                {
                    which += step;
                    if (which < 0)
                        which = m_AllSlices.size() - 1;
                    else if (which >= m_AllSlices.size())
                        which = 0;

                    if (!m_AllSlices.empty())
                        SelectSlice(m_AllSlices[which], true);
                }

                m_EnoughInput = true;
            }
        }
    }

    // Check for activation of the current hover
    if (m_pHoveredSlice && m_PieEnabled != DISABLED)
    {
        // If used pressed primary button or released secondary when hovering over a slice, then activate it
        if (m_pController->IsState(PRESS_PRIMARY))
        {
            if (m_pHoveredSlice->m_Enabled)
            {
                m_pActivatedSlice = m_pHoveredSlice;
                m_AlreadyActivated = m_pActivatedSlice->m_SliceType;
				g_GUISound.SlicePickedSound()->Play(m_pController->GetPlayer());
            }
            else
				g_GUISound.DisabledPickedSound()->Play(m_pController->GetPlayer());
            // Reset the digital input hold timer so the hover stays here
            m_DInputHoldTimer.Reset();
        }

        // Only allow release of pie menu button to activate a slice IF it hasn't already been activated with the primary button!
        if (m_AlreadyActivated != m_pHoveredSlice->m_SliceType &&  m_pController->IsState(RELEASE_SECONDARY))
        {
            if (m_pHoveredSlice->m_Enabled)
            {
                m_pActivatedSlice = m_pHoveredSlice;
                m_AlreadyActivated = m_pActivatedSlice->m_SliceType;
				g_GUISound.SlicePickedSound()->Play(m_pController->GetPlayer());
            }
            else
				g_GUISound.DisabledPickedSound()->Play(m_pController->GetPlayer());
        }

        // Reset the timer so we can measure how long the cursor has been neutral before letting go of the hover
        if (m_EnoughInput)
            m_HoverTimer.Reset();
    }
    
    // Clear the hover slice if we've been without enough input magnitude for a while
    if (!m_EnoughInput && m_pHoveredSlice && m_HoverTimer.IsPastRealMS(500))
    {
        m_pHoveredSlice = 0;
        m_RedrawBG = true;
    }

    // Redraw the background circle, if necessary
    if (m_RedrawBG && m_PieEnabled != DISABLED)
    {
        // Clear it out
        clear_to_color(m_pBGBitmap, g_MaskColor);

        int centerX = m_pBGBitmap->w / 2;
        int centerY = m_pBGBitmap->h / 2;
        // Do outer circle
//        circlefill(m_pBGBitmap, centerX, centerY, m_InnerRadius + m_Thickness, g_BlackColor);
        circlefill(m_pBGBitmap, centerX, centerY, m_InnerRadius + m_Thickness, 4);
        // Remove inner circle
        circlefill(m_pBGBitmap, centerX, centerY, m_InnerRadius, g_MaskColor);

        // Draw the separator lines, cutting up the circle into slices, only if fully enabled
        if (m_PieEnabled == ENABLED)
        {
            Vector separator;
            for (vector<Slice *>::iterator sItr = m_AllSlices.begin(); sItr != m_AllSlices.end(); ++sItr)
            {
                separator.SetXY(m_InnerRadius + m_Thickness + 2, 0);
                separator.RadRotate((*sItr)->m_AreaStart);
                // Draw four so that the result will be at least 2px thick, no matter what angle
                line(m_pBGBitmap, centerX, centerY, centerX + separator.GetCeilingIntX(), centerY + separator.GetCeilingIntY(), g_MaskColor);
                line(m_pBGBitmap, centerX + 1, centerY, centerX + 1 + separator.GetCeilingIntX(), centerY + separator.GetCeilingIntY(), g_MaskColor);
                line(m_pBGBitmap, centerX, centerY + 1, centerX + separator.GetCeilingIntX(), centerY + 1 + separator.GetCeilingIntY(), g_MaskColor);
                line(m_pBGBitmap, centerX + 1, centerY + 1, centerX + 1 + separator.GetCeilingIntX(), centerY + 1 + separator.GetCeilingIntY(), g_MaskColor);
            }

            // Indicate the highlighted segment, only if it is also enabled?
            if (m_pHoveredSlice && m_pHoveredSlice->m_Enabled)
            {
                separator.SetXY(m_InnerRadius + (m_Thickness / 2), 0);
                separator.RadRotate(m_pHoveredSlice->m_MidAngle);
    //            floodfill(m_pBGBitmap, centerX + separator.GetFloorIntX(), centerY + separator.GetFloorIntY(), 122);
                floodfill(m_pBGBitmap, centerX + separator.GetFloorIntX(), centerY + separator.GetFloorIntY(), m_pHoveredSlice->m_Enabled ? g_BlackColor : g_RedColor);
            }
        }

        m_RedrawBG = false;
    }

    // Save the pie slice command
    if (m_pActivatedSlice)
    {
        m_PieCommand = m_pActivatedSlice->m_SliceType;
        // If the activated command is a scripted one, then run the script!
        if (m_PieCommand == PSI_SCRIPTED)
        {
            // Pass in the Actor this menu may be over when running the function
            g_LuaMan.SetTempEntity(m_pActor);
// TODO: Really reload the file every time? - YES ITS HOT
            g_LuaMan.RunScriptFile(m_pActivatedSlice->GetScriptPath());
            // Run the actual function this slice is supposed to run, passing in the Actor this is currently hovering over
            g_LuaMan.RunScriptString(m_pActivatedSlice->GetFunctionName() + "(ToActor(LuaMan.TempEntity));");
        }
    }
    else
        m_PieCommand = PSI_NONE;

    // Clear out the actor each frame, because we can't be sure it hasn't been deleted by next
    m_pActor = 0;
	// Delete last known actor if it's not an actor anymore, or dead or something else
	if (m_pLastKnownActor && (!g_MovableMan.IsActor(m_pLastKnownActor) || !dynamic_cast<Actor *>(m_pLastKnownActor)))
		m_pLastKnownActor = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the menu

void PieMenuGUI::Draw(BITMAP *pTargetBitmap, const Vector &targetPos) const
{
    Vector drawPos = m_CenterPos - targetPos;

//    GUIFont *pFont = g_FrameMan.GetSmallFont();
    GUIFont *pFont = g_FrameMan.GetLargeFont();

    // Adjust the draw position to work if drawn to a target screen bitmap that is straddling a scene seam
    if (!targetPos.IsZero())
    {
        // Get all possible wrapped boxes of the screen
        Box screenBox(targetPos, pTargetBitmap->w, pTargetBitmap->h);
        list<Box> wrappedBoxes;
        g_SceneMan.WrapBox(screenBox, wrappedBoxes);

        // Iterate through the wrapped screen boxes - will only be one if there's no wrapping
        // Try to the find one that contains the objective point
        bool withinAny = false;
        list<Box>::iterator nearestBoxItr = wrappedBoxes.begin();
        float distance, shortestDist = 1000000.0;
        for (list<Box>::iterator wItr = wrappedBoxes.begin(); wItr != wrappedBoxes.end(); ++wItr)
        {
            // See if we found the point to be within the screen or not
            if (wItr->IsWithinBox(m_CenterPos))
            {
                nearestBoxItr = wItr;
                withinAny = true;
                break;
            }
            // Well, which wrapped screen box is closest to the point?
            distance = g_SceneMan.ShortestDistance(wItr->GetCenter(), m_CenterPos).GetLargest();
            if (distance < shortestDist)
            {
                shortestDist = distance;
                nearestBoxItr = wItr;
            }
        }

        // Get the difference that the wrapped screen has from the actual one
        Vector wrappingOffset = screenBox.GetCorner() - nearestBoxItr->GetCorner();
        // Apply that offet to the menu's center scene position
        drawPos += wrappingOffset;

        // Menu is not within the screen, so place it accordingly to the closest screen box
        if (!withinAny)
        {
            // Figure out which point is closest to the box, taking scene wrapping into account
            drawPos = nearestBoxItr->GetCenter() + g_SceneMan.ShortestDistance(nearestBoxItr->GetCenter(), m_CenterPos) - targetPos;
        }
/*
        // Spans vertical scene seam
        int sceneWidth = g_SceneMan.GetSceneWidth();
        if (g_SceneMan.SceneWrapsX() && pTargetBitmap->w < sceneWidth)
        {
            if ((targetPos.m_X < 0) && (m_CenterPos.m_X > (sceneWidth - pTargetBitmap->w)))
                drawPos.m_X -= sceneWidth;
            else if (((targetPos.m_X + pTargetBitmap->w) > sceneWidth) && (m_CenterPos.m_X < pTargetBitmap->w))
                drawPos.m_X += sceneWidth;
        }
        // Spans horizontal scene seam
        int sceneHeight = g_SceneMan.GetSceneHeight();
        if (g_SceneMan.SceneWrapsY() && pTargetBitmap->h < sceneHeight)
        {
            if ((targetPos.m_Y < 0) && (m_CenterPos.m_Y > (sceneHeight - pTargetBitmap->h)))
                drawPos.m_Y -= sceneHeight;
            else if (((targetPos.m_Y + pTargetBitmap->h) > sceneHeight) && (m_CenterPos.m_Y < pTargetBitmap->h))
                drawPos.m_Y += sceneHeight;
        }
*/
    }

    // Adjust the draw position so that the menu will always be drawn fully inside the player's screen
    int menuDrawRadius = m_InnerRadius + m_Thickness + 2 + pFont->GetFontHeight();
    if (drawPos.m_X - menuDrawRadius < 0)
        drawPos.m_X = menuDrawRadius;
    else if (drawPos.m_X + menuDrawRadius > pTargetBitmap->w)
        drawPos.m_X = pTargetBitmap->w - menuDrawRadius;

    if (drawPos.m_Y - menuDrawRadius < 0)
        drawPos.m_Y = menuDrawRadius;
    else if (drawPos.m_Y + menuDrawRadius > pTargetBitmap->h)
        drawPos.m_Y = pTargetBitmap->h - menuDrawRadius;

	// Draw selected actor's inventory if it exists, alive and actually an actor
	if (m_PieEnabled == ENABLED && m_pLastKnownActor && g_MovableMan.IsActor(m_pLastKnownActor) && dynamic_cast<AHuman *>(m_pLastKnownActor))
	{
		const std::deque<MovableObject *> * pInventory = m_pLastKnownActor->GetInventory();

		float height = 0;
		float width = 0;
		float inventoryMass = 0;

		AllegroBitmap allegroBitmap(pTargetBitmap);
	    GUIFont *pSymbolFont = g_FrameMan.GetLargeFont();

		// Calculate the max height of inventory panel
		for (deque<MovableObject *>::const_iterator gItr = pInventory->begin(); gItr < pInventory->end(); ++gItr)
			height = (*gItr)->GetGraphicalIcon()->h > height ? (*gItr)->GetGraphicalIcon()->h : height;

		float actorMass = m_pLastKnownActor->GetMass();

		// Setup item display
		float x = drawPos.m_X;
		float y = drawPos.m_Y - (m_EnabledRadius + m_Thickness + height + 10);
		float gap = 2;
		int half = pInventory->size() / 2;
		float jetImpulseFactor = 0;
		char str[4];

		if (pInventory->size() == 1)
			half = 1;

		// Draw held item
		AHuman *pHuman = dynamic_cast<AHuman *>(m_pLastKnownActor);
		if (pHuman) 
		{
			MovableObject *pItem = pHuman->GetEquippedItem();
			if (pItem)
			{
				// Adjust max height for carried item if necessary
				height = pItem->GetGraphicalIcon()->h > height ? pItem->GetGraphicalIcon()->h : height;

				// Draw item
				BITMAP * pIcon = pItem->GetGraphicalIcon();
				draw_sprite(pTargetBitmap, pIcon, x - pIcon->w / 2, y);
				width = pIcon->w;

				// Draw weight indicator
				float itemMass = pItem->GetMass();
				str[0] = itemMass > 25 ? -29 : (itemMass > 15 ? -30 : -31);
				str[1] = 0;
				pSymbolFont->DrawAligned(&allegroBitmap, x, y - height - 6, str, GUIFont::Centre);
			}
		}

		/*ACrab *pCrab = dynamic_cast<ACrab *>(m_pLastKnownActor);
		if (pCrab) 
		{
			//Calculate estimated jetpack impulse
			AEmitter *pJetpack = pCrab->GetJetpack();
			if (pJetpack && pJetpack->IsAttached())
				jetImpulseFactor = pJetpack->EstimateImpulse(false) * c_PPM / g_TimerMan.GetDeltaTimeSecs();
		}*/

		x += width / 2 + gap * 2;
		
		// Draw right part of the list
		for (int i = 0; i < half; ++i)
		{
			BITMAP * pIcon = pInventory->at(i)->GetGraphicalIcon();
			//rect(pTargetBitmap, x - gap, y - gap, x + pIcon->w + gap, y + height + gap, 5);


			draw_sprite(pTargetBitmap, pIcon, x, y); 

			// Draw weight indicator
			float itemMass = pInventory->at(i)->GetMass();
			str[0] = itemMass > 25 ? -29 : (itemMass > 15 ? -30 : -31);
			str[1] = 0;
			pSymbolFont->DrawAligned(&allegroBitmap, x + pIcon->w / 2, y - height - 6, str, GUIFont::Centre);

			// Move to draw next item
			x += pIcon->w + gap * 2;
		}

		//Draw left part of the list
		x = drawPos.m_X - width / 2;

		for (int i = pInventory->size() - 1; i >= half; --i)
		{
			BITMAP * pIcon = pInventory->at(i)->GetGraphicalIcon();
			//Move to draw thi item
			x -= pIcon->w + 4;
			//rect(pTargetBitmap, x - gap, y - gap, x + pIcon->w + gap, y + height + gap, 5);
			draw_sprite(pTargetBitmap, pIcon, x, y); 

			// Draw weight indicator
			float itemMass = pInventory->at(i)->GetMass();
			str[0] = itemMass > 25 ? -29 : (itemMass > 15 ? -30 : -31);
			str[1] = 0;
			pSymbolFont->DrawAligned(&allegroBitmap, x + pIcon->w / 2, y - height - 6, str, GUIFont::Centre);
		}
	}

    // Don't draw bg cirlce if completely disabled
    if (m_PieEnabled != DISABLED)
    {
        // Blit the background circle, half transparent
		if (!g_FrameMan.IsInMultiplayerMode())
		{
			g_FrameMan.SetTransTable(MoreTrans);
			draw_trans_sprite(pTargetBitmap, m_pBGBitmap, drawPos.m_X - m_pBGBitmap->w / 2, drawPos.m_Y - m_pBGBitmap->h / 2);
		}
		else 
		{
			draw_sprite(pTargetBitmap, m_pBGBitmap, drawPos.m_X - m_pBGBitmap->w / 2, drawPos.m_Y - m_pBGBitmap->h / 2);
		}
    }

    // Only draw details if fully enabled
    if (m_PieEnabled == ENABLED)
    {
        // Draw all the icons
        Vector iconPos;
        BITMAP *pIcon = 0;
        for (vector<Slice *>::const_iterator sItr = m_AllSlices.begin(); sItr != m_AllSlices.end(); ++sItr)
        {
            // Check for available Icon
            pIcon = 0;
            if ((*sItr)->m_Icon.GetBitmaps8() && (*sItr)->m_Icon.GetFrameCount() > 0)
            {
                // If we have the disabled etc frames, then use em if applicable
                pIcon = (*sItr)->m_Icon.GetBitmaps8()[(!((*sItr)->m_Enabled) && (*sItr)->m_Icon.GetFrameCount() > PIS_DISABLED) ? PIS_DISABLED : (((*sItr) == m_pHoveredSlice && (*sItr)->m_Icon.GetFrameCount() > PIS_SELECTED) ? PIS_SELECTED : PIS_NORMAL)];
                // Position and draw the icon bitmap
                iconPos.SetXY(m_InnerRadius + (m_Thickness / 2), 0);
                iconPos.RadRotate((*sItr)->m_MidAngle);
                iconPos += Vector(1, 1);
                draw_sprite(pTargetBitmap, pIcon, drawPos.m_X + iconPos.m_X - (pIcon->w / 2), drawPos.m_Y + iconPos.m_Y - (pIcon->h / 2)); 
            }
        }

        // Draw the cursor arrow
        if (m_EnoughInput)
        {
            Vector cursorPos(m_InnerRadius/* - s_pCursor->w*/, 0);
            cursorPos.RadRotate(m_CursorAng);
            pivot_sprite(pTargetBitmap, s_pCursor, drawPos.m_X + cursorPos.m_X, drawPos.m_Y + cursorPos.m_Y, s_pCursor->w / 2, s_pCursor->h / 2,  ftofix((m_CursorAng / c_PI) * -128));
    //        g_SceneMan.RegisterPostEffect(cursorPos, s_pCursorGlow, 
        }

        // Draw the description text over the hovered over slice
        if (m_pHoveredSlice)
        {
            AllegroBitmap allegroBitmap(pTargetBitmap);

            Vector textPos(m_InnerRadius + m_Thickness + pFont->GetFontHeight() * 0.5, 0);
            textPos.RadRotate(m_pHoveredSlice->m_MidAngle);
            // Adjust for font height
            textPos.m_Y -= pFont->GetFontHeight() * 0.45;

            // If the UP or DOWN slice, then center the text
            if (m_pHoveredSlice == &m_UpSlice || m_pHoveredSlice == &m_DownSlice)
                pFont->DrawAligned(&allegroBitmap, drawPos.m_X + textPos.m_X, drawPos.m_Y + textPos.m_Y, m_pHoveredSlice->m_Description.c_str(), GUIFont::Centre);
            // Right side, so align text left
            else if (m_CursorAng < c_HalfPI || m_CursorAng > c_PI + c_HalfPI)
                pFont->DrawAligned(&allegroBitmap, drawPos.m_X + textPos.m_X, drawPos.m_Y + textPos.m_Y, m_pHoveredSlice->m_Description.c_str(), GUIFont::Left);
            // Left side, so align text right
            else
                pFont->DrawAligned(&allegroBitmap, drawPos.m_X + textPos.m_X, drawPos.m_Y + textPos.m_Y, m_pHoveredSlice->m_Description.c_str(), GUIFont::Right);
        }
    }
/*
    Vector mouseDir(m_InnerRadius * m_pController->GetAnalogCursor().GetMagnitude(), 0);
    mouseDir.RadRotate(m_CursorAng);
    line(pTargetBitmap, drawPos.GetFloorIntX(), drawPos.GetFloorIntY(), drawPos.GetFloorIntX() + mouseDir.m_X, drawPos.GetFloorIntY() + mouseDir.m_Y, 122);
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SelectSlice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a slice to be selected.

bool PieMenuGUI::SelectSlice(Slice *pSelected, bool moveCursor)
{
    bool diffSlice = false;
    // If new slice selected, can't be considered to already have been activated
    if (pSelected != m_pHoveredSlice)
    {
        m_RedrawBG = true;
        diffSlice = true;
        if (pSelected->m_Enabled)
			g_GUISound.HoverChangeSound()->Play(m_pController->GetPlayer());
        else
			g_GUISound.HoverDisabledSound()->Play(m_pController->GetPlayer());
    }

    m_pHoveredSlice = pSelected;

    if (m_pHoveredSlice && moveCursor)
        m_CursorAng = m_pHoveredSlice->m_MidAngle;

    return m_pHoveredSlice && diffSlice;
}
