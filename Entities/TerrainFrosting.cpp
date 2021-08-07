#include "TerrainFrosting.h"

namespace RTE {

	const std::string TerrainFrosting::c_ClassName = "TerrainFrosting";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TerrainFrosting::Clear() {
		m_TargetMaterial.Reset();
		m_FrostingMaterial.Reset();
		m_MinThickness = 5;
		m_MaxThickness = 5;
		m_InAirOnly = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TerrainFrosting::Create(const TerrainFrosting &reference) {
		m_TargetMaterial = reference.m_TargetMaterial;
		m_FrostingMaterial = reference.m_FrostingMaterial;
		m_MinThickness = reference.m_MinThickness;
		m_MaxThickness = reference.m_MaxThickness;
		m_InAirOnly = reference.m_InAirOnly;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TerrainFrosting::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "TargetMaterial") {
			reader >> m_TargetMaterial;
		} else if (propName == "FrostingMaterial") {
			reader >> m_FrostingMaterial;
		} else if (propName == "MinThickness") {
			reader >> m_MinThickness;
		} else if (propName == "MaxThickness") {
			reader >> m_MaxThickness;
		} else if (propName == "InAirOnly") {
			reader >> m_InAirOnly;
		} else {
			return Serializable::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TerrainFrosting::Save(Writer &writer) const {
		Serializable::Save(writer);

		writer.NewPropertyWithValue("TargetMaterial", m_TargetMaterial);
		writer.NewPropertyWithValue("FrostingMaterial", m_FrostingMaterial);
		writer.NewPropertyWithValue("MinThickness", m_MinThickness);
		writer.NewPropertyWithValue("MaxThickness", m_MaxThickness);
		writer.NewPropertyWithValue("InAirOnly", m_InAirOnly);

		return 0;
	}
}