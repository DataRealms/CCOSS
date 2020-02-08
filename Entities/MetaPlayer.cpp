//////////////////////////////////////////////////////////////////////////////////////////
// File:            MetaPlayer.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the MetaPlayer class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "MetaPlayer.h"
#include "ActivityMan.h"
#include "PresetMan.h"
#include "DataModule.h"

namespace RTE {

CONCRETECLASSINFO(MetaPlayer, Entity, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MetaPlayer, effectively
//                  resetting the members of this abstraction level only.

void MetaPlayer::Clear()
{
    m_Name = "";
    m_Team = Activity::NOTEAM;
    m_Human = true;
    m_GameOverRound = -1;
    m_InGamePlayer = Activity::PLAYER_1;
    m_Aggressiveness = 0.5;
    // Everything is natively priced
    m_NativeTechModule = 0;
    m_NativeCostMult = 1.0;
    m_ForeignCostMult = 4.0;
    m_BrainPool = 0;
    m_BrainsInTransit = 0;
    m_Funds = 0;
    m_PhaseStartFunds = 0;
    m_OffensiveBudget = 0;
    m_OffensiveTarget = "";
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MetaPlayer object ready for use.

int MetaPlayer::Create()
{
    if (Entity::Create() < 0)
        return -1;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a MetaPlayer to be identical to another, by deep copy.

int MetaPlayer::Create(const MetaPlayer &reference)
{
    Entity::Create(reference);

    m_Name = reference.m_Name;
    m_Team = reference.m_Team;
    m_Human = reference.m_Human;
    m_GameOverRound = reference.m_GameOverRound;
    m_InGamePlayer = reference.m_InGamePlayer;
    m_Aggressiveness = reference.m_Aggressiveness;
    m_NativeTechModule = reference.m_NativeTechModule;
    m_NativeCostMult = reference.m_NativeCostMult;
    m_ForeignCostMult = reference.m_ForeignCostMult;
    m_BrainPool = reference.m_BrainPool;
    m_BrainsInTransit = reference.m_BrainsInTransit;
    m_Funds = reference.m_Funds;
    m_PhaseStartFunds = reference.m_PhaseStartFunds;
    m_OffensiveBudget = reference.m_OffensiveBudget;
    m_OffensiveTarget = reference.m_OffensiveTarget;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int MetaPlayer::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "Name")
        reader >> m_Name;
    else if (propName == "Team")
        reader >> m_Team;
    else if (propName == "Human")
        reader >> m_Human;
    else if (propName == "GameOverRound")
        reader >> m_GameOverRound;
    else if (propName == "InGamePlayer")
        reader >> m_InGamePlayer;
    else if (propName == "Aggressiveness")
        reader >> m_Aggressiveness;
    // Need to match the name to the index
    else if (propName == "NativeTechModule")
    {
        string moduleName;
        reader >> moduleName;
        m_NativeTechModule = g_PresetMan.GetModuleID(moduleName);
        // Default to no native tech if the one we're looking for coudln't be found
        if (m_NativeTechModule < 0)
            m_NativeTechModule = 0;
    }
    else if (propName == "NativeCostMultiplier")
        reader >> m_NativeCostMult;
    else if (propName == "ForeignCostMultiplier")
        reader >> m_ForeignCostMult;
    else if (propName == "BrainPool")
        reader >> m_BrainPool;
    else if (propName == "Funds")
        reader >> m_Funds;
    else if (propName == "OffensiveBudget")
        reader >> m_OffensiveBudget;
    else if (propName == "OffensiveTarget")
        reader >> m_OffensiveTarget;
    else
        // See if the base class(es) can find a match instead
        return Entity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this MetaPlayer with a Writer for
//                  later recreation with Create(Reader &reader);

int MetaPlayer::Save(Writer &writer) const
{
    Entity::Save(writer);

    writer.NewProperty("Name");
    writer << m_Name;
    writer.NewProperty("Team");
    writer << m_Team;
    writer.NewProperty("Human");
    writer << m_Human;
    writer.NewProperty("GameOverRound");
    writer << m_GameOverRound;
    writer.NewProperty("InGamePlayer");
    writer << m_InGamePlayer;
    writer.NewProperty("Aggressiveness");
    writer << m_Aggressiveness;
    // Need to write out the name, and not just the index of the module.. it might change
    writer.NewProperty("NativeTechModule");
    writer << g_PresetMan.GetDataModule(m_NativeTechModule)->GetFileName();
    writer.NewProperty("NativeCostMultiplier");
    writer << m_NativeCostMult;
    writer.NewProperty("ForeignCostMultiplier");
    writer << m_ForeignCostMult;
    writer.NewProperty("BrainPool");
    writer << m_BrainPool;
    writer.NewProperty("Funds");
    writer << m_Funds;
    writer.NewProperty("OffensiveBudget");
    writer << m_OffensiveBudget;
    writer.NewProperty("OffensiveTarget");
    writer << (m_OffensiveTarget.empty() ? "None" : m_OffensiveTarget);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the MetaPlayer object.

void MetaPlayer::Destroy(bool notInherited)
{
    

    if (!notInherited)
        Entity::Destroy();
    Clear();
}

} // namespace RTE