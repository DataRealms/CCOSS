//////////////////////////////////////////////////////////////////////////////////////////
// File:            MOPixel.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the MOPixel class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "MOPixel.h"
#include "Atom.h"
#include "RTEManagers.h"

namespace RTE {

CONCRETECLASSINFO(MOPixel, MovableObject, 500)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MOPixel, effectively
//                  resetting the members of this abstraction level only.

void MOPixel::Clear()
{
    m_Color.Reset();
    m_pAtom = 0;
    m_DistanceTraveled = 0;
    m_MinLethalRange = 1;
    m_MaxLethalRange = 1;
    m_LethalSharpness = 1;
    m_LethalRange = max(g_FrameMan.GetPlayerScreenWidth(), g_FrameMan.GetPlayerScreenHeight()) / g_FrameMan.GetMPP();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MOPixel object ready for use.

int MOPixel::Create()
{
    if (MovableObject::Create() < 0)
        return -1;

    if (!m_pAtom)
        m_pAtom = new Atom;

    if (m_MinLethalRange < m_MaxLethalRange)
        m_LethalRange *= RangeRand(m_MinLethalRange, m_MaxLethalRange);

    m_LethalSharpness = m_Sharpness * 0.5;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MOPixel object ready for use.

int MOPixel::Create(Color color,
                    const float mass,
                    const Vector &position,
                    const Vector &velocity,
                    Atom *atom,
                    const unsigned long lifetime)
{
    m_Color = color;
    m_pAtom = atom;
    m_pAtom->SetOwner(this);

    if (m_MinLethalRange < m_MaxLethalRange)
        m_LethalRange *= RangeRand(m_MinLethalRange, m_MaxLethalRange);

    m_LethalSharpness = m_Sharpness * 0.5;

    return MovableObject::Create(mass, position, velocity, lifetime);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a MOPixel to be identical to another, by deep copy.

int MOPixel::Create(const MOPixel &reference)
{
    MovableObject::Create(reference);

    m_Color = reference.m_Color;
    m_pAtom = new Atom(*(reference.m_pAtom));
    m_pAtom->SetOwner(this);
    m_LethalRange = reference.m_LethalRange;
    m_LethalSharpness = reference.m_LethalSharpness;
    m_MinLethalRange = reference.m_MinLethalRange;
    m_MaxLethalRange = reference.m_MaxLethalRange;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int MOPixel::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "Color")
        reader >> m_Color;
    else if (propName == "Atom")
    {
        if (!m_pAtom)
            m_pAtom = new Atom;
        reader >> m_pAtom;
        m_pAtom->SetOwner(this);
    }
    else if (propName == "MinLethalRange")
        reader >> m_MinLethalRange;
    else if (propName == "MaxLethalRange")
        reader >> m_MaxLethalRange;
	else
        // See if the base class(es) can find a match instead
        return MovableObject::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this MOPixel with a Writer for
//                  later recreation with Create(Reader &reader);

int MOPixel::Save(Writer &writer) const
{
    MovableObject::Save(writer);

    writer.NewProperty("Color");
    writer << m_Color;
    writer.NewProperty("Atom");
    writer << m_pAtom;
    writer.NewProperty("MinLethalRange");
    writer << m_MinLethalRange;
    writer.NewProperty("MaxLethalRange");
    writer << m_MaxLethalRange;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the MOPixel object.

void MOPixel::Destroy(bool notInherited)
{
//    g_MovableMan.RemoveEntityPreset(this);

    delete m_pAtom;

    if (!notInherited)
        MovableObject::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the main Material of this MOPixel.

Material const * MOPixel::GetMaterial() const
{
    return m_pAtom->GetMaterial();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetDrawPriority
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the drawing priority of this MovableObject, if two things were
//                  overlap when copying to the terrain, the higher priority MO would
//                  end up getting drawn.

int MOPixel::GetDrawPriority() const
{
    return m_pAtom->GetMaterial()->priority;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetAtom
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Replaces the current Atom of this MOPixel with a new one.

void MOPixel::SetAtom(Atom *newAtom)
{
    delete m_pAtom;
    m_pAtom = newAtom;
    m_pAtom->SetOwner(this);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLethalRange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Travel distance until the bullet start to lose lethality.

void MOPixel::SetLethalRange(float range)
{
    m_LethalRange = range;
    if (m_MinLethalRange < m_MaxLethalRange)
        m_LethalRange *= RangeRand(m_MinLethalRange, m_MaxLethalRange);
    m_LethalRange /= g_FrameMan.GetPPM();   // convert to meters
};

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  HitsMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets whether this MovableObject is set to collide with other
//                  MovableObject:s during travel.

bool MOPixel::HitsMOs() const
{
    return m_pAtom->HitsMOs();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetToHitMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this MovableObject to collide with other MovableObjects during
//                  travel.

void MOPixel::SetToHitMOs(bool hitMOs)
{
    m_pAtom->SetToHitMOs(hitMOs);
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CollideAtPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the collision response when another MO's Atom collides with
//                  this MO's physical representation. The effects will be applied
//                  directly to this MO, and also represented in the passed in HitData.

bool MOPixel::CollideAtPoint(HitData &hd)
{
    RTEAssert(hd.hitPoint.GetFloored() == m_Pos.GetFloored(), "Collision mismatch in MOPixel::CollideAtPoint!");
    RTEAssert(hd.pBody[HITOR], "Valid MO not passed into MOPixel::CollideAtPoint!");

    hd.mass[HITEE] = m_Mass;

    // See if we were already hit by this MO earlier during this frame update.
    if (m_AlreadyHitBy.find(hd.pBody[HITOR]->GetID()) != m_AlreadyHitBy.end())
        // If we were hit, then remove so that if we 
        m_AlreadyHitBy.erase(hd.pBody[HITOR]->GetID());
    // We weren't previously hit by this MO, so go ahead and apply collision response
    else {
        // Note that we now have been hit by this MO
        m_AlreadyHitBy.insert(hd.pBody[HITOR]->GetID());
        
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnBounce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  bounces off of something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.

bool MOPixel::OnBounce(HitData &hd)
{
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnSink
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  sink into something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.

bool MOPixel::OnSink(HitData &hd)
{
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RestDetection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does the calculations necessary to detect whether this MO appears to
//                  have has settled in the world and is at rest or not. IsAtRest()
//                  retreves the answer.

void MOPixel::RestDetection()
{
    MovableObject::RestDetection();

    // If we seem to be about to settle, make sure we're not flying in the air still
    if (m_ToSettle || IsAtRest())
    {
        if (g_SceneMan.OverAltitude(m_Pos, 2, 0))
        {
            m_RestTimer.Reset();
            m_ToSettle = false;
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Travel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Travels this MovableObject, using its physical representation.

void MOPixel::Travel()
{
    MovableObject::Travel();

    if (m_PinStrength)
        return;

    // Set the atom to ignore a certain MO, if set and applicable.
    if (m_HitsMOs && m_pMOToNotHit && g_MovableMan.ValidMO(m_pMOToNotHit) && !m_MOIgnoreTimer.IsPastSimTimeLimit())
    {
        MOID root = m_pMOToNotHit->GetID();
        int footprint = m_pMOToNotHit->GetMOIDFootprint();
        for (int i = 0; i < footprint; ++i)
            m_pAtom->AddMOIDToIgnore(root + i);
    }

    // Do static particle bounce calculations.
    int hitCount = m_pAtom->Travel(g_TimerMan.GetDeltaTimeSecs(), true, g_SceneMan.SceneIsLocked());

    // Now clear out the ignore override for next frame
    m_pAtom->ClearMOIDIgnoreList();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MOPixel. Supposed to be done every frame.

void MOPixel::Update()
{
    MovableObject::Update();

    // Check for hard age expiration to avoid bouncy ground particles that never settle
    if (m_AgeTimer.GetElapsedSimTimeMS() > 10000)
    {
        // If traveling at very slow speed, probably on the ground, so safe to settle
//        if (m_Vel.GetMagnitude() < 1.0)
//            m_ToSettle = true;
//        // Traveling fast through space... should be ok to delete
//        else
            m_ToDelete = true;
    }
    else if (m_HitsMOs && m_Sharpness > 0)
    {
        m_DistanceTraveled += m_Vel.GetLargest() * g_TimerMan.GetDeltaTimeSecs();
        if (m_DistanceTraveled > m_LethalRange)
        {
			
            if (m_Sharpness < m_LethalSharpness)
                m_Sharpness = max(m_Sharpness * (1.0 - (20.0 * g_TimerMan.GetDeltaTimeSecs())) - 0.1, 0.0);
            else
                m_Sharpness *= 1.0 - (10.0 * g_TimerMan.GetDeltaTimeSecs());
			
			if (m_LethalRange > 0)
				m_HitsMOs = false;
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this MOPixel's current graphical representation to a
//                  BITMAP of choice.

void MOPixel::Draw(BITMAP *pTargetBitmap,
                   const Vector &targetPos,
                   DrawMode mode,
                   bool onlyPhysical) const
{
    // Don't draw color if this isn't a drawing frame
    if (!g_TimerMan.DrawnSimUpdate() && mode == g_DrawColor)
        return;

/*
    pTargetBitmap->Line(m_PrevPos.GetFloorIntX(),
                       m_PrevPos.GetFloorIntY(),
                       m_Pos.GetFloorIntX(),
                       m_Pos.GetFloorIntY(),
                       m_TrailColor);
*/

    acquire_bitmap(pTargetBitmap);

/*
    if (m_pAtom->GetMaterial().id == c_GoldMaterialID) {
        if (mode == g_DrawKey) {
            
        }
        else if (mode == g_DrawMOID)
    }
*/
    putpixel(pTargetBitmap, m_Pos.GetFloorIntX() - targetPos.m_X,
                            m_Pos.GetFloorIntY() - targetPos.m_Y,
                            mode == g_DrawMaterial ? m_pAtom->GetMaterial()->GetSettleMaterialID() :
                                                   (mode == g_DrawAir ? g_MaterialAir :
                                                   (mode == g_DrawKey ? g_MaskColor :
                                                   (mode == g_DrawMOID ? m_MOID :
                                                   (mode == g_DrawNoMOID ? g_NoMOID : m_Color.GetIndex())))));

    if (mode == g_DrawMOID)
        g_SceneMan.RegisterMOIDDrawing(m_Pos - targetPos, 1);

    // Set the screen effect to draw at the final post processing stage
    if (m_pScreenEffect && mode == g_DrawColor && !onlyPhysical && m_AgeTimer.IsPastSimMS(m_EffectStartTime) && (m_EffectStopTime == 0 || !m_AgeTimer.IsPastSimMS(m_EffectStopTime)) && (m_EffectAlwaysShows || !g_SceneMan.ObscuredPoint(m_Pos.GetFloorIntX(), m_Pos.GetFloorIntY())))
		g_PostProcessMan.RegisterPostEffect(m_Pos, m_pScreenEffect, m_ScreenEffectHash, LERP(m_EffectStartTime, m_EffectStopTime, m_EffectStartStrength, m_EffectStopStrength, m_AgeTimer.GetElapsedSimTimeMS()), m_EffectRotAngle);

    release_bitmap(pTargetBitmap);
}

} // namespace RTE