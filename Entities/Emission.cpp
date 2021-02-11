//////////////////////////////////////////////////////////////////////////////////////////
// File:            Emission.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Emission class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


#include "Emission.h"
#include "PresetMan.h"

namespace RTE {


//const string Emission::m_sClassName = "Emission";

ConcreteClassInfo(Emission, Entity, 100)

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Emission, effectively
//                  resetting the members of this abstraction level only.

void Emission::Clear()
{
	m_pEmission = 0;
	m_PPM = 0;
	m_BurstSize = 0;
	m_Accumulator = 0;
	m_Spread = 0;
	m_MinVelocity = 0;
	m_MaxVelocity = 0;
	m_LifeVariation = 0.1;
	m_PushesEmitter = true;
	m_InheritsVel = 0;
	m_StartTimer.SetSimTimeLimitMS(0);
	m_StartTimer.Reset();
	m_StopTimer.SetSimTimeLimitMS(1000000);
	m_StopTimer.Reset();
	m_Offset.Reset();
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Emission object ready for use.

int AEmitter::Emission::Create()
{
if (Serializable::Create() < 0)
return -1;

return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Emission to be identical to another, by deep copy.

int Emission::Create(const Emission &reference)
{
	m_pEmission = reference.m_pEmission;
	m_PPM = reference.m_PPM;
	m_BurstSize = reference.m_BurstSize;
	m_Accumulator = reference.m_Accumulator;
	m_Spread = reference.m_Spread;
	m_MinVelocity = reference.m_MinVelocity;
	m_MaxVelocity = reference.m_MaxVelocity;
	m_LifeVariation = reference.m_LifeVariation;
	m_PushesEmitter = reference.m_PushesEmitter;
	m_InheritsVel = reference.m_InheritsVel;
	m_StartTimer = reference.m_StartTimer;
	m_StopTimer = reference.m_StopTimer;
	m_Offset = reference.m_Offset;

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Emission::ReadProperty(const std::string_view &propName, Reader &reader)
{
	if (propName == "EmittedParticle")
	{
		m_pEmission = dynamic_cast<const MovableObject *>(g_PresetMan.GetEntityPreset(reader));
		RTEAssert(m_pEmission, "Stream suggests allocating an unallocatable type in AEmitter::Emission::Create!");
	}
	else if (propName == "ParticlesPerMinute")
		reader >> m_PPM;
	else if (propName == "BurstSize")
		reader >> m_BurstSize;
	else if (propName == "Spread")
		reader >> m_Spread;
	else if (propName == "MinVelocity")
		reader >> m_MinVelocity;
	else if (propName == "MaxVelocity")
		reader >> m_MaxVelocity;
	else if (propName == "LifeVariation")
		reader >> m_LifeVariation;
	else if (propName == "PushesEmitter")
		reader >> m_PushesEmitter;
	else if (propName == "Offset")
		reader >> m_Offset;
	else if (propName == "InheritsVel")
	{
		reader >> m_InheritsVel;
		Clamp(m_InheritsVel, 1, 0);
	}
	else if (propName == "StartTimeMS")
	{
		int startTime;
		reader >> startTime;
		m_StartTimer.SetSimTimeLimitMS(startTime);
	}
	else if (propName == "StopTimeMS")
	{
		int stopTime;
		reader >> stopTime;
		m_StopTimer.SetSimTimeLimitMS(stopTime);
	}
	else
		return Serializable::ReadProperty(propName, reader);

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Emission with a Writer for
//                  later recreation with Create(Reader &reader);

int Emission::Save(Writer &writer) const
{
	Serializable::Save(writer);

	writer.NewProperty("EmittedParticle");
	writer << m_pEmission;
	writer.NewProperty("ParticlesPerMinute");
	writer << m_PPM;
	writer.NewProperty("BurstSize");
	writer << m_BurstSize;
	writer.NewProperty("Spread");
	writer << m_Spread;
	writer.NewProperty("MaxVelocity");
	writer << m_MinVelocity;
	writer.NewProperty("MinVelocity");
	writer << m_MaxVelocity;
	writer.NewProperty("LifeVariation");
	writer << m_LifeVariation;
	writer.NewProperty("PushesEmitter");
	writer << m_PushesEmitter;
	writer.NewProperty("InheritsVel");
	writer << m_InheritsVel;
	writer.NewProperty("Offset");
	writer << m_Offset;
	writer.NewProperty("StartTimeMS");
	writer << m_StartTimer.GetSimTimeLimitMS();
	writer.NewProperty("StopTimeMS");
	writer << m_StopTimer.GetSimTimeLimitMS();

	return 0;
}

}