//////////////////////////////////////////////////////////////////////////////////////////
// File:            Atom.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Atom class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Atom.h"
#include "SLTerrain.h"
#include "MovableObject.h"
#include "MOSRotating.h"
#include "PresetMan.h"
#include "RTETools.h"
#include "Actor.h"

namespace RTE {

const string Atom::ClassName = "Atom";
std::vector<void *> Atom::m_AllocatedPool;
int Atom::m_PoolAllocBlockCount = 200;
int Atom::m_InstancesInUse = 0;

// This forms a circle around the Atom's offset center, to check for key color pixels in order to determine the normal at the Atom's position
//const Vector Atom::m_sNormalChecks[NormalCheckCount] = { Vector(0, -3), Vector(1, -3), Vector(2, -2), Vector(3, -1), Vector(3, 0), Vector(3, 1), Vector(2, 2), Vector(1, 3), Vector(0, 3), Vector(-1, 3), Vector(-2, 2), Vector(-3, 1), Vector(-3, 0), Vector(-3, -1), Vector(-2, -2), Vector(-1, -3) };
const int Atom::m_sNormalChecks[NormalCheckCount][2] = { {0, -3}, {1, -3}, {2, -2}, {3, -1}, {3, 0}, {3, 1}, {2, 2}, {1, 3}, {0, 3}, {-1, 3}, {-2, 2}, {-3, 1}, {-3, 0}, {-3, -1}, {-2, -2}, {-1, -3} };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this HitData, effectively
//                  resetting the members of this abstraction level only.

void HitData::Clear()
{
    hitPoint.Reset();
    velDiff.Reset();
    bitmapNormal.Reset();
    hitDenominator = 0;

    for (int i = 0; i < 2; ++i) {
        pBody[i] = 0;
        pRootBody[i] = 0;
        hitVel[i].Reset();
        mass[i] = 0;
        momInertia[i] = 0;
        hitRadius[i].Reset();
		hitMaterial[i] = 0;// Material();
        preImpulse[i].Reset();
        resImpulse[i].Reset();
        impFactor[i] = 0;
        squaredMIHandle[i] = 0;
        terminate[i] = false;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        HitData assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one HitData equal to another.

HitData & HitData::operator=(const HitData &rhs)
{
    if (this == &rhs)
        return *this;

    Clear();

    hitPoint = rhs.hitPoint;
    velDiff = rhs.velDiff;
    bitmapNormal = rhs.bitmapNormal;
    hitDenominator = rhs.hitDenominator;

    for (int i = 0; i < 2; ++i) {
        pBody[i] = rhs.pBody[i];
        pRootBody[i] = rhs.pRootBody[i];
        hitVel[i] = rhs.hitVel[i];
        mass[i] = rhs.mass[i];
        momInertia[i] = rhs.momInertia[i];
        hitRadius[i] = rhs.hitRadius[i];
        hitMaterial[i] = rhs.hitMaterial[i];
        preImpulse[i] = rhs.preImpulse[i];
        resImpulse[i] = rhs.resImpulse[i];
        impFactor[i] = rhs.impFactor[i];
        squaredMIHandle[i] = rhs.squaredMIHandle[i];
        terminate[i] = rhs.terminate[i];
    }

    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FillPool
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a certain number of newly allocated instances to this' pool.

void Atom::FillPool(int fillAmount)
{
    // Default to the set block allocation size if fillAmount is 0
    if (fillAmount <= 0)
        fillAmount = m_PoolAllocBlockCount;

    // If concrete class, fill up the pool with pre-allocated memory blocks the size of the type
    if (fillAmount > 0)
    {
        // As many as we're asked to make
        for (int i = 0; i < fillAmount; ++i)
            m_AllocatedPool.push_back(malloc(sizeof(Atom)));
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Static method:  GetPoolMemory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Grabs from the pre-allocated pool, an available chunk of memory the
//                  exact size of the Entity this ClassInfo represents.

void * Atom::GetPoolMemory()
{
    // If the pool is empty, then fill it up again with as many instances as we are set to
    if (m_AllocatedPool.empty())
        FillPool(m_PoolAllocBlockCount > 0 ? m_PoolAllocBlockCount : 10);

    // Get the instance in the top of the pool and pop it off
    void *pFoundMemory = m_AllocatedPool.back();
    m_AllocatedPool.pop_back();

    RTEAssert(pFoundMemory, "Could not find an available instance in the pool, even after increasing its size!");

    // Keep track of the number of instaces passed out
    m_InstancesInUse++;

    return pFoundMemory;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Static method:  ReturnPoolMemory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a raw chunk of memory back to the pre-allocated available pool.

int Atom::ReturnPoolMemory(void *pReturnedMemory)
{
    if (!pReturnedMemory)
        return false;

    m_AllocatedPool.push_back(pReturnedMemory);

    // Keep track of the number of instaces passed in
    m_InstancesInUse--;

    return m_InstancesInUse;
}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Atom, effectively
//                  resetting the members of this abstraction level only.

void Atom::Clear()
{
    m_Offset.Reset();
    m_OriginalOffset.Reset();
    m_Normal.Reset();
	m_pMaterial = g_SceneMan.GetMaterialFromID(g_MaterialAir);
    m_SubgroupID = 0;
    m_MOHitsDisabled = false;
    m_TerrainHitsDisabled = false;
    m_pOwnerMO = 0;
    m_IgnoreMOID = g_NoMOID;
    m_IgnoreMOIDs.clear();
    m_MOIDHit = g_NoMOID;
    m_TerrainMatHit = g_MaterialAir;

    m_LastHit.Reset();
/*
    m_HitVel.Reset();
    m_HitRadius.Reset();
    m_HitImpulse.Reset();
*/
    m_TrailColor.Reset();
    m_TrailLength = 0;
    m_NumPenetrations = 0;
    m_ChangedDir = true;
    m_ResultWrapped = false;
    m_PrevError = 0;
    m_StepRatio = 1.0;
    m_SegProgress = 0.0;

	m_pIgnoreMOIDsByGroup = 0;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Atom object ready for use.

int Atom::Create()
{
    // Read all the properties
    if (Serializable::Create() < 0)
        return -1;

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Atom object ready for use.

int Atom::Create(const Vector &offset,
                 Material const *material,
                 MovableObject *owner,
                 Color trailColor,
                 int trailLength)
{
    m_Offset = m_OriginalOffset = offset;
    // Use the offset as normal for now
    m_Normal = m_Offset;
    m_Normal.Normalize();
    m_pMaterial = material;
    m_pOwnerMO = owner;
    m_TrailColor = trailColor;
    m_TrailLength = trailLength;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Atom to be identical to another, by deep copy.

int Atom::Create(const Atom &reference)
{
//    Entity::Create(reference);

    m_Offset = reference.m_Offset;
    m_OriginalOffset = reference.m_OriginalOffset;
    m_Normal = reference.m_Normal;
	m_pMaterial = reference.m_pMaterial;
    m_SubgroupID = reference.m_SubgroupID;
    // Needs to be set manually by the new MO owner.
    m_pOwnerMO = 0;
	// Needs to be set manually by the new AtomGroup owner.
	m_pIgnoreMOIDsByGroup = 0;
	m_TrailColor = reference.m_TrailColor;
    m_TrailLength = reference.m_TrailLength;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Atom::ReadProperty(std::string propName, Reader &reader)
{
// TODO: this right?
    if (propName == "Offset")
        reader >> m_Offset;
    else if (propName == "OriginalOffset")
        reader >> m_OriginalOffset;
	else if (propName == "Material")
	{
		Material mat;
		mat.Reset();
		reader >> mat;
		m_pMaterial = g_SceneMan.AddMaterialCopy(&mat);
		if (!m_pMaterial)
		{
			RTEAbort("Failed to store material \"" + mat.GetPresetName() + "\" Aborting!");
		}

		/*if (mat.id)
			m_pMaterial = g_SceneMan.GetMaterialFromID(mat.id);
		else
			m_pMaterial = g_SceneMan.GetMaterial(mat.GetPresetName());

		if (!m_pMaterial)
		{
			g_ConsoleMan.PrintString("ERROR: Can't find material by ID or PresetName while processing \"" + mat.GetPresetName() + "\". Was it defined with AddMaterial?");
			m_pMaterial = g_SceneMan.GetMaterialFromID(g_MaterialAir);
			// Crash if could not fall back to g_MaterialAir. Will crash due to null-pointer somewhere anyway
			if (!m_pMaterial)
			{
				RTEAbort("Failed to find a matching material \"" + mat.GetPresetName() + "\" or even fall back to g_MaterialAir. Aborting!");
			}
		}*/
	}
    else if (propName == "TrailColor")
        reader >> m_TrailColor;
    else if (propName == "TrailLength")
        reader >> m_TrailLength;
    else
        // See if the base class(es) can find a match instead
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Atom with a Writer for
//                  later recreation with Create(Reader &reader);

int Atom::Save(Writer &writer) const
{
    Serializable::Save(writer);

    writer.NewProperty("Offset");
    writer << m_Offset;
    writer.NewProperty("OriginalOffset");
    writer << m_OriginalOffset;
    writer.NewProperty("Material");
    writer << m_pMaterial;
    writer.NewProperty("TrailColor");
    writer << m_TrailColor;
    writer.NewProperty("TrailLength");
    writer << m_TrailLength;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Atom object.

void Atom::Destroy(bool notInherited)
{
    Clear();
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Atom
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate an Atom object
//                  identical to an already existing one.

Atom::Atom(const Atom &reference):
    m_Offset(reference.GetOffset()),
    m_Elasticity(reference.GetElasticity()),
    m_Friction(reference.GetFriction()),
    m_ResultPos(0, 0),
    m_RestTrajectory(0, 0),
    m_ResultAcc(0, 0),
    m_TrailColor(reference.GetDrawTrail())
{
    
}
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Atom assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one Atom equal to another.

Atom & Atom::operator=(const Atom &rhs)
{
    if (*this == rhs)
        return *this;

    m_Offset = rhs.m_Offset;
    m_pMaterial = rhs.m_pMaterial;

    return *this;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateNormal
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets up the normal of this atom, based on its position on a sprite's
//                  bitmap. It will check piels around it and see if they are inside the
//                  object or not, and infer a collision normal based on that.

bool Atom::CalculateNormal(BITMAP *pSprite, Vector spriteCenter)
{
    RTEAssert(pSprite, "Trying to set up Atom normal without passing in bitmap");
//    RTEAssert(m_pOwnerMO, "Trying to set up Atom normal without a parent MO!");
    
    // Can't set up a normal on an atom that doesn't have an offset from its parent's center
    if (m_Offset.IsZero())
    {
        m_Normal.Reset();
        return false;
    }

    // See if the atom even ends up in the sprite at all
    Vector atomPos = spriteCenter + m_Offset;
    if (atomPos.m_X < 0 || atomPos.m_Y < 0 || atomPos.m_X >= pSprite->w || atomPos.m_Y >= pSprite->h)
        return false;

    // Go through all the check positions from the atom's position on the sprite
    m_Normal.Reset();
    int checkPixel = 0;
    for (int check = 0; check < NormalCheckCount; ++check)
    {
        // Establish the current integer position to check for nothingness on the sprite
        checkPixel = getpixel(pSprite, atomPos.m_X + m_sNormalChecks[check][X], atomPos.m_Y + m_sNormalChecks[check][Y]);

        // If the pixel was outside of the bitmap, or on the key color, then that's a valid direction for normal, add it to the accumulated normal
        if (checkPixel < 0 || checkPixel == g_KeyColor)
        {
            m_Normal.m_X += m_sNormalChecks[check][X];
            m_Normal.m_Y += m_sNormalChecks[check][Y];
        }
    }
    // Now normalize the normal vector (duh)
    m_Normal.Normalize();
/*
    // Check whether the normal vector makes sense at all. It can't point against the offset, for example
    if (m_Normal.Dot(m_Offset) < 0)
    {
        // Abort and revert to offset-based normal
        m_Normal = m_Offset;
        m_Normal.Normalize();
        return false;
    }
*/
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsIgnoringMOID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether this Atom is set to ignore collisions with a MO of a
///                 specific MOID.

bool Atom::IsIgnoringMOID(MOID which)
{
    if (which == m_IgnoreMOID)
        return true;

    // First check if we are ignoring the team of the MO we hit, or if it's an AtomGroup and we're ignoring all those
    const MovableObject *pHitMO = g_MovableMan.GetMOFromID(which);
    pHitMO = pHitMO ? pHitMO->GetRootParent() : 0;
    if (m_pOwnerMO && pHitMO)
    {
        // Team hits ignored?
        if (m_pOwnerMO->IgnoresTeamHits() && pHitMO->IgnoresTeamHits() && m_pOwnerMO->GetTeam() == pHitMO->GetTeam())
            return true;

        // AtomGroup hits ignored?
        if ((m_pOwnerMO->IgnoresAtomGroupHits() && dynamic_cast<const MOSRotating *>(pHitMO)) ||
            (pHitMO->IgnoresAtomGroupHits() && dynamic_cast<const MOSRotating *>(m_pOwnerMO)))
            return true;
    }

    // Now check for explicit ignore
    bool ignored = false;
    for (list<MOID>::iterator itr = m_IgnoreMOIDs.begin(); itr != m_IgnoreMOIDs.end(); ++itr) {
        if ((*itr) == which)
        {
            ignored = true;
            break;
        }
    }
	// Check in AtomGroup-owned list if it's assigned to this atom
	if (!ignored && m_pIgnoreMOIDsByGroup)
	{
		for (list<MOID>::const_iterator itr = m_pIgnoreMOIDsByGroup->begin(); itr != m_pIgnoreMOIDsByGroup->end(); ++itr) {
			if ((*itr) == which)
			{
				ignored = true;
				break;
			}
		}
	}

    return ignored;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetupPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this Atom's initial position up for a straight seg of a trajectory
//                  to step through. This is to be done before SetupSeg. It will report
//                  whether the atom's position is inside either the terrain or another MO.

bool Atom::SetupPos(Vector startPos)
{
    RTEAssert(m_pOwnerMO, "Stepping an Atom without a parent MO!");

    // Only save the previous positions if they are in the scene
    if (m_IntPos[X] > 0 && m_IntPos[Y] > 0)
    {
        m_PrevIntPos[X] = m_IntPos[X];
        m_PrevIntPos[Y] = m_IntPos[Y];
        m_IntPos[X] = floorf(startPos.m_X);
        m_IntPos[Y] = floorf(startPos.m_Y);
    }
    else
    {
        m_IntPos[X] = m_PrevIntPos[X] = floorf(startPos.m_X);
        m_IntPos[Y] = m_PrevIntPos[Y] = floorf(startPos.m_Y);
    }

    if ((m_TerrainMatHit = g_SceneMan.GetTerrMatter(m_IntPos[X], m_IntPos[Y])) != g_MaterialAir)
    {
		m_pOwnerMO->SetHitWhatTerrMaterial(m_TerrainMatHit);

        if (m_pOwnerMO->IntersectionWarning())
            m_TerrainHitsDisabled = true;
    }
    else
    {
        m_TerrainHitsDisabled = false;
    }
/*
    if (m_pOwnerMO->m_HitsMOs && (m_MOIDHit = g_SceneMan.GetMOIDPixel(m_IntPos[X], m_IntPos[Y])) != g_NoMOID)
    {
        m_MOHitsDisabled = true;
    }
    else
    {
        m_MOHitsDisabled = false;
    }
*/
    return m_MOIDHit != g_NoMOID || m_TerrainMatHit != g_MaterialAir;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetupSeg
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this Atom up for a straight seg of a trajectory to step through.
//                  If this Atom find the startPos to be on an MO, it will ignore any
//                  collisions with that MO for the entire seg.

int Atom::SetupSeg(Vector startPos, Vector trajectory, float stepRatio)
{
    RTEAssert(m_pOwnerMO, "Stepping an Atom without a parent MO!");
    bool inNewMO = false;
    m_TerrainMatHit = g_MaterialAir;
    m_MOIDHit = g_NoMOID;

    m_StepRatio = stepRatio;
    m_SegProgress = 0.0;
    m_SegTraj = trajectory;

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm preparation

    m_Delta[X] = floorf(startPos.m_X + trajectory.m_X) - floorf(startPos.m_X);
    m_Delta[Y] = floorf(startPos.m_Y + trajectory.m_Y) - floorf(startPos.m_Y);
    m_DomSteps = 0;
    m_SubSteps = 0;
    m_SubStepped = false;

    if (m_Delta[X] < 0) {
        m_Increment[X] = -1;
        m_Delta[X] = -m_Delta[X];
    }
    else
        m_Increment[X] = 1;

    if (m_Delta[Y] < 0) {
        m_Increment[Y] = -1;
        m_Delta[Y] = -m_Delta[Y];
    }
    else
        m_Increment[Y] = 1;

    // Scale by 2, for better accuracy of the error at the first pixel
    m_Delta2[X] = m_Delta[X] << 1;
    m_Delta2[Y] = m_Delta[Y] << 1;

    // If X is dominant, Y is submissive, and vice versa.
    if (m_Delta[X] > m_Delta[Y]) {
        m_Dom = X;
        m_Sub = Y;
    }
    else {
        m_Dom = Y;
        m_Sub = X;
    }

    m_Error = m_Delta2[m_Sub] - m_Delta[m_Dom];

    m_DomSteps = 0;
    m_SubSteps = 0;
    m_SubStepped = false;
    m_StepWasTaken = false;

    // Return how many steps there are for this atom to take
    return m_Delta[m_Dom] - m_DomSteps;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StepForward
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes one step along the trajectory seg set up by SetupSeg().

bool Atom::StepForward(int numSteps)
{
    RTEAssert(m_pOwnerMO, "Stepping an Atom without a parent MO!");

    // Only take the step if the step ratio permits it
    float prevProgress = m_SegProgress;
    if (m_Delta[m_Dom] && (m_SegProgress += m_StepRatio) >= floorf(prevProgress + 1.0))
    {
        m_StepWasTaken = true;
        m_MOIDHit = g_NoMOID;
        m_TerrainMatHit = g_MaterialAir;
        bool hitStep = false;

        if (m_DomSteps < m_Delta[m_Dom]) {
            ++m_DomSteps;
            if (m_SubStepped)
                ++m_SubSteps;
            m_SubStepped = false;

            m_IntPos[m_Dom] += m_Increment[m_Dom];
            if (m_Error >= 0) {
                m_IntPos[m_Sub] += m_Increment[m_Sub];
                m_SubStepped = true;
                m_Error -= m_Delta2[m_Dom];
            }
    //        if (m_ChangedDir)
                m_Error += m_Delta2[m_Sub];
    //        else
    //            m_Error = m_PrevError;


            // Scene wrapping, if necessary
            g_SceneMan.WrapPosition(m_IntPos[X], m_IntPos[Y]);

            // Detect terrain hits, if not disabled.
            if (g_MaterialAir != (m_TerrainMatHit = g_SceneMan.GetTerrMatter(m_IntPos[X], m_IntPos[Y])))
            {
                // Check if we're temporarily disabled from hitting terrain
                if (!m_TerrainHitsDisabled)
                {
					m_pOwnerMO->SetHitWhatTerrMaterial(m_TerrainMatHit);

                    m_HitPos[X] = m_IntPos[X];
                    m_HitPos[Y] = m_IntPos[Y];
                    RTEAssert(m_TerrainMatHit != 0, "Atom returning step with positive hit but without ID stored!");
                    hitStep = true;
                }
            }
            // Re-enable terrain hits if we are now out of the terrain again
            else
                m_TerrainHitsDisabled = false;

            // Detect hits with non-ignored MO's, if enabled.
            if (m_pOwnerMO->m_HitsMOs)
            {
                m_MOIDHit = g_SceneMan.GetMOIDPixel(m_IntPos[X], m_IntPos[Y]);

                if (IsIgnoringMOID(m_MOIDHit))
                    m_MOIDHit = g_NoMOID;

                if (m_MOIDHit != g_NoMOID)
                {
                    if (!m_MOHitsDisabled)
                    {
                        m_HitPos[X] = m_IntPos[X];
                        m_HitPos[Y] = m_IntPos[Y];
                        RTEAssert(m_MOIDHit != g_NoMOID, "Atom returning step with positive hit but without ID stored!");
                        hitStep = true;

						m_pOwnerMO->SetHitWhatMOID(m_MOIDHit);
                    }
                }
                else
                    m_MOHitsDisabled = false;
            }

            return hitStep;
        }
        RTEAssert(0, "Atom shouldn't be taking steps beyond the trajectory!");
        m_pOwnerMO->SetToDelete();
    }
    m_StepWasTaken = false;
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StepBack
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes one step back, or undos the step, if any, previously taken along
//                  the trajectory seg set up by SetupSeg(). CAUTION: The previous HitWhat
//                  vars are not reset to what they previously were!

void Atom::StepBack()
{
    RTEAssert(m_pOwnerMO, "Stepping an Atom without a parent MO!");

    // Not complete undo because we lost what these were during last step.
//    m_MOIDHit = g_NoMOID;
//    m_TerrainMatHit = g_MaterialAir;

//    m_ChangedDir = true;
//    m_PrevError = m_Error;

    m_SegProgress -= m_StepRatio;
    if (m_StepWasTaken) {
        --m_DomSteps;
        m_IntPos[m_Dom] -= m_Increment[m_Dom];
        if (m_SubStepped) {
            --m_SubSteps;
            m_IntPos[m_Sub] -= m_Increment[m_Sub];
        }
        g_SceneMan.WrapPosition(m_IntPos[X], m_IntPos[Y]);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MOHitResponse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the collision reponse with another MO, if the last step
//                  taken forward resulted in an intersection with a non-ignored MO. Note
//                  that one step backward should be taken after the intersecting step so
//                  that the Atom is not intersecting the hit MO anymore, and the hit
//                  response calculations can be made accurately.

bool Atom::MOHitResponse()
{
    RTEAssert(m_pOwnerMO, "Stepping an Atom without a parent MO!");

    if (m_pOwnerMO->m_HitsMOs && m_MOIDHit != g_NoMOID/* && IsIgnoringMOID(m_MOIDHit)*/) {
        m_LastHit.hitPoint.Reset();
        m_LastHit.bitmapNormal.Reset();
        bool hit[2];
        hit[X] = hit[Y] = false;
        bool validHit = true;

        // Check for the collision point in the dominant direction of travel.
        if (m_Delta[m_Dom] && ((m_Dom == X && g_SceneMan.GetMOIDPixel(m_HitPos[X], m_IntPos[Y]) != g_NoMOID) ||
                               (m_Dom == Y && g_SceneMan.GetMOIDPixel(m_IntPos[X], m_HitPos[Y]) != g_NoMOID))) {
            hit[m_Dom] = true;
            m_LastHit.hitPoint = m_Dom == X ? Vector(m_HitPos[X], m_IntPos[Y]) :
                                    Vector(m_IntPos[X], m_HitPos[Y]);
            m_LastHit.bitmapNormal[m_Dom] = -m_Increment[m_Dom];
        }

        // Check for the collision point in the submissive direction of travel.
        if (m_SubStepped && m_Delta[m_Sub] && ((m_Sub == X && g_SceneMan.GetMOIDPixel(m_HitPos[X], m_IntPos[Y]) != g_NoMOID) ||
                                               (m_Sub == Y && g_SceneMan.GetMOIDPixel(m_IntPos[X], m_HitPos[Y]) != g_NoMOID))) {
            hit[m_Sub] = true;
            if (m_LastHit.hitPoint.IsZero())
                m_LastHit.hitPoint = m_Sub == X ? Vector(m_HitPos[X], m_IntPos[Y]) :
                                                  Vector(m_IntPos[X], m_HitPos[Y]);
            // We hit pixels in both sub and dom directions on the other MO, a corner hit.
            else
                m_LastHit.hitPoint.SetXY(m_HitPos[X], m_HitPos[Y]);
            m_LastHit.bitmapNormal[m_Sub] = -m_Increment[m_Sub];
        }

        // If neither the direct dominant or sub directions yielded a collision point, then
        // that means we hit right on the corner edge of a pixel, and that is the collision point.
        if (!hit[m_Dom] && !hit[m_Sub]) {
            hit[m_Dom] = hit[m_Sub] = true;
            m_LastHit.hitPoint.SetXY(m_HitPos[X], m_HitPos[Y]);
            m_LastHit.bitmapNormal.SetXY(-m_Increment[X], -m_Increment[Y]);
        }
        m_LastHit.bitmapNormal.Normalize();

        if (!m_Normal.IsZero())
            m_LastHit.bitmapNormal = -m_pOwnerMO->RotateOffset(m_Normal);

        // Cancel collision response for this if it appears the collision is happening in the 'wrong' direction, meaning away from the center
        // This happens whn things are sunk into each other, and thus getting 'hooked' on each other
        if (m_LastHit.hitRadius[HITOR].Dot(m_LastHit.bitmapNormal) >= 0)
        {
            // Hitee hit radius and the normal presented to the hitor are facing each other! We are colliding in the wrong direction!
            validHit = false;
        }

        m_LastHit.pBody[HITOR] = m_pOwnerMO;
        m_LastHit.pBody[HITEE] = g_MovableMan.GetMOFromID(m_MOIDHit);
        RTEAssert(m_LastHit.pBody[HITEE], "Hitee MO is 0 in Atom::MOHitResponse!");
        RTEAssert(m_MOIDHit == m_LastHit.pBody[HITEE]->GetID(), "g_MovableMan.GetMOFromID messed up in in Atom::MOHitResponse!");
        // Get the roots for both bodies
        if (m_LastHit.pBody[HITOR])
            m_LastHit.pRootBody[HITOR] = m_LastHit.pBody[HITOR]->GetRootParent();
        if (m_LastHit.pBody[HITEE])
            m_LastHit.pRootBody[HITEE] = m_LastHit.pBody[HITEE]->GetRootParent();

        validHit = validHit && m_LastHit.pBody[HITEE]->CollideAtPoint(m_LastHit);

        return validHit;
    }
    RTEAbort("Atom not supposed to do MO hit response if it didnt hit anything!");
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TerrHitResponse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the collision reponse with the Terrain, if the last step
//                  taken forward resulted in an intersection with the Terrain. Note
//                  that one step backward should be taken after the intersecting step so
//                  that the Atom is not intersecting the Terrain anymore, and the hit
//                  response calculations can be made accurately.

HitData & Atom::TerrHitResponse()
{
    RTEAssert(m_pOwnerMO, "Stepping an Atom without a parent MO!");

    if (m_TerrainMatHit) {
        MID hitMaterialID = g_SceneMan.GetTerrMatter(m_HitPos[X], m_HitPos[Y]);
        MID domMaterialID = g_MaterialAir, subMaterialID = g_MaterialAir;
        m_LastHit.hitMaterial[HITOR] = m_pMaterial;
        Material const * hitMaterial = m_LastHit.hitMaterial[HITEE] = g_SceneMan.GetMaterialFromID(hitMaterialID);
		Material const * domMaterial = g_SceneMan.GetMaterialFromID(g_MaterialAir);
		Material const * subMaterial = g_SceneMan.GetMaterialFromID(g_MaterialAir);
        bool hit[2];
        hit[X] = hit[Y] = false;
        m_LastHit.bitmapNormal.Reset();
        Vector hitAcc = m_LastHit.hitVel[HITOR];

        // Check for and react upon a collision in the dominant direction of travel.
        if (m_Delta[m_Dom] && ((m_Dom == X && g_SceneMan.GetTerrMatter(m_HitPos[X], m_IntPos[Y])) ||
                               (m_Dom == Y && g_SceneMan.GetTerrMatter(m_IntPos[X], m_HitPos[Y])))) {
            hit[m_Dom] = true;
            domMaterialID = m_Dom == X ? g_SceneMan.GetTerrMatter(m_HitPos[X], m_IntPos[Y]) :
                                         g_SceneMan.GetTerrMatter(m_IntPos[X], m_HitPos[Y]);
            domMaterial = g_SceneMan.GetMaterialFromID(domMaterialID);

            // Edit the normal accordingly.
            m_LastHit.bitmapNormal[m_Dom] = -m_Increment[m_Dom];
            // Bounce according to the collision.
            hitAcc[m_Dom] = -hitAcc[m_Dom] - hitAcc[m_Dom] *
                                             m_pMaterial->restitution *
                                             domMaterial->restitution;
        }

        // Check for and react upon a collision in the submissive direction of travel.
        if (m_SubStepped && m_Delta[m_Sub] && ((m_Sub == X && g_SceneMan.GetTerrMatter(m_HitPos[X], m_IntPos[Y])) ||
                                             (m_Sub == Y && g_SceneMan.GetTerrMatter(m_IntPos[X], m_HitPos[Y])))) {
            hit[m_Sub] = true;
            subMaterialID = m_Sub == X ? g_SceneMan.GetTerrMatter(m_HitPos[X], m_IntPos[Y]) :
                                         g_SceneMan.GetTerrMatter(m_IntPos[X], m_HitPos[Y]);
            subMaterial = g_SceneMan.GetMaterialFromID(subMaterialID);

            // Edit the normal accordingly.
            m_LastHit.bitmapNormal[m_Sub] = -m_Increment[m_Sub];
            // Bounce according to the collision.
            hitAcc[m_Sub] = -hitAcc[m_Sub] - hitAcc[m_Sub] *
                                             m_pMaterial->restitution *
                                             subMaterial->restitution;
        }

        // If hit right on the corner of a pixel, bounce straight back with no friction.
        if (!hit[m_Dom] && !hit[m_Sub]) {
            // Edit the normal accordingly.
            m_LastHit.bitmapNormal[m_Dom] = -m_Increment[m_Dom];
            m_LastHit.bitmapNormal[m_Sub] = -m_Increment[m_Sub];

            hit[m_Dom] = true;
            hitAcc[m_Dom] = -hitAcc[m_Dom] - hitAcc[m_Dom] *
                                             m_pMaterial->restitution *
                                             hitMaterial->restitution;
            hit[m_Sub] = true;
            hitAcc[m_Sub] = -hitAcc[m_Sub] - hitAcc[m_Sub] *
                                             m_pMaterial->restitution *
                                             hitMaterial->restitution;
        }
        // Calculate the effects of friction.
        else if (hit[m_Dom] && !hit[m_Sub]) {
            m_LastHit.bitmapNormal[m_Sub] = -m_Increment[m_Sub] * m_pMaterial->friction * domMaterial->friction;
            hitAcc[m_Sub] = -hitAcc[m_Sub] * m_pMaterial->friction * domMaterial->friction;
        }
        else if (hit[m_Sub] && !hit[m_Dom]) {
            m_LastHit.bitmapNormal[m_Dom] = -m_Increment[m_Dom] * m_pMaterial->friction * domMaterial->friction;
            hitAcc[m_Dom] = -hitAcc[m_Dom] * m_pMaterial->friction * subMaterial->friction;
        }
        m_LastHit.bitmapNormal.Normalize();

        // Calc effects of moment of inertia will have on the impulse.
        float MIhandle = m_LastHit.hitRadius[HITOR].GetPerpendicular().Dot(m_LastHit.bitmapNormal);

        // Calc the actual impulse force.
        m_LastHit.resImpulse[HITOR] = hitAcc / ((1.0 / m_LastHit.mass[HITOR]) +
                                                (MIhandle * MIhandle / m_LastHit.momInertia[HITOR]));
        // Scale by the impulse factor.
        m_LastHit.resImpulse[HITOR] *= m_LastHit.impFactor[HITOR];
        // Return the hit info.
        return m_LastHit;
    }
    RTEAbort("Atom not supposed to do Terrain hit response if it didnt hit anything!");
    return m_LastHit;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Travel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Uses the current state of the owning MovableObject to determine if
//                  if there are any collisions in the path of its travel during this
//                  frame, and if so, apply all collision responses to the MO.

int Atom::Travel(float travelTime,
                 bool autoTravel,
                 bool scenePreLocked)
{
    if (!m_pOwnerMO) {
        RTEAbort("Travelling an Atom without a parent MO!");
        return travelTime;
    }
    Vector &position = m_pOwnerMO->m_Pos;
    Vector &velocity = m_pOwnerMO->m_Vel;
    float mass = m_pOwnerMO->GetMass();
    float sharpness = m_pOwnerMO->GetSharpness();
    bool &didWrap = m_pOwnerMO->m_DidWrap;
    m_LastHit.Reset();

    BITMAP *pTrailBitmap;

    int hitCount = 0, error, dom, sub, domSteps, subSteps;
    int intPos[2], hitPos[2], delta[2], delta2[2], increment[2];
    float timeLeft = travelTime, segProgress, retardation;
    bool hit[2], sinkHit, subStepped, endOfTraj = false;
    unsigned char hitMaterialID, domMaterialID, subMaterialID;
	Material const *hitMaterial = g_SceneMan.GetMaterialFromID(g_MaterialAir);
	Material const *domMaterial = g_SceneMan.GetMaterialFromID(g_MaterialAir);
	Material const *subMaterial = g_SceneMan.GetMaterialFromID(g_MaterialAir);
    Vector segTraj, hitAccel;
    vector<pair<int, int> > trailPoints;
	// This saves us a few ms because Atom::Travel does a lot of allocations and rellocations
	// if you have a lot of particles. 6 should be enough for most not so fast travels, everything 
	// above simply works as usual.
	trailPoints.reserve(6);
    didWrap = false;
	int removeOrphansRadius = m_pOwnerMO->m_RemoveOrphanTerrainRadius;
	int removeOrphansMaxArea = m_pOwnerMO->m_RemoveOrphanTerrainMaxArea;
	float removeOrphansRate = m_pOwnerMO->m_RemoveOrphanTerrainRate;

    // Bake in the Atom offset.
    position += m_Offset;

    // Lock all bitmaps involved outside the loop.
    if (!scenePreLocked)
        g_SceneMan.LockScene();

    // Loop for all the different straight segs (between bounces etc) that
    // have to be traveled during the timeLeft.
    do {
        intPos[X] = floorf(position.m_X);
        intPos[Y] = floorf(position.m_Y);

        // Get trail bitmap and put first pixel.
        if (m_TrailLength) {
            pTrailBitmap = g_SceneMan.GetMOColorBitmap();
            trailPoints.push_back(make_pair(intPos[X], intPos[Y]));
        }

        // Compute and scale the actual on-screen travel trajectory for this seg,
        // based on the velocity, the travel time and the pixels-per-meter constant.
        segTraj = velocity * timeLeft * g_FrameMan.GetPPM();

        delta[X] = floorf(position.m_X + segTraj.m_X) - intPos[X];
        delta[Y] = floorf(position.m_Y + segTraj.m_Y) - intPos[Y];

        segProgress = 0.0;
//      delta2[X] = 0;
//      delta2[Y] = 0;
//      increment[X] = 0;
//      increment[Y] = 0;
        hit[X] = false;
        hit[Y] = false;
        domSteps = 0;
        subSteps = 0;
        subStepped = false;
        sinkHit = false;
        hitAccel.Reset();

        if (delta[X] == 0 && delta[Y] == 0)
            break;

        hitMaterialID = domMaterialID = subMaterialID = 0;
        //hitMaterial->Reset();
        //domMaterial->Reset();
        //subMaterial->Reset();

        /////////////////////////////////////////////////////
        // Bresenham's line drawing algorithm preparation

        if (delta[X] < 0) {
            increment[X] = -1;
            delta[X] = -delta[X];
        }
        else
            increment[X] = 1;

        if (delta[Y] < 0) {
            increment[Y] = -1;
            delta[Y] = -delta[Y];
        }
        else
            increment[Y] = 1;

        // Scale by 2, for better accuracy of the error at the first pixel
        delta2[X] = delta[X] << 1;
        delta2[Y] = delta[Y] << 1;

        // If X is dominant, Y is submissive, and vice versa.
        if (delta[X] > delta[Y]) {
            dom = X;
            sub = Y;
        }
        else {
            dom = Y;
            sub = X;
        }

        if (m_ChangedDir)
            error = delta2[sub] - delta[dom];
        else
            error = m_PrevError;

        /////////////////////////////////////////////////////
        // Bresenham's line drawing algorithm execution

        for (domSteps = 0; domSteps < delta[dom] && !(hit[X] || hit[Y]); ++domSteps)
        {
            // Check for the special case if the Atom is starting out embedded in terrain.
            // This can happen if something large gets copied to the terrain and imbeds some Atom:s.
            if (domSteps == 0 && g_SceneMan.GetTerrMatter(intPos[X], intPos[Y]) != g_MaterialAir) {
                ++hitCount;
                hit[X] = hit[Y] = true;
                if (g_SceneMan.TryPenetrate(intPos[X],
                                            intPos[Y],
                                            velocity * mass * sharpness,
                                            velocity,
                                            retardation,
                                            0.5,
                                            m_NumPenetrations,
											removeOrphansRadius,
											removeOrphansMaxArea,
											removeOrphansRate)) {
                    segProgress = 0.0;
                    velocity += velocity * retardation;
                    continue;
                }
                else {
                    segProgress = 1.0;
                    velocity.SetXY(0, 0);
                    timeLeft = 0.0;
                    break;
                }       
            }

            if (subStepped)
                ++subSteps;
            subStepped = false;

            intPos[dom] += increment[dom];
            if (error >= 0) {
                intPos[sub] += increment[sub];
                subStepped = true;
                error -= delta2[dom];
            }
            error += delta2[sub];

            // Scene wrapping, if necessary
            g_SceneMan.WrapPosition(intPos[X], intPos[Y]);

            /////////////////////////////////////////////////////
            // Atom-MO collision detection and response
            // Detect hits with non-ignored MO's, if enabled.

            m_MOIDHit = g_SceneMan.GetMOIDPixel(intPos[X], intPos[Y]);

            if (m_pOwnerMO->m_HitsMOs && m_MOIDHit != g_NoMOID && !IsIgnoringMOID(m_MOIDHit))
            {
				m_pOwnerMO->SetHitWhatMOID(m_MOIDHit);

                ++hitCount;
                hitPos[X] = intPos[X];
                hitPos[Y] = intPos[Y];

                // Back up so the Atom is not inside the MO.
                intPos[dom] -= increment[dom];
                if (subStepped)
                    intPos[sub] -= increment[sub];

                m_LastHit.Reset();
                m_LastHit.mass[HITOR] = mass;
// TODO: Is this right? Perhaps should be 0?")
                m_LastHit.momInertia[HITOR] = 1.0;
                m_LastHit.impFactor[HITOR] = 1.0;
                m_LastHit.impFactor[HITEE] = 1.0;
//                m_LastHit.hitPoint = Vector(hitPos[X], hitPos[Y]);
                m_LastHit.hitVel[HITOR] = velocity;

				MovableObject * pMO = g_MovableMan.GetMOFromID(m_MOIDHit);

				if (pMO)
					pMO->SetHitWhatParticleUniqueID(m_pOwnerMO->GetUniqueID());

                m_LastHit.pBody[HITOR] = m_pOwnerMO;
				m_LastHit.pBody[HITEE] = pMO;
                RTEAssert(m_LastHit.pBody[HITEE], "Hitee MO is 0 in Atom::Travel!");
                RTEAssert(m_MOIDHit == m_LastHit.pBody[HITEE]->GetID(), "g_MovableMan.GetMOFromID messed up in in Atom::MOHitResponse!");

                // Don't do this normal approximation based on object centers, it causes particles to 'slide into' sprite objects when they
                // should be resting on them. Orthogonal normals only, as the pixel boundaries themselves! See further down for the setting og this.
//                m_LastHit.bitmapNormal = m_LastHit.pBody[HITOR]->GetPos() - m_LastHit.pBody[HITEE]->GetPos();
//                m_LastHit.bitmapNormal.Normalize();

                // Gold special collection case!
// TODO: Make material IDs more robust!")
                if (m_pMaterial->id == c_GoldMaterialID && g_MovableMan.IsOfActor(m_MOIDHit)) {
                    Actor *pActor = dynamic_cast<Actor *>(g_MovableMan.GetMOFromID(m_LastHit.pBody[HITEE]->GetRootID()));
                    if (pActor)
                    {
						pActor->AddGold(m_pOwnerMO->GetMass() * g_SceneMan.GetOzPerKg() * removeOrphansRadius ? 1.25 : 1.0);
                        m_pOwnerMO->SetToDelete(true);
                        // This is to break out of the do-while and the function properly.
                        m_LastHit.terminate[HITOR] = hit[dom] = hit[sub] = true;
                        break;
                    }
                }

                // Check for the collision point in the dominant direction of travel.
                if (delta[dom] && ((dom == X && g_SceneMan.GetMOIDPixel(hitPos[X], intPos[Y]) != g_NoMOID) ||
                                   (dom == Y && g_SceneMan.GetMOIDPixel(intPos[X], hitPos[Y]) != g_NoMOID))) {
                    hit[dom] = true;
                    m_LastHit.hitPoint = dom == X ? Vector(hitPos[X], intPos[Y]) :
                                                    Vector(intPos[X], hitPos[Y]);
                    m_LastHit.bitmapNormal[dom] = -increment[dom];
                }

                // Check for the collision point in the submissive direction of travel.
                if (subStepped && delta[sub] && ((sub == X && g_SceneMan.GetMOIDPixel(hitPos[X], intPos[Y]) != g_NoMOID) ||
                                                 (sub == Y && g_SceneMan.GetMOIDPixel(intPos[X], hitPos[Y]) != g_NoMOID))) {
                    hit[sub] = true;
                    if (m_LastHit.hitPoint.IsZero())
                        m_LastHit.hitPoint = sub == X ? Vector(hitPos[X], intPos[Y]) :
                                                        Vector(intPos[X], hitPos[Y]);
                    // We hit pixels in both sub and dom directions on the other MO, a corner hit.
                    else
                        m_LastHit.hitPoint.SetXY(hitPos[X], hitPos[Y]);
                        m_LastHit.bitmapNormal[sub] = -increment[sub];
                }

                // If neither the direct dominant or sub directions yielded a collision point, then
                // that means we hit right on the corner edge of a pixel, and that is the collision point.
                if (!hit[dom] && !hit[sub]) {
                    hit[dom] = hit[sub] = true;
                    m_LastHit.hitPoint.SetXY(hitPos[X], hitPos[Y]);
                    m_LastHit.bitmapNormal.SetXY(-increment[X], -increment[Y]);
                }

                // Now normalize the normal in case it's diagonal due to hit in both directions
                m_LastHit.bitmapNormal.Normalize();

                // Make this Atom ignore hits with this MO for the rest of the frame, to avoid
                // erroneous multiple hits because the hit MO doesn't move away until it itself

                // is updated and the impulses produced in this hit are taken into effect.
                AddMOIDToIgnore(m_MOIDHit);

                m_LastHit.pBody[HITEE]->CollideAtPoint(m_LastHit);
                hitAccel = m_LastHit.resImpulse[HITOR] / mass;

                // Report the hit to both MO's in collision
                m_LastHit.pRootBody[HITOR] = m_LastHit.pBody[HITOR]->GetRootParent();
                m_LastHit.pRootBody[HITEE] = m_LastHit.pBody[HITEE]->GetRootParent();
                m_LastHit.pRootBody[HITOR]->OnMOHit(m_LastHit.pRootBody[HITEE]);
                m_LastHit.pRootBody[HITEE]->OnMOHit(m_LastHit.pRootBody[HITOR]);
            }
            /////////////////////////////////////////////////////
            // Atom-Terrain collision detection and response
            // If there was no MO collision detected, then check for terrain hits.
			else if ((hitMaterialID = g_SceneMan.GetTerrMatter(intPos[X], intPos[Y])) && !m_pOwnerMO->m_IgnoreTerrain)
            {
				if (hitMaterialID != g_MaterialAir)
					m_pOwnerMO->SetHitWhatTerrMaterial(hitMaterialID);

                hitMaterial = g_SceneMan.GetMaterialFromID(hitMaterialID);
                hitPos[X] = intPos[X];
                hitPos[Y] = intPos[Y];
                ++hitCount;

#ifdef DEBUG_BUILD
                if (m_TrailLength)
                    putpixel(pTrailBitmap, intPos[X], intPos[Y], 199);
#endif
                // Try penetration of the terrain.
                if (hitMaterial->id != g_MaterialOutOfBounds &&
                    g_SceneMan.TryPenetrate(intPos[X],
                                            intPos[Y],
                                            velocity * mass * sharpness,
                                            velocity,
                                            retardation,
                                            0.65,
                                            m_NumPenetrations,
											removeOrphansRadius,
											removeOrphansMaxArea,
											removeOrphansRate)) {
                    hit[dom] = hit[sub] = sinkHit = true;
                    ++m_NumPenetrations;
                    m_ChangedDir = false;
                    m_PrevError = error;

                    // Calc the penetration/sink response effects.
                    hitAccel = velocity * retardation;
                }
                // Penetration failed, bounce.
                else {
                    m_NumPenetrations = 0;
                    m_ChangedDir = true;
                    m_PrevError = error;

                    // Back up so the Atom is not inside the terrain.
                    intPos[dom] -= increment[dom];
                    if (subStepped)
                        intPos[sub] -= increment[sub];

                    // Undo scene wrapping, if necessary
                    g_SceneMan.WrapPosition(intPos[X], intPos[Y]);

// TODO: improve sticky logic!
                    // Check if particle is sticky and should adhere to where it collided
                    if (m_pMaterial->stickiness >= PosRand() && velocity.GetLargest() > 0.5)
                    {
                        // SPLAT, so update position, apply to terrain and delete, and stop traveling
                        m_pOwnerMO->SetPos(Vector(intPos[X], intPos[Y]));
                        g_SceneMan.GetTerrain()->ApplyMovableObject(m_pOwnerMO);
                        m_pOwnerMO->SetToDelete(true);
                        m_LastHit.terminate[HITOR] = hit[dom] = hit[sub] = true;
                        break;
                    }

                    // Check for and react upon a collision in the dominant direction of travel.
                    if (delta[dom] && ((dom == X && g_SceneMan.GetTerrMatter(hitPos[X], intPos[Y])) ||
                                       (dom == Y && g_SceneMan.GetTerrMatter(intPos[X], hitPos[Y])))) {
                        hit[dom] = true;
                        domMaterialID = dom == X ? g_SceneMan.GetTerrMatter(hitPos[X], intPos[Y]) :
                                                   g_SceneMan.GetTerrMatter(intPos[X], hitPos[Y]);
                        domMaterial = g_SceneMan.GetMaterialFromID(domMaterialID);

                        // Bounce according to the collision.
                        hitAccel[dom] = -velocity[dom] - velocity[dom] *
                                                         m_pMaterial->restitution *
                                                         domMaterial->restitution;
                    }

                    // Check for and react upon a collision in the submissive direction of travel.
                    if (subStepped && delta[sub] && ((sub == X && g_SceneMan.GetTerrMatter(hitPos[X], intPos[Y])) ||
                                                     (sub == Y && g_SceneMan.GetTerrMatter(intPos[X], hitPos[Y])))) {
                        hit[sub] = true;
                        subMaterialID = sub == X ? g_SceneMan.GetTerrMatter(hitPos[X], intPos[Y]) :
                                                   g_SceneMan.GetTerrMatter(intPos[X], hitPos[Y]);
                        subMaterial = g_SceneMan.GetMaterialFromID(subMaterialID);

                        // Bounce according to the collision.
                        hitAccel[sub] = -velocity[sub] - velocity[sub] *
                                                         m_pMaterial->restitution *
                                                         subMaterial->restitution;
                    }

                    // If hit right on the corner of a pixel, bounce straight back with no friction.
                    if (!hit[dom] && !hit[sub]) {
                        hit[dom] = true;
                        hitAccel[dom] = -velocity[dom] - velocity[dom] *
                                                         m_pMaterial->restitution *
                                                         hitMaterial->restitution;
                        hit[sub] = true;
                        hitAccel[sub] = -velocity[sub] - velocity[sub] *
                                                         m_pMaterial->restitution *
														 hitMaterial->restitution;
                    }
                    // Calculate the effects of friction.
                    else if (hit[dom] && !hit[sub]) {
						hitAccel[sub] -= velocity[sub] * m_pMaterial->friction * domMaterial->friction;
                    }
                    else if (hit[sub] && !hit[dom]) {
						hitAccel[dom] -= velocity[dom] * m_pMaterial->friction * subMaterial->friction;
                    }
                }
            }
            else if (m_TrailLength)
                trailPoints.push_back(make_pair(intPos[X], intPos[Y]));

            // APPLY COLLISION RESPONSES ///////////////////////////////////////////////////
            // If we hit anything, and are about to start a new seg instead of a step,
            // apply the collision response effects to the owning MO.
            if ((hit[X] || hit[Y]) && !m_LastHit.terminate[HITOR]) {
                // Calculate the progress made on this seg before hitting something.
                // We count the hitting step made if it resulted in a terrain sink,
                // because the Atoms weren't stepped back out of intersection.
//                segProgress = (float)(domSteps + sinkHit) / (float)delta[dom];

                if ((domSteps + sinkHit) < delta[dom])
                    segProgress = (float)(domSteps + sinkHit) / fabs((float)segTraj[dom]);
                else
                    segProgress = 1.0;
                // Now calculate the total time left to travel, according to the progress made.
                timeLeft -= timeLeft * segProgress;

                // Move position forward to the hit position.
//                position += segTraj * segProgress;
                // Only move the dom forward by int domSteps, so we don't cross into a pixel too far
                position[dom] += (domSteps + sinkHit) * increment[dom];

                // Move the submissive direction forward by as many int steps, or the full float segTraj if all substeps are clear
                if ((subSteps + (subStepped && sinkHit)) < delta[sub])
                    position[sub] += (subSteps + (subStepped && sinkHit)) * increment[sub];
                else
                    position[sub] += segTraj[sub];

				Vector testPos = position - m_Offset;
				
                // Wrap position, if necessary.
                didWrap = g_SceneMan.WrapPosition(testPos) || didWrap;

                // Apply the collision response acceleration to the
                // linear velocity of the owner MO.
                velocity += hitAccel;
            }
        }
    } while ((hit[X] || hit[Y]) &&
//             !segTraj.GetFloored().IsZero() &&
             hitCount < 100 &&
             !m_LastHit.terminate[HITOR]);

//    RTEAssert(hitCount < 100, "Atom travel resulted in more than 100 segs!!");

    // Draw the trail
    if (g_TimerMan.DrawnSimUpdate() && m_TrailLength) {
        int length = m_TrailLength/* + 3 * PosRand()*/;
        for (int i = trailPoints.size() - MIN(length, trailPoints.size()); i < trailPoints.size(); ++i)
        {
//            RTEAssert(is_inside_bitmap(pTrailBitmap, trailPoints[i].first, trailPoints[i].second, 0), "Trying to draw out of bounds trail!");
//            _putpixel(pTrailBitmap, trailPoints[i].first, trailPoints[i].second, m_TrailColor.GetIndex());
            putpixel(pTrailBitmap, trailPoints[i].first, trailPoints[i].second, m_TrailColor.GetIndex());
        }
    }

    // Unlock all bitmaps involved.
//    if (m_TrailLength)
//        pTrailBitmap->UnLock();
    if (!scenePreLocked)
        g_SceneMan.UnlockScene();

    // Extract Atom offset.
    position -= m_Offset;

    // Travel along the remaining segTraj.
    if (!(hit[X] || hit[Y]) && autoTravel) {
        position += segTraj;
    }
    // Wrap, if necessary.
    didWrap = g_SceneMan.WrapPosition(position) || didWrap;

    // Clear out the MOID ignore list
    ClearMOIDIgnoreList();

    return hitCount;
}

} // namespace RTE
