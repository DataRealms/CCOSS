//////////////////////////////////////////////////////////////////////////////////////////
// File:            Loadout.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation file for the Loadout class
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Loadout.h"
#include "PresetMan.h"
#include "MovableObject.h"
#include "ACraft.h"

namespace RTE {

ConcreteClassInfo(Loadout, Entity, 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Loadout, effectively
//                  resetting the members of this abstraction level only.

void Loadout::Clear()
{
    m_Complete = true;
    m_pDeliveryCraft = 0;
    m_CargoItems.clear();
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Loadout object ready for use.

int Loadout::Create()
{
    if (Entity::Create() < 0)
        return -1;

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Loadout to be identical to another, by deep copy.

int Loadout::Create(const Loadout &reference)
{
    Entity::Create(reference);

    m_Complete = reference.m_Complete;
    // These are preset instances, not owned by the reference or this.
    m_pDeliveryCraft = reference.m_pDeliveryCraft;
    for (list<const SceneObject *>::const_iterator itr = reference.m_CargoItems.begin(); itr != reference.m_CargoItems.end(); ++itr)
        m_CargoItems.push_back(*itr);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Loadout::ReadProperty(const std::string_view &propName, Reader &reader)
{
    // Need to load all this stuff without the assumption that it all is available. Mods might have changed etc so things might still not be around, and that's ok.
    if (propName == "DeliveryCraft")
    {
        string className, presetName;
        // Load class name and then preset instance name
        reader >> className;
        // Ignore the property name, just interested in the value
        if (reader.ReadPropName() != "PresetName")
            reader.ReportError("Expected property \"PresetName\" not found when reading BuyMenu Loadout settings!");
        // Read the preset's name and try to find it
        presetName = reader.ReadPropValue();
        // It's OK if we can't find it.. just means we aren't a complete loadout 
        m_pDeliveryCraft = dynamic_cast<const ACraft *>(g_PresetMan.GetEntityPreset(className, presetName, -1));
        if (!m_pDeliveryCraft)
            m_Complete = false;
        // Artificially end reading this property since we got all we needed
        reader.NextProperty();
    }
    else if (propName == "AddCargoItem")
    {
        string className, presetName;
        // Load class name and then preset instance name
        reader >> className;
        // Ignore the property name, just interested in the value
        if (reader.ReadPropName() != "PresetName")
            reader.ReportError("Expected property \"PresetName\" not found when reading BuyMenu Loadout settings!");
        // Read the preset's name and try to find it
        presetName = reader.ReadPropValue();
        // It's OK if we can't find it.. just means we aren't a complete loadout 
        const MovableObject *pCargo = dynamic_cast<const MovableObject *>(g_PresetMan.GetEntityPreset(className, presetName, -1));
        if (!pCargo)
            m_Complete = false;
        // Add the found cargo item to the list
        else
            m_CargoItems.push_back(pCargo);

        // Artificially end reading this property since we got all we needed
        reader.NextProperty();

        pCargo = 0;
    }
    else
        return Entity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Loadout with a Writer for
//                  later recreation with Create(Reader &reader);

int Loadout::Save(Writer &writer) const
{
    Entity::Save(writer);

    if (m_pDeliveryCraft)
    {
        writer.NewProperty("DeliveryCraft");
        writer.ObjectStart(m_pDeliveryCraft->GetClassName());
        writer.NewProperty("PresetName");
        writer << m_pDeliveryCraft->GetModuleAndPresetName();
        writer.ObjectEnd();
    }
    for (list<const SceneObject *>::const_iterator itr = m_CargoItems.begin(); itr != m_CargoItems.end(); ++itr)
    {
        writer.NewProperty("AddCargoItem");
        writer.ObjectStart((*itr)->GetClassName());
        writer.NewProperty("PresetName");
        writer << (*itr)->GetModuleAndPresetName();
        writer.ObjectEnd();
    }

    return 0;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Icon object.

void Loadout::Destroy(bool notInherited)
{
//  delete;    

    if (!notInherited)
        Entity::Destroy();
    Clear();
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CreateFirstActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates and returns the first Actor that this Loadout has and equips.
//                  Ownership IS transferred!! All items of the Loadout of this Deployment
//                  will be added to the Actor's inventory as well (and also owned by it)

Actor * Loadout::CreateFirstActor(int nativeModule, float foreignMult, float nativeMult, float &costTally) const
{
    // The Actor instance we return and pass ownership of
    Actor *pReturnActor = 0;

    // Now go through the cargo list of items and load new instances of all
    // Devices into inventory of an instance of the first Actor found in the list
    if (!m_CargoItems.empty())
    {
        // Go through the list of things ordered, and give any actors all the items that is present after them,
        // until the next actor. Also, the first actor gets all stuff in the list above him.
        MovableObject *pInventoryObject = 0;
        Actor *pActor = 0;
        list<MovableObject *> tempCargo;
        for (list<const SceneObject *>::const_iterator itr = m_CargoItems.begin(); itr != m_CargoItems.end(); ++itr)
        {
            // Add to the total cost tally
            costTally += (*itr)->GetGoldValue(nativeModule, foreignMult, nativeMult);
            // Make copy of the preset instance in the list
            pInventoryObject = dynamic_cast<MovableObject *>((*itr)->Clone());
            // See if it's actually a passenger, as opposed to a regular item
            pActor = dynamic_cast<Actor *>(pInventoryObject);
            // If it's an actor, then set its team and add it to the Craft's inventory!
            if (pActor)
            {
                // If this is the first passenger, then give him all the shit found in the list before him
                if (!pReturnActor)
                {
                    for (list<MovableObject *>::iterator iItr = tempCargo.begin(); iItr != tempCargo.end(); ++iItr)
                        pActor->AddInventoryItem(*iItr);
                }
                // This isn't the first passenger, so give the previous guy all the stuff that was found since processing him
                else
                {
                    // Go through the temporary list and give the previous, real first actor all the stuff
                    for (list<MovableObject *>::iterator iItr = tempCargo.begin(); iItr != tempCargo.end(); ++iItr)
                        pReturnActor->AddInventoryItem(*iItr);
                    // Clear out the temporary cargo list since we've assigned all the stuff in it to the return Actor
                    tempCargo.clear();

                    // REMOVE the value of this bookend actor from the total cost tally - he is not included!
                    costTally -= pActor->GetGoldValue(nativeModule, foreignMult, nativeMult);
                    // Also stop going through the list; we only need ONE Actor.. so destroy the instance we jsut created of a second one.
                    delete pActor;
                    pActor = 0;
                    // STOP!
                    break;
                }
                // Clear out the temporary cargo list since we've assigned all the stuff in it to the return Actor
                tempCargo.clear();

                // Now set the current Actor as the one we return; he'll eventually get everything found after him as well
                pReturnActor = pActor;
                // Set the position and team etc for the Actor we are prepping to spawn
                pReturnActor->SetControllerMode(Controller::CIM_AI);
                pReturnActor->SetAIMode(Actor::AIMODE_SENTRY);
            }
            // If not an Actor, then add it to the temp list of items which will be added to the last passenger's inventory
            else
                tempCargo.push_back(pInventoryObject);
        }
        pActor = 0;

        // If there was a last passenger and only things after him, stuff all the items into his inventory
        if (pReturnActor)
        {
            // Passing ownership
            for (list<MovableObject *>::iterator iItr = tempCargo.begin(); iItr != tempCargo.end(); ++iItr)
                pReturnActor->AddInventoryItem(*iItr);
            tempCargo.clear();
        }
        // If there wa NO actor in the Loadout's cargo list, then see if there's a craft we can stuff thigns into instead
        else if (m_pDeliveryCraft)
        {
            // The craft is now the Actor we are spawning, so make an instance
            pReturnActor = dynamic_cast<Actor *>(m_pDeliveryCraft->Clone());
            // Add the cost of the ship
            costTally += pReturnActor->GetGoldValue(nativeModule, foreignMult, nativeMult);
            // Fill it with the stuff, passing ownership
            for (list<MovableObject *>::iterator iItr = tempCargo.begin(); iItr != tempCargo.end(); ++iItr)
                pReturnActor->AddInventoryItem(*iItr);
            tempCargo.clear();
        }
    }

    // PASSING OWNERSHIP
    return pReturnActor;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CreateFirstDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates and returns the first Device that is defined in this Loadout.
//                  Ownership IS transferred!! Only the first Device is created.

SceneObject * Loadout::CreateFirstDevice(int nativeModule, float foreignMult, float nativeMult, float &costTally) const
{
    // The Actor instance we return and pass ownership of
    SceneObject *pReturnObject = 0;

    // Now go through the Loadout list of items and load new instances of all
    // Devices into inventory of an instance of the first Actor found in the list
    if (!m_CargoItems.empty())
    {
        for (list<const SceneObject *>::const_iterator itr = m_CargoItems.begin(); itr != m_CargoItems.end(); ++itr)
        {
            // If not an Actor, then we should create and return it.
            if (!dynamic_cast<const Actor *>(*itr))
            {
                pReturnObject = dynamic_cast<SceneObject *>((*itr)->Clone());
                // Add to the total cost tally
                costTally += pReturnObject->GetGoldValue(nativeModule, foreignMult, nativeMult);
                // We're done
                break;
            }
        }
    }

    // PASSING OWNERSHIP
    return pReturnObject;
}

} // namespace RTE