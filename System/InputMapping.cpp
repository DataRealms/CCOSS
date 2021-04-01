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
		m_StickMap = 0;
		m_AxisMap = 0;
		m_DirectionMap = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int InputMapping::Create(const InputMapping &reference) {
		m_KeyMap = reference.m_KeyMap;
		m_MouseButtonMap = reference.m_MouseButtonMap;
		m_DirectionMapped = reference.m_DirectionMapped;
		m_JoyButtonMap = reference.m_JoyButtonMap;
		m_StickMap = reference.m_StickMap;
		m_AxisMap = reference.m_AxisMap;
		m_DirectionMap = reference.m_DirectionMap;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int InputMapping::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "KeyMap") {
			int key;
			reader >> key;
			if (key != 0) { m_KeyMap = key; }
		} else if (propName == "MouseButtonMap") {
			reader >> m_MouseButtonMap;
		} else if (propName == "JoyButtonMap") {
			reader >> m_JoyButtonMap;
		} else if (propName == "StickMap") {
			reader >> m_StickMap;
			m_DirectionMapped = true;
		} else if (propName == "AxisMap") {
			reader >> m_AxisMap;
			m_DirectionMapped = true;
		} else if (propName == "DirectionMap") {
			reader >> m_DirectionMap;
			m_DirectionMapped = true;
		} else {
			return Serializable::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int InputMapping::Save(Writer &writer) const {
		Serializable::Save(writer);

		writer.NewPropertyWithValue("KeyMap", m_KeyMap);

		if (m_MouseButtonMap >= 0) { writer.NewPropertyWithValue("MouseButtonMap", m_MouseButtonMap); }
		if (m_JoyButtonMap >= 0) { writer.NewPropertyWithValue("JoyButtonMap", m_JoyButtonMap); }

		if (m_DirectionMapped) {
			writer.NewPropertyWithValue("StickMap", m_StickMap);
			writer.NewPropertyWithValue("AxisMap", m_AxisMap);
			writer.NewPropertyWithValue("DirectionMap", m_DirectionMap);
		}

		return 0;
	}
}