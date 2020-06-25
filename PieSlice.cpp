#include "PieSlice.h"

#include "Constants.h"

#include "PresetMan.h"

using namespace RTE;

const string PieSlice::m_sClassName = "Slice";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PieSlice::Clear() {
	m_SliceType = PSI_NONE;
	m_Description.clear();
	m_Direction = UP;
	m_Enabled = true;
	m_Icon.Reset();
	m_AreaStart = 0;
	m_AreaArc = c_QuarterPI;
	m_MidAngle = m_AreaStart + (m_AreaArc / 2);
	m_ScriptPath.clear();
	m_FunctionName.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PieSlice::Create() {
	if (Serializable::Create() < 0) {
		return -1;
	}

	switch (m_SliceType) {
		case PSI_NONE:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Blank"));
			break;
		case PSI_PICKUP:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Pick Up"));
			break;
		case PSI_DROP:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Drop"));
			break;
		case PSI_NEXTITEM:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Next"));
			break;
		case PSI_PREVITEM:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Previous"));
			break;
		case PSI_RELOAD:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Reload"));
			break;
		case PSI_BUYMENU:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Trade Star"));
			break;
		case PSI_STATS:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Stats"));
			break;
		case PSI_MINIMAP:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Minimap"));
			break;
		case PSI_FORMSQUAD:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Follow"));
			break;
		case PSI_CEASEFIRE:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "White Flag"));
			break;
		case PSI_SENTRY:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Eye"));
			break;
		case PSI_PATROL:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Cycle"));
			break;
		case PSI_BRAINHUNT:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Brain"));
			break;
		case PSI_GOLDDIG:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Dig"));
			break;
		case PSI_GOTO:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Go To"));
			break;
		case PSI_RETURN:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Launch"));
			break;
		case PSI_STAY:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Land"));
			break;
		case PSI_SCUTTLE:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Death"));
			break;
		case PSI_DONE:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Check"));
			break;
		case PSI_LOAD:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Load"));
			break;
		case PSI_SAVE:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Save"));
			break;
		case PSI_NEW:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Clear"));
			break;
		case PSI_PICK:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Menu"));
			break;
		case PSI_MOVE:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Move"));
			break;
		case PSI_REMOVE:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Remove"));
			break;
		case PSI_INFRONT:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "In Front"));
			break;
		case PSI_BEHIND:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Behind"));
			break;
		case PSI_ZOOMIN:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Zoom In"));
			break;
		case PSI_ZOOMOUT:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Zoom Out"));
			break;
		case PSI_TEAM1:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 1"));
			break;
		case PSI_TEAM2:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 2"));
			break;
		case PSI_TEAM3:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 3"));
			break;
		case PSI_TEAM4:
			m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 4"));
			break;
		case PSI_SCRIPTED:
			break;
		default:
			RTEAbort("Invalid sliceType " + m_SliceType);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PieSlice::Create(const PieSlice &reference) {
	m_SliceType = reference.m_SliceType;
	m_Description = reference.m_Description;
	m_Direction = reference.m_Direction;
	m_Enabled = reference.m_Enabled;
	m_Icon = reference.m_Icon;
	m_AreaStart = reference.m_AreaStart;
	m_AreaArc = reference.m_AreaArc;
	m_MidAngle = reference.m_MidAngle;
	m_ScriptPath = reference.m_ScriptPath;
	m_FunctionName = reference.m_FunctionName;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PieSlice::ReadProperty(std::string propName, Reader &reader) {
	if (propName == "Description") {
		reader >> m_Description;
	} else if (propName == "Icon") {
		reader >> m_Icon;
	} else if (propName == "Direction") {
		m_Direction = static_cast<SliceDirection>(std::stoi(reader.ReadPropValue()));
	} else if (propName == "ScriptPath") {
		reader >> m_ScriptPath;
	} else if (propName == "FunctionName") {
		reader >> m_FunctionName;
	} else {
		return Serializable::ReadProperty(propName, reader);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PieSlice::Save(Writer &writer) const {
	Serializable::Save(writer);

	writer.NewProperty("Description");
	writer << m_Description;
	writer.NewProperty("Icon");
	writer << m_Icon;
	writer.NewProperty("Direction");
	writer << m_Direction;
	writer.NewProperty("ScriptPath");
	writer << m_ScriptPath;
	writer.NewProperty("FunctionName");
	writer << m_FunctionName;

	return 0;
}