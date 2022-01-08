//////////////////////////////////////////////////////////////////////////////////////////
// File:            HeldDevice.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the HeldDevice class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "HeldDevice.h"
#include "MovableMan.h"
#include "AtomGroup.h"
#include "Arm.h"
#include "AHuman.h"

#include "GameActivity.h"

#include "GUI.h"
#include "AllegroBitmap.h"

#include "SettingsMan.h"

namespace RTE {

ConcreteClassInfo(HeldDevice, Attachable, 50);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this HeldDevice, effectively
//                  resetting the members of this abstraction level only.

void HeldDevice::Clear()
{
    m_HeldDeviceType = WEAPON;
    m_IsExplosiveWeapon = false;
    m_Activated = false;
    m_ActivationTimer.Reset();
    m_OneHanded = false;
	m_DualWieldable = false;
    m_StanceOffset.Reset();
    m_SharpStanceOffset.Reset();
    m_SharpAim = 0.0F;
    m_MaxSharpLength = 0;
    m_Supported = false;
    m_SupportOffset.Reset();
	m_SeenByPlayer.fill(false);
    m_IsUnPickupable = false;
    m_PickupableByPresetNames.clear();
    m_GripStrengthMultiplier = 1.0F;
    m_BlinkTimer.Reset();
	m_BlinkTimer.SetSimTimeLimitMS(1000);
    m_PieSlices.clear();
    m_Loudness = -1;

    // NOTE: This special override of a parent class member variable avoids needing an extra variable to avoid overwriting INI values.
    m_CollidesWithTerrainWhileAttached = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Round object ready for use.

int HeldDevice::Create()
{
    if (Attachable::Create() < 0)
        return -1;

    // Set MO Type.
    m_MOType = MovableObject::TypeHeldDevice;

    // Set HeldDeviceType based on tags
    if (IsInGroup("Weapons"))
        m_HeldDeviceType = WEAPON;
    else if (IsInGroup("Tools"))
        m_HeldDeviceType = TOOL;
    else if (IsInGroup("Shields"))
        m_HeldDeviceType = SHIELD;

    if (IsInGroup("Weapons - Explosive"))
        m_IsExplosiveWeapon = true;
    else
        m_IsExplosiveWeapon = false;
    
    // Backwards compatibility so that the tag is added for sure
    if (m_HeldDeviceType == WEAPON)
        AddToGroup("Weapons");
    else if (m_HeldDeviceType == TOOL)
        AddToGroup("Tools");
    else if (m_HeldDeviceType == SHIELD)
        AddToGroup("Shields");
    
    // No Loudness set in the ini-file
    if (m_Loudness < 0)
    {
        if (m_HeldDeviceType == TOOL)
            m_Loudness = 0.5;   // Force tools to make less noise
        else
            m_Loudness = 1.0;
    }

    // Make it so held devices are dropped gently when their parent gibs
    m_ParentGibBlastStrengthMultiplier = 0.0F;

    // Make it so users can't accidentally set this to true for HeldDevices, since it'll cause crashes when swapping inventory items around.
    m_DeleteWhenRemovedFromParent = false;
    
    // All HeldDevice:s by default avoid hitting and getting physically hit by AtomGoups when they are at rest
    m_IgnoresAGHitsWhenSlowerThan = 1.0;
    
    // By default, held items should not be able to be squished and destroyed into the ground at all
    m_CanBeSquished = false;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a HeldDevice to be identical to another, by deep copy.

int HeldDevice::Create(const HeldDevice &reference)
{
    Attachable::Create(reference);

    // Set MO Type.
    m_MOType = MovableObject::TypeHeldDevice;

    m_HeldDeviceType = reference.m_HeldDeviceType;

    m_Activated = reference.m_Activated;
    m_ActivationTimer = reference.m_ActivationTimer;

    m_OneHanded = reference.m_OneHanded;
	m_DualWieldable = reference.m_DualWieldable;
    m_StanceOffset = reference.m_StanceOffset;
    m_SharpStanceOffset = reference.m_SharpStanceOffset;
    m_SupportOffset = reference.m_SupportOffset;
    m_IsUnPickupable = reference.m_IsUnPickupable;
    for (std::string referenceActorWhoCanPickThisUp : reference.m_PickupableByPresetNames) {
        m_PickupableByPresetNames.insert(referenceActorWhoCanPickThisUp);
    }
    m_GripStrengthMultiplier = reference.m_GripStrengthMultiplier;

    m_SharpAim = reference.m_SharpAim;
    m_MaxSharpLength = reference.m_MaxSharpLength;
    m_Supported = reference.m_Supported;
    m_Loudness = reference.m_Loudness;

    for (list<PieSlice>::const_iterator itr = reference.m_PieSlices.begin(); itr != reference.m_PieSlices.end(); ++itr)
        m_PieSlices.push_back(*itr);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int HeldDevice::ReadProperty(const std::string_view &propName, Reader &reader)
{
    if (propName == "HeldDeviceType")
        reader >> m_HeldDeviceType;
    else if (propName == "OneHanded")
        reader >> m_OneHanded;
	else if (propName == "DualWieldable")
		reader >> m_DualWieldable;
	else if (propName == "StanceOffset")
        reader >> m_StanceOffset;
    else if (propName == "SharpStanceOffset")
        reader >> m_SharpStanceOffset;
    else if (propName == "SupportOffset")
        reader >> m_SupportOffset;
    else if (propName == "PickupableBy") {
        std::string pickupableByValue = reader.ReadPropValue();
        if (pickupableByValue == "PickupableByEntries") {
            while (reader.NextProperty()) {
                std::string pickupableByEntryType = reader.ReadPropName();
                if (pickupableByEntryType == "AddPresetNameEntry") {
                    m_PickupableByPresetNames.insert(reader.ReadPropValue());
                } else if (pickupableByEntryType == "AddClassNameEntry ") {
                    reader.ReportError("AddClassNameEntry is not yet supported.");
                } else if (pickupableByEntryType == "AddGroupEntry") {
                    reader.ReportError("AddGroupEntry is not yet supported.");
                } else if (pickupableByEntryType == "AddDataModuleEntry ") {
                    reader.ReportError("AddDataModuleEntry is not yet supported.");
                } else {
                    break;
                }
            }
        } else if (pickupableByValue == "None") {
            SetUnPickupable(true);
        }
    } else if (propName == "GripStrengthMultiplier") {
        reader >> m_GripStrengthMultiplier;
    } else if (propName == "SharpLength")
        reader >> m_MaxSharpLength;
    else if (propName == "Loudness")
        reader >> m_Loudness;
    else if (propName == "AddPieSlice")
    {
        PieSlice newSlice;
        reader >> newSlice;
        m_PieSlices.push_back(newSlice);
		PieMenuGUI::StoreCustomLuaPieSlice(newSlice);
    }
    else
        return Attachable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this HeldDevice with a Writer for
//                  later recreation with Create(Reader &reader);

int HeldDevice::Save(Writer &writer) const
{
    Attachable::Save(writer);
/*
    writer.NewLine();
    writer << "// 0 = Offensive Weapon, 1 = Tool, 2 = Shield";
    writer.NewProperty("HeldDeviceType");
    writer << m_HeldDeviceType;
*/
    writer.NewProperty("OneHanded");
    writer << m_OneHanded;
    writer.NewProperty("StanceOffset");
    writer << m_StanceOffset;
    writer.NewProperty("SharpStanceOffset");
    writer << m_SharpStanceOffset;
    writer.NewProperty("SupportOffset");
    writer << m_SupportOffset;
    writer.NewProperty("GripStrengthMultiplier");
    writer << m_GripStrengthMultiplier;
    writer.NewProperty("SharpLength");
    writer << m_MaxSharpLength;
    writer.NewProperty("Loudness");
    writer << m_Loudness;
    for (list<PieSlice>::const_iterator itr = m_PieSlices.begin(); itr != m_PieSlices.end(); ++itr)
    {
        writer.NewProperty("AddPieSlice");
        writer << *itr;
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the HeldDevice object.

void HeldDevice::Destroy(bool notInherited)
{

    if (!notInherited)
        Attachable::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetStanceOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current position offset of this HeldDevice's joint relative
//                  from the parent Actor's position, if attached.
// Arguments:       None.
// Return value:    A const reference to the current stance parent offset.

Vector HeldDevice::GetStanceOffset() const
{
	if (m_SharpAim > 0) {
		float rotAngleScalar = std::abs(std::sin(GetRootParent()->GetRotAngle()));
		// Deviate the vertical axis towards regular StanceOffset based on the user's rotation so that sharp aiming doesn't look awkward when prone
		return Vector(m_SharpStanceOffset.GetX(), m_SharpStanceOffset.GetY() * (1.0F - rotAngleScalar) + m_StanceOffset.GetY() * rotAngleScalar).GetXFlipped(m_HFlipped);
	}
    else
        return m_StanceOffset.GetXFlipped(m_HFlipped);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSupportPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absolute position of the support handhold that this HeldDevice
//                  offers.

Vector HeldDevice::GetSupportPos() const
{
/*
    Vector rotOff(m_SupportOffset.GetYFlipped(m_HFlipped));
    rotOff.RadRotate(m_HFlipped ? (c_PI + m_Rotation) : m_Rotation);
    return m_Pos + rotOff;
*/
    return m_Pos + RotateOffset(m_SupportOffset);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMagazinePos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absolute position of the magazine or other equivalent point of
//                  this.

Vector HeldDevice::GetMagazinePos() const
{
    return m_Pos;    
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HeldDevice::RemovePickupableByPresetName(const std::string &actorPresetName) {
    std::unordered_set<std::string>::iterator pickupableByPresetNameEntry = m_PickupableByPresetNames.find(actorPresetName);
    if (pickupableByPresetNameEntry != m_PickupableByPresetNames.end()) { m_PickupableByPresetNames.erase(pickupableByPresetNameEntry); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  AddPieMenuSlices
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds all slices this needs on a pie menu.

bool HeldDevice::AddPieMenuSlices(PieMenuGUI *pPieMenu)
{
    // Add the custom scripted options of this specific device
    for (list<PieSlice>::iterator itr = m_PieSlices.begin(); itr != m_PieSlices.end(); ++itr)
        pPieMenu->AddSlice(*itr);

    return false;
}



//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CollideAtPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the collision response when another MO's Atom collides with
//                  this MO's physical representation. The effects will be applied
//                  directly to this MO, and also represented in the passed in HitData.

bool HeldDevice::CollideAtPoint(HitData &hd)
{
    return Attachable::CollideAtPoint(hd);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Activate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Activates this HDFirearm. Analogous to 'pulling the trigger'.

void HeldDevice::Activate()
{
    if (!m_Activated) { m_ActivationTimer.Reset(); }
    m_Activated = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Deactivate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Deactivates one of this HDFirearm's features. Analogous to 'releasing
//                  the trigger'.

void HeldDevice::Deactivate()
{
    m_Activated = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  OnMOHit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits another MO.
//                  This is called by the owned Atom/AtomGroup of this MovableObject during
//                  travel.

bool HeldDevice::OnMOHit(MovableObject *pOtherMO)
{
    // Don't terminate, continue on
    return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HeldDevice::TransferJointImpulses(Vector &jointImpulses, float jointStiffnessValueToUse, float jointStrengthValueToUse, float gibImpulseLimitValueToUse) {
    MovableObject *parent = m_Parent;
    if (!parent) {
        return false;
    }
    if (m_ImpulseForces.empty()) {
        return true;
    }
    const Arm *parentAsArm = dynamic_cast<Arm *>(parent);
    if (parentAsArm && parentAsArm->GetGripStrength() > 0 && jointStrengthValueToUse < 0) {
        jointStrengthValueToUse = parentAsArm->GetGripStrength() * m_GripStrengthMultiplier;
        if (m_Supported) {
            const AHuman *rootParentAsAHuman = dynamic_cast<AHuman *>(GetRootParent());
            if (rootParentAsAHuman != nullptr) { jointStrengthValueToUse += rootParentAsAHuman->GetBGArm() ? rootParentAsAHuman->GetBGArm()->GetGripStrength() * m_GripStrengthMultiplier : 0.0F; }
        }
    }
    bool intact = Attachable::TransferJointImpulses(jointImpulses, jointStiffnessValueToUse, jointStrengthValueToUse, gibImpulseLimitValueToUse);
    if (!intact) {
        Actor *rootParentAsActor = dynamic_cast<Actor *>(parent->GetRootParent());
        if (rootParentAsActor && rootParentAsActor->GetStatus() == Actor::STABLE) { rootParentAsActor->SetStatus(Actor::UNSTABLE); }
    }
    return intact;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Travel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Travels this, using its physical representation.
// Arguments:       None.
// Return value:    None.

void HeldDevice::Travel()
{
    Attachable::Travel();
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this HeldDevice. Supposed to be done every frame.

void HeldDevice::Update()
{
    Attachable::Update();

    // Remove loose items that have completely disappeared into the terrain, unless they're pinned
    if (!m_Parent && m_PinStrength <= 0 && m_RestTimer.IsPastSimMS(20000) && m_CanBeSquished && m_pAtomGroup->RatioInTerrain() > 0.9)
        GibThis();

    if (m_Activated)
        m_RestTimer.Reset();

    ////////////////////////////////////////
    // Animate the sprite, if applicable

    if (m_FrameCount > 1)
    {
        if (m_SpriteAnimMode == LOOPWHENACTIVE && m_Activated)
        {
            float cycleTime = ((long)m_SpriteAnimTimer.GetElapsedSimTimeMS()) % m_SpriteAnimDuration;
            m_Frame = std::floor((cycleTime / (float)m_SpriteAnimDuration) * (float)m_FrameCount);
        }
    }

    if (!m_Parent) {

    }
    else {
        /////////////////////////////////
        // Update and apply rotations and scale

        // Taken care of by holder/owner Arm.
//        m_Pos += m_ParentOffset;
// Don't apply state changes to BITMAP anywhere else than Draw().
//        m_aSprite->SetAngle(m_Rotation);
//        m_aSprite->SetScale(m_Scale);
    }

	if (m_BlinkTimer.IsPastSimTimeLimit()) { m_BlinkTimer.Reset(); }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this HeldDevice's current graphical representation to a
//                  BITMAP of choice.

void HeldDevice::Draw(BITMAP *pTargetBitmap,
                      const Vector &targetPos,
                      DrawMode mode,
                      bool onlyPhysical) const
{
    Attachable::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
/*
    // Draw suporting hand if applicable.
    if (m_Supported) {
        Vector handPos(m_Pos.GetFloored() +
                       RotateOffset(m_SupportOffset) +
                       (m_Recoiled ? m_RecoilOffset : Vector()) -
                       targetPos);
        handPos.m_X -= m_pSupportHand->GetWidth() >> 1;
        handPos.m_Y -= m_pSupportHand->GetHeight() >> 1;
        if (!m_HFlipped)
            m_pSupportHand->DrawTrans(pTargetBitmap, handPos.m_X, handPos.m_Y);
        else
            m_pSupportHand->DrawTransHFlip(pTargetBitmap, handPos.m_X, handPos.m_Y);
    }
*/
/*
#ifdef DEBUG_BUILD
    if (mode == g_DrawColor && !onlyPhysical)
    {
        m_pAtomGroup->Draw(pTargetBitmap, targetPos, false, 122);
        m_pDeepGroup->Draw(pTargetBitmap, targetPos, false, 13);
    }
#endif
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Actor's current graphical HUD overlay representation to a
//                  BITMAP of choice.

void HeldDevice::DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos, int whichScreen, bool playerControlled) {
	if (!m_HUDVisible) {
		return;
	}

    Attachable::DrawHUD(pTargetBitmap, targetPos, whichScreen);

	if (!IsUnPickupable()) {
		if (m_Parent) {
			m_SeenByPlayer.fill(false);
			m_BlinkTimer.Reset();
		} else {
			// Only draw if the team viewing this has seen the space where this is located.
			int viewingPlayer = g_ActivityMan.GetActivity()->PlayerOfScreen(whichScreen);
			int viewingTeam = g_ActivityMan.GetActivity()->GetTeamOfPlayer(viewingPlayer);
			if (viewingTeam == Activity::NoTeam || g_SceneMan.IsUnseen(m_Pos.GetFloorIntX(), m_Pos.GetFloorIntY(), viewingTeam)) {
				return;
			}

			Vector drawPos = m_Pos - targetPos;
			// Adjust the draw position to work if drawn to a target screen bitmap that is straddling a scene seam.
			if (!targetPos.IsZero()) {
				int sceneWidth = g_SceneMan.GetSceneWidth();
				if (g_SceneMan.SceneWrapsX() && pTargetBitmap->w < sceneWidth) {
					if ((targetPos.GetFloorIntX() < 0) && (m_Pos.GetFloorIntX() > (sceneWidth - pTargetBitmap->w))) {
						drawPos.m_X -= static_cast<float>(sceneWidth);
					} else if ((targetPos.GetFloorIntX() + pTargetBitmap->w > sceneWidth) && (m_Pos.GetFloorIntX() < pTargetBitmap->w)) {
						drawPos.m_X += static_cast<float>(sceneWidth);
					}
				}
				int sceneHeight = g_SceneMan.GetSceneHeight();
				if (g_SceneMan.SceneWrapsY() && pTargetBitmap->h < sceneHeight) {
					if ((targetPos.GetFloorIntY() < 0) && (m_Pos.GetFloorIntY() > (sceneHeight - pTargetBitmap->h))) {
						drawPos.m_Y -= static_cast<float>(sceneHeight);
					} else if ((targetPos.GetFloorIntY() + pTargetBitmap->h > sceneHeight) && (m_Pos.GetFloorIntY() < pTargetBitmap->h)) {
						drawPos.m_Y += static_cast<float>(sceneHeight);
					}
				}
			}

			GUIFont *pSymbolFont = g_FrameMan.GetLargeFont();
			GUIFont *pTextFont = g_FrameMan.GetSmallFont();
			if (pSymbolFont && pTextFont) {
				const Activity *activity = g_ActivityMan.GetActivity();
				float unheldItemDisplayRange = activity->GetActivityState() == Activity::ActivityState::Running ? g_SettingsMan.GetUnheldItemsHUDDisplayRange() : -1.0F;
				if (g_SettingsMan.AlwaysDisplayUnheldItemsInStrategicMode()) {
					const GameActivity *gameActivity = dynamic_cast<const GameActivity *>(activity);
					if (gameActivity && gameActivity->GetViewState(viewingPlayer) == GameActivity::ViewState::ActorSelect) { unheldItemDisplayRange = -1.0F; }
				}
				// Note - to avoid item HUDs flickering in and out, we need to add a little leeway when hiding them if they're already displayed.
				if (m_SeenByPlayer.at(viewingPlayer) && unheldItemDisplayRange > 0) { unheldItemDisplayRange += 3.0F; }
				m_SeenByPlayer.at(viewingPlayer) = unheldItemDisplayRange < 0 || (unheldItemDisplayRange > 0 && g_SceneMan.ShortestDistance(m_Pos, g_SceneMan.GetScrollTarget(whichScreen), g_SceneMan.SceneWrapsX()).GetMagnitude() < unheldItemDisplayRange);

				if (m_SeenByPlayer.at(viewingPlayer)) {
					char pickupArrowString[64];
					pickupArrowString[0] = 0;
					if (m_BlinkTimer.GetElapsedSimTimeMS() < 250) {
						pickupArrowString[0] = 0;
					} else if (m_BlinkTimer.GetElapsedSimTimeMS() < 500) {
						pickupArrowString[0] = -42;
						pickupArrowString[1] = 0;
					} else if (m_BlinkTimer.GetElapsedSimTimeMS() < 750) {
						pickupArrowString[0] = -41;
						pickupArrowString[1] = 0;
					} else if (m_BlinkTimer.GetElapsedSimTimeMS() < 1000) {
						pickupArrowString[0] = -40;
						pickupArrowString[1] = 0;
					}

					AllegroBitmap targetAllegroBitmap(pTargetBitmap);
					pSymbolFont->DrawAligned(&targetAllegroBitmap, drawPos.GetFloorIntX() - 1, drawPos.GetFloorIntY() - 20, pickupArrowString, GUIFont::Centre);
					pTextFont->DrawAligned(&targetAllegroBitmap, drawPos.GetFloorIntX(), drawPos.GetFloorIntY() - 29, m_PresetName, GUIFont::Centre);
				}
			}
		}
	}
}

} // namespace RTE
