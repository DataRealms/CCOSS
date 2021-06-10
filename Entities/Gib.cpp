#include "Gib.h"
#include "PresetMan.h"
#include "MovableObject.h"

namespace RTE {

	const std::string Gib::c_ClassName = "Gib";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Gib::Clear() {
		m_GibParticle = 0;
		m_Offset.Reset();
		m_Count = 1;
		m_Spread = 0.1F;
		m_MinVelocity = 0;
		m_MaxVelocity = 0;
		m_LifeVariation = 0.1F;
		m_InheritsVel = true;
		m_IgnoresTeamHits = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Gib::Create(const Gib &reference) {
		m_GibParticle = reference.m_GibParticle;
		m_Offset = reference.m_Offset;
		m_Count = reference.m_Count;
		m_Spread = reference.m_Spread;
		m_MinVelocity = reference.m_MinVelocity;
		m_MaxVelocity = reference.m_MaxVelocity;
		m_LifeVariation = reference.m_LifeVariation;
		m_InheritsVel = reference.m_InheritsVel;
		m_IgnoresTeamHits = reference.m_IgnoresTeamHits;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Gib::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "GibParticle") {
			m_GibParticle = dynamic_cast<const MovableObject *>(g_PresetMan.GetEntityPreset(reader));
			RTEAssert(m_GibParticle, "Stream suggests allocating an unallocable type in Gib::Create!");
		} else if (propName == "Offset") {
			reader >> m_Offset;
		} else if (propName == "Count") {
			reader >> m_Count;
		} else if (propName == "Spread") {
			reader >> m_Spread;
		} else if (propName == "MinVelocity") {
			reader >> m_MinVelocity;
		} else if (propName == "MaxVelocity") {
			reader >> m_MaxVelocity;
		} else if (propName == "LifeVariation") {
			reader >> m_LifeVariation;
		} else if (propName == "InheritsVel") {
			reader >> m_InheritsVel;
		} else if (propName == "IgnoresTeamHits") {
			reader >> m_IgnoresTeamHits;
		} else {
			return Serializable::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Gib::Save(Writer &writer) const {
		Serializable::Save(writer);

		writer.NewProperty("GibParticle");
		// All of this is needed to make a preset look like not original and save as CopyOf instead of separate preset.
		Entity *gibEntity = m_GibParticle->Clone();
		gibEntity->ResetOriginalPresetFlag();
		writer << gibEntity;
		delete gibEntity;
		gibEntity = 0;

		writer.NewProperty("Offset");
		writer << m_Offset;

		// These are disabled because we're writing CopyOfs. Might need these in the future if we need to write original presets.
		/*
		writer.NewProperty("Count");
		writer << m_Count;
		writer.NewProperty("Spread");
		writer << m_Spread;
		writer.NewProperty("MinVelocity");
		writer << m_MinVelocity;
		writer.NewProperty("MaxVelocity");
		writer << m_MaxVelocity;
		writer.NewProperty("LifeVariation");
		writer << m_LifeVariation;
		writer.NewProperty("InheritsVel");
		writer << m_InheritsVel;
		*/

		return 0;
	}
}