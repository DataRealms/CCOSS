#include "InputMapping.h"

namespace RTE {

	const std::string InputMapping::c_ClassName = "InputMapping";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InputMapping::Clear() {
		m_PresetDesc.clear();
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

	int InputMapping::ReadProperty(std::string propName, Reader &reader) {
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

		writer.NewProperty("KeyMap");
		writer << m_KeyMap;

		if (m_MouseButtonMap >= 0) {
			writer.NewProperty("MouseButtonMap");
			writer << m_MouseButtonMap;
		}
		if (m_JoyButtonMap >= 0) {
			writer.NewProperty("JoyButtonMap");
			writer << m_JoyButtonMap;
		}
		// Only save direction map if it's enabled
		if (m_DirectionMapped) {
			writer.NewProperty("StickMap");
			writer << m_StickMap;
			writer.NewProperty("AxisMap");
			writer << m_AxisMap;
			writer.NewProperty("DirectionMap");
			writer << m_DirectionMap;
		}
		return 0;
	}
}