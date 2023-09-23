#include "Gib.h"
#include "PresetMan.h"
#include "MovableObject.h"

namespace RTE {

	const std::string Gib::c_ClassName = "Gib";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Gib::Clear() {
		m_GibParticle = nullptr;
		m_Offset.Reset();
		m_Count = 1;
		m_Spread = 0.1F;
		m_MinVelocity = 0;
		m_MaxVelocity = 0;
		m_LifeVariation = 0.1F;
		m_InheritsVel = 1.0F;
		m_IgnoresTeamHits = false;
		m_SpreadMode = SpreadMode::SpreadRandom;
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
		m_SpreadMode = reference.m_SpreadMode;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Gib::ReadProperty(const std::string_view &propName, Reader &reader) {
		StartPropertyList(return Serializable::ReadProperty(propName, reader));
		
		MatchProperty("GibParticle", {
			m_GibParticle = dynamic_cast<const MovableObject *>(g_PresetMan.GetEntityPreset(reader));
			RTEAssert(m_GibParticle, "Stream suggests allocating an unallocable type in Gib::Create!");
		});
		MatchProperty("Offset", { reader >> m_Offset; });
		MatchProperty("Count", { reader >> m_Count; });
		MatchProperty("Spread", { reader >> m_Spread; });
		MatchProperty("MinVelocity", { reader >> m_MinVelocity; });
		MatchProperty("MaxVelocity", { reader >> m_MaxVelocity; });
		MatchProperty("LifeVariation", { reader >> m_LifeVariation; });
		MatchProperty("InheritsVel", { reader >> m_InheritsVel; });
		MatchProperty("IgnoresTeamHits", { reader >> m_IgnoresTeamHits; });
		MatchProperty("SpreadMode", { m_SpreadMode = static_cast<SpreadMode>(std::stoi(reader.ReadPropValue())); });

		EndPropertyList;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Gib::Save(Writer &writer) const {
		Serializable::Save(writer);

		writer.NewProperty("GibParticle");
		// All of this is needed to make a preset look like not original and save as CopyOf instead of separate preset.
		std::unique_ptr<Entity> gibEntity(m_GibParticle->Clone());
		gibEntity->ResetOriginalPresetFlag();
		gibEntity->Entity::Save(writer);
		writer.ObjectEnd();

		writer.NewProperty("Offset");
		writer << m_Offset;

		// These are disabled because we're writing CopyOfs. Might need these in the future if we need to write original presets.
		/*
		writer.NewProperty("Count");
		writer << m_Count;
		writer.NewProperty("Spread");
		writer << m_Spread;
		writer.NewProperty("MinVelocity");
		writer << GetMinVelocity();
		writer.NewProperty("MaxVelocity");
		writer << GetMaxVelocity();
		writer.NewProperty("LifeVariation");
		writer << m_LifeVariation;
		writer.NewProperty("InheritsVel");
		writer << m_InheritsVel;
		*/

		return 0;
	}
}