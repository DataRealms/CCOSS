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
#include "HDFirearm.h"

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
    m_ReferenceHardcodedAttachableUniqueIDs.clear();
    m_HardcodedAttachableUniqueIDsAndSetters.clear();
    m_RadiusAffectingAttachable = nullptr;
    m_FarthestAttachableDistanceAndRadius = 0.0F;
    m_AttachableAndWoundMass = 0.0F;
    m_Gibs.clear();
    m_GibImpulseLimit = 0;
    m_GibWoundLimit = 0;
    m_GibBlastStrength = 10.0F;
    m_GibSound = nullptr;
    m_EffectOnGib = true;
    m_pFlipBitmap = 0;
	m_pFlipBitmapS = 0;
	m_pTempBitmap = 0;
	m_pTempBitmapS = 0;
    m_LoudnessOnGib = 1;
	m_DamageMultiplier = 0;
    m_NoSetDamageMultiplier = true;
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

    m_SpriteCenter.SetXY(m_aSprite[m_Frame]->w / 2, m_aSprite[m_Frame]->h / 2);
    m_SpriteCenter += m_SpriteOffset;

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
    if (m_SpriteDiameter >= 256)
	{
        m_pTempBitmap = m_spTempBitmap512;
        m_pTempBitmapS = m_spTempBitmapS512;
	}
    else if (m_SpriteDiameter >= 128)
	{
        m_pTempBitmap = m_spTempBitmap256;
        m_pTempBitmapS = m_spTempBitmapS256;
	}
    else if (m_SpriteDiameter >= 64)
	{
        m_pTempBitmap = m_spTempBitmap128;
        m_pTempBitmapS = m_spTempBitmapS128;
	}
    else if (m_SpriteDiameter >= 32)
	{
        m_pTempBitmap = m_spTempBitmap64;
        m_pTempBitmapS = m_spTempBitmapS64;
	}
    else if (m_SpriteDiameter >= 16)
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

int MOSRotating::Create(const MOSRotating &reference) {
    MOSprite::Create(reference);

    if (!reference.m_pAtomGroup) {
        return -1;
    }

    // THESE ATOMGROUP COPYING ARE A TIME SINK!
    m_pAtomGroup = new AtomGroup();
    m_pAtomGroup->Create(*reference.m_pAtomGroup, true);
    if (m_pAtomGroup) { m_pAtomGroup->SetOwner(this); }

    if (reference.m_pDeepGroup) {
        m_pDeepGroup = dynamic_cast<AtomGroup *>(reference.m_pDeepGroup->Clone());
        if (m_pDeepGroup) { m_pDeepGroup->SetOwner(this); }
    }

    m_DeepCheck = reference.m_DeepCheck;
    m_SpriteCenter = reference.m_SpriteCenter;
    m_OrientToVel = reference.m_OrientToVel;

    m_Recoiled = reference.m_Recoiled;
    m_RecoilForce = reference.m_RecoilForce;
    m_RecoilOffset = reference.m_RecoilOffset;

    for (const AEmitter *wound : reference.m_Wounds) {
        AddWound(dynamic_cast<AEmitter *>(wound->Clone()), wound->GetParentOffset(), false);
    }

    //TODO This could probably be replaced with just using HardcodedAttachableUniqueIDs entirely. At this point in, these lists should have the same UIDs in them, so it should work.
    for (const Attachable *referenceAttachable : reference.m_Attachables) {
        if (m_ReferenceHardcodedAttachableUniqueIDs.find(referenceAttachable->GetUniqueID()) == m_ReferenceHardcodedAttachableUniqueIDs.end()) {
            AddAttachable(dynamic_cast<Attachable *>(referenceAttachable->Clone()));
        }
    }
    m_ReferenceHardcodedAttachableUniqueIDs.clear();

    for (const Gib &gib : reference.m_Gibs) {
        m_Gibs.push_back(gib);
    }

    m_StringValueMap = reference.m_StringValueMap;
    m_NumberValueMap = reference.m_NumberValueMap;
    m_ObjectValueMap = reference.m_ObjectValueMap;

    m_GibImpulseLimit = reference.m_GibImpulseLimit;
    m_GibWoundLimit = reference.m_GibWoundLimit;
    m_GibBlastStrength = reference.m_GibBlastStrength;
	if (reference.m_GibSound) { m_GibSound = dynamic_cast<SoundContainer*>(reference.m_GibSound->Clone()); }
	m_EffectOnGib = reference.m_EffectOnGib;
    m_LoudnessOnGib = reference.m_LoudnessOnGib;

	m_DamageMultiplier = reference.m_DamageMultiplier;
    m_NoSetDamageMultiplier = reference.m_NoSetDamageMultiplier;

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

int MOSRotating::ReadProperty(const std::string_view &propName, Reader &reader)
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
    else if (propName == "GibBlastStrength") {
        reader >> m_GibBlastStrength;
	} else if (propName == "GibSound") {
		if (!m_GibSound) { m_GibSound = new SoundContainer; }
		reader >> m_GibSound;
	} else if (propName == "EffectOnGib")
        reader >> m_EffectOnGib;
    else if (propName == "LoudnessOnGib")
        reader >> m_LoudnessOnGib;
	else if (propName == "DamageMultiplier") {
		reader >> m_DamageMultiplier;
        m_NoSetDamageMultiplier = false;
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MOSRotating::GetGibWoundLimit(bool includePositiveDamageAttachables, bool includeNegativeDamageAttachables, bool includeNoDamageAttachables) const {
    int gibWoundLimit = m_GibWoundLimit;
    if (includePositiveDamageAttachables || includeNegativeDamageAttachables || includeNoDamageAttachables) {
        for (const Attachable *attachable : m_Attachables) {
            bool attachableSatisfiesConditions = (includePositiveDamageAttachables && attachable->GetDamageMultiplier() > 0) ||
                (includeNegativeDamageAttachables && attachable->GetDamageMultiplier() < 0) ||
                (includeNoDamageAttachables && attachable->GetDamageMultiplier() == 0);

            if (attachableSatisfiesConditions) {
                gibWoundLimit += attachable->GetGibWoundLimit(includePositiveDamageAttachables, includeNegativeDamageAttachables, includeNoDamageAttachables);
            }
        }
    }
    return gibWoundLimit;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MOSRotating::GetWoundCount(bool includePositiveDamageAttachables, bool includeNegativeDamageAttachables, bool includeNoDamageAttachables) const {
    int woundCount = m_Wounds.size();
    if (includePositiveDamageAttachables || includeNegativeDamageAttachables || includeNoDamageAttachables) {
        for (const Attachable *attachable : m_Attachables) {
            bool attachableSatisfiesConditions = (includePositiveDamageAttachables && attachable->GetDamageMultiplier() > 0) ||
                (includeNegativeDamageAttachables && attachable->GetDamageMultiplier() < 0) ||
                (includeNoDamageAttachables && attachable->GetDamageMultiplier() == 0);

            if (attachableSatisfiesConditions) {
                woundCount += attachable->GetWoundCount(includePositiveDamageAttachables, includeNegativeDamageAttachables, includeNoDamageAttachables);
            }
        }
    }
    return woundCount;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MOSRotating::AddWound(AEmitter *woundToAdd, const Vector &parentOffsetToSet, bool checkGibWoundLimit) {
    if (woundToAdd) {
        if (checkGibWoundLimit && !ToDelete() && m_GibWoundLimit && m_Wounds.size() + 1 > m_GibWoundLimit) {
            // Indicate blast in opposite direction of emission
            // TODO: don't hardcode here, get some data from the emitter
            Vector blast(-5, 0);
            blast.RadRotate(woundToAdd->GetEmitAngle());
            GibThis(blast);
            return;
        } else {
            woundToAdd->SetCollidesWithTerrainWhileAttached(false);
            woundToAdd->SetParentOffset(parentOffsetToSet);
            woundToAdd->SetInheritsHFlipped(false);
            woundToAdd->SetParent(this);
            woundToAdd->SetIsWound(true);
            if (woundToAdd->HasNoSetDamageMultiplier()) { woundToAdd->SetDamageMultiplier(1.0F); }
            m_AttachableAndWoundMass += woundToAdd->GetMass();
            m_Wounds.push_back(woundToAdd);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float MOSRotating::RemoveWounds(int numberOfWoundsToRemove, bool includePositiveDamageAttachables, bool includeNegativeDamageAttachables, bool includeNoDamageAttachables) {
    float damage = 0;
    int woundCount = GetWoundCount(includePositiveDamageAttachables, includeNegativeDamageAttachables, includeNoDamageAttachables);

    std::vector<std::pair<MOSRotating *, std::size_t>> woundedParts;
    if (woundCount > 0) { woundedParts.push_back({this, woundCount}); }

    for (Attachable *attachable : m_Attachables) {
        bool attachableSatisfiesConditions = (includePositiveDamageAttachables && attachable->GetDamageMultiplier() > 0) ||
            (includeNegativeDamageAttachables && attachable->GetDamageMultiplier() < 0) ||
            (includeNoDamageAttachables && attachable->GetDamageMultiplier() == 0);
        int attachableWoundCount = attachable->GetWoundCount(includePositiveDamageAttachables, includeNegativeDamageAttachables, includeNoDamageAttachables);

        if (attachableSatisfiesConditions && attachableWoundCount > 0) { woundedParts.push_back({attachable, attachableWoundCount}); }
    }

    if (woundedParts.empty()) {
        return damage;
    }

    /// <summary>
    /// Internal lambda function to remove the first wound emitter from this MOSRotating.
    /// </summary>
    auto removeFirstWoundEmitter = [this]() {
        if (m_Wounds.empty()) {
            return 0.0F;
        }
        float woundDamage = m_Wounds.front()->GetBurstDamage();
        AEmitter *wound = m_Wounds.front();
        m_AttachableAndWoundMass -= wound->GetMass();
        m_Wounds.pop_front();
        delete wound;
        return woundDamage;
    };

    for (int i = 0; i < numberOfWoundsToRemove; i++) {
        if (woundedParts.empty()) {
            break;
        }

        int woundedPartIndex = RandomNum(0, static_cast<int>(woundedParts.size()) - 1);
        MOSRotating *woundedPart = woundedParts[woundedPartIndex].first;
        if (woundedPart == this) {
            damage += removeFirstWoundEmitter() * GetDamageMultiplier();
        } else {
            //TODO This is less efficient than it should be. We already collected all wounded parts and their wounds above, we should pass that in (make another function overload) instead of collecting everything again. It might be wise to use a tree for this purpose.
            damage += woundedPart->RemoveWounds(1, includePositiveDamageAttachables, includeNegativeDamageAttachables, includeNoDamageAttachables);
        }
        if (woundedParts[woundedPartIndex].second-- <= 0) { woundedParts.erase(woundedParts.begin() + woundedPartIndex); }
    }

	return damage;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

	delete m_GibSound;

    if (!notInherited)
        MOSprite::Destroy();
    Clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MOSRotating::SetAsNoID() {
    MovableObject::SetAsNoID();
    for (Attachable *attachable : m_Attachables) {
        attachable->SetAsNoID();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
        intPos[X] = std::floor(entryPos.m_X);
        intPos[Y] = std::floor(entryPos.m_Y);

        // Get the un-rotated direction and max possible
        // travel length of the particle.
        Vector dir(max(bounds[X], bounds[Y]), 0);
        dir.AbsRotateTo(hd.HitVel[HITOR] / m_Rotation);
        dir = dir.GetXFlipped(m_HFlipped);

        // Bresenham's line drawing algorithm preparation
        delta[X] = std::floor(entryPos.m_X + dir.m_X) - intPos[X];
        delta[Y] = std::floor(entryPos.m_Y + dir.m_Y) - intPos[Y];
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
            float damageMultiplier = pEntryWound->HasNoSetDamageMultiplier() ? 1.0F : pEntryWound->GetDamageMultiplier();
			pEntryWound->SetDamageMultiplier(damageMultiplier * hd.Body[HITOR]->WoundDamageMultiplier());
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
                float damageMultiplier = pExitWound->HasNoSetDamageMultiplier() ? 1.0F : pExitWound->GetDamageMultiplier();
				pExitWound->SetDamageMultiplier(damageMultiplier * hd.Body[HITOR]->WoundDamageMultiplier());
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MOSRotating::GibThis(const Vector &impactImpulse, MovableObject *movableObjectToIgnore) {
    if (m_MissionCritical || m_ToDelete) {
        return;
    }

    CreateGibsWhenGibbing(impactImpulse, movableObjectToIgnore);

    RemoveAttachablesWhenGibbing(impactImpulse, movableObjectToIgnore);

	if (m_GibSound) { m_GibSound->Play(m_Pos); }

    if (m_pScreenEffect && m_EffectOnGib && (m_EffectAlwaysShows || !g_SceneMan.ObscuredPoint(m_Pos.GetFloorIntX(), m_Pos.GetFloorIntY()))) {
		g_PostProcessMan.RegisterPostEffect(m_Pos, m_pScreenEffect, m_ScreenEffectHash, 255, m_EffectRotAngle);
    }

    if (m_LoudnessOnGib > 0) { g_MovableMan.RegisterAlarmEvent(AlarmEvent(m_Pos, m_Team, m_LoudnessOnGib)); }

    m_ToDelete = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MOSRotating::CreateGibsWhenGibbing(const Vector &impactImpulse, MovableObject *movableObjectToIgnore) {
    for (const Gib &gibSettingsObject : m_Gibs) {
        if (gibSettingsObject.GetCount() == 0) {
            continue;
        }
        MovableObject *gibParticleClone = dynamic_cast<MovableObject *>(gibSettingsObject.GetParticlePreset()->Clone());

		float mass = (gibParticleClone->GetMass() != 0 ? gibParticleClone->GetMass() : 0.0001F);
        float minVelocity = gibSettingsObject.GetMinVelocity();
        float velocityRange = gibSettingsObject.GetMaxVelocity() - gibSettingsObject.GetMinVelocity();
        if (gibSettingsObject.GetMinVelocity() == 0 && gibSettingsObject.GetMaxVelocity() == 0) {
            minVelocity = m_GibBlastStrength / mass;
            velocityRange = 10.0F;
        }
        Vector rotatedGibOffset = RotateOffset(gibSettingsObject.GetOffset());
        for (int i = 0; i < gibSettingsObject.GetCount(); i++) {
            gibParticleClone = dynamic_cast<MovableObject *>(gibSettingsObject.GetParticlePreset()->Clone());

            if (gibParticleClone->GetLifetime() != 0) {
                gibParticleClone->SetLifetime(static_cast<int>(static_cast<float>(gibParticleClone->GetLifetime()) * (1.0F + (gibSettingsObject.GetLifeVariation() * RandomNormalNum()))));
            }

            gibParticleClone->SetRotAngle(GetRotAngle() + gibParticleClone->GetRotAngle());
            gibParticleClone->SetAngularVel((gibParticleClone->GetAngularVel() * 0.35F) + (gibParticleClone->GetAngularVel() * 0.65F / mass) * RandomNum());
            if (rotatedGibOffset.GetRoundIntX() > m_aSprite[0]->w / 3) {
                float offCenterRatio = rotatedGibOffset.m_X / (static_cast<float>(m_aSprite[0]->w) / 2.0F);
                float angularVel = fabs(gibParticleClone->GetAngularVel() * 0.5F) + std::fabs(gibParticleClone->GetAngularVel() * 0.5F * offCenterRatio);
                gibParticleClone->SetAngularVel(angularVel * (rotatedGibOffset.m_X > 0 ? -1 : 1));
            } else {
                gibParticleClone->SetAngularVel((gibParticleClone->GetAngularVel() * 0.5F + (gibParticleClone->GetAngularVel() * RandomNum())) * (RandomNormalNum() > 0.0F ? 1.0F : -1.0F));
            }

            gibParticleClone->SetPos(m_Pos + rotatedGibOffset);
			gibParticleClone->SetHFlipped(m_HFlipped);
            Vector gibVelocity = rotatedGibOffset.IsZero() ? Vector(minVelocity + RandomNum(0.0F, velocityRange), 0.0F) : rotatedGibOffset.SetMagnitude(minVelocity + RandomNum(0.0F, velocityRange));
			// TODO: Figure out how much the magnitude of an offset should affect spread
			float gibSpread = (rotatedGibOffset.IsZero() && gibSettingsObject.GetSpread() == 0.1F) ? c_PI : gibSettingsObject.GetSpread();
            gibVelocity.RadRotate(impactImpulse.GetAbsRadAngle() + (gibSpread * RandomNormalNum()));
            gibParticleClone->SetVel(gibVelocity + (gibSettingsObject.InheritsVelocity() ? m_Vel : Vector()));
            if (movableObjectToIgnore) { gibParticleClone->SetWhichMOToNotHit(movableObjectToIgnore); }
			gibParticleClone->SetTeam(m_Team);
			gibParticleClone->SetIgnoresTeamHits(gibSettingsObject.IgnoresTeamHits());

            g_MovableMan.AddParticle(gibParticleClone);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MOSRotating::RemoveAttachablesWhenGibbing(const Vector &impactImpulse, MovableObject *movableObjectToIgnore) {
    Attachable *attachable;
    for (std::list<Attachable *>::iterator attachableIterator = m_Attachables.begin(); attachableIterator != m_Attachables.end();) {
        RTEAssert((*attachableIterator), "Broken Attachable!");
        attachable = *attachableIterator;

        if (RandomNum() < attachable->GetGibWithParentChance()) {
            ++attachableIterator;
            attachable->GibThis();
            continue;
        }

        if (!attachable->GetDeleteWhenRemovedFromParent()) {
            float attachableGibBlastStrength = (attachable->GetParentGibBlastStrengthMultiplier() * m_GibBlastStrength) / (1 + attachable->GetMass());
            attachable->SetAngularVel((attachable->GetAngularVel() * 0.5F) + (attachable->GetAngularVel() * 0.5F * attachableGibBlastStrength * RandomNormalNum()));
            Vector gibBlastVel = Vector(attachable->GetParentOffset()).SetMagnitude(attachableGibBlastStrength * 0.5F + (attachableGibBlastStrength * RandomNum()));
            attachable->SetVel(m_Vel + gibBlastVel + impactImpulse);

            if (movableObjectToIgnore) { attachable->SetWhichMOToNotHit(movableObjectToIgnore); }
        }

        ++attachableIterator;
        RemoveAttachable(attachable, true, true);
    }
    m_Attachables.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  MoveOutOfTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether any of the Atom:s in this MovableObject are on top of
//                  terrain pixels, and if so, attempt to move this out so none of this'
//                  Atoms are on top of the terrain any more.

bool MOSRotating::MoveOutOfTerrain(unsigned char strongerThan)
{
    return m_pAtomGroup->ResolveTerrainIntersection(m_Pos, strongerThan);
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

    // If we seem to be about to settle, make sure we're not flying in the air still.
    // Note that this uses sprite radius to avoid possibly settling when it shouldn't (e.g. if there's a lopsided attachable enlarging the radius, using GetRadius might make it settle in the air).
    if (m_ToSettle || IsAtRest())
    {
        if (g_SceneMan.OverAltitude(m_Pos, m_SpriteRadius + 4, 3))
        {
            m_RestTimer.Reset();
            m_ToSettle = false;
        }
    }

    m_PrevRotation = m_Rotation;
    m_PrevAngVel = m_AngularVel;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MOSRotating::IsOnScenePoint(Vector &scenePoint) const {
    if (!m_aSprite[m_Frame]) {
        return false;
    }

    //TODO this should really use GetRadius() instead of sprite radius, then check attachable's sprites directly here. It'd save some computation but I didn't wanna deal with it.
    if (WithinBox(scenePoint, m_Pos.m_X - m_SpriteRadius, m_Pos.m_Y - m_SpriteRadius, m_Pos.m_X + m_SpriteRadius, m_Pos.m_Y + m_SpriteRadius)) {
        Vector spritePoint = scenePoint - m_Pos;
        spritePoint = UnRotateOffset(spritePoint);
        int pixel = getpixel(m_aSprite[m_Frame], static_cast<int>(spritePoint.m_X - m_SpriteOffset.m_X), static_cast<int>(spritePoint.m_Y - m_SpriteOffset.m_Y));
        if (pixel != -1 && pixel != g_MaskColor) {
            return true;
        }
    }

    for (const Attachable *attachable : m_Attachables) {
        if (attachable->IsOnScenePoint(scenePoint)) {
            return true;
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

void MOSRotating::PreTravel() {
	MOSprite::PreTravel();

	// If this is going slow enough, check for and redraw the MOID representations of any other MOSRotatings that may be overlapping this
	if (m_GetsHitByMOs && m_HitsMOs && m_Vel.GetX() < 2.0F && m_Vel.GetY() < 2.0F) { g_MovableMan.RedrawOverlappingMOIDs(this); }
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

    // Set the atom to ignore a certain MO, if set and applicable.
    if (m_HitsMOs && m_pMOToNotHit && g_MovableMan.ValidMO(m_pMOToNotHit) && !m_MOIgnoreTimer.IsPastSimTimeLimit()) {
        std::vector<MOID> MOIDsNotToHit;
        m_pMOToNotHit->GetMOIDs(MOIDsNotToHit);
        for (const MOID &MOIDNotToHit : MOIDsNotToHit) {
            m_pAtomGroup->AddMOIDToIgnore(MOIDNotToHit);
        }
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


    Attachable *attachable;
    for (std::list<Attachable *>::iterator attachableIterator = m_Attachables.begin(); attachableIterator != m_Attachables.end(); ) {
        attachable = *attachableIterator;
        RTEAssert(attachable, "Broken Attachable in PostTravel!");
        ++attachableIterator;
        attachable->PostTravel();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MOSRotating. Supposed to be done every frame.

void MOSRotating::Update() {
#ifndef RELEASE_BUILD
	RTEAssert(m_MOID == g_NoMOID || (m_MOID >= 0 && m_MOID < g_MovableMan.GetMOIDCount()), "MOID out of bounds!");
#endif

    MOSprite::Update();

    if (m_InheritEffectRotAngle) { m_EffectRotAngle = m_Rotation.GetRadAngle(); }

    if (m_OrientToVel > 0 && m_Vel.GetLargest() > 5.0F) {
        m_OrientToVel = std::clamp(m_OrientToVel, 0.0F, 1.0F);

        float velInfluence = std::clamp(m_OrientToVel < 1.0F ? m_Vel.GetMagnitude() / 100.0F : 1.0F, 0.0F, 1.0F);
        float radsToGo = m_Rotation.GetRadAngleTo(m_Vel.GetAbsRadAngle() + (m_HFlipped ? -c_PI : 0));
        m_Rotation += radsToGo * m_OrientToVel * velInfluence;
    }

    AEmitter *wound = nullptr;
    for (std::list<AEmitter *>::iterator woundIterator = m_Wounds.begin(); woundIterator != m_Wounds.end(); ) {
        wound = *woundIterator;
        RTEAssert(wound && wound->IsAttachedTo(this), "Broken wound AEmitter in Update");
        ++woundIterator;
        wound->Update();

        if (wound->IsSetToDelete() || (wound->GetLifetime() > 0 && wound->GetAge() > wound->GetLifetime())) {
            m_Wounds.remove(wound);
            m_AttachableAndWoundMass -= wound->GetMass();
            delete wound;
        } else {
            Vector totalImpulseForce;
            for (const std::pair<Vector, Vector> &impulseForce : wound->GetImpulses()) {
                totalImpulseForce += impulseForce.first;
            }
            totalImpulseForce *= wound->GetJointStiffness();

            if (!totalImpulseForce.IsZero()) { AddImpulseForce(totalImpulseForce, wound->GetApplyTransferredForcesAtOffset() ? wound->GetParentOffset() * m_Rotation * c_MPP : Vector()); }

            wound->ClearImpulseForces();
        }
    }

    Attachable *attachable = nullptr;
    for (std::list<Attachable *>::iterator attachableIterator = m_Attachables.begin(); attachableIterator != m_Attachables.end(); ) {
        attachable = *attachableIterator;
        RTEAssert(attachable, "Broken Attachable in Update!");
        RTEAssert(attachable->IsAttached(), "Found Attachable on " + GetModuleAndPresetName() + " (" + attachable->GetModuleAndPresetName() + ") with no parent, this should never happen!");
        RTEAssert(attachable->IsAttachedTo(this), "Found Attachable on " + GetModuleAndPresetName() + " (" + attachable->GetModuleAndPresetName() + ") with another parent (" + attachable->GetParent()->GetModuleAndPresetName() + "), this should never happen!");
        ++attachableIterator;

        attachable->Update();
        RTEAssert(attachable, "Broken Attachable after Updating it!");

        if (attachable->IsAttachedTo(this) && attachable->IsSetToDelete()) {
            RemoveAttachable(attachable, true, true);
        } else if (attachable->IsAttachedTo(this) && !attachable->IsSetToDelete()) {
            TransferForcesFromAttachable(attachable);
        }
    }

    if (m_HFlipped && !m_pFlipBitmap && m_aSprite[0]) { m_pFlipBitmap = create_bitmap_ex(8, m_aSprite[0]->w, m_aSprite[0]->h); }
    if (m_HFlipped && !m_pFlipBitmapS && m_aSprite[0]) { m_pFlipBitmapS = create_bitmap_ex(c_MOIDLayerBitDepth, m_aSprite[0]->w, m_aSprite[0]->h); }
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
        float combinedRadii = GetRadius() + pOverlapMO->GetRadius();
        Vector otherPos = pOverlapMO->GetPos();

        // Quick check
        if (fabs(otherPos.m_X - m_Pos.m_X) > combinedRadii || fabs(otherPos.m_Y - m_Pos.m_Y) > combinedRadii)
            return false;

        // Check if the offset is within the combined radii of the two object, and therefore might be overlapping
        if (g_SceneMan.ShortestDistance(m_Pos, otherPos, g_SceneMan.SceneWrapsX()).GetMagnitude() < combinedRadii)
        {
            // They may be overlapping, so draw the MOID rep of this to the MOID layer
            Draw(g_SceneMan.GetMOIDBitmap(), Vector(), g_DrawMOID, true);
            return true;
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//TODO This should just be defined in MOSR instead of having an empty definition in MO. MOSR would need to override UpdateMOID accordingly, but this would clean things up a little.
void MOSRotating::UpdateChildMOIDs(vector<MovableObject *> &MOIDIndex, MOID rootMOID, bool makeNewMOID) {
    MOSprite::UpdateChildMOIDs(MOIDIndex, m_RootMOID, makeNewMOID);

    for (Attachable *attachable : m_Attachables) {
        // Anything that doesn't get hit by MOs doesn't need an ID, since that's only actually used for collision stuff.
        if (attachable->GetsHitByMOs()) { attachable->UpdateMOID(MOIDIndex, m_RootMOID, makeNewMOID); }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MOSRotating::AddAttachable(Attachable *attachable) {
	if (attachable) { AddAttachable(attachable, attachable->GetParentOffset()); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MOSRotating::AddAttachable(Attachable *attachable, const Vector& parentOffsetToSet) {
	if (attachable) {
        RTEAssert(!attachable->IsAttached(), "Tried to add Attachable " + attachable->GetModuleAndPresetName() + " but it already has a parent, " + (attachable->IsAttached() ? attachable->GetParent()->GetModuleAndPresetName() : "ERROR") + ".");
        if (g_MovableMan.ValidMO(attachable)) { g_MovableMan.RemoveMO(attachable); }
        attachable->SetParentOffset(parentOffsetToSet);
        attachable->SetParent(this);
        m_AttachableAndWoundMass += attachable->GetMass();
        HandlePotentialRadiusAffectingAttachable(attachable);
        m_Attachables.push_back(attachable);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MOSRotating::RemoveAttachable(long attachableUniqueID, bool addToMovableMan, bool addBreakWounds) {
    MovableObject *attachableAsMovableObject = g_MovableMan.FindObjectByUniqueID(attachableUniqueID);
    if (attachableAsMovableObject) {
        return RemoveAttachable(dynamic_cast<Attachable *>(attachableAsMovableObject), addToMovableMan, addBreakWounds);
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool MOSRotating::RemoveAttachable(Attachable *attachable, bool addToMovableMan, bool addBreakWounds) {
    if (!attachable || !attachable->IsAttached()) {
        return false;
    }
    RTEAssert(attachable->IsAttachedTo(this), "Tried to remove Attachable " + attachable->GetPresetNameAndUniqueID() + " from presumed parent " + GetPresetNameAndUniqueID() + ", but it had a different parent (" + (attachable->GetParent() ? attachable->GetParent()->GetPresetNameAndUniqueID() : "ERROR") + "). This should never happen!");

    if (!m_Attachables.empty()) { m_Attachables.remove(attachable); }
    attachable->SetParent(nullptr);
    m_AttachableAndWoundMass -= attachable->GetMass();

    std::unordered_map<unsigned long, std::function<void(MOSRotating *, Attachable *)>>::iterator hardcodedAttachableMapEntry = m_HardcodedAttachableUniqueIDsAndSetters.find(attachable->GetUniqueID());
    if (hardcodedAttachableMapEntry != m_HardcodedAttachableUniqueIDsAndSetters.end()) {
        hardcodedAttachableMapEntry->second(this, nullptr);
        m_HardcodedAttachableUniqueIDsAndSetters.erase(hardcodedAttachableMapEntry);
    }
    
    if (addBreakWounds) {
        if (!m_ToDelete && attachable->GetParentBreakWound()) {
            AEmitter *parentBreakWound = dynamic_cast<AEmitter *>(attachable->GetParentBreakWound()->Clone());
            if (parentBreakWound) {
                parentBreakWound->SetEmitAngle((attachable->GetParentOffset() * m_Rotation).GetAbsRadAngle());
                AddWound(parentBreakWound, attachable->GetParentOffset(), false);
                parentBreakWound = nullptr;
            }
        }
        if (!attachable->IsSetToDelete() && attachable->GetBreakWound()) {
            AEmitter *childBreakWound = dynamic_cast<AEmitter *>(attachable->GetBreakWound()->Clone());
            if (childBreakWound) {
                childBreakWound->SetEmitAngle(attachable->GetJointOffset().GetAbsRadAngle());
                attachable->AddWound(childBreakWound, attachable->GetJointOffset());
                childBreakWound = nullptr;
            }
        }
    }
    if (attachable->GetDeleteWhenRemovedFromParent()) { attachable->SetToDelete(); }
    if (addToMovableMan || attachable->IsSetToDelete()) { g_MovableMan.AddMO(attachable); }

    if (attachable == m_RadiusAffectingAttachable) {
        m_RadiusAffectingAttachable = nullptr;
        m_FarthestAttachableDistanceAndRadius = 0;
        std::for_each(m_Attachables.begin(), m_Attachables.end(), [this](const Attachable *attachableToCheck) { HandlePotentialRadiusAffectingAttachable(attachableToCheck); });

        Attachable *thisAsAttachable = dynamic_cast<Attachable *>(this);
        if (thisAsAttachable && m_Attachables.empty() && thisAsAttachable->IsAttached()) {
            thisAsAttachable->m_Parent->HandlePotentialRadiusAffectingAttachable(thisAsAttachable);
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MOSRotating::RemoveOrDestroyAllAttachables(bool destroy) {
    Attachable *attachable;
    for (std::list<Attachable *>::iterator attachableIterator = m_Attachables.begin(); attachableIterator != m_Attachables.end(); ) {
        attachable = *attachableIterator;
        RTEAssert(attachable, "Broken Attachable!");
        ++attachableIterator;

        if (destroy) {
            delete attachable;
        } else {
            RemoveAttachable(attachable);
        }
    }
	m_Attachables.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MOSRotating::GetMOIDs(std::vector<MOID> &MOIDs) const {
    MOSprite::GetMOIDs(MOIDs);
    for (const Attachable *attachable : m_Attachables) {
        if (attachable->GetsHitByMOs()) { attachable->GetMOIDs(MOIDs); }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MOSRotating::SetWhichMOToNotHit(MovableObject *moToNotHit, float forHowLong) {
    MOSprite::SetWhichMOToNotHit(moToNotHit, forHowLong);
    for (Attachable *attachable : m_Attachables) { attachable->SetWhichMOToNotHit(moToNotHit, forHowLong); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

    // Draw all the attached wound emitters, and only if the mode is g_DrawColor and not onlyphysical
    // Only draw attachables and emitters which are not drawn after parent, so we draw them before
    if (mode == g_DrawColor || (!onlyPhysical && mode == g_DrawMaterial)) {
        for (const AEmitter *woundToDraw : m_Wounds) {
            if (!woundToDraw->IsDrawnAfterParent()) { woundToDraw->Draw(pTargetBitmap, targetPos, mode, onlyPhysical); }
        }
    }

    // Draw all the attached attachables
    for (const Attachable *attachableToDraw : m_Attachables) {
        if (!attachableToDraw->IsDrawnAfterParent() && attachableToDraw->IsDrawnNormallyByParent()) { attachableToDraw->Draw(pTargetBitmap, targetPos, mode, onlyPhysical); }
    }

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
        // Draw the requested material silhouette on the material bitmap
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
		else if (mode == g_DrawDoor)
			draw_character_ex(pTempBitmap, m_aSprite[m_Frame], 0, 0, g_MaterialDoor, -1);
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
            if (spritePos.m_X < m_SpriteDiameter)
            {
                aDrawPos[passes] = spritePos;
                aDrawPos[passes].m_X += pTargetBitmap->w;
                passes++;
            }
            else if (spritePos.m_X > pTargetBitmap->w - m_SpriteDiameter)
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
                    g_SceneMan.RegisterMOIDDrawing(aDrawPos[i].GetFloored(), m_SpriteRadius + 2);
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
                    g_SceneMan.RegisterMOIDDrawing(aDrawPos[i].GetFloored(), m_SpriteRadius + 2);
            }
        }
    }

    // Draw all the attached wound emitters, and only if the mode is g_DrawColor and not onlyphysical
    // Only draw attachables and emitters which are not drawn after parent, so we draw them before
    if (mode == g_DrawColor || (!onlyPhysical && mode == g_DrawMaterial)) {
        for (const AEmitter *woundToDraw : m_Wounds) {
            if (woundToDraw->IsDrawnAfterParent()) { woundToDraw->Draw(pTargetBitmap, targetPos, mode, onlyPhysical); }
        }
    }

    // Draw all the attached attachables
    for (const Attachable *attachableToDraw : m_Attachables) {
        if (attachableToDraw->IsDrawnAfterParent() && attachableToDraw->IsDrawnNormallyByParent()) { attachableToDraw->Draw(pTargetBitmap, targetPos, mode, onlyPhysical); }
    }

    if (mode == g_DrawColor && !onlyPhysical && m_pAtomGroup && g_SettingsMan.DrawAtomGroupVisualizations() && GetRootParent() == this) {
        m_pAtomGroup->Draw(pTargetBitmap, targetPos, false, 122);
        //m_pDeepGroup->Draw(pTargetBitmap, targetPos, false, 13);
    }
}

bool MOSRotating::HandlePotentialRadiusAffectingAttachable(const Attachable *attachable) {
    if (!attachable->IsAttachedTo(this) && !attachable->IsWound()) {
        return false;
    }
    const HDFirearm *thisAsFirearm = dynamic_cast<HDFirearm *>(this);
    const AEmitter *thisAsEmitter = dynamic_cast<AEmitter *>(this);
    if ((thisAsFirearm && attachable == thisAsFirearm->GetFlash()) || (thisAsEmitter && attachable == thisAsEmitter->GetFlash())) {
        return false;
    }
    float distanceAndRadiusFromParent = g_SceneMan.ShortestDistance(m_Pos, attachable->m_Pos, g_SceneMan.SceneWrapsX()).GetMagnitude() + attachable->GetRadius();
    if (attachable == m_RadiusAffectingAttachable && distanceAndRadiusFromParent < m_FarthestAttachableDistanceAndRadius) {
        m_FarthestAttachableDistanceAndRadius = distanceAndRadiusFromParent;
        if (m_Attachables.size() > 1) {
            std::for_each(m_Attachables.begin(), m_Attachables.end(), [this](Attachable *attachableToCheck) { attachableToCheck->UpdatePositionAndJointPositionBasedOnOffsets(); HandlePotentialRadiusAffectingAttachable(attachableToCheck); });
        }
        return true;
    } else if (distanceAndRadiusFromParent > m_FarthestAttachableDistanceAndRadius) {
        m_FarthestAttachableDistanceAndRadius = distanceAndRadiusFromParent;
        m_RadiusAffectingAttachable = attachable;
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MOSRotating::TransferForcesFromAttachable(Attachable *attachable) {
    bool intact = false;
    Vector forces;
    Vector impulses;
    intact = attachable->TransferJointForces(forces) && attachable->TransferJointImpulses(impulses);

    if (!forces.IsZero()) { AddForce(forces, attachable->GetApplyTransferredForcesAtOffset() ? attachable->GetParentOffset() * m_Rotation * c_MPP : Vector()); }
    if (!impulses.IsZero()) { AddImpulseForce(impulses, attachable->GetApplyTransferredForcesAtOffset() ? attachable->GetParentOffset() * m_Rotation * c_MPP : Vector()); }
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
