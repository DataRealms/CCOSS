#include "PieSlice.h"

#include "Constants.h"

#include "PresetMan.h"

namespace RTE {

	const string PieSlice::c_ClassName = "Slice";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieSlice::Clear() {
		m_SliceType = PieSliceIndex::PSI_NONE;
		m_Description.clear();
		m_Direction = SliceDirection::UP;
		m_Enabled = true;
		m_Icon.Reset();

		m_ScriptPath.clear();
		m_FunctionName.clear();

		m_AreaStart = 0;
		m_AreaArc = c_QuarterPI;
		m_MidAngle = m_AreaStart + (m_AreaArc / 2);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PieSlice::Create() {
		if (Serializable::Create() < 0) {
			return -1;
		}

		switch (m_SliceType) {
			case PieSliceIndex::PSI_NONE:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Blank"));
				break;
			case PieSliceIndex::PSI_PICKUP:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Pick Up"));
				break;
			case PieSliceIndex::PSI_DROP:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Drop"));
				break;
			case PieSliceIndex::PSI_NEXTITEM:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Next"));
				break;
			case PieSliceIndex::PSI_PREVITEM:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Previous"));
				break;
			case PieSliceIndex::PSI_RELOAD:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Reload"));
				break;
			case PieSliceIndex::PSI_BUYMENU:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Trade Star"));
				break;
			case PieSliceIndex::PSI_FULLINVENTORY:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Unknown"));
				break;
			case PieSliceIndex::PSI_STATS:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Stats"));
				break;
			case PieSliceIndex::PSI_MINIMAP:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Minimap"));
				break;
			case PieSliceIndex::PSI_FORMSQUAD:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Follow"));
				break;
			case PieSliceIndex::PSI_CEASEFIRE:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "White Flag"));
				break;
			case PieSliceIndex::PSI_SENTRY:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Eye"));
				break;
			case PieSliceIndex::PSI_PATROL:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Cycle"));
				break;
			case PieSliceIndex::PSI_BRAINHUNT:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Brain"));
				break;
			case PieSliceIndex::PSI_GOLDDIG:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Dig"));
				break;
			case PieSliceIndex::PSI_GOTO:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Go To"));
				break;
			case PieSliceIndex::PSI_RETURN:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Launch"));
				break;
			case PieSliceIndex::PSI_STAY:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Land"));
				break;
			case PieSliceIndex::PSI_SCUTTLE:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Death"));
				break;
			case PieSliceIndex::PSI_DONE:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Check"));
				break;
			case PieSliceIndex::PSI_LOAD:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Load"));
				break;
			case PieSliceIndex::PSI_SAVE:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Save"));
				break;
			case PieSliceIndex::PSI_NEW:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Clear"));
				break;
			case PieSliceIndex::PSI_PICK:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Menu"));
				break;
			case PieSliceIndex::PSI_MOVE:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Move"));
				break;
			case PieSliceIndex::PSI_REMOVE:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Remove"));
				break;
			case PieSliceIndex::PSI_INFRONT:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "In Front"));
				break;
			case PieSliceIndex::PSI_BEHIND:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Behind"));
				break;
			case PieSliceIndex::PSI_ZOOMIN:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Zoom In"));
				break;
			case PieSliceIndex::PSI_ZOOMOUT:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Zoom Out"));
				break;
			case PieSliceIndex::PSI_TEAM1:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 1"));
				break;
			case PieSliceIndex::PSI_TEAM2:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 2"));
				break;
			case PieSliceIndex::PSI_TEAM3:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 3"));
				break;
			case PieSliceIndex::PSI_TEAM4:
				m_Icon = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 4"));
				break;
			case PieSliceIndex::PSI_SCRIPTED:
				break;
			default:
				RTEAbort("Invalid sliceType " + static_cast<int>(m_SliceType));
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

		m_ScriptPath = reference.m_ScriptPath;
		m_FunctionName = reference.m_FunctionName;

		m_AreaStart = reference.m_AreaStart;
		m_AreaArc = reference.m_AreaArc;
		m_MidAngle = reference.m_MidAngle;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP *RTE::PieSlice::GetAppropriateIcon(bool sliceIsSelected) const {
		int iconFrameCount = m_Icon.GetFrameCount();
		if (iconFrameCount > 0) {
			if (!IsEnabled() && iconFrameCount > 2) {
				return m_Icon.GetBitmaps8()[2];
			} else if (sliceIsSelected && iconFrameCount > 1) {
				return m_Icon.GetBitmaps8()[1];
			} else {
				return m_Icon.GetBitmaps8()[0];
			}
		}
		return nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PieSlice::ReadProperty(const std::string_view &propName, Reader &reader) {
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

		writer.NewPropertyWithValue("Description", m_Description);
		writer.NewPropertyWithValue("Icon", m_Icon);
		writer.NewPropertyWithValue("Direction", static_cast<int>(m_Direction));
		if (!m_ScriptPath.empty() && !m_FunctionName.empty()) {
			writer.NewPropertyWithValue("ScriptPath", m_ScriptPath);
			writer.NewPropertyWithValue("FunctionName", m_FunctionName);
		}

		return 0;
	}
}