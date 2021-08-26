//////////////////////////////////////////////////////////////////////////////////////////
// File:            Deployment.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Deployment class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Deployment.h"
#include "PresetMan.h"
#include "MetaMan.h"
#include "ContentFile.h"
#include "Loadout.h"
#include "MetaPlayer.h"
#include "Actor.h"
#include "ACraft.h"
#include "ActivityMan.h"
#include "DataModule.h"

namespace RTE {

ConcreteClassInfo(Deployment, SceneObject, 0);


std::vector<BITMAP *> Deployment::m_apArrowLeftBitmap;
std::vector<BITMAP *> Deployment::m_apArrowRightBitmap;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Deployment, effectively
//                  resetting the members of this abstraction level only.

void Deployment::Clear()
{
    m_LoadoutName = "Default";
    m_Icon.Reset();
    m_SpawnRadius = 40;
	m_WalkRadius = 250;
	m_ID = 0;
	m_HFlipped = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Deployment object ready for use.

int Deployment::Create()
{
    if (SceneObject::Create() < 0)
        return -1;

	if (m_apArrowLeftBitmap.empty())
	{
		ContentFile("Base.rte/GUIs/DeploymentIcons/ArrowLeft.png").GetAsAnimation(m_apArrowLeftBitmap, 1);
	}
	if (m_apArrowRightBitmap.empty())
	{
		ContentFile("Base.rte/GUIs/DeploymentIcons/ArrowRight.png").GetAsAnimation(m_apArrowRightBitmap, 1);
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Deployment object ready for use.

int Deployment::Create(string loadoutName, const Icon &icon, float spawnRadius)
{
    m_LoadoutName = loadoutName;
    m_Icon = icon;
    m_SpawnRadius = spawnRadius;
	m_WalkRadius = 250;
	m_ID = 0;
	m_HFlipped = false;

	if (m_apArrowLeftBitmap.empty())
	{
		ContentFile("Base.rte/GUIs/DeploymentIcons/ArrowLeft.png").GetAsAnimation(m_apArrowLeftBitmap, 1);
	}
	if (m_apArrowRightBitmap.empty())
	{
		ContentFile("Base.rte/GUIs/DeploymentIcons/ArrowRight.png").GetAsAnimation(m_apArrowRightBitmap, 1);
	}

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a MOPixel to be identical to another, by deep copy.

int Deployment::Create(const Deployment &reference)
{
    SceneObject::Create(reference);

    m_LoadoutName = reference.m_LoadoutName;
    m_Icon = reference.m_Icon;
    m_SpawnRadius = reference.m_SpawnRadius;
	m_WalkRadius = reference.m_WalkRadius;
    m_ID = reference.m_ID;
	m_HFlipped = reference.m_HFlipped;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Deployment::ReadProperty(const std::string_view &propName, Reader &reader)
{
    if (propName == "LoadoutName")
        reader >> m_LoadoutName;
    else if (propName == "Icon")
        reader >> m_Icon;
    else if (propName == "SpawnRadius")
        reader >> m_SpawnRadius;
    else if (propName == "WalkRadius")
        reader >> m_WalkRadius;
    else if (propName == "ID")
        reader >> m_ID;
	else if (propName == "HFlipped")
		reader >> m_HFlipped;
    else
        return SceneObject::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Deployment with a Writer for
//                  later recreation with Create(Reader &reader);

int Deployment::Save(Writer &writer) const
{
    SceneObject::Save(writer);

    writer.NewProperty("LoadoutName");
    writer << m_LoadoutName;
    writer.NewProperty("Icon");
    writer << m_Icon;
    writer.NewProperty("SpawnRadius");
    writer << m_SpawnRadius;
    writer.NewProperty("WalkRadius");
    writer << m_WalkRadius;
	writer.NewProperty("HFlipped");
	writer << m_HFlipped;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Deployment object.

void Deployment::Destroy(bool notInherited)
{
    

    if (!notInherited)
        SceneObject::Destroy();
    Clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CreateDeployedActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates and returns the Actor that this Deployment dictates should
//                  spawn here. Ownership IS transferred!! All items of the Loadout of
//                  this Deployment will be added to the Actor's inventory as well (and
//                  also owned by it)

Actor * Deployment::CreateDeployedActor()
{
	float cost;
	return CreateDeployedActor(-1, cost);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CreateDeployedActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates and returns the Actor that this Deployment dictates should
//                  spawn here. Ownership IS transferred!! All items of the Loadout of
//                  this Deployment will be added to the Actor's inventory as well (and
//                  also owned by it)

Actor * Deployment::CreateDeployedActor(int player, float &costTally)
{
    // The Actor instance we return and pass ownership of
    Actor *pReturnActor = 0;

    // Take metaplayer tech modifiers into account when calculating costs of this Deployment spawn
    int nativeModule = 0;
    float foreignCostMult = 1.0;
	float nativeCostMult = 1.0;
    MetaPlayer *pMetaPlayer = g_MetaMan.GetMetaPlayerOfInGamePlayer(player);
    // Put 
	if (g_MetaMan.GameInProgress() && pMetaPlayer)
    {
        nativeModule = pMetaPlayer->GetNativeTechModule();
        foreignCostMult = pMetaPlayer->GetForeignCostMultiplier();
        nativeCostMult = pMetaPlayer->GetNativeCostMultiplier();
        // Also set the team of this Deployable to match the player's
        m_Team = pMetaPlayer->GetTeam();
	}
	else
	{
		GameActivity * activity = dynamic_cast<GameActivity *>(g_ActivityMan.GetActivity());
		if (activity)
		{
			// Also set the team of this Deployable to match the player's
			//m_Team = activity->GetTeamOfPlayer(player);
			nativeModule = g_PresetMan.GetModuleID(activity->GetTeamTech(m_Team));
			// Select some random module if player selected all or something else
			if (nativeModule < 0) {
				std::vector<std::string> moduleList;

				for (int moduleID = 0; moduleID < g_PresetMan.GetTotalModuleCount(); ++moduleID) {
					if (const DataModule *dataModule = g_PresetMan.GetDataModule(moduleID)) {
						if (dataModule->IsFaction()) { moduleList.emplace_back(dataModule->GetFileName()); }
					}
				}
				int selection = RandomNum<int>(1, moduleList.size() - 1);
				nativeModule = g_PresetMan.GetModuleID(moduleList.at(selection));
			}
			foreignCostMult = 1.0;
			nativeCostMult = 1.0;
		}
    }

    // Find the Loadout that this Deployment is referring to
    const Loadout *pLoadout = dynamic_cast<const Loadout *>(g_PresetMan.GetEntityPreset("Loadout", m_LoadoutName, nativeModule));
    if (pLoadout)
    {
        // Create and pass along the first Actor and his inventory defined in the Loadout
        pReturnActor = pLoadout->CreateFirstActor(nativeModule, foreignCostMult, nativeCostMult, costTally);
        // Set the position and team etc for the Actor we are prepping to spawn
        if (pReturnActor)
        {
            pReturnActor->SetPos(m_Pos);
            pReturnActor->SetTeam(m_Team);
			pReturnActor->SetHFlipped(m_HFlipped);
            pReturnActor->SetControllerMode(Controller::CIM_AI);
            pReturnActor->SetAIMode(Actor::AIMODE_SENTRY);
			pReturnActor->SetDeploymentID(m_ID);
        }
    }

    // PASSING OWNERSHIP
    return pReturnActor;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CreateDeployedObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates and returns the first Device that Deployment dictates should
//                  spawn here. Ownership IS transferred!! Only the first Device is created.

SceneObject * Deployment::CreateDeployedObject()
{
	float cost;
	return CreateDeployedObject(-1, cost);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CreateDeployedObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates and returns the first Device that Deployment dictates should
//                  spawn here. Ownership IS transferred!! Only the first Device is created.

SceneObject * Deployment::CreateDeployedObject(int player, float &costTally)
{
    // The Actor instance we return and pass ownership of
    SceneObject *pReturnObject = 0;

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
        // Also set the team of this Deployable to match the player's
        m_Team = pMetaPlayer->GetTeam();
	} else {
		GameActivity * activity = dynamic_cast<GameActivity *>(g_ActivityMan.GetActivity());
		if (activity)
		{
			// Also set the team of this Deployable to match the player's
			//m_Team = activity->GetTeamOfPlayer(player);
			nativeModule = g_PresetMan.GetModuleID(activity->GetTeamTech(m_Team));
			// Select some random module if player selected all or something else
			if (nativeModule < 0) {
				std::vector<std::string> moduleList;

				for (int moduleID = 0; moduleID < g_PresetMan.GetTotalModuleCount(); ++moduleID) {
					if (const DataModule *dataModule = g_PresetMan.GetDataModule(moduleID)) {
						if (dataModule->IsFaction()) { moduleList.emplace_back(dataModule->GetFileName()); }
					}
				}
				int selection = RandomNum<int>(1, moduleList.size() - 1);
				nativeModule = g_PresetMan.GetModuleID(moduleList.at(selection));
			}
			foreignCostMult = 1.0;
			nativeCostMult = 1.0;
		}
    }


    // Find the Loadout that this Deployment is referring to
    const Loadout *pLoadout = dynamic_cast<const Loadout *>(g_PresetMan.GetEntityPreset("Loadout", m_LoadoutName, nativeModule));
    if (pLoadout)
    {
        // Get the first object in the Loadout
        pReturnObject = pLoadout->CreateFirstDevice(nativeModule, foreignCostMult, nativeCostMult, costTally);
        // Set the position and team etc for the Actor we are prepping to spawn
        pReturnObject->SetPos(m_Pos);
		pReturnObject->SetHFlipped(m_HFlipped);
        pReturnObject->SetTeam(m_Team);
    }

    // PASSING OWNERSHIP
    return pReturnObject;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DeploymentBlocked
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tests whether the Object this is supposed to spawn/deploy is blocked
//                  by an already exiting object in the a list being positioned within the
//                  spawn radius of this.

bool Deployment::DeploymentBlocked(int player, const list<SceneObject *> &existingObjects)
{
    bool blocked = false;

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

	// First try to find an object via ID's, objects with ID can be far enough
    // Go through all already-placed things in the Scene to see if there's anything with matching spawn ID
	if (m_ID)
	{
		for (list<SceneObject *>::const_iterator existingItr = existingObjects.begin(); existingItr != existingObjects.end(); ++existingItr)
		{
			Actor *pActor = dynamic_cast<Actor *>(*existingItr);
			if (pActor && pActor->GetDeploymentID() == m_ID)
			{
				// Do ghetto distance calc between the thing we want to place and the similar thing we found already placed
				// Note this doesn't take into account Scene wrapping, which is problematic when the Scene might not be loaded.. it's okay in this case though
				Vector distance = (*existingItr)->GetPos() - m_Pos;
				// If the same thing is within the spawn radius, then signal that this Deployment location is indeed BLOCKED
				if (distance.GetMagnitude() < m_WalkRadius)
				{
					blocked = true;
					break;
				}
			}
		}
	}

	// No need to do expensive search if it's already blocked
	if (blocked)
		return blocked;

    // The object we would spawn by this Deployment, IF it's not blocked by somehting already
    const SceneObject *pSpawnObject = 0;

    // Find the Loadout that this Deployment is referring to
    Loadout *pLoadout = dynamic_cast<Loadout *>(g_PresetMan.GetEntityPreset("Loadout", m_LoadoutName, nativeModule)->Clone());
    if (pLoadout)
    {
        // Now go through the Loadout list of items and tally the cost of all devices that would go into inventory of the first Actor found in the list
        const list<const SceneObject *> *pMOList = pLoadout->GetCargoList();
        if (pMOList && !pMOList->empty())
        {
            // Go through the list of things ordered, and give any actors all the items that is present after them,
            // until the next actor. Also, the first actor gets all stuff in the list above him.
            const MovableObject *pInventoryObject = 0;
            const Actor *pActor = 0;
            for (list<const SceneObject *>::const_iterator itr = pMOList->begin(); itr != pMOList->end(); ++itr)
            {
                // Save pointer of the preset in the list
                pInventoryObject = dynamic_cast<const MovableObject *>(*itr);
                // See if it's actually a passenger, as opposed to a regular item
                pActor = dynamic_cast<const Actor *>(pInventoryObject);
                // If it's an actor, then that's the guy which would be spawned, so use him to check agianst blockage
                if (pActor)
                {
                    pSpawnObject = pActor;
                    // We're done looking for the object that would be spawned
                    break;
                }
                // If not an Actor, then skip because we're still looking for one
            }
            pActor = 0;

            // If no Actor was found, then see if we can use the delivery Craft instead
            if (!pSpawnObject && pLoadout->GetDeliveryCraft())
            {
                // The craft is now the Actor we are looking at spawning
                pSpawnObject = pLoadout->GetDeliveryCraft();
            }

            // If there's no Actor in this Deployment's Loadout at all, then we should just count the first Item or device in the Loadout
            if (!pSpawnObject)
            {
                // Find the first non-actor
                for (list<const SceneObject *>::const_iterator itr = pMOList->begin(); itr != pMOList->end(); ++itr)
                {
                    // If not an Actor, then we should count it and then stop
                    if (!dynamic_cast<const Actor *>(*itr))
                    {
                        pSpawnObject = *itr;
                        // We're done finding the spawning object
                        break;
                    }
                }
            }
        }

        // Delete the Loadout instance we have
        delete pLoadout;
        pLoadout = 0;
    }

    // Now check for whether the object that is going to be spawned is blocked by anyhting sufficently similar positioned within the spawn radius
    if (pSpawnObject)
    {
        // Go through all already-placed things in the Scene to see if there's anything similar/same
        for (list<SceneObject *>::const_iterator existingItr = existingObjects.begin(); existingItr != existingObjects.end(); ++existingItr)
        {
            if (((*existingItr)->GetClassName() == pSpawnObject->GetClassName()) && ((*existingItr)->GetPresetName() == pSpawnObject->GetPresetName()))
            {
                // Do ghetto distance calc between the thing we want to place and the similar thing we found already placed
                // Note this doesn't take into account Scene wrapping, which is problematic when the Scene might not be loaded.. it's okay in this case though
                Vector distance = (*existingItr)->GetPos() - m_Pos;
                // If the same thing is within the spawn radius, then signal that this Deployment location is indeed BLOCKED
                if (distance.GetMagnitude() < m_SpawnRadius)
                {
                    blocked = true;
                    break;
                }
            }
        }
    }

    return blocked;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total liquidation value of a spawn of this, including
//                  everything carried by it.

float Deployment::GetTotalValue(int nativeModule, float foreignMult, float nativeMult) const
{
    float totalValue = 0;
    const Actor *pFirstActor = 0;

    // Find the Loadout that this Deployment is referring to
    Loadout *pLoadout = dynamic_cast<Loadout *>(g_PresetMan.GetEntityPreset("Loadout", m_LoadoutName, nativeModule)->Clone());
    if (pLoadout)
    {
        // Now go through the Loadout list of items and tally the cost ofall devices that would go into inventory of the first Actor found in the list
        const list<const SceneObject *> *pMOList = pLoadout->GetCargoList();
        if (pMOList && !pMOList->empty())
        {
            // Go through the list of things ordered, and give any actors all the items that is present after them,
            // until the next actor. Also, the first actor gets all stuff in the list above him.
            const MovableObject *pInventoryObject = 0;
            const Actor *pActor = 0;
            list<const MovableObject *> cargoItems;
            for (list<const SceneObject *>::const_iterator itr = pMOList->begin(); itr != pMOList->end(); ++itr)
            {
                // Save pointer of the preset in the list
                pInventoryObject = dynamic_cast<const MovableObject *>(*itr);
                // See if it's actually a passenger, as opposed to a regular item
                pActor = dynamic_cast<const Actor *>(pInventoryObject);
                // If it's an actor, then that's the guy which will be spawned
                if (pActor)
                {
	                // Add to the total cost tally
	                totalValue += (*itr)->GetGoldValue(nativeModule, foreignMult, nativeMult);
                    // If this is the first passenger, then give him all the shit found in the list before him
                    if (!pFirstActor)
                    {
                        for (list<const MovableObject *>::iterator iItr = cargoItems.begin(); iItr != cargoItems.end(); ++iItr)
                            totalValue += (*iItr)->GetTotalValue(nativeModule, foreignMult);
                    }
                    // This isn't the first passenger, so give the previous guy all the stuff that was found since processing him
                    else
                    {
                        for (list<const MovableObject *>::iterator iItr = cargoItems.begin(); iItr != cargoItems.end(); ++iItr)
                            totalValue += (*iItr)->GetTotalValue(nativeModule, foreignMult);

                        // Also stop going through the list; we only need to count the value of ONE actor and his stuff
                        break;
                    }
                    // Clear out the temporary cargo list since we've assigned all the stuff in it to the return Actor
                    cargoItems.clear();

                    // Now set the current Actor as the one we found first
                    pFirstActor = pActor;
                }
                // If not an Actor, then add it to the temp list of items which will be added to the last passenger's inventory
                else
                    cargoItems.push_back(pInventoryObject);
            }
            pActor = 0;

            // If there was a last passenger and only things after him, count the value of all the items in his inventory
            if (pFirstActor)
            {
                // Passing ownership
                for (list<const MovableObject *>::iterator iItr = cargoItems.begin(); iItr != cargoItems.end(); ++iItr)
                    totalValue += (*iItr)->GetTotalValue(nativeModule, foreignMult);
                cargoItems.clear();
            }
            // If there wa NO actor in the Loadout's cargo list, then see if there's a craft we can stuff thigns into instead
            else if (pLoadout->GetDeliveryCraft())
            {
                // The craft is now the Actor we are counting, so make an instance
                pFirstActor = pLoadout->GetDeliveryCraft();
                // Add the cost of the ship
                totalValue += pFirstActor->GetGoldValue(nativeModule, foreignMult);
                // Count the stuff it would be filled with, passing ownership
                for (list<const MovableObject *>::iterator iItr = cargoItems.begin(); iItr != cargoItems.end(); ++iItr)
                    totalValue += (*iItr)->GetTotalValue(nativeModule, foreignMult);
                cargoItems.clear();
            }

            // If there's no Actor in this Deployment's Loadout, then we should just count the first Item or device in the Loadout
            if (!pFirstActor)
            {
                // Start over the count; we might have only had items/devices in the Loadout list, but no Actors yet
                totalValue = 0;
                for (list<const SceneObject *>::const_iterator itr = pMOList->begin(); itr != pMOList->end(); ++itr)
                {
                    // If not an Actor, then we should count it and then stop
                    if (!dynamic_cast<const Actor *>(*itr))
                    {
                        // Add to the total cost tally
                        totalValue += (*itr)->GetGoldValue(nativeModule, foreignMult);
                        // We're done
                        break;
                    }
                }
            }
        }

        // Delete the Loadout instance we have
        delete pLoadout;
        pLoadout = 0;
    }

    return totalValue;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsOnScenePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this' current graphical representation overlaps
//                  a point in absolute scene coordinates.

bool Deployment::IsOnScenePoint(Vector &scenePoint) const
{
    if (m_Icon.GetBitmaps8().empty() || !(m_Icon.GetBitmaps8().at(0)))
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
    BITMAP *pBitmap = m_Icon.GetBitmaps8().at(0);
    Vector bitmapPos = m_Pos - Vector(pBitmap->w / 2, pBitmap->h / 2);
    if (WithinBox(scenePoint, bitmapPos, pBitmap->w, pBitmap->h))
    {
        // Scene point on the bitmap
        Vector bitmapPoint = scenePoint - bitmapPos;
        if (getpixel(pBitmap, bitmapPoint.m_X, bitmapPoint.m_Y) != g_MaskColor)
           return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Deployment's current graphical representation to a
//                  BITMAP of choice.

void Deployment::Draw(BITMAP *pTargetBitmap, const Vector &targetPos, DrawMode mode, bool onlyPhysical) const
{
    if (m_Icon.GetBitmaps8().empty() || !(m_Icon.GetBitmaps8().at(0)))
        RTEAbort("Deployment's Icon bitmaps are null when drawing!");

	if (m_apArrowLeftBitmap.empty() || m_apArrowRightBitmap.empty())
		RTEAbort("Deployment's Arrow bitmaps are null when drawing!");

	{
		BITMAP *pBitmap = m_Icon.GetBitmaps8().at(0);

		// Take care of wrapping situations
		Vector aDrawPos[4];
		aDrawPos[0] = m_Pos - Vector(pBitmap->w / 2, pBitmap->h / 2) - targetPos;
		int passes = 1;

		// See if need to double draw this across the scene seam if we're being drawn onto a scenewide bitmap
		if (targetPos.IsZero())
		{
			if (aDrawPos[0].m_X < pBitmap->w)
			{
				aDrawPos[passes] = aDrawPos[0];
				aDrawPos[passes].m_X += pTargetBitmap->w;
				passes++;
			}
			else if (aDrawPos[0].m_X > pTargetBitmap->w - pBitmap->w)
			{
				aDrawPos[passes] = aDrawPos[0];
				aDrawPos[passes].m_X -= pTargetBitmap->w;
				passes++;
			}
		}
		// Only screenwide target bitmap, so double draw within the screen if the screen is straddling a scene seam
		else
		{
			if (g_SceneMan.SceneWrapsX())
			{
				int sceneWidth = g_SceneMan.GetSceneWidth();
				if (targetPos.m_X < 0)
				{
					aDrawPos[passes] = aDrawPos[0];
					aDrawPos[passes].m_X -= sceneWidth;
					passes++;
				}
				if (targetPos.m_X + pTargetBitmap->w > sceneWidth)
				{
					aDrawPos[passes] = aDrawPos[0];
					aDrawPos[passes].m_X += sceneWidth;
					passes++;
				}
			}
		}

		// Draw all the passes needed
		for (int i = 0; i < passes; ++i)
		{
			if (mode == g_DrawColor)
			{
				masked_blit(pBitmap, pTargetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), pBitmap->w, pBitmap->h);
				// Draw the spawn radius circle too
				circle(pTargetBitmap, aDrawPos[i].GetFloorIntX() + (pBitmap->w / 2), aDrawPos[i].GetFloorIntY() + (pBitmap->h / 2), m_SpawnRadius, c_GUIColorGray);
			}
			else if (mode == g_DrawLess)
			{
				masked_blit(pBitmap, pTargetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), pBitmap->w, pBitmap->h);
			}
			else if (mode == g_DrawTrans)
			{
				draw_trans_sprite(pTargetBitmap, pBitmap, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY());
				// Draw the spawn radius circle too
				circle(pTargetBitmap, aDrawPos[i].GetFloorIntX() + (pBitmap->w / 2), aDrawPos[i].GetFloorIntY() + (pBitmap->h / 2), m_SpawnRadius, c_GUIColorGray);
			}
		}
	}

	{
		// Draw direction arrow
		BITMAP * pBitmap = 0;
		Vector offset;
		if (m_HFlipped)
		{
			pBitmap = m_apArrowLeftBitmap[0];
			offset = Vector(-14, 0);
		}
		else {
			pBitmap = m_apArrowRightBitmap[0];
			offset = Vector(14, 0);
		}

		// Take care of wrapping situations
		Vector aDrawPos[4];
		aDrawPos[0] = m_Pos - Vector(pBitmap->w / 2, pBitmap->h / 2) - targetPos + offset;
		int passes = 1;

		// See if need to double draw this across the scene seam if we're being drawn onto a scenewide bitmap
		if (targetPos.IsZero())
		{
			if (aDrawPos[0].m_X < pBitmap->w)
			{
				aDrawPos[passes] = aDrawPos[0];
				aDrawPos[passes].m_X += pTargetBitmap->w;
				passes++;
			}
			else if (aDrawPos[0].m_X > pTargetBitmap->w - pBitmap->w)
			{
				aDrawPos[passes] = aDrawPos[0];
				aDrawPos[passes].m_X -= pTargetBitmap->w;
				passes++;
			}
		}
		// Only screenwide target bitmap, so double draw within the screen if the screen is straddling a scene seam
		else
		{
			if (g_SceneMan.SceneWrapsX())
			{
				int sceneWidth = g_SceneMan.GetSceneWidth();
				if (targetPos.m_X < 0)
				{
					aDrawPos[passes] = aDrawPos[0];
					aDrawPos[passes].m_X -= sceneWidth;
					passes++;
				}
				if (targetPos.m_X + pTargetBitmap->w > sceneWidth)
				{
					aDrawPos[passes] = aDrawPos[0];
					aDrawPos[passes].m_X += sceneWidth;
					passes++;
				}
			}
		}

		// Draw all the passes needed
		for (int i = 0; i < passes; ++i)
		{
			if (mode == g_DrawColor)
			{
				masked_blit(pBitmap, pTargetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), pBitmap->w, pBitmap->h);
			}
			else if (mode == g_DrawLess)
			{
				masked_blit(pBitmap, pTargetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), pBitmap->w, pBitmap->h);
			}
			else if (mode == g_DrawTrans)
			{
				draw_trans_sprite(pTargetBitmap, pBitmap, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY());
			}
		}
	}
}

} // namespace RTE
