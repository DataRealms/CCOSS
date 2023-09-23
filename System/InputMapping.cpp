#include "InputMapping.h"

namespace RTE {

	const std::string InputMapping::c_ClassName = "InputMapping";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InputMapping::Clear() {
		m_PresetDescription.clear();
		m_KeyMap = 0;
		m_MouseButtonMap = -1;
		m_DirectionMapped = false;
		m_JoyButtonMap = -1;
		m_AxisMap = 0;
		m_DirectionMap = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int InputMapping::Create(const InputMapping &reference) {
		m_KeyMap = reference.m_KeyMap;
		m_MouseButtonMap = reference.m_MouseButtonMap;
		m_DirectionMapped = reference.m_DirectionMapped;
		m_JoyButtonMap = reference.m_JoyButtonMap;
		m_AxisMap = reference.m_AxisMap;
		m_DirectionMap = reference.m_DirectionMap;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int InputMapping::ReadProperty(const std::string_view &propName, Reader &reader) {
		StartPropertyList(return Serializable::ReadProperty(propName, reader));
		
		MatchProperty("KeyMap", { reader >> m_KeyMap; });
		MatchProperty("MouseButtonMap", { reader >> m_MouseButtonMap; });
		MatchProperty("JoyButtonMap", { reader >> m_JoyButtonMap; });
		MatchProperty("AxisMap", {
			reader >> m_AxisMap;
			m_DirectionMapped = true;
		});
		MatchProperty("DirectionMap", {
			reader >> m_DirectionMap;
			m_DirectionMapped = true;
		});
		
		EndPropertyList;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int InputMapping::Save(Writer &writer) const {
		Serializable::Save(writer);

		if (m_JoyButtonMap < 0 || !m_DirectionMapped) { writer.NewPropertyWithValue("KeyMap", m_KeyMap); }

		if (m_MouseButtonMap >= 0) { writer.NewPropertyWithValue("MouseButtonMap", m_MouseButtonMap); }
		if (m_JoyButtonMap >= 0) { writer.NewPropertyWithValue("JoyButtonMap", m_JoyButtonMap); }

		if (m_DirectionMapped) {
			writer.NewPropertyWithValue("AxisMap", m_AxisMap);
			writer.NewPropertyWithValue("DirectionMap", m_DirectionMap);
		}

		return 0;
	}
}
