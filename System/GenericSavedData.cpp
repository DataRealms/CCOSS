#include "GenericSavedData.h"

namespace RTE {

	const std::string GenericSavedData::c_ClassName = "GenericSavedData";
	const std::string GenericSavedData::GenericSavedStrings::c_ClassName = "GenericSavedStrings";
	const std::string GenericSavedData::GenericSavedNumbers::c_ClassName = "GenericSavedNumbers";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int GenericSavedData::ReadProperty(const std::string_view &propName, Reader &reader) {
		StartPropertyList(return Serializable::ReadProperty(propName, reader));
		
		MatchProperty("StringValues", {
			reader >> m_SavedStrings;
		}); MatchProperty("NumberValues", {
			reader >> m_SavedNumbers;
		});

		EndPropertyList;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int GenericSavedData::Save(Writer &writer) const {
		Serializable::Save(writer);

		writer.NewPropertyWithValue("StringValues", m_SavedStrings);
		writer.NewPropertyWithValue("NumberValues", m_SavedNumbers);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int GenericSavedData::GenericSavedStrings::ReadProperty(const std::string_view &propName, Reader &reader) {
		m_Data[std::string(propName)] = reader.ReadPropValue(); // until we get P0919R2.
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int GenericSavedData::GenericSavedStrings::Save(Writer &writer) const {
		Serializable::Save(writer);

		for (const auto &[propName, value] : m_Data) {
			if (!value.empty()) { writer.NewPropertyWithValue(propName, value); }
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int GenericSavedData::GenericSavedNumbers::ReadProperty(const std::string_view &propName, Reader &reader) {
		float value;
		reader >> value;
		m_Data[std::string(propName)] = value; // until we get P0919R2.
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int GenericSavedData::GenericSavedNumbers::Save(Writer &writer) const {
		Serializable::Save(writer);

		for (const auto &[propName, value] : m_Data) {
			writer.NewPropertyWithValue(propName, value);
		}

		return 0;
	}
}