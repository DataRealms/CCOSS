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
#include "RTEManagers.h"
#include "SLTerrain.h"
#include "MovableObject.h"
#include "MOSRotating.h"
#include <deque>
#include <set>

#include "ConsoleMan.h"

using namespace std;

namespace RTE
{

CONCRETECLASSINFO(AtomGroup, Entity, 200)


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
    else if (m_pMaterial->id != m_Atoms.front()->GetMaterial()->id)
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

int AtomGroup::Create(const AtomGroup &reference, boolean onlyCopyOwnerAtoms)
{
    SLICK_PROFILE(0xFF662348);

    Entity::Create(reference);

    m_AutoGenerate = false; //reference.m_AutoGenerate; Don't because we'll copy the atoms below
    m_pMaterial = reference.m_pMaterial;
    m_Resolution = reference.m_Resolution;
    m_Depth = reference.m_Depth;

	m_SubGroups.clear();
	for (list<Atom *>::const_iterator itr = reference.m_Atoms.begin(); itr != reference.m_Atoms.end(); ++itr)
	{
		if (!onlyCopyOwnerAtoms || (*itr)->GetSubID() == 0)
		{
			Atom *pAtomCopy = new Atom(**itr);
			pAtomCopy->SetIgnoreMOIDsByGroup(&m_IgnoreMOIDs);

			m_Atoms.push_back(pAtomCopy);

			// Add to the appropriate spot in the subgroup map
			long int subID = pAtomCopy->GetSubID();
			if (subID != 0)
			{
				// Try to find the group
				// No atom added to that group yet, so it doesn't exist, so make it
				if (m_SubGroups.find(subID) == m_SubGroups.end())
				{
					m_SubGroups.insert(pair<long int, list<Atom *> >(subID, list<Atom *>())).first;
				}
				// Add Atom to the list of that group
				m_SubGroups.find(subID)->second.push_back(pAtomCopy);
			}
		}
	}

	// Copy ignored MOIDs list
	for (list<MOID>::const_iterator itr = reference.m_IgnoreMOIDs.begin(); itr != reference.m_IgnoreMOIDs.end(); ++itr)
	{
		m_IgnoreMOIDs.push_back(*itr);
	}


    // Make sure the tansfer of material properties happens
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
        DDTAbort("Trying to generate an AtomGroup without sprite and//or 0 resolution setting!");
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
    int width = refSprite->w * pOwnerMOSRotating->GetScale();
    int height = refSprite->h * pOwnerMOSRotating->GetScale();
    int x, y, i;
    Vector spriteOffset = pOwnerMOSRotating->GetSpriteOffset();
    bool inside = false;


// TODO: GET THIS WORKING WITH SCALED SPRITES!!$@#")
    // This offset is to fix some weirdness going on with the rotozoom
    int offX = -refSprite->w;
    int offY = -refSprite->h;

	Atom *pAtom = 0;

	// Only try to generate AtomGorup if scaled width and height are > 0 as we're playing with fire trying to create 0x0 bitmap. 
	// In debug mode it fires an assertion fail somewhere in DirectX
	if (width > 0 && height > 0)
	{
		checkBitmap = create_bitmap_ex(8, width, height);
		clear_to_color(checkBitmap, g_KeyColor);

		acquire_bitmap(refSprite);
		acquire_bitmap(checkBitmap);

		// If atoms are to be placed right at (below) the bitmap of the sprite.
		if (depth <= 0)
		{
			// First scan horizontally, and place Atom:s on outer silhouette edges.
			for (y = 0; y < height; y += m_Resolution)
			{
				// Scan LEFT to RIGHT, placing one Atom on each first encountered silhouette edge
				inside = false;
				for (x = 0; x < width; ++x)
				{
					// Detect if we are crossing a silhouette boundary.
					if (getpixel(refSprite, x, y) != g_KeyColor)
					{
						// Mark that an atom has been put in this location, to avoid duplicate Atom:s
						putpixel(checkBitmap, x, y, 99);
						pAtom = new Atom;
						pAtom->Create(Vector(x + spriteOffset.m_X, y + spriteOffset.m_Y), m_pMaterial, pOwnerMOSRotating);
						pAtom->CalculateNormal(pOwnerMOSRotating->GetSpriteFrame(), -(pOwnerMOSRotating->GetSpriteOffset()));
						pAtom->SetIgnoreMOIDsByGroup(&m_IgnoreMOIDs);
						m_Atoms.push_back(pAtom);
						pAtom = 0;
						break;
					}
				}

				// Scan RIGHT to LEFT, placing one Atom on each first encountered silhouette edge
				inside = false;
				for (x = width - 1; x >= 0; --x)
				{
					// Detect if we are crossing a silhouette boundary.
					if (getpixel(refSprite, x, y) != g_KeyColor)
					{
						// Mark that an atom has been put in this location, to avoid duplicate Atom:s
						putpixel(checkBitmap, x, y, 99);
						pAtom = new Atom;
						pAtom->Create(Vector(x + spriteOffset.m_X, y + spriteOffset.m_Y), m_pMaterial, pOwnerMOSRotating);
						pAtom->CalculateNormal(pOwnerMOSRotating->GetSpriteFrame(), -(pOwnerMOSRotating->GetSpriteOffset()));
						pAtom->SetIgnoreMOIDsByGroup(&m_IgnoreMOIDs);
						m_Atoms.push_back(pAtom);
						pAtom = 0;
						break;
					}
				}
			}

			// Then scan vertically, and place Atom:s on silhouette edge, but avoiding duplicates.
			for (x = 0; x < width; x += m_Resolution)
			{
				// Scan TOP to BOTTOM, placing one Atom on each first encountered silhouette edge
				inside = false;
				for (y = 0; y < height; ++y)
				{
					// Detect if we are crossing a silhouette boundary, but make sure Atom wasn't
					// already placed during the horizontal scans.
					if (getpixel(refSprite, x, y) != g_KeyColor && getpixel(checkBitmap, x, y) == g_KeyColor)
					{
						pAtom = new Atom;
						pAtom->Create(Vector(x + spriteOffset.m_X, y + spriteOffset.m_Y - (int)inside), m_pMaterial, pOwnerMOSRotating);
						pAtom->CalculateNormal(pOwnerMOSRotating->GetSpriteFrame(), -(pOwnerMOSRotating->GetSpriteOffset()));
						pAtom->SetIgnoreMOIDsByGroup(&m_IgnoreMOIDs);
						m_Atoms.push_back(pAtom);
						pAtom = 0;
						break;
					}
				}

				// Scan BOTTOM to TOP, placing one Atom on each first encountered silhouette edge
				inside = false;
				for (y = height - 1; y >= 0; --y)
				{
					// Detect if we are crossing a silhouette boundary, but make sure Atom wasn't
					// already placed during the horizontal scans.
					if (getpixel(refSprite, x, y) != g_KeyColor && getpixel(checkBitmap, x, y) == g_KeyColor)
					{
						pAtom = new Atom;
						pAtom->Create(Vector(x + spriteOffset.m_X, y + spriteOffset.m_Y - (int)inside), m_pMaterial, pOwnerMOSRotating);
						pAtom->CalculateNormal(pOwnerMOSRotating->GetSpriteFrame(), -(pOwnerMOSRotating->GetSpriteOffset()));
						pAtom->SetIgnoreMOIDsByGroup(&m_IgnoreMOIDs);
						m_Atoms.push_back(pAtom);
						pAtom = 0;
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
			// and place Atom:s in depth beyond the sihouette edge.
			for (y = 0; y < height; y += m_Resolution)
			{
				inside = false;
				for (x = 0; x < width; ++x)
				{
					// Detect if we are crossing a silhouette boundary.
					if ((getpixel(refSprite, x, y) != g_KeyColor && !inside) ||
						(getpixel(refSprite, x, y) == g_KeyColor && inside))
					{
						// Reset the depth counter
						depthCount = 0;
						inside = !inside;
					}
					// Increment the depth counter if inside silhouette,
					// and propose making atom if specified depth is reached.
					else if (inside && ++depthCount == depth)
					{
						clear = true;
						// Check whether depth is sufficient in the other cardinal directions.
						for (i = 1; i <= depth && clear; ++i)
						{
							if (x + i >= refSprite->w ||
								y + i >= refSprite->h ||
								y - i < 0 ||
								getpixel(refSprite, x + i, y) == g_KeyColor ||
								getpixel(refSprite, x, y + i) == g_KeyColor ||
								getpixel(refSprite, x, y - i) == g_KeyColor)
								clear = false;
						}
						// Depth is cleared in all directions, so go ahead and place Atom.
						if (clear && getpixel(checkBitmap, x, y) == g_KeyColor)
						{
							// Mark that an atom has been put in this location, to avoid duplicate Atom:s
							putpixel(checkBitmap, x, y, 99);
							pAtom = new Atom;
							pAtom->Create(Vector(x + spriteOffset.m_X, y + spriteOffset.m_Y), m_pMaterial, pOwnerMOSRotating);
							pAtom->SetIgnoreMOIDsByGroup(&m_IgnoreMOIDs);
							m_Atoms.push_back(pAtom);
							pAtom = 0;
						}
					}
				}
			}
			// Scan HORIZONTALLY from RIGHT to LEFT,
			// and place Atom:s in depth beyond the sihouette edge.
			for (y = 0; y < height; y += m_Resolution)
			{
				inside = false;
				for (x = width - 1; x >= 0; --x)
				{
					// Detect if we are crossing a silhouette boundary.
					if ((getpixel(refSprite, x, y) != g_KeyColor && !inside) ||
						(getpixel(refSprite, x, y) == g_KeyColor && inside))
					{
						// Reset the depth counter
						depthCount = 0;
						inside = !inside;
					}
					// Increment the depth counter if inside silhouette,
					// and propose making atom if specified depth is reached.
					else if (inside && ++depthCount == depth)
					{
						clear = true;
						// Check whether depth is sufficient in the other cardinal directions.
						for (i = 1; i <= depth && clear; ++i)
						{
							if (x - i < 0 ||
								y + i >= refSprite->h ||
								y - i < 0 ||
								getpixel(refSprite, x - i, y) == g_KeyColor ||
								getpixel(refSprite, x, y + i) == g_KeyColor ||
								getpixel(refSprite, x, y - i) == g_KeyColor)
								clear = false;
						}
						// Depth is cleared in all directions, so go ahead and place Atom.
						if (clear && getpixel(checkBitmap, x, y) == g_KeyColor)
						{
							// Mark that an atom has been put in this location, to avoid duplicate Atom:s
							putpixel(checkBitmap, x, y, 99);
							pAtom = new Atom;
							pAtom->Create(Vector(x + spriteOffset.m_X, y + spriteOffset.m_Y), m_pMaterial, pOwnerMOSRotating);
							pAtom->SetIgnoreMOIDsByGroup(&m_IgnoreMOIDs);
							m_Atoms.push_back(pAtom);
							pAtom = 0;
						}
					}
				}
			}
			// Scan VERTICALLY from TOP to BOTTOM,
			// and place Atom:s in depth beyond the sihouette edge.
			for (x = 0; x < width; x += m_Resolution)
			{
				inside = false;
				for (y = 0; y < height; ++y)
				{
					// Detect if we are crossing a silhouette boundary.
					if ((getpixel(refSprite, x, y) != g_KeyColor && !inside) ||
						(getpixel(refSprite, x, y) == g_KeyColor && inside))
					{
						// Reset the depth counter
						depthCount = 0;
						inside = !inside;
					}
					// Increment the depth counter if inside silhouette,
					// and propose making atom if specified depth is reached.
					else if (inside && ++depthCount == depth)
					{
						clear = true;
						// Check whether depth is sufficient in the other cardinal directions.
						for (i = 1; i <= depth && clear; ++i)
						{
							if (x + i >= refSprite->w ||
								x - i < 0 ||
								y + i >= refSprite->h ||
								getpixel(refSprite, x + i, y) == g_KeyColor ||
								getpixel(refSprite, x - i, y) == g_KeyColor ||
								getpixel(refSprite, x, y + i) == g_KeyColor)
								clear = false;
						}
						// Depth is cleared in all directions, so go ahead and place Atom.
						if (clear && getpixel(checkBitmap, x, y) == g_KeyColor)
						{
							// Mark that an atom has been put in this location, to avoid duplicate Atom:s
							putpixel(checkBitmap, x, y, 99);
							pAtom = new Atom;
							pAtom->Create(Vector(x + spriteOffset.m_X, y + spriteOffset.m_Y), m_pMaterial, pOwnerMOSRotating);
							m_Atoms.push_back(pAtom);
							pAtom->SetIgnoreMOIDsByGroup(&m_IgnoreMOIDs);
							pAtom = 0;
						}
					}
				}
			}
			// Scan VERTICALLY from BOTTOM to TOP,
			// and place Atom:s in depth beyond the sihouette edge.
			for (x = 0; x < width; x += m_Resolution)
			{
				inside = false;
				for (y = height - 1; y >= 0; --y)
				{
					// Detect if we are crossing a silhouette boundary.
					if ((getpixel(refSprite, x, y) != g_KeyColor && !inside) ||
						(getpixel(refSprite, x, y) == g_KeyColor && inside))
					{
						// Reset the depth counter
						depthCount = 0;
						inside = !inside;
					}
					// Increment the depth counter if inside silhouette,
					// and propose making atom if specified depth is reached.
					else if (inside && ++depthCount == depth)
					{
						clear = true;
						// Check whether depth is sufficient in the other cardinal directions.
						for (i = 1; i <= depth && clear; ++i)
						{
							if (x + i >= refSprite->w ||
								x - i < 0 ||
								y - i < 0 ||
								getpixel(refSprite, x + i, y) == g_KeyColor ||
								getpixel(refSprite, x - i, y) == g_KeyColor ||
								getpixel(refSprite, x, y - i) == g_KeyColor)
								clear = false;
						}
						// Depth is cleared in all directions, so go ahead and place Atom.
						if (clear && getpixel(checkBitmap, x, y) == g_KeyColor)
						{
							// Mark that an atom has been put in this location, to avoid duplicate Atom:s
							putpixel(checkBitmap, x, y, 99);
							pAtom = new Atom;
							pAtom->Create(Vector(x + spriteOffset.m_X, y + spriteOffset.m_Y), m_pMaterial, pOwnerMOSRotating);
							m_Atoms.push_back(pAtom);
							pAtom->SetIgnoreMOIDsByGroup(&m_IgnoreMOIDs);
							pAtom = 0;
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
        pAtom = new Atom;
        pAtom->Create(-Vector((width / 2) + spriteOffset.m_X, (height / 2) + spriteOffset.m_Y), m_pMaterial, pOwnerMOSRotating);
        m_Atoms.push_back(pAtom);
		pAtom->SetIgnoreMOIDsByGroup(&m_IgnoreMOIDs);
		pAtom = 0;
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
		if (mat.id)
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
				DDTAbort("Failed to find a matching material \"" + mat.GetPresetName() + "\" or even fall back to g_MaterialAir. Aborting!");
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
        // See if the base class(es) can find a match instead
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
        for (list<Atom *>::const_iterator itr = m_Atoms.begin(); itr != m_Atoms.end(); ++itr)
        {
            writer.NewProperty("AddAtom");
            writer << **itr;
        }
//    }
    writer.NewProperty("JointOffset");
    writer << m_JointOffset;

    return 0;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the AtomGroup object ready for use.

int AtomGroup::Create(istream &stream, bool checkType)
{
    if (checkType)
    {
        string name;
        stream >> name;
        if (name != m_sClass.GetName())
        {
           DDTAbort("Wrong type in stream when passed to Create");
           return -1;
        }
    }

    int numAtoms;
    stream >> numAtoms;
    Atom *atom;
    for (int i = 0; i < numAtoms; ++i)
    {
        atom = new Atom;
        stream >> *atom;
        m_Atoms[0].push_back(atom);
    }

    stream >> m_PathCount;
    m_aFootPaths = new LimbPath[m_PathCount];
    for (i = 0; i < m_PathCount; ++i)
        stream >> m_aFootPaths[i];

// TODO: Exception handling")
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this AtomGroup to an output stream for
//                  later recreation with Create(istream &stream);

int AtomGroup::Save(ostream &stream) const
{
// TODO: Exception checking/handling")

    stream << m_sClass.GetName() << " ";

    stream << m_Atoms[0].size() << " ";
    for (list<Atom *>::const_iterator itr = m_Atoms[0].begin(); itr != m_Atoms[0].end(); ++itr)
        stream << **itr << " ";

    stream << m_PathCount << " ";
    for (int i = 0; i < m_PathCount; ++i)
        stream << m_aFootPaths[i] << " ";

    return 0;
}
*/

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
//                  Atom:s of two Groups and merges them into one.

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
//                  the Atom:s of two Groups pointed to and merges them into one.
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

float AtomGroup::CalculateMaxRadius()
{
    float magnitude, longest = 0;

    for (list<Atom *>::iterator aItr = m_Atoms.begin(); aItr != m_Atoms.end(); ++aItr)
    {
        magnitude = (*aItr)->GetOffset().GetMagnitude();
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
    if (m_MomInertia == 0)
    {
        if (!m_pOwnerMO)
        {
            DDTAbort("Getting AtomGroup stuff without a parent MO!");
            return 0;
        }
        float distMass = m_pOwnerMO->GetMass() / m_Atoms.size();
        float radius = 0;
        for (list<Atom *>::const_iterator itr = m_Atoms.begin(); itr != m_Atoms.end(); ++itr)
        {
            radius = (*itr)->GetOffset().GetMagnitude() * g_FrameMan.GetMPP();
            m_MomInertia += distMass * radius * radius;
        }
    }

    // Avoid zero (if radius is nonexistent, for example), will cause divide by zero problems otherwise
    if (m_MomInertia == 0)
        m_MomInertia = 0.000001;

    return m_MomInertia;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOwner
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current owner MovableObject of this AtomGroup.

void AtomGroup::SetOwner(MOSRotating *newOwner)
{
    m_pOwnerMO = newOwner;
    for (list<Atom *>::const_iterator itr = m_Atoms.begin(); itr != m_Atoms.end(); ++itr)
        (*itr)->SetOwner(m_pOwnerMO);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddAtoms
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a list of new Atom:s to the internal list that makes up this group.
//                  Ownership of all Atom:s in the list IS NOT transferred!

void AtomGroup::AddAtoms(const std::list<Atom *> &atomList, long int subID, const Vector &offset, const Matrix &offsetRotation)
{
    Atom *pAtom;

    // Try to find existing subgroup with that ID to add to
    // Couldn't find any, so make a new one for the new ID so we can add to it
	if (m_SubGroups.find(subID) == m_SubGroups.end())
	{
		m_SubGroups.insert(pair<long int, list<Atom *> >(subID, list<Atom *>())).first;
	}
    for (list<Atom *>::const_iterator itr = atomList.begin(); itr != atomList.end(); ++itr)
    {
        pAtom = new Atom(**itr);
        pAtom->SetSubID(subID);
        pAtom->SetOffset(offset + (pAtom->GetOriginalOffset() * offsetRotation));
        pAtom->SetOwner(m_pOwnerMO);
        // Put ownership here
        m_Atoms.push_back(pAtom);

        // Add the atom to the subgroup in the SubGroups map, not transferring ownership
        m_SubGroups.find(subID)->second.push_back(pAtom);
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
	if (m_SubGroups.find(subID) == m_SubGroups.end())
	{
		return false;
	}
	DAssert(m_SubGroups.find(subID)->second.empty(), "Found empty atom subgroup list!?");

	for (list<Atom *>::const_iterator aItr = m_SubGroups.find(subID)->second.begin(); aItr != m_SubGroups.find(subID)->second.end(); ++aItr)
	{
		// Re-set ID just to make sure
		(*aItr)->SetSubID(subID);
		(*aItr)->SetOffset(newOffset + ((*aItr)->GetOriginalOffset() * newOffsetRotation));
		//pAtom->SetOwner(m_pOwnerMO);
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
// Description:     Makes the group of Atom:s travel together and react when terrain is
//                  hit. Effects are direcly applied to the owning MovableObject.

float AtomGroup::Travel(const float travelTime,
                        bool callOnBounce,
                        bool callOnSink,
                        bool scenePreLocked)
{
    if (!m_pOwnerMO)
    {
        DDTAbort("Travelling an AtomGroup without a parent MO!");
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
// Description:     Makes the group of Atom:s travel together and react when terrain is
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

                // All atoms whose (progress += velRatio) >= ceilf(prevProgress), take a step to their
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
        DDTAbort("Travelling an AtomGroup without a parent MO!");
        return travelTime;
    }
    m_MomInertia = GetMomentOfInertia();
    bool hFlipped = m_pOwnerMO->m_HFlipped;

//    g_FrameMan.SaveBitmapToBMP(g_SceneMan.GetMOIDBitmap(), "MOIDTest");

    int segCount = 0, stepCount = 0, stepsOnSeg = 0, hitCount = 0, atomsHitMOsCount = 0;
    float distMass, distMI, hitFactor, timeLeft = travelTime;
    float rotDelta, longestTrajMag, segProgress;
    float segRatio, preHitRot, radMag, retardation;
    bool hitStep, newDir, halted = false, hitMOs = m_pOwnerMO->m_HitsMOs;
    Atom *pFastestAtom = 0;
    map<MOID, list<Atom *> > hitMOAtoms;
    map<MOID, list<Atom *> >::iterator mapMOItr;
    list<Atom *> hitTerrAtoms;
    list<Atom *> penetratingAtoms;
    list<Atom *> hitResponseAtoms;
    list<Atom *>::iterator aItr;
    Vector linSegTraj, startOff, targetOff, atomTraj, tempVec, tempVel, preHitPos, hitNormal;
    MOID tempMOID = g_NoMOID;
    HitData hitData;

// TODO: Make this dependent on agroup radius!, not hcoded")
    float segRotLimit = PI / 6;

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
        for (aItr = m_Atoms.begin(); aItr != m_Atoms.end(); ++aItr)
        {
            startOff = (*aItr)->GetOffset().GetXFlipped(hFlipped);
            startOff *= rotation;

            if ((*aItr)->SetupPos(position + startOff))
            {
                hitData.Reset();
                if ((*aItr)->IsIgnoringTerrain())
                {
                    // Calc and store the accurate hit radius of the Atom in relation to the CoM
                    tempVec = (*aItr)->GetOffset().GetXFlipped(hFlipped);
                    hitData.hitRadius[HITOR] = tempVec.RadRotate(rotation.GetRadAngle()) *= g_FrameMan.GetMPP();
                    // Figure out the pre-collision velocity of the hitting atom due to body translation and rotation.
                    hitData.hitVel[HITOR] = velocity + tempVec.Perpendicularize() * angVel;
/*
                    radMag = hitData.hitRadius[HITOR].GetMagnitude();
                    // These are set temporarily here, will be re-set later when the normal of the hit terrain bitmap (ortho pixel side) is known.
                    hitData.hitDenominator = (1.0 / distMass) + ((radMag * radMag) / distMI);
                    hitData.preImpulse[HITOR] = hitData.hitVel[HITOR] / hitData.hitDenominator;
                    // Set the atom with the hit data with all the info we have so far.
                    (*aItr)->SetHitData(hitData);

                    hitFactor = 1.0;//  / (float)hitTerrAtoms.size();
                    (*aItr)->GetHitData().mass[HITOR] = mass;
                    (*aItr)->GetHitData().momInertia[HITOR] = m_MomInertia;
                    (*aItr)->GetHitData().impFactor[HITOR] = hitFactor;

                    // Call the call-on-bounce function, if requested.
//                    if (m_pOwnerMO && callOnBounce)
//                        halted = halted || m_pOwnerMO->OnBounce((*aItr)->GetHitData());

                    // Compute and store this Atom's collision response impulse force.
                    // Calc effects of moment of inertia will have on the impulse.
                    float MIhandle = m_LastHit.hitRadius[HITOR].GetPerpendicular().Dot(m_LastHit.bitmapNormal);
*/
                    if (!(*aItr)->GetNormal().IsZero())
                    {
                        hitData.resImpulse[HITOR] = m_pOwnerMO->RotateOffset((*aItr)->GetNormal());
                        hitData.resImpulse[HITOR] = -hitData.resImpulse[HITOR];
                        hitData.resImpulse[HITOR].SetMagnitude(hitData.hitVel[HITOR].GetMagnitude());
//                        hitData.resImpulse[HITOR].SetMagnitude(hitData.hitVel[HITOR].GetMagnitude());

                        // Apply terrain conflict response
                        velocity += hitData.resImpulse[HITOR] / mass;
                        angVel += hitData.hitRadius[HITOR].GetPerpendicular().Dot(hitData.resImpulse[HITOR]) / m_MomInertia;
                        // Accumulate all the impulse forces so the MO can determine if it took damaged as a result
                        totalImpulse += hitData.resImpulse[HITOR];
                    }
// Dangerous, will cause oscillations
//                    else
//                        hitData.resImpulse[HITOR] = -hitData.hitVel[HITOR];
                }
            }
#ifdef _DEBUG
            // Draw the positions of the atoms at the start of each segment, for visual debugging.
            putpixel(g_SceneMan.GetMOColorBitmap(), (*aItr)->GetCurrentPos().m_X, (*aItr)->GetCurrentPos().m_Y, 122);
#endif //_DEBUG
        }

        // Compute and scale the actual on-screen travel trajectory of the origin of thid AtomGroup
        // for this segment, using the remaining travel time and the pixels-per-meter constant.
        linSegTraj = velocity * timeLeft * g_FrameMan.GetPPM();

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
            segRatio = 1.0;

        segProgress = 0.0;
        hitStep = false;
        atomsHitMOsCount = 0;

        if (linSegTraj.IsZero() && rotDelta == 0)
            break;

        hitMOAtoms.clear();
        hitTerrAtoms.clear();
        penetratingAtoms.clear();
        hitData.Reset();
        hitResponseAtoms.clear();

        longestTrajMag = 0.0;

        for (aItr = m_Atoms.begin(); aItr != m_Atoms.end(); ++aItr)
        {
            // Calc the segment trajectory for each individual Atom, with rotations considered.
//            startOff = (position + (*aItr)->GetOffset().GetXFlipped(hFlipped)) - position.GetFloored();
// TODO: Get flipping working inside the matrix instead!")
            startOff = (*aItr)->GetOffset().GetXFlipped(hFlipped);
//            startOff.RadRotate(/*hFlipped ? (PI + rotation) :*/ rotation);
            startOff *= rotation;

            // Get the atom's travel direction due to body rotation.
            // This is tangent to the atom's body CoM offset, and scaled to the delta rotation.
            targetOff = startOff.GetPerpendicular().GetNormalized();
            targetOff *= tan(rotDelta) * startOff.GetMagnitude();

            // Set up the inital rasterized step for each Atom and save the longest trajectory
            if ((*aItr)->SetupSeg(position + startOff, linSegTraj + targetOff) > stepsOnSeg)
            {
                pFastestAtom = *aItr;
                stepsOnSeg = (*aItr)->GetStepsLeft();
            }
        }

        // If the longest seg is 0 steps, there is no movement or rotation, so quit.
//        if (stepsOnSeg == 0)
//            break;

        for (aItr = m_Atoms.begin(); aItr != m_Atoms.end(); ++aItr)
//            (*aItr)->SetStepRatio((*aItr)->GetSegLength() / longestTrajMag);
            (*aItr)->SetStepRatio((float)(*aItr)->GetStepsLeft() / (float)stepsOnSeg);


        // STEP LOOP //////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////
        for (stepCount = 0; !hitStep && stepsOnSeg != 0 && stepCount < stepsOnSeg; ++stepCount)
        {

            // SCENE COLLISION DETECTION //////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////////////////////

            for (aItr = m_Atoms.begin(); aItr != m_Atoms.end(); ++aItr)
            {
                // Take one step, and check if the atom hit anything
                if ((*aItr)->StepForward())
                {
                    //  So something was hit, first check for terrain hit.
                    if ((*aItr)->HitWhatTerrMaterial())
                    {
						m_pOwnerMO->SetHitWhatTerrMaterial((*aItr)->HitWhatTerrMaterial());
                        hitTerrAtoms.push_back(*aItr);
                    }

                    // MO hits?
                    if (hitMOs && (tempMOID = (*aItr)->HitWhatMOID()) != g_NoMOID)
                    {
						m_pOwnerMO->m_MOIDHit = tempMOID;

						MovableObject * pMO = g_MovableMan.GetMOFromID(tempMOID);
						if (pMO)
						{
							if (pMO->HitWhatMOID() == g_NoMOID)
								pMO->SetHitWhatMOID(m_pOwnerMO->m_MOID);
						}

                        // Yes, MO hit. See if we already have another atom hitting this MO in this step.
                        mapMOItr = hitMOAtoms.find(tempMOID);
                        // If not, then create a new deque unique for that MO's ID
                        // and insert into the map of MO-hitting Atom:s.
                        if (mapMOItr == hitMOAtoms.end())
                        {
                            list<Atom *> newDeque;
                            newDeque.push_back(*aItr);
                            hitMOAtoms.insert(pair<MOID, list<Atom *> >(tempMOID, newDeque));
                        }
                        // If another Atom of this group has already hit this same MO
                        // during this step, go ahead and add the new atom to the
                        // corresponding deque for that MOID.
                        else
                            ((*mapMOItr).second).push_back(*aItr);

                        // Add the hit MO to the ignore list of ignored MOIDs
//                        AddMOIDToIgnore(tempMOID);

                        // Count the number of Atoms of this group that hit MO:s this step.
                        // Used to properly distribute the mass of the owner MO in later
                        // collision responses during this step.
                        atomsHitMOsCount++;
                    }
//                  else
//                      DDTAbort("Atom reported hit to AtomGroup, but then reported neither MO or Terr hit!");

#ifdef _DEBUG
                    Vector tPos = (*aItr)->GetCurrentPos();
                    Vector tNorm = m_pOwnerMO->RotateOffset((*aItr)->GetNormal()) * 7;
                    line(g_SceneMan.GetMOColorBitmap(), tPos.m_X, tPos.m_Y, tPos.m_X + tNorm.m_X, tPos.m_Y + tNorm.m_Y, 244);
                    // Draw the positions of the hitpoints on screen for easy debugging.
//                    putpixel(g_SceneMan.GetMOColorBitmap(), tPos.m_X, tPos.m_Y, 5);
#endif //_DEBUG
                }
            }

            // If no collisions, continue on to the next step.
            if (hitTerrAtoms.empty() && hitMOAtoms.empty())
                continue;

            // There are colliding Atom:s, therefore the group hit something.
            hitStep = true;
            ++hitCount;

            // Calculate the progress made on this segment before hitting something.
            // Special case of being at rest
            if (stepCount == 0 && stepsOnSeg == 1)
            {
                segProgress = 0.0f;
                halted = true;
//                m_pOwnerMO->SetToSettle(true);
            }
            // Normal travel
            else
            {
                // Hit on first atom, but has farther to go
                if (stepCount == 0)
                    segProgress = 0.75f / (float)stepsOnSeg;
                else
                    segProgress = (float)stepCount / (float)stepsOnSeg;
            }

            // Move position forward to the hit position.
            preHitPos = position;
            position += linSegTraj * segProgress;
            // Wrap position, if necessary.
            didWrap = g_SceneMan.WrapPosition(position) || didWrap;
            // Move rotation forward according to the progress made on the segment.
            preHitRot = rotation.GetRadAngle();
            rotation += rotDelta * segProgress;

            // TERRAIN COLLISION RESPONSE /////////////////////////////////////////////////////
            // Determine which of the colliding Atom:s will penetrate the terrain.
            do
            {
                penetratingAtoms.clear();

                distMass = mass / (hitTerrAtoms.size() * (m_Resolution ? m_Resolution : 1));
                distMI = m_MomInertia / (hitTerrAtoms.size() * (m_Resolution ? m_Resolution : 1));

                for (aItr = hitTerrAtoms.begin(); aItr != hitTerrAtoms.end(); )
                {
                    // Calc and store the accurate hit radius of the Atom in relation to the CoM
                    tempVec = (*aItr)->GetOffset().GetXFlipped(hFlipped);
                    hitData.hitRadius[HITOR] = tempVec.RadRotate(rotation.GetRadAngle()) *= g_FrameMan.GetMPP();
                    // Figure out the pre-collision velocity of the hitting atom due to body translation and rotation.
                    hitData.hitVel[HITOR] = velocity + tempVec.Perpendicularize() * angVel;

                    radMag = hitData.hitRadius[HITOR].GetMagnitude();
                    // These are set temporarily here, will be re-set later when the normal of the hit terrain bitmap (ortho pixel side) is known.
                    hitData.hitDenominator = (1.0 / distMass) + ((radMag * radMag) / distMI);
                    hitData.preImpulse[HITOR] = hitData.hitVel[HITOR] / hitData.hitDenominator;
                    // Set the atom with the hit data with all the info we have so far.
                    (*aItr)->SetHitData(hitData);

//                    float test1 = hitData.preImpulse[HITOR].GetMagnitude();

                    if (g_SceneMan.WillPenetrate((*aItr)->GetCurrentPos().m_X, (*aItr)->GetCurrentPos().m_Y, hitData.preImpulse[HITOR]))
                    {
                        // Move the penetrating atom to the pen. list from the coll. list.
                        penetratingAtoms.push_back(*aItr);
                        aItr = hitTerrAtoms.erase(aItr);
                    }
                    else
                        ++aItr;
                }
            }
            while (!hitTerrAtoms.empty() && !penetratingAtoms.empty());

            // TERRAIN BOUNCE //////////////////////////////////////////////////////////////////
            // If some Atom:s could not penetrate even though all the impulse was on them,
            // gather the bounce results and apply them to the owner.
            if (!hitTerrAtoms.empty())
            {
//                m_NumPenetrations = 0;
                newDir = true;

                // Step back all atoms that previously took one during this step iteration.
                // This is so we aren't intersecting the hit MO anymore.
//                for (aItr = m_Atoms.begin(); aItr != m_Atoms.end(); ++aItr)
                for (aItr = hitTerrAtoms.begin(); aItr != hitTerrAtoms.end(); ++aItr)
                    (*aItr)->StepBack();

                // Calculate the distributed mass that each bouncing Atom has.
//                distMass = mass /*/ (hitTerrAtoms.size() * (m_Resolution ? m_Resolution : 1))*/;
//                distMI = m_MomInertia/* / (hitTerrAtoms.size() * (m_Resolution ? m_Resolution : 1))*/;
                hitFactor = 1.0 / (float)hitTerrAtoms.size();

                // Gather the collision response effects so that the impulse force can be calculated.
                for (aItr = hitTerrAtoms.begin(); aItr != hitTerrAtoms.end(); ++aItr)
                {
                    (*aItr)->GetHitData().mass[HITOR] = mass;
                    (*aItr)->GetHitData().momInertia[HITOR] = m_MomInertia;
                    (*aItr)->GetHitData().impFactor[HITOR] = hitFactor;

                    // Get the hitdata so far gathered for this Atom.
//                  hitData = (*aItr)->GetHitData();

                    // Call the call-on-bounce function, if requested.
                    if (m_pOwnerMO && callOnBounce)
                        halted = halted || m_pOwnerMO->OnBounce((*aItr)->GetHitData());

                    // Copy back the new hit data with all the info we have so far.
//                  (*aItr)->SetHitData(hitData);

                    // Compute and store this Atom's collision response impulse force.
                    (*aItr)->TerrHitResponse();
                    hitResponseAtoms.push_back(*aItr);
                }
            }
            // TERRAIN SINK ////////////////////////////////////////////////////////////////
            // Else all Atom:s must have penetrated and therefore the entire group has
            // sunken into the terrain. Get the penetration resistance results and
            // apply them to the owner.
            else if (!penetratingAtoms.empty())
            {
                // Re-calculate the progress made on this segment before hitting something.
                // We do this because the hitting step made resulted in a terrain sink,
                // which means the segment progressed another step, and the segmentProgress
                // should reflect this.
                segProgress = (float)(stepCount + 1) / (float)stepsOnSeg;

// TODO: Wrapping detection here may be screwed up on very special cases.")
                // Move position forward to the sink hit position.
                position = preHitPos + linSegTraj * segProgress;
                // Wrap position, if necessary.
                didWrap = g_SceneMan.WrapPosition(position) || didWrap;
                // Move rotation forward according to the progress made on the segment, incl sink.
                rotation = preHitRot + rotDelta * segProgress;
/*
                // Adjust the radius etc to account for the sinking into the terrain.
                (*aItr)->SetHitRadius((*aItr)->GetOffset().RadRotate(rotation));
                // Figure out the pre-collision velocity of the
                // hitting atom due to body translation and rotation.
                (*aItr)->SetHitVel(velocity + (*aItr)->GetOffset().RadRotate(rotation + HalfPI) *
                                            g_FrameMan.GetMPP() * angVel);
*/
                hitFactor = 1.0 / (float)penetratingAtoms.size();

                // Calc and store the collision response effects.
                for (aItr = penetratingAtoms.begin(); aItr != penetratingAtoms.end(); ++aItr)
                {


                    // This gets re-set later according to the ortho pixel edges hit.
//                  hitData.bitmapNormal = -(hitData.hitVel[HITOR].GetNormalized());
//                  hitData.squaredMIHandle[HITOR] = hitData.hitRadius[HITOR].GetPerpendicular()/*.Dot(hitData.bitmapNormal)*/;
//                  hitData.squaredMIHandle[HITOR] *= hitData.squaredMIHandle[HITOR];
//                  hitData.hitDenominator = (1.0 / distMass) + (hitData.squaredMIHandle[HITOR] / distMI);
//                  hitData.preImpulse[HITOR] = hitData.hitVel[HITOR] / hitData.hitDenominator;

                    // Get the hitdata so far gathered for this Atom.
                    hitData = (*aItr)->GetHitData();

                    if (g_SceneMan.TryPenetrate((*aItr)->GetCurrentPos().m_X,
                                                (*aItr)->GetCurrentPos().m_Y,
                                                hitData.preImpulse[HITOR],
                                                hitData.hitVel[HITOR],
                                                retardation,
                                                1.0,
                                                1/*(*aItr)->GetNumPenetrations()*/))
                    {

                        // Recalc these here without the distributed mass and MI.
                        radMag = hitData.hitRadius[HITOR].GetMagnitude();
                        hitData.hitDenominator = (1.0 / mass) + ((radMag * radMag) / m_MomInertia);
                        hitData.preImpulse[HITOR] = hitData.hitVel[HITOR] / hitData.hitDenominator;
                        hitData.mass[HITOR] = mass;
                        hitData.momInertia[HITOR] = m_MomInertia;
                        hitData.impFactor[HITOR] = hitFactor;
                        // Finally calculate the hit response impulse.
                        hitData.resImpulse[HITOR] = ((hitData.hitVel[HITOR] * retardation) /
                                                     hitData.hitDenominator) * hitFactor;

                        // Call the call-on-sink function, if requested.
                        if (callOnSink)
                            halted = halted || m_pOwnerMO->OnSink(hitData);

                        // Copy back the new hit data with all the info we have so far.
                        (*aItr)->SetHitData(hitData);
                        // Save the atom for later application of its hit data to the body.
                        hitResponseAtoms.push_back(*aItr);
                    }
                }
            }

            // MOVABLEOBJECT COLLISION RESPONSE ///////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////////////////////
            if (hitMOs && !hitMOAtoms.empty())
            {
                newDir = true;

                // Step back all atoms that hit MO:s during this step iteration.
                // This is so we aren't intersecting the hit MO anymore.
                for (mapMOItr = hitMOAtoms.begin(); mapMOItr != hitMOAtoms.end(); ++mapMOItr)
                {
					for (aItr = (*mapMOItr).second.begin(); aItr != (*mapMOItr).second.end(); ++aItr)
						(*aItr)->StepBack();
//                    for (aItr = m_Atoms.begin(); aItr != m_Atoms.end(); ++aItr)
//                      (*aItr)->StepBack();
                }

                // Set the mass and other data pertaining to the hitor,
                // aka this AtomGroup's owner MO.
                hitData.mass[HITOR] = mass;
                hitData.momInertia[HITOR] = m_MomInertia;
                hitData.impFactor[HITOR] = 1.0 / (float)atomsHitMOsCount;

                for (mapMOItr = hitMOAtoms.begin(); mapMOItr != hitMOAtoms.end(); ++mapMOItr)
                {
                    // The denominator that the MovableObject being hit should
                    // divide its mass with for each atom of this AtomGroup that is
                    // colliding with it during this step.
                    hitData.impFactor[HITEE] = 1.0 / (float)((*mapMOItr).second.size());

                    for (aItr = (*mapMOItr).second.begin(); aItr != (*mapMOItr).second.end(); ++aItr)
                    {
//                      hitData.hitPoint = (*aItr)->GetCurrentPos();
                        // Calc and store the accurate hit radius of the Atom in relation to the CoM
                        tempVec = (*aItr)->GetOffset().GetXFlipped(hFlipped);
                        hitData.hitRadius[HITOR] = tempVec.RadRotate(rotation.GetRadAngle()) *= g_FrameMan.GetMPP();
                        // Figure out the pre-collision velocity of the hitting atom due to body translation and rotation.
                        hitData.hitVel[HITOR] = velocity + tempVec.Perpendicularize() * angVel;
                        // Set the atom with the hit data with all the info we have so far.
                        (*aItr)->SetHitData(hitData);
                        // Let the atom calc the impulse force resulting from the collision., and only add it if collision is valid
                        if ((*aItr)->MOHitResponse())
                        {
                            // Report the hit to both MO's in collision
                            HitData &hd = (*aItr)->GetHitData();
                            // Don't count collision if either says tehy got terminated
                            if (!hd.pRootBody[HITOR]->OnMOHit(hd) && !hd.pRootBody[HITEE]->OnMOHit(hd))
                            {
                                // Save the filled out atom in the list for later application in this step.
                                hitResponseAtoms.push_back(*aItr);
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
            for (aItr = hitResponseAtoms.begin(); aItr != hitResponseAtoms.end(); ++aItr)
            {
// TODO: Investigate damping!")
// TODO: Clean up here!#$#$#$#")
                hitData = (*aItr)->GetHitData();
//                  tempVec = hitData.resImpulse[HITOR];
                velocity += hitData.resImpulse[HITOR] / mass;
                angVel += hitData.hitRadius[HITOR].GetPerpendicular().Dot(hitData.resImpulse[HITOR]) / m_MomInertia;
                // Accumulate all the impulse forces so the MO can determine if it took damaged as a result
                totalImpulse += hitData.resImpulse[HITOR];
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
            segProgress = 1.0;
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
//            DDTAbort("AtomGroup travel resulted in more than 1000 segs!!");
            break;
        }
    }
    while (segRatio != 1.0 || hitStep && /*!linSegTraj.GetFloored().IsZero() &&*/ !halted);

    // Resolove all MO intersections!
    ResolveMOSIntersection(position, rotation);

    // Unlock the scene
    if (!scenePreLocked)
        g_SceneMan.UnlockScene();

    // Clear out the MOIDs from the ignore list.
    ClearMOIDIgnoreList();

    // If too many Atom:s are ignoring terrain, make a hole for the body so they won't
    int ignoreCount = 0;
    int maxIgnore = m_Atoms.size() / 2;
    for (aItr = m_Atoms.begin(); aItr != m_Atoms.end(); ++aItr)
    {
        if ((*aItr)->IsIgnoringTerrain())
        {
            if (++ignoreCount >= maxIgnore)
            {
                m_pOwnerMO->ForceDeepCheck();
                break;
            }
        }
    }

    // Travel along the remaining trajectory if we didn't
    // hit anyhting on the last segment and weren't told to halt.
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
// Description:     Makes the group of Atom:s travel together and react when terrain is
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
        DDTAbort("Travelling an AtomGroup without a parent MO!");
        return Vector();
    }

    bool hFlipped = m_pOwnerMO->m_HFlipped;

    // Compute and scale the actual on-screen travel trajectory for this travel, using
    // the passed in travel time and the pixels-per-meter constant.
    Vector trajectory = velocity * travelTime * g_FrameMan.GetPPM();

    int legCount = 0, stepCount = 0, hitCount = 0, atomsHitMOsCount = 0;
    int /*startPos[2], */intPos[2], hitPos[2], delta[2], delta2[2], increment[2];
    int error, prevError, dom, sub, domSteps, subSteps;
    float prevVelMag, prevTrajMag, prevTimeLeft, mass = m_pOwnerMO->GetMass(), massDist, timeLeft = travelTime, retardation;
// TODO: Fix HitMOs issue!!")
    bool hit[2], subStepped, newDir, endOfTraj = false, halted = false, hitMOs = false/*m_pOwnerMO->m_HitsMOs*/, sinkHit = false;
    unsigned char hitMaterialID, domMaterialID, subMaterialID;
	Material const * hitMaterial = g_SceneMan.GetMaterialFromID(g_MaterialAir);
	Material const * domMaterial = g_SceneMan.GetMaterialFromID(g_MaterialAir);
	Material const * subMaterial = g_SceneMan.GetMaterialFromID(g_MaterialAir);
    map<MOID, set<Atom *> >MOIgnoreMap;
    map<MOID, set<Atom *> >::iterator igItr;
    map<MOID, deque<pair<Atom *, Vector> > > hitMOAtoms;
    map<MOID, deque<pair<Atom *, Vector> > >::iterator mapItr;
    deque<pair<Atom *, Vector> > hitTerrAtoms;
    deque<pair<Atom *, Vector> > penetratingAtoms;
    deque<pair<Atom *, Vector> >::iterator aoItr;
    list<Atom *>::iterator aItr;
    // First Vector is the impulse force in kg * m/s, the second is force point,
    // or its offset from the origin of the AtomGroup.
    deque<pair<Vector, Vector> > impulseForces;
    deque<pair<Vector, Vector> >::iterator ifItr;
//    deque<Vector> angVelResults;
    Vector rotatedOffset, tempVel, legProgress, forceVel, returnPush;
    MOID tempMOID = g_NoMOID;
    HitData hitData;

    // Init the raw positions.
    /*startPos[X] = */intPos[X] = floorf(position.m_X);
    /*startPos[Y] = */intPos[Y] = floorf(position.m_Y);

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
        for (aItr = m_Atoms.begin(); aItr != m_Atoms.end(); ++aItr)
        {
            rotatedOffset = (*aItr)->GetOffset().GetXFlipped(hFlipped);
//            rotatedOffset.AbsRadRotate(-rotation);
            rotatedOffset.Floor();
            // See if the atom is starting out on top of another MO
            if (g_NoMOID != (tempMOID = g_SceneMan.GetMOIDPixel(intPos[X] + rotatedOffset.m_X,
                                                                intPos[Y] + rotatedOffset.m_Y)))
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
                if (MOIgnoreMap.end() != (igItr = MOIgnoreMap.find(tempMOID)))
                    // Found an entry for this MOID, so add the Atom entry to it
                    (*igItr).second.insert(*aItr);
                // There wasnt already an entry for this MOID,
                // so create one and add the Atom to it.
                else {
                    set<Atom *> newSet;
                    newSet.insert(*aItr);
                    MOIgnoreMap.insert(pair<MOID, set<Atom *> >(tempMOID, newSet));
                }
            }
        }
    }

    // Loop for all the different straight legs (between bounces etc) that
    // have to be traveled during the travelTime.
    do {
        intPos[X] = floorf(position.m_X);
        intPos[Y] = floorf(position.m_Y);

        prevVelMag = velocity.GetMagnitude();
        prevTrajMag = trajectory.GetMagnitude();
        // Compute and scale the actual on-screen travel trajectory for this leg, using
        // the remaining travel time and the pixels-per-meter constant.
        trajectory = velocity * timeLeft * g_FrameMan.GetPPM();

        delta[X] = floorf(position.m_X + trajectory.m_X) - intPos[X];
        delta[Y] = floorf(position.m_Y + trajectory.m_Y) - intPos[Y];

        hit[X] = false;
        hit[Y] = false;
        sinkHit = false;
        domSteps = 0;
        subSteps = 0;
        subStepped = false;
        atomsHitMOsCount = 0;

        if (delta[X] == 0 && delta[Y] == 0)
            break;

        hitMaterialID = domMaterialID = subMaterialID = 0;
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

            for (aItr = m_Atoms.begin(); aItr != m_Atoms.end(); ++aItr)
            {
//                  rotatedOffset = (*aItr)->GetOffset().GetYFlipped(hFlipped);
                rotatedOffset = (*aItr)->GetOffset().GetXFlipped(hFlipped);
//                rotatedOffset.AbsRadRotate(-rotation);
                rotatedOffset.Floor();

                // First check if we hit any MO's, if applicable.
                bool ignoreHit = false;
                if (hitMOs)
                {
                    tempMOID = g_SceneMan.GetMOIDPixel(intPos[X] + rotatedOffset.m_X,
                                                       intPos[Y] + rotatedOffset.m_Y);

                    // Check the ignore map for Atom:s that should ignore hits against certain MO:s
                    if (tempMOID != g_NoMOID && (igItr = MOIgnoreMap.find(tempMOID)) != MOIgnoreMap.end())
                        ignoreHit = (*igItr).second.find(*aItr) != (*igItr).second.end();
                }

                if (hitMOs && tempMOID && !ignoreHit)
                {
                    // See if we already have another atom hitting this MO in this step.
                    mapItr = hitMOAtoms.find(tempMOID);
                    // If not, then create a new deque unique for that MO's ID
                    // and insert into the map of MO-hitting Atom:s.
                    if (mapItr == hitMOAtoms.end())
                    {
                        deque<pair<Atom *, Vector> > newDeque;
                        newDeque.push_back(pair<Atom *, Vector>(*aItr, rotatedOffset));
                        hitMOAtoms.insert(pair<MOID, deque<pair<Atom *, Vector> > >(tempMOID, newDeque));
                    }
                    // If another Atom of this group has already hit this same MO
                    // during this step, go ahead and add the new atom to the
                    // corresponding deque for that MOID.
                    else
                        ((*mapItr).second).push_back(pair<Atom *, Vector>(*aItr, rotatedOffset));
                    // Count the number of Atoms of this group that hit MO:s this step.
                    // Used to properly distribute the mass of the owner MO in later
                    // collision responses during this step.
                    atomsHitMOsCount++;
                }
                // If no MO has ever been hit yet during this step, then keep checking for terrain hits.
                else if (atomsHitMOsCount == 0 && g_SceneMan.GetTerrMatter(intPos[X] + rotatedOffset.m_X,
                                                                           intPos[Y] + rotatedOffset.m_Y))
                    hitTerrAtoms.push_back(pair<Atom *, Vector>(*aItr, rotatedOffset));
/*
#ifdef _DEBUG
                // Draw the positions of the hitpoints on screen for easy debugging.
                putpixel(g_SceneMan.GetMOColorBitmap(), floorf(position.m_X + rotatedOffset.m_X), floorf(position.m_Y + rotatedOffset.m_Y), 122);
#endif //_DEBUG
*/
            }

            // If no collisions, continue on to the next step.
            if (hitTerrAtoms.empty() && hitMOAtoms.empty())
                continue;

            // There are colliding Atom:s, therefore the group hit something.
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
            (forceVel = velocity).CapMagnitude((pushForce * timeLeft) / mass);

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
                hitData.mass[HITOR] = mass;
                hitData.momInertia[HITOR] = 1.0;
                hitData.impFactor[HITOR] = 1.0 / (float)atomsHitMOsCount;
                // Figure out the pre-collision velocity of the
                // hitting atoms due to the max push force allowed.
                hitData.hitVel[HITOR] = forceVel;

                // The distributed mass of one hitting atom of the
                // hitting (this AtomGroup's owner) MovableObject.
//                float hitorMass = mass / ((atomsHitMOsCount/* + hitTerrAtoms.size()*/) *
//                                       (m_Resolution ? m_Resolution : 1));
//                float hiteeMassDenom = 0;

                for (mapItr = hitMOAtoms.begin(); mapItr != hitMOAtoms.end(); ++mapItr)
                {
                    // The denominator that the MovableObject being hit should
                    // divide its mass with for each atom of this AtomGroup that is
                    // colliding with it during this step.
                    hitData.impFactor[HITEE] = 1.0 / (float)((*mapItr).second.size());

                    for (aoItr = (*mapItr).second.begin(); aoItr != (*mapItr).second.end(); ++aoItr)
                    {
                        // Bake in current Atom's offset into the int positions.
                        intPos[X] += (*aoItr).second.m_X;
                        intPos[Y] += (*aoItr).second.m_Y;
                        hitPos[X] += (*aoItr).second.m_X;
                        hitPos[Y] += (*aoItr).second.m_Y;

//                      hitData.hitPoint.SetXY(intPos[X], intPos[Y]);
                        // Calc and store the accurate hit radius of the Atom in relation to the CoM
                        hitData.hitRadius[HITOR] = (*aoItr).second * g_FrameMan.GetMPP();
                        hitData.hitPoint.Reset();
                        hitData.bitmapNormal.Reset();

                        // Check for the collision point in the dominant direction of travel.
                        if (delta[dom] && ((dom == X && g_SceneMan.GetMOIDPixel(hitPos[X], intPos[Y]) != g_NoMOID) ||
                                           (dom == Y && g_SceneMan.GetMOIDPixel(intPos[X], hitPos[Y]) != g_NoMOID)))
                        {
                            hit[dom] = true;
                            hitData.hitPoint = dom == X ? Vector(hitPos[X], intPos[Y]) :
                                                          Vector(intPos[X], hitPos[Y]);
                            hitData.bitmapNormal[dom] = -increment[dom];
                        }

                        // Check for the collision point in the submissive direction of travel.
                        if (subStepped && delta[sub] && ((sub == X && g_SceneMan.GetMOIDPixel(hitPos[X], intPos[Y]) != g_NoMOID) ||
                                                         (sub == Y && g_SceneMan.GetMOIDPixel(intPos[X], hitPos[Y]) != g_NoMOID)))
                        {
                            hit[sub] = true;
//                            if (hitData.hitPoint.IsZero())
                                hitData.hitPoint = sub == X ? Vector(hitPos[X], intPos[Y]) :
                                                              Vector(intPos[X], hitPos[Y]);
// NOTE: THis can actually be wrong since there may not in fact be a corner pixel, but two pixels hit on X and Y directions
                            // We hit pixels in both sub and dom directions on the other MO, a corner hit.
//                            else
//                                hitData.hitPoint.SetXY(hitPos[X], hitPos[Y]);
                            hitData.bitmapNormal[sub] = -increment[sub];
                        }

                        // If neither the direct dominant or sub directions yielded a collision point, then
                        // that means we hit right on the corner of a pixel, and that is the collision point.
                        if (!hit[dom] && !hit[sub])
                        {
                            hit[dom] = hit[sub] = true;
                            hitData.hitPoint.SetXY(hitPos[X], hitPos[Y]);
                            hitData.bitmapNormal.SetXY(-increment[X], -increment[Y]);
                        }
                        hitData.bitmapNormal.Normalize();

                        // Extract the current Atom's offset from the int positions.
                        intPos[X] -= (*aoItr).second.m_X;
                        intPos[Y] -= (*aoItr).second.m_Y;
                        hitPos[X] -= (*aoItr).second.m_X;
                        hitPos[Y] -= (*aoItr).second.m_Y;

                        MOID hitMOID = g_SceneMan.GetMOIDPixel(hitData.hitPoint.m_X, hitData.hitPoint.m_Y);

                        if (hitMOID != g_NoMOID)
                        {
                            hitData.pBody[HITOR] = m_pOwnerMO;
                            hitData.pBody[HITEE] = g_MovableMan.GetMOFromID(hitMOID);
                            DAssert(hitData.pBody[HITEE], "Hitee MO is 0 in AtomGroup::PushTravel!");

                            hitData.pBody[HITEE]->CollideAtPoint(hitData);

                            // Save the impulse force resulting from the MO collision response calc.
                            impulseForces.push_back(make_pair(hitData.resImpulse[HITOR], (*aoItr).second));
                        }
                    }
                }
                // If any MO's were hit, continue on to the next leg without doing terrain stuff now.
                // Any terrain collisions will be taken care of on the next leg, when the effects of
                // these MO hits have been applied.
                hitTerrAtoms.clear();
            }

            // TERRAIN COLLISION RESPONSE /////////////////////////////////////////////////////
            // Determine which of the colliding Atom:s will penetrate the terrain.
            do {
                penetratingAtoms.clear();

                massDist = mass / hitTerrAtoms.size() * (m_Resolution ? m_Resolution : 1);

                for (aoItr = hitTerrAtoms.begin(); aoItr != hitTerrAtoms.end(); )
                {
                    if (g_SceneMan.WillPenetrate(intPos[X] + (*aoItr).second.m_X,
                                                 intPos[Y] + (*aoItr).second.m_Y,
                                                 forceVel,
                                                 massDist))
                    {
                        // Move the penetrating atom to the pen. list from the coll. list.
                        penetratingAtoms.push_back(pair<Atom *, Vector>((*aoItr).first, (*aoItr).second));
                        aoItr = hitTerrAtoms.erase(aoItr);
                    }
                    else
                        ++aoItr;
                }
            } while (!hitTerrAtoms.empty() && !penetratingAtoms.empty());

            // TERRAIN BOUNCE //////////////////////////////////////////////////////////////////
            // If some Atom:s could not penetrate even though all the mass was on them,
            // gather the bounce results and apply them to the owner.
            if (!hitTerrAtoms.empty())
            {
//                  m_NumPenetrations = 0;
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
                massDist = mass / (hitTerrAtoms.size()/* + atomsHitMOsCount*/) *
                                (m_Resolution ? m_Resolution : 1);

                // Gather the collision response effects so that the impulse force can be calculated.
                for (aoItr = hitTerrAtoms.begin(); aoItr != hitTerrAtoms.end(); ++aoItr)
                {
                    // Bake in current Atom's offset into the int positions.
                    intPos[X] += (*aoItr).second.m_X;
                    intPos[Y] += (*aoItr).second.m_Y;
                    hitPos[X] += (*aoItr).second.m_X;
                    hitPos[Y] += (*aoItr).second.m_Y;

                    // Reset the temp velocity response vector to the unaltered velocity.
                    tempVel = forceVel;

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
                        tempVel[dom] = -tempVel[dom] *
                                       (*aoItr).first->GetMaterial()->restitution *
                                       domMaterial->restitution;
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
                        tempVel[sub] = -tempVel[sub] *
                                       (*aoItr).first->GetMaterial()->restitution *
                                       subMaterial->restitution;
                    }

                    // If hit right on the corner of a pixel, bounce straight back with no friction.
                    if (!hit[dom] && !hit[sub])
                    {
                        hit[dom] = true;
                        tempVel[dom] = -tempVel[dom] *
                                       (*aoItr).first->GetMaterial()->restitution *
                                       hitMaterial->restitution;
                        hit[sub] = true;
                        tempVel[sub] = -tempVel[sub] *
                                       (*aoItr).first->GetMaterial()->restitution *
                                       hitMaterial->restitution;
                    }
                    // Calculate the effects of friction.
                    else if (hit[dom] && !hit[sub])
                    {
                        tempVel[sub] -= tempVel[sub] * (*aoItr).first->GetMaterial()->friction * domMaterial->friction;
                    }
                    else if (hit[sub] && !hit[dom])
                    {
                        tempVel[dom] -= tempVel[dom] * (*aoItr).first->GetMaterial()->friction * subMaterial->friction;
                    }

                    // Compute and store this Atom's collision response impulse force.
                    impulseForces.push_back(make_pair((tempVel - forceVel) * massDist,
                                                                      (*aoItr).second));

                    // Extract the current Atom's offset from the int positions.
                    intPos[X] -= (*aoItr).second.m_X;
                    intPos[Y] -= (*aoItr).second.m_Y;
                    hitPos[X] -= (*aoItr).second.m_X;
                    hitPos[Y] -= (*aoItr).second.m_Y;
                }
            }
            // TERRAIN SINK ////////////////////////////////////////////////////////////////
            // Else all Atom:s must have penetrated and therefore the entire group has
            // sunken into the terrain. Get the penetration resistance results and
            // apply them to the owner.
            else if (!penetratingAtoms.empty())
            {
                sinkHit = hit[dom] = hit[sub] = true;
//                  ++m_NumPenetrations;
                prevError = false;
                prevError = error;

                // Call the call-on-sink function, if requested.
//                if (callOnSink)
//                  halted = m_pOwnerMO->OnSink(position);

                massDist = mass / penetratingAtoms.size() * (m_Resolution ? m_Resolution : 1);

                // Apply the collision response effects.
                for (aoItr = penetratingAtoms.begin(); aoItr != penetratingAtoms.end(); ++aoItr)
                {
                    if (g_SceneMan.TryPenetrate(intPos[X] + (*aoItr).second.m_X,
                                                intPos[Y] + (*aoItr).second.m_Y,
                                                forceVel * massDist,
                                                forceVel,
                                                retardation,
                                                1.0,
                                                (*aoItr).first->GetNumPenetrations()))
                    {
                        
                        impulseForces.push_back(make_pair(forceVel * massDist * retardation,
                                                                          (*aoItr).second));
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
                for (ifItr = impulseForces.begin(); ifItr != impulseForces.end(); ++ifItr)
                {
                    // Cap the impulse to what the max push force is
//                  (*ifItr).first.CapMagnitude(pushForce * (travelTime/* - timeLeft*/));
                    velocity += (*ifItr).first / mass;
                    returnPush += (*ifItr).first;
                }

                // Stunt travel time if there is no more velocity
                if (velocity.IsZero())
                    timeLeft = 0;
            }
            ++stepCount;
        }
        ++legCount;
    } while ((hit[X] || hit[Y]) &&
             timeLeft > 0 &&
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
// Description:     Makes the group of Atom:s travel together as a limb, relative to the
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
        DDTAbort("Travelling an AtomGroup without a parent MO!");
        return false;
    }

    bool didWrap = false;
    Vector target, distVec, pushImpulse;

    // Pin the path to where the owner wants the joint to be.
    limbPath.SetJointPos(jointPos);
    limbPath.SetJointVel(velocity);
    limbPath.SetRotation(rotation);
    limbPath.SetFrameTime(travelTime);

    distVec = g_SceneMan.ShortestDistance(jointPos, m_LimbPos);
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
    if (pushImpulse.GetLargest() > 10000)
        pushImpulse.Reset();

    // Add the resulting impulse force, add the lever of the joint offset if set to do so
    if (affectRotation)
        m_pOwnerMO->AddImpulseForce(pushImpulse, g_SceneMan.ShortestDistance(m_pOwnerMO->GetPos(), jointPos) * g_FrameMan.GetMPP());
    else
        m_pOwnerMO->AddImpulseForce(pushImpulse, Vector());

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FlailAsLimb
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the group of Atom:s travel together as a lifeless limb, constrained
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
        DDTAbort("Travelling an AtomGroup without a parent MO!");
        return;
    }

    bool didWrap = false;
    Vector target, distVec, pushImpulse;
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
// Description:     Checks whether any of the Atom:s in this AtomGroup are on terrain
//                  pixels.

bool AtomGroup::InTerrain()
{
    AAssert(m_pOwnerMO, "Using an AtomGroup without a parent MO!");

    if (!g_SceneMan.SceneIsLocked())
        g_SceneMan.LockScene();

    bool penetrates = false;
    Vector aPos;
// TODO: UNCOMMENT
    for (list<Atom *>::iterator aItr = m_Atoms.begin(); aItr != m_Atoms.end() && !penetrates; ++aItr)
    {
        aPos = (m_pOwnerMO->GetPos() + ((*aItr)->GetOffset().GetXFlipped(m_pOwnerMO->m_HFlipped) * m_pOwnerMO->GetRotMatrix())).GetFloored();
        if (g_SceneMan.GetTerrMatter(aPos.m_X, aPos.m_Y) != g_MaterialAir)
            penetrates = true;
/*
#ifdef _DEBUG
        // Draw a dot for each atom for visual reference.
        putpixel(g_SceneMan.GetDebugBitmap(), aPos.m_X, aPos.m_Y, 112);
#endif //_DEBUG
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
    AAssert(m_pOwnerMO, "Using an AtomGroup without a parent MO!");

    int inTerrain = 0;
    Vector aPos;

    for (list<Atom *>::iterator aItr = m_Atoms.begin(); aItr != m_Atoms.end(); ++aItr)
    {
        aPos = (m_pOwnerMO->GetPos() + ((*aItr)->GetOffset().GetXFlipped(m_pOwnerMO->m_HFlipped) * m_pOwnerMO->GetRotMatrix())).GetFloored();
        if (g_SceneMan.GetTerrMatter(aPos.m_X, aPos.m_Y) != g_MaterialAir)
            inTerrain++;
    }

    return (float)inTerrain / (float)m_Atoms.size();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResolveTerrainIntersection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether any of the Atom:s in this AtomGroup are on top of
//                  terrain pixels, and if so, attempt to move the OwnerMO of this out
//                  so none of this' Atoms are inside any other MOS's silhouette anymore.

bool AtomGroup::ResolveTerrainIntersection(Vector &position, Matrix &rotation, unsigned char strongerThan)
{
    SLICK_PROFILE(0xFF335546);

    Vector atomOffset, atomPos, atomNormal, clearPos, exitDirection, atomExitVector, totalExitVector;
    list<Atom *>::iterator aItr;
    list<Atom *> intersectingAtoms;
    MOID hitMaterial = g_MaterialAir;
    float strengthThreshold = strongerThan != g_MaterialAir ? g_SceneMan.GetMaterialFromID(strongerThan)->strength : 0;
    bool rayHit = false;

    exitDirection.Reset();
    atomExitVector .Reset();
    totalExitVector.Reset();
    intersectingAtoms.clear();

    // First go through all atoms to find the first intersection and get the intersected MO
    for (aItr = m_Atoms.begin(); aItr != m_Atoms.end(); ++aItr)
    {
        atomOffset = (*aItr)->GetOffset().GetXFlipped(m_pOwnerMO->IsHFlipped());
        atomOffset *= rotation;
        (*aItr)->SetupPos(position + atomOffset);

        atomPos = (*aItr)->GetCurrentPos();
        if ((hitMaterial = g_SceneMan.GetTerrain()->GetPixel(atomPos.m_X, atomPos.m_Y)) != g_MaterialAir)
        {
            if (strengthThreshold > 0 && g_SceneMan.GetMaterialFromID(hitMaterial)->strength > strengthThreshold)
            {
                // Add atom to list of intersecting ones
                intersectingAtoms.push_back(*aItr);
            }
        }
    }

    // No intersections - we're clear!
    if (intersectingAtoms.empty())
        return true;

    // If all atoms are intersecting, we're screwed?!
    if (intersectingAtoms.size() >= m_Atoms.size())
        return false;

    // Go through all intesecting atoms and find their average inverse normal
    for (aItr = intersectingAtoms.begin(); aItr != intersectingAtoms.end(); ++aItr)
        exitDirection += m_pOwnerMO->RotateOffset((*aItr)->GetNormal());

    // We don't have a direction to go, so quit
// TODO: Maybe use previous position to create an exit direction instead then?
    if (exitDirection.IsZero())
        return false;

    // Invert and set appropriate length
    exitDirection = -exitDirection;
// TODO: Use reasonable diameter of owner MO!
    exitDirection.SetMagnitude(m_pOwnerMO->GetDiameter());

    // See which of the intersecting atoms has the longest to travel along the exit direction before it clears
    float longestDistance = 0;
    for (aItr = intersectingAtoms.begin(); aItr != intersectingAtoms.end(); ++aItr)
    {
        atomPos = (*aItr)->GetCurrentPos();

        if (strengthThreshold <= 0)
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
// Description:     Checks whether any of the Atom:s in this AtomGroup are on top of
//                  other MOSprite:s, and if so, attempt to move the OwnerMO of this out
//                  so none of this' Atoms are inside any other MOS's silhouette anymore.

bool AtomGroup::ResolveMOSIntersection(Vector &position, Matrix &rotation)
{
    if (!m_pOwnerMO->m_HitsMOs)
        return true;

    Vector atomOffset, atomPos, atomNormal, clearPos, exitDirection, atomExitVector, totalExitVector;
    list<Atom *>::iterator aItr;
    list<Atom *> intersectingAtoms;
    MOID hitMOID = g_NoMOID, currentMOID = g_NoMOID;
    MovableObject *pIntersectedMO = 0;
    MOSRotating *pIntersectedMOS = 0;
    float massA, massB, invMassA, invMassB, normMassA, normMassB;

    hitMOID = g_NoMOID;
    currentMOID = g_NoMOID;
    pIntersectedMO = 0;
    exitDirection.Reset();
    atomExitVector .Reset();
    totalExitVector.Reset();
    intersectingAtoms.clear();

    // First go through all atoms to find the first intersection and get the intersected MO
    for (aItr = m_Atoms.begin(); aItr != m_Atoms.end() && !pIntersectedMO; ++aItr)
    {
        atomOffset = (*aItr)->GetOffset().GetXFlipped(m_pOwnerMO->IsHFlipped());
        atomOffset *= rotation;
        (*aItr)->SetupPos(position + atomOffset);

        atomPos = (*aItr)->GetCurrentPos();
        if ((hitMOID = g_SceneMan.GetMOIDPixel(atomPos.m_X, atomPos.m_Y)) != g_NoMOID)
        {
            // Don't count MOIDs ignored
            if (!(*aItr)->IsIgnoringMOID(hitMOID))
            {
                // Save the correct MOID to search for other atom intersections with
                currentMOID = hitMOID;

                // Get the MO we seem to be intersecting
                pIntersectedMO = g_MovableMan.GetMOFromID(hitMOID);
                AAssert(pIntersectedMO, "Intersected MOID couldn't be translated to a real MO!");
                pIntersectedMO = pIntersectedMO->GetRootParent();

                if (pIntersectedMO->GetsHitByMOs())
                {
                    // Make that MO draw itself again in the MOID layer so we can find its true edges
                    pIntersectedMO->Draw(g_SceneMan.GetMOIDBitmap(), Vector(), g_DrawMOID, true);
    // TODO: Remove
    //                g_FrameMan.SaveBitmapToBMP(g_SceneMan.GetMOIDBitmap(), "MOIDMap");
                }
                // Continue searching
                else
                    pIntersectedMO = 0;
            }
        }
    }

    // If no intersected MO is found, we are clear and can quit
    if (!pIntersectedMO)
        return false;

    // Tell both MO's that they have hit an MO, and see if they want to continue
    if (m_pOwnerMO->OnMOHit(pIntersectedMO))
        return false;
    if (pIntersectedMO->OnMOHit(m_pOwnerMO->GetRootParent()))
        return false;

    // Restart and go through all atoms to find all intersecting the specific intersected MO
    for (aItr = m_Atoms.begin(); aItr != m_Atoms.end(); ++aItr)
    {
        atomPos = (*aItr)->GetCurrentPos();
        if (g_SceneMan.GetMOIDPixel(atomPos.m_X, atomPos.m_Y) == currentMOID)
        {
            // Add atom to list of intersecting ones
            intersectingAtoms.push_back(*aItr);
        }
    }

    // No intersections - we're clear?!
//        if (intersectingAtoms.empty())
//            return true;
//        AAssert(!intersectingAtoms.empty(), "Couldn't find any intersections after finding one?!");

    // If all atoms are intersecting, we're screwed?!
//        if (intersectingAtoms.size() >= m_Atoms.size())
//            return false;

    // Go through all intesecting atoms and find their average inverse normal
    for (aItr = intersectingAtoms.begin(); aItr != intersectingAtoms.end(); ++aItr)
        exitDirection += m_pOwnerMO->RotateOffset((*aItr)->GetNormal());

    // We don't have a direction to go, so quit
// TODO: Maybe use previous position to create an exit direction instead then?
    if (exitDirection.IsZero())
        return false;

    // Invert and set appropriate length
    exitDirection = -exitDirection;
// TODO: Use reasonable diameter of owner MO!
    exitDirection.SetMagnitude(m_pOwnerMO->GetDiameter());

    // See which of the intersecting atoms has the longest to travel along the exit direction before it clears
    float longestDistance = 0;
    for (aItr = intersectingAtoms.begin(); aItr != intersectingAtoms.end(); ++aItr)
    {
        atomPos = (*aItr)->GetCurrentPos();
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
    Vector thisExit, intersectedExit;

    // If the other object is pinned, then only move this
    if (pIntersectedMO->GetPinStrength() > 0)
        thisExit = totalExitVector;
    // Otherwise, apply the object exit vector to both this owner object, and the one it intersected, proportional to their masses
    else
    {
        massA = m_pOwnerMO->GetMass();
        massB = pIntersectedMO->GetMass();
        invMassA = 1 / massA;
        invMassB = 1 / massB;
        // Normalize the masses
        normMassA = invMassA / (invMassA + invMassB);
        normMassB = invMassB / (invMassA + invMassB);

        // If the intersected is much larger than this' MO, then only move this
        if (normMassB < 0.33)
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
    if (m_pOwnerMO->CanBeSquished() && RatioInTerrain() > 0.75)// && totalExitVector.GetMagnitude() > m_pOwnerMO->GetDiameter())
    {
        // Move back before gibbing so gibs don't end up inside terrain
        position -= thisExit;
        m_pOwnerMO->GibThis(-totalExitVector);
    }

    pIntersectedMOS = dynamic_cast<MOSRotating *>(pIntersectedMO);

    if (pIntersectedMOS && pIntersectedMOS->CanBeSquished() && pIntersectedMOS->GetAtomGroup()->RatioInTerrain() > 0.75)// && totalExitVector.GetMagnitude() > pIntersectedMO->GetDiameter())
    {
        // Move back before gibbing so gibs don't end up inside terrain
        pIntersectedMO->SetPos(pIntersectedMO->GetPos() - intersectedExit);
        pIntersectedMOS->GibThis(totalExitVector);
    }

// TODO: this isn't really true since we don't check for clearness after moving the position
    return intersectingAtoms.empty();
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
    Vector aPos, normal;
    for (list<Atom *>::const_iterator aItr = m_Atoms.begin(); aItr != m_Atoms.end(); ++aItr)
    {
        if (!useLimbPos)
            aPos = (m_pOwnerMO->GetPos() + ((*aItr)->GetOffset().GetXFlipped(m_pOwnerMO->m_HFlipped)
                                            /** m_pOwnerMO->GetRotMatrix()*/)).GetFloored();
        else
            aPos = (m_LimbPos + ((*aItr)->GetOffset().GetXFlipped(m_pOwnerMO->m_HFlipped)
                                 /* * m_pOwnerMO->GetRotMatrix()*/)).GetFloored();

        // Draw normal first
        if (!(*aItr)->GetNormal().IsZero())
        {
            normal = (*aItr)->GetNormal().GetXFlipped(m_pOwnerMO->m_HFlipped) * 5;
            line(pTargetBitmap, aPos.m_X - targetPos.m_X,
                              aPos.m_Y - targetPos.m_Y,
                              aPos.m_X - targetPos.m_X,
                              aPos.m_Y - targetPos.m_Y,
                              244);
        }

        // Then draw the atom position
        putpixel(pTargetBitmap, aPos.m_X - targetPos.m_X, aPos.m_Y - targetPos.m_Y, color);
    }

    release_bitmap(pTargetBitmap);
}

} // namespace RTE
