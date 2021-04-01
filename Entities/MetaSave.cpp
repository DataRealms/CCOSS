#include "MetaSave.h"
#include "GameActivity.h"
#include "MetaMan.h"
#include "ConsoleMan.h"

namespace RTE {

	ConcreteClassInfo(MetaSave, Entity, 0)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MetaSave::Clear() {
		m_SavePath.clear();
		m_PlayerCount = 0;
		m_Difficulty = Activity::MediumDifficulty;
		m_RoundCount = 0;
		m_SiteCount = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MetaSave::Create(string savePath) {
		if (Entity::Create() < 0) {
			return -1;
		}
		if (!g_MetaMan.GameInProgress()) {
			g_ConsoleMan.PrintString("ERROR: Tried to save a Metagame that isn't in progress!?");
			return -1;
		}
		m_SavePath = savePath;

		m_PlayerCount = g_MetaMan.m_Players.size();
		m_Difficulty = g_MetaMan.m_Difficulty;
		m_RoundCount = g_MetaMan.m_CurrentRound;
		m_SiteCount = g_MetaMan.m_Scenes.size();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MetaSave::Create(const MetaSave &reference) {
		Entity::Create(reference);

		m_SavePath = reference.m_SavePath;
		m_PlayerCount = reference.m_PlayerCount;
		m_Difficulty = reference.m_Difficulty;
		m_RoundCount = reference.m_RoundCount;
		m_SiteCount = reference.m_SiteCount;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MetaSave::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "SavePath") {
			reader >> m_SavePath;
		} else if (propName == "PlayerCount") {
			reader >> m_PlayerCount;
		} else if (propName == "Difficulty") {
			reader >> m_Difficulty;
		} else if (propName == "RoundCount") {
			reader >> m_RoundCount;
		} else if (propName == "SiteCount") {
			reader >> m_SiteCount;
		} else {
			return Entity::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MetaSave::Save(Writer &writer) const {
		Entity::Save(writer);

		writer.NewProperty("SavePath");
		writer << m_SavePath;
		writer.NewProperty("PlayerCount");
		writer << m_PlayerCount;
		writer.NewProperty("Difficulty");
		writer << m_Difficulty;
		writer.NewProperty("RoundCount");
		writer << m_RoundCount;
		writer.NewProperty("SiteCount");
		writer << m_SiteCount;

		return 0;
	}
}