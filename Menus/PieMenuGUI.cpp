#include "PieMenuGUI.h"

#include "FrameMan.h"
#include "UInputMan.h"

#include "Controller.h"
#include "AHuman.h"
#include "ContentFile.h"

#include "GUI/GUI.h"
#include "GUI/GUIFont.h"
#include "GUI/AllegroBitmap.h"

using namespace RTE;

BITMAP *PieMenuGUI::s_Cursor;
std::unordered_map<std::string, PieSlice> PieMenuGUI::s_AllCustomLuaSlices;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PieMenuGUI::Clear() {
	m_EnablingTimer.Reset();
	m_HoverTimer.Reset();

	m_Controller = nullptr;
	m_AffectedObject = nullptr;

	m_PieEnabled = PieEnabled::DISABLED;
	m_CenterPos.Reset();

	m_HoveredSlice = nullptr;
	m_ActivatedSlice = nullptr;
	m_AlreadyActivatedSlice = nullptr;

	m_UpSlice.Reset();
	m_RightSlice.Reset();
	m_DownSlice.Reset();
	m_LeftSlice.Reset();

	m_UpRightSlices.clear();
	m_UpLeftSlices.clear();
	m_DownLeftSlices.clear();
	m_DownRightSlices.clear();

	m_CurrentSlices.clear();
	m_SliceGroupCount = 0;

	m_Wobbling = false;
	m_Freeze = false;

	m_InnerRadius = 0;
	m_EnabledRadius = 58;
	m_Thickness = 16;
	m_CursorAngle = 0;

	m_BGBitmap = nullptr;
	m_RedrawBG = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PieMenuGUI::Create(Controller *controller, Actor *focusActor) {
	RTEAssert(controller, "No controller sent to PieMenuGUI on creation!");
	m_Controller = controller;

	m_Actor = focusActor;
	if (m_Actor) {
		m_CenterPos = m_Actor->GetCPUPos();
		m_LastKnownActor = m_Actor;
	}

	if (!s_Cursor) {
		s_Cursor = ContentFile("Base.rte/GUIs/Skins/PieCursor.png").GetAsBitmap();
	}

	if (!m_BGBitmap) {
		int diameter = (m_EnabledRadius + m_Thickness + 2) * 2;
		m_BGBitmap = create_bitmap_ex(8, diameter, diameter);
		clear_to_color(m_BGBitmap, g_MaskColor);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PieMenuGUI::SetEnabled(bool enable) {
	m_Wobbling = false;
	m_Freeze = false;

	if (!((enable && m_PieEnabled != ENABLED && m_PieEnabled != ENABLING) || (!enable && m_PieEnabled != DISABLED && m_PieEnabled != DISABLING))) {
		return;
	}

	m_PieEnabled = enable ? ENABLING : DISABLING;
	m_EnablingTimer.Reset();
	if (m_Controller->IsMouseControlled()) {
		g_UInputMan.SetMouseValueMagnitude(0);
		m_Controller->m_AnalogCursor = Vector();
	}
	SoundContainer *soundToPlay = enable ? g_GUISound.PieMenuEnterSound() : g_GUISound.PieMenuExitSound();
	soundToPlay->Play();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PieMenuGUI::ResetSlices() {
	m_HoveredSlice = nullptr;
	m_ActivatedSlice = nullptr;
	m_AlreadyActivatedSlice = nullptr;

	m_UpSlice.Reset();
	m_RightSlice.Reset();
	m_DownSlice.Reset();
	m_LeftSlice.Reset();

	m_UpRightSlices.clear();
	m_UpLeftSlices.clear();
	m_DownLeftSlices.clear();
	m_DownRightSlices.clear();

	m_CurrentSlices.clear();
	m_SliceGroupCount = 0;
	m_RedrawBG = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PieMenuGUI::AddSlice(PieSlice &newSlice, bool takeAnyFreeCardinal) {
	if (newSlice.GetIcon().GetFrameCount() <= 0) {
		newSlice.Create();
	}
	std::unordered_map<PieSlice::SliceDirection, PieSlice *> sliceCardinalDirections = {
		{PieSlice::UP, &m_UpSlice}, {PieSlice::LEFT, &m_LeftSlice}, {PieSlice::DOWN, &m_DownSlice}, {PieSlice::RIGHT, &m_RightSlice}
	};
	std::unordered_map<PieSlice::SliceDirection, std::pair<std::list<PieSlice> *, std::list<PieSlice> *>> sliceIntercardinalDirections = {
		{PieSlice::UP, {&m_UpRightSlices, &m_UpLeftSlices}},
		{PieSlice::LEFT, {&m_UpLeftSlices, &m_DownLeftSlices}},
		{PieSlice::DOWN, {&m_DownLeftSlices, &m_DownRightSlices}},
		{PieSlice::RIGHT, {&m_UpRightSlices, &m_DownRightSlices}}
	};

	for (const auto &[sliceDirection, pieSlice] : sliceCardinalDirections) {
		if ((takeAnyFreeCardinal || sliceDirection == newSlice.GetDirection()) && pieSlice->GetType() == PieSlice::PSI_NONE) {
			*pieSlice = newSlice;
			return true;
		}
	}

	const auto &[firstPieSliceList, secondPieSliceList] = sliceIntercardinalDirections.at(newSlice.GetDirection());
	if (firstPieSliceList->size() <= secondPieSliceList->size()) {
		firstPieSliceList->push_back(newSlice);
	} else {
		secondPieSliceList->push_back(newSlice);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PieMenuGUI::RealignSlices() {
	float sliceSpacer = c_EighthPI;
	std::vector<PieSlice *> cardinalSlices = {&m_UpSlice, &m_LeftSlice, &m_DownSlice, &m_RightSlice};
	std::vector<std::list<PieSlice> *> intercardinalSliceCollections = {&m_UpRightSlices, &m_UpLeftSlices, &m_DownLeftSlices, &m_DownRightSlices};

	m_CurrentSlices.clear();

	/// <summary>
	/// Internal lambda function to set area values for a given intercardinal slice, and add it to the list of all slices.
	/// </summary>
	/// <param name="pieSlice">A reference to the slice object.</param>
	/// <param name="sliceListSize">The size of the list the slice is in.</param>
	/// <param name="currentSliceNumber">The current slice number being handled in the current list of intercardinal slices.</param>
	/// <param name="currentAngleOffset">The current angle offset for this list of intercardinal slices.</param>
	auto handleIntercardinalSlice = [this](PieSlice &pieSlice, int sliceListSize, int currentSliceNumber, float currentAngleOffset) {
		pieSlice.SetAreaArc(c_QuarterPI / static_cast<float>(sliceListSize));
		pieSlice.SetAreaStart(currentAngleOffset + (static_cast<float>(currentSliceNumber) * pieSlice.GetAreaArc()));
		m_CurrentSlices.push_back(&pieSlice);
	};

	std::list<PieSlice>::reverse_iterator sliceReverseIterator;
	float currentAngleOffset = sliceSpacer;
	for (int i = 0; i < intercardinalSliceCollections.size(); i++) {
		std::list<PieSlice> &currentIntercardinalSliceList = *intercardinalSliceCollections[i];

		if (!currentIntercardinalSliceList.empty()) {
			// Narrow the previous direction's cardinal slice's arc area, to account for intercardinal slices here.
			if (i > 0) {
				cardinalSlices[i - 1]->SetAreaArc(cardinalSlices[i - 1]->GetAreaArc() - sliceSpacer);
			}

			int currentSliceNumber = 0;
			// Right side slices need to be handled in reverse order to properly traverse the slices in CCW order. Outside of that, all intercardinal slices are handled in the same way.
			if (i == 0 || i == 3) {
				for (sliceReverseIterator = currentIntercardinalSliceList.rbegin(); sliceReverseIterator != currentIntercardinalSliceList.rend(); ++sliceReverseIterator) {
					handleIntercardinalSlice((*sliceReverseIterator), static_cast<int>(currentIntercardinalSliceList.size()), currentSliceNumber, currentAngleOffset);
					currentSliceNumber++;
				}
			} else {
				for (PieSlice &slice : currentIntercardinalSliceList) {
					handleIntercardinalSlice(slice, static_cast<int>(currentIntercardinalSliceList.size()), currentSliceNumber, currentAngleOffset);
					currentSliceNumber++;
				}
			}
		}

		float spaceToRemove = currentIntercardinalSliceList.empty() ? sliceSpacer : 0;
		cardinalSlices[i]->SetAreaStart(currentAngleOffset + c_QuarterPI - spaceToRemove);
		
		spaceToRemove = (i == 3 && !intercardinalSliceCollections[0]->empty()) ? sliceSpacer : 0;
		cardinalSlices[i]->SetAreaArc((intercardinalSliceCollections[i]->empty() ? c_HalfPI : c_QuarterPI + sliceSpacer) - spaceToRemove);

		m_CurrentSlices.push_back(cardinalSlices[i]);

		currentAngleOffset += c_HalfPI;
	}

	for (PieSlice *slice : m_CurrentSlices) {
		slice->SetMidAngle(slice->GetAreaStart() + (slice->GetAreaArc() / 2));
	}
	m_RedrawBG = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const PieSlice * PieMenuGUI::GetSliceOnAngle(float angle) const {
	float areaEnd;
	for (const PieSlice *slice : m_CurrentSlices) {
		areaEnd = slice->GetAreaStart() + slice->GetAreaArc();
		if (slice->GetAreaStart() <= angle && areaEnd > angle) {
			return slice;
		}
		// Check for if the target slice overlaps the 0 point on the arc
		if (areaEnd > c_TwoPI && angle >= 0 && angle < (areaEnd - c_TwoPI)) {
			return slice;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PieMenuGUI::AddSliceLua(const std::string &description, const std::string &functionName, PieSlice::SliceDirection direction, bool isEnabled) {
	if (s_AllCustomLuaSlices.find(description + "::" + functionName) != s_AllCustomLuaSlices.end()) {
		PieSlice foundSlice = s_AllCustomLuaSlices[description + "::" + functionName];
		foundSlice.SetDirection(direction);
		foundSlice.SetEnabled(isEnabled);
		AddSlice(foundSlice);
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PieMenuGUI::AlterSliceLua(const std::string &description, const std::string &functionName, PieSlice::SliceDirection direction, bool isEnabled) {
	PieSlice foundSlice;
	std::vector<PieSlice *>::iterator sliceIterator = std::find_if(m_CurrentSlices.begin(), m_CurrentSlices.end(), [&description, &functionName](const PieSlice *slice) {
		return slice->GetDescription() == description && slice->GetFunctionName() == functionName;
	});

	if (sliceIterator != m_CurrentSlices.end() && (*sliceIterator)->GetType() != PieSlice::PSI_NONE) {
		foundSlice.SetDirection(direction);
		foundSlice.SetEnabled(isEnabled);

		RemoveSliceLua(description, functionName);
		AddSlice(foundSlice);

		RealignSlices();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PieSlice PieMenuGUI::RemoveSliceLua(const std::string &description, const std::string &functionName) {
	PieSlice removedSlice;

	if (m_UpSlice.GetDescription() == description && m_UpSlice.GetFunctionName() == functionName)
		m_UpSlice.Reset();
	if (m_DownSlice.GetDescription() == description && m_DownSlice.GetFunctionName() == functionName)
		m_DownSlice.Reset();
	if (m_LeftSlice.GetDescription() == description && m_LeftSlice.GetFunctionName() == functionName)
		m_LeftSlice.Reset();
	if (m_RightSlice.GetDescription() == description && m_RightSlice.GetFunctionName() == functionName)
		m_RightSlice.Reset();
	else {
		std::vector<std::list<PieSlice> *> sliceCollections = {&m_UpRightSlices, &m_UpLeftSlices, &m_DownLeftSlices, &m_DownRightSlices};
		for (std::list<PieSlice> *sliceList : sliceCollections) {
			std::list<PieSlice>::iterator sliceIterator = std::find_if(sliceList->begin(), sliceList->end(), [&description, &functionName](const PieSlice &slice) {
				return slice.GetDescription() == description && slice.GetFunctionName() == functionName;
			});
			if (sliceIterator != sliceList->end()) {
				removedSlice = (*sliceIterator);
				sliceList->erase(sliceIterator);
				break;
			}
		}
	}

	RealignSlices();

	return removedSlice;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PieMenuGUI::Update() {
	m_ActivatedSlice = nullptr;

	if (m_Wobbling) {
		UpdateWobbling();
	} else if (m_Freeze) {
		m_PieEnabled = ENABLING;
	} else if (m_PieEnabled == ENABLING || m_PieEnabled == DISABLING) {
		UpdateEnablingAndDisablingAnimations();
	}

	if (m_Controller->IsDisabled()) {
		return;
	}

	if (IsEnabled() && (UpdateAnalogInput() || UpdateDigitalInput())) { m_HoverTimer.Reset(); }

	if (m_HoveredSlice && m_PieEnabled != DISABLED) {
		UpdateSliceActivation();
		
		if (m_HoverTimer.IsPastSimMS(1500)) {
			m_HoveredSlice = nullptr;
			m_RedrawBG = true;
		}
	}

	//TODO try moving this into draw, it'd make more sense there
	if (m_RedrawBG && m_PieEnabled != DISABLED) { RedrawMenuBackground(); }

	//TODO All this actor stuff should be gotten rid of/replaced so pie menus can be on any MO
	// Clear out actor and last known actor cause it may have been deleted by the time we next update.
	m_Actor = 0;
	if (m_LastKnownActor && (!g_MovableMan.IsActor(m_LastKnownActor) || !dynamic_cast<Actor *>(m_LastKnownActor))) { m_LastKnownActor = 0; }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PieMenuGUI::Draw(BITMAP *targetBitmap, const Vector &targetPos) const  {
	GUIFont *menuFont = g_FrameMan.GetLargeFont();

	Vector drawPos;
	CalculateDrawPositionAccountingForSeamsAndFont(targetBitmap, targetPos, menuFont, drawPos);

	if (m_PieEnabled != DISABLED) {
		if (!g_FrameMan.IsInMultiplayerMode()) {
			g_FrameMan.SetTransTable(MoreTrans);
			draw_trans_sprite(targetBitmap, m_BGBitmap, drawPos.GetFloorIntX() - m_BGBitmap->w / 2, drawPos.GetFloorIntY() - m_BGBitmap->h / 2);
		} else {
			draw_sprite(targetBitmap, m_BGBitmap, drawPos.GetFloorIntX() - m_BGBitmap->w / 2, drawPos.GetFloorIntY() - m_BGBitmap->h / 2);
		}
	}

	if (m_PieEnabled == ENABLED) {
		DrawPieIcons(targetBitmap, drawPos);
		if (m_HoveredSlice) { DrawPieCursorAndSliceDescriptions(targetBitmap, drawPos, menuFont); }
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PieMenuGUI::SelectSlice(const PieSlice *sliceToSelect, bool moveCursorToSlice) {
	if (sliceToSelect == nullptr || sliceToSelect == m_HoveredSlice) {
		return false;
	}

	m_HoveredSlice = sliceToSelect;

	m_RedrawBG = true;
	if (moveCursorToSlice) { m_CursorAngle = m_HoveredSlice->GetMidAngle(); }

	SoundContainer *soundToPlay = sliceToSelect->IsEnabled() ? g_GUISound.HoverChangeSound() : g_GUISound.HoverDisabledSound();
	soundToPlay->Play();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PieMenuGUI::UpdateWobbling() {
	float innerRadiusChange = static_cast<float>(m_EnablingTimer.GetElapsedRealTimeMS()) / 6.0F;

	m_RedrawBG = true;
	m_InnerRadius += static_cast<int>(innerRadiusChange) * (m_PieEnabled == DISABLING ? -1 : 1);

	if (m_InnerRadius < 0) {
		m_PieEnabled = ENABLING;
	} else if (m_InnerRadius > m_EnabledRadius / 2) {
		m_PieEnabled = DISABLING;
	}

	m_InnerRadius = std::clamp(m_InnerRadius, 0, m_EnabledRadius / 2);
	m_EnablingTimer.Reset();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PieMenuGUI::UpdateEnablingAndDisablingAnimations() {
	m_RedrawBG = true;
	if (m_PieEnabled == ENABLING) {
		m_InnerRadius = static_cast<int>(LERP(0.0F, static_cast<float>(s_EnablingDelay), 0.0F, static_cast<float>(m_EnabledRadius), static_cast<float>(m_EnablingTimer.GetElapsedRealTimeMS())));
		if (m_EnablingTimer.IsPastRealMS(s_EnablingDelay)) {
			m_PieEnabled = ENABLED;
			m_InnerRadius = m_EnabledRadius;
		}
	} else if (m_PieEnabled == DISABLING) {
		m_InnerRadius = static_cast<int>(LERP(0.0F, static_cast<float>(s_EnablingDelay), static_cast<float>(m_EnabledRadius), 0.0F, static_cast<float>(m_EnablingTimer.GetElapsedRealTimeMS())));
		if (m_EnablingTimer.IsPastRealMS(s_EnablingDelay)) {
			m_PieEnabled = DISABLED;
			m_InnerRadius = 0;
			if (m_AffectedObject && static_cast<Actor *>(m_AffectedObject)) {
				static_cast<Actor *>(m_AffectedObject)->FlashWhite();
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PieMenuGUI::UpdateAnalogInput() {
	float cursorDistance = m_Controller->GetAnalogCursor().GetLargest();
	if (cursorDistance > 0.45F) {
		m_CursorAngle = m_Controller->GetAnalogCursor().GetAbsRadAngle();

		while (m_CursorAngle < 0) { m_CursorAngle += c_TwoPI; }

		SelectSlice(GetSliceOnAngle(m_CursorAngle));
		return true;
	} else if (cursorDistance > 0.35 && cursorDistance <= 0.45) {
		m_HoveredSlice = nullptr;
		m_RedrawBG = true;
		return false;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PieMenuGUI::UpdateDigitalInput() {
	std::map<ControlState, PieSlice *> controlStateSlices = {
		{PRESS_UP, &m_UpSlice},
		{PRESS_LEFT, &m_LeftSlice},
		{PRESS_DOWN, &m_DownSlice},
		{PRESS_RIGHT, &m_RightSlice}
	};
	std::map<ControlState, std::pair<std::pair<float, float>, bool>> controlStateSelectionZoomAngles = {
		{PRESS_UP, {{m_LeftSlice.GetMidAngle(), m_RightSlice.GetMidAngle()}, true}},
		{PRESS_LEFT, {{m_DownSlice.GetMidAngle(), m_UpSlice.GetMidAngle()}, false}},
		{PRESS_DOWN, {{m_RightSlice.GetMidAngle(), m_LeftSlice.GetMidAngle()}, false}},
		{PRESS_RIGHT, {{m_UpSlice.GetMidAngle(), m_DownSlice.GetMidAngle()}, true}}
	};
	std::map<ControlState, std::pair<std::pair<float, float>, bool>> controlStateSelectionStepAngles = {
		{PRESS_UP, {{m_UpSlice.GetMidAngle(), m_DownSlice.GetMidAngle()}, true}},
		{PRESS_LEFT, {{m_LeftSlice.GetMidAngle(), 0}, false}},
		{PRESS_DOWN, {{m_DownSlice.GetMidAngle(), m_UpSlice.GetMidAngle()}, false}},
		{PRESS_RIGHT, {{0, m_LeftSlice.GetMidAngle()}, true}}
	};

	for (const auto &[controlState, pieSlice] : controlStateSlices) {
		if (m_Controller->IsState(controlState)) {
			const auto &[firstSelectionZoomAngle, secondSelectionZoomAngle] = controlStateSelectionZoomAngles.at(controlState).first;
			bool shouldZoom = !m_HoveredSlice;
			shouldZoom |= controlStateSelectionZoomAngles.at(controlState).second ?
				(m_CursorAngle > firstSelectionZoomAngle && m_CursorAngle < secondSelectionZoomAngle) :
				(m_CursorAngle > firstSelectionZoomAngle || m_CursorAngle < secondSelectionZoomAngle);

			if (shouldZoom) {
				return SelectSlice(pieSlice, true);
			} else if (m_HoveredSlice != pieSlice) {
				const auto &[firstSelectionStepAngle, secondSelectionStepAngle] = controlStateSelectionStepAngles.at(controlState).first;
				int stepDirection = controlStateSelectionStepAngles.at(controlState).second ?
					((m_CursorAngle > firstSelectionStepAngle && m_CursorAngle < secondSelectionStepAngle) ? -1 : 1) :
					((m_CursorAngle > firstSelectionStepAngle || m_CursorAngle < secondSelectionStepAngle) ? -1 : 1);

				vector<PieSlice *>::iterator sliceToSelect = std::find(m_CurrentSlices.begin(), m_CurrentSlices.end(), m_HoveredSlice) + stepDirection;
				if (sliceToSelect >= m_CurrentSlices.end()) {
					sliceToSelect = m_CurrentSlices.begin();
				} else if (sliceToSelect < m_CurrentSlices.begin()) {
					sliceToSelect = m_CurrentSlices.end() - 1;
				}

				if ((*sliceToSelect) != nullptr) {
					return SelectSlice((*sliceToSelect), true);
				}
			}
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PieMenuGUI::UpdateSliceActivation() {
	if (m_Controller->IsState(PRESS_PRIMARY) || (m_HoveredSlice != m_AlreadyActivatedSlice && m_Controller->IsState(RELEASE_SECONDARY))) {
		m_HoverTimer.Reset();
		m_ActivatedSlice = m_HoveredSlice->IsEnabled() ? m_HoveredSlice : m_ActivatedSlice;
		m_AlreadyActivatedSlice = m_ActivatedSlice;

		SoundContainer *soundToPlay = m_HoveredSlice->IsEnabled() ? g_GUISound.SlicePickedSound() : g_GUISound.DisabledPickedSound();
		soundToPlay->Play();
	}

	if (GetPieCommand() == PieSlice::PSI_SCRIPTED) {
		g_LuaMan.SetTempEntity(m_Actor);
		// TODO: Investigate reloading the file each time. I think it's needed cause this stuff isn't in PresetMan, so this is the only way for it to be reloadable. To test, have script on slice, edit it and see what happens with and without this.
		g_LuaMan.RunScriptFile(m_ActivatedSlice->GetScriptPath());
		g_LuaMan.RunScriptString(m_ActivatedSlice->GetFunctionName() + "(ToActor(LuaMan.TempEntity));");
	}
}

void PieMenuGUI::RedrawMenuBackground() {
	clear_to_color(m_BGBitmap, g_MaskColor);
	int centerX = m_BGBitmap->w / 2;
	int centerY = m_BGBitmap->h / 2;

	// Pie menu circle drawing is done by drawing a larger circle and drawing a smaller one in the mask color (i.e. transparent) on top of it.
	circlefill(m_BGBitmap, centerX, centerY, m_InnerRadius + m_Thickness, 4);
	circlefill(m_BGBitmap, centerX, centerY, m_InnerRadius, g_MaskColor);

	if (m_PieEnabled == ENABLED) {
		Vector separator;

		// Draw four separator lines between each slice so the resulting separation will be at least 2 pixels thick, regardless of the separated slices' angles.
		for (const PieSlice *slice : m_CurrentSlices) {
			separator.SetXY(static_cast<float>(m_InnerRadius + m_Thickness) + 2.0F, 0.0F);
			separator.RadRotate(slice->GetAreaStart());
			line(m_BGBitmap, centerX, centerY, centerX + separator.GetCeilingIntX(), centerY + separator.GetCeilingIntY(), g_MaskColor);
			line(m_BGBitmap, centerX + 1, centerY, centerX + 1 + separator.GetCeilingIntX(), centerY + separator.GetCeilingIntY(), g_MaskColor);
			line(m_BGBitmap, centerX, centerY + 1, centerX + separator.GetCeilingIntX(), centerY + 1 + separator.GetCeilingIntY(), g_MaskColor);
			line(m_BGBitmap, centerX + 1, centerY + 1, centerX + 1 + separator.GetCeilingIntX(), centerY + 1 + separator.GetCeilingIntY(), g_MaskColor);
		}

		if (m_HoveredSlice && m_HoveredSlice->IsEnabled()) {
			separator.SetXY(static_cast<float>(m_InnerRadius + (m_Thickness / 2)), 0.0F);
			separator.RadRotate(m_HoveredSlice->GetMidAngle());
			floodfill(m_BGBitmap, centerX + separator.GetFloorIntX(), centerY + separator.GetFloorIntY(), g_BlackColor);
		}
	}

	m_RedrawBG = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//TODO Need to investigate this stuff, I don't fully understand what it does and maybe it's mostly undesirable since we'll want to be able to draw pie menus partly off the screen
void PieMenuGUI::CalculateDrawPositionAccountingForSeamsAndFont(const BITMAP *targetBitmap, const Vector &targetPos, GUIFont *menuFont, Vector &drawPos) const {
	drawPos = m_CenterPos - targetPos;
	if (!targetPos.IsZero()) {
		const Box *nearestBox = nullptr;

		Box screenBox(targetPos, static_cast<float>(targetBitmap->w), static_cast<float>(targetBitmap->h));
		list<Box> wrappedBoxes;
		bool withinAnyBox = false;
		float distance, shortestDist = 1000000.0F;
		//TODO under what conditions would the pie menu not be on the screen and, if that's the case, would we still want to draw it? Investigate this!
		g_SceneMan.WrapBox(screenBox, wrappedBoxes);
		for (const Box &wrappedBox : wrappedBoxes) {
			if (wrappedBox.IsWithinBox(m_CenterPos)) {
				nearestBox = &wrappedBox;
				withinAnyBox = true;
				break;
			}

			distance = g_SceneMan.ShortestDistance(wrappedBox.GetCenter(), m_CenterPos).GetLargest();
			if (distance < shortestDist) {
				shortestDist = distance;
				nearestBox = &wrappedBox;
			}
		}
		drawPos += screenBox.GetCorner() - nearestBox->GetCorner();

		if (!withinAnyBox) {
			drawPos = nearestBox->GetCenter() + g_SceneMan.ShortestDistance(nearestBox->GetCenter(), m_CenterPos) - targetPos;
		}
	}

	//TODO judging by the comment this will force it onto the screen if it's not on it? Better behaviour would be only draw if it's on the screen. See above TODO as well.
	// Adjust the draw position so that the menu will always be drawn fully inside the player's screen
	int menuDrawRadius = m_InnerRadius + m_Thickness + 2 + menuFont->GetFontHeight();
	if (drawPos.m_X - static_cast<float>(menuDrawRadius) < 0.0F) {
		drawPos.m_X = static_cast<float>(menuDrawRadius);
	} else if (drawPos.m_X + static_cast<float>(menuDrawRadius) > static_cast<float>(targetBitmap->w)) {
		drawPos.m_X = static_cast<float>(targetBitmap->w - menuDrawRadius);
	}
	if (drawPos.m_Y - static_cast<float>(menuDrawRadius) < 0.0F) {
		drawPos.m_Y = static_cast<float>(menuDrawRadius);
	} else if (drawPos.m_Y + static_cast<float>(menuDrawRadius) > static_cast<float>(targetBitmap->h)) {
		drawPos.m_Y = static_cast<float>(targetBitmap->h - menuDrawRadius);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PieMenuGUI::DrawPieIcons(BITMAP *targetBitmap, const Vector &drawPos) const {
	BITMAP *sliceIcon;
	BITMAP **sliceFrames;
	int sliceFrameCount;
	Vector sliceIconOffset;

	for (const PieSlice *slice : m_CurrentSlices) {
		sliceFrames = slice->GetIcon().GetBitmaps8();
		sliceFrameCount = slice->GetIcon().GetFrameCount();

		if (sliceFrames && sliceFrameCount > 0) {
			sliceIcon = sliceFrames[PIS_NORMAL];
			if (!slice->IsEnabled() && sliceFrameCount > PIS_DISABLED) {
				sliceIcon = sliceFrames[PIS_DISABLED];
			} else if (slice == m_HoveredSlice && sliceFrameCount > PIS_SELECTED) {
				sliceIcon = sliceFrames[PIS_SELECTED];
			}

			sliceIconOffset = Vector(static_cast<float>(m_InnerRadius + (m_Thickness / 2)), 0.0F).RadRotate(slice->GetMidAngle()) + Vector(1.0F - static_cast<float>(sliceIcon->w / 2), 1.0F - static_cast<float>(sliceIcon->h / 2));
			draw_sprite(targetBitmap, sliceIcon, drawPos.GetFloorIntX() + sliceIconOffset.GetFloorIntX(), drawPos.GetFloorIntY() + sliceIconOffset.GetFloorIntY());
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PieMenuGUI::DrawPieCursorAndSliceDescriptions(BITMAP *targetBitmap, const Vector &drawPos, GUIFont *menuFont) const {
	Vector cursorPos = Vector(static_cast<float>(m_InnerRadius)/* - s_Cursor->w*/, 0.0F).RadRotate(m_CursorAngle);
	pivot_sprite(targetBitmap, s_Cursor, drawPos.GetFloorIntX() + cursorPos.GetFloorIntX(), drawPos.GetFloorIntY() + cursorPos.GetFloorIntY(), s_Cursor->w / 2, s_Cursor->h / 2, ftofix((m_CursorAngle / c_PI) * -128));

	// Align text center, left or right respectively, based on which side of the menu the hovered slice is on.
	AllegroBitmap allegroBitmap(targetBitmap);
	Vector textPos = Vector(static_cast<float>(m_InnerRadius + m_Thickness + menuFont->GetFontHeight()) * 0.5F, 0.0F).RadRotate(m_HoveredSlice->GetMidAngle()) - Vector(0.0F, static_cast<float>(menuFont->GetFontHeight()) * 0.45F);
	if (m_HoveredSlice == &m_UpSlice || m_HoveredSlice == &m_DownSlice) {
		menuFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() + textPos.GetFloorIntX(), drawPos.GetFloorIntY() + textPos.GetFloorIntY(), m_HoveredSlice->GetDescription().c_str(), GUIFont::Centre);
	} else if (m_CursorAngle < c_HalfPI || m_CursorAngle > c_PI + c_HalfPI) {
		menuFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() + textPos.GetFloorIntX(), drawPos.GetFloorIntY() + textPos.GetFloorIntY(), m_HoveredSlice->GetDescription().c_str(), GUIFont::Left);
	} else {
		menuFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() + textPos.GetFloorIntX(), drawPos.GetFloorIntY() + textPos.GetFloorIntY(), m_HoveredSlice->GetDescription().c_str(), GUIFont::Right);
	}
}
