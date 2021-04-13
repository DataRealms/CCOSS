//////////////////////////////////////////////////////////////////////////////////////////
// File:            Scene.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Scene class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Scene.h"
#include "PresetMan.h"
#include "MovableMan.h"
#include "ConsoleMan.h"
#include "SettingsMan.h"
#include "MetaMan.h"
#include "ContentFile.h"
#include "SLTerrain.h"
#include "PathFinder.h"
#include "MovableObject.h"
#include "TerrainObject.h"
#include "Deployment.h"
#include "Loadout.h"
#include "Actor.h"
#include "BunkerAssemblyScheme.h"
#include "BunkerAssembly.h"

#include "ADoor.h"
#include "AHuman.h"
#include "Arm.h"
#include "HeldDevice.h"

namespace RTE {

ConcreteClassInfo(Scene, Entity, 0)
const string Scene::Area::c_ClassName = "Area";


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Area, effectively
//                  resetting the members of this abstraction level only.

void Scene::Area::Clear()
{
    m_BoxList.clear();
    m_Name.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Area to be identical to another, by deep copy.

int Scene::Area::Create(const Area &reference)
{
    for (vector<Box>::const_iterator itr = reference.m_BoxList.begin(); itr != reference.m_BoxList.end(); ++itr)
        m_BoxList.push_back(*itr);

    m_Name = reference.m_Name;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Area object ready for use.

int Scene::Area::Create()
{
    if (Serializable::Create() < 0)
        return -1;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Scene::Area::ReadProperty(const std::string_view &propName, Reader &reader)
{
    if (propName == "AddBox")
    {
        Box box;
        reader >> box;
        m_BoxList.push_back(box);
    }
    else if (propName == "Name")
        reader >> m_Name;
    else
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Area with a Writer for
//                  later recreation with Create(Reader &reader);

int Scene::Area::Save(Writer &writer) const
{
    Serializable::Save(writer);

    for (vector<Box>::const_iterator itr = m_BoxList.begin(); itr != m_BoxList.end(); ++itr)
    {
        writer.NewProperty("AddBox");
        writer << *itr;
    }
    writer.NewProperty("Name");
    writer << m_Name;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  AddBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a Box to this' area coverage.

bool Scene::Area::AddBox(const Box &newBox)
{
    if (newBox.IsEmpty())
        return false;

    m_BoxList.push_back(newBox);
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  HasNoArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this really has no Area at all, ie it doesn't have any
//                  Box:es with both width and height.

bool Scene::Area::HasNoArea() const
{
    // If no boxes, then yeah we don't have any area
    if (m_BoxList.empty())
        return true;

    // Search through the boxes to see if we find any with both width and height
    for (vector<Box>::const_iterator itr = m_BoxList.begin(); itr != m_BoxList.end(); ++itr)
    {
        if (!itr->IsEmpty())
            return false;
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsInside
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a point is anywhere inside this Area's coverage.

bool Scene::Area::IsInside(const Vector &point) const
{
    list<Box> wrappedBoxes;
    for (vector<Box>::const_iterator aItr = m_BoxList.begin(); aItr != m_BoxList.end(); ++aItr)
    {
        // Handle wrapped boxes properly
        wrappedBoxes.clear();
        g_SceneMan.WrapBox(*aItr, wrappedBoxes);

        // Iterate through the wrapped boxes - will only be one if there's no wrapping
        for (list<Box>::iterator wItr = wrappedBoxes.begin(); wItr != wrappedBoxes.end(); ++wItr)
        {
            if (wItr->IsWithinBox(point))
                return true;
        }
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsInsideX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a coordinate is anywhere inside this Area's coverage, in the
//                  X-axis only.

bool Scene::Area::IsInsideX(float pointX) const
{
    list<Box> wrappedBoxes;
    for (vector<Box>::const_iterator aItr = m_BoxList.begin(); aItr != m_BoxList.end(); ++aItr)
    {
        // Handle wrapped boxes properly
        wrappedBoxes.clear();
        g_SceneMan.WrapBox(*aItr, wrappedBoxes);

        // Iterate through the wrapped boxes - will only be one if there's no wrapping
        for (list<Box>::iterator wItr = wrappedBoxes.begin(); wItr != wrappedBoxes.end(); ++wItr)
        {
            if (wItr->IsWithinBoxX(pointX))
                return true;
        }
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsInsideY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a coordinate is anywhere inside this Area's coverage, in the
//                  Y-axis only.

bool Scene::Area::IsInsideY(float pointY) const
{
    list<Box> wrappedBoxes;
    for (vector<Box>::const_iterator aItr = m_BoxList.begin(); aItr != m_BoxList.end(); ++aItr)
    {
        // Handle wrapped boxes properly
        wrappedBoxes.clear();
        g_SceneMan.WrapBox(*aItr, wrappedBoxes);

        // Iterate through the wrapped boxes - will only be one if there's no wrapping
        for (list<Box>::iterator wItr = wrappedBoxes.begin(); wItr != wrappedBoxes.end(); ++wItr)
        {
            if (wItr->IsWithinBoxY(pointY))
                return true;
        }
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  MovePointInsideX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Moves a coordinate to the closest value which is within any of this
//                  Area's Box:es, in the X axis only.

bool Scene::Area::MovePointInsideX(float &pointX, int direction) const
{
    if (HasNoArea() || IsInsideX(pointX))
        return false;

    float notFoundValue = 10000000;
    float shortest = notFoundValue;
    float shortestConstrained = notFoundValue;
    float testDistance = 0;
    list<Box> wrappedBoxes;
    for (vector<Box>::const_iterator aItr = m_BoxList.begin(); aItr != m_BoxList.end(); ++aItr)
    {
        // Handle wrapped boxes properly
        wrappedBoxes.clear();
        g_SceneMan.WrapBox(*aItr, wrappedBoxes);

        // Iterate through the wrapped boxes - will only be one if there's no wrapping
        for (list<Box>::const_iterator wItr = wrappedBoxes.begin(); wItr != wrappedBoxes.end(); ++wItr)
        {
            // Check against one edge of the box for the shortest distance
            testDistance = g_SceneMan.ShortestDistanceX(pointX, (*wItr).GetCorner().m_X, false, direction);
            // See if it's shorter than the shortest without constraints
            if (fabs(testDistance) < fabs(shortest))
                shortest = testDistance;
            // Also see if it's the shortest constrained distance
            if (fabs(testDistance) < fabs(shortestConstrained) && (direction == 0 || (direction > 0 && testDistance > 0) || (direction < 0 && testDistance < 0)))
                shortestConstrained = testDistance;

            // Then check against the other edge of the box
            testDistance = g_SceneMan.ShortestDistanceX(pointX, (*wItr).GetCorner().m_X + (*wItr).GetWidth(), false, direction);
            // See if it's shorter than the shortest without constraints
            if (fabs(testDistance) < fabs(shortest))
                shortest = testDistance;
            // Also see if it's the shortest constrained distance
            if (fabs(testDistance) < fabs(shortestConstrained) && (direction == 0 || (direction > 0 && testDistance > 0) || (direction < 0 && testDistance < 0)))
                shortestConstrained = testDistance;
        }
    }

    // If we couldn't find any by adhering to the direction constraint, then use the shortest unconstrained found
    if (shortestConstrained == notFoundValue)
        pointX += shortest;
    // Move the point the shortest distance we found, recognizing the direction constraints
    else
        pointX += shortestConstrained;

    // Wrap it so it's inside the scene still
    int x = pointX, crap = 0;
    g_SceneMan.ForceBounds(x, crap);
    pointX = x;

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetBoxInside
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the first Box encountered in this that contains a specific point.

Box * Scene::Area::GetBoxInside(const Vector &point)
{
    list<Box> wrappedBoxes;
    for (vector<Box>::iterator aItr = m_BoxList.begin(); aItr != m_BoxList.end(); ++aItr)
    {
        // Handle wrapped boxes properly
        wrappedBoxes.clear();
        g_SceneMan.WrapBox(*aItr, wrappedBoxes);

        // Iterate through the wrapped boxes - will only be one if there's no wrapping
        for (list<Box>::const_iterator wItr = wrappedBoxes.begin(); wItr != wrappedBoxes.end(); ++wItr)
        {
            // Return the BoxList box, not the inconsequential wrapped copy
            if (wItr->IsWithinBox(point))
                return &(*aItr);
        }
    }
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RemoveBoxInside
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes the first Box encountered in this that contains a specific point.

Box Scene::Area::RemoveBoxInside(const Vector &point)
{
    Box returnBox;

    list<Box> wrappedBoxes;
    for (vector<Box>::iterator aItr = m_BoxList.begin(); aItr != m_BoxList.end(); ++aItr)
    {
        // Handle wrapped boxes properly
        wrappedBoxes.clear();
        g_SceneMan.WrapBox(*aItr, wrappedBoxes);

        // Iterate through the wrapped boxes - will only be one if there's no wrapping
        for (list<Box>::iterator wItr = wrappedBoxes.begin(); wItr != wrappedBoxes.end(); ++wItr)
        {
            if (wItr->IsWithinBox(point))
            {
                // Remove the BoxList box, not the inconsequential wrapped copy
                returnBox = (*aItr);
                m_BoxList.erase(aItr);
                return returnBox;
            }
        }
    }
    return returnBox;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetCenterPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a center point for this of all the boxes waeighted by their sizes.
// Arguments:       None.

Vector Scene::Area::GetCenterPoint() const
{
    Vector areaCenter;

    if (!m_BoxList.empty())
    {
        float totalWeight = 0;
        for (vector<Box>::const_iterator itr = m_BoxList.begin(); itr != m_BoxList.end(); ++itr)
        {
            // Doubly weighted
            areaCenter += (*itr).GetCenter() * (*itr).GetArea() * 2;
            totalWeight += (*itr).GetArea() * 2;
        }
        // Average center of the all the boxes, weighted by their respective areas
        areaCenter /= totalWeight;
    }

    return areaCenter;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetRandomPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a random coordinate contained within any of this' Box:es.

Vector Scene::Area::GetRandomPoint() const
{
    // If no boxes, then can't return valid point
    if (m_BoxList.empty())
        return Vector();

    // Randomly choose a box, and a point within it
	return m_BoxList[RandomNum<int>(0, m_BoxList.size() - 1)].GetRandomPoint();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Scene, effectively
//                  resetting the members of this abstraction level only.

void Scene::Clear()
{
    m_Location.Reset();
    m_LocationOffset.Reset();
    m_MetagamePlayable = false; // Let scenes be non-metagame playable by default, they need AI building plans anyway
    m_Revealed = false;
    m_OwnedByTeam = Activity::NoTeam;
    m_RoundIncome = 1000;
    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
    {
        m_ResidentBrains[player] = 0;
        m_BuildBudget[player] = 0;
        m_BuildBudgetRatio[player] = 0;
    }
    m_AutoDesigned = true;
    m_TotalInvestment = 0;
    m_pTerrain = 0;
    m_pPathFinder = 0;
    m_PathfindingUpdated = false;
    m_FullPathUpdateTimer.Reset();
    m_PartialPathUpdateTimer.Reset();
    for (int set = PLACEONLOAD; set < PLACEDSETSCOUNT; ++set)
        m_PlacedObjects[set].clear();
    m_BackLayerList.clear();
	m_Deployments.clear();
    for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
    {
        m_UnseenPixelSize[team].Reset();
        m_apUnseenLayer[team] = 0;
        m_SeenPixels[team].clear();
        m_CleanedPixels[team].clear();
        m_ScanScheduled[team] = false;
    }
	m_AreaList.clear();
    m_Locked = false;
    m_GlobalAcc.Reset();
    m_ScenePath.clear();
	m_SelectedAssemblies.clear();
    m_AssembliesCounts.clear();
	m_pPreviewBitmap = 0;
	m_MetasceneParent.clear();
	m_IsMetagameInternal = false;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Scene object ready for use.

int Scene::Create()
{
    if (Entity::Create() < 0)
        return -1;

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Scene object ready for use.
// Arguments:       The Terrain to use. Ownership IS transferred!
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

int Scene::Create(SLTerrain *pNewTerrain)
{
    m_pTerrain = pNewTerrain;
// TODO: allow setting of other stuff too
    m_GlobalAcc = Vector(0, 20);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a MOPixel to be identical to another, by deep copy.

int Scene::Create(const Scene &reference)
{
    Entity::Create(reference);

    m_Location = reference.m_Location;
    m_LocationOffset = reference.m_LocationOffset;
    m_MetagamePlayable = reference.m_MetagamePlayable;
    m_Revealed = reference.m_Revealed;
    m_OwnedByTeam = reference.m_OwnedByTeam;
    m_RoundIncome = reference.m_RoundIncome;
    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
    {
        if (reference.m_ResidentBrains[player])
            m_ResidentBrains[player] = dynamic_cast<SceneObject *>(reference.m_ResidentBrains[player]->Clone());
        m_BuildBudget[player] = reference.m_BuildBudget[player];
        m_BuildBudgetRatio[player] = reference.m_BuildBudgetRatio[player];
    }
    m_AutoDesigned = reference.m_AutoDesigned;
    m_TotalInvestment = reference.m_TotalInvestment;
    m_pTerrain = dynamic_cast<SLTerrain *>(reference.m_pTerrain->Clone());

    for (int set = PLACEONLOAD; set < PLACEDSETSCOUNT; ++set)
    {
        for (list<SceneObject *>::const_iterator oItr = reference.m_PlacedObjects[set].begin(); oItr != reference.m_PlacedObjects[set].end(); ++oItr)
            m_PlacedObjects[set].push_back(dynamic_cast<SceneObject *>((*oItr)->Clone()));
    }

    for (list<SceneLayer *>::const_iterator lItr = reference.m_BackLayerList.begin(); lItr != reference.m_BackLayerList.end(); ++lItr)
        m_BackLayerList.push_back(dynamic_cast<SceneLayer *>((*lItr)->Clone()));

    for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
    {
        // If the Unseen layers are loaded, then copy them. If not, then copy the procedural param that is responsible for creating them
        if (reference.m_apUnseenLayer[team])
            m_apUnseenLayer[team] = dynamic_cast<SceneLayer *>(reference.m_apUnseenLayer[team]->Clone());
        else
            m_UnseenPixelSize[team] = reference.m_UnseenPixelSize[team];

        // Always copy the scan scheduling flags
        m_ScanScheduled[team] = reference.m_ScanScheduled[team];
    }

    // Copy areas
    for (list<Area>::const_iterator aItr = reference.m_AreaList.begin(); aItr != reference.m_AreaList.end(); ++aItr)
        m_AreaList.push_back(*aItr);

    m_GlobalAcc = reference.m_GlobalAcc;
	
	// Deep copy of the bitmap
    if (reference.m_pPreviewBitmap)
    {
        // Copy the bitmap from the ContentFile, because we're going to be changing it!
        BITMAP *pCopyFrom = reference.m_pPreviewBitmap;
		// Destination
		m_pPreviewBitmap = create_bitmap_ex(8, pCopyFrom->w, pCopyFrom->h);
		RTEAssert(m_pPreviewBitmap, "Failed to allocate BITMAP in Scene::Create");

		// Copy!
		blit(pCopyFrom, m_pPreviewBitmap, 0, 0, 0, 0, pCopyFrom->w, pCopyFrom->h);
	}

	m_MetasceneParent = reference.m_MetasceneParent;
	m_IsMetagameInternal = reference.m_IsMetagameInternal;
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LoadData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Actually loads previously specified/created data into memory. Has
//                  to be done before using this SceneLayer.

int Scene::LoadData(bool placeObjects, bool initPathfinding, bool placeUnits)
{
    RTEAssert(m_pTerrain, "Terrain not instantiated before trying to load its data!");

    ///////////////////////////////////
    // Load Terrain's data
    if (m_pTerrain->LoadData() < 0)
    {
        RTEAbort("Loading Terrain " + m_pTerrain->GetPresetName() + "\'s data failed!");
        return -1;
    }
    
    ///////////////////////////////////
    // Load Unseen layers before applying objects to the scene,
    // so we can reveal around stuff that is getting placed for the appropriate team
    for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
    {
        // Specified to dynamically create the unseen layer?
        if (!m_UnseenPixelSize[team].IsZero())
        {
            // Create the bitmap to make the unseen scene layer out of
            BITMAP *pUnseenBitmap = create_bitmap_ex(8, GetWidth() / m_UnseenPixelSize[team].m_X, GetHeight() / m_UnseenPixelSize[team].m_Y);
            clear_to_color(pUnseenBitmap, g_BlackColor);
            // Replace any old unseen layer with the new one that is generated
            delete m_apUnseenLayer[team];
            m_apUnseenLayer[team] = new SceneLayer();
            m_apUnseenLayer[team]->Create(pUnseenBitmap, true, Vector(), WrapsX(), WrapsY(), Vector(1.0, 1.0));
            m_apUnseenLayer[team]->SetScaleFactor(m_UnseenPixelSize[team]);
        }
        // If not dynamically generated, was it custom loaded?
        else if (m_apUnseenLayer[team])
        {
            // Load unseen layer data from file
            if (m_apUnseenLayer[team]->LoadData() < 0)
            {
                g_ConsoleMan.PrintString("ERROR: Loading unseen layer " + m_apUnseenLayer[team]->GetPresetName() + "\'s data failed!");
                return -1;
            }
        }
    }

	m_SelectedAssemblies.clear();
    m_AssembliesCounts.clear();

    //////////////////////////////////
    // Place all the specified Scene Objects that are set up to be placed on load
    // But don't if we are only loading the scene bitmap layer data
    if (placeObjects)
    {
		Actor * pBrains[Activity::MaxTeamCount];
		for (int i = Activity::TeamOne; i < Activity::MaxTeamCount; i++)
			pBrains[i] = 0;

		// Indicates whether we need to process static brain deployments or mobile
		// whichever comes first is selected and used everywhere
		string activeBrainDeployment[Activity::MaxTeamCount];

		// Lists of found brain deployment locations used to place brain
		std::vector<Vector> brainLocations[Activity::MaxTeamCount];

        
		//for (list<SceneObject *>::iterator oItr = m_PlacedObjects[AIPLAN].begin(); oItr != m_PlacedObjects[AIPLAN].end(); ++oItr) // I'm using this to dump AI plans with ctrl+w
        for (list<SceneObject *>::iterator oItr = m_PlacedObjects[PLACEONLOAD].begin(); oItr != m_PlacedObjects[PLACEONLOAD].end(); ++oItr)
		{
            // MovableObject:s get added to the MovableMan
            MovableObject *pMO = dynamic_cast<MovableObject *>(*oItr);
            if (pMO)
            {
                // PASSING OWNERSHIP INTO the Add* ones - we are clearing out this list!
                if (pMO->IsActor())
				{
					// Skip playable actors if we're told to not place actors
					if (!placeUnits)
					{
						if (dynamic_cast<ADoor *>(pMO))
							g_MovableMan.AddActor(dynamic_cast<Actor *>(pMO));
						else
						{
							//Just delete the object
							delete pMO;
							pMO = 0;
						}
					} else {
						// Place units as usual if we're told to place units
						g_MovableMan.AddActor(dynamic_cast<Actor *>(pMO));
					}
				}
                else if (pMO->IsDevice())
                    g_MovableMan.AddItem(dynamic_cast<MovableObject *>(pMO));
                else
                    g_MovableMan.AddParticle(dynamic_cast<MovableObject *>(pMO));
            }
            else
            {
                // Deployment:s are translated into the appropriate loadout and put in place in the scene
                Deployment *pDep = dynamic_cast<Deployment *>(*oItr);
                if (pDep)
                {
					Deployment * pDepCopy = dynamic_cast<Deployment *>(pDep->Clone());
					if (pDepCopy)
					{
						pDepCopy->CloneID(pDep);
						m_Deployments.push_back(pDepCopy);
					}

					if (placeUnits)
					{
						int team = pDep->GetTeam();
						if (team < Activity::TeamOne || team >= Activity::MaxTeamCount)
							team = 0;

						// Select first brain deployemnt type as the only deployment type for this team

						bool toIgnore = false;
						// Ignore brain deployment if it's not matching the previously selected deployment type, or save it's position if it does
						if (pDep->IsInGroup("Brains") && pDep->GetPresetName() != "Brain Hideout")
						{
							if (activeBrainDeployment[team] == "")
								activeBrainDeployment[team] = pDep->GetPresetName();

							if (pDep->GetPresetName() != activeBrainDeployment[team])
								toIgnore = true;
							else
								brainLocations[team].push_back(pDep->GetPos());
						}

						// Ignore brain hideouts, they are used only by metagame when applying build budget
						if (pDep->GetPresetName() == "Brain Hideout")
							toIgnore = true;
						
						if (!toIgnore)
						{
							// Ownership IS transferred here; pass it along into the MovableMan
							float cost = 0;
							Actor *pActor = pDep->CreateDeployedActor(pDep->GetPlacedByPlayer(), cost);
							if (pActor)
							{
								// Treat brain eployements the special way
								if (pDep->GetPresetName() == activeBrainDeployment[team] && pActor->IsInGroup("Brains"))
								{
									// If it's the first created brain then place it, otherwise just delete
									if (pBrains[team] == 0)
									{
										g_MovableMan.AddActor(pActor);
										pBrains[team] = pActor;
									}
									else
									{
										delete pActor;
										pActor = 0;
									}
								}
								else
									g_MovableMan.AddActor(pActor);
							}
							// Just a simple Device in the Deployment?
							else
							{
								// Get the Item/Device and add to scene, passing ownership
								SceneObject *pObject = pDep->CreateDeployedObject(pDep->GetPlacedByPlayer(), cost);
								pMO = dynamic_cast<MovableObject *>(pObject);
								if (pMO)
									g_MovableMan.AddItem(pMO);
								else
								{
									delete pObject;
									pObject = 0;
								}
							}
						}
					}
                }
                // Has to be a TerrainObject then, so apply to terrain
                else
                {
					//Place random BunkerAssembly instead of BunkerAssemblyScheme if this is a BunkerAssemblyScheme
					TerrainObject *pTO = 0;
					BunkerAssemblyScheme *pBAS = dynamic_cast<BunkerAssemblyScheme *>(*oItr);

					if (pBAS)
					{
						const BunkerAssembly * pBAPreset = 0;

						// Try to select previousy selected assembly
						if (pBAS->IsOneTypePerScene())
						{
							map<string, const BunkerAssembly *>::iterator itr = m_SelectedAssemblies.find(pBAS->GetModuleAndPresetName());
							if (itr != m_SelectedAssemblies.end())
								pBAPreset = (*itr).second;
						}

						//Find random bunker assembly and clone it
						if (!pBAPreset)
						{
							pBAPreset = dynamic_cast<const BunkerAssembly *>(g_PresetMan.GetRandomOfGroup(pBAS->GetPresetName(), "BunkerAssembly", -1));

							if (pBAPreset)
							{
								// Remember selected BunkerAssembly for this scheme to use it everywhere on the map.
								if (pBAS->IsOneTypePerScene())
								{
									m_SelectedAssemblies.insert(pair<string, const BunkerAssembly *>(pBAS->GetModuleAndPresetName(), pBAPreset));

									// Find out if this scheme has symmetric scheme, and this assembly have symmetric version
									// so we could set them to be used when we need to create symmetric versions of this scheme
									string symmetricScheme = pBAS->GetSymmetricSchemeName();
									string symmetricAssembly = pBAPreset->GetSymmetricAssemblyName();

									if (symmetricScheme.size() > 0 && symmetricAssembly.size() > 0)
									{
										const BunkerAssembly * pBAPresetSymmetric = dynamic_cast<const BunkerAssembly *>(g_PresetMan.GetEntityPreset("BunkerAssembly", symmetricAssembly));

										if (pBAPresetSymmetric)
											m_SelectedAssemblies.insert(pair<string, const BunkerAssembly *>(symmetricScheme, pBAPresetSymmetric));
									}
								}
							}
						}
						// Finally make a copy of found or selected assembly to put it on scene later
						if (pBAPreset)
						{
							pTO = dynamic_cast<BunkerAssembly *>(pBAPreset->Clone());
							pTO->SetPos(pBAS->GetPos());
							pTO->SetTeam(pBAS->GetTeam());
						}
					}

					// Try to read as terrain object if it's not Assembly Scheme
					// NOT passing ownership here, so have to delete it ourselves
					if (!pTO)
						pTO = dynamic_cast<TerrainObject *>(*oItr);

					// Add deployments placed by bunker assemblies, but not in metagame, 
					// as they are spawned and placed during ApplyBuildBudget
					if (placeUnits && !g_MetaMan.GameInProgress())
					{
						BunkerAssembly * pBA = dynamic_cast<BunkerAssembly *>(pTO);
						if (pBA)
						{
							std::vector<Deployment *>deployments = pBA->GetDeployments();
							for (std::vector<Deployment *>::iterator itr = deployments.begin(); itr != deployments.end() ; ++itr)
							{
								// Create a copy of deployment because assemblies own their placed objects and scenes own theirs
								SceneObject * so = (SceneObject *)(*itr)->Clone();
								so->SetPos(so->GetPos() + pBA->GetPos() + pBA->GetBitmapOffset());
								so->SetTeam(pBA->GetTeam());
								so->SetPlacedByPlayer(pBA->GetPlacedByPlayer());
								m_PlacedObjects[PLACEONLOAD].push_back(so);
							}
						}
					}

					// Finally place TerrainObject
					if (pTO)
                    {
                        // First clear out the box of unseen layer for whichever team placed this
                        if (pTO->GetFGColorBitmap() && pTO->GetPlacedByPlayer() != Players::NoPlayer && g_ActivityMan.GetActivity())
                        {
                            // Learn which team placed this thing so we can reveal for them only
                            int ownerTeam = pTO->GetTeam();
                            if (ownerTeam != Activity::NoTeam && m_apUnseenLayer[ownerTeam] && m_apUnseenLayer[ownerTeam]->GetBitmap())
                            {
                                // Translate to the scaled unseen layer's coordinates
                                Vector scale = m_apUnseenLayer[ownerTeam]->GetScaleInverse();
                                int scaledX = std::floor((pTO->GetPos().m_X - (float)(pTO->GetFGColorBitmap()->w / 2)) * scale.m_X);
                                int scaledY = std::floor((pTO->GetPos().m_Y - (float)(pTO->GetFGColorBitmap()->h / 2)) * scale.m_Y);
                                int scaledW = std::ceil(pTO->GetFGColorBitmap()->w * scale.m_X);
                                int scaledH = std::ceil(pTO->GetFGColorBitmap()->h * scale.m_Y);
                                // Fill the box with key color for the owner ownerTeam, revealing the area that this thing is on
                                rectfill(m_apUnseenLayer[ownerTeam]->GetBitmap(), scaledX, scaledY, scaledX + scaledW, scaledY + scaledH, g_MaskColor);
                                // Expand the box a little so the whole placed object is going to be hidden
                                scaledX -= 1;
                                scaledY -= 1;
                                scaledW += 2;
                                scaledH += 2;
                                // Fill the box with BLACK for all the other teams so they can't see the new developments here!
                                for (int t = Activity::TeamOne; t < Activity::MaxTeamCount; ++t)
                                {
                                    if (t != ownerTeam && m_apUnseenLayer[t] && m_apUnseenLayer[t]->GetBitmap())
                                        rectfill(m_apUnseenLayer[t]->GetBitmap(), scaledX, scaledY, scaledX + scaledW, scaledY + scaledH, g_BlackColor);
                                }
                            }
                        }
                        // Now actually stamp the terrain object onto the terrain's scene layers
                        m_pTerrain->ApplyTerrainObject(pTO);
                        //delete pTO;
                        //pTO = 0;
                    }

					// Finally delete the object retreived from the list
					delete (*oItr);
					(*oItr) = 0;
                }
            }
        }
		// Finally select random brain location from the list of found brain deployments
		for (int t = Activity::TeamOne; t < Activity::MaxTeamCount; t++)
		{
			if (pBrains[t])
			{
				int team = pBrains[t]->GetTeam();
				if (team < Activity::TeamOne || team >= Activity::MaxTeamCount)
					team = 0;
				if (brainLocations[team].size() > 0)
				{
					int selection = RandomNum<int>(0, brainLocations[team].size() - 1);
					pBrains[t]->SetPos(brainLocations[team].at(selection));
				}
			}
		}

        // Clear out the list of things that were placed above.. the ownership of everything WAS transferred anyway
        m_PlacedObjects[PLACEONLOAD].clear();
        //m_PlacedObjects[AIPLAN].clear(); // I'm using this to dump AI plans via CTRL+W

// TODO: CLEAN AIR IN AN AFTER EACH OBJECT PLACED, becuase the items refuse to be placed in a hollowness otherwise?
        // Clear the air out of objects placed
        m_pTerrain->CleanAir();
    }

    /////////////////////////////////
    // Go through and init the teams on all remaining placed objects, so their flag icons etc are correct for the current Activity
    for (int set = PLACEONLOAD; set < PLACEDSETSCOUNT; ++set)
    {
        for (list<SceneObject *>::iterator oItr = m_PlacedObjects[set].begin(); oItr != m_PlacedObjects[set].end(); ++oItr)
            (*oItr)->SetTeam((*oItr)->GetTeam());
    }

    ///////////////////////////////
    // Pathfinding init
    if (initPathfinding)
    {
        // Create the pathfinding stuff based on the current scene
        m_pPathFinder = new PathFinder(this, 20, 2000);
        // Update all the pathfinding data
        m_pPathFinder->RecalculateAllCosts();

        // Load Background layers' data
        for (list<SceneLayer *>::iterator slItr = m_BackLayerList.begin(); slItr != m_BackLayerList.end(); ++slItr)
        {
            RTEAssert((*slItr), "Background layer not instantiated before trying to load its data!");
            if ((*slItr)->LoadData() < 0)
            {
                g_ConsoleMan.PrintString("ERROR: Loading background layer " + (*slItr)->GetPresetName() + "\'s data failed!");
                return -1;
            }
        }
    }

    return 0;
}




//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ExpandAIPlanAssemblySchemes
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
//                  

int Scene::ExpandAIPlanAssemblySchemes()
{
	list<SceneObject *> newAIPlan;

    for (list<SceneObject *>::iterator oItr = m_PlacedObjects[AIPLAN].begin(); oItr != m_PlacedObjects[AIPLAN].end(); ++oItr)
	{
		BunkerAssemblyScheme *pBAS = dynamic_cast<BunkerAssemblyScheme *>(*oItr);

		if (pBAS)
		{
			const BunkerAssembly * pBAPreset = 0;
			//BunkerAssembly * pPA = 0;

			// Try to select previousy selected assembly
			if (pBAS->IsOneTypePerScene())
			{
				map<string, const BunkerAssembly *>::iterator itr = m_SelectedAssemblies.find(pBAS->GetModuleAndPresetName());
				if (itr != m_SelectedAssemblies.end())
					pBAPreset = (*itr).second;
			}

			//Find random bunker assembly and clone it
			if (!pBAPreset)
			{
				pBAPreset = dynamic_cast<const BunkerAssembly *>(g_PresetMan.GetRandomOfGroup(pBAS->GetPresetName(), "BunkerAssembly", -1));

				// Remember selected BunkerAssembly for this scheme to use it everywhere on the map.
				if (pBAS->IsOneTypePerScene())
					m_SelectedAssemblies.insert(pair<string, const BunkerAssembly *>(pBAS->GetModuleAndPresetName(), pBAPreset));
			}

			// Add found assembly to the AI plan
			if (pBAPreset)
			{
				BunkerAssembly * pBA = dynamic_cast<BunkerAssembly *>(pBAPreset->Clone());
				pBA->SetPos(pBAS->GetPos());
				pBA->SetTeam(pBAS->GetTeam());
				pBA->SetPlacedByPlayer(pBAS->GetPlacedByPlayer());

				newAIPlan.push_back(pBA);
				
				std::vector<Deployment *>pDeployments = pBA->GetDeployments();
				for (std::vector<Deployment *>::iterator itr = pDeployments.begin(); itr != pDeployments.end() ; ++itr)
				{
					// Create a copy of deployment because assemblies own their placed objects and scenes own theirs
					SceneObject * so = (SceneObject *)(*itr)->Clone();
					so->SetPos(so->GetPos() + pBA->GetPos() + pBA->GetBitmapOffset());
					so->SetTeam(pBA->GetTeam());
					so->SetPlacedByPlayer(pBA->GetPlacedByPlayer());
					newAIPlan.push_back(so);
				}
			}
			//Delete scheme as it was replaced by an assembly by now
			delete (*oItr);
			(*oItr) = 0;
		} else {
			// If it's not a scheme, then just put it in the new list
			newAIPlan.push_back(*oItr);
		}
    }

	// Copy new AI plan list to replace the original
	m_PlacedObjects[AIPLAN].clear();
    for (list<SceneObject *>::iterator oItr = newAIPlan.begin(); oItr != newAIPlan.end(); ++oItr)
		m_PlacedObjects[AIPLAN].push_back(*oItr);

    return 0;
}





//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SaveData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves currently loaded bitmap data in memory to disk.

int Scene::SaveData(string pathBase)
{
    if (pathBase.empty())
        return -1;

    if (!m_pTerrain)
        return 0;

    // Save Terrain's data
    if (m_pTerrain->SaveData(pathBase) < 0)
    {
        RTEAbort("Saving Terrain " + m_pTerrain->GetPresetName() + "\'s data failed!");
        return -1;
    }

    // Don't bother saving background layers to disk, as they are never altered

    // Save unseen layers' data
    char str[64];
    for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
    {
        if (m_apUnseenLayer[team])
        {
            std::snprintf(str, sizeof(str), "T%d", team);
            // Save unseen layer data to disk
            if (m_apUnseenLayer[team]->SaveData(pathBase + " US" + str + ".bmp") < 0)
            {
                g_ConsoleMan.PrintString("ERROR: Saving unseen layer " + m_apUnseenLayer[team]->GetPresetName() + "\'s data failed!");
                return -1;
            }
        }
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SavePreview
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves preview bitmap for this scene.
//

int Scene::SavePreview(const std::string &bitmapPath) {
	// Do not save preview for MetaScenes!
	if (!m_MetasceneParent.empty()) {
		return -1;
	}

	if (m_pPreviewBitmap && (m_pPreviewBitmap->w != PREVIEW_WIDTH || m_pPreviewBitmap->h != PREVIEW_HEIGHT)) {
		destroy_bitmap(m_pPreviewBitmap);
		m_pPreviewBitmap = nullptr;
	}
	if (!m_pPreviewBitmap) { m_pPreviewBitmap = create_bitmap_ex(8, PREVIEW_WIDTH, PREVIEW_HEIGHT); }

	ContentFile scenePreviewGradient("Base.rte/GUIs/PreviewSkyGradient.png");
	draw_sprite(m_pPreviewBitmap, scenePreviewGradient.GetAsBitmap(COLORCONV_NONE, true), 0, 0);

	int sceneWidth = m_pTerrain->GetBitmap()->w;
	int sceneHeight = m_pTerrain->GetBitmap()->h;
	BITMAP *previewBuffer = create_bitmap_ex(8, sceneWidth, sceneHeight);
	clear_to_color(previewBuffer, ColorKeys::g_MaskColor);

	masked_blit(m_pTerrain->GetBGColorBitmap(), previewBuffer, 0, 0, 0, 0, sceneWidth, sceneHeight);
	masked_blit(m_pTerrain->GetFGColorBitmap(), previewBuffer, 0, 0, 0, 0, sceneWidth, sceneHeight);

	for (SceneObject *sceneObject : m_PlacedObjects[PlacedObjectSets::PLACEONLOAD]) {
		if (const TerrainObject *terrainObject = dynamic_cast<TerrainObject *>(sceneObject)) {
			Vector pos = terrainObject->GetPos() + terrainObject->GetBitmapOffset();
			BITMAP *terrainObjectBG = terrainObject->GetBGColorBitmap();
			BITMAP *terrainObjectFG = terrainObject->GetFGColorBitmap();

			// Wrapped drawing
			if (pos.GetFloorIntX() < 0) {
				masked_blit(terrainObjectBG, previewBuffer, 0, 0, pos.GetFloorIntX() + sceneWidth, pos.GetFloorIntY(), terrainObjectBG->w, terrainObjectBG->h);
				masked_blit(terrainObjectFG, previewBuffer, 0, 0, pos.GetFloorIntX() + sceneWidth, pos.GetFloorIntY(), terrainObjectFG->w, terrainObjectFG->h);
			} else if (pos.GetFloorIntX() + terrainObject->GetBitmapWidth() >= sceneWidth) {
				masked_blit(terrainObjectBG, previewBuffer, 0, 0, pos.GetFloorIntX() - sceneWidth, pos.GetFloorIntY(), terrainObjectBG->w, terrainObjectBG->h);
				masked_blit(terrainObjectFG, previewBuffer, 0, 0, pos.GetFloorIntX() - sceneWidth, pos.GetFloorIntY(), terrainObjectFG->w, terrainObjectFG->h);
			}
			masked_blit(terrainObjectBG, previewBuffer, 0, 0, pos.GetFloorIntX(), pos.GetFloorIntY(), terrainObjectBG->w, terrainObjectBG->h);
			masked_blit(terrainObjectFG, previewBuffer, 0, 0, pos.GetFloorIntX(), pos.GetFloorIntY(), terrainObjectFG->w, terrainObjectFG->h);
		}

		// TODO: Figure out how to draw the actual modules that compose an assembly that is part of the scheme. For now just disable.
		/*
		else if (const BunkerAssemblyScheme *assemblyScheme = dynamic_cast<BunkerAssemblyScheme *>(sceneObject)) {
			Vector pos = assemblyScheme->GetPos() + assemblyScheme->GetBitmapOffset();

			// Wrapped drawing
			if (pos.GetFloorIntX() < 0) {
				rect(previewBuffer, pos.GetFloorIntX() + sceneWidth, pos.GetFloorIntY(), pos.GetFloorIntX() + sceneWidth + assemblyScheme->GetBitmapWidth(), pos.GetFloorIntY() + assemblyScheme->GetBitmapHeight(), 5);
			} else if (pos.GetFloorIntX() + assemblyScheme->GetBitmapWidth() >= sceneWidth) {
				rect(previewBuffer, pos.GetFloorIntX() - sceneWidth, pos.GetFloorIntY(), pos.GetFloorIntX() - sceneWidth + assemblyScheme->GetBitmapWidth(), pos.GetFloorIntY() + assemblyScheme->GetBitmapHeight(), 5);
			}
			rect(previewBuffer, pos.GetFloorIntX(), pos.GetFloorIntY(), pos.GetFloorIntX() + ((float)assemblyScheme->GetBitmapWidth()), pos.GetFloorIntY() + ((float)assemblyScheme->GetBitmapHeight()), 5);
		}
		*/
	}

	masked_stretch_blit(previewBuffer, m_pPreviewBitmap, 0, 0, previewBuffer->w, previewBuffer->h, 0, 0, m_pPreviewBitmap->w, m_pPreviewBitmap->h);
	destroy_bitmap(previewBuffer);

	if (g_FrameMan.SaveBitmapToPNG(m_pPreviewBitmap, bitmapPath.c_str()) == 0) { m_PreviewBitmapFile.SetDataPath(bitmapPath); }

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ClearData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears out any previously loaded bitmap data from memory.

int Scene::ClearData()
{
    if (!m_pTerrain)
        return 0;

    // Clear Terrain's data
    if (m_pTerrain->ClearData() < 0)
    {
        RTEAbort("Clearing Terrain " + m_pTerrain->GetPresetName() + "\'s data failed!");
        return -1;
    }

    // Clear Background layers' data
    for (list<SceneLayer *>::iterator slItr = m_BackLayerList.begin(); slItr != m_BackLayerList.end(); ++slItr)
    {
        RTEAssert((*slItr), "Background layer not instantiated before trying to clear its data!");
        if ((*slItr)->ClearData() < 0)
        {
            g_ConsoleMan.PrintString("ERROR: Clearing background layer " + (*slItr)->GetPresetName() + "\'s data failed!");
            return -1;
        }
    }

    // Clear unseen layers' data
    for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
    {
        if (m_apUnseenLayer[team])
        {
            // Clear unseen layer data from memory
            if (m_apUnseenLayer[team]->ClearData() < 0)
            {
                g_ConsoleMan.PrintString("ERROR: Clearing unseen layer " + m_apUnseenLayer[team]->GetPresetName() + "\'s data failed!");
                return -1;
            }
        }
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Scene::ReadProperty(const std::string_view &propName, Reader &reader)
{

    if (propName == "LocationOnPlanet")
        reader >> m_Location;
    else if (propName == "MetagamePlayable")
        reader >> m_MetagamePlayable;
    else if (propName == "Revealed")
        reader >> m_Revealed;
    else if (propName == "MetasceneParent")
        reader >> m_MetasceneParent;
    else if (propName == "MetagameInternal")
        reader >> m_IsMetagameInternal;
    else if (propName == "OwnedByTeam")
        reader >> m_OwnedByTeam;
    else if (propName == "RoundIncome")
        reader >> m_RoundIncome;
    else if (propName == "P1ResidentBrain")
        m_ResidentBrains[Players::PlayerOne] = dynamic_cast<SceneObject *>(g_PresetMan.ReadReflectedPreset(reader));
    else if (propName == "P2ResidentBrain")
        m_ResidentBrains[Players::PlayerTwo] = dynamic_cast<SceneObject *>(g_PresetMan.ReadReflectedPreset(reader));
    else if (propName == "P3ResidentBrain")
        m_ResidentBrains[Players::PlayerThree] = dynamic_cast<SceneObject *>(g_PresetMan.ReadReflectedPreset(reader));
    else if (propName == "P4ResidentBrain")
        m_ResidentBrains[Players::PlayerFour] = dynamic_cast<SceneObject *>(g_PresetMan.ReadReflectedPreset(reader));
    else if (propName == "P1BuildBudget")
        reader >> m_BuildBudget[Players::PlayerOne];
    else if (propName == "P2BuildBudget")
        reader >> m_BuildBudget[Players::PlayerTwo];
    else if (propName == "P3BuildBudget")
        reader >> m_BuildBudget[Players::PlayerThree];
    else if (propName == "P4BuildBudget")
        reader >> m_BuildBudget[Players::PlayerFour];
    else if (propName == "P1BuildBudgetRatio")
        reader >> m_BuildBudgetRatio[Players::PlayerOne];
    else if (propName == "P2BuildBudgetRatio")
        reader >> m_BuildBudgetRatio[Players::PlayerTwo];
    else if (propName == "P3BuildBudgetRatio")
        reader >> m_BuildBudgetRatio[Players::PlayerThree];
    else if (propName == "P4BuildBudgetRatio")
        reader >> m_BuildBudgetRatio[Players::PlayerFour];
    else if (propName == "AutoDesigned")
        reader >> m_AutoDesigned;
    else if (propName == "TotalInvestment")
        reader >> m_TotalInvestment;
    else if (propName == "PreviewBitmapFile")
	{
        reader >> m_PreviewBitmapFile;
		m_pPreviewBitmap = m_PreviewBitmapFile.GetAsBitmap(COLORCONV_NONE, false);
	}
    else if (propName == "Terrain")
    {
        delete m_pTerrain;
        m_pTerrain = new SLTerrain();
        reader >> m_pTerrain;
    }
    else if (propName == "PlaceSceneObject" || propName == "PlaceMovableObject")
    {
        SceneObject *pSO = dynamic_cast<SceneObject *>(g_PresetMan.ReadReflectedPreset(reader));
        if (pSO)
            m_PlacedObjects[PLACEONLOAD].push_back(pSO);
    }
    else if (propName == "BlueprintObject")
    {
        SceneObject *pSO = dynamic_cast<SceneObject *>(g_PresetMan.ReadReflectedPreset(reader));
        if (pSO)
            m_PlacedObjects[BLUEPRINT].push_back(pSO);
    }
    else if (propName == "PlaceAIPlanObject")
    {
        SceneObject *pSO = dynamic_cast<SceneObject *>(g_PresetMan.ReadReflectedPreset(reader));
        if (pSO)
            m_PlacedObjects[AIPLAN].push_back(pSO);
    }
    else if (propName == "AddBackgroundLayer")
    {
        SceneLayer *pLayer = dynamic_cast<SceneLayer *>(g_PresetMan.ReadReflectedPreset(reader));
        RTEAssert(pLayer, "Something went wrong with reading SceneLayer");
        if (pLayer)
            m_BackLayerList.push_back(pLayer);
    }
    else if (propName == "AllUnseenPixelSizeTeam1")
    {
        // Read the desired pixel dimensions of the dynamically generated unseen map
        reader >> m_UnseenPixelSize[Activity::TeamOne];
    }
    else if (propName == "AllUnseenPixelSizeTeam2")
    {
        // Read the desired pixel dimensions of the dynamically generated unseen map
        reader >> m_UnseenPixelSize[Activity::TeamTwo];
    }
    else if (propName == "AllUnseenPixelSizeTeam3")
    {
        // Read the desired pixel dimensions of the dynamically generated unseen map
        reader >> m_UnseenPixelSize[Activity::TeamThree];
    }
    else if (propName == "AllUnseenPixelSizeTeam4")
    {
        // Read the desired pixel dimensions of the dynamically generated unseen map
        reader >> m_UnseenPixelSize[Activity::TeamFour];
    }
    else if (propName == "UnseenLayerTeam1")
    {
        delete m_apUnseenLayer[Activity::TeamOne];
        m_apUnseenLayer[Activity::TeamOne] = dynamic_cast<SceneLayer *>(g_PresetMan.ReadReflectedPreset(reader));
    }
    else if (propName == "UnseenLayerTeam2")
    {
        delete m_apUnseenLayer[Activity::TeamTwo];
        m_apUnseenLayer[Activity::TeamTwo] = dynamic_cast<SceneLayer *>(g_PresetMan.ReadReflectedPreset(reader));
    }
    else if (propName == "UnseenLayerTeam3")
    {
        delete m_apUnseenLayer[Activity::TeamThree];
        m_apUnseenLayer[Activity::TeamThree] = dynamic_cast<SceneLayer *>(g_PresetMan.ReadReflectedPreset(reader));
    }
    else if (propName == "UnseenLayerTeam4")
    {
        delete m_apUnseenLayer[Activity::TeamFour];
        m_apUnseenLayer[Activity::TeamFour] = dynamic_cast<SceneLayer *>(g_PresetMan.ReadReflectedPreset(reader));
    }
    else if (propName == "ScanScheduledTeam1")
        reader >> m_ScanScheduled[Activity::TeamOne];
    else if (propName == "ScanScheduledTeam2")
        reader >> m_ScanScheduled[Activity::TeamTwo];
    else if (propName == "ScanScheduledTeam3")
        reader >> m_ScanScheduled[Activity::TeamThree];
    else if (propName == "ScanScheduledTeam4")
        reader >> m_ScanScheduled[Activity::TeamFour];
    else if (propName == "AddArea")
    {
        Area area;
        reader >> area;
        // This replaces any existing ones
        SetArea(area);
    }
    else if (propName == "GlobalAcceleration")
        reader >> m_GlobalAcc;
    else
        return Entity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Scene with a Writer for
//                  later recreation with Create(Reader &reader);

int Scene::Save(Writer &writer) const
{
    Entity::Save(writer);
    writer.NewProperty("LocationOnPlanet");
    writer << m_Location;
    writer.NewProperty("MetagamePlayable");
    writer << m_MetagamePlayable;
	//Do not save preview if it's path is empty, for example in metagame
	if (m_PreviewBitmapFile.GetDataPath().length() > 0 && m_MetasceneParent.length() == 0)
	{
		writer.NewProperty("PreviewBitmapFile");
		writer << m_PreviewBitmapFile;
	}
	if (m_MetasceneParent.length() > 0)
	{
		writer.NewProperty("MetasceneParent");
		writer << m_MetasceneParent;
	}
    writer.NewProperty("MetagameInternal");
    writer << m_IsMetagameInternal;
    writer.NewProperty("Revealed");
    writer << m_Revealed;
    writer.NewProperty("OwnedByTeam");
    writer << m_OwnedByTeam;
    writer.NewProperty("RoundIncome");
    writer << m_RoundIncome;
    // Write out the brains and the minimal info needed to place them in the scene
    char str[64];
    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
    {
        if (m_ResidentBrains[player])
        {
            std::snprintf(str, sizeof(str), "P%dResidentBrain", player + 1);
            writer.NewProperty(str);
            writer.ObjectStart(m_ResidentBrains[player]->GetClassName());
            writer.NewProperty("CopyOf");
            writer << m_ResidentBrains[player]->GetModuleAndPresetName();
            writer.NewProperty("Position");
            writer << m_ResidentBrains[player]->GetPos();
            writer.NewProperty("HFlipped");
            writer << m_ResidentBrains[player]->IsHFlipped();
            writer.NewProperty("Team");
            writer << m_ResidentBrains[player]->GetTeam();
			
			//Write out brain's inventory if it is Actor
			Actor *pActor = dynamic_cast<Actor *>(m_ResidentBrains[player]);
            if (pActor)
            {
                const deque<MovableObject *> *pInventory = pActor->GetInventory();
                for (deque<MovableObject *>::const_iterator iitr = pInventory->begin(); iitr != pInventory->end(); ++iitr)
                {
                    writer.NewProperty("AddInventory");
                    writer.ObjectStart((*iitr)->GetClassName());

                    writer.NewProperty("CopyOf");
                    writer << (*iitr)->GetModuleAndPresetName();
                    
                    writer.ObjectEnd();
                }

                // Also put whatever was held in the hand into inventory
                AHuman *pAHuman = dynamic_cast<AHuman *>(m_ResidentBrains[player]);
                if (pAHuman && dynamic_cast<Arm *>(pAHuman->GetFGArm()) && dynamic_cast<Arm *>(pAHuman->GetFGArm())->GetHeldDevice())
                {
                    writer.NewProperty("AddInventory");
                    writer.ObjectStart(dynamic_cast<Arm *>(pAHuman->GetFGArm())->GetHeldDevice()->GetClassName());

                    writer.NewProperty("CopyOf");
                    writer << dynamic_cast<Arm *>(pAHuman->GetFGArm())->GetHeldDevice()->GetModuleAndPresetName();

                    writer.ObjectEnd();                            
                }
            }

            writer.ObjectEnd();
        }
    }
    writer.NewProperty("P1BuildBudget");
    writer << m_BuildBudget[Players::PlayerOne];
    writer.NewProperty("P2BuildBudget");
    writer << m_BuildBudget[Players::PlayerTwo];
    writer.NewProperty("P3BuildBudget");
    writer << m_BuildBudget[Players::PlayerThree];
    writer.NewProperty("P4BuildBudget");
    writer << m_BuildBudget[Players::PlayerFour];
    writer.NewProperty("P1BuildBudgetRatio");
    writer << m_BuildBudgetRatio[Players::PlayerOne];
    writer.NewProperty("P2BuildBudgetRatio");
    writer << m_BuildBudgetRatio[Players::PlayerTwo];
    writer.NewProperty("P3BuildBudgetRatio");
    writer << m_BuildBudgetRatio[Players::PlayerThree];
    writer.NewProperty("P4BuildBudgetRatio");
    writer << m_BuildBudgetRatio[Players::PlayerFour];
    writer.NewProperty("AutoDesigned");
    writer << m_AutoDesigned;
    writer.NewProperty("TotalInvestment");
    writer << m_TotalInvestment;
    writer.NewProperty("Terrain");
    writer << m_pTerrain;

    for (int set = PLACEONLOAD; set < PLACEDSETSCOUNT; ++set)
    {
        for (list<SceneObject *>::const_iterator oItr = m_PlacedObjects[set].begin(); oItr != m_PlacedObjects[set].end(); ++oItr)
        {
            if (set == PLACEONLOAD)
                writer.NewProperty("PlaceSceneObject");
            else if (set == BLUEPRINT)
                writer.NewProperty("BlueprintObject");
            else if (set == AIPLAN)
                writer.NewProperty("PlaceAIPlanObject");

    //        writer << (*oItr);
            writer.ObjectStart((*oItr)->GetClassName());

            writer.NewProperty("CopyOf");
            writer << (*oItr)->GetModuleAndPresetName();

            writer.NewProperty("Position");
            writer << (*oItr)->GetPos();

            if ((*oItr)->GetPlacedByPlayer() != Players::NoPlayer)
            {
                writer.NewProperty("PlacedByPlayer");
                writer << (*oItr)->GetPlacedByPlayer();
            }

			// Save deployment's and assemblies teams if we're saving default scene set
			// because they can spawn actors and other deployments
			if (set == PLACEONLOAD)
			{
				if (dynamic_cast<Deployment *>(*oItr) || dynamic_cast<BunkerAssembly *>(*oItr) || dynamic_cast<BunkerAssemblyScheme *>(*oItr))
				{
					writer.NewProperty("Team");
					writer << (*oItr)->GetTeam();
				}
			}

            // Only write certain properties if they are applicable to the type of SceneObject being written
            MOSRotating *pSpriteObj = dynamic_cast<MOSRotating *>(*oItr);
            if (pSpriteObj)
            {
                writer.NewProperty("HFlipped");
                writer << pSpriteObj->IsHFlipped();
                Actor *pActor = dynamic_cast<Actor *>(pSpriteObj);
                if (pActor)
                {
                    writer.NewProperty("Team");
                    writer << pActor->GetTeam();
                    // Rotation of doors is important
                    ADoor *pDoor = dynamic_cast<ADoor *>(pActor);
                    if (pDoor)
                    {
                        writer.NewProperty("Rotation");
                        writer << pDoor->GetRotMatrix();
                    }
                    // Inventory is important to preserve too
                    Actor *pActor = dynamic_cast<Actor *>(*oItr);
                    if (pActor)
                    {
                        writer.NewProperty("Health");
                        writer << pActor->GetHealth();
                        writer.NewProperty("MaxHealth");
                        writer << pActor->GetMaxHealth();
						if (pActor->GetDeploymentID())
						{
							writer.NewProperty("DeploymentID");
							writer << pActor->GetDeploymentID();
						}

                        const deque<MovableObject *> *pInventory = pActor->GetInventory();
                        for (deque<MovableObject *>::const_iterator iitr = pInventory->begin(); iitr != pInventory->end(); ++iitr)
                        {
                            writer.NewProperty("AddInventory");
                            writer.ObjectStart((*iitr)->GetClassName());

                            writer.NewProperty("CopyOf");
                            writer << (*iitr)->GetModuleAndPresetName();
                            
                            writer.ObjectEnd();
                        }

                        // Also put whatever was held in the hand into inventory
                        AHuman *pAHuman = dynamic_cast<AHuman *>(*oItr);
                        if (pAHuman && dynamic_cast<Arm *>(pAHuman->GetFGArm()) && dynamic_cast<Arm *>(pAHuman->GetFGArm())->GetHeldDevice())
                        {
                            writer.NewProperty("AddInventory");
                            writer.ObjectStart(dynamic_cast<Arm *>(pAHuman->GetFGArm())->GetHeldDevice()->GetClassName());

                            writer.NewProperty("CopyOf");
                            writer << dynamic_cast<Arm *>(pAHuman->GetFGArm())->GetHeldDevice()->GetModuleAndPresetName();

                            writer.ObjectEnd();                            
                        }
                    }
                }
            }
            TerrainObject *pTObject = dynamic_cast<TerrainObject *>(*oItr);
            if (pTObject && !pTObject->GetChildObjects().empty())
            {
                writer.NewProperty("Team");
                writer << pTObject->GetTeam();
            }
            Deployment *pDeployment = dynamic_cast<Deployment *>(*oItr);
            if (pDeployment && pDeployment->GetID())
            {
                writer.NewProperty("ID");
                writer << pDeployment->GetID();
				writer.NewProperty("HFlipped");
				writer << pDeployment->IsHFlipped();
            }
            writer.ObjectEnd();
        }
    }

    for (list<SceneLayer *>::const_iterator slItr = m_BackLayerList.begin(); slItr != m_BackLayerList.end(); ++slItr)
    {
        writer.NewProperty("AddBackgroundLayer");
        (*slItr)->SavePresetCopy(writer);
    }
    if (!m_UnseenPixelSize[Activity::TeamOne].IsZero())
    {
        writer.NewProperty("AllUnseenPixelSizeTeam1");
        writer << m_UnseenPixelSize[Activity::TeamOne];
    }
    if (!m_UnseenPixelSize[Activity::TeamTwo].IsZero())
    {
        writer.NewProperty("AllUnseenPixelSizeTeam2");
        writer << m_UnseenPixelSize[Activity::TeamTwo];
    }
    if (!m_UnseenPixelSize[Activity::TeamThree].IsZero())
    {
        writer.NewProperty("AllUnseenPixelSizeTeam3");
        writer << m_UnseenPixelSize[Activity::TeamThree];
    }
    if (!m_UnseenPixelSize[Activity::TeamFour].IsZero())
    {
        writer.NewProperty("AllUnseenPixelSizeTeam4");
        writer << m_UnseenPixelSize[Activity::TeamFour];
    }
    if (m_apUnseenLayer[Activity::TeamOne])
    {
        writer.NewProperty("UnseenLayerTeam1");
        writer << m_apUnseenLayer[Activity::TeamOne];
    }
    if (m_apUnseenLayer[Activity::TeamTwo])
    {
        writer.NewProperty("UnseenLayerTeam2");
        writer << m_apUnseenLayer[Activity::TeamTwo];
    }
    if (m_apUnseenLayer[Activity::TeamThree])
    {
        writer.NewProperty("UnseenLayerTeam3");
        writer << m_apUnseenLayer[Activity::TeamThree];
    }
    if (m_apUnseenLayer[Activity::TeamFour])
    {
        writer.NewProperty("UnseenLayerTeam4");
        writer << m_apUnseenLayer[Activity::TeamFour];
    }
    if (m_ScanScheduled[Activity::TeamOne])
    {
        writer.NewProperty("ScanScheduledTeam1");
        writer << m_ScanScheduled[Activity::TeamOne];
    }
    if (m_ScanScheduled[Activity::TeamTwo])
    {
        writer.NewProperty("ScanScheduledTeam2");
        writer << m_ScanScheduled[Activity::TeamTwo];
    }
    if (m_ScanScheduled[Activity::TeamThree])
    {
        writer.NewProperty("ScanScheduledTeam3");
        writer << m_ScanScheduled[Activity::TeamThree];
    }
    if (m_ScanScheduled[Activity::TeamFour])
    {
        writer.NewProperty("ScanScheduledTeam4");
        writer << m_ScanScheduled[Activity::TeamFour];
    }
	for (list<Area>::const_iterator aItr = m_AreaList.begin(); aItr != m_AreaList.end(); ++aItr)
	{
		// Only write the area if it has any boxes/area at all
		if (!(*aItr).HasNoArea())
		{
			writer.NewProperty("AddArea");
			writer << *aItr;
		}
	}
    writer.NewProperty("GlobalAcceleration");
    writer << m_GlobalAcc;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Scene object.

void Scene::Destroy(bool notInherited)
{
    delete m_pTerrain;
    delete m_pPathFinder;

    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
        delete m_ResidentBrains[player];

    for (int set = PLACEONLOAD; set < PLACEDSETSCOUNT; ++set)
    {
        for (list<SceneObject *>::iterator oItr = m_PlacedObjects[set].begin(); oItr != m_PlacedObjects[set].end(); ++oItr)
        {
            delete (*oItr);
            *oItr = 0;
        }
    }

    for (list<SceneLayer *>::iterator slItr = m_BackLayerList.begin(); slItr != m_BackLayerList.end(); ++slItr)
    {
        delete (*slItr);
        *slItr = 0;
    }

	for (list<Deployment *>::iterator slItr = m_Deployments.begin(); slItr != m_Deployments.end(); ++slItr)
	{
		delete (*slItr);
		*slItr = 0;
	}

    delete m_apUnseenLayer[Activity::TeamOne];
    delete m_apUnseenLayer[Activity::TeamTwo];
    delete m_apUnseenLayer[Activity::TeamThree];
    delete m_apUnseenLayer[Activity::TeamFour];

	//if (m_PreviewBitmapOwned)
	destroy_bitmap(m_pPreviewBitmap);
	m_pPreviewBitmap = 0;

    if (!notInherited)
        Entity::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  MigrateToModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this an original Preset in a different module than it was before.
//                  It severs ties deeply to the old module it was saved in.

bool Scene::MigrateToModule(int whichModule)
{
    if (!Entity::MigrateToModule(whichModule))
        return false;

    m_pTerrain->MigrateToModule(whichModule);

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FillUnseenLayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a new SceneLayer for a specific team and fills it with black
//                  pixels that end up being a specific size on the screen.

void Scene::FillUnseenLayer(Vector pixelSize, int team, bool createNow)
{
    if (team == Activity::NoTeam || !(pixelSize.m_X >= 1.0 && pixelSize.m_Y >= 1.0))
        return;

    m_UnseenPixelSize[team] = pixelSize;

    // Dynamically create the unseen layer
    // Create the bitmap to make the unseen scene layer out of
    if (createNow)
    {
        BITMAP *pUnseenBitmap = create_bitmap_ex(8, GetWidth() / m_UnseenPixelSize[team].m_X, GetHeight() / m_UnseenPixelSize[team].m_Y);
        clear_to_color(pUnseenBitmap, g_BlackColor);
        // Replace any old unseen layer with the new one that is generated
        delete m_apUnseenLayer[team];
        m_apUnseenLayer[team] = new SceneLayer();
        m_apUnseenLayer[team]->Create(pUnseenBitmap, true, Vector(), WrapsX(), WrapsY(), Vector(1.0, 1.0));
        // Calculate how many times smaller the unseen map is compared to the entire terrain's dimensions, and set it as the scale factor on the Unseen layer
        m_apUnseenLayer[team]->SetScaleFactor(Vector((float)GetTerrain()->GetBitmap()->w / (float)m_apUnseenLayer[team]->GetBitmap()->w, (float)GetTerrain()->GetBitmap()->h / (float)m_apUnseenLayer[team]->GetBitmap()->h));
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetUnseenLayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the unseen layer of a specific team.

void Scene::SetUnseenLayer(SceneLayer *pNewLayer, int team)
{
    if (team == Activity::NoTeam || !pNewLayer)
        return;

    // Replace any old unseen layer with the new one that is generated
    delete m_apUnseenLayer[team];
    m_apUnseenLayer[team] = pNewLayer;
    // Calculate how many times smaller the unseen map is compared to the entire terrain's dimensions, and set it as the scale factor on the Unseen layer
    m_apUnseenLayer[team]->SetScaleFactor(Vector((float)GetTerrain()->GetBitmap()->w / (float)m_apUnseenLayer[team]->GetBitmap()->w, (float)GetTerrain()->GetBitmap()->h / (float)m_apUnseenLayer[team]->GetBitmap()->h));
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearSeenPixels
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the pixels that have been seen on a team's unseen layer.

void Scene::ClearSeenPixels(int team)
{
    if (team != Activity::NoTeam)
    {
        // Clear all the pixels off the map, set them to key color
        if (m_apUnseenLayer[team])
        {
            for (list<Vector>::iterator itr = m_SeenPixels[team].begin(); itr != m_SeenPixels[team].end(); ++itr)
            {
                putpixel(m_apUnseenLayer[team]->GetBitmap(), (*itr).m_X, (*itr).m_Y, g_MaskColor);

                // Clean up around the removed pixels too
                CleanOrphanPixel((*itr).m_X + 1, (*itr).m_Y, W, team);
                CleanOrphanPixel((*itr).m_X - 1, (*itr).m_Y, E, team);
                CleanOrphanPixel((*itr).m_X, (*itr).m_Y + 1, N, team);
                CleanOrphanPixel((*itr).m_X, (*itr).m_Y - 1, S, team);
                CleanOrphanPixel((*itr).m_X + 1, (*itr).m_Y + 1, NW, team);
                CleanOrphanPixel((*itr).m_X - 1, (*itr).m_Y + 1, NE, team);
                CleanOrphanPixel((*itr).m_X - 1, (*itr).m_Y - 1, SE, team);
                CleanOrphanPixel((*itr).m_X + 1, (*itr).m_Y - 1, SW, team);
            }
        }

        // Now actually clear the list too
        m_SeenPixels[team].clear();

        // Transfer all cleaned pixels from orphans to the seen pixels for next frame
        for (list<Vector>::iterator itr = m_CleanedPixels[team].begin(); itr != m_CleanedPixels[team].end(); ++itr)
            m_SeenPixels[team].push_back(*itr);

        // We have moved the cleaned pixels to the seen pixels list, now clean up the list for next frame
        m_CleanedPixels[team].clear();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CleanOrphanPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks a specific unseen pixel for only having two or less unseen
//                  neighbors, and if so, makes it seen.

bool Scene::CleanOrphanPixel(int posX, int posY, NeighborDirection checkingFrom, int team)
{
    if (team == Activity::NoTeam || !m_apUnseenLayer[team])
        return false;

    // Do any necessary wrapping
    m_apUnseenLayer[team]->WrapPosition(posX, posY, false);

    // First check the actual position of the checked pixel, it may already been seen.
    if (getpixel(m_apUnseenLayer[team]->GetBitmap(), posX, posY) == g_MaskColor)
        return false;

    // Ok, not seen, so check surrounding pixels for 'support', ie unseen ones that will keep this also unseen
    float support = 0;
    int testPosX, testPosY;
    if (checkingFrom != E)
    {
        testPosX = posX + 1;
        testPosY = posY;
        m_apUnseenLayer[team]->WrapPosition(testPosX, testPosY, false);
        support += getpixel(m_apUnseenLayer[team]->GetBitmap(), testPosX, testPosY) != g_MaskColor ? 1 : 0;
    }
    if (checkingFrom != W)
    {
        testPosX = posX - 1;
        testPosY = posY;
        m_apUnseenLayer[team]->WrapPosition(testPosX, testPosY, false);
        support += getpixel(m_apUnseenLayer[team]->GetBitmap(), testPosX, testPosY) != g_MaskColor ? 1 : 0;
    }
    if (checkingFrom != S)
    {
        testPosX = posX;
        testPosY = posY + 1;
        m_apUnseenLayer[team]->WrapPosition(testPosX, testPosY, false);
        support += getpixel(m_apUnseenLayer[team]->GetBitmap(), testPosX, testPosY) != g_MaskColor ? 1 : 0;
    }
    if (checkingFrom != N)
    {
        testPosX = posX;
        testPosY = posY - 1;
        m_apUnseenLayer[team]->WrapPosition(testPosX, testPosY, false);
        support += getpixel(m_apUnseenLayer[team]->GetBitmap(), testPosX, testPosY) != g_MaskColor ? 1 : 0;
    }
    if (checkingFrom != SE)
    {
        testPosX = posX + 1;
        testPosY = posY + 1;
        m_apUnseenLayer[team]->WrapPosition(testPosX, testPosY, false);
        support += getpixel(m_apUnseenLayer[team]->GetBitmap(), testPosX, testPosY) != g_MaskColor ? 0.5f : 0;
    }
    if (checkingFrom != SW)
    {
        testPosX = posX - 1;
        testPosY = posY + 1;
        m_apUnseenLayer[team]->WrapPosition(testPosX, testPosY, false);
        support += getpixel(m_apUnseenLayer[team]->GetBitmap(), testPosX, testPosY) != g_MaskColor ? 0.5f : 0;
    }
    if (checkingFrom != NW)
    {
        testPosX = posX - 1;
        testPosY = posY - 1;
        m_apUnseenLayer[team]->WrapPosition(testPosX, testPosY, false);
        support += getpixel(m_apUnseenLayer[team]->GetBitmap(), testPosX, testPosY) != g_MaskColor ? 0.5f : 0;
    }
    if (checkingFrom != NE)
    {
        testPosX = posX + 1;
        testPosY = posY - 1;
        m_apUnseenLayer[team]->WrapPosition(testPosX, testPosY, false);
        support += getpixel(m_apUnseenLayer[team]->GetBitmap(), testPosX, testPosY) != g_MaskColor ? 0.5f : 0;
    }

    // Orphaned enough to remove?
    if (support <= 2.5)
    {
        putpixel(m_apUnseenLayer[team]->GetBitmap(), posX, posY, g_MaskColor);
        m_CleanedPixels[team].push_back(Vector(posX, posY));
        return true;
    }    

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDimensions
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total dimensions (width and height) of the scene, in pixels.

Vector Scene::GetDimensions() const
{
    return m_pTerrain ? Vector(m_pTerrain->GetBitmap()->w, m_pTerrain->GetBitmap()->h) : Vector();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total width of the scene, in pixels.

int Scene::GetWidth() const { return m_pTerrain ? m_pTerrain->GetBitmap()->w : 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total height of the scene, in pixels.

int Scene::GetHeight() const { return m_pTerrain ? m_pTerrain->GetBitmap()->h : 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WrapsX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the scene wraps its scrolling around the X axis.

bool Scene::WrapsX() const { return m_pTerrain ? m_pTerrain->WrapsX() : false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WrapsY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the scene wraps its scrolling around the Y axis.

bool Scene::WrapsY() const { return m_pTerrain ? m_pTerrain->WrapsY() : false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlaceResidentBrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Places the individual brain of a single player which may be stationed
//                  on this Scene, and registers them as such in an Activity.

bool Scene::PlaceResidentBrain(int player, Activity &newActivity)
{
    if (m_ResidentBrains[player])
    {
#ifdef DEBUG_BUILD
        RTEAssert(m_ResidentBrains[player]->GetTeam() == newActivity.GetTeamOfPlayer(player), "Resident Brain is of the wrong team!!");
#endif

        Actor *pBrainActor = dynamic_cast<Actor *>(m_ResidentBrains[player]);
        if (pBrainActor)// && pBrainActor->IsActor())
        {
            // Set the team before adding it to the MovableMan
            pBrainActor->SetTeam(newActivity.GetTeamOfPlayer(player));
            // Passing in ownership of the brain here
            g_MovableMan.AddActor(pBrainActor);
            // Register it with the Activity too
            newActivity.SetPlayerBrain(pBrainActor, player);
            // Clear the resident brain slot.. it may be set again at the end of the game, if this fella survives
            m_ResidentBrains[player] = 0;
            return true;
        }
// TODO: Handle brains being inside other things as residents??
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlaceResidentBrains
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Places the individual brains of the various players which may be
//                  stationed on this Scene, and registers them as such in an Activity.

int Scene::PlaceResidentBrains(Activity &newActivity)
{
    int found = 0;

    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
    {
        if (PlaceResidentBrain(player, newActivity))
            ++found;
    }

    return found;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RetrieveResidentBrains
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Looks at the Activity and its players' registered brain Actors, and
//                  saves them as resident brains for this Scene. Done when a fight is over
//                  and the survivors remain!

int Scene::RetrieveResidentBrains(Activity &oldActivity)
{
    int found = 0;

    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
    {
//        RTEAssert(oldActivity.GetPlayerBrain(player) && oldActivity.GetPlayerBrain(player)->GetTeam() == oldActivity.GetTeamOfPlayer(player), "Resident Brain is of the wrong team BEFORE being retrieved!!");

        // Replace existing brain residencies
        delete m_ResidentBrains[player];
        // Slurp up any brains and save em, transferring ownership when we release below
        m_ResidentBrains[player] = oldActivity.GetPlayerBrain(player);
        // Nullify the Activity brain
        oldActivity.SetPlayerBrain(0, player);
        // Try to find and remove the activity's brain actors in the MO pools, releasing ownership
        if (g_MovableMan.RemoveActor(dynamic_cast<Actor *>(m_ResidentBrains[player])))
            ++found;
        // If failed to find, then we didn't retrieve it
        else
            m_ResidentBrains[player] = 0;

//        RTEAssert(m_ResidentBrains[player] && m_ResidentBrains[player]->GetTeam() == oldActivity.GetTeamOfPlayer(player), "Resident Brain is of the wrong team AFTER being retrieved!!");
    }

    return found;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RetrieveActorsAndDevices
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sucks up all the Actors and Devices currently active in MovableMan and
//                  puts them into this' list of objects to place on next load.
//                  Should be done AFTER RetrieveResidentBrains!

int Scene::RetrieveActorsAndDevices(int onlyTeam, bool noBrains)
{
    int found = 0;
    
    // Suck out all the Actors from the MovableMan - TAKING OVER ownership
    found += g_MovableMan.EjectAllActors(m_PlacedObjects[PLACEONLOAD], onlyTeam, noBrains);
    // Suck out all the Items from the MovableMan - TAKING OVER ownership
    found += g_MovableMan.EjectAllItems(m_PlacedObjects[PLACEONLOAD]);

    return found;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddPlacedObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a SceneObject to be placed in this scene. Ownership IS transferred!

void Scene::AddPlacedObject(int whichSet, SceneObject *pObjectToAdd, int listOrder)
{
    if (!pObjectToAdd)
        return;

	// Create unique ID for this deployment
	Deployment * pDeployment = dynamic_cast<Deployment *>(pObjectToAdd);
	if (pDeployment)
		pDeployment->NewID();

    if (listOrder < 0 || listOrder >= m_PlacedObjects[whichSet].size())
        m_PlacedObjects[whichSet].push_back(pObjectToAdd);
    else
    {
        // Find the spot
        list<SceneObject *>::iterator itr = m_PlacedObjects[whichSet].begin();
        for (int i = 0; i != listOrder && itr != m_PlacedObjects[whichSet].end(); ++i, ++itr)
            ;

        // Put 'er in
        m_PlacedObjects[whichSet].insert(itr, pObjectToAdd);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemovePlacedObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a SceneObject placed in this scene.

void Scene::RemovePlacedObject(int whichSet, int whichToRemove)
{
    if (m_PlacedObjects[whichSet].empty())
        return;

    if (whichToRemove < 0 || whichToRemove >= m_PlacedObjects[whichSet].size())
    {
        delete (m_PlacedObjects[whichSet].back());
        m_PlacedObjects[whichSet].pop_back();
    }
    else
    {
        // Find the spot
        list<SceneObject *>::iterator itr = m_PlacedObjects[whichSet].begin();
        for (int i = 0; i != whichToRemove && itr != m_PlacedObjects[whichSet].end(); ++i, ++itr)
            ;

        delete (*itr);
        m_PlacedObjects[whichSet].erase(itr);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PickPlacedObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the last placed object that graphically overlaps an absolute
//                  point in the scene.

const SceneObject * Scene::PickPlacedObject(int whichSet, Vector &scenePoint, int *pListOrderPlace) const
{
    // REVERSE!
    int i = m_PlacedObjects[whichSet].size() - 1;
    for (list<SceneObject *>::const_reverse_iterator itr = m_PlacedObjects[whichSet].rbegin(); itr != m_PlacedObjects[whichSet].rend(); ++itr, --i)
    {
        if ((*itr)->IsOnScenePoint(scenePoint))
        {
            if (pListOrderPlace)
                *pListOrderPlace = i;
            return *itr;
        }
    }

    if (pListOrderPlace)
        *pListOrderPlace = -1;
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PickPlacedActorInRange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the last placed actor object that is closer than range to scenePoint
//
// Arguments:       Which set of placed objects to pick from. See the PlacedObjectSets enum.
//                  The point in absolute scene coordinates that will be used to pick the
//                  closest placed SceneObject near it.
//                  The range to check for nearby objects.
//                  An int which will be filled out with the order place of any found object
//                  in the list. if nothing is found, it will get a value of -1.
//
// Return value:    The closest actor SceneObject, if any. Ownership is NOT transferred!

const SceneObject * Scene::PickPlacedActorInRange(int whichSet, Vector &scenePoint, int range, int *pListOrderPlace) const
{
	SceneObject * pFoundObject = 0;
	float distance = range;
	
	// REVERSE!
    int i = m_PlacedObjects[whichSet].size() - 1;
    for (list<SceneObject *>::const_reverse_iterator itr = m_PlacedObjects[whichSet].rbegin(); itr != m_PlacedObjects[whichSet].rend(); ++itr, --i)
    {
		if (dynamic_cast<const Actor *>(*itr))
		{
			float d = g_SceneMan.ShortestDistance((*itr)->GetPos(), scenePoint, true).GetMagnitude();
			if (d < distance)
			{
				if (pListOrderPlace)
					*pListOrderPlace = i;
				distance = d;
				pFoundObject = *itr;
			}
		}
    }

	if (pFoundObject)
		return pFoundObject;

    if (pListOrderPlace)
        *pListOrderPlace = -1;
    return 0;
}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePlacedObjects
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updated the objects in the placed scene objects list of this. This is
//                  mostly for the editor to represent the items correctly.

void Scene::UpdatePlacedObjects(int whichSet)
{
    if (whichSet == PLACEONLOAD)
    {
        for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
            if (m_ResidentBrains[player])
                m_ResidentBrains[player]->Update();
    }

    for (list<SceneObject *>::iterator itr = m_PlacedObjects[whichSet].begin(); itr != m_PlacedObjects[whichSet].end(); ++itr)
    {
        (*itr)->Update();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearPlacedObjectSet
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes all entries in a specific set of placed Objects.

int Scene::ClearPlacedObjectSet(int whichSet)
{
    int count = 0;
    for (list<SceneObject *>::iterator itr = m_PlacedObjects[whichSet].begin(); itr != m_PlacedObjects[whichSet].end(); ++itr)
    {
        delete *itr;
        (*itr) = 0;
        ++count;
    }
    m_PlacedObjects[whichSet].clear();

    return count;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetResidentBrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the resident brain Actor of a specific player from this scene,
//                  if there is any. OWNERSHIP IS NOT TRANSFERRED!

SceneObject * Scene::GetResidentBrain(int player) const
{
//    if (m_ResidentBrains[player])
        return m_ResidentBrains[player];

//    for (list<SceneObject *>::iterator itr = m_PlacedObjects[PLACEONLOAD].begin(); itr != m_PlacedObjects[PLACEONLOAD].end(); ++itr)
//    {
//        (*itr)->teamUpdate();
//    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetResidentBrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the resident brain Actor of a specific player from this scene,
//                  if there is any. Ownership IS transferred!

void Scene::SetResidentBrain(int player, SceneObject *pNewBrain)
{
    delete m_ResidentBrains[player];
    m_ResidentBrains[player] = pNewBrain;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetResidentBrainCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of brains currently residing in this scene.

int Scene::GetResidentBrainCount() const
{
    int count = 0;
    for (int p = Players::PlayerOne; p < Players::MaxPlayerCount; ++p)
    {
        if (m_ResidentBrains[p])
            count++;
    }
    return count;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds or modifies an existing area of this Scene.

bool Scene::SetArea(Area &newArea)
{
    for (list<Area>::iterator aItr = m_AreaList.begin(); aItr != m_AreaList.end(); ++aItr)
    {
        // Try to find an existing area of the same name
        if ((*aItr).GetName() == newArea.GetName())
        {
            // Deep copy into the existing area
            (*aItr).Reset();
            (*aItr).Create(newArea);
            return true;
        }
    }
    // Couldn't find one, so just add the new Area
    m_AreaList.push_back(newArea);

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks for the existence of a specific Area identified by a name.
//                  This won't throw any errors to the console if the Area isn't found.

bool Scene::HasArea(string areaName)
{
    for (list<Area>::iterator aItr = m_AreaList.begin(); aItr != m_AreaList.end(); ++aItr)
    {
        if ((*aItr).GetName() == areaName)
            return true;
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific area box identified by a name. Ownership is NOT transferred!

Scene::Area * Scene::GetArea(string areaName)
{
    for (list<Area>::iterator aItr = m_AreaList.begin(); aItr != m_AreaList.end(); ++aItr)
    {
        if ((*aItr).GetName() == areaName)
            return &(*aItr);
    }

    g_ConsoleMan.PrintString("ERROR: Could not find the requested Scene Area named: " + areaName);
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a specific Area identified by a name.

bool Scene::RemoveArea(std::string areaName)
{
    for (list<Area>::iterator aItr = m_AreaList.begin(); aItr != m_AreaList.end(); ++aItr)
    {
        if ((*aItr).GetName() == areaName)
        {
            m_AreaList.erase(aItr);
            return true;
        }
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WithinArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if a point is within a specific named Area of this Scene. If
//                  no Area of the name is found, this just returns false without error.
// Arguments:       The name of the Area to try to check against.

bool Scene::WithinArea(string areaName, const Vector &point) const
{
    if (areaName.empty())
        return false;

    for (list<Area>::const_iterator aItr = m_AreaList.begin(); aItr != m_AreaList.end(); ++aItr)
    {
        if ((*aItr).GetName() == areaName && (*aItr).IsInside(point))
            return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTeamOwnership
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the team who owns this Scene in a Metagame

void Scene::SetTeamOwnership(int newTeam)
{
    m_OwnedByTeam = newTeam;

    // Go through all the things placed and make sure they are all set to the new owner team
    for (int set = PLACEONLOAD; set <= AIPLAN; ++set)
    {
        for (list<SceneObject *>::const_iterator bpItr = m_PlacedObjects[set].begin(); bpItr != m_PlacedObjects[set].end(); ++bpItr)
        {
            if (*bpItr)
                (*bpItr)->SetTeam(m_OwnedByTeam);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalcBuildBudgetUse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Figure out exactly how much of the build budget would be used if
//                  as many blueprint objects as can be afforded and exists would be built.

float Scene::CalcBuildBudgetUse(int player, int *pAffordCount, int *pAffordAIPlanCount) const
{
    if (pAffordCount)
        *pAffordCount = 0;

    if (player < Players::PlayerOne || player >= Players::MaxPlayerCount)
        return 0;

    // Take metaplayer tech modifiers into account when calculating costs of this Deployment spawn
    int nativeModule = 0;
    float foreignCostMult = 1.0;
	float nativeCostMult = 1.0;
    MetaPlayer *pMetaPlayer = g_MetaMan.GetMetaPlayerOfInGamePlayer(player);
    if (g_MetaMan.GameInProgress() && pMetaPlayer)
    {
        nativeModule = pMetaPlayer->GetNativeTechModule();
        foreignCostMult = pMetaPlayer->GetForeignCostMultiplier();
        nativeCostMult = pMetaPlayer->GetNativeCostMultiplier();
    }

    // Go through the list of blueprint objects and move as many as can be afforded by the budget to the list that is placed on next scene load
    int objCount = 0;
    float fundsAfforded = 0;
    float budget = m_BuildBudget[player];
    float objectCost = 0;
    Deployment *pDeployment = 0;
    SceneObject *pObjectToPlace = 0;
    // The last resident brain that is encountered in the building list, starting with the preexisting resident brain. Not owned here
    SceneObject *pLastBrain = m_ResidentBrains[player];
    // The total list of objects that WILL be placed as the building phase goes on for real - nothing is owned by this!
    list<SceneObject *> virtualPlacedList;
    // Add all the already placed objects int he scene to it; then we'll add the objects that would be placed this round - ownership is NOT passed
    for (list<SceneObject *>::const_iterator placedItr = m_PlacedObjects[PLACEONLOAD].begin(); placedItr != m_PlacedObjects[PLACEONLOAD].end(); ++placedItr)
        virtualPlacedList.push_back(*placedItr);

    // First go through the blueprints that are already placed, THEN go through the AI plan objects if we are specified to
    for (int set = BLUEPRINT; set <= AIPLAN; ++set)
    {
        // Skip the AI plan set if we're not asked to consider it
        if (set == AIPLAN && !pAffordAIPlanCount)
            continue;

        // Two passes, one for only things placed by this player, second to see if we can still afford any placed by teammates
        for (int pass = 0; pass < 2; ++pass)
        {
            for (list<SceneObject *>::const_iterator bpItr = m_PlacedObjects[set].begin(); bpItr != m_PlacedObjects[set].end(); ++bpItr)
            {
                // Skip objects on the first pass that aren't placed by this player
                // Skip objects on the second pass that WERE placed by this player.. because we already counted them 
                if ((pass == 0 && (*bpItr)->GetPlacedByPlayer() != player) ||
                    (pass == 1 && (*bpItr)->GetPlacedByPlayer() == player))
                    continue;

                // If Deployment, we need to check if we're going to be spawning something this building round or not
                pDeployment = dynamic_cast<Deployment *>(*bpItr);
                if (pDeployment)
                {
                    // Reset the object cost because the creating of the Deployment spawn only adds to the passed-in tally
                    objectCost = 0;
                    // See if we can spawn an Actor from this Deployment
                    pObjectToPlace = pDeployment->CreateDeployedActor(player, objectCost);
                    // If not an Actor, at least an item?
                    if (!pObjectToPlace)
                        pObjectToPlace = pDeployment->CreateDeployedObject(player, objectCost);

                    // Only place things if there isn't somehting similar of the same team within a radius
                    if (pObjectToPlace)
                    {
                        // If there's already similar placed in the scene that is close enough to this Deployment to block it, then abort placing the new spawn!
                        // We're passing in the virtual list of things that already were placed before, and that have been placed up til now in this build phase
                        if (pDeployment->DeploymentBlocked(player, virtualPlacedList))
                        {
                            delete pObjectToPlace;
                            pObjectToPlace = 0;
                        }
                    }

                    // If we didn't end up spawning anything, just continue to the next thing in the blueprint queue
                    if (!pObjectToPlace)
                        continue;
                }
                // Regular object, will always be bought
                else
                {
                    pObjectToPlace = *bpItr;
					if (pObjectToPlace)
						objectCost = pObjectToPlace->GetGoldValue(nativeModule, foreignCostMult, nativeCostMult);
                }

                // If this is a brain, then we will replace any previous/existing resident brain with this one, and adjust the difference in cost
                if (pObjectToPlace && pObjectToPlace->IsInGroup("Brains") && pLastBrain)
                {
                    objectCost = pObjectToPlace->GetTotalValue(nativeModule, foreignCostMult, nativeCostMult) - pLastBrain->GetTotalValue(nativeModule, foreignCostMult, nativeCostMult);
                    pLastBrain = pObjectToPlace;
                }

                // Check if the remaining budget allows for this item
                if (budget >= objectCost)
                {
                    // Add it to the virtual list of things we're comparing against for spawn blockages - ownership is NOT passed
                    virtualPlacedList.push_back(pObjectToPlace);
                    fundsAfforded += objectCost;
                    budget -= objectCost;
                    objCount++;
                    // Count the number of AI Plan objects we can afford, if we're asked to do so
                    if (set == AIPLAN && pAffordAIPlanCount)
                        (*pAffordAIPlanCount)++;
                }
                // That's it, we can't afford the next item in the queue
                else
                    break;
            }
        }
    }

    // Report the number of objects actually built
    if (pAffordCount)
        *pAffordCount = objCount;

    return fundsAfforded;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyAIPlan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Puts the pre-built AI base plan into effect by transferring as many
//                  pieces as the current base budget allows from the AI plan to the actual
//                  blueprints to be built at this Scene.

float Scene::ApplyAIPlan(int player, int *pObjectsApplied)
{
    if (pObjectsApplied)
        *pObjectsApplied = 0;

    if (player < Players::PlayerOne || player >= Players::MaxPlayerCount)
        return 0;

    // Take metaplayer tech modifiers into account when calculating costs of this Deployment spawn
    int nativeModule = 0;
    float foreignCostMult = 1.0;
	float nativeCostMult = 1.0;
    MetaPlayer *pMetaPlayer = g_MetaMan.GetMetaPlayerOfInGamePlayer(player);
    if (g_MetaMan.GameInProgress() && pMetaPlayer)
    {
        nativeModule = pMetaPlayer->GetNativeTechModule();
        foreignCostMult = pMetaPlayer->GetForeignCostMultiplier();
        nativeCostMult = pMetaPlayer->GetNativeCostMultiplier();
    }

    float valueOfApplied = 0;
    int totalToBuildCount = 0;
    int affordAIPlanObjectsCount = 0;
    // Figure out how many objects in the bluprints we can afford to build already, without adding AI plans
    float bpTotalValue = CalcBuildBudgetUse(player, &totalToBuildCount, &affordAIPlanObjectsCount);

    // If we have budget enough to build everything we have in blueprints already, AND dip into AI plans,
    // then we should move over as many Objects from the AI plan queue to the blueprint as we have been told we can afford
    float objValue = 0;
    for (int i = 0; i < affordAIPlanObjectsCount && !m_PlacedObjects[AIPLAN].empty(); ++i)
    {
        // Get the object off the AI plan
        SceneObject *pObject = m_PlacedObjects[AIPLAN].front();
		if (pObject)
		{
			m_PlacedObjects[AIPLAN].pop_front();

			// How much does it cost? Add it to the total blueprint tally
			objValue = pObject->GetTotalValue(nativeModule, foreignCostMult, nativeCostMult);
			bpTotalValue += objValue;
			valueOfApplied += objValue;

			// Mark this as having been placed by this player
			pObject->SetPlacedByPlayer(player);

			// Create unique ID for this deployment
			Deployment * pDeployment = dynamic_cast<Deployment *>(pObject);
			if (pDeployment)
				pDeployment->NewID();

			// Now add it to the blueprint queue
			m_PlacedObjects[BLUEPRINT].push_back(pObject);

			// Count it
			if (pObjectsApplied)
				++(*pObjectsApplied);
		}
    }

    return valueOfApplied;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyBuildBudget
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Actually builds as many objects in the specific player's Blueprint
//                  list as can be afforded by his build budget. The budget is deducted
//                  accordingly.

float Scene::ApplyBuildBudget(int player, int *pObjectsBuilt)
{
    if (pObjectsBuilt)
        *pObjectsBuilt = 0;

    if (player < Players::PlayerOne || player >= Players::MaxPlayerCount)
        return 0;

    // Take metaplayer tech modifiers into account when calculating costs of this Deployment spawn
    int nativeModule = 0;
    float foreignCostMult = 1.0;
	float nativeCostMult = 1.0;
    int team = Activity::NoTeam;
    MetaPlayer *pMetaPlayer = g_MetaMan.GetMetaPlayerOfInGamePlayer(player);
    if (g_MetaMan.GameInProgress() && pMetaPlayer)
    {
        nativeModule = pMetaPlayer->GetNativeTechModule();
        foreignCostMult = pMetaPlayer->GetForeignCostMultiplier();
        nativeCostMult = pMetaPlayer->GetNativeCostMultiplier();

        // Also find out the team so we can apply it to the things we are building
        team = pMetaPlayer->GetTeam();
    }

    // Go through the list of blueprint objects and move as many as can be afforded by the budget to the list that is placed on next scene load
    bool remove = false;
    int placedCount = 0;
    float fundsSpent = 0;
    float objectCost = 0;
    list<SceneObject *>::iterator bpItr;
    list<SceneObject *>::iterator delItr;
    SceneObject *pObjectToPlace = 0;
    Deployment *pDeployment = 0;
    // The last resident brain that is encountered in the building list, starting with the preexisting resident brain. Not owned here
    SceneObject *pLastBrain = m_ResidentBrains[player];
    // Two passes, one for only things placed by this player, second to see if we can still afford any placed by teammates
    for (int pass = 0; pass < 2; ++pass)
    {
        for (bpItr = m_PlacedObjects[BLUEPRINT].begin(); bpItr != m_PlacedObjects[BLUEPRINT].end();)
        {
            // Skip objects on the first pass that aren't placed by this player
            // Skip objects on the second pass that WERE placed by this player.. because we already went through them on first pass
            if ((pass == 0 && (*bpItr)->GetPlacedByPlayer() != player) ||
                (pass == 1 && (*bpItr)->GetPlacedByPlayer() == player))
            {
                // Increment since the for loop header doesn't do it
                ++bpItr;
                continue;
            }

            // If Deployment, just add the new instances of whatever we're spawning
            // and LEAVE the blueprint Deployment where it is in the queue, so it can keep spitting out new stuff each buying round
            pDeployment = dynamic_cast<Deployment *>(*bpItr);
            if (pDeployment)
            {
                // Set the team
                pDeployment->SetTeam(team);
                // If there's already similar placed in the scene that is close enough to this Deployment to block it, then don't count it!
                if (pDeployment->DeploymentBlocked(player, m_PlacedObjects[PLACEONLOAD]))
                {
                    ++bpItr;
                    continue;
                }
                // Okay there's a valid deployment happening here, so count how much it costs
                else
                {
                    // Reset the object cost because the creating of the Deployment spawn only adds to the passed-in tally
                    objectCost = 0;
                    // See if we can spawn an Actor from this Deployment
                    pObjectToPlace = pDeployment->CreateDeployedActor(player, objectCost);

					// Assign deployment ID to placed actor
					if (pObjectToPlace)
					{
						if (!pDeployment->GetID())
							pDeployment->NewID();

						Actor *pActor = dynamic_cast<Actor *>(pObjectToPlace);
						if (pActor)
							pActor->SetDeploymentID(pDeployment->GetID());
					}

                    // If this is a BRAIN, replace the old resident brain with this new one
                    if (pObjectToPlace->IsInGroup("Brains"))
                    {
                        // Get a refund for the previous brain we are replacing
                        if (m_ResidentBrains[player])
                        {
                            float refund = m_ResidentBrains[player]->GetTotalValue(nativeModule, foreignCostMult, nativeCostMult);
                            m_BuildBudget[player] += refund;
                            fundsSpent -= refund;
                            delete m_ResidentBrains[player];
                        }
                        // Deduct the cost of the new brain from the budget and tally the total
                        m_BuildBudget[player] -= objectCost;
                        fundsSpent += objectCost;
                        // Transfer ownership of the brain to the scene.. we are done with this object now
                        m_ResidentBrains[player] = pObjectToPlace;
                        // We are done with this since it's a special brain placement; continue to next item
                        pObjectToPlace = 0;
                    }
                    else
                    {
                        // If not an Actor, at least an item?
                        if (!pObjectToPlace)
                            pObjectToPlace = pDeployment->CreateDeployedObject(player, objectCost);
                    }
                }
            }
            // A regular blueprint object which only gets placed once and then removed from blueprints
            else
            {
                pObjectToPlace = *bpItr;
				if (pObjectToPlace)
	                // Get the cost here since a deployment spawn above already gets it
		            objectCost = pObjectToPlace->GetGoldValue(nativeModule, foreignCostMult, nativeCostMult);
            }

            // If we didn't end up spawning anything, just continue to the next thing in the blueprint queue
            if (!pObjectToPlace)
            {
                ++bpItr;
                continue;
            }

            // If this is a brain, then we will replace any previous/existing resident brain with this one, and adjust the difference in cost
            if (pObjectToPlace && pObjectToPlace->IsInGroup("Brains") && pLastBrain)
            {
                objectCost = pObjectToPlace->GetTotalValue(nativeModule, foreignCostMult, nativeCostMult) - pLastBrain->GetTotalValue(nativeModule, foreignCostMult, nativeCostMult);
                pLastBrain = pObjectToPlace;
            }

            // Check if the remaining budget allows for this thing to be placed
            if (m_BuildBudget[player] >= objectCost)
            {
                // Deduct the cost from the budget and tally the total
                m_BuildBudget[player] -= objectCost;
                fundsSpent += objectCost;
                // Set the team of the thing we're building
                pObjectToPlace->SetTeam(team);
                // Mark this as having been placed by this player
                pObjectToPlace->SetPlacedByPlayer(player);
                // If applicable, replace the old resident brain with this new one
                if (pObjectToPlace->IsInGroup("Brains"))
                {
                    // Get a refund for the previous brain we are replacing
                    if (m_ResidentBrains[player])
                    {
                        float refund = m_ResidentBrains[player]->GetTotalValue(nativeModule, foreignCostMult, nativeCostMult);
                        m_BuildBudget[player] += refund;
                        fundsSpent -= refund;
                        delete m_ResidentBrains[player];
                        // Don't count the 'replacing' of a brain as an item
                    }
                    // If there wasn't a brain before, then count this as a new placement
                    else
                        placedCount++;
                    m_ResidentBrains[player] = pObjectToPlace;
                }
                // Regular non-brain object; simply move it to the list of objects to place on next load, TRANSFERRING OWNERSHIP
                else
                {
                    m_PlacedObjects[PLACEONLOAD].push_back(pObjectToPlace);
                    // Always count regular objects placed
                    placedCount++;

					// Add placed object's locations as boxes to the specified 'MetaBase' area
					TerrainObject * pTO = dynamic_cast<TerrainObject *>(pObjectToPlace);
					if (pTO)
					{
						if (HasArea(METABASE_AREA_NAME))
						{
							Scene::Area * metaBase = GetArea(METABASE_AREA_NAME);
							if (metaBase)
							{
								float x1 = pTO->GetPos().m_X + pTO->GetBitmapOffset().m_X;
								float y1 = pTO->GetPos().m_Y + pTO->GetBitmapOffset().m_Y;
								float x2 = x1 + pTO->GetBitmapWidth();
								float y2 = y1 + pTO->GetBitmapHeight();

								metaBase->AddBox(Box(x1, y1, x2, y2));
							}
						}
					}
                }

                // Save the iterator so we can remove its entry after we increment
                delItr = bpItr;
                // Don't remove Deployment blueprints; they remain in the blueprints and re-spawn their Loadouts each building round
                remove = pDeployment ? false : true;
            }
            // Ok we can't afford any more stuff in the queue
            else
                break;

            // Increment to next placed object
            ++bpItr;
            // Remove the previous entry from the blueprint list if it was built and added to the PLACEONLOAD list
            // DON'T remove any Deployment objects; they will re-spawn their Loadout things anew each building round!
            if (remove)
                m_PlacedObjects[BLUEPRINT].erase(delItr);
            remove = false;
        }
    }

	// Traverse through all brain hideouts to always move the brain to the last available brain hideout no matter what
    if (m_ResidentBrains[player] && dynamic_cast<AHuman *>(m_ResidentBrains[player]))
		for (bpItr = m_PlacedObjects[BLUEPRINT].begin(); bpItr != m_PlacedObjects[BLUEPRINT].end(); bpItr++)
		{
			pDeployment = dynamic_cast<Deployment *>(*bpItr);
			if (pDeployment)
			{
				// If this is a brain hideout, then move curent brain to new hideout or infantry brain deployment
				if (pDeployment->GetPresetName() == "Brain Hideout" || pDeployment->GetPresetName() == "Infantry Brain")
				{
					// Get a refund for the previous brain we are replacing
					if (m_ResidentBrains[player])
					{
						m_ResidentBrains[player]->SetPos(pDeployment->GetPos());
					}
				}
			}
		}

    // Report the number of objects actually built
    if (pObjectsBuilt)
        *pObjectsBuilt = placedCount;

    // Add the spent amount to the tally of total investments in this scene
    m_TotalInvestment += fundsSpent;

    return fundsSpent;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveAllPlacedActors
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Remove all actors that are in the placed set of objects to load for
//                  this scene. All except for an optionally specified team, that is.

int Scene::RemoveAllPlacedActors(int exceptTeam)
{
    int removedCount = 0;

    bool remove = false;
    Actor *pActor = 0;
    list<SceneObject *>::iterator soItr;
    list<SceneObject *>::iterator delItr;

    // Scrub both blueprints and the stuff that is already bought and about to be placed on loading the scene
    for (int set = PLACEONLOAD; set <= BLUEPRINT; ++set)
    {
        for (soItr = m_PlacedObjects[set].begin(); soItr != m_PlacedObjects[set].end();)
        {
            remove = false;
            // Only look for actors of any team except the specified exception team
            pActor = dynamic_cast<Actor *>(*soItr);
            if (pActor && pActor->GetTeam() != exceptTeam)
            {
                // Mark for removal
                remove = true;
                delItr = soItr;
            }

            // Increment to next placed object
            ++soItr;

            // Remove the previous entry from the set list now after we incremented so our soItr can remain valid
            if (remove)
            {
                // Properly destroy and remove the entry from the set list
                delete (*delItr);
                m_PlacedObjects[set].erase(delItr);
                removedCount++;
            }
        }
    }

    return removedCount;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOwnerOfAllDoors
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the ownership of all doors placed in this scene to a specific team
// Arguments:       The team to change the ownership to

int Scene::SetOwnerOfAllDoors(int team, int player)
{
    int changedCount = 0;

    ADoor *pDoor = 0;
    list<SceneObject *>::iterator soItr;

    // Affect both blueprints and the stuff that is already bought and about to be placed on loading the scene
    for (int set = PLACEONLOAD; set <= BLUEPRINT; ++set)
    {
        for (soItr = m_PlacedObjects[set].begin(); soItr != m_PlacedObjects[set].end(); ++soItr)
        {
            // Only mess with doors
            if (pDoor = dynamic_cast<ADoor *>(*soItr))
            {
                // Update team
                pDoor->SetTeam(team);
                // Update which player placed this
                pDoor->SetPlacedByPlayer(player);
                ++changedCount;
            }
        }
    }

    return changedCount;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResetPathFinding
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Recalculates all of the pathfinding data. This is very expensive, so
//                  do very rarely!

void Scene::ResetPathFinding()
{
    if (m_pPathFinder)
        m_pPathFinder->RecalculateAllCosts();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePathFinding
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Recalculates only the areas of the pathfinding data that have been
//                  marked as outdated.

void Scene::UpdatePathFinding()
{
    m_pPathFinder->RecalculateAreaCosts(m_pTerrain->GetUpdatedMaterialAreas());
    m_pTerrain->ClearUpdatedAreas();
    m_PartialPathUpdateTimer.Reset();
    m_PathfindingUpdated = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculatePath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates and returns the least difficult path between two points on
//                  the current scene. Takes both distance and materials into account.

float Scene::CalculatePath(const Vector &start, const Vector &end, std::list<Vector> &pathResult, float digStrenght)
{
    float totalCostResult = -1;
    if (m_pPathFinder)
    {
        int result = m_pPathFinder->CalculatePath(start, end, pathResult, totalCostResult, digStrenght);

        // It's ok if start and end nodes happen to be the same, the exact pixel locations are added at the front and end of the result regardless
        return (result == micropather::MicroPather::SOLVED || result == micropather::MicroPather::START_END_SAME) ? totalCostResult : -1;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateScenePath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the least difficult path between two points on
//                  the current scene. Takes both distance and materials into account.
//                  A list of waypoints can be retrived from m_ScenePath;
//                  For exposing CalculatePath to Lua.

int Scene::CalculateScenePath(const Vector start, const Vector end, bool movePathToGround, float digStrength)
{
    int pathSize = -1;
    if (m_pPathFinder)
    {
        float notUsed;
        m_pPathFinder->CalculatePath(start, end, m_ScenePath, notUsed, digStrength);

        // Process the new path we now have, if any
        if (!m_ScenePath.empty())
        {
            pathSize = m_ScenePath.size();
            if (movePathToGround)
            {
                // Smash all airborne waypoints down to just above the ground
                list<Vector>::iterator finalItr = m_ScenePath.end();
                for (list<Vector>::iterator lItr = m_ScenePath.begin(); lItr != finalItr; ++lItr)
                    (*lItr) = g_SceneMan.MovePointToGround((*lItr), 20, 15);
            }
        }
    }
    
    return pathSize;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Lock
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Locks all dynamic internal scene bitmaps so that manipulaitons of the
//                  scene's color and matter representations can take place.
//                  Doing it in a separate method like this is more efficient because
//                  many bitmap manipulaitons can be performed between a lock and unlock.
//                  UnlockScene() should always be called after accesses are completed.

void Scene::Lock()
{
//    RTEAssert(!m_Locked, "Hey, locking already locked scene!");
    if (!m_Locked)
    {
        m_pTerrain->LockBitmaps();
        m_Locked = true;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Unlock
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Unlocks the scene's bitmaps and prevents access to display memory.
//                  Doing it in a separate method like this is more efficient because
//                  many bitmap accesses can be performed between a lock and an unlock.
//                  UnlockScene() should only be called after LockScene().

void Scene::Unlock()
{
//    RTEAssert(m_Locked, "Hey, unlocking already unlocked scene!");
    if (m_Locked)
    {
        m_pTerrain->UnlockBitmaps();
        m_Locked = false;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this Scene. Supposed to be done every frame
//                  before drawing.

void Scene::Update()
{
    m_PathfindingUpdated = false;

	if (g_SettingsMan.BlipOnRevealUnseen())
	{
		// Highlight the pixels that have been revealed on the unseen maps
		for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
		{
			if (m_apUnseenLayer[team])
			{
				for (list<Vector>::iterator itr = m_SeenPixels[team].begin(); itr != m_SeenPixels[team].end(); ++itr)
				{
					putpixel(m_apUnseenLayer[team]->GetBitmap(), (*itr).m_X, (*itr).m_Y, g_WhiteColor);
				}
			}
		}
	}

    // Do full update every two minutes
    if (m_FullPathUpdateTimer.IsPastSimMS(120000))
    {
        m_pPathFinder->RecalculateAllCosts();
        m_FullPathUpdateTimer.Reset();
        m_PathfindingUpdated = true;
    }

    // Do partial update every 10 seconds
    if (m_PartialPathUpdateTimer.IsPastRealMS(10000))
        UpdatePathFinding();
}

} // namespace RTE
