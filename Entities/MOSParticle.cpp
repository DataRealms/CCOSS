//////////////////////////////////////////////////////////////////////////////////////////
// File:            MOSParticle.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the MOSParticle class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "MOSParticle.h"
#include "Atom.h"
#include "RTEManagers.h"
#include "DDTTools.h"

using namespace std;

namespace RTE
{

CONCRETECLASSINFO(MOSParticle, MovableObject, 200)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MOSParticle, effectively
//                  resetting the members of this abstraction level only.

void MOSParticle::Clear()
{
    m_pAtom = 0;
    m_Framerate = 0;
    m_TimeRest = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MOSParticle object ready for use.

int MOSParticle::Create()
{
    if (MOSprite::Create() < 0)
        return -1;

    if (!m_pAtom)
        m_pAtom = new Atom();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MOSParticle object ready for use.

int MOSParticle::Create(ContentFile spriteFile,
                     const int frameCount,
                     const float mass,
                     const Vector &position,
                     const Vector &velocity,
                     const unsigned long lifetime)
{
    MOSprite::Create(spriteFile, frameCount, mass, position, velocity, lifetime);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a MOSParticle to be identical to another, by deep copy.

int MOSParticle::Create(const MOSParticle &reference)
{
    MOSprite::Create(reference);

    m_pAtom = new Atom(*(reference.m_pAtom));
    m_pAtom->SetOwner(this);
    m_Framerate = reference.m_Framerate;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int MOSParticle::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "Atom")
    {
        if (!m_pAtom)
            m_pAtom = new Atom;
        reader >> *m_pAtom;
        m_pAtom->SetOwner(this);
    }
    else if (propName == "Framerate")
        reader >> m_Framerate;
    else
        // See if the base class(es) can find a match instead
        return MOSprite::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this MOSParticle with a Writer for
//                  later recreation with Create(Reader &reader);

int MOSParticle::Save(Writer &writer) const
{
    MOSprite::Save(writer);
// TODO: Make proper save system that knows not to save redundant data!
/*
    writer.NewProperty("Atom");
    writer << m_pAtom;
    writer.NewProperty("Framerate");
    writer << m_Framerate;
*/
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the MOSParticle object.

void MOSParticle::Destroy(bool notInherited)
{
    delete m_pAtom;

    if (!notInherited)
        MOSprite::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the main material index of this MOSParticle.

Material const * MOSParticle::GetMaterial() const
{
    return m_pAtom->GetMaterial();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetDrawPriority
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the drawing priority of this MovableObject, if two things were
//                  overlap when copying to the terrain, the higher priority MO would
//                  end up getting drawn.

int MOSParticle::GetDrawPriority() const
{
    return m_pAtom->GetMaterial()->priority;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetAtom
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Replaces the current Atom of this MOSParticle with a new one.
// Arguments:       A reference to the new Atom.
// Return value:    None.

void MOSParticle::SetAtom(Atom *newAtom)
{
    delete m_pAtom;
    m_pAtom = newAtom;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  HitsMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets whether this MovableObject is set to collide with other
//                  MovableObject:s during travel.

bool MOSParticle::HitsMOs() const
{
    return m_pAtom->HitsMOs();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetToHitMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this MovableObject to collide with other MovableObjects during
//                  travel.

void MOSParticle::SetToHitMOs(bool hitMOs)
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

bool MOSParticle::CollideAtPoint(HitData &hitData)
{
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnBounce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  bounces off of something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.

bool MOSParticle::OnBounce(HitData &hd)
{
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnSink
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  sink into something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.

bool MOSParticle::OnSink(HitData &hd)
{
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RestDetection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does the calculations necessary to detect whether this MO appears to
//                  have has settled in the world and is at rest or not. IsAtRest()
//                  retreves the answer.

void MOSParticle::RestDetection()
{
    MOSprite::RestDetection();

    // If we seem to be about to settle, make sure we're not flying in the air still
    if (m_ToSettle || IsAtRest())
    {
        if (g_SceneMan.OverAltitude(m_Pos, (m_aSprite[m_Frame]->h / 2) + 3, 2))
        {
            m_RestTimer.Reset();
            m_ToSettle = false;
        }
// TODO: REMOVE
//        bool KUIKEN = g_SceneMan.OverAltitude(m_Pos, (m_aSprite[m_Frame]->h / 2) + 3, 2);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Travel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Travels this MovableObject, using its physical representation.

void MOSParticle::Travel()
{
    SLICK_PROFILE(0xFF323324);

    MOSprite::Travel();

    // Pinned objects don't travel!
    if (m_PinStrength)
        return;

    float deltaTime = g_TimerMan.GetDeltaTimeSecs();
    float velMag = m_Vel.GetMagnitude();

    /////////////////////////////////
    // Atom update and handling

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

    /////////////////////////////////
    // Animation logic
// TODO: SO GHETTO, IMPROVE!")
/*
    // Change animation characteristics after collision.
    if (bounceCount >= 1) {
// TODO: Tweak this!")
        m_Framerate *= 0.5 * velMag * NormalRand();
        m_Framerate = -m_Framerate;
    }

    int framesPlayed = 0;

    if (m_Framerate != 0) {
        // s per frame.
        float spf = 1 / m_Framerate;
        framesPlayed = (m_TimeRest + deltaTime) / spf;
        m_TimeRest += deltaTime;
        m_TimeRest -= abs(framesPlayed) * fabs(spf);
    }
    int newFrame = m_aSprite->GetFrame() + framesPlayed;
    if (newFrame < 0)
        newFrame += m_FrameCount;
    m_aSprite->SetFrame(newFrame % m_FrameCount);
*/
    // Change angular velocity after collision.
    if (hitCount >= 1) {
// TODO: Tweak this!")
        m_AngularVel *= 0.5 * velMag * NormalRand();
        m_AngularVel = -m_AngularVel;
    }

    // Animate based on rotation.. temporary.
    if (m_Framerate) {
        double newFrame = m_Rotation.GetRadAngle();
        newFrame -= floorf(m_Rotation.GetRadAngle() / (2 * PI)) * (2 * PI);
        newFrame /= (2 * PI);
        newFrame *= m_FrameCount;
        m_Frame = floorf(newFrame);
        m_Rotation += (long double)m_AngularVel * (long double)deltaTime;
    }
    // Animate over lifetime
    else {
        double newFrame = (double)m_FrameCount *
                          ((double)m_AgeTimer.GetElapsedSimTimeMS() / (double)m_Lifetime);
        m_Frame = floorf(newFrame);
    }

    // Bounds
    if (m_Frame < 0)
        m_Frame = 0;
    if (m_Frame >= m_FrameCount)
        m_Frame = m_FrameCount - 1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MOSParticle. Supposed to be done every frame.

void MOSParticle::Update()
{
    MOSprite::Update();

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
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this MOSParticle's current graphical representation to a
//                  BITMAP of choice.

void MOSParticle::Draw(BITMAP *pTargetBitmap,
                       const Vector &targetPos,
                       DrawMode mode,
                       bool onlyPhysical) const
{
    Vector spritePos(m_Pos + m_SpriteOffset - targetPos);

    // Draw the requested material sihouette on the material bitmap
    if (mode == g_DrawMaterial)
        draw_character_ex(pTargetBitmap, m_aSprite[m_Frame], spritePos.GetFloorIntX(), spritePos.GetFloorIntY(), m_SettleMaterialDisabled ? GetMaterial()->id : GetMaterial()->GetSettleMaterialID(), -1);
    else if (mode == g_DrawAir)
        draw_character_ex(pTargetBitmap, m_aSprite[m_Frame], spritePos.GetFloorIntX(), spritePos.GetFloorIntY(), g_MaterialAir, -1);
    else if (mode == g_DrawKey)
        draw_character_ex(pTargetBitmap, m_aSprite[m_Frame], spritePos.GetFloorIntX(), spritePos.GetFloorIntY(), g_KeyColor, -1);
    else if (mode == g_DrawWhite)
        draw_character_ex(pTargetBitmap, m_aSprite[m_Frame], spritePos.GetFloorIntX(), spritePos.GetFloorIntY(), g_WhiteColor, -1);
    else if (mode == g_DrawMOID)
    {
        int spriteX = spritePos.GetFloorIntX();
        int spriteY = spritePos.GetFloorIntY();
        draw_character_ex(pTargetBitmap, m_aSprite[m_Frame], spriteX, spriteY, m_MOID, -1);
        g_SceneMan.RegisterMOIDDrawing(spriteX, spriteY, spriteX + m_aSprite[m_Frame]->w, spriteY + m_aSprite[m_Frame]->h);
    }
    else if (mode == g_DrawNoMOID)
        draw_character_ex(pTargetBitmap, m_aSprite[m_Frame], spritePos.GetFloorIntX(), spritePos.GetFloorIntY(), g_NoMOID, -1);
    else if (mode == g_DrawTrans)
        draw_trans_sprite(pTargetBitmap, m_aSprite[m_Frame], spritePos.GetFloorIntX(), spritePos.GetFloorIntY());
    else if (mode == g_DrawAlpha)
    {
        set_alpha_blender();
        draw_trans_sprite(pTargetBitmap, m_aSprite[m_Frame], spritePos.GetFloorIntX(), spritePos.GetFloorIntY());
    }
    else
        draw_sprite(pTargetBitmap, m_aSprite[m_Frame], spritePos.GetFloorIntX(), spritePos.GetFloorIntY());

    // Set the screen effect to draw at the final post processing stage
    if (m_pScreenEffect && mode == g_DrawColor && !onlyPhysical && m_AgeTimer.IsPastSimMS(m_EffectStartTime) && (m_EffectStopTime == 0 || !m_AgeTimer.IsPastSimMS(m_EffectStopTime)) &&  (m_EffectAlwaysShows || !g_SceneMan.ObscuredPoint(m_Pos.GetFloorIntX(), m_Pos.GetFloorIntY())))
    {
//        int strength = m_EffectStartStrength + (m_AgeTimer.GetElapsedSimTimeMS() - m_EffectStartTime) * ((m_EffectStartStrength) / ())   m_EffectStartStrength * (1.0f - (float)(m_AgeTimer.GetElapsedSimTimeMS() - m_EffectStartTime) / (float)DMin(m_EffectStopTime, m_Lifetime));
        g_SceneMan.RegisterPostEffect(m_Pos, m_pScreenEffect, m_ScreenEffectHash, LERP(m_EffectStartTime, m_EffectStopTime, m_EffectStartStrength, m_EffectStopStrength, m_AgeTimer.GetElapsedSimTimeMS()), m_EffectRotAngle);
    }

/*
    if (!onlyPhysical) {
        char str[64];
        sprintf(str, "aVel:%.2f", m_AngularVel);
        g_FrameMan.DrawText(pTargetBitmap, str, m_Pos + Vector(-45, -75), false);
    }
*/
}

} // namespace RTE