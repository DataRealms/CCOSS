//////////////////////////////////////////////////////////////////////////////////////////
// File:            AtomGroup.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the AtomGroup class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "AtomGroup.h"
#include "SLTerrain.h"
#include "MOSRotating.h"
#include "ConsoleMan.h"

namespace RTE {

ConcreteClassInfo(AtomGroup, Entity, 500)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this AtomGroup, effectively
//                  resetting the members of this abstraction level only.

void AtomGroup::Clear()
{
    m_AutoGenerate = false;
	m_pMaterial = g_SceneMan.GetMaterialFromID(g_MaterialAir);
    m_Resolution = 1;
    m_Depth = 0;
    m_Atoms.clear();
    m_SubGroups.clear();
    m_MomInertia = 0;
    m_pOwnerMO = 0;
	m_IgnoreMOIDs.clear();

    m_LimbPos.Reset();
    m_JointOffset.Reset();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the AtomGroup object ready for use.

int AtomGroup::Create()
{
    if (Entity::Create() < 0)
        return -1;

    // If we weren't specified to be automatically generated
    if (!m_AutoGenerate)
    {
        // Needs to be set manually by the new MO owner.
        m_pOwnerMO = 0;
        m_Resolution = 0;
    }

    // Make sure we have at least one atom in the group, and that we have a proper material assigned
	if (m_Atoms.empty())
	{
		Atom * pAtom = new Atom(Vector(), m_pMaterial, m_pOwnerMO);

		m_Atoms.push_back(pAtom);
	}
    else if (m_pMaterial->GetIndex() != m_Atoms.front()->GetMaterial()->GetIndex())
        m_pMaterial = m_Atoms.front()->GetMaterial();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a AtomGroup to be identical to another, by deep copy.

int AtomGroup::Create(const AtomGroup &reference)
{
    return Create(reference, false);
}

int AtomGroup::Create(const AtomGroup &reference, bool onlyCopyOwnerAtoms)
{
    Entity::Create(reference);

    m_AutoGenerate = false; //Don't autogenerate because we'll copy the atoms below
    m_pMaterial = reference.m_pMaterial;
    m_Resolution = reference.m_Resolution;
    m_Depth = reference.m_Depth;

	m_SubGroups.clear();
	for (const Atom * atom : reference.m_Atoms)
	{
		if (!onlyCopyOwnerAtoms || atom->GetSubID() == 0)
		{
			Atom *pAtomCopy = new Atom(*atom);
			pAtomCopy->SetIgnoreMOIDsByGroup(&m_IgnoreMOIDs);

			m_Atoms.push_back(pAtomCopy);

			// Add to the appropriate spot in the subgroup map
			long int subID = pAtomCopy->GetSubID();
			if (subID != 0)
			{
				// Make a new list for the subgroup ID if there isn't one already
				if (m_SubGroups.find(subID) == m_SubGroups.end())
				{
					m_SubGroups.insert(pair<long int, list<Atom *>>(subID, list<Atom *>()));
				}
				// Add Atom to the list of that group
				m_SubGroups.find(subID)->second.push_back(pAtomCopy);
			}
		}
	}

	// Copy ignored MOIDs list
	for (const MOID  moidToIgnore : reference.m_IgnoreMOIDs)
	{
		m_IgnoreMOIDs.push_back(moidToIgnore);
	}


    // Make sure the transfer of material properties happens
    if (!reference.m_Atoms.empty())
    {
        m_pMaterial = reference.m_Atoms.front()->GetMaterial();
    }

    // Needs to be set manually by the new MO owner.
    m_pOwnerMO = 0;

    m_JointOffset = reference.m_JointOffset;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a AtomGroup after the silhouette shape of a passed in MOSRotating,
//                  by dotting the outline of the sprite with atoms. The passed in MOSRotating
//                  will also be made the owner of this AtomGroup.

int AtomGroup::Create(MOSRotating *pOwnerMOSRotating)
{
    return Create(pOwnerMOSRotating, m_pMaterial, m_Resolution, m_Depth);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a AtomGroup after the silhouette shape of a passed in MOSRotating,
//                  by dotting the outline of the sprite with atoms. The passed in MOSRotating
//                  will also be made the owner of this AtomGroup.

int AtomGroup::Create(MOSRotating *pOwnerMOSRotating, Material const *material, int resolution, int depth)
{
    if (!pOwnerMOSRotating || resolution < 0)
    {
        RTEAbort("Trying to generate an AtomGroup without sprite and//or 0 resolution setting!");
        return -1;
    }

    m_pOwnerMO = pOwnerMOSRotating;
    m_AutoGenerate = true;
    m_pMaterial = material;
    m_Resolution = resolution > 0 ? resolution : g_MovableMan.GetAGResolution();
    m_Resolution = m_Resolution > 0 ? m_Resolution : 1;
    m_Depth = depth;
    BITMAP *refSprite = pOwnerMOSRotating->GetSpriteFrame();
	BITMAP *checkBitmap = 0;
    int width = refSprite->w * static_cast<int>(pOwnerMOSRotating->GetScale());
    int height = refSprite->h * static_cast<int>(pOwnerMOSRotating->GetScale());
	int x;
	int y;
	int i;
    Vector spriteOffset = pOwnerMOSRotating->GetSpriteOffset();
    bool inside = false;


// TODO: GET THIS WORKING WITH SCALED SPRITES!!$@#")
    // This offset is to fix some weirdness going on with the rotozoom
    int offX = -refSprite->w;
    int offY = -refSprite->h;

	// Only try to generate AtomGroup if scaled width and height are > 0 as we're playing with fire trying to create 0x0 bitmap. 
	// In debug mode it fires an assertion fail somewhere in DirectX
	if (width > 0 && height > 0)
	{
		checkBitmap = create_bitmap_ex(8, width, height);
		clear_to_color(checkBitmap, g_MaskColor);

		acquire_bitmap(refSprite);
		acquire_bitmap(checkBitmap);

		// If atoms are to be placed right at (below) the bitmap of the sprite.
		if (depth <= 0)
		{
			// First scan horizontally, and place Atoms on outer silhouette edges.
			for (y = 0; y < height; y += m_Resolution)
			{
				// Scan LEFT to RIGHT, placing one Atom on each first encountered silhouette edge
				inside = false;
				for (x = 0; x < width; ++x)
				{
					// Detect if we are crossing a silhouette boundary.
					if (getpixel(refSprite, x, y) != g_MaskColor)
					{
						// Mark that an atom has been put in this location, to avoid duplicate Atoms
						putpixel(checkBitmap, x, y, 99);
						AddAtomToGroup(x, y, spriteOffset, pOwnerMOSRotating, true);
						break;
					}
				}

				// Scan RIGHT to LEFT, placing one Atom on each first encountered silhouette edge
				inside = false;
				for (x = width - 1; x >= 0; --x)
				{
					// Detect if we are crossing a silhouette boundary.
					if (getpixel(refSprite, x, y) != g_MaskColor)
					{
						// Mark that an atom has been put in this location, to avoid duplicate Atoms
						putpixel(checkBitmap, x, y, 99);
						AddAtomToGroup(x, y, spriteOffset, pOwnerMOSRotating, true);
						break;
					}
				}
			}

			// Then scan vertically, and place Atoms on silhouette edge, but avoiding duplicates.
			for (x = 0; x < width; x += m_Resolution)
			{
				// Scan TOP to BOTTOM, placing one Atom on each first encountered silhouette edge
				inside = false;
				for (y = 0; y < height; ++y)
				{
					// Detect if we are crossing a silhouette boundary, but make sure Atom wasn't
					// already placed during the horizontal scans.
					if (getpixel(refSprite, x, y) != g_MaskColor && getpixel(checkBitmap, x, y) == g_MaskColor)
					{
						AddAtomToGroup(x, y - static_cast<int>(inside), spriteOffset, pOwnerMOSRotating, true);
						break;
					}
				}

				// Scan BOTTOM to TOP, placing one Atom on each first encountered silhouette edge
				inside = false;
				for (y = height - 1; y >= 0; --y)
				{
					// Detect if we are crossing a silhouette boundary, but make sure Atom wasn't
					// already placed during the horizontal scans.
					if (getpixel(refSprite, x, y) != g_MaskColor && getpixel(checkBitmap, x, y) == g_MaskColor)
					{
						AddAtomToGroup(x, y - static_cast<int>(inside), spriteOffset, pOwnerMOSRotating, true);
						break;
					}
				}
			}
		}
		// Atoms are to be placed at a depth into the bitmap
		else {
			int depthCount = 0;
			bool clear = true;

			// First scan HORIZONTALLY from LEFT to RIGHT,
			// and place Atoms in depth beyond the sihouette edge.
			for (y = 0; y < height; y += m_Resolution)
			{
				inside = false;
				for (x = 0; x < width; ++x)
				{
					// Detect if we are crossing a silhouette boundary.
					if ((getpixel(refSprite, x, y) != g_MaskColor && !inside) ||
						(getpixel(refSprite, x, y) == g_MaskColor && inside))
					{
						// Reset the depth counter
						depthCount = 0;
						inside = !inside;
					}
					// Increment the depth counter if inside silhouette,
					// and propose making atom if specified depth is reached.
					else if (inside) {
						++depthCount;
						if (depthCount == depth)
						{
							clear = true;
							// Check whether depth is sufficient in the other cardinal directions.
							for (i = 1; i <= depth && clear; ++i)
							{
								if (x + i >= refSprite->w ||
									y + i >= refSprite->h ||
									y - i < 0 ||
									getpixel(refSprite, x + i, y) == g_MaskColor ||
									getpixel(refSprite, x, y + i) == g_MaskColor ||
									getpixel(refSprite, x, y - i) == g_MaskColor)
									clear = false;
							}
							// Depth is cleared in all directions, so go ahead and place Atom.
							if (clear && getpixel(checkBitmap, x, y) == g_MaskColor)
							{
								// Mark that an atom has been put in this location, to avoid duplicate Atoms.
								putpixel(checkBitmap, x, y, 99);
								AddAtomToGroup(x, y, spriteOffset, pOwnerMOSRotating, false);
							}
						}
					}
				}
			}
			// Scan HORIZONTALLY from RIGHT to LEFT,
			// and place Atoms in depth beyond the sihouette edge.
			for (y = 0; y < height; y += m_Resolution)
			{
				inside = false;
				for (x = width - 1; x >= 0; --x)
				{
					// Detect if we are crossing a silhouette boundary.
					if ((getpixel(refSprite, x, y) != g_MaskColor && !inside) ||
						(getpixel(refSprite, x, y) == g_MaskColor && inside))
					{
						// Reset the depth counter
						depthCount = 0;
						inside = !inside;
					}
					// Increment the depth counter if inside silhouette,
					// and propose making atom if specified depth is reached.
					else if (inside)
					{
						++depthCount;
						if (depthCount == depth) {
							clear = true;
							// Check whether depth is sufficient in the other cardinal directions.
							for (i = 1; i <= depth && clear; ++i)
							{
								if (x - i < 0 ||
									y + i >= refSprite->h ||
									y - i < 0 ||
									getpixel(refSprite, x - i, y) == g_MaskColor ||
									getpixel(refSprite, x, y + i) == g_MaskColor ||
									getpixel(refSprite, x, y - i) == g_MaskColor)
									clear = false;
							}
							// Depth is cleared in all directions, so go ahead and place Atom.
							if (clear && getpixel(checkBitmap, x, y) == g_MaskColor)
							{
								// Mark that an atom has been put in this location, to avoid duplicate Atoms
								putpixel(checkBitmap, x, y, 99);
								AddAtomToGroup(x, y, spriteOffset, pOwnerMOSRotating, false);
							}
						}
					}
				}
			}
			// Scan VERTICALLY from TOP to BOTTOM,
			// and place Atoms in depth beyond the sihouette edge.
			for (x = 0; x < width; x += m_Resolution)
			{
				inside = false;
				for (y = 0; y < height; ++y)
				{
					// Detect if we are crossing a silhouette boundary.
					if ((getpixel(refSprite, x, y) != g_MaskColor && !inside) ||
						(getpixel(refSprite, x, y) == g_MaskColor && inside))
					{
						// Reset the depth counter
						depthCount = 0;
						inside = !inside;
					}
					// Increment the depth counter if inside silhouette,
					// and propose making atom if specified depth is reached.
					else if (inside)
					{
						++depthCount;
						if (depthCount == depth) {
							clear = true;
							// Check whether depth is sufficient in the other cardinal directions.
							for (i = 1; i <= depth && clear; ++i)
							{
								if (x + i >= refSprite->w ||
									x - i < 0 ||
									y + i >= refSprite->h ||
									getpixel(refSprite, x + i, y) == g_MaskColor ||
									getpixel(refSprite, x - i, y) == g_MaskColor ||
									getpixel(refSprite, x, y + i) == g_MaskColor)
									clear = false;
							}
							// Depth is cleared in all directions, so go ahead and place Atom.
							if (clear && getpixel(checkBitmap, x, y) == g_MaskColor)
							{
								// Mark that an atom has been put in this location, to avoid duplicate Atoms.
								putpixel(checkBitmap, x, y, 99);
								AddAtomToGroup(x, y, spriteOffset, pOwnerMOSRotating, false);
							}
						}
					}
				}
			}
			// Scan VERTICALLY from BOTTOM to TOP,
			// and place Atoms in depth beyond the sihouette edge.
			for (x = 0; x < width; x += m_Resolution)
			{
				inside = false;
				for (y = height - 1; y >= 0; --y)
				{
					// Detect if we are crossing a silhouette boundary.
					if ((getpixel(refSprite, x, y) != g_MaskColor && !inside) ||
						(getpixel(refSprite, x, y) == g_MaskColor && inside))
					{
						// Reset the depth counter
						depthCount = 0;
						inside = !inside;
					}
					// Increment the depth counter if inside silhouette,
					// and propose making atom if specified depth is reached.
					else if (inside)
					{
						++depthCount;
						if (depthCount == depth) {
							clear = true;
							// Check whether depth is sufficient in the other cardinal directions.
							for (i = 1; i <= depth && clear; ++i)
							{
								if (x + i >= refSprite->w ||
									x - i < 0 ||
									y - i < 0 ||
									getpixel(refSprite, x + i, y) == g_MaskColor ||
									getpixel(refSprite, x - i, y) == g_MaskColor ||
									getpixel(refSprite, x, y - i) == g_MaskColor)
									clear = false;
							}
							// Depth is cleared in all directions, so go ahead and place Atom.
							if (clear && getpixel(checkBitmap, x, y) == g_MaskColor)
							{
								// Mark that an atom has been put in this location, to avoid duplicate Atoms.
								putpixel(checkBitmap, x, y, 99);
								AddAtomToGroup(x, y, spriteOffset, pOwnerMOSRotating, false);
							}
						}
					}
				}
			}
		}

		release_bitmap(refSprite);
		release_bitmap(checkBitmap);
	}

    // If no atoms were made, just place a default one in the middle
    if (m_Atoms.empty())
    {
		AddAtomToGroup(width / 2, height / 2, spriteOffset, pOwnerMOSRotating, false);
    }

// TODO: Consider m_JointOffset!")

    destroy_bitmap(checkBitmap); checkBitmap = 0;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int AtomGroup::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "AutoGenerate")
        reader >> m_AutoGenerate;
	else if (propName == "Material")
	{
		Material mat;
		mat.Reset();
		reader >> mat;
		if (mat.GetIndex())
			m_pMaterial = g_SceneMan.GetMaterialFromID(mat.GetIndex());
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
		}
	}
    else if (propName == "Resolution")
        reader >> m_Resolution;
    else if (propName == "Depth")
        reader >> m_Depth;
    else if (propName == "AddAtom")
    {
        Atom *pAtom = new Atom;
        reader >> *pAtom;
        m_Atoms.push_back(pAtom);
    }
    else if (propName == "JointOffset")
        reader >> m_JointOffset;
    else
        return Entity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this AtomGroup with a Writer for
//                  later recreation with Create(Reader &reader);

int AtomGroup::Save(Writer &writer) const
{
    Entity::Save(writer);

    writer.NewProperty("AutoGenerate");
    writer << m_AutoGenerate;
    writer.NewProperty("Material");
    writer << m_pMaterial;
    writer.NewProperty("Resolution");
    writer << m_Resolution;
    writer.NewProperty("Depth");
    writer << m_Depth;
    // Only write out atoms if they were manually specified
//    if (!m_AutoGenerate)
//    {
        for (const Atom * atom : m_Atoms)
        {
            writer.NewProperty("AddAtom");
            writer << *atom;
        }
//    }
    writer.NewProperty("JointOffset");
    writer << m_JointOffset;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the AtomGroup object.

void AtomGroup::Destroy(bool notInherited)
{
	for (list<Atom *>::const_iterator itr = m_Atoms.begin(); itr != m_Atoms.end(); ++itr)
		delete *itr;

    if (!notInherited)
        Entity::Destroy();
    Clear();
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: AtomGroup addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for AtomGroup:s which adds together the
//                  Atoms of two Groups and merges them into one.

AtomGroup operator+(const AtomGroup &lhs, const AtomGroup &rhs)
{
    AtomGroup returnAG(lhs);
    returnAG.SetOwner(lhs.GetOwner());
    returnAG.AddAtoms(rhs.GetAtomList());
    return returnAG;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: AtomGroup pointer addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for AtomGroup:s pointer which adds together
//                  the Atoms of two Groups pointed to and merges them into one.
//                  Ownership of the returned dallocated AG IS TRANSFERRED!

AtomGroup * operator+(const AtomGroup *lhs, const AtomGroup *rhs)
{
    AtomGroup *pReturnAG = new AtomGroup(*lhs);
    pReturnAG->SetOwner(lhs->GetOwner());
    pReturnAG->AddAtoms(rhs->GetAtomList());
    return pReturnAG;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateMaxRadius
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the longest magnitude of all the atom's offsets.

float AtomGroup::CalculateMaxRadius() const
{
	float magnitude;
	float longest = 0.0F;

	for (const Atom *atom : m_Atoms)
    {
        magnitude = atom->GetOffset().GetMagnitude();
        if (magnitude > longest)
            longest = magnitude;
    }

    return longest;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMomentOfInertia
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current mass moment of inertia of this AtomGroup

float AtomGroup::GetMomentOfInertia()
{
    if (m_MomInertia == 0.0F)
    {
        if (!m_pOwnerMO)
        {
            RTEAbort("Getting AtomGroup stuff without a parent MO!");
            return 0.0F;
        }
        float distMass = m_pOwnerMO->GetMass() / static_cast<float>(m_Atoms.size());
        float radius = 0.0F;
        for (const Atom * atom : m_Atoms)
        {
            radius = atom->GetOffset().GetMagnitude() * c_MPP;
            m_MomInertia += distMass * radius * radius;
        }
    }

    // Avoid zero (if radius is nonexistent, for example), will cause divide by zero problems otherwise
    if (m_MomInertia == 0.0F)
        m_MomInertia = 0.000001F;

    return m_MomInertia;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOwner
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current owner MovableObject of this AtomGroup.

void AtomGroup::SetOwner(MOSRotating *newOwner)
{
    m_pOwnerMO = newOwner;
    for (Atom *atom : m_Atoms)
		atom->SetOwner(m_pOwnerMO);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddAtoms
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a list of new Atoms to the internal list that makes up this group.
//                  Ownership of all Atoms in the list IS NOT transferred!

void AtomGroup::AddAtoms(const std::list<Atom *> &atomList, long int subID, const Vector &offset, const Matrix &offsetRotation)
{
    Atom *pAtom;

    // Make a new list for the subgroup ID if there isn't one already
	if (m_SubGroups.count(subID) == 0)
	{
		m_SubGroups.insert(pair<long int, list<Atom *> >(subID, list<Atom *>()));
	}
    for (const Atom * atom : atomList)
    {
        pAtom = new Atom(*atom);
        pAtom->SetSubID(subID);
        pAtom->SetOffset(offset + (pAtom->GetOriginalOffset() * offsetRotation));
        pAtom->SetOwner(m_pOwnerMO);
        // Put ownership here
        m_Atoms.push_back(pAtom);

        // Add the atom to the subgroup in the SubGroups map, not transferring ownership
        m_SubGroups.at(subID).push_back(pAtom);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateSubAtoms
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the offsets of a subgroup of Atoms of this. This allows chunks
//                  representing sub parts of the whole to more closely represent the
//                  graphics better.

bool AtomGroup::UpdateSubAtoms(long int subID, const Vector &newOffset, const Matrix &newOffsetRotation)
{
	// Try to find existing subgroup with that ID to update
	if (m_SubGroups.count(subID) == 0)
	{
		return false;
	}
	RTEAssert(!m_SubGroups.at(subID).empty(), "Found empty atom subgroup list!?");

	for (Atom *subGroupAtom : m_SubGroups.at(subID))
	{
		subGroupAtom->SetSubID(subID); // Re-set ID just to make sure - TODO I don't think we need this?!
		subGroupAtom->SetOffset(newOffset + (subGroupAtom->GetOriginalOffset() * newOffsetRotation));
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveAtoms
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes all atoms of a specific subgroup ID from this AtomGroup.

bool AtomGroup::RemoveAtoms(long int removeID)
{
    bool removedAny = false;
    list<Atom *>::iterator eraseItr;

    for (list<Atom *>::iterator aItr = m_Atoms.begin(); aItr != m_Atoms.end();)
    {
        if ((*aItr)->GetSubID() == removeID)
        {
            // This hanky panky necessary to not invalidate the aItr iterator
            delete (*aItr);
            eraseItr = aItr;
            aItr++;
            m_Atoms.erase(eraseItr);
            removedAny = true;
        }
        else
            aItr++;
    }

    // Try to erase the group from the subgroup map
    m_SubGroups.erase(removeID);

    return removedAny;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddMOIDToIgnore
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a MOID that this Atom should ignore collisions with during its
//                  next travel sequence.

void AtomGroup::AddMOIDToIgnore(MOID ignore)
{
    /*for (list<Atom *>::iterator aItr = m_Atoms.begin(); aItr != m_Atoms.end(); ++aItr)
        (*aItr)->AddMOIDToIgnore(ignore);*/
	// m_IgnoreMOIDs is passed to every atom which belongs to this group to avoid messing with every single atom
	// when adding or removing ignored MOs
	m_IgnoreMOIDs.push_back(ignore);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearMOIDIgnoreList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clear the list of MOIDs that this Atom is set to ignore collisions
//                  with during its next travel sequence. This should be done each frame
//                  so that fresh MOIDs can be re-added. (MOIDs are only valid during a
//                  frame)

void AtomGroup::ClearMOIDIgnoreList()
{
    /*for (list<Atom *>::iterator aItr = m_Atoms.begin(); aItr != m_Atoms.end(); ++aItr)
        (*aItr)->ClearMOIDIgnoreList();*/
	// m_IgnoreMOIDs is passed to every atom which belongs to this group to avoid messing with every single atom
	// when adding or removing ignored MOs
	m_IgnoreMOIDs.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsIgnoringMOID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether this Atom is set to ignore collisions with a MO of a
///                 specific MOID.

bool AtomGroup::IsIgnoringMOID(MOID which)
{
    return (*(m_Atoms.begin()))->IsIgnoringMOID(which);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Travel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the group of Atoms travel together and react when terrain is
//                  hit. Effects are direcly applied to the owning MovableObject.

float AtomGroup::Travel(const float travelTime,
                        bool callOnBounce,
                        bool callOnSink,
                        bool scenePreLocked)
{
    if (!m_pOwnerMO)
    {
        RTEAbort("Travelling an AtomGroup without a parent MO!");
        return travelTime;
    }

    return Travel(m_pOwnerMO->m_Pos,
                  m_pOwnerMO->m_Vel,
                  m_pOwnerMO->m_Rotation,
                  m_pOwnerMO->m_AngularVel,
                  m_pOwnerMO->m_DidWrap,
                  m_pOwnerMO->m_TravelImpulse,
                  m_pOwnerMO->GetMass(),
                  travelTime,
                  callOnBounce,
                  callOnSink,
                  scenePreLocked);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Travel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the group of Atoms travel together and react when terrain is
//                  hit. Effects are applied to the passed in variables.

float AtomGroup::Travel(Vector &position,
                        Vector &velocity,
                        Matrix &rotation,
                        float &angVel,
                        bool &didWrap,
                        Vector &totalImpulse,
                        float mass,
                        float travelTime,
                        bool callOnBounce,
                        bool callOnSink,
                        bool scenePreLocked)
{
	// Establish the travel trajectory of the origin of the AG during the time frame

	// SEGMENT LOOP //////////////////////////////////////////////
	// loop through all segments between hits

		// Loop through the atoms

			// Establish each atom's added trajectory based on its offset and the MO's angular vel

			// Add the origin traj and added offset rotation traj for each atom, to get its total trajectory

			// Reset the Atoms and init them for a new straight segment, using the calculated traj. As a first step of segment,
			// make the start step for all atoms, the atoms themselves should check if they are already on another MO,
			// and if so, to remember to ignore that MO for the rest of this AG's travel.

		// Find out which atom's total individual trajectory is the longest of them all, meaning its velocity is the
		// greatest of all atoms in this group during this travel. Calculate and store that max velocity also
		// highestVel = ((longestTraj / PPM) / traveltime).

		// Loop through the atoms again

			// Calc and save the normalized velocity ratio of all atoms by dividing their traj length with the longest.
			// The fastest atom(s) thus gets a ratio of 1.0, and all others get something 1.0 > x >= 0.0

			// Somehow save each step ratio associated with its corresponding atom

		// STEP LOOP //////////////////////////////////////////////
		// Loop through all the steps that the longest atom traj has to take this AG travel

			// Loop through atoms

				// All atoms whose (progress += velRatio) >= std::ceil(prevProgress), take a step to their
                // next pixel locations and check for collisions. all others do nothing.

                    // If any collision, add atom to approprite collision list (MO or terrain), and if MO, to
                    // save the MOID hit in the MO hit map, also.

            // If collision lists are empty

                // step loop continues to next iteration

            // Else if MO hit list isn't empty

                // Calc the distributed mass for each MO hitting atom this step, by diving total mass with total
                // number of MO hitting atoms

                // For each MOID hit entry in the MO hit map

                    // Step back all atoms that previously took one during this step iteration

                    // calc the mass distribution denominator for the hit MO by taking the number of atoms hitting it,
                    // and adjusting for the atom resolution/density rating.

                    // For each atom hitting this specific MO

                        // tell the atom to calculate the exact hit point and bitmap normal
                        // at that point on the MO that it's hitting

                        // Calculate the atom's velocity = highestVel * thisatom.velRatio

                        // tell the atom to then call CollideAtPoint on the MO that it hit,
                        // passing in hit point, bitmap normal, atom's velocity, the distributed mass of the atom,
                        // and the mass denominator of the hit MO.

                        // store the returned resulting impulse force vector in the list of impulse forces and their offsets 

            // Else if terrain hit list isn't empty

                // first determine which atoms will penetrate the terrain, by passing in their kinetic forces calculated with
                // their distributed masses and individial velocities (highestVel * atom.velRatio). keep looping if not all
                // terrain hitting atoms penetrate, and keep increasing the distributed mass on them

                // Do terrain bounce, if not all terrain hitting atoms could penetrate, even when all mass was on them.

                    // Back up all atoms one step that previously took one during this step iteration

                // else do terrain sink stuff, all Atoms who peeked steps, take them now.

            // Apply all collision responses to the state of the travelling MO, and start on the next segment iteration.

    if (!m_pOwnerMO)
    {
        RTEAbort("Travelling an AtomGroup without a parent MO!");
        return travelTime;
    }
    m_MomInertia = GetMomentOfInertia();

	int segCount = 0;
	int stepCount = 0;
	int stepsOnSeg = 0;
	int hitCount = 0;
	int atomsHitMOsCount = 0;
	float rotDelta;
	float segProgress;
	float timeLeft = travelTime;
	float retardation;
	float segRatio;
	float preHitRot;
	bool hitStep;
	bool newDir;
	bool halted = false;
	bool hitMOs = m_pOwnerMO->m_HitsMOs;
    map<MOID, list<Atom *> > hitMOAtoms;
    list<Atom *> hitTerrAtoms;
    list<Atom *> penetratingAtoms;
    list<Atom *> hitResponseAtoms;
	Vector linSegTraj; //!< The planned travelling distance of this AtomGroup's origin in pixels.
	Vector atomTraj;
	Vector preHitPos;
	Vector hitNormal;
    HitData hitData;

// TODO: Make this dependent on agroup radius!, not hcoded")
	const float segRotLimit = c_PI / 6.0F;

    didWrap = false;
    newDir = true;

    // Lock all bitmaps involved outside the loop.
    if (!scenePreLocked)
        g_SceneMan.LockScene();

	// Loop for all the different straight segments (between bounces etc) that
	// have to be traveled during the travelTime.
    do
    {
		// First see what atoms are inside either the terrain or another MO, and cause collisions responses before even starting the segment
        for (Atom *atom : m_Atoms)
        {
			const Vector startOff = m_pOwnerMO->RotateOffset(atom->GetOffset());

            if (atom->SetupPos(position + startOff))
            {
                hitData.Reset();
                if (atom->IsIgnoringTerrain())
                {
					// Calc and store the accurate hit radius of the Atom in relation to the CoM.
					hitData.HitRadius[HITOR] = startOff * c_MPP;
                    // Figure out the pre-collision velocity of the hitting atom due to body translation and rotation.
					hitData.HitVel[HITOR] = velocity + hitData.HitRadius[HITOR].GetPerpendicular() * angVel;
					/*
					radMag = hitData.HitRadius[HITOR].GetMagnitude();
					// These are set temporarily here, will be re-set later when the normal of the hit terrain bitmap (ortho pixel side) is known.
					hitData.HitDenominator = (1.0 / massDistribution) + ((radMag * radMag) / momentInertiaDistribution);
					hitData.PreImpulse[HITOR] = hitData.HitVel[HITOR] / hitData.HitDenominator;
					// Set the atom with the hit data with all the info we have so far.
					(*aItr)->SetHitData(hitData);

					hitFactor = 1.0;//  / (float)hitTerrAtoms.size();
					(*atom)->GetHitData().mass[HITOR] = mass;
					(*atom)->GetHitData().MomInertia[HITOR] = m_MomInertia;
					(*atom)->GetHitData().ImpulseFactor[HITOR] = hitFactor;

					// Call the call-on-bounce function, if requested.
//                    if (m_pOwnerMO && callOnBounce)
//                        halted = halted || m_pOwnerMO->OnBounce((*atom)->GetHitData());

					// Compute and store this Atom's collision response impulse force.
					// Calc effects of moment of inertia will have on the impulse.
					float MIhandle = m_LastHit.HitRadius[HITOR].GetPerpendicular().Dot(m_LastHit.BitmapNormal);
*/
                    if (!(atom->GetNormal().IsZero()))
                    {
                        hitData.ResImpulse[HITOR] = m_pOwnerMO->RotateOffset(atom->GetNormal());
                        hitData.ResImpulse[HITOR] = -hitData.ResImpulse[HITOR];
						hitData.ResImpulse[HITOR].SetMagnitude(hitData.HitVel[HITOR].GetMagnitude());

                        // Apply terrain conflict response
                        velocity += hitData.ResImpulse[HITOR] / mass;
						angVel += hitData.HitRadius[HITOR].GetPerpendicular().Dot(hitData.ResImpulse[HITOR]) / m_MomInertia;
                        // Accumulate all the impulse forces so the MO can determine if it took damaged as a result
                        totalImpulse += hitData.ResImpulse[HITOR];
                    }
// Dangerous, will cause oscillations
//                    else
//                        hitData.ResImpulse[HITOR] = -hitData.HitVel[HITOR];
                }
            }
#ifdef DEBUG_BUILD
            // Draw the positions of the atoms at the start of each segment, for visual debugging.
            putpixel(g_SceneMan.GetMOColorBitmap(), atom->GetCurrentPos().GetIntX(), atom->GetCurrentPos().GetIntY(), 122);
#endif
        }

        linSegTraj = velocity * timeLeft * c_PPM;

        // The amount of rotation to be achieved during the time slot, in radians
        rotDelta = angVel * timeLeft;

        // Cap the segment if the roation is too severe
        // This will chunk the segment into several in order to more closely approximate the arc
        // an atom on a rotating body will trace.
        if (fabs(rotDelta) > segRotLimit)
        {
            segRatio = segRotLimit / fabs(rotDelta);
            rotDelta = rotDelta > 0 ? segRotLimit : -segRotLimit;
            linSegTraj *= segRatio;
        }
        else
            segRatio = 1.0F;

        segProgress = 0.0F;
        hitStep = false;
        atomsHitMOsCount = 0;

        if (linSegTraj.IsZero() && rotDelta == 0)
            break;

        hitMOAtoms.clear();
        hitTerrAtoms.clear();
        penetratingAtoms.clear();
        hitData.Reset();
        hitResponseAtoms.clear();

        for (Atom *atom : m_Atoms)
        {
            // Calc the segment trajectory for each individual Atom, with rotations considered.
			const Vector startOff = m_pOwnerMO->RotateOffset(atom->GetOffset());

			const Vector trajFromAngularTravel = Vector(startOff).RadRotate(rotDelta) - startOff;

            // Set up the inital rasterized step for each Atom and save the longest trajectory
            if (atom->SetupSeg(position + startOff, linSegTraj + trajFromAngularTravel) > stepsOnSeg)
            {
                stepsOnSeg = atom->GetStepsLeft();
            }
        }

        // If the longest seg is 0 steps, there is no movement or rotation, so quit.
//        if (stepsOnSeg == 0)
//            break;

		for (Atom *atom : m_Atoms) {
			atom->SetStepRatio(static_cast<float>(atom->GetStepsLeft()) / static_cast<float>(stepsOnSeg));
		}


        // STEP LOOP //////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////
        for (stepCount = 0; !hitStep && stepsOnSeg != 0 && stepCount < stepsOnSeg; ++stepCount)
        {

            // SCENE COLLISION DETECTION //////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////////////////////

            for (Atom *atom : m_Atoms)
            {
                // Take one step, and check if the atom hit anything
                if (atom->StepForward())
                {
                    //  So something was hit, first check for terrain hit.
                    if (atom->HitWhatTerrMaterial())
                    {
						m_pOwnerMO->SetHitWhatTerrMaterial(atom->HitWhatTerrMaterial());
                        hitTerrAtoms.push_back(atom);
                    }

					// MO hits?
					if (hitMOs) {
						const MOID tempMOID = atom->HitWhatMOID();
						if (tempMOID != g_NoMOID)
						{
							m_pOwnerMO->m_MOIDHit = tempMOID;

							MovableObject *pMO = g_MovableMan.GetMOFromID(tempMOID);
							if (pMO && pMO->HitWhatMOID() == g_NoMOID) {
								pMO->SetHitWhatMOID(m_pOwnerMO->m_MOID);
							}

							// Yes, MO hit. See if we already have another atom hitting this MO in this step.
							// If not, then create a new deque unique for that MO's ID
							// and insert into the map of MO-hitting Atoms.
							if (!(hitMOAtoms.count(tempMOID)))
							{
								list<Atom *> newDeque;
								newDeque.push_back(atom);
								hitMOAtoms.insert(pair<MOID, list<Atom *> >(tempMOID, newDeque));
							}
							// If another Atom of this group has already hit this same MO
							// during this step, go ahead and add the new atom to the
							// corresponding deque for that MOID.
							else {
								hitMOAtoms.at(tempMOID).push_back(atom);
							}

							// Add the hit MO to the ignore list of ignored MOIDs
	//                        AddMOIDToIgnore(tempMOID);

							// Count the number of Atoms of this group that hit MO:s this step.
							// Used to properly distribute the mass of the owner MO in later
							// collision responses during this step.
							atomsHitMOsCount++;
						}
					}
//                  else
//                      RTEAbort("Atom reported hit to AtomGroup, but then reported neither MO or Terr hit!");

#ifdef DEBUG_BUILD
                    Vector tPos = atom->GetCurrentPos();
                    Vector tNorm = m_pOwnerMO->RotateOffset(atom->GetNormal()) * 7;
                    line(g_SceneMan.GetMOColorBitmap(), tPos.GetFloorIntX(), tPos.GetFloorIntY(), tPos.GetFloorIntX() + tNorm.GetFloorIntX(), tPos.GetFloorIntY() + tNorm.GetFloorIntY(), 244);
                    // Draw the positions of the hitpoints on screen for easy debugging.
//                    putpixel(g_SceneMan.GetMOColorBitmap(), tPos.GetFloorIntX(), tPos.GetFloorIntY(), 5);
#endif
                }
            }

            // If no collisions, continue on to the next step.
            if (hitTerrAtoms.empty() && hitMOAtoms.empty())
                continue;

			// There are colliding Atoms, therefore the group hit something.
            hitStep = true;
            ++hitCount;

			// Calculate the progress made on this segment before hitting something.
			// Special case of being at rest
			if (stepCount == 0 && stepsOnSeg == 1)
			{
				halted = true;
				//                m_pOwnerMO->SetToSettle(true);
			}
				segProgress = static_cast<float>(stepCount) / static_cast<float>(stepsOnSeg);

            // Move position forward to the hit position.
            preHitPos = position;
            position += linSegTraj * segProgress;
            // Wrap position, if necessary.
            didWrap = g_SceneMan.WrapPosition(position) || didWrap;
            // Move rotation forward according to the progress made on the segment.
            preHitRot = rotation.GetRadAngle();
            rotation += rotDelta * segProgress;

            // TERRAIN COLLISION RESPONSE /////////////////////////////////////////////////////
			// Determine which of the colliding Atoms will penetrate the terrain.
			bool somethingPenetrated = false;
			do
            {
				somethingPenetrated = false;

				const float massDistribution = mass / static_cast<float>(hitTerrAtoms.size() * (m_Resolution ? m_Resolution : 1));
				const float momentInertiaDistribution = m_MomInertia / static_cast<float>(hitTerrAtoms.size() * (m_Resolution ? m_Resolution : 1));

				for (std::list<Atom*>::iterator aItr = hitTerrAtoms.begin(); aItr != hitTerrAtoms.end(); )
                {
					// Calc and store the accurate hit radius of the Atom in relation to the CoM
					hitData.HitRadius[HITOR] = m_pOwnerMO->RotateOffset((*aItr)->GetOffset()) * c_MPP;
					// Figure out the pre-collision velocity of the hitting atom due to body translation and rotation.
					hitData.HitVel[HITOR] = velocity + hitData.HitRadius[HITOR].GetPerpendicular() * angVel;

					const float radMag = hitData.HitRadius[HITOR].GetMagnitude();
					// These are set temporarily here, will be re-set later when the normal of the hit terrain bitmap (ortho pixel side) is known.
					hitData.HitDenominator = (1.0F / massDistribution) + ((radMag * radMag) / momentInertiaDistribution);
					hitData.PreImpulse[HITOR] = hitData.HitVel[HITOR] / hitData.HitDenominator;
					// Set the atom with the hit data with all the info we have so far.
					(*aItr)->SetHitData(hitData);

					if (g_SceneMan.WillPenetrate((*aItr)->GetCurrentPos().GetFloorIntX(), (*aItr)->GetCurrentPos().GetFloorIntY(), hitData.PreImpulse[HITOR]))
					{
						// Move the penetrating atom to the pen. list from the coll. list.
						penetratingAtoms.push_back(*aItr);
						aItr = hitTerrAtoms.erase(aItr);
						somethingPenetrated = true;
					} else
						++aItr;
                }
            }
			while (!hitTerrAtoms.empty() && somethingPenetrated);

            // TERRAIN BOUNCE //////////////////////////////////////////////////////////////////
            // If some Atoms could not penetrate even though all the impulse was on them,
            // gather the bounce results and apply them to the owner.
            if (!hitTerrAtoms.empty())
            {
                newDir = true;

                // Step back all atoms that previously took one during this step iteration.
                // This is so we aren't intersecting the hit MO anymore.
				for (Atom *hitTerrAtom : hitTerrAtoms) {
					hitTerrAtom->StepBack();
				}

                // Calculate the distributed mass that each bouncing Atom has.
//                massDistribution = mass /*/ (hitTerrAtoms.size() * (m_Resolution ? m_Resolution : 1))*/;
//                momentInertiaDistribution = m_MomInertia/* / (hitTerrAtoms.size() * (m_Resolution ? m_Resolution : 1))*/;
				const float hitFactor = 1.0F / static_cast<float>(hitTerrAtoms.size());

                // Gather the collision response effects so that the impulse force can be calculated.
                for (Atom *hitTerrAtom : hitTerrAtoms)
                {
                    hitTerrAtom->GetHitData().TotalMass[HITOR] = mass;
                    hitTerrAtom->GetHitData().MomInertia[HITOR] = m_MomInertia;
                    hitTerrAtom->GetHitData().ImpulseFactor[HITOR] = hitFactor;

                    // Get the hitdata so far gathered for this Atom.
//                  hitData = (*hitTerrAtom)->GetHitData();

                    // Call the call-on-bounce function, if requested.
                    if (m_pOwnerMO && callOnBounce)
                        halted = halted || m_pOwnerMO->OnBounce(hitTerrAtom->GetHitData());

                    // Copy back the new hit data with all the info we have so far.
//                  (*hitTerrAtom)->SetHitData(hitData);

                    // Compute and store this Atom's collision response impulse force.
                    hitTerrAtom->TerrHitResponse();
                    hitResponseAtoms.push_back(hitTerrAtom);
                }
            }
            // TERRAIN SINK ////////////////////////////////////////////////////////////////
			// Handle terrain penetration effects.
            if (!penetratingAtoms.empty()) {
				const float hitFactor = 1.0F / static_cast<float>(penetratingAtoms.size());

                // Calc and store the collision response effects.
                for (Atom *penetratingAtom : penetratingAtoms)
                {
                    // This gets re-set later according to the ortho pixel edges hit.
//                  hitData.BitmapNormal = -(hitData.HitVel[HITOR].GetNormalized());
//                  hitData.SquaredMIHandle[HITOR] = hitData.HitRadius[HITOR].GetPerpendicular()/*.Dot(hitData.BitmapNormal)*/;
//                  hitData.SquaredMIHandle[HITOR] *= hitData.SquaredMIHandle[HITOR];
//                  hitData.HitDenominator = (1.0 / massDistribution) + (hitData.SquaredMIHandle[HITOR] / momentInertiaDistribution);
//                  hitData.PreImpulse[HITOR] = hitData.HitVel[HITOR] / hitData.HitDenominator;

                    // Get the hitdata so far gathered for this Atom.
                    hitData = penetratingAtom->GetHitData();

                    if (g_SceneMan.TryPenetrate(penetratingAtom->GetCurrentPos().GetFloorIntX(),
                                                penetratingAtom->GetCurrentPos().GetFloorIntY(),
                                                hitData.PreImpulse[HITOR],
                                                hitData.HitVel[HITOR],
                                                retardation,
                                                1.0F,
                                                1/*(*penetratingAtom)->GetNumPenetrations()*/))
                    {

                        // Recalc these here without the distributed mass and MI.
						const float radMag = hitData.HitRadius[HITOR].GetMagnitude();
						hitData.HitDenominator = (1.0F / mass) + ((radMag * radMag) / m_MomInertia);
						hitData.PreImpulse[HITOR] = hitData.HitVel[HITOR] / hitData.HitDenominator;
                        hitData.TotalMass[HITOR] = mass;
                        hitData.MomInertia[HITOR] = m_MomInertia;
                        hitData.ImpulseFactor[HITOR] = hitFactor;
                        // Finally calculate the hit response impulse.
						hitData.ResImpulse[HITOR] = ((hitData.HitVel[HITOR] * retardation) /
							hitData.HitDenominator) * hitFactor;

                        // Call the call-on-sink function, if requested.
                        if (callOnSink)
                            halted = halted || m_pOwnerMO->OnSink(hitData);

                        // Copy back the new hit data with all the info we have so far.
                        penetratingAtom->SetHitData(hitData);
                        // Save the atom for later application of its hit data to the body.
                        hitResponseAtoms.push_back(penetratingAtom);
                    }
                }
            }

            // MOVABLEOBJECT COLLISION RESPONSE ///////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////////////////////
            if (hitMOs && !hitMOAtoms.empty())
            {
                newDir = true;

				// Set the mass and other data pertaining to the hitor,
				// aka this AtomGroup's owner MO.
				hitData.TotalMass[HITOR] = mass;
				hitData.MomInertia[HITOR] = m_MomInertia;
				hitData.ImpulseFactor[HITOR] = 1.0F / static_cast<float>(atomsHitMOsCount);

				for (const map<MOID, std::list<Atom *>>::value_type &MOAtomMapEntry : hitMOAtoms)
                {
					// The denominator that the MovableObject being hit should
                    // divide its mass with for each atom of this AtomGroup that is
                    // colliding with it during this step.
                    hitData.ImpulseFactor[HITEE] = 1.0F / static_cast<float>(MOAtomMapEntry.second.size());

                    for (Atom *hitMOAtom : MOAtomMapEntry.second)
                    {
						// Step back all atoms that hit MOs during this step iteration.
						// This is so we aren't intersecting the hit MO anymore.
						hitMOAtom->StepBack();

//                      hitData.HitPoint = aItr->GetCurrentPos();
                        // Calc and store the accurate hit radius of the Atom in relation to the CoM
						hitData.HitRadius[HITOR] = m_pOwnerMO->RotateOffset(hitMOAtom->GetOffset()) * c_MPP;
                        // Figure out the pre-collision velocity of the hitting atom due to body translation and rotation.
						hitData.HitVel[HITOR] = velocity + hitData.HitRadius[HITOR].GetPerpendicular() * angVel;
                        // Set the atom with the hit data with all the info we have so far.
                        hitMOAtom->SetHitData(hitData);
                        // Let the atom calc the impulse force resulting from the collision, and only add it if collision is valid
                        if (hitMOAtom->MOHitResponse())
                        {
                            // Report the hit to both MO's in collision
                            HitData &hd = hitMOAtom->GetHitData();
                            // Don't count collision if either says they got terminated
                            if (!hd.RootBody[HITOR]->OnMOHit(hd) && !hd.RootBody[HITEE]->OnMOHit(hd))
                            {
                                // Save the filled out atom in the list for later application in this step.
                                hitResponseAtoms.push_back(hitMOAtom);
                            }
                        }
                    }
                }
            }
        }
        ++segCount;

        // APPLY COLLISION RESPONSES ///////////////////////////////////////////////////
        // If we hit anything, during this almost completed segment, and are about to
        // start a new one, apply the calculated response effects to the owning MO.
        if (hitStep)
        {
            // Apply all the collision response impulse forces to the
            // linear- and angular velocities of the owner MO.
            for (Atom *hitResponseAtom : hitResponseAtoms)
            {
// TODO: Investigate damping!")
// TODO: Clean up here!#$#$#$#")
                hitData = hitResponseAtom->GetHitData();
//                  tempVec = hitData.ResImpulse[HITOR];
                velocity += hitData.ResImpulse[HITOR] / mass;
				angVel += hitData.HitRadius[HITOR].GetPerpendicular().Dot(hitData.ResImpulse[HITOR]) / m_MomInertia;
                // Accumulate all the impulse forces so the MO can determine if it took damaged as a result
                totalImpulse += hitData.ResImpulse[HITOR];
            }

            // Make subpixel progress if there was a hit on the very first step.
//                if (segProgress == 0)
//                  segProgress = 0.1 / (float)stepsOnSeg;

            // Now calculate the total time left to travel, according to the progress made.
            timeLeft -= timeLeft * (segProgress * segRatio);
        }
        // If last completed segment didn't result in a hit,
        // move things forward to reflect the progress made.
        else
        {
            // Full progress was made on this segment without hitting anything.
            segProgress = 1.0F;
            // Move position forward to the end segment position.
			position += linSegTraj * segProgress;
            // Wrap position, if necessary.
            didWrap = g_SceneMan.WrapPosition(position) || didWrap;
            // Move rotation forward according to the progress made on the segment.
			rotation += rotDelta * segProgress;
            // Now calculate the total time left to travel, according to the progress made.
			timeLeft -= timeLeft * (segProgress * segRatio);
        }

        if (hitCount > 10)
        {
//            RTEAbort("AtomGroup travel resulted in more than 1000 segs!!");
            break;
        }
    }
	while (segRatio != 1.0F || hitStep && /*!linSegTraj.GetFloored().IsZero() &&*/ !halted);

    // Resolove all MO intersections!
    ResolveMOSIntersection(position, rotation);

    // Unlock the scene
    if (!scenePreLocked)
        g_SceneMan.UnlockScene();

    // Clear out the MOIDs from the ignore list.
    ClearMOIDIgnoreList();

	// If too many Atoms are ignoring terrain, make a hole for the body so they won't
    int ignoreCount = 0;
    int maxIgnore = m_Atoms.size() / 2;
	for (const Atom *atom : m_Atoms)
	{
		if (atom->IsIgnoringTerrain()) {
			++ignoreCount;
			if (ignoreCount >= maxIgnore)
			{
				m_pOwnerMO->ForceDeepCheck();
				break;
			}
		}
    }

    // Travel along the remaining trajectory if we didn't
	// hit anything on the last segment and weren't told to halt.
    if (!hitStep && !halted)
    {
/*
        // Move position forward the whole way.
        position += linSegTraj;
        // Wrap, if necessary.
        didWrap = g_SceneMan.WrapPosition(position) || didWrap;
        // Move rotation forward the whole way.
        rotation += rotDelta;
*/
        return 0;
    }
    return timeLeft;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PushTravel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the group of Atoms travel together and react when terrain is
//                  hit. Effects are applied to the passed in variables. This is the
//                  version that doesn't do rotations, only translational effect.

Vector AtomGroup::PushTravel(Vector &position,
                             Vector velocity,
                             float pushForce,
                             bool &didWrap,
                             float travelTime,
                             bool callOnBounce,
                             bool callOnSink,
                             bool scenePreLocked)
{
    if (!m_pOwnerMO)
    {
        RTEAbort("Travelling an AtomGroup without a parent MO!");
        return Vector();
    }

    bool hFlipped = m_pOwnerMO->m_HFlipped;

    Vector trajectory = velocity * travelTime * c_PPM; //!< Trajectory length in pixels.

	int legCount = 0;
	int stepCount = 0;
	int hitCount = 0;
	int atomsHitMOsCount = 0;
	int intPos[2];
	int hitPos[2];
	int delta[2];
	int delta2[2];
	int increment[2];
	int error;
	int prevError;
	int dom;
	int sub;
	int domSteps;
	int subSteps;
	float prevVelMag;
	float prevTrajMag;
	float prevTimeLeft;
	float mass = m_pOwnerMO->GetMass();
	float massDist;
	float timeLeft = travelTime;
	float retardation;
// TODO: Fix HitMOs issue!!")
	bool hit[2];
	bool subStepped;
	bool newDir;
	bool halted = false;
	bool hitMOs = false	/*m_pOwnerMO->m_HitsMOs*/;
	bool sinkHit = false;
	unsigned char hitMaterialID;
	unsigned char domMaterialID;
	unsigned char subMaterialID;
	Material const * hitMaterial = g_SceneMan.GetMaterialFromID(g_MaterialAir);
	Material const * domMaterial = g_SceneMan.GetMaterialFromID(g_MaterialAir);
	Material const * subMaterial = g_SceneMan.GetMaterialFromID(g_MaterialAir);
    map<MOID, set<Atom *> >MOIgnoreMap;
    map<MOID, deque<pair<Atom *, Vector> > > hitMOAtoms;
    deque<pair<Atom *, Vector> > hitTerrAtoms;
    deque<pair<Atom *, Vector> > penetratingAtoms;
    // First Vector is the impulse force in kg * m/s, the second is force point,
    // or its offset from the origin of the AtomGroup.
    deque<pair<Vector, Vector> > impulseForces;
	Vector legProgress;
	Vector forceVel;
	Vector returnPush;
    MOID tempMOID = g_NoMOID;
    HitData hitData;

    // Init the raw positions.
    intPos[X] = position.GetFloorIntX();
    intPos[Y] = position.GetFloorIntY();

    didWrap = false;
    newDir = true;

    // Lock all bitmaps involved outside the loop.
    if (!scenePreLocked)
        g_SceneMan.LockScene();

    // Before the very first step of the first leg of this travel, we find
    // that we're already intersecting with another MO, then we completely
    // ignore collisions with that MO for this entire travel. This is to
    // prevent MO's from getting stuck in each other.
    if (hitMOs)
    {
        for (Atom *atom : m_Atoms)
        {
			const Vector flippedOffset = atom->GetOffset().GetXFlipped(hFlipped);
            // See if the atom is starting out on top of another MO
			tempMOID = g_SceneMan.GetMOIDPixel(intPos[X] + flippedOffset.GetFloorIntX(), intPos[Y] + flippedOffset.GetFloorIntY());
            if (g_NoMOID != (tempMOID))
            {
/* Can't implement because we cant get the generating MO reliably.
Solved issue by instead making any generator to update their generated MO's once themselves
before adding them to the MovableMan.
                // If we find that we are still intersecting with the MO that generated the
                // owner MO of this AtomGroup, then make a special note of that so that collisions
                // with that MO can be ignored in all steps up till the first step that none of
                // the atoms are hitting that MO anymore, ie when the owner MO of this AtomGroup
                // has fully separated with the MO that generated (fired, launched, squirted) it.
                if (m_pOwnerMO->m_GeneratingMO && tempMOID == m_pOwnerMO->GetID())
                    leftOwner = false;
*/
                // Make the appropriate entry in the MO-Atom interaction ignore map
				if (MOIgnoreMap.count(tempMOID) != 0) {
					// Found an entry for this MOID, so add the Atom entry to it
					MOIgnoreMap.at(tempMOID).insert(atom);
				}
                // There wasnt already an entry for this MOID,
                // so create one and add the Atom to it.
                else {
                    set<Atom *> newSet;
                    newSet.insert(atom);
                    MOIgnoreMap.insert(pair<MOID, set<Atom *> >(tempMOID, newSet));
                }
            }
        }
    }

    // Loop for all the different straight legs (between bounces etc) that
    // have to be traveled during the travelTime.
    do {
        intPos[X] = position.GetFloorIntX();
        intPos[Y] = position.GetFloorIntY();

        prevVelMag = velocity.GetMagnitude();
        prevTrajMag = trajectory.GetMagnitude();
        trajectory = velocity * timeLeft * c_PPM;

		const Vector nextPosition = position + trajectory;
        delta[X] = nextPosition.GetFloorIntX() - intPos[X];
        delta[Y] = nextPosition.GetFloorIntY() - intPos[Y];

        hit[X] = false;
        hit[Y] = false;
        sinkHit = false;
        domSteps = 0;
        subSteps = 0;
        subStepped = false;
        atomsHitMOsCount = 0;

		if (delta[X] == 0 && delta[Y] == 0)
			break;

		hitMaterialID = 0;
		domMaterialID = 0;
		subMaterialID = 0;
		hitMaterial = 0;// .Reset();
		domMaterial = 0;// .Reset();
		subMaterial = 0;// .Reset();
        hitTerrAtoms.clear();
        penetratingAtoms.clear();
        impulseForces.clear();

        /////////////////////////////////////////////////////
        // Bresenham's line drawing algorithm preparation

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
		delta2[X] = delta[X] * 2;
		delta2[Y] = delta[Y] * 2;

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

        if (newDir)
            error = delta2[sub] - delta[dom];
        else
            error = prevError;

        // Sanity check
        if (delta[X] > 1000)
            delta[X] = 1000;
        if (delta[Y] > 1000)
            delta[Y] = 1000;

        /////////////////////////////////////////////////////
        // Bresenham's line drawing algorithm execution

        for (domSteps = 0; domSteps < delta[dom] && !(hit[X] || hit[Y]); ++domSteps)
        {
            if (subStepped)
                ++subSteps;
            subStepped = false;

            // Take one step forward along the leg.
            intPos[dom] += increment[dom];
            if (error >= 0)
            {
                intPos[sub] += increment[sub];
                subStepped = true;
                error -= delta2[dom];
            }
            error += delta2[sub];

            // Wrap, if necessary.
            didWrap = g_SceneMan.WrapPosition(intPos[X], intPos[Y]) || didWrap;

            // SCENE COLLISION DETECTION //////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////////////////////

            hitMOAtoms.clear();
            hitTerrAtoms.clear();

            for (Atom *atom : m_Atoms)
            {
				const Vector flippedOffset = atom->GetOffset().GetXFlipped(hFlipped);

                // First check if we hit any MO's, if applicable.
                bool ignoreHit = false;
                if (hitMOs)
                {
                    tempMOID = g_SceneMan.GetMOIDPixel(intPos[X] + flippedOffset.GetFloorIntX(), intPos[Y] + flippedOffset.GetFloorIntY());

                    // Check the ignore map for Atoms that should ignore hits against certain MOs.
					if (tempMOID != g_NoMOID && (MOIgnoreMap.count(tempMOID) != 0)) {
						ignoreHit = (MOIgnoreMap.at(tempMOID).count(atom) != 0);
					}
                }

                if (hitMOs && tempMOID && !ignoreHit)
                {
                    // See if we already have another atom hitting this MO in this step.
                    // If not, then create a new deque unique for that MO's ID
                    // and insert into the map of MO-hitting Atoms.
                    if (hitMOAtoms.count(tempMOID) == 0)
                    {
                        deque<pair<Atom *, Vector> > newDeque;
                        newDeque.push_back(pair<Atom *, Vector>(atom, flippedOffset));
                        hitMOAtoms.insert(pair<MOID, deque<pair<Atom *, Vector> > >(tempMOID, newDeque));
                    }
                    // If another Atom of this group has already hit this same MO
                    // during this step, go ahead and add the new atom to the
                    // corresponding deque for that MOID.
                    else
                        (hitMOAtoms.at(tempMOID)).push_back(pair<Atom *, Vector>(atom, flippedOffset));
                    // Count the number of Atoms of this group that hit MO:s this step.
                    // Used to properly distribute the mass of the owner MO in later
                    // collision responses during this step.
                    atomsHitMOsCount++;
                }
                // If no MO has ever been hit yet during this step, then keep checking for terrain hits.
                else if (atomsHitMOsCount == 0 && g_SceneMan.GetTerrMatter(intPos[X] + flippedOffset.GetFloorIntX(), intPos[Y] + flippedOffset.GetFloorIntY()))
                    hitTerrAtoms.push_back(pair<Atom *, Vector>(atom, flippedOffset));
/*
#ifdef DEBUG_BUILD
                // Draw the positions of the hitpoints on screen for easy debugging.
                putpixel(g_SceneMan.GetMOColorBitmap(), std::floor(position.m_X + flippedOffset.m_X), std::floor(position.m_Y + flippedOffset.m_Y), 122);
#endif
*/
            }

            // If no collisions, continue on to the next step.
            if (hitTerrAtoms.empty() && hitMOAtoms.empty())
                continue;

			// There are colliding Atoms, therefore the group hit something.
            hitPos[X] = intPos[X];
            hitPos[Y] = intPos[Y];
            ++hitCount;
            // Calculate the progress made on this leg before hitting something.
            legProgress[dom] = static_cast<float>(domSteps * increment[dom]);
            legProgress[sub] = static_cast<float>(subSteps * increment[sub]);
            // Now calculate the total time left to travel, according to the progress made.
            prevTimeLeft = timeLeft;
            timeLeft *= (trajectory.GetMagnitude() - legProgress.GetMagnitude()) / prevTrajMag;
            // The capped velocity used for the push calculations. a = F / m
            forceVel = Vector(velocity).CapMagnitude((pushForce * timeLeft) / mass);

            // MOVABLEOBJECT COLLISION RESPONSE ///////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////////////////////

            if (hitMOs && !hitMOAtoms.empty())
            {
                newDir = true;
                prevError = error;
                MovableObject *pHiteeMO = 0;

                // Back up one step so that we're not intersecting the other MO(s) anymore
                intPos[dom] -= increment[dom];
                if (subStepped)
                    intPos[sub] -= increment[sub];
                // Undo wrap, if necessary.
                didWrap = !g_SceneMan.WrapPosition(intPos[X], intPos[Y]) && didWrap;

                // Set the mass and other data pertaining to the hitor,
                // aka this AtomGroup's owner MO.
                hitData.TotalMass[HITOR] = mass;
                hitData.MomInertia[HITOR] = 1.0F;
                hitData.ImpulseFactor[HITOR] = 1.0F / static_cast<float>(atomsHitMOsCount);
                // Figure out the pre-collision velocity of the
                // hitting atoms due to the max push force allowed.
                hitData.HitVel[HITOR] = forceVel;

                // The distributed mass of one hitting atom of the
                // hitting (this AtomGroup's owner) MovableObject.
//                float hitorMass = mass / ((atomsHitMOsCount/* + hitTerrAtoms.size()*/) *
//                                       (m_Resolution ? m_Resolution : 1));
//                float hiteeMassDenom = 0;

                for (const map<MOID,deque<pair<Atom*,Vector>>>::value_type &MOAtomMapEntry : hitMOAtoms)
                {
                    // The denominator that the MovableObject being hit should
                    // divide its mass with for each atom of this AtomGroup that is
                    // colliding with it during this step.
                    hitData.ImpulseFactor[HITEE] = 1.0F / static_cast<float>(MOAtomMapEntry.second.size());

                    for (const pair<Atom *, Vector> &hitMOAtomEntry : MOAtomMapEntry.second)
                    {
                        // Bake in current Atom's offset into the int positions.
						const Vector &atomOffset = hitMOAtomEntry.second;
                        intPos[X] += atomOffset.GetFloorIntX();
                        intPos[Y] += atomOffset.GetFloorIntY();
                        hitPos[X] += atomOffset.GetFloorIntX();
                        hitPos[Y] += atomOffset.GetFloorIntY();

//                      hitData.HitPoint.SetXY(intPos[X], intPos[Y]);
                        // Calc and store the accurate hit radius of the Atom in relation to the CoM
                        hitData.HitRadius[HITOR] = atomOffset * c_MPP;
                        hitData.HitPoint.Reset();
                        hitData.BitmapNormal.Reset();

                        // Check for the collision point in the dominant direction of travel.
                        if (delta[dom] && ((dom == X && g_SceneMan.GetMOIDPixel(hitPos[X], intPos[Y]) != g_NoMOID) ||
                                           (dom == Y && g_SceneMan.GetMOIDPixel(intPos[X], hitPos[Y]) != g_NoMOID)))
                        {
                            hit[dom] = true;
                            hitData.HitPoint = dom == X ? Vector(static_cast<float>(hitPos[X]), static_cast<float>(intPos[Y])) :
                                                          Vector(static_cast<float>(intPos[X]), static_cast<float>(hitPos[Y]));
                            hitData.BitmapNormal[dom] = static_cast<float>(-increment[dom]);
                        }

                        // Check for the collision point in the submissive direction of travel.
                        if (subStepped && delta[sub] && ((sub == X && g_SceneMan.GetMOIDPixel(hitPos[X], intPos[Y]) != g_NoMOID) ||
                                                         (sub == Y && g_SceneMan.GetMOIDPixel(intPos[X], hitPos[Y]) != g_NoMOID)))
                        {
                            hit[sub] = true;
//                            if (hitData.HitPoint.IsZero())
                                hitData.HitPoint = sub == X ? Vector(static_cast<float>(hitPos[X]), static_cast<float>(intPos[Y])) :
                                                              Vector(static_cast<float>(intPos[X]), static_cast<float>(hitPos[Y]));
// NOTE: THis can actually be wrong since there may not in fact be a corner pixel, but two pixels hit on X and Y directions
                            // We hit pixels in both sub and dom directions on the other MO, a corner hit.
//                            else
//                                hitData.HitPoint.SetXY(hitPos[X], hitPos[Y]);
                            hitData.BitmapNormal[sub] = static_cast<float>(-increment[sub]);
                        }

                        // If neither the direct dominant or sub directions yielded a collision point, then
                        // that means we hit right on the corner of a pixel, and that is the collision point.
                        if (!hit[dom] && !hit[sub])
                        {
                            hit[dom] = hit[sub] = true;
                            hitData.HitPoint.SetXY(static_cast<float>(hitPos[X]), static_cast<float>(hitPos[Y]));
                            hitData.BitmapNormal.SetXY(static_cast<float>(-increment[X]), static_cast<float>(-increment[Y]));
                        }
                        hitData.BitmapNormal.Normalize();

                        // Extract the current Atom's offset from the int positions.
                        intPos[X] -= atomOffset.GetFloorIntX();
                        intPos[Y] -= atomOffset.GetFloorIntY();
                        hitPos[X] -= atomOffset.GetFloorIntX();
                        hitPos[Y] -= atomOffset.GetFloorIntY();

                        MOID hitMOID = g_SceneMan.GetMOIDPixel(hitData.HitPoint.GetFloorIntX(), hitData.HitPoint.GetFloorIntY());

                        if (hitMOID != g_NoMOID)
                        {
                            hitData.Body[HITOR] = m_pOwnerMO;
                            hitData.Body[HITEE] = g_MovableMan.GetMOFromID(hitMOID);
                            RTEAssert(hitData.Body[HITEE], "Hitee MO is 0 in AtomGroup::PushTravel!");

                            hitData.Body[HITEE]->CollideAtPoint(hitData);

                            // Save the impulse force resulting from the MO collision response calc.
                            impulseForces.push_back(make_pair(hitData.ResImpulse[HITOR], atomOffset));
                        }
                    }
                }
                // If any MO's were hit, continue on to the next leg without doing terrain stuff now.
                // Any terrain collisions will be taken care of on the next leg, when the effects of
                // these MO hits have been applied.
                hitTerrAtoms.clear();
            }

            // TERRAIN COLLISION RESPONSE /////////////////////////////////////////////////////
            // Determine which of the colliding Atoms will penetrate the terrain.
			bool somethingPenetrated = false;
            do {
				somethingPenetrated = false;

				massDist = mass / static_cast<float>(hitTerrAtoms.size() * (m_Resolution ? m_Resolution : 1));

                for (deque<pair<Atom *, Vector> >::iterator aoItr = hitTerrAtoms.begin(); aoItr != hitTerrAtoms.end(); )
                {
						if (g_SceneMan.WillPenetrate(intPos[X] + (*aoItr).second.GetFloorIntX(),
							intPos[Y] + (*aoItr).second.GetFloorIntY(),
							forceVel,
							massDist))
						{
							// Move the penetrating atom to the pen. list from the coll. list.
							penetratingAtoms.push_back(pair<Atom *, Vector>((*aoItr).first, (*aoItr).second));
							aoItr = hitTerrAtoms.erase(aoItr);
							somethingPenetrated = true;
						}
						else
							++aoItr;
                }
			} while (!hitTerrAtoms.empty() && somethingPenetrated);

            // TERRAIN BOUNCE //////////////////////////////////////////////////////////////////
            // If some Atoms could not penetrate even though all the mass was on them,
            // gather the bounce results and apply them to the owner.
			if (!hitTerrAtoms.empty())
            {
                newDir = true;
                prevError = error;

                // Back up one step so that we're not intersecting the terrain anymore
                intPos[dom] -= increment[dom];
                if (subStepped)
                    intPos[sub] -= increment[sub];
                // Undo wrap, if necessary.
                didWrap = !g_SceneMan.WrapPosition(intPos[X], intPos[Y]) && didWrap;

                // Call the call-on-bounce function, if requested.
//                if (m_pOwnerMO && callOnBounce)
//                  halted = m_pOwnerMO->OnBounce(position);

                // Calculate the distributed mass that each bouncing Atom has.
                massDist = mass / static_cast<float>((hitTerrAtoms.size()/* + atomsHitMOsCount*/) *
                                (m_Resolution ? m_Resolution : 1));

                // Gather the collision response effects so that the impulse force can be calculated.
                for (const pair<Atom *, Vector> &hitTerrAtomsEntry : hitTerrAtoms)
                {
                    // Bake in current Atom's offset into the int positions.
					const Vector &atomOffset = hitTerrAtomsEntry.second;
                    intPos[X] += atomOffset.GetFloorIntX();
                    intPos[Y] += atomOffset.GetFloorIntY();
					hitPos[X] += atomOffset.GetFloorIntX();
					hitPos[Y] += atomOffset.GetFloorIntY();

                    Vector newVel = forceVel;

                    hitMaterialID = g_SceneMan.GetTerrMatter(hitPos[X], hitPos[Y]);
                    hitMaterial = g_SceneMan.GetMaterialFromID(hitMaterialID);

                    // Check for and react upon a collision in the dominant direction of travel.
                    if (delta[dom] && ((dom == X && g_SceneMan.GetTerrMatter(hitPos[X], intPos[Y])) ||
                                    (dom == Y && g_SceneMan.GetTerrMatter(intPos[X], hitPos[Y]))))
                    {
                        hit[dom] = true;
                        domMaterialID = dom == X ? g_SceneMan.GetTerrMatter(hitPos[X], intPos[Y]) :
                                                   g_SceneMan.GetTerrMatter(intPos[X], hitPos[Y]);
                        domMaterial = g_SceneMan.GetMaterialFromID(domMaterialID);

                        // Bounce according to the collision.
                        newVel[dom] = -newVel[dom] *
                                       hitTerrAtomsEntry.first->GetMaterial()->GetRestitution() *
                                       domMaterial->GetRestitution();
                    }

                    // Check for and react upon a collision in the submissive direction of travel.
                    if (subStepped && delta[sub] && ((sub == X && g_SceneMan.GetTerrMatter(hitPos[X], intPos[Y])) ||
                                                    (sub == Y && g_SceneMan.GetTerrMatter(intPos[X], hitPos[Y]))))
                    {
                        hit[sub] = true;
                        subMaterialID = sub == X ? g_SceneMan.GetTerrMatter(hitPos[X], intPos[Y]) :
                                                g_SceneMan.GetTerrMatter(intPos[X], hitPos[Y]);
                        subMaterial = g_SceneMan.GetMaterialFromID(subMaterialID);

                        // Bounce according to the collision.
                        newVel[sub] = -newVel[sub] *
                                       hitTerrAtomsEntry.first->GetMaterial()->GetRestitution() *
                                       subMaterial->GetRestitution();
                    }

                    // If hit right on the corner of a pixel, bounce straight back with no friction.
                    if (!hit[dom] && !hit[sub])
                    {
                        hit[dom] = true;
                        newVel[dom] = -newVel[dom] *
                                       hitTerrAtomsEntry.first->GetMaterial()->GetRestitution() *
                                       hitMaterial->GetRestitution();
                        hit[sub] = true;
                        newVel[sub] = -newVel[sub] *
                                       hitTerrAtomsEntry.first->GetMaterial()->GetRestitution() *
                                       hitMaterial->GetRestitution();
                    }
                    // Calculate the effects of friction.
                    else if (hit[dom] && !hit[sub])
                    {
                        newVel[sub] -= newVel[sub] * hitTerrAtomsEntry.first->GetMaterial()->GetFriction() * domMaterial->GetFriction();
                    }
                    else if (hit[sub] && !hit[dom])
                    {
                        newVel[dom] -= newVel[dom] * hitTerrAtomsEntry.first->GetMaterial()->GetFriction() * subMaterial->GetFriction();
                    }

                    // Compute and store this Atom's collision response impulse force.
                    impulseForces.push_back(make_pair((newVel - forceVel) * massDist,
						atomOffset));

					// Extract the current Atom's offset from the int positions.
					intPos[X] -= atomOffset.GetFloorIntX();
					intPos[Y] -= atomOffset.GetFloorIntY();
					hitPos[X] -= atomOffset.GetFloorIntX();
					hitPos[Y] -= atomOffset.GetFloorIntY();
                }
            }
            // TERRAIN SINK ////////////////////////////////////////////////////////////////
            // Else all Atoms must have penetrated and therefore the entire group has
            // sunken into the terrain. Get the penetration resistance results and
            // apply them to the owner.
            else if (!penetratingAtoms.empty())
            {
                sinkHit = hit[dom] = hit[sub] = true;
                prevError = false;
                prevError = error;

                // Call the call-on-sink function, if requested.
//                if (callOnSink)
//                  halted = m_pOwnerMO->OnSink(position);

                massDist = mass / static_cast<float>(penetratingAtoms.size() * (m_Resolution ? m_Resolution : 1));

                // Apply the collision response effects.
				for (const pair<Atom *, Vector> &penetratingAtomsEntry : penetratingAtoms)
				{
					if (g_SceneMan.TryPenetrate(intPos[X] + penetratingAtomsEntry.second.GetFloorIntX(),
						intPos[Y] + penetratingAtomsEntry.second.GetFloorIntY(),
						forceVel * massDist,
						forceVel,
						retardation,
						1.0F,
						penetratingAtomsEntry.first->GetNumPenetrations()))
					{

						impulseForces.push_back(make_pair(forceVel * massDist * retardation,
							penetratingAtomsEntry.second));
					}
                }
            }

            // APPLY COLLISION RESPONSES ///////////////////////////////////////////////////
            // If we hit anything, and are about to start a new leg instead of a step,
            // apply the averaged collision response effects to the owning MO.
            if (hit[X] || hit[Y])
            {
                // Move position forward to the hit position.
                position += legProgress;
                // Wrap position, if necessary.
                didWrap = didWrap || g_SceneMan.WrapPosition(position);

                // Apply velocity averages to the final resulting velocity for this leg.
                for (const pair<Vector, Vector> &impulseForcesEntry : impulseForces)
                {
                    // Cap the impulse to what the max push force is
					//impulseForcesEntry.first.CapMagnitude(pushForce * (travelTime/* - timeLeft*/));
                    velocity += impulseForcesEntry.first / mass;
                    returnPush += impulseForcesEntry.first;
                }

                // Stunt travel time if there is no more velocity
                if (velocity.IsZero())
                    timeLeft = 0;
            }
            ++stepCount;
        }
        ++legCount;
    } while ((hit[X] || hit[Y]) &&
             timeLeft > 0.0F &&
             /*!trajectory.GetFloored().IsZero() &&*/
             !halted &&
             hitCount < 3);

    // Unlock the scene
    if (!scenePreLocked)
        g_SceneMan.UnlockScene();

    // Travel along the remaining trajectory.
	if (!(hit[X] || hit[Y]) && !halted)
    {
        position += trajectory;
        // Wrap, if necessary.
        didWrap = g_SceneMan.WrapPosition(position) || didWrap;
		return returnPush;
    }
    return returnPush;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PushAsLimb
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the group of Atoms travel together as a limb, relative to the
//                  position of the owning MovableObject. If terrain is hit, resulting
//                  accelerations should be applied to the MovableObject's static body.

bool AtomGroup::PushAsLimb(const Vector &jointPos,
                           const Vector &velocity,
                           const Matrix &rotation,
                           LimbPath &limbPath,
                           const float travelTime,
                           bool *pRestarted,
                           bool affectRotation)
{
    if (!m_pOwnerMO)
    {
        RTEAbort("Travelling an AtomGroup without a parent MO!");
        return false;
    }

    bool didWrap = false;
	Vector pushImpulse;

    // Pin the path to where the owner wants the joint to be.
    limbPath.SetJointPos(jointPos);
    limbPath.SetJointVel(velocity);
    limbPath.SetRotation(rotation);
    limbPath.SetFrameTime(travelTime);

    const Vector distVec = g_SceneMan.ShortestDistance(jointPos, m_LimbPos);
    // Restart the path if the limb is way off somewhere else
    if (distVec.GetMagnitude() > m_pOwnerMO->GetDiameter())
        limbPath.Terminate();

    do {
        if (limbPath.PathEnded())
        {
            if (pRestarted)
                *pRestarted = true;
            if (!limbPath.RestartFree(m_LimbPos, m_pOwnerMO->GetRootID(), m_pOwnerMO->IgnoresWhichTeam()))
                return false;
        }

        // Do the push travel calculations and get the resulting push impulse vector back.
        pushImpulse = PushTravel(m_LimbPos,
                                 limbPath.GetCurrentVel(m_LimbPos),
                                 limbPath.GetPushForce(),
                                 didWrap,
                                 limbPath.GetNextTimeChunk(m_LimbPos),
                                 false,
                                 false);

        // Report back to the path where we've ended up.
        limbPath.ReportProgress(m_LimbPos);

    // End the path push loop if the path has ended or we ran out of time.
    } while (!limbPath.FrameDone() && !limbPath.PathEnded());

    // Sanity check this force coming out of really old crummy physics code
    if (pushImpulse.GetLargest() > 10000.0F)
        pushImpulse.Reset();

    // Add the resulting impulse force, add the lever of the joint offset if set to do so
    if (affectRotation)
        m_pOwnerMO->AddImpulseForce(pushImpulse, g_SceneMan.ShortestDistance(m_pOwnerMO->GetPos(), jointPos) * c_MPP);
    else
        m_pOwnerMO->AddImpulseForce(pushImpulse, Vector());

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FlailAsLimb
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the group of Atoms travel together as a lifeless limb, constrained
//                  to a radius around the joint pin in the center.

void AtomGroup::FlailAsLimb(const Vector ownerPos,
                            const Vector jointOffset,
                            const float limbRadius,
                            const Vector velocity,
                            const float angVel,
                            const float mass,
                            const float travelTime)
{
    if (!m_pOwnerMO)
    {
        RTEAbort("Travelling an AtomGroup without a parent MO!");
        return;
    }

    bool didWrap = false;
	Vector target;
	Vector distVec;
	Vector pushImpulse;
    Vector jointPos = ownerPos + jointOffset;

    Vector centrifugalVel = jointOffset;
    centrifugalVel *= fabs(angVel);

    // Do the push travel calculations and get the resulting push impulse vector back.
    pushImpulse = PushTravel(m_LimbPos,
                             velocity + centrifugalVel,
                             100,
                             didWrap,
                             travelTime,
                             false,
                             false,
                             false);

    // Constrain within the range of the limb
    Vector limbVec = m_LimbPos - jointPos;
    if (limbVec.GetMagnitude() > limbRadius)
    {
        limbVec.SetMagnitude(limbRadius);
        m_LimbPos = jointPos + limbVec;
    }

    return;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          InTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether any of the Atoms in this AtomGroup are on terrain
//                  pixels.

bool AtomGroup::InTerrain()
{
    RTEAssert(m_pOwnerMO, "Using an AtomGroup without a parent MO!");

    if (!g_SceneMan.SceneIsLocked())
        g_SceneMan.LockScene();

    bool penetrates = false;
    Vector aPos;
// TODO: UNCOMMENT
    for (const Atom * atom : m_Atoms)
    {
        aPos = (m_pOwnerMO->GetPos() + (atom->GetOffset().GetXFlipped(m_pOwnerMO->m_HFlipped) * m_pOwnerMO->GetRotMatrix()));
		if (g_SceneMan.GetTerrMatter(aPos.GetFloorIntX(), aPos.GetFloorIntY()) != g_MaterialAir) {
			penetrates = true;
			break;
		}
/*
#ifdef DEBUG_BUILD
        // Draw a dot for each atom for visual reference.
        putpixel(g_SceneMan.GetDebugBitmap(), aPos.m_X, aPos.m_Y, 112);
#endif
*/
    }

    if (g_SceneMan.SceneIsLocked())
        g_SceneMan.UnlockScene();

    return penetrates;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RatioInTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows the ratio of how many atoms of this group that are on top of
//                  intact terrain pixels.

float AtomGroup::RatioInTerrain()
{
    RTEAssert(m_pOwnerMO, "Using an AtomGroup without a parent MO!");

    int inTerrain = 0;
    Vector aPos;

    for (const Atom * atom : m_Atoms)
    {
        aPos = (m_pOwnerMO->GetPos() + ((atom)->GetOffset().GetXFlipped(m_pOwnerMO->m_HFlipped) * m_pOwnerMO->GetRotMatrix()));
        if (g_SceneMan.GetTerrMatter(aPos.GetFloorIntX(), aPos.GetFloorIntY()) != g_MaterialAir)
            inTerrain++;
    }

    return static_cast<float>(inTerrain) / static_cast<float>(m_Atoms.size());
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResolveTerrainIntersection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether any of the Atoms in this AtomGroup are on top of
//                  terrain pixels, and if so, attempt to move the OwnerMO of this out
//                  so none of this' Atoms are inside any other MOS's silhouette anymore.

bool AtomGroup::ResolveTerrainIntersection(Vector &position, Matrix &rotation, unsigned char strongerThan)
{
	Vector atomOffset;
	Vector atomPos;
	Vector atomNormal;
	Vector clearPos;
	Vector exitDirection;
	Vector atomExitVector;
	Vector totalExitVector;
    list<Atom *> intersectingAtoms;
    MOID hitMaterial = g_MaterialAir;
    float strengthThreshold = strongerThan != g_MaterialAir ? g_SceneMan.GetMaterialFromID(strongerThan)->GetIntegrity() : 0.0F;
    bool rayHit = false;

    exitDirection.Reset();
    atomExitVector .Reset();
    totalExitVector.Reset();
    intersectingAtoms.clear();

    // First go through all atoms to find the first intersection and get the intersected MO
    for (Atom *atom : m_Atoms)
    {
        atomOffset = atom->GetOffset().GetXFlipped(m_pOwnerMO->IsHFlipped());
        atomOffset *= rotation;
		atom->SetupPos(position + atomOffset);

        atomPos = atom->GetCurrentPos();
		hitMaterial = g_SceneMan.GetTerrain()->GetPixel(atomPos.GetFloorIntX(), atomPos.GetFloorIntY());
		if (hitMaterial != g_MaterialAir && strengthThreshold > 0.0F && g_SceneMan.GetMaterialFromID(hitMaterial)->GetIntegrity() > strengthThreshold)
		{
			// Add atom to list of intersecting ones
			intersectingAtoms.push_back(atom);
		}
    }

    // No intersections - we're clear!
	if (intersectingAtoms.empty()) {
		return true;
	}

    // If all atoms are intersecting, we're screwed?!
	if (intersectingAtoms.size() >= m_Atoms.size()) {
		return false;
	}

    // Go through all intesecting atoms and find their average inverse normal
	for (const Atom *intersectingAtom : intersectingAtoms) {
		exitDirection += m_pOwnerMO->RotateOffset(intersectingAtom->GetNormal());
	}

    // We don't have a direction to go, so quit
// TODO: Maybe use previous position to create an exit direction instead then?
	if (exitDirection.IsZero()) {
		return false;
	}

    // Invert and set appropriate length
    exitDirection = -exitDirection;
// TODO: Use reasonable diameter of owner MO!
    exitDirection.SetMagnitude(m_pOwnerMO->GetDiameter());

    // See which of the intersecting atoms has the longest to travel along the exit direction before it clears
    float longestDistance = 0.0F;
    for (const Atom *intersectingAtom : intersectingAtoms)
    {
        atomPos = intersectingAtom->GetCurrentPos();

        if (strengthThreshold <= 0.0F)
            rayHit = g_SceneMan.CastMaterialRay(atomPos, exitDirection, g_MaterialAir, clearPos, 0, false);
        else
            rayHit = g_SceneMan.CastWeaknessRay(atomPos, exitDirection, strengthThreshold, clearPos, 0, false);

        if (rayHit)
        {
            // Determine the longest clearing distance so far
            atomExitVector = clearPos - atomPos.GetFloored();
            if (atomExitVector.GetMagnitude() > longestDistance)
            {
                // We found the atom with the longest to travel along the exit direction to clear,
                //  so that's the distance to move the whole object to clear all its atoms
                longestDistance = atomExitVector.GetMagnitude();
                totalExitVector = atomExitVector;
            }
        }
    }

    // If the exit vector is too large, then avoid the jarring jump and report that we didn't make it out
    if (totalExitVector.GetMagnitude() > m_pOwnerMO->GetRadius())
    {
//        position += totalExitVector / 2;
        return false;
    }

    // Now actually apply the exit vectors to this 
    position += totalExitVector;

// TODO: this isn't really true since we don't check for clearness after moving the position
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResolveMOSIntersection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether any of the Atoms in this AtomGroup are on top of
//                  other MOSprite:s, and if so, attempt to move the OwnerMO of this out
//                  so none of this' Atoms are inside any other MOS's silhouette anymore.

bool AtomGroup::ResolveMOSIntersection(Vector &position, Matrix &rotation)
{
    if (!m_pOwnerMO->m_HitsMOs)
        return true;

	Vector atomOffset;
	Vector atomPos;
	Vector atomNormal;
	Vector clearPos;
	Vector exitDirection;
	Vector atomExitVector;
	Vector totalExitVector;
    list<Atom *> intersectingAtoms;
	MOID hitMOID = g_NoMOID;
	MOID currentMOID = g_NoMOID;
    MovableObject *pIntersectedMO = 0;
    MOSRotating *pIntersectedMOS = 0;
	float massA;
	float massB;
	float invMassA;
	float invMassB;
	float normMassA;
	float normMassB;

    hitMOID = g_NoMOID;
    currentMOID = g_NoMOID;
    pIntersectedMO = 0;
    exitDirection.Reset();
    atomExitVector .Reset();
    totalExitVector.Reset();
    intersectingAtoms.clear();

    // First go through all atoms to find the first intersection and get the intersected MO
    for (Atom *atom : m_Atoms)
    {
        atomOffset = atom->GetOffset().GetXFlipped(m_pOwnerMO->IsHFlipped());
        atomOffset *= rotation;
        atom->SetupPos(position + atomOffset);

        atomPos = atom->GetCurrentPos();
		hitMOID = g_SceneMan.GetMOIDPixel(atomPos.GetFloorIntX(), atomPos.GetFloorIntY());
		// Don't count MOIDs ignored
		if (hitMOID != g_NoMOID && !atom->IsIgnoringMOID(hitMOID))
		{
			// Save the correct MOID to search for other atom intersections with
			currentMOID = hitMOID;

			// Get the MO we seem to be intersecting
			MovableObject *tempMO = g_MovableMan.GetMOFromID(hitMOID);
			RTEAssert(tempMO, "Intersected MOID couldn't be translated to a real MO!");
			tempMO = tempMO->GetRootParent();

			if (tempMO->GetsHitByMOs())
			{
				// Make that MO draw itself again in the MOID layer so we can find its true edges
				pIntersectedMO = tempMO;
				pIntersectedMO->Draw(g_SceneMan.GetMOIDBitmap(), Vector(), g_DrawMOID, true);
				break;
				// TODO: Remove
				//                g_FrameMan.SaveBitmapToBMP(g_SceneMan.GetMOIDBitmap(), "MOIDMap");
			}
		}
    }

    // If no intersected MO is found, we are clear and can quit
	if (!pIntersectedMO) {
		return false;
	}

    // Tell both MO's that they have hit an MO, and see if they want to continue
	if (m_pOwnerMO->OnMOHit(pIntersectedMO)) {
		return false;
	}
	if (pIntersectedMO->OnMOHit(m_pOwnerMO->GetRootParent())) {
		return false;
	}

    // Restart and go through all atoms to find all intersecting the specific intersected MO
    for (Atom *atom : m_Atoms)
    {
        atomPos = atom->GetCurrentPos();
        if (g_SceneMan.GetMOIDPixel(atomPos.GetFloorIntX(), atomPos.GetFloorIntY()) == currentMOID)
        {
            // Add atom to list of intersecting ones
            intersectingAtoms.push_back(atom);
        }
    }

    // No intersections - we're clear?!
//        if (intersectingAtoms.empty())
//            return true;
//        RTEAssert(!intersectingAtoms.empty(), "Couldn't find any intersections after finding one?!");

    // If all atoms are intersecting, we're screwed?!
//        if (intersectingAtoms.size() >= m_Atoms.size())
//            return false;

    // Go through all intesecting atoms and find their average inverse normal
	for (const Atom *intersectingAtom : intersectingAtoms) {
		exitDirection += m_pOwnerMO->RotateOffset(intersectingAtom->GetNormal());
	}

    // We don't have a direction to go, so quit
// TODO: Maybe use previous position to create an exit direction instead then?
	if (exitDirection.IsZero()) {
		return false;
	}

    // Invert and set appropriate length
    exitDirection = -exitDirection;
// TODO: Use reasonable diameter of owner MO!
    exitDirection.SetMagnitude(m_pOwnerMO->GetDiameter());

    // See which of the intersecting atoms has the longest to travel along the exit direction before it clears
    float longestDistance = 0.0F;
    for (const Atom *intersectingAtom : intersectingAtoms)
    {
        atomPos = intersectingAtom->GetCurrentPos();
        if (g_SceneMan.CastFindMORay(atomPos, exitDirection, g_NoMOID, clearPos, 0, true, 0))
        {
            // Determine the longest clearing distance so far
            atomExitVector = clearPos - atomPos.GetFloored();
            if (atomExitVector.GetMagnitude() > longestDistance)
            {
                // We found the atom with the longest to travel along the exit direction to clear,
                //  so that's the distance to move the whole object to clear all its atoms
                longestDistance = atomExitVector.GetMagnitude();
                totalExitVector = atomExitVector;
            }
        }
    }

    // The final exit movement vectors for this' Owner MO and the MO intersected by this
	Vector thisExit;
	Vector intersectedExit;

    // If the other object is pinned, then only move this
    if (pIntersectedMO->GetPinStrength() > 0.0F)
        thisExit = totalExitVector;
    // Otherwise, apply the object exit vector to both this owner object, and the one it intersected, proportional to their masses
    else
    {
        massA = m_pOwnerMO->GetMass();
        massB = pIntersectedMO->GetMass();
        invMassA = 1.0F / massA;
        invMassB = 1.0F / massB;
        // Normalize the masses
        normMassA = invMassA / (invMassA + invMassB);
        normMassB = invMassB / (invMassA + invMassB);

        // If the intersected is much larger than this' MO, then only move this
        if (normMassB < 0.33F)
            thisExit = totalExitVector;
        // Apply the movements to both this and the intersected MO's, proportional to their respective masses
        else
        {
            thisExit = totalExitVector * normMassA;
            intersectedExit = -totalExitVector * normMassB;
        }
    }

    // Now actually apply the exit vectors to both, but only if the jump isn't too jarring
    if (thisExit.GetMagnitude() < m_pOwnerMO->GetRadius())
        position += thisExit;
    if (!intersectedExit.IsZero() && intersectedExit.GetMagnitude() < pIntersectedMO->GetRadius())
        pIntersectedMO->SetPos(pIntersectedMO->GetPos() + intersectedExit);

    // If we've been pushed into the terrain enough, just gib as the squashing effect
    if (m_pOwnerMO->CanBeSquished() && RatioInTerrain() > 0.75F)// && totalExitVector.GetMagnitude() > m_pOwnerMO->GetDiameter())
    {
        // Move back before gibbing so gibs don't end up inside terrain
        position -= thisExit;
        m_pOwnerMO->GibThis(-totalExitVector);
    }

    pIntersectedMOS = dynamic_cast<MOSRotating *>(pIntersectedMO);

    if (pIntersectedMOS && pIntersectedMOS->CanBeSquished() && pIntersectedMOS->GetAtomGroup()->RatioInTerrain() > 0.75F)// && totalExitVector.GetMagnitude() > pIntersectedMO->GetDiameter())
    {
        // Move back before gibbing so gibs don't end up inside terrain
        pIntersectedMO->SetPos(pIntersectedMO->GetPos() - intersectedExit);
        pIntersectedMOS->GibThis(totalExitVector);
    }

// TODO: this isn't really true since we don't check for clearness after moving the position
    return intersectingAtoms.empty();
}


void AtomGroup::AddAtomToGroup(int x, int y, const Vector &spriteOffset, MOSRotating *pOwnerMOSRotating, bool calcNormal)
{
	Atom *pAtom = new Atom;
	pAtom->Create(Vector(static_cast<float>(x) + spriteOffset.m_X, static_cast<float>(y) + spriteOffset.m_Y), m_pMaterial, pOwnerMOSRotating);
	if (calcNormal) {
		pAtom->CalculateNormal(pOwnerMOSRotating->GetSpriteFrame(), -(pOwnerMOSRotating->GetSpriteOffset()));
	}
	pAtom->SetIgnoreMOIDsByGroup(&m_IgnoreMOIDs);
	m_Atoms.push_back(pAtom);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this AtomGroup's current graphical debug representation to a
//                  BITMAP of choice.

void AtomGroup::Draw(BITMAP *pTargetBitmap,
                     const Vector &targetPos,
                     bool useLimbPos,
                     unsigned char color) const
{
    acquire_bitmap(pTargetBitmap);
	Vector aPos;
	Vector normal;
    for (const Atom * atom : m_Atoms)
    {
        if (!useLimbPos)
            aPos = (m_pOwnerMO->GetPos() + (atom->GetOffset().GetXFlipped(m_pOwnerMO->m_HFlipped)
                                            /** m_pOwnerMO->GetRotMatrix()*/)).GetFloored();
        else
            aPos = (m_LimbPos + (atom->GetOffset().GetXFlipped(m_pOwnerMO->m_HFlipped)
                                 /* * m_pOwnerMO->GetRotMatrix()*/)).GetFloored();

        // Draw normal first
        if (!atom->GetNormal().IsZero())
        {
            normal = atom->GetNormal().GetXFlipped(m_pOwnerMO->m_HFlipped) * 5;
            line(pTargetBitmap, aPos.GetFloorIntX() - targetPos.GetFloorIntX(),
                              aPos.GetFloorIntY() - targetPos.GetFloorIntY(),
                              aPos.GetFloorIntX() - targetPos.GetFloorIntX(),
                              aPos.GetFloorIntY() - targetPos.GetFloorIntY(),
                              244);
        }

        // Then draw the atom position
        putpixel(pTargetBitmap, aPos.GetFloorIntX() - targetPos.GetFloorIntX(), aPos.GetFloorIntY() - targetPos.GetFloorIntY(), color);
    }

    release_bitmap(pTargetBitmap);
}

} // namespace RTE
