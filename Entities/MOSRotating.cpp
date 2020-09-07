//////////////////////////////////////////////////////////////////////////////////////////
// File:            MOSRotating.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the MOSRotating class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "MOSRotating.h"
#include "SettingsMan.h"
#include "AtomGroup.h"
#include "SLTerrain.h"
#include "MOPixel.h"
#include "MOSParticle.h"
#include "AEmitter.h"
#include "Attachable.h"

#include "RTEError.h"

namespace RTE {

ConcreteClassInfo(MOSRotating, MOSprite, 500)

BITMAP * MOSRotating::m_spTempBitmap16 = 0;
BITMAP * MOSRotating::m_spTempBitmap32 = 0;
BITMAP * MOSRotating::m_spTempBitmap64 = 0;
BITMAP * MOSRotating::m_spTempBitmap128 = 0;
BITMAP * MOSRotating::m_spTempBitmap256 = 0;
BITMAP * MOSRotating::m_spTempBitmap512 = 0;

BITMAP * MOSRotating::m_spTempBitmapS16 = 0;
BITMAP * MOSRotating::m_spTempBitmapS32 = 0;
BITMAP * MOSRotating::m_spTempBitmapS64 = 0;
BITMAP * MOSRotating::m_spTempBitmapS128 = 0;
BITMAP * MOSRotating::m_spTempBitmapS256 = 0;
BITMAP * MOSRotating::m_spTempBitmapS512 = 0;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MOSRotating, effectively
//                  resetting the members of this abstraction level only.

void MOSRotating::Clear()
{
    m_pAtomGroup = 0;
    m_pDeepGroup = 0;
    m_DeepCheck = false;
    m_ForceDeepCheck = false;
    m_DeepHardness = 0;
    m_TravelImpulse.Reset();
    m_SpriteCenter.Reset();
    m_OrientToVel = 0;
    m_Recoiled = false;
    m_RecoilForce.Reset();
    m_RecoilOffset.Reset();
    m_Wounds.clear();
    m_Attachables.clear();
    m_AllAttachables.clear();
    m_Gibs.clear();
    m_GibImpulseLimit = 0;
    m_GibWoundLimit = 0;
    m_GibSound.Reset();
    m_EffectOnGib = true;
    m_pFlipBitmap = 0;
	m_pFlipBitmapS = 0;
	m_pTempBitmap = 0;
	m_pTempBitmapS = 0;
    m_LoudnessOnGib = 1;
	m_DamageMultiplier = 1;
	m_DamageMultiplierRedefined = false;
    m_StringValueMap.clear();
    m_NumberValueMap.clear();
    m_ObjectValueMap.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MOSParticle object ready for use.

int MOSRotating::Create()
{
    if (MOSprite::Create() < 0)
        return -1;

    if (m_pAtomGroup && m_pAtomGroup->AutoGenerate()/* && m_pAtomGroup->GetAtomCount() == 0*/)
        m_pAtomGroup->Create(this);
    else if (m_pAtomGroup)
        m_pAtomGroup->SetOwner(this);

    if (m_pDeepGroup && m_pDeepGroup->AutoGenerate()/* && m_pDeepGroup->GetAtomCount() == 0*/)
        m_pDeepGroup->Create(this);
    else if (m_pDeepGroup)
        m_pDeepGroup->SetOwner(this);

    // Set up the sprite center Vector.
    m_SpriteCenter.SetXY(m_aSprite[m_Frame]->w / 2,
                         m_aSprite[m_Frame]->h / 2);
    m_SpriteCenter += m_SpriteOffset;

// Now done in MOSprite::Create, based on the sprite
    // Calc radius based on the atomgroup
//    m_MaxRadius = m_pAtomGroup->CalculateMaxRadius() + 4;
//    m_MaxDiameter = m_MaxRadius * 2;

/* Allocated in lazy fashion as needed when drawing flipped
    if (!m_pFlipBitmap && m_aSprite[0])
        m_pFlipBitmap = create_bitmap_ex(8, m_aSprite[0]->w, m_aSprite[0]->h);
*/
/* Not anymore; points to shared static bitmaps
    if (!m_pTempBitmap && m_aSprite[0])
        m_pTempBitmap = create_bitmap_ex(8, m_aSprite[0]->w, m_aSprite[0]->h);
*/

    // Can't create these earlier in the static declaration because allegro_init needs to be called before create_bitmap
    if (!m_spTempBitmap16)
        m_spTempBitmap16 = create_bitmap_ex(8, 16, 16);
    if (!m_spTempBitmap32)
        m_spTempBitmap32 = create_bitmap_ex(8, 32, 32);
    if (!m_spTempBitmap64)
        m_spTempBitmap64 = create_bitmap_ex(8, 64, 64);
    if (!m_spTempBitmap128)
        m_spTempBitmap128 = create_bitmap_ex(8, 128, 128);
    if (!m_spTempBitmap256)
        m_spTempBitmap256 = create_bitmap_ex(8, 256, 256);
    if (!m_spTempBitmap512)
        m_spTempBitmap512 = create_bitmap_ex(8, 512, 512);

    // Can't create these earlier in the static declaration because allegro_init needs to be called before create_bitmap
    if (!m_spTempBitmapS16)
        m_spTempBitmapS16 = create_bitmap_ex(c_MOIDLayerBitDepth, 16, 16);
    if (!m_spTempBitmapS32)
        m_spTempBitmapS32 = create_bitmap_ex(c_MOIDLayerBitDepth, 32, 32);
    if (!m_spTempBitmapS64)
        m_spTempBitmapS64 = create_bitmap_ex(c_MOIDLayerBitDepth, 64, 64);
    if (!m_spTempBitmapS128)
        m_spTempBitmapS128 = create_bitmap_ex(c_MOIDLayerBitDepth, 128, 128);
    if (!m_spTempBitmapS256)
        m_spTempBitmapS256 = create_bitmap_ex(c_MOIDLayerBitDepth, 256, 256);
    if (!m_spTempBitmapS512)
        m_spTempBitmapS512 = create_bitmap_ex(c_MOIDLayerBitDepth, 512, 512);

    // Choose an appropriate size for this' diameter
    if (m_MaxDiameter >= 256)
	{
        m_pTempBitmap = m_spTempBitmap512;
        m_pTempBitmapS = m_spTempBitmapS512;
	}
    else if (m_MaxDiameter  >= 128)
	{
        m_pTempBitmap = m_spTempBitmap256;
        m_pTempBitmapS = m_spTempBitmapS256;
	}
    else if (m_MaxDiameter  >= 64)
	{
        m_pTempBitmap = m_spTempBitmap128;
        m_pTempBitmapS = m_spTempBitmapS128;
	}
    else if (m_MaxDiameter >= 32)
	{
        m_pTempBitmap = m_spTempBitmap64;
        m_pTempBitmapS = m_spTempBitmapS64;
	}
    else if (m_MaxDiameter >= 16)
	{
        m_pTempBitmap = m_spTempBitmap32;
        m_pTempBitmapS = m_spTempBitmapS32;
	}
    else
	{
		m_pTempBitmap = m_spTempBitmap16;
		m_pTempBitmapS = m_spTempBitmapS16;
	}

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MOSParticle object ready for use.

int MOSRotating::Create(ContentFile spriteFile,
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
// Description:     Creates a MOSRotating to be identical to another, by deep copy.

int MOSRotating::Create(const MOSRotating &reference)
{
    MOSprite::Create(reference);

    if (!reference.m_pAtomGroup)
        return -1;

    {
        // THESE ATOMGROUP COPYING ARE A TIME SINK!
        m_pAtomGroup = new AtomGroup();
        m_pAtomGroup->Create(*reference.m_pAtomGroup, true);
        if (m_pAtomGroup)
            m_pAtomGroup->SetOwner(this);

        if (reference.m_pDeepGroup)
        {
            m_pDeepGroup = dynamic_cast<AtomGroup *>(reference.m_pDeepGroup->Clone());
            if (m_pDeepGroup)
                m_pDeepGroup->SetOwner(this);
        }
    }

    m_DeepCheck = reference.m_DeepCheck;
    m_SpriteCenter = reference.m_SpriteCenter;
    m_OrientToVel = reference.m_OrientToVel;

    m_Recoiled = reference.m_Recoiled;
    m_RecoilForce = reference.m_RecoilForce;
    m_RecoilOffset = reference.m_RecoilOffset;

	// Wound emitter copies
    AEmitter *pWound = 0;
    for (list<AEmitter *>::const_iterator itr = reference.m_Wounds.begin(); itr != reference.m_Wounds.end(); ++itr)
    {
		pWound = dynamic_cast<AEmitter *>((*itr)->Clone());
		AddWound(pWound, pWound->GetParentOffset());
		pWound = 0;
    }

	// Attachable copies
    m_AllAttachables.clear();
    Attachable *pAttachable = 0;
    for (list<Attachable *>::const_iterator aItr = reference.m_Attachables.begin(); aItr != reference.m_Attachables.end(); ++aItr)
    {
        pAttachable = dynamic_cast<Attachable *>((*aItr)->Clone());
        AddAttachable(pAttachable, pAttachable->GetParentOffset());
        pAttachable = 0;
    }

	// Gib copies
    for (list<Gib>::const_iterator gItr = reference.m_Gibs.begin(); gItr != reference.m_Gibs.end(); ++gItr)
    {
        m_Gibs.push_back(*gItr);
    }

    m_StringValueMap = reference.m_StringValueMap;
    m_NumberValueMap = reference.m_NumberValueMap;
    m_ObjectValueMap = reference.m_ObjectValueMap;

    m_GibImpulseLimit = reference.m_GibImpulseLimit;
    m_GibWoundLimit = reference.m_GibWoundLimit;
    m_GibSound = reference.m_GibSound;
    m_EffectOnGib = reference.m_EffectOnGib;
    m_LoudnessOnGib = reference.m_LoudnessOnGib;

	m_DamageMultiplier = reference.m_DamageMultiplier;
	m_DamageMultiplierRedefined = reference.m_DamageMultiplierRedefined;

/* Allocated in lazy fashion as needed when drawing flipped
    if (!m_pFlipBitmap && m_aSprite[0])
        m_pFlipBitmap = create_bitmap_ex(8, m_aSprite[0]->w, m_aSprite[0]->h);
*/
/* Not anymore; points to shared static bitmaps
    if (!m_pTempBitmap && m_aSprite[0])
        m_pTempBitmap = create_bitmap_ex(8, m_aSprite[0]->w, m_aSprite[0]->h);
*/
    m_pTempBitmap = reference.m_pTempBitmap;
    m_pTempBitmapS = reference.m_pTempBitmapS;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int MOSRotating::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "AtomGroup")
    {
        delete m_pAtomGroup;
        m_pAtomGroup = new AtomGroup();
        reader >> *m_pAtomGroup;
    }
    else if (propName == "DeepGroup")
    {
        delete m_pDeepGroup;
        m_pDeepGroup = new AtomGroup();
        reader >> *m_pDeepGroup;
    }
    else if (propName == "DeepCheck")
        reader >> m_DeepCheck;
    else if (propName == "OrientToVel")
        reader >> m_OrientToVel;
    else if (propName == "AddEmitter")
    {
        AEmitter *pEmitter = new AEmitter;
        reader >> pEmitter;
		m_Attachables.push_back(pEmitter);
    }
    else if (propName == "AddAttachable")
    {
        Attachable *pAttachable = new Attachable;
        reader >> pAttachable;
        m_Attachables.push_back(pAttachable);
    }
    else if (propName == "AddGib")
    {
        Gib gib;
        reader >> gib;
        m_Gibs.push_back(gib);
    }
    else if (propName == "GibImpulseLimit")
        reader >> m_GibImpulseLimit;
    else if (propName == "GibWoundLimit" || propName == "WoundLimit")
        reader >> m_GibWoundLimit;
    else if (propName == "GibSound")
        reader >> m_GibSound;
    else if (propName == "EffectOnGib")
        reader >> m_EffectOnGib;
    else if (propName == "LoudnessOnGib")
        reader >> m_LoudnessOnGib;
	else if (propName == "DamageMultiplier") {
		reader >> m_DamageMultiplier;
		m_DamageMultiplierRedefined = true;
    } else if (propName == "AddCustomValue") {
        ReadCustomValueProperty(reader);
    } else
        return MOSprite::ReadProperty(propName, reader);

    return 0;
}

void MOSRotating::ReadCustomValueProperty(Reader &reader) {
    std::string customValueType;
    reader >> customValueType;
    std::string customKey = reader.ReadPropName();
    std::string customValue = reader.ReadPropValue();
    if (customValueType == "NumberValue") {
        try {
            SetNumberValue(customKey, std::stod(customValue));
        } catch (const std::invalid_argument) {
            reader.ReportError("Tried to read a non-number value for SetNumberValue.");
        }
    } else if (customValueType == "StringValue") {
        SetStringValue(customKey, customValue);
    } else {
        reader.ReportError("Invalid CustomValue type " + customValueType);
    }
    // Artificially end reading this property since we got all we needed
    reader.NextProperty();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this MOSRotating with a Writer for
//                  later recreation with Create(Reader &reader);

int MOSRotating::Save(Writer &writer) const
{
    MOSprite::Save(writer);

// TODO: Make proper save system that knows not to save redundant data!
/*
    writer.NewProperty("AtomGroup");
    writer << m_pAtomGroup;
    writer.NewProperty("DeepGroup");
    writer << m_pDeepGroup;
    writer.NewProperty("DeepCheck");
    writer << m_DeepCheck;
    writer.NewProperty("OrientToVel");
    writer << m_OrientToVel;

    for (list<AEmitter *>::const_iterator itr = m_Wounds.begin(); itr != m_Wounds.end(); ++itr)
    {
        writer.NewProperty("AddEmitter");
        writer << (*itr);
    }
    for (list<Attachable *>::const_iterator aItr = m_Attachables.begin(); aItr != m_Attachables.end(); ++aItr)
    {
        writer.NewProperty("AddAttachable");
        writer << (*aItr);
    }
*/
    for (list<Gib>::const_iterator gItr = m_Gibs.begin(); gItr != m_Gibs.end(); ++gItr)
    {
        writer.NewProperty("AddGib");
        writer << (*gItr);
    }
/*
    writer.NewProperty("GibImpulseLimit");
    writer << m_GibImpulseLimit;
    writer.NewProperty("GibWoundLimit");
    writer << m_GibWoundLimit;
    writer.NewProperty("GibSound");
    writer << m_GibSound;
    writer.NewProperty("EffectOnGib");
    writer << m_EffectOnGib;
*/
    return 0;
}


/// <summary>
/// Attaches the passed in wound AEmitter and adds it to the list of wounds, changing its parent offset to the passed in Vector.
/// </summary>
/// <param name="pWound">The wound AEmitter to add</param>
/// <param name="parentOffsetToSet">The vector to set as the wound AEmitter's parent offset</param>
void MOSRotating::AddWound(AEmitter *pWound, const Vector & parentOffsetToSet, bool checkGibWoundLimit)
{
	if (pWound)
	{
		if (checkGibWoundLimit && !ToDelete() && m_GibWoundLimit && m_Wounds.size() + 1 > m_GibWoundLimit)
		{
			// Indicate blast in opposite direction of emission
			// TODO: don't hardcode here, get some data from the emitter
			Vector blast(-5, 0);
			blast.RadRotate(pWound->GetEmitAngle());
			GibThis(blast);
			return;
		}
		else
		{
			pWound->Attach(this, parentOffsetToSet);
			m_Wounds.push_back(pWound);
		}
	}
}


/// <summary>
/// Removes a specified amount of wounds and returns damage caused by this wounds. Head multiplier is not used.				
/// </summary>
/// <param name="amount">Amount of wounds to remove.</param>
/// <returns>Amount of damage caused by these wounds.</returns>
int MOSRotating::RemoveWounds(int amount)
{
	int deleted = 0;
	float damage = 0;

    for (list<AEmitter *>::iterator itr = m_Wounds.begin(); itr != m_Wounds.end();)
	{
		damage += (*itr)->GetBurstDamage();
        delete (*itr);
		(*itr) = 0;
		itr = m_Wounds.erase(itr);
		deleted++;

		if (deleted >= amount)
			break;
	}

	return damage;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the MOSRotating object.

void MOSRotating::Destroy(bool notInherited)
{
    delete m_pAtomGroup;
    delete m_pDeepGroup;

    for (list<AEmitter *>::iterator itr = m_Wounds.begin(); itr != m_Wounds.end(); ++itr)
        delete (*itr);
    for (list<Attachable *>::iterator aItr = m_Attachables.begin(); aItr != m_Attachables.end(); ++aItr)
        delete (*aItr);

    destroy_bitmap(m_pFlipBitmap);
    destroy_bitmap(m_pFlipBitmapS);

// Not anymore; point to shared static bitmaps
//    destroy_bitmap(m_pTempBitmap);

    if (!notInherited)
        MOSprite::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the mass value of this ACDropShip, including the mass of its
//                  currently attached body parts and inventory.

float MOSRotating::GetMass() const
{
    float totalMass = MOSprite::GetMass();

    for (list<Attachable *>::const_iterator aItr = m_Attachables.begin(); aItr != m_Attachables.end(); ++aItr)
        totalMass += (*aItr)->GetMass();

    return totalMass;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the main Material of this MOSRotating.

Material const * MOSRotating::GetMaterial() const
{
//    if (m_pAtomGroup)
        return m_pAtomGroup->GetMaterial();
//    return 0;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  HitsMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets whether this MovableObject is set to collide with other
//                  MovableObject:s during travel.

bool MOSRotating::HitsMOs() const
{
    if (m_pAtomGroup)
        return m_pAtomGroup->HitsMOs();
    return false;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetDrawPriority
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the drawing priority of this MovableObject, if two things were
//                  overlap when copying to the terrain, the higher priority MO would
//                  end up getting drawn.

int MOSRotating::GetDrawPriority() const
{
    return INT_MAX;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetAtom
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Replaces the current AtomGroup of this MOSRotating with a new one.
// Arguments:       A reference to the new AtomGroup.
// Return value:    None.

void MOSRotating::SetAtom(AtomGroup *newAtom)
{
    delete m_pAtomGroup;
    m_pAtomGroup = newAtom;
}
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetToHitMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this MovableObject to collide with other MovableObjects during
//                  travel.

void MOSRotating::SetToHitMOs(bool hitMOs)
{
    if (m_pAtomGroup)
        m_pAtomGroup->SetToHitMOs(hitMOs);
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddRecoil
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds recoil effects to this MOSprite.

void MOSRotating::AddRecoil()
{
    m_RecoilOffset.SetXY(1, 0);
    m_RecoilOffset.RadRotate(m_Rotation.GetRadAngle() + c_PI);
    m_Recoiled = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CollideAtPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the collision response when another MO's Atom collides with
//                  this MO's physical representation. The effects will be applied
//                  directly to this MO, and also represented in the passed in HitData.

bool MOSRotating::CollideAtPoint(HitData &hd)
{
    hd.ResImpulse[HITOR].Reset();
    hd.ResImpulse[HITEE].Reset();

//    if (m_AlreadyHitBy.find(hd.Body[HITOR]->GetID()) == m_AlreadyHitBy.end())
//    {
//        m_AlreadyHitBy.insert(hd.Body[HITOR]->GetID());

    hd.HitRadius[HITEE] = (hd.HitPoint - m_Pos) * c_MPP;
/*
    // Cancel if both hitor and hitee's hitpoint radii are pointing int he same direction, meaning the objects are really tangled
    if (!hd.HitRadius[HITOR].IsZero() && hd.HitRadius[HITOR].Dot(hd.HitRadius[HITEE]) >= 0)
        return false;
*/
    hd.TotalMass[HITEE] = m_Mass;
    hd.MomInertia[HITEE] = m_pAtomGroup->GetMomentOfInertia();
    hd.HitVel[HITEE] = m_Vel + hd.HitRadius[HITEE].GetPerpendicular() * m_AngularVel;
    hd.VelDiff = hd.HitVel[HITOR] - hd.HitVel[HITEE];

    // Only do collision response for this if it appears the collision is happening in the 'right' direction, meaning away from the hitee collision normal
    // The wrong way happens when things are sunk into each other, and thus getting 'hooked' on each other
    if (hd.VelDiff.Dot(hd.BitmapNormal) < 0)
    {
        Vector hitAcc = -hd.VelDiff * (1 + (hd.Body[HITOR]->GetMaterial()->GetRestitution() * GetMaterial()->GetRestitution()));

        float hittorLever = hd.HitRadius[HITOR].GetPerpendicular().Dot(hd.BitmapNormal);
        float hitteeLever = hd.HitRadius[HITEE].GetPerpendicular().Dot(hd.BitmapNormal);
        hittorLever *= hittorLever;
        hitteeLever *= hitteeLever;
        float impulse = hitAcc.Dot(hd.BitmapNormal) / (((1 / hd.TotalMass[HITOR]) + (1 / hd.TotalMass[HITEE])) +
                        (hittorLever / hd.MomInertia[HITOR]) + (hitteeLever / hd.MomInertia[HITEE]));
    // TODO: Should the impfactor not be swapped? -EE vs -OR?")
        hd.ResImpulse[HITOR] = hd.BitmapNormal * impulse * hd.ImpulseFactor[HITOR];
        hd.ResImpulse[HITEE] = hd.BitmapNormal * -impulse * hd.ImpulseFactor[HITEE];

        // If a particle, which does not penetrate, but bounces, do any additional
        // effects of that bounce.
        if (!ParticlePenetration(hd))
        {
// TODO: Add blunt trauma effects here!")
            ;
        }

        // If the hittee is pinned, see if the collision's impulse is enough to dislodge it.
        float hiteePin = hd.Body[HITEE]->GetPinStrength();
        // See if it's pinned, and compare it to the impulse force from the collision
        if (m_PinStrength > 0 && hd.ResImpulse[HITEE].GetMagnitude() > m_PinStrength)
        {
            // Unpin and set the threshold to 0
            hd.Body[HITEE]->SetPinStrength(0);
        }
        // If not knocked loose, then move the impulse of the hitee to the hitor
        else if (hiteePin)
        {
// No good, causes crazy bounces
//            hd.ResImpulse[HITOR] -= hd.ResImpulse[HITEE];
            hd.ResImpulse[HITEE].Reset();
        }

        AddImpulseForce(hd.ResImpulse[HITEE], hd.HitRadius[HITEE]);
//        m_Vel += hd.ResImpulse[HITEE] / hd.mass[HITEE];
//        m_AngularVel += hd.HitRadius[HITEE].GetPerpendicular().Dot(hd.ResImpulse[HITEE]) / hd.MomInertia[HITEE];
    }
    else
        return false;

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnBounce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  bounces off of something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.

bool MOSRotating::OnBounce(HitData &hd)
{

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnSink
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  sink into something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.

bool MOSRotating::OnSink(HitData &hd)
{
/*
    Vector oldPos(m_Pos);
    m_Pos = pos;
    Draw(g_SceneMan.GetTerrainColorBitmap(), Vector(), g_DrawMask);
    Draw(g_SceneMan.GetTerrainMaterialBitmap(), Vector(), g_DrawAir);
    m_Pos = oldPos;
*/
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ParticlePenetration
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Determines whether a particle which has hit this MO will penetrate,
//                  and if so, whether it gets lodged or exits on the other side of this
//                  MO. Appropriate effects will be determined and applied ONLY IF there
//                  was penetration! If not, nothing will be affected.

bool MOSRotating::ParticlePenetration(HitData &hd)
{
    // Only particles can penetrate.
    if (!(dynamic_cast<MOPixel *>(hd.Body[HITOR]) || dynamic_cast<MOSParticle *>(hd.Body[HITOR])))
        return false;

    float impulseForce = hd.ResImpulse[HITEE].GetMagnitude();
    Material const * myMat = GetMaterial();
    float myStrength = myMat->GetIntegrity() / hd.Body[HITOR]->GetSharpness();


    // See if there is enough energy in the collision for the particle to penetrate
    if (impulseForce * hd.Body[HITOR]->GetSharpness() > myMat->GetIntegrity())
    {
        // Ok penetration happened, now figure out if and where the exit point
        // would be by tracing a rasterized line through the sprite.

        // Declare all vars needed for good ole' Bresenham..
        int intPos[2], delta[2], delta2[2], increment[2], bounds[2];
        int error, dom, sub, domSteps, subSteps;
        bool inside = false, exited = false, subStepped = false;

        // Lock all bitmaps involved outside the loop.
        acquire_bitmap(m_aSprite[m_Frame]);

        bounds[X] = m_aSprite[m_Frame]->w;
        bounds[Y] = m_aSprite[m_Frame]->h;

        // Figure out the entry position in the un-rotated sprite's coordinates.
        Vector entryPos = (g_SceneMan.ShortestDistance(m_Pos, hd.HitPoint) / m_Rotation).GetXFlipped(m_HFlipped) - m_SpriteOffset;
        intPos[X] = floorf(entryPos.m_X);
        intPos[Y] = floorf(entryPos.m_Y);

        // Get the un-rotated direction and max possible
        // travel length of the particle.
        Vector dir(max(bounds[X], bounds[Y]), 0);
        dir.AbsRotateTo(hd.HitVel[HITOR] / m_Rotation);
        dir = dir.GetXFlipped(m_HFlipped);

        // Bresenham's line drawing algorithm preparation
        delta[X] = floorf(entryPos.m_X + dir.m_X) - intPos[X];
        delta[Y] = floorf(entryPos.m_Y + dir.m_Y) - intPos[Y];
        domSteps = 0;
        subSteps = 0;

        if (delta[X] < 0)
        {
            increment[X] = -1;
            delta[X] = -delta[X];
        }
        else
            increment[X] = 1;

        if (delta[Y] < 0)
        {
            increment[Y] = -1;
            delta[Y] = -delta[Y];
        }
        else
            increment[Y] = 1;

        // Scale by 2, for better accuracy of the error at the first pixel
        delta2[X] = delta[X] << 1;
        delta2[Y] = delta[Y] << 1;

        // If X is dominant, Y is submissive, and vice versa.
        if (delta[X] > delta[Y])
        {
            dom = X;
            sub = Y;
        }
        else {
            dom = Y;
            sub = X;
        }
        error = delta2[sub] - delta[dom];

        // Bresenham's line drawing algorithm execution
        for (domSteps = 0; domSteps < delta[dom]; ++domSteps)
        {
            if (intPos[X] < 0 || intPos[X] >= bounds[X] ||
                intPos[Y] < 0 || intPos[Y] >= bounds[Y])
            {
                exited = false;
                break;
            }

            RTEAssert(is_inside_bitmap(m_aSprite[m_Frame], intPos[X], intPos[Y], 0), "Particle penetration test is outside of sprite!");

            // Check if we are inside the sprite.
            if (_getpixel(m_aSprite[m_Frame], intPos[X], intPos[Y]) != g_MaskColor)
            {
                inside = true;
                // Break if the particle can't force its way through any further.
                if (impulseForce <= myStrength) {
                    exited = false;
                    break;
                }
                // Do the resistance calculation which is retarding the
                // kinetic force of the penetrating particle by each pixel penetrated.
                impulseForce -= myStrength;
            }
            // If we are inside and are now outside, we have just exited!
            else if (inside)
            {
                impulseForce += myStrength;
                exited = true;
                break;
            }

            // Advance to the next pixel
            intPos[dom] += increment[dom];
            if (error >= 0) {
                intPos[sub] += increment[sub];
                ++subSteps;
                error -= delta2[dom];
            }
            error += delta2[sub];
        }
        // Unlock all bitmaps involved outside the loop.
        release_bitmap(m_aSprite[m_Frame]);

        if (m_pEntryWound)
        {
            // Add entry wound AEmitter to actor where the particle penetrated.
            AEmitter *pEntryWound = dynamic_cast<AEmitter *>(m_pEntryWound->Clone());
            pEntryWound->SetEmitAngle(dir.GetXFlipped(m_HFlipped).GetAbsRadAngle() + c_PI);
			pEntryWound->SetDamageMultiplier(hd.Body[HITOR]->WoundDamageMultiplier());
            // Adjust position so that it looks like the hole is actually *on* the Hitee.
            entryPos[dom] += increment[dom] * (pEntryWound->GetSpriteFrame()->w / 2);
			AddWound(pEntryWound, entryPos + m_SpriteOffset);
            pEntryWound = 0;
        }

        // Add exit wound AEmitter to actor, if applicable.
        if (exited)
        {
            Vector exitPos;
            exitPos[dom] = entryPos[dom] + (increment[dom] * domSteps);
            exitPos[sub] = entryPos[sub] + (increment[sub] * subSteps);
            if (m_pExitWound)
            {
                AEmitter *pExitWound = dynamic_cast<AEmitter *>(m_pExitWound->Clone());
                // Adjust position so that it looks like the hole is actually *on* the Hitee.
                exitPos[dom] -= increment[dom] * (pExitWound->GetSpriteFrame()->w / 2);
                pExitWound->SetEmitAngle(dir.GetXFlipped(m_HFlipped).GetAbsRadAngle());
				pExitWound->SetDamageMultiplier(hd.Body[HITOR]->WoundDamageMultiplier());
				AddWound(pExitWound, exitPos + m_SpriteOffset);
                pExitWound = 0;
            }

            // Set the exiting particle's position to where the exit wound is,
            // in world coordinates.
            hd.Body[HITOR]->SetPos(((exitPos + m_SpriteOffset) / m_Rotation) + m_Pos);

            // Finally apply the forces imposed on both this MOSRotating
            // and the hitting particle due to the penetrating and exiting hit.
            // These don't need to be manipulated if there was no exit, since this
            // absorbed all the energy, and the hittee gets deleted from the scene.
            hd.ResImpulse[HITEE] -= hd.ResImpulse[HITEE] * (impulseForce / hd.ResImpulse[HITEE].GetMagnitude());
            hd.ResImpulse[HITOR] = -(hd.ResImpulse[HITEE]);
        }
        // Particle got lodged inside this MOSRotating, so stop it and delete it from scene.
        else
        {
            // Set the exiting particle's position to where it looks lodged
//            hd.Body[HITOR]->SetPos(m_Pos - m_SpriteOffset + entryPos);
//            hd.Body[HITOR]->SetVel(Vector());
            hd.Body[HITOR]->SetToDelete(true);
            hd.Terminate[HITOR] = true;
        }

        // Report that penetration occured.
        return true;
    }

    // Report that penetration didn't occur and signal to client to
    // calculate bounce effects instead.
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GibThis
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gibs this, effectively destroying it and creating multiple gibs or
//                  pieces in its place.

void MOSRotating::GibThis(Vector impactImpulse, float internalBlast, MovableObject *pIgnoreMO)
{
    // Can't, or is already gibbed, so don't do anything
    if (m_MissionCritical || m_ToDelete)
        return;

    MovableObject *pGib = 0;
    float velMin, velRange, spread, angularVel;
    Vector gibROffset, gibVel;
    for (list<Gib>::iterator gItr = m_Gibs.begin(); gItr != m_Gibs.end(); ++gItr)
    {
		// Throwing out gibs
        for (int i = 0; i < (*gItr).GetCount(); ++i)
        {
            // Make a copy after the preset particle
            // THIS IS A TIME SINK, takes up the vast bulk of time of GibThis
			{
				// Create gibs
				pGib = dynamic_cast<MovableObject *>((*gItr).GetParticlePreset()->Clone());
			}

            // Generate the velocities procedurally
            if ((*gItr).GetMinVelocity() == 0 && (*gItr).GetMaxVelocity() == 0)
            {
                velMin = internalBlast / pGib->GetMass();
                velRange = 10.0f;
            }
            // Use the ones defined already
            else
            {
                velMin = (*gItr).GetMinVelocity();
                velRange = (*gItr).GetMaxVelocity() - (*gItr).GetMinVelocity();
            }
            spread = (*gItr).GetSpread();
            gibROffset = RotateOffset((*gItr).GetOffset());
            // Put variation on the lifetime, if it's not set to be endless
            if (pGib->GetLifetime() != 0)
                pGib->SetLifetime(pGib->GetLifetime() * (1.0F + ((*gItr).GetLifeVariation() * RandomNormalNum())));
            // Set up its position and velocity according to the parameters of this AEmitter.
            pGib->SetPos(m_Pos + gibROffset/*Vector(m_Pos.m_X + 5 * NormalRand(), m_Pos.m_Y + 5 * NormalRand())*/);
            pGib->SetRotAngle(m_Rotation.GetRadAngle() + pGib->GetRotMatrix().GetRadAngle());
            // Rotational angle
            pGib->SetAngularVel((pGib->GetAngularVel() * 0.35F) + (pGib->GetAngularVel() * 0.65F / pGib->GetMass()) * RandomNum());
            // Make it rotate away in the appropriate direction depending on which side of the object it is on
            // If the object is far to the relft or right of the center, make it always rotate outwards to some degree
            if (gibROffset.m_X > m_aSprite[0]->w / 3)
            {
                float offCenterRatio = gibROffset.m_X / (m_aSprite[0]->w / 2);
                angularVel = fabs(pGib->GetAngularVel() * 0.5);
                angularVel += fabs(pGib->GetAngularVel() * 0.5 * offCenterRatio);
                pGib->SetAngularVel(angularVel * (gibROffset.m_X > 0 ? -1 : 1));
            }
            // Gib is too close to center to always make it rotate in one direction, so give it a baseline rotation and then randomize
            else
            {
                pGib->SetAngularVel((pGib->GetAngularVel() * 0.5F + pGib->GetAngularVel() * RandomNum()) * (RandomNormalNum() > 0.0F ? 1.0F : -1.0F));
            }

// TODO: Optimize making the random angles!")
            {
				// Pretty much always zero
                gibVel = gibROffset;
				if (gibVel.IsZero())
					gibVel.SetXY(velMin + RandomNum(0.0F, velRange), 0);
				else
					gibVel.SetMagnitude(velMin + RandomNum(0.0F, velRange));
				gibVel.RadRotate(impactImpulse.GetAbsRadAngle() + spread * RandomNormalNum());
// Don't! the offset was already rotated!
//                gibVel = RotateOffset(gibVel);
                // Distribute any impact implse out over all the gibs
//                gibVel += (impactImpulse / m_Gibs.size()) / pGib->GetMass();
            }

            // Only add the velocity of the parent if it's suppposed to
            if ((*gItr).InheritsVelocity())
                pGib->SetVel(m_Vel + gibVel);
            else
                pGib->SetVel(gibVel);

            // Set the gib to not hit a specific MO
            if (pIgnoreMO)
                pGib->SetWhichMOToNotHit(pIgnoreMO);

            // Add the gib to the scene
            g_MovableMan.AddParticle(pGib);
            pGib = 0;
        }
    }

    // Throw out all the attachables
    Attachable *pAttachable = 0;
    for (list<Attachable *>::iterator aItr = m_Attachables.begin(); aItr != m_Attachables.end(); ) //NOTE: No increment to handle RemoveAttachable removing the object
    {
        RTEAssert((*aItr), "Broken Attachable!");
        if (!(*aItr))
            continue;

        // Get handy handle to the object we're putting
        pAttachable = *aItr;

		// TODO: Rework this whole system
        // Generate the velocities procedurally
        velMin = internalBlast / (1 + pAttachable->GetMass());
        velRange = 10.0f;

        // Rotational angle velocity
        pAttachable->SetAngularVel((pAttachable->GetAngularVel() * 0.35F) + (pAttachable->GetAngularVel() * 0.65F / pAttachable->GetMass()) * RandomNum());
        // Make it rotate away in the appropriate direction depending on which side of the object it is on
        // If the object is far to the relft or right of the center, make it always rotate outwards to some degree
        if (pAttachable->GetParentOffset().m_X > m_aSprite[0]->w / 3)
        {
            float offCenterRatio = pAttachable->GetParentOffset().m_X / (m_aSprite[0]->w / 2);
            angularVel = fabs(pAttachable->GetAngularVel() * 0.5);
            angularVel += fabs(pAttachable->GetAngularVel() * 0.5 * offCenterRatio);
            pAttachable->SetAngularVel(angularVel * (pAttachable->GetParentOffset().m_X > 0 ? -1 : 1));
        }
        // Gib is too close to center to always make it rotate in one direction, so give it a baseline rotation and then randomize
        else
        {
            pAttachable->SetAngularVel((pAttachable->GetAngularVel() * 0.5F + pAttachable->GetAngularVel() * RandomNum()) * (RandomNormalNum() > 0.0F ? 1.0F : -1.0F));
        }

// TODO: Optimize making the random angles!")
        gibVel = pAttachable->GetParentOffset();
        if (gibVel.IsZero())
            gibVel.SetXY(velMin + RandomNum(0.0F, velRange), 0);
        else
            gibVel.SetMagnitude(velMin + RandomNum(0.0F, velRange));
        gibVel.RadRotate(impactImpulse.GetAbsRadAngle());
        pAttachable->SetVel(m_Vel + gibVel);

        // Set the gib to not hit a specific MO
        if (pIgnoreMO)
            pAttachable->SetWhichMOToNotHit(pIgnoreMO);

        // Safely remove attachable and add it to the scene
        ++aItr;
        RemoveAttachable(pAttachable);
        g_MovableMan.AddParticle(pAttachable);
        pAttachable = 0;
    }
    // Clear the attachables list, all the attachables ownership have been handed to the movableman
    m_Attachables.clear();
    m_AllAttachables.clear();

    // Play the gib sound
    m_GibSound.Play(m_Pos);

    // Flash post effect if it is defined
    if (m_pScreenEffect && m_EffectOnGib && (m_EffectAlwaysShows || !g_SceneMan.ObscuredPoint(m_Pos.GetFloorIntX(), m_Pos.GetFloorIntY())))
    {
        // Set the screen effect to draw at the final post processing stage
		g_PostProcessMan.RegisterPostEffect(m_Pos, m_pScreenEffect, m_ScreenEffectHash, 255, m_EffectRotAngle);
    }

    // Things breaking apart makes alarming noises!
    if (m_LoudnessOnGib > 0)
        g_MovableMan.RegisterAlarmEvent(AlarmEvent(m_Pos, m_Team, m_LoudnessOnGib));

    // Mark this for deletion!
    m_ToDelete = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  MoveOutOfTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether any of the Atom:s in this MovableObject are on top of
//                  terrain pixels, and if so, attempt to move this out so none of this'
//                  Atoms are on top of the terrain any more.

bool MOSRotating::MoveOutOfTerrain(unsigned char strongerThan)
{
    return m_pAtomGroup->ResolveTerrainIntersection(m_Pos, m_Rotation, strongerThan);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ApplyForces
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gathers and applies the global and accumulated forces. Then it clears
//                  out the force list.Note that this does NOT apply the accumulated
//                  impulses (impulse forces)!

void MOSRotating::ApplyForces()
{
    float deltaTime = g_TimerMan.GetDeltaTimeSecs();

    // Apply the rotational effects of all the forces accumulated during the Update()
    for (deque<pair<Vector, Vector> >::iterator fItr = m_Forces.begin();
         fItr != m_Forces.end(); ++fItr) {
        // Continuous force application to rotational velocity.
        if (!(*fItr).second.IsZero())
            m_AngularVel += ((*fItr).second.GetPerpendicular().Dot((*fItr).first) /
                            m_pAtomGroup->GetMomentOfInertia()) * deltaTime;
    }

    MOSprite::ApplyForces();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ApplyImpulses
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gathers and applies the accumulated impulse forces. Then it clears
//                  out the impulse list.Note that this does NOT apply the accumulated
//                  regular forces (non-impulse forces)!

void MOSRotating::ApplyImpulses()
{
    // Apply the rotational effects of all the impulses accumulated during the Update()
    for (deque<pair<Vector, Vector> >::iterator iItr = m_ImpulseForces.begin();
         iItr != m_ImpulseForces.end(); ++iItr) {
        // Impulse force application to the rotational velocity of this MO.
        if (!(*iItr).second.IsZero())
            m_AngularVel += (*iItr).second.GetPerpendicular().Dot((*iItr).first) /
                            m_pAtomGroup->GetMomentOfInertia();
    }

    // See if the impulses are enough to gib this
    Vector totalImpulse;
    for (deque<pair<Vector, Vector> >::iterator iItr = m_ImpulseForces.begin(); iItr != m_ImpulseForces.end(); ++iItr)
    {
        totalImpulse += (*iItr).first;
    }
    // If impulse gibbing threshold is enabled for this, see if it's below the total impulse force
    if (m_GibImpulseLimit > 0 && totalImpulse.GetMagnitude() > m_GibImpulseLimit)
        GibThis(totalImpulse);

    MOSprite::ApplyImpulses();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ResetAllTimers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resest all the timers used by this. Can be emitters, etc. This is to
//                  prevent backed up emissions to come out all at once while this has been
//                  held dormant in an inventory.

void MOSRotating::ResetAllTimers()
{
    MovableObject::ResetAllTimers();

    for (list<AEmitter *>::iterator emitter = m_Wounds.begin(); emitter != m_Wounds.end(); ++emitter)
        (*emitter)->ResetAllTimers();

    for (list<Attachable *>::iterator attachable = m_Attachables.begin(); attachable != m_Attachables.end(); ++attachable)
        (*attachable)->ResetAllTimers();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RestDetection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does the calculations necessary to detect whether this MO appears to
//                  have has settled in the world and is at rest or not. IsAtRest()
//                  retreves the answer.

void MOSRotating::RestDetection()
{
    MOSprite::RestDetection();

    // Rotational settling detection.
    if (((m_AngularVel > 0 && m_PrevAngVel < 0) || (m_AngularVel < 0 && m_PrevAngVel > 0)) && m_RestThreshold >= 0) {
        if (m_AngOscillations >= 2)
            m_ToSettle = true;
        else
            ++m_AngOscillations;
    }
    else
        m_AngOscillations = 0;

//    if (fabs(m_AngularVel) >= 1.0)
//        m_RestTimer.Reset();

    if (fabs(m_Rotation.GetRadAngle() - m_PrevRotation.GetRadAngle()) >= 0.01)
        m_RestTimer.Reset();

    // If we seem to be about to settle, make sure we're not flying in the air still
    if (m_ToSettle || IsAtRest())
    {
        if (g_SceneMan.OverAltitude(m_Pos, m_MaxRadius + 4, 3))
        {
            m_RestTimer.Reset();
            m_ToSettle = false;
        }
// TODO: REMOVE
//        bool KUK = g_SceneMan.OverAltitude(m_Pos, m_MaxRadius + 4, 3);
    }

    m_PrevRotation = m_Rotation;
    m_PrevAngVel = m_AngularVel;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsOnScenePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this' current graphical representation overlaps
//                  a point in absolute scene coordinates.

bool MOSRotating::IsOnScenePoint(Vector &scenePoint) const
{
    if (!m_aSprite[m_Frame])
        return false;
// TODO: TAKE CARE OF WRAPPING
/*
    // Take care of wrapping situations
    bitmapPos = m_Pos + m_BitmapOffset;
    Vector aScenePoint[4];
    aScenePoint[0] = scenePoint;
    int passes = 1;

    // See if need to double draw this across the scene seam if we're being drawn onto a scenewide bitmap
    if (targetPos.IsZero())
    {
        if (g_SceneMan.SceneWrapsX())
        {
            if (bitmapPos.m_X < m_pFGColor->w)
            {
                aScenePoint[passes] = aScenePoint[0];
                aScenePoint[passes].m_X += g_SceneMan.GetSceneWidth();
                passes++;
            }
            else if (aScenePoint[0].m_X > pTargetBitmap->w - m_pFGColor->w)
            {
                aScenePoint[passes] = aScenePoint[0];
                aScenePoint[passes].m_X -= g_SceneMan.GetSceneWidth();
                passes++;
            }
        }
        if (g_SceneMan.SceneWrapsY())
        {
            
        }
    }

    // Check all the passes needed
    for (int i = 0; i < passes; ++i)
    {
        if (IsWithinBox(aScenePoint[i], m_Pos + m_BitmapOffset, m_pFGColor->w, m_pFGColor->h))
        {
            if (getpixel(m_pFGColor, aScenePoint[i].m_X, aScenePoint[i].m_Y) != g_MaskColor ||
               (m_pBGColor && getpixel(m_pBGColor, aScenePoint[i].m_X, aScenePoint[i].m_Y) != g_MaskColor) ||
               (m_pMaterial && getpixel(m_pMaterial, aScenePoint[i].m_X, aScenePoint[i].m_Y) != g_MaterialAir))
               return true;
        }
    }
*/
    if (WithinBox(scenePoint, m_Pos.m_X - m_MaxRadius, m_Pos.m_Y - m_MaxRadius, m_Pos.m_X + m_MaxRadius, m_Pos.m_Y + m_MaxRadius))
    {
        // Get scene point in object's relative space
        Vector spritePoint = scenePoint - m_Pos;
        // Rotate it back to correspond with the unrotated bitmap
        spritePoint = UnRotateOffset(spritePoint);
        // Check for overlap on the local rotated relative point. subtract spriteoffset to get into sprite bitmap's space
        int pixel = getpixel(m_aSprite[m_Frame], spritePoint.m_X - m_SpriteOffset.m_X, spritePoint.m_Y - m_SpriteOffset.m_Y);
        // Check that it isn't outside the bitmap, and not of the key color
        if (pixel != -1 && pixel != g_MaskColor)
           return true;
    }

    // Check the attachables too, backward since the latter ones tend to be larger, and therefore more likeyl to be on the point
    for (list<Attachable *>::const_reverse_iterator aItr = m_Attachables.rbegin(); aItr != m_Attachables.rend(); ++aItr)
    {
        if ((*aItr)->IsOnScenePoint(scenePoint))
            return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  EraseFromTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Cuts this' silhouette out from the terrain's material and color layers.

void MOSRotating::EraseFromTerrain()
{
    Vector pivot = -m_SpriteOffset;

    if (m_HFlipped)
    {
        // Create intermediate flipping bitmap if there isn't one yet
        // Don't size the intermediate bitmaps to teh m_Scale, because the scaling happens after they are done
        if (!m_pFlipBitmap)
            m_pFlipBitmap = create_bitmap_ex(8, m_aSprite[m_Frame]->w, m_aSprite[m_Frame]->h);
        clear_to_color(m_pFlipBitmap, g_MaskColor);

        // Draw eitehr the source color bitmap or the intermediate material bitmap onto the intermediate flipping bitmap
        draw_sprite_h_flip(m_pFlipBitmap, m_aSprite[m_Frame], 0, 0);

        pivot.m_X = m_pFlipBitmap->w + m_SpriteOffset.m_X;
    }

    deque<MOPixel *> pixels = g_SceneMan.GetTerrain()->EraseSilhouette(m_HFlipped ? m_pFlipBitmap : m_aSprite[m_Frame], m_Pos, pivot, m_Rotation, m_Scale, false);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DeepCheck
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if any of this' deep group atmos are on top of the terrain, and
//                  if so, erases this' silhouette from the terrain.

bool MOSRotating::DeepCheck(bool makeMOPs, int skipMOP, int maxMOPs)
{
    // Check for deep penetration of the terrain and
    // generate splash of MOPixels accordingly.
    if (m_pDeepGroup && (m_pDeepGroup->InTerrain() || m_ForceDeepCheck))
    {
        m_ForceDeepCheck = false;
        m_DeepHardness = true;
/*
        // Just make the outline of this disappear from the terrain
        {
// TODO: These don't work at all because they're drawing shapes of color 0 to an intermediate field of 0!
            Draw(g_SceneMan.GetTerrain()->GetFGColorBitmap(), Vector(), g_DrawMask, true);
            Draw(g_SceneMan.GetTerrain()->GetMaterialBitmap(), Vector(), g_DrawAir, true);
        }
*/
// TODO: This stuff is just way too slow, EraseSilhouette is a hog
        // Make particles fly at least somewhat
        float velMag = MAX(10.0f, m_Vel.GetMagnitude());
        float splashDir = m_Vel.m_X >= 0 ? 1 : -1;
        float splashRatio = g_MovableMan.GetSplashRatio();
        float tally = 0.0;
        int depth = m_pDeepGroup->GetDepth() >> 1;
        Vector pivot = -m_SpriteOffset;

        if (m_HFlipped)
        {
            // Create intermediate flipping bitmap if there isn't one yet
            // Don't size the intermediate bitmaps to teh m_Scale, because the scaling happens after they are done
            if (!m_pFlipBitmap)
                m_pFlipBitmap = create_bitmap_ex(8, m_aSprite[m_Frame]->w, m_aSprite[m_Frame]->h);
            clear_to_color(m_pFlipBitmap, g_MaskColor);

            // Draw eitehr the source color bitmap or the intermediate material bitmap onto the intermediate flipping bitmap
            draw_sprite_h_flip(m_pFlipBitmap, m_aSprite[m_Frame], 0, 0);

            pivot.m_X = m_pFlipBitmap->w + m_SpriteOffset.m_X;
        }

        {
            // Particle generation
            // Erase the silhouette and get all the pixels that were created as a result
            deque<MOPixel *> pixels = g_SceneMan.GetTerrain()->EraseSilhouette(m_HFlipped ? m_pFlipBitmap : m_aSprite[m_Frame], m_Pos, pivot, m_Rotation, m_Scale, makeMOPs, skipMOP, maxMOPs);

            for (deque<MOPixel *>::iterator itr = pixels.begin(); itr != pixels.end(); ++itr)
            {
                tally += splashRatio;
                if (tally >= 1.0)
                {
                    tally -= 1.0;
                    (*itr)->SetPos((*itr)->GetPos() - m_Vel.GetNormalized() * depth);
					(*itr)->SetVel(Vector(velMag * RandomNum(0.0F, splashDir), -RandomNum(0.0F, velMag)));
                    m_DeepHardness += (*itr)->GetMaterial()->GetIntegrity() * (*itr)->GetMaterial()->GetPixelDensity();
                    g_MovableMan.AddParticle(*itr);
                    *itr = 0;
                }
                else
                {
                    delete (*itr);
                    *itr = 0;
                }
            }
        }
// EXPERIMENTAL
        // Move the remaining out and away from teh terrain to help unsticking
        MoveOutOfTerrain(g_MaterialSand);

        return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PreTravel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does stuff that needs to be done before Travel(). Always call before
//                  calling Travel.
// Arguments:       None.
// Return value:    None.

void MOSRotating::PreTravel()
{
    MOSprite::PreTravel();

    // If this is going slow enough, check for and redraw the MOID representations of
    // any other MOSRotating:s that may be overlapping this
	if (m_GetsHitByMOs && m_HitsMOs && m_Vel.m_X < 2.0 && m_Vel.m_Y < 2.0 && g_SettingsMan.PreciseCollisions())
        g_MovableMan.RedrawOverlappingMOIDs(this);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Travel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Travels this MovableObject, using its physical representation.

void MOSRotating::Travel()
{
    MOSprite::Travel();

    // Pinned objects don't travel!
    if (m_PinStrength)
        return;

    float deltaTime = g_TimerMan.GetDeltaTimeSecs();

    // Reset the travel impulse for this frame
    m_TravelImpulse.Reset();


	// Set the atom to ignore this MO and all of it's children if PreciseCollisions are off
	// When PreciseCollisions are on PreTravel takes care of it by removing this MO's silhouette from MO layer
	if (!g_SettingsMan.PreciseCollisions())
	{
		std::vector<MOID> MOIDs;
		GetMOIDs(MOIDs);
		for (vector<MOID>::const_iterator aItr = MOIDs.begin(); aItr != MOIDs.end(); ++aItr)
			m_pAtomGroup->AddMOIDToIgnore(*aItr);
	}

    // Set the atom to ignore a certain MO, if set and applicable.
    if (m_HitsMOs && m_pMOToNotHit && g_MovableMan.ValidMO(m_pMOToNotHit) && !m_MOIgnoreTimer.IsPastSimTimeLimit())
    {
        MOID root = m_pMOToNotHit->GetID();
        int footprint = m_pMOToNotHit->GetMOIDFootprint();
        for (int i = 0; i < footprint; ++i)
            m_pAtomGroup->AddMOIDToIgnore(root + i);
    }

    /////////////////////////////////
    // AtomGroup travel

	if (!IsTooFast())
		m_pAtomGroup->Travel(deltaTime, true, true, g_SceneMan.SceneIsLocked());

    // Now clear out the ignore override for next frame
    m_pAtomGroup->ClearMOIDIgnoreList();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PostTravel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does stuff that needs to be done after Update(). Always call after
//                  calling Update.

void MOSRotating::PostTravel()
{
    // Check for stupid velocities to gib instead of outright deletion that MOSprite::PostTravel() will do
    if (IsTooFast())
        GibThis();

    MOSprite::PostTravel();

    // Check if travel hits created enough impulse forces to gib this
    if (m_GibImpulseLimit > 0 && m_TravelImpulse.GetMagnitude() > m_GibImpulseLimit)
        GibThis();

    // Reset
    m_DeepHardness = 0;

    // Check for deep penetration of the terrain and
    // generate splash of MOPixels accordingly.
// TODO: don't hardcode the MOPixel limits!
    if (g_MovableMan.IsMOSubtractionEnabled() && (m_ForceDeepCheck || m_DeepCheck))
        DeepCheck(true, 8, 50);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MOSRotating. Supposed to be done every frame.

void MOSRotating::Update()
{

#if defined DEBUG_BUILD || defined MIN_DEBUG_BUILD
	RTEAssert(m_MOID == g_NoMOID || (m_MOID >= 0 && m_MOID < g_MovableMan.GetMOIDCount()), "MOID out of bounds!");
#endif

    MOSprite::Update();

	if (m_InheritEffectRotAngle)
		m_EffectRotAngle = m_Rotation.GetRadAngle();

    // Align the orienatation with the velocity vector, if any
    if (m_OrientToVel > 0 && m_Vel.GetLargest() > 5.0)
    {
        // Clamp
        if (m_OrientToVel > 1.0)
            m_OrientToVel = 1.0;

        // Velocity influence
        float velInfluence = m_OrientToVel < 1.0 ? m_Vel.GetMagnitude() / 100 : 1.0f;
        if (velInfluence > 1.0)
            velInfluence = 1.0;

        // Figure the difference in current velocity vector and 
        float radsToGo = m_Rotation.GetRadAngleTo(m_Vel.GetAbsRadAngle());
        m_Rotation += (radsToGo * m_OrientToVel * velInfluence);
    }

    // Update all the attached wound emitters
    for (list<AEmitter *>::iterator itr = m_Wounds.begin();
        itr != m_Wounds.end(); ++itr)
    {
        if ((*itr))
        {
            (*itr)->SetJointPos(m_Pos + RotateOffset((*itr)->GetParentOffset()));
			if ((*itr)->InheritsRotAngle())
				(*itr)->SetRotAngle(m_Rotation.GetRadAngle());
//            (*itr)->SetEmitAngle(m_Rotation);
            (*itr)->Update();
        }
        else
            RTEAbort("Broken emitter!!");
    }

    // Update all the attachables
    Attachable *pAttachable = 0;
    for (list<Attachable *>::iterator aItr = m_Attachables.begin(); aItr != m_Attachables.end(); ) // NOTE NO INCCREMENT!
    {
        RTEAssert((*aItr), "Broken Attachable!");
        if (!(*aItr))
            continue;

        pAttachable = *aItr;
        ++aItr;

        pAttachable->SetHFlipped(m_HFlipped);
        pAttachable->SetJointPos(m_Pos + RotateOffset((pAttachable)->GetParentOffset()));
        if (pAttachable->InheritsRotAngle())
        {
            pAttachable->SetRotAngle(m_Rotation.GetRadAngle());
        }
        pAttachable->Update();

        ApplyAttachableForces(pAttachable);
    }

    // Create intermediate flipping bitmap if there isn't one yet
    if (m_HFlipped && !m_pFlipBitmap && m_aSprite[0])
        m_pFlipBitmap = create_bitmap_ex(8, m_aSprite[0]->w, m_aSprite[0]->h);

	if (m_HFlipped && !m_pFlipBitmapS && m_aSprite[0])
        m_pFlipBitmapS = create_bitmap_ex(c_MOIDLayerBitDepth, m_aSprite[0]->w, m_aSprite[0]->h);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawMOIDIfOverlapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the MOID representation of this to the SceneMan's MOID layer if
//                  this is found to potentially overlap another MovableObject.

bool MOSRotating::DrawMOIDIfOverlapping(MovableObject *pOverlapMO)
{
    if (pOverlapMO != this && m_GetsHitByMOs)
    {
        float combinedRadii = m_MaxRadius + pOverlapMO->GetRadius();
        Vector otherPos = pOverlapMO->GetPos();

        // Quick check
        if (fabs(otherPos.m_X - m_Pos.m_X) > combinedRadii || fabs(otherPos.m_Y - m_Pos.m_Y) > combinedRadii)
            return false;

        // Check if the offset is within the combined radii of the two object, and therefore might be overlapping
        Vector offset = otherPos - m_Pos;
        if (offset.GetMagnitude() < combinedRadii)
        {
            // They may be overlapping, so draw the MOID rep of this to the MOID layer
            Draw(g_SceneMan.GetMOIDBitmap(), Vector(), g_DrawMOID, true);
            return true;
        }
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateChildMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this MO register itself and all its attached children in the
//                  MOID register and get ID:s for itself and its children for this frame.

void MOSRotating::UpdateChildMOIDs(vector<MovableObject *> &MOIDIndex,
                                  MOID rootMOID,
                                  bool makeNewMOID)
{
    // Register all the eligible attachables
    for (list<Attachable *>::iterator aItr = m_Attachables.begin(); aItr != m_Attachables.end(); ++aItr)
    {
// TODO: Which should it be, don't register at all, or register as same as parent??
        if ((*aItr)->GetsHitByMOs())
            (*aItr)->UpdateMOID(MOIDIndex, m_RootMOID, (*aItr)->GetsHitByMOs());
    }

    MOSprite::UpdateChildMOIDs(MOIDIndex, m_RootMOID, makeNewMOID);
}


/// <summary>
/// Attaches the passed in Attachable and adds it to the list of attachables, not changing its parent offset and not treating it as hardcoded.
/// </summary>
/// <param name="pAttachable">The Attachable to attach.</param>
void MOSRotating::AddAttachable(Attachable *pAttachable)
{
	if (pAttachable)
	{
		AddAttachable(pAttachable, pAttachable->GetParentOffset());
	}
}


/// <summary>
/// Attaches the passed in Attachable and adds it to the list of attachables, changing its parent offset to the passed in Vector but not treating it as hardcoded.
/// </summary>
/// <param name="pAttachable">The Attachable to add.</param>
/// <param name="parentOffsetToSet">The vector to set as the Attachable's parent offset.</param>
void MOSRotating::AddAttachable(Attachable *pAttachable, const Vector& parentOffsetToSet)
{
	AddAttachable(pAttachable, parentOffsetToSet, false);
}


/// <summary>
/// Attaches the passed in Attachable and adds it to the list of attachables, not changing its parent offset but treating it as hardcoded depending on the passed in boolean.
/// </summary>
/// <param name="pAttachable">The Attachable to add.</param>
/// <param name="isHardcodedAttachable">Whether or not the Attachable should be treated as hardcoded.</param>
void MOSRotating::AddAttachable(Attachable *pAttachable, bool isHardcodedAttachable)
{
	if (pAttachable)
	{
		AddAttachable(pAttachable, pAttachable->GetParentOffset(), isHardcodedAttachable);
	}
}


/// <summary>
/// Attaches the passed in Attachable and adds it to the list of attachables, changing its parent offset to the passed in Vector and treating it as hardcoded depending on the passed in boolean.
/// </summary>
/// <param name="pAttachable">The Attachable to add.</param>
/// <param name="parentOffsetToSet">The vector to set as the Attachable's parent offset.</param>
/// <param name="isHardcodedAttachable">Whether or not the Attachable should be treated as hardcoded.</param>
void MOSRotating::AddAttachable(Attachable *pAttachable, const Vector & parentOffsetToSet, bool isHardcodedAttachable)
{
	if (pAttachable)
	{
		pAttachable->Attach(this, parentOffsetToSet);

		// Set the attachable's subgroup ID to it's Unique ID to avoid any possible conflicts when adding atoms to parent group.
		pAttachable->SetAtomSubgroupID(pAttachable->GetUniqueID());

		if (pAttachable->CanCollideWithTerrainWhenAttached())
		{
			pAttachable->EnableTerrainCollisions(true);
		}

		if (!isHardcodedAttachable)
		{
			m_Attachables.push_back(pAttachable);
		}
		m_AllAttachables.push_back(pAttachable);
	}
}


/// <summary>
/// Detaches the Attachable corresponding to the passed in UniqueId, and removes it from the appropriate attachable lists
/// </summary>
/// <param name="attachableUniqueId">The UniqueId of the the attachable to remove</param>
/// <returns>False if the attachable is invalid, otherwise true</returns>
bool MOSRotating::RemoveAttachable(long attachableUniqueId)
{
	MovableObject *attachableAsMovableObject = g_MovableMan.FindObjectByUniqueID(attachableUniqueId);
	if (attachableAsMovableObject)
	{
		return RemoveAttachable((Attachable *)attachableAsMovableObject);
	}
	return false;
}


/// <summary>
/// Detaches the passed in Attachable and removes it from the appropriate attachable lists
/// </summary>
/// <param name="pAttachable">The attachable to remove</param>
/// <returns>False if the attachable is invalid, otherwise true</returns>
bool MOSRotating::RemoveAttachable(Attachable *pAttachable) {
	if (pAttachable) {
		if (m_Attachables.size() > 0) {
			m_Attachables.remove(pAttachable);
		}
		if (m_AllAttachables.size() > 0) {
			m_AllAttachables.remove(pAttachable);
		}
		pAttachable->ToDeleteWithParent() ? pAttachable->SetToDelete() : pAttachable->Detach();
		return true;
	}
	return false;
}


/// <summary>
/// Either detaches or deletes all of this MOSRotating's attachables
/// </summary>
/// <param name="destroy">Whether to detach or delete the attachables. Setting this to true deletes them, setting it to false detaches them</param>
void MOSRotating::DetachOrDestroyAll(bool destroy)
{
	for (list<Attachable *>::const_iterator aItr = m_AllAttachables.begin(); aItr != m_AllAttachables.end(); ++aItr)
	{
		if (destroy)
			delete (*aItr);
		else
			(*aItr)->Detach();
	}

	m_AllAttachables.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Puts all MOIDs associated with this MO and all it's descendants into MOIDs vector
// Arguments:       Vector to store MOIDs
// Return value:    None.

void MOSRotating::GetMOIDs(std::vector<MOID> &MOIDs) const
{
	// Get MOIDs all the eligible attachables
	for (list<Attachable *>::const_iterator aItr = m_Attachables.begin(); aItr != m_Attachables.end(); ++aItr)
		(*aItr)->GetMOIDs(MOIDs);

	// Get self MOID
	MOSprite::GetMOIDs(MOIDs);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this MOSRotating's current graphical representation to a
//                  BITMAP of choice.

void MOSRotating::Draw(BITMAP *pTargetBitmap,
                       const Vector &targetPos,
                       DrawMode mode,
                       bool onlyPhysical) const
{
    RTEAssert(m_aSprite, "No sprite bitmaps loaded to draw!");
    RTEAssert(m_Frame >= 0 && m_Frame < m_FrameCount, "Frame is out of bounds!");
    
    // Only draw MOID if this gets hit by MO's and it has a valid MOID assigned to it
    if (mode == g_DrawMOID && (!m_GetsHitByMOs || m_MOID == g_NoMOID))
        return;

	BITMAP * pTempBitmap = m_pTempBitmap;
	BITMAP * pFlipBitmap = m_pFlipBitmap;
	int keyColor = g_MaskColor;

	// Switch to non 8-bit drawing mode if we're drawing onto MO layer
	if (mode == g_DrawMOID || mode == g_DrawNoMOID)
	{
		pTempBitmap = m_pTempBitmapS;
		pFlipBitmap = m_pFlipBitmapS;
		keyColor = g_MOIDMaskColor;
	}

    Vector spritePos(m_Pos.GetFloored() - targetPos);

    if (m_Recoiled)
        spritePos += m_RecoilOffset;

    // If we're drawing a material silhouette, then create an intermediate material bitmap as well
    if (mode != g_DrawColor && mode != g_DrawTrans)
    {
        clear_to_color(pTempBitmap, keyColor);

// TODO: Fix that MaterialAir and KeyColor don't work at all because they're drawing 0 to a field of 0's
        // Draw the requested material sihouette on the material bitmap
        if (mode == g_DrawMaterial)
            draw_character_ex(pTempBitmap, m_aSprite[m_Frame], 0, 0, m_SettleMaterialDisabled ? GetMaterial()->GetIndex() : GetMaterial()->GetSettleMaterial(), -1);
        else if (mode == g_DrawAir)
            draw_character_ex(pTempBitmap, m_aSprite[m_Frame], 0, 0, g_MaterialAir, -1);
        else if (mode == g_DrawMask)
            draw_character_ex(pTempBitmap, m_aSprite[m_Frame], 0, 0, keyColor, -1);
        else if (mode == g_DrawWhite)
            draw_character_ex(pTempBitmap, m_aSprite[m_Frame], 0, 0, g_WhiteColor, -1);
        else if (mode == g_DrawMOID)
            draw_character_ex(pTempBitmap, m_aSprite[m_Frame], 0, 0, m_MOID, -1);
        else if (mode == g_DrawNoMOID)
            draw_character_ex(pTempBitmap, m_aSprite[m_Frame], 0, 0, g_NoMOID, -1);
        else if (mode == g_DrawRedTrans)
            draw_trans_sprite(pTempBitmap, m_aSprite[m_Frame], 0, 0);
        else
        {
//            return;
//            RTEAbort("Unknown draw mode selected in MOSRotating::Draw()!");
        }
    }

    // Take care of wrapping situations
    Vector aDrawPos[4];
    int passes = 1;

    aDrawPos[0] = spritePos;

    // Only bother with wrap drawing if the scene actually wraps around
    if (g_SceneMan.SceneWrapsX())
    {
        // See if need to double draw this across the scene seam if we're being drawn onto a scenewide bitmap
        if (targetPos.IsZero() && m_WrapDoubleDraw)
        {
            if (spritePos.m_X < m_MaxDiameter)
            {
                aDrawPos[passes] = spritePos;
                aDrawPos[passes].m_X += pTargetBitmap->w;
                passes++;
            }
            else if (spritePos.m_X > pTargetBitmap->w - m_MaxDiameter)
            {
                aDrawPos[passes] = spritePos;
                aDrawPos[passes].m_X -= pTargetBitmap->w;
                passes++;
            }
        }
        // Only screenwide target bitmap, so double draw within the screen if the screen is straddling a scene seam
        else if (m_WrapDoubleDraw)
        {
            if (targetPos.m_X < 0)
            {
                aDrawPos[passes] = aDrawPos[0];
                aDrawPos[passes].m_X -= g_SceneMan.GetSceneWidth();
                passes++;
            }
            if (targetPos.m_X + pTargetBitmap->w > g_SceneMan.GetSceneWidth())
            {
                aDrawPos[passes] = aDrawPos[0];
                aDrawPos[passes].m_X += g_SceneMan.GetSceneWidth();
                passes++;
            }
        }
    }


	// Draw all the attached wound emitters, and only if the mode is g_DrawColor and not onlyphysical
	// Only draw attachables and emitters which are not drawn after parent, so we draw them before
	if (mode == g_DrawColor || (!onlyPhysical && mode == g_DrawMaterial))
	{
		for (list<AEmitter *>::const_iterator itr = m_Wounds.begin(); itr != m_Wounds.end(); ++itr)
		{
			if (!(*itr)->IsDrawnAfterParent())
				(*itr)->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
		}
	}

	// Draw all the attached attachables
	for (list<Attachable *>::const_iterator aItr = m_Attachables.begin(); aItr != m_Attachables.end(); ++aItr)
	{
		if (!(*aItr)->IsDrawnAfterParent())
			(*aItr)->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
	}


    //////////////////
    // FLIPPED
    if (m_HFlipped && pFlipBitmap)
    {
        // Don't size the intermediate bitmaps to the m_Scale, because the scaling happens after they are done
        clear_to_color(pFlipBitmap, keyColor);
        // Draw eitehr the source color bitmap or the intermediate material bitmap onto the intermediate flipping bitmap
        if (mode == g_DrawColor || mode == g_DrawTrans)
            draw_sprite_h_flip(pFlipBitmap, m_aSprite[m_Frame], 0, 0);
        // If using the temp bitmap (which is always larger than the sprite) make sure the flipped image ends up in the upper right corner as if it was just as small as the sprite bitmap
        else
            draw_sprite_h_flip(pFlipBitmap, pTempBitmap, -(pTempBitmap->w - m_aSprite[m_Frame]->w), 0);

        // Transparent mode
        if (mode == g_DrawTrans)
        {
            clear_to_color(pTempBitmap, keyColor);
            // Draw the rotated thing onto the intermediate bitmap so its COM position aligns with the middle of the temp bitmap.
            // The temp bitmap should be able to hold the full size since it is larger than the max diameter.
            // Take into account the h-flipped pivot point
            pivot_scaled_sprite(pTempBitmap,
                                pFlipBitmap,
                                pTempBitmap->w / 2,
                                pTempBitmap->h / 2,
                                pFlipBitmap->w + m_SpriteOffset.m_X,
                                -(m_SpriteOffset.m_Y),
                                ftofix(m_Rotation.GetAllegroAngle()),
                                ftofix(m_Scale));

            // Draw the now rotated object's temporary bitmap onto the final drawing bitmap with transperency
            // Do the passes loop in here so the intermediate drawing doesn't get done multiple times
            for (int i = 0; i < passes; ++i)
                draw_trans_sprite(pTargetBitmap, pTempBitmap, aDrawPos[i].GetFloorIntX() - (pTempBitmap->w / 2), aDrawPos[i].GetFloorIntY() - (pTempBitmap->h / 2));
        }
        // Non-transparent mode
        else
        {
            // Do the passes loop in here so the flipping operation doesn't get done multiple times
            for (int i = 0; i < passes; ++i)
            {
                // Take into account the h-flipped pivot point
                pivot_scaled_sprite(pTargetBitmap,
                                    pFlipBitmap,
                                    aDrawPos[i].GetFloorIntX(),
                                    aDrawPos[i].GetFloorIntY(),
                                    pFlipBitmap->w + m_SpriteOffset.m_X,
                                    -(m_SpriteOffset.m_Y),
                                    ftofix(m_Rotation.GetAllegroAngle()),
                                    ftofix(m_Scale));

                // Register potential MOID drawing
                if (mode == g_DrawMOID)
                    g_SceneMan.RegisterMOIDDrawing(aDrawPos[i].GetFloored(), m_MaxRadius + 2);
            }
        }
    }
    /////////////////
    // NON-FLIPPED
    else
    {
//        spritePos += m_SpriteOffset;
//        spritePos += (m_SpriteCenter * m_Rotation - m_SpriteCenter);

        // Transparent mode
        if (mode == g_DrawTrans)
        {
            clear_to_color(pTempBitmap, keyColor);
            // Draw the rotated thing onto the intermediate bitmap so its COM position aligns with the middle of the temp bitmap.
            // The temp bitmap should be able to hold the full size since it is larger than the max diameter.
            // Take into account the h-flipped pivot point
            pivot_scaled_sprite(pTempBitmap,
                                m_aSprite[m_Frame],
                                pTempBitmap->w / 2,
                                pTempBitmap->h / 2,
                                -(m_SpriteOffset.m_X),
                                -(m_SpriteOffset.m_Y),
                                ftofix(m_Rotation.GetAllegroAngle()),
                                ftofix(m_Scale));

            // Draw the now rotated object's temporary bitmap onto the final drawing bitmap with transperency
            // Do the passes loop in here so the intermediate drawing doesn't get done multiple times
            for (int i = 0; i < passes; ++i)
                draw_trans_sprite(pTargetBitmap, pTempBitmap, aDrawPos[i].GetFloorIntX() - (pTempBitmap->w / 2), aDrawPos[i].GetFloorIntY() - (pTempBitmap->h / 2));
        }
        // Non-transparent mode
        else
        {
            for (int i = 0; i < passes; ++i)
            {
                pivot_scaled_sprite(pTargetBitmap,
                                    mode == g_DrawColor ? m_aSprite[m_Frame] : pTempBitmap,
                                    aDrawPos[i].GetFloorIntX(),
                                    aDrawPos[i].GetFloorIntY(),
                                    -(m_SpriteOffset.m_X),
                                    -(m_SpriteOffset.m_Y),
                                    ftofix(m_Rotation.GetAllegroAngle()),
                                    ftofix(m_Scale));

                // Register potential MOID drawing
                if (mode == g_DrawMOID)
                    g_SceneMan.RegisterMOIDDrawing(aDrawPos[i].GetFloored(), m_MaxRadius + 2);
            }
        }
    }

    // Draw all the attached wound emitters, and only if the mode is g_DrawColor and not onlyphysical
    if (mode == g_DrawColor || (!onlyPhysical && mode == g_DrawMaterial))
    {
		for (list<AEmitter *>::const_iterator itr = m_Wounds.begin(); itr != m_Wounds.end(); ++itr)
		{
			if ((*itr)->IsDrawnAfterParent())
				(*itr)->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
		}
    }

    // Draw all the attached attachables
	for (list<Attachable *>::const_iterator aItr = m_Attachables.begin(); aItr != m_Attachables.end(); ++aItr)
	{
		if ((*aItr)->IsDrawnAfterParent())
			(*aItr)->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyAttachableForces
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does the joint force transfer stuff for an attachable. Convencinece
//                  method. If this returns false, it means the attachable has been knocked
//                  off and has been passed to MovableMan. In either case, if false is
//                  returned just set the pointer to 0 and be done with it.

bool MOSRotating::ApplyAttachableForces(Attachable *pAttachable, bool isCritical)
{
    bool intact = true;
    Vector forces, impulses;

    if (pAttachable)
    {
        // If the attahcable is set to be deleted (probably by it having been gibbed), do so
        // Note that is done regardless of whether it's attached anymore or not
        if (pAttachable->IsSetToDelete())
        {
            intact = false;
            RemoveAttachable(pAttachable);
            // If set to delete, then add to the movableman, and it'll delete it when it's safe to!
            g_MovableMan.AddParticle(pAttachable);
        }
        // If still attached, see what forces are being transferred to this, and if they attachable can withstand them.
        // If they rip the thing off, it will add itself to the Movableman, transferring ownership.
        else if (pAttachable->IsAttached())
        {
            pAttachable->PostTravel();
            intact = pAttachable->TransferJointForces(forces);
            intact = intact && pAttachable->TransferJointImpulses(impulses);

            if (!forces.IsZero())
                AddForce(forces, pAttachable->GetOnlyLinearForces() ? Vector() : pAttachable->GetParentOffset() * m_Rotation);
            if (!impulses.IsZero())
                AddImpulseForce(impulses, pAttachable->GetOnlyLinearForces() ? Vector() : pAttachable->GetParentOffset() * m_Rotation);
        }
        // If not attached, then we're not intact
        else
            intact = false;

        // If the attachable was ripped off or gibbed, handle it
        if (!intact)
        {
            // Get and attach the wound to this if the attachable has one and it was ripped off or gibbed
            if (pAttachable->GetBreakWound())
            {
                AEmitter *pWound = dynamic_cast<AEmitter *>(pAttachable->GetBreakWound()->Clone());
                if (pWound)
                {
                    pWound->SetEmitAngle((pAttachable->GetParentOffset() * m_Rotation).GetAbsRadAngle());
					if (isCritical && !IsInGroup("Brains"))
					{
						pWound->SetEmitCountLimit(1000000);
						pWound->SetEmitDamage(0.5);
						pWound->SetBurstDamage(35);
					}
					// IMPORTANT to pass false here so the added wound doesn't potentially gib this and cause the Attachables list to get f'd up while we're iterating through it in MOSRotating::Update!
					AddWound(pWound, pAttachable->GetParentOffset(), false);
					pWound = 0;
                }
            }
        }
    }
    else
        intact = false;

    return intact;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetStringValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the string value associated with the specified key or "" if it does not exist.

std::string MOSRotating::GetStringValue(std::string key)
{
	if (StringValueExists(key))
		return m_StringValueMap[key];
	else
		return "";
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetNumberValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the number value associated with the specified key or 0 if it does not exist.
// Arguments:       Key to retrieve value.
// Return value:    Number (double) value.

double MOSRotating::GetNumberValue(std::string key)
{
	if (NumberValueExists(key))
		return m_NumberValueMap[key];
	else
		return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetObjectValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the object value associated with the specified key or 0 if it does not exist.
// Arguments:       None.
// Return value:    Object (Entity *) value.

Entity * MOSRotating::GetObjectValue(std::string key)
{
	if (ObjectValueExists(key))
		return m_ObjectValueMap[key];
	else
		return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetStringValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the string value associated with the specified key.

void MOSRotating::SetStringValue(std::string key, std::string value)
{
	m_StringValueMap[key] = value;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetNumberValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the string value associated with the specified key.

void MOSRotating::SetNumberValue(std::string key, double value)
{
	m_NumberValueMap[key] = value;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetObjectValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the string value associated with the specified key.

void MOSRotating::SetObjectValue(std::string key, Entity * value)
{
	m_ObjectValueMap[key] = value;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RemoveStringValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Remove the string value associated with the specified key.

void MOSRotating::RemoveStringValue(std::string key)
{
	m_StringValueMap.erase(key);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RemoveNumberValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Remove the number value associated with the specified key.

void MOSRotating::RemoveNumberValue(std::string key)
{
	m_NumberValueMap.erase(key);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RemoveObjectValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Remove the object value associated with the specified key.

void MOSRotating::RemoveObjectValue(std::string key)
{
	m_ObjectValueMap.erase(key);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  StringValueExists
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether the value associated with the specified key exists.

bool MOSRotating::StringValueExists(std::string key)
{
	if (m_StringValueMap.find(key) != m_StringValueMap.end())
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  NumberValueExists
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether the value associated with the specified key exists.

bool MOSRotating::NumberValueExists(std::string key)
{
	if (m_NumberValueMap.find(key) != m_NumberValueMap.end())
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ObjectValueExists
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether the value associated with the specified key exists.

bool MOSRotating::ObjectValueExists(std::string key)
{
	if (m_ObjectValueMap.find(key) != m_ObjectValueMap.end())
		return true;
	return false;
}

} // namespace RTE