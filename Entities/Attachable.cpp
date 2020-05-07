//////////////////////////////////////////////////////////////////////////////////////////
// File:            Attachable.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Attachable class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Attachable.h"
#include "Atom.h"
#include "AtomGroup.h"
#include "RTEManagers.h"
#include "RTETools.h"
#include "AEmitter.h"
#include "Actor.h"

namespace RTE {

CONCRETECLASSINFO(Attachable, MOSRotating, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Attachable, effectively
//                  resetting the members of this abstraction level only.

void Attachable::Clear()
{
    m_pParent = 0;
    m_ParentOffset.Reset();
    m_JointStrength = 10;
    m_JointStiffness = 1.0;
    m_pBreakWound = 0;
    m_JointOffset.Reset();
    m_JointPos.Reset();
    m_RotTarget.Reset();
    m_AtomSubgroupID = -1;
    m_DrawAfterParent = true;
    m_DamageCount = 0;
    m_OnlyLinForces = false;
	m_InheritsRotAngle = true;
	m_CanCollideWithTerrainWhenAttached = false;
	m_IsCollidingWithTerrainWhileAttached = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MOSParticle object ready for use.

int Attachable::Create()
{
    if (MOSRotating::Create() < 0)
        return -1;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Attachable to be identical to another, by deep copy.

int Attachable::Create(const Attachable &reference)
{
    MOSRotating::Create(reference);

    m_pParent = reference.m_pParent;
    m_ParentOffset = reference.m_ParentOffset;
    m_JointStrength = reference.m_JointStrength;
    m_JointStiffness = reference.m_JointStiffness;
    m_pBreakWound = reference.m_pBreakWound;
    m_JointOffset = reference.m_JointOffset;
    m_JointPos = reference.m_JointPos;
    m_RotTarget = reference.m_RotTarget;
    m_AtomSubgroupID = reference.m_AtomSubgroupID;
    m_DrawAfterParent = reference.m_DrawAfterParent;
    m_DamageCount = reference.m_DamageCount;
    m_OnlyLinForces = reference.m_OnlyLinForces;
	m_InheritsRotAngle = reference.m_InheritsRotAngle;
	m_CanCollideWithTerrainWhenAttached = reference.m_CanCollideWithTerrainWhenAttached;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Attachable::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "ParentOffset")
        reader >> m_ParentOffset;
    else if (propName == "JointStrength" || propName == "Strength")
        reader >> m_JointStrength;
    else if (propName == "JointStiffness" || propName == "Stiffness")
        reader >> m_JointStiffness;
    else if (propName == "BreakWound")
        m_pBreakWound = dynamic_cast<const AEmitter *>(g_PresetMan.GetEntityPreset(reader));
    else if (propName == "JointOffset")
        reader >> m_JointOffset;
    else if (propName == "InheritsRotAngle")
        reader >> m_InheritsRotAngle;
    else if (propName == "DrawAfterParent")
        reader >> m_DrawAfterParent;
    else if (propName == "CollidesWithTerrainWhenAttached")
        reader >> m_CanCollideWithTerrainWhenAttached;
    else
        // See if the base class(es) can find a match instead
        return MOSRotating::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Attachable with a Writer for
//                  later recreation with Create(Reader &reader);

int Attachable::Save(Writer &writer) const
{
    MOSRotating::Save(writer);

    writer.NewProperty("ParentOffset");
    writer << m_ParentOffset;
    writer.NewProperty("JointStrength");
    writer << m_JointStrength;
    writer.NewProperty("JointStiffness");
    writer << m_JointStiffness;
    writer.NewProperty("BreakWound");
    writer << m_pBreakWound;
    writer.NewProperty("JointOffset");
    writer << m_JointOffset;
	writer.NewProperty("InheritsRotAngle");
	writer << m_InheritsRotAngle;
	writer.NewProperty("DrawAfterParent");
    writer << m_DrawAfterParent;
    writer.NewProperty("CollidesWithTerrainWhenAttached");
    writer << m_CanCollideWithTerrainWhenAttached;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Attachable object.

void Attachable::Destroy(bool notInherited)
{

    if (!notInherited)
        MOSRotating::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CollideAtPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the collision response when another MO's Atom collides with
//                  this MO's physical representation. The effects will be applied
//                  directly to this MO, and also represented in the passed in HitData.

bool Attachable::CollideAtPoint(HitData &hd)
{
    return MOSRotating::CollideAtPoint(hd);
/*
    // See if the impact created a force enough to detach from parent.
    if (m_pParent && hd.resImpulse[HITEE].GetMagnitude() > m_JointStrength) {
        m_pParent->AddAbsImpulseForce(Vector(hd.resImpulse[HITEE]).SetMagnitude(m_JointStrength), m_JointPos);
        
        Detach();
    }
    else {
        
    }
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ParticlePenetration
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Determines whether a particle which has hit this MO will penetrate,
//                  and if so, whether it gets lodged or exits on the other side of this
//                  MO. Appropriate effects will be determined and applied ONLY IF there
//                  was penetration! If not, nothing will be affected.

bool Attachable::ParticlePenetration(HitData &hd)
{
    bool penetrated = MOSRotating::ParticlePenetration(hd);

	// Add damage points if MO is set to damage actors
	if (hd.pBody[HITOR]->DamageOnCollision() != 0)
		AddDamage(hd.pBody[HITOR]->DamageOnCollision());

    // If penetrated, propogate an alarm up to the root parent, if it's an actor
    if (penetrated && m_pParent)
    {
		// Add damage points if MO is set to damage actors on penetration
		if (hd.pBody[HITOR]->DamageOnPenetration() != 0)
			AddDamage(hd.pBody[HITOR]->DamageOnPenetration());

        Actor *pParentActor = dynamic_cast<Actor *>(GetRootParent());
        if (pParentActor)
        {
            // Move the alarm point out a bit from the body so the reaction is better
//            Vector extruded(g_SceneMan.ShortestDistance(pParentActor->GetPos(), hd.hitPoint));
            Vector extruded(hd.hitVel[HITOR]);
            extruded.SetMagnitude(pParentActor->GetHeight());
            extruded = m_Pos - extruded;
            g_SceneMan.WrapPosition(extruded);
            pParentActor->AlarmPoint(extruded);
        }
    }

    return penetrated;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GibThis
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gibs this, effectively destroying it and creating multiple gibs or
//                  pieces in its place.

void Attachable::GibThis(Vector impactImpulse, float internalBlast, MovableObject *pIgnoreMO)
{
    if (m_pParent)
    {
        (MOSRotating *)m_pParent->RemoveAttachable(this);
    }
    else
    {
        Detach();
    }

    MOSRotating::GibThis(impactImpulse, internalBlast, pIgnoreMO);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Attach
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Attaches this Attachable to a host MOSprite

void Attachable::Attach(MOSRotating *pParent)
{
    m_pParent = pParent;

    // Adopt the team of parent
    if (pParent)
    {
        m_Team = pParent->GetTeam();
    }

    // Reset the attachables timers so things that have been sitting in inventory don't make backed up emissions
    ResetAllTimers();

    if (m_pParent != NULL && m_pParent->GetID() != 255) {
        RunScriptedFunctionInAppropriateScripts("OnAttach", false, false, {m_pParent});
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Detach
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Detaches this Attachable from its host MOSprite

void Attachable::Detach()
{
    if (m_pParent)
    {
        // Attempt to remove any atoms of this that may have been added to the parent's AtomGroup before detaching
        m_pParent->GetAtomGroup()->RemoveAtoms(m_AtomSubgroupID);
    }

    m_Team = -1;
    MOSRotating *temporaryParent = m_pParent;
    m_pParent = 0;
	// Since it's no longer atteched it should belong to itself
	m_RootMOID = m_MOID;

#if defined DEBUG_BUILD || defined MIN_DEBUG_BUILD
	RTEAssert(m_RootMOID == g_NoMOID || (m_RootMOID >= 0 && m_RootMOID < g_MovableMan.GetMOIDCount()), "MOID out of bounds!");
	RTEAssert(m_MOID == g_NoMOID || (m_MOID >= 0 && m_MOID < g_MovableMan.GetMOIDCount()), "MOID out of bounds!");
#endif

    m_RestTimer.Reset();

    if (temporaryParent != NULL && temporaryParent->GetID() != 255) {
        RunScriptedFunctionInAppropriateScripts("OnDetach", false, false, {temporaryParent});
    }
}


/// <summary>
/// Turns on/off this Attachable's collisions, by adding/removing its atoms to/from its parent's AtomGroup.
/// </summary>
/// <param name="enable">Adds this Attachable's atoms to the parent's AtomGroup if true, removes them if false.</param>
void Attachable::EnableTerrainCollisions(bool enable)
{
	if (IsAttached() && CanCollideWithTerrainWhenAttached())
	{
		if (!IsCollidingWithTerrainWhileAttached() && enable)
		{
			m_pParent->GetAtomGroup()->AddAtoms(GetAtomGroup()->GetAtomList(), GetAtomSubgroupID(), GetParentOffset() - GetJointOffset());
			SetIsCollidingWithTerrainWhileAttached(true);
		}
		else if (IsCollidingWithTerrainWhileAttached() && !enable)
		{
			m_pParent->GetAtomGroup()->RemoveAtoms(GetAtomSubgroupID());
			SetIsCollidingWithTerrainWhileAttached(false);
		}
	}
	else if (IsAttached() && !CanCollideWithTerrainWhenAttached())
	{
		if (enable || !enable)
		{
			g_ConsoleMan.PrintString("ERROR: Tried to toggle collisions for attachable that was not set to collide when initially created!");
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TransferJointForces
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Bundles up all the accumulated forces of this Attachable and calcs
//                  how they transfer to the joint, and therefore to the parent.

bool Attachable::TransferJointForces(Vector &jointForces)
{
    // Exit trivially if not attahced to anything
    if (!m_pParent)
        return false;

    Vector forces;
    // Add up all the forces
    for (deque<pair<Vector, Vector> >::iterator fItr = m_Forces.begin();
         fItr != m_Forces.end(); ++fItr)
        forces += fItr->first;

    // Joint held up, so act accordingly
// TODO: Maybe not do this here, we might need the forces for other stuff?")
    // Clear out forces after we've bundled them up.
    m_Forces.clear();

    jointForces += forces;
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TransferJointImpulses
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Bundles up all the accumulated impulses of this Attachable and calcs
//                  how they transfer to the joint, and therefore to the parent.

bool Attachable::TransferJointImpulses(Vector &jointImpulses)
{
    // Exit trivially if not attahced to anything
    if (!m_pParent)
        return false;

    Vector impulses;
    // Add up the impulses
    for (deque<pair<Vector, Vector> >::iterator iItr = m_ImpulseForces.begin(); iItr != m_ImpulseForces.end(); ++iItr)
        impulses += iItr->first;

    // Factor in the stiffness, or lack thereof
    impulses *= m_JointStiffness;

    // Check if joint breaks and act accordingly
    if (impulses.GetMagnitude() > m_JointStrength)
    {
        impulses.SetMagnitude(m_JointStrength);
        jointImpulses += impulses;
/*
        float deltaTime = g_TimerMan.GetDeltaTimeSecs();
        m_ImpulseForces.push_back(make_pair<Vector, Vector>(-impulses, m_JointOffset));
        for (iItr = m_ImpulseForces.begin(); iItr != m_ImpulseForces.end(); ++iItr)
        {
            // Impulse force application to the transformational velocity of this MO.
            // Don't timescale these because they're already in kg * m/s (as opposed to kg * m/s^2).
            m_Vel += iItr->first / m_Mass;
            // Impulse force application to the rotational velocity of this MO.
            if (!(*iItr).second.IsZero())
                m_AngularVel += iItr->second.GetPerpendicular().Dot(iItr->first) /
                                m_pAtomGroup->GetMomentOfInertia();
        }
*/
        if (m_pBreakWound)
        {
            // Add velocity and wound before detaching.
            // The forces should be applied to this' vel next round when detached
            AEmitter *pWound = dynamic_cast<AEmitter *>(m_pBreakWound->Clone());
            if (pWound)
            {
                pWound->SetEmitAngle(m_JointOffset.GetAbsRadAngle());
				AddWound(pWound, m_JointOffset, false);
                pWound = 0;
            }
        }

        if (m_pParent)
        {
            m_pParent->RemoveAttachable(this);
        }
        else
        {
            Detach();
        }
        g_MovableMan.AddParticle(this);
        return false;
    }

    ////////////////////////////////////////////
    // Joint held up, so act accordingly

    // Apply the rotational effects of all the impulses accumulated.
    for (deque<pair<Vector, Vector> >::iterator iItr = m_ImpulseForces.begin(); iItr != m_ImpulseForces.end(); ++iItr)
    {
        // Impulse force application to the rotational velocity of this MO.
        if (!(*iItr).second.IsZero())
            m_AngularVel += ((*iItr).second.GetPerpendicular().Dot((*iItr).first) /
                             m_pAtomGroup->GetMomentOfInertia()) * (1.0 - m_JointStiffness);
    }
// TODO: Maybe not do this here, we might need the forces for other stuff?")
    // Clear out forces after we've bundled them up.
    m_ImpulseForces.clear();

    jointImpulses += impulses;
    return true;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PostTravel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does stuff that needs to be done after Update(). Always call after
//                  calling Update.

void Attachable::PostTravel()
{
    MOSRotating::PostTravel();
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CollectDamage
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of damage points this Attachable has sustained and
//                  should cause its parent. Calling this will reset the damage count.
//                  This should normally be called AFTER Update() to get the correct
//                  damage for a given frame.

float Attachable::CollectDamage()
{
    float totalDamage = m_DamageCount;
    m_DamageCount = 0;

    for (list<AEmitter *>::iterator itr = m_Wounds.begin(); itr != m_Wounds.end(); ++itr)
        totalDamage += (*itr)->CollectDamage();

    return totalDamage * m_DamageMultiplier;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RemoveWounds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a specified amount of wounds and returns damage caoused by this wounds. 
//					Head multiplier is not used.

int Attachable::RemoveWounds(int amount)
{
	return MOSRotating::RemoveWounds(amount) * m_DamageMultiplier;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this Attachable. Supposed to be done every frame.

void Attachable::Update()
{
    if (!m_pParent) {
// This doesn't work so well, sinking problems
//        m_DeepCheck = true;
        m_JointPos = m_Pos + RotateOffset(m_JointOffset);
    }
    else {
        // Save previous position and velocities before moving
        m_PrevPos = m_Pos;
        m_PrevVel = m_Vel;
        // Attached, so get all metrics from parent and apply
//        m_HFlipped = m_pParent->IsHFlipped();  not flexible enough
        if (!m_JointPos.IsZero())
            m_Pos = m_JointPos.GetFloored() - RotateOffset(m_JointOffset);
        else
            m_Pos = m_pParent->GetPos().GetFloored() - RotateOffset(m_JointOffset);
//        m_Rotation = m_pParent->GetRotMatrix();
        m_Vel = m_pParent->GetVel();
//        m_AngularVel =  m_pParent->GetAngularVel();
        m_Team = m_pParent->GetTeam();
/*
        ///////////////////////////////////////
        // Rotation spring calc
        // Get the rotation in radians.
        float springDelta = m_Rotation.GetRadAngle() - m_RotTarget.GetRadAngle();
        // Eliminate full rotations
        while (fabs(springDelta) > c_TwoPI) {
            springDelta -= springDelta > 0 ? c_TwoPI : -c_TwoPI;
        }
        // Eliminate rotations over half a turn
//        if (fabs(springDelta) > c_PI)
//            springDelta = (springDelta > 0 ? -c_PI : c_PI) + (springDelta - (springDelta > 0 ? c_PI : -c_PI));
        // Break the spring if close to target angle.
        if (fabs(springDelta) > 0.1)
            m_AngularVel -= springDelta * fabs(springDelta);// * m_JointStiffness;
        else if (fabs(m_AngularVel) > 0.1)
            m_AngularVel *= 0.5;
//        m_Rotation += springDelta * m_JointStiffness;

//        m_AngularVel -= springDelta * m_JointStiffness;

        // Apply the rotational velocity
        float deltaTime = g_TimerMan.GetDeltaTimeSecs();
//        m_Rotation += m_AngularVel * deltaTime;
*/

        m_DeepCheck = false;
    }

    MOSRotating::Update();

    // If we're attached to something, MoveableMan doesn't own us, and therefore isn't calling our ScriptUpdate (and our parent isn't calling it either), so we should here
    if (m_pParent != NULL && m_pParent->GetID() != 255) { UpdateScripts(); }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Attachable's current graphical representation to a
//                  BITMAP of choice.

void Attachable::Draw(BITMAP *pTargetBitmap,
                      const Vector &targetPos,
                      DrawMode mode,
                      bool onlyPhysical) const
{
    MOSRotating::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

/*
    Vector spritePos(m_Pos.GetFloored() - targetPos);

    // If attached to something, rotate about the joint,
    // otherwise rotate about the center of mass
//    if (m_pParent)
//        spritePos += m_JointOffset;

    if (m_Recoiled)
        spritePos += m_RecoilOffset;

    if (m_HFlipped) {
        if (!m_pTempBitmapB) {
            m_pTempBitmapB = new BITMAP();
            m_pTempBitmapB->Create(g_FrameMan.GetScreen(),
                                m_aSprite->GetTile()->GetBlockWidth(),
                                m_aSprite->GetTile()->GetBlockHeight(),
                                CDXMEM_SYSTEMONLY);
        }
        m_pTempBitmapB->Fill(g_MaskColor);
        m_pTempBitmapB->SetColorKey(g_MaskColor);
        m_aSprite->SetPos(0, 0);

        if (mode != g_DrawColor) {
            if (!m_pTempBitmapA) {
                m_pTempBitmapA = new BITMAP();
                m_pTempBitmapA->Create(g_FrameMan.GetScreen(),
                                   m_aSprite->GetTile()->GetBlockWidth(),
                                   m_aSprite->GetTile()->GetBlockHeight(),
                                   CDXMEM_SYSTEMONLY);
            }
            m_pTempBitmapA->Fill(g_MaskColor);
            m_pTempBitmapA->SetColorKey(g_MaskColor);

            if (mode == g_DrawMaterial)
                DrawMaterial(m_aSprite, m_pTempBitmapA, GetSettleMaterialID());
            else if (mode == g_DrawAir)
                DrawMaterial(m_aSprite, m_pTempBitmapA, g_MaterialAir);
            else if (mode == g_DrawKey)
                DrawMaterial(m_aSprite, m_pTempBitmapA, g_MaskColor);
            else if (mode == g_DrawMOID)
                DrawMaterial(m_aSprite, m_pTempBitmapA, m_MOID);
            else
                RTEAbort("Unknown draw mode selected in Attachable:Draw()!");

            m_pTempBitmapA->DrawTransHFlip(m_pTempBitmapB, 0, 0);
        }
        else
            m_aSprite->Draw(m_pTempBitmapB, 0, 0, CDXBLT_TRANSHFLIP);

        Vector newSpriteCent(m_SpriteCenter);
        newSpriteCent.m_X = -newSpriteCent.m_X;
        newSpriteCent.RadRotate(m_Rotation);
        spritePos += newSpriteCent;

        // Hack to correct shitty cdx rotozoom
        spritePos.m_X += 2.0;

        // This guy wants the center of the sprite, not the top left corner like Draw().
        m_pTempBitmapB->DrawTransRotoZoom(pTargetBitmap,
                                       spritePos.m_X,
                                       spritePos.m_Y,
                                       m_pTempBitmapB->GetClipRect(),
                                       m_Rotation,
                                       m_Scale);
    }
    else {
        spritePos += m_SpriteOffset;
        spritePos += (m_SpriteCenter * m_Rotation - m_SpriteCenter);
        // Hack to correct shitty cdx rotozoom
        spritePos.m_X += 1.0;

        m_aSprite->SetPos(spritePos.m_X, spritePos.m_Y);

        if (mode == g_DrawMaterial)
            DrawMaterialRotoZoomed(m_aSprite, pTargetBitmap, GetSettleMaterialID());
        else if (mode == g_DrawAir)
            DrawMaterialRotoZoomed(m_aSprite, pTargetBitmap, g_MaterialAir);
        else if (mode == g_DrawKey)
            DrawMaterialRotoZoomed(m_aSprite, pTargetBitmap, g_MaskColor);
        else if (mode == g_DrawMOID)
            DrawMaterialRotoZoomed(m_aSprite, pTargetBitmap, m_MOID);
        else
            m_aSprite->Draw(pTargetBitmap, 0, 0, CDXBLT_TRANSROTOZOOM);
    }

// TODO: Clean up the drawing hierarchy!#@!")
    // Finally draw all the attached emitters, and only if the mode is g_DrawColor
    if (mode == g_DrawColor || mode == g_DrawMaterial) {
        for (list<AEmitter *>::iterator itr = m_Wounds.begin(); itr != m_Wounds.end(); ++itr)
            (*itr)->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
    }
*/
/*
#ifdef DEBUG_BUILD
        pTargetBitmap->Lock();
        pTargetBitmap->PutPixel(m_Pos.GetFloorIntX() - targetPos.m_X,
                              m_Pos.GetFloorIntY() - targetPos.m_Y,
                              64);
        pTargetBitmap->UnLock();
#endif
*/
}

} // namespace RTE