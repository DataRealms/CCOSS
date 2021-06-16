#include "PieMenuGUI.h"

#include "FrameMan.h"
#include "UInputMan.h"

#include "Controller.h"
#include "AHuman.h"
#include "ContentFile.h"

#include "GUI.h"
#include "GUIFont.h"
#include "AllegroBitmap.h"

namespace RTE {

	BITMAP *PieMenuGUI::s_CursorBitmap;
	std::unordered_map<std::string, PieSlice> PieMenuGUI::s_AllCustomLuaSlices;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::Clear() {
		m_LargeFont = nullptr;

		m_MenuController = nullptr;
		m_AffectedObject = nullptr;
		m_MenuMode = MenuMode::Normal;
		m_CenterPos.Reset();

		m_EnabledState = EnabledState::Disabled;
		m_EnableDisableAnimationTimer.Reset();
		m_HoverTimer.Reset();

		m_IconSeparatorMode = IconSeparatorMode::Line;
		m_BackgroundThickness = 16;
		m_BackgroundSeparatorSize = 2;
		m_DrawBackgroundTransparent = true;
		m_BackgroundColor = 4;
		m_BackgroundBorderColor = g_MaskColor;
		m_SelectedItemBackgroundColor = g_BlackColor;

		ResetSlices();

		m_InnerRadius = 0;
		m_CursorAngle = 0;

		m_BGBitmap = nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PieMenuGUI::Create(Controller *controller, MovableObject *affectedObject) {
		RTEAssert(controller, "No controller sent to PieMenuGUI on creation!");

		if (!s_CursorBitmap) { s_CursorBitmap = ContentFile("Base.rte/GUIs/Skins/PieCursor.png").GetAsBitmap(); }

		if (!m_LargeFont) { m_LargeFont = g_FrameMan.GetLargeFont(); }

		m_MenuController = controller;
		m_AffectedObject = affectedObject;
		if (m_AffectedObject) {
			const Actor *affectedObjectAsActor = static_cast<Actor *>(m_AffectedObject);
			m_CenterPos = (affectedObjectAsActor ? affectedObjectAsActor->GetCPUPos() : m_AffectedObject->GetPos());
		}

		if (!m_BGBitmap) {
			int diameter = (c_FullRadius + std::max(m_BackgroundThickness, m_BackgroundSeparatorSize) + 2) * 2;
			m_BGBitmap = create_bitmap_ex(8, diameter, diameter);
			clear_to_color(m_BGBitmap, g_MaskColor);
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::Destroy() {
		destroy_bitmap(m_BGBitmap);

		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::SetEnabled(bool enable) {
		m_MenuMode = MenuMode::Normal;

		bool enabledStateDoesNotMatchInput = (enable && m_EnabledState != EnabledState::Enabled && m_EnabledState != EnabledState::Enabling) || (!enable && m_EnabledState != EnabledState::Disabled && m_EnabledState != EnabledState::Disabling);
		if (enabledStateDoesNotMatchInput) {
			m_EnabledState = enable ? EnabledState::Enabling : EnabledState::Disabling;
			m_EnableDisableAnimationTimer.Reset();
			if (m_MenuController->IsMouseControlled()) {
				g_UInputMan.SetMouseValueMagnitude(0);
				m_MenuController->m_AnalogCursor = Vector();
			}
			SoundContainer *soundToPlay = enable ? g_GUISound.PieMenuEnterSound() : g_GUISound.PieMenuExitSound();
			soundToPlay->Play();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::ResetSlices() {
		m_HoveredSlice = nullptr;
		m_ActivatedSlice = nullptr;
		m_AlreadyActivatedSlice = nullptr;

		m_UpSlice.Reset();
		m_LeftSlice.Reset();
		m_DownSlice.Reset();
		m_RightSlice.Reset();

		m_UpRightSlices.clear();
		m_UpLeftSlices.clear();
		m_DownLeftSlices.clear();
		m_DownRightSlices.clear();

		m_CurrentSlices.clear();
		m_SliceGroupCount = 0;
		m_BGBitmapNeedsRedrawing = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PieMenuGUI::AddSlice(PieSlice &newPieSlice, bool takeAnyFreeCardinal) {
		if (!newPieSlice.HasIcon()) { newPieSlice.Create(); }

		std::unordered_map<PieSlice::SliceDirection, PieSlice *> sliceCardinalDirections = {
			{PieSlice::SliceDirection::UP, &m_UpSlice}, {PieSlice::SliceDirection::LEFT, &m_LeftSlice}, {PieSlice::SliceDirection::DOWN, &m_DownSlice}, {PieSlice::SliceDirection::RIGHT, &m_RightSlice}
		};
		std::unordered_map<PieSlice::SliceDirection, std::pair<std::list<PieSlice> *, std::list<PieSlice> *>> sliceIntercardinalDirections = {
			{PieSlice::SliceDirection::UP, {&m_UpRightSlices, &m_UpLeftSlices}},
			{PieSlice::SliceDirection::LEFT, {&m_UpLeftSlices, &m_DownLeftSlices}},
			{PieSlice::SliceDirection::DOWN, {&m_DownLeftSlices, &m_DownRightSlices}},
			{PieSlice::SliceDirection::RIGHT, {&m_UpRightSlices, &m_DownRightSlices}}
		};

		for (const auto &[sliceDirection, pieSlice] : sliceCardinalDirections) {
			if ((takeAnyFreeCardinal || sliceDirection == newPieSlice.GetDirection()) && pieSlice->GetType() == PieSlice::PieSliceIndex::PSI_NONE) {
				*pieSlice = newPieSlice;
				return true;
			}
		}

		const auto &[firstPieSliceList, secondPieSliceList] = sliceIntercardinalDirections.at(newPieSlice.GetDirection());
		if (firstPieSliceList->size() <= secondPieSliceList->size()) {
			firstPieSliceList->emplace_back(newPieSlice);
		} else {
			secondPieSliceList->emplace_back(newPieSlice);
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
			m_CurrentSlices.emplace_back(&pieSlice);
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

			m_CurrentSlices.emplace_back(cardinalSlices[i]);

			currentAngleOffset += c_HalfPI;
		}

		for (PieSlice *slice : m_CurrentSlices) {
			slice->SetMidAngle(slice->GetAreaStart() + (slice->GetAreaArc() / 2));
		}

		m_BGBitmapNeedsRedrawing = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PieMenuGUI::AddPieSliceLua(const std::string &description, const std::string &functionName, PieSlice::SliceDirection direction, bool isEnabled) {
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

	void PieMenuGUI::AlterPieSliceLua(const std::string &description, const std::string &functionName, PieSlice::SliceDirection direction, bool isEnabled) {
		std::vector<PieSlice *>::iterator sliceIterator = std::find_if(m_CurrentSlices.begin(), m_CurrentSlices.end(), [&description, &functionName](const PieSlice *slice) {
			return slice->GetDescription() == description && slice->GetFunctionName() == functionName;
		});

		PieSlice foundSlice;
		if (sliceIterator != m_CurrentSlices.end() && (*sliceIterator)->GetType() != PieSlice::PieSliceIndex::PSI_NONE) {
			foundSlice.SetDirection(direction);
			foundSlice.SetEnabled(isEnabled);

			RemovePieSliceLua(description, functionName);
			AddSlice(foundSlice);

			RealignSlices();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PieSlice PieMenuGUI::RemovePieSliceLua(const std::string &description, const std::string &functionName) {
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

		if (m_MenuMode == MenuMode::Wobble) {
			UpdateWobbling();
		} else if (m_MenuMode == MenuMode::Freeze) {
			m_EnabledState = EnabledState::Enabling;
		} else if (m_EnabledState == EnabledState::Enabling || m_EnabledState == EnabledState::Disabling) {
			UpdateEnablingAndDisablingProgress();
		}

		if (m_MenuController->IsDisabled()) {
			return;
		}

		if (m_AffectedObject && !g_MovableMan.ValidMO(m_AffectedObject)) { m_AffectedObject = nullptr; }

		if (IsEnabled() && (HandleMouseInput() || HandleNonMouseInput())) { m_HoverTimer.Reset(); }

		if (m_HoveredSlice && m_EnabledState != EnabledState::Disabled) {
			UpdateSliceActivation();

			if (m_HoverTimer.IsPastSimMS(1500)) {
				m_HoveredSlice = nullptr;
				m_BGBitmapNeedsRedrawing = true;
			}
		}

		if (m_BGBitmapNeedsRedrawing && m_EnabledState != EnabledState::Disabled) { UpdatePredrawnMenuBackgroundBitmap(); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::Draw(BITMAP *targetBitmap, const Vector &targetPos) const {
		Vector drawPos;
		CalculateDrawPosition(targetBitmap, targetPos, drawPos);

		if (m_EnabledState != EnabledState::Disabled) {
			if (m_DrawBackgroundTransparent && !g_FrameMan.IsInMultiplayerMode()) {
				g_FrameMan.SetTransTable(MoreTrans);
				draw_trans_sprite(targetBitmap, m_BGBitmap, drawPos.GetFloorIntX() - m_BGBitmap->w / 2, drawPos.GetFloorIntY() - m_BGBitmap->h / 2);
			} else {
				draw_sprite(targetBitmap, m_BGBitmap, drawPos.GetFloorIntX() - m_BGBitmap->w / 2, drawPos.GetFloorIntY() - m_BGBitmap->h / 2);
			}
		}

		if (m_EnabledState == EnabledState::Enabled) {
			DrawPieIcons(targetBitmap, drawPos);
			if (m_HoveredSlice) { DrawPieCursorAndSliceDescriptions(targetBitmap, drawPos); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::UpdateWobbling() {
		float innerRadiusChange = static_cast<float>(m_EnableDisableAnimationTimer.GetElapsedRealTimeMS()) / 6.0F;

		m_BGBitmapNeedsRedrawing = true;
		m_InnerRadius += static_cast<int>(innerRadiusChange) * (m_EnabledState == EnabledState::Disabling ? -1 : 1);

		if (m_InnerRadius < 0) {
			m_EnabledState = EnabledState::Enabling;
		} else if (m_InnerRadius > c_FullRadius / 2) {
			m_EnabledState = EnabledState::Disabling;
		}

		m_InnerRadius = std::clamp(m_InnerRadius, 0, c_FullRadius / 2);
		m_EnableDisableAnimationTimer.Reset();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::UpdateEnablingAndDisablingProgress() {
		m_BGBitmapNeedsRedrawing = true;
		if (m_EnabledState == EnabledState::Enabling) {
			m_InnerRadius = static_cast<int>(LERP(0.0F, static_cast<float>(c_EnablingDelay), 0.0F, static_cast<float>(c_FullRadius), static_cast<float>(m_EnableDisableAnimationTimer.GetElapsedRealTimeMS())));
			if (m_EnableDisableAnimationTimer.IsPastRealMS(c_EnablingDelay)) {
				m_EnabledState = EnabledState::Enabled;
				m_InnerRadius = c_FullRadius;
			}
		} else if (m_EnabledState == EnabledState::Disabling) {
			m_InnerRadius = static_cast<int>(LERP(0.0F, static_cast<float>(c_EnablingDelay), static_cast<float>(c_FullRadius), 0.0F, static_cast<float>(m_EnableDisableAnimationTimer.GetElapsedRealTimeMS())));
			if (m_EnableDisableAnimationTimer.IsPastRealMS(c_EnablingDelay)) {
				m_EnabledState = EnabledState::Disabled;
				m_InnerRadius = 0;
				if (Actor *affectedObjectAsActor = dynamic_cast<Actor *>(m_AffectedObject)) { affectedObjectAsActor->FlashWhite(); }
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PieMenuGUI::HandleMouseInput() {
		if (float cursorDistance = m_MenuController->GetAnalogCursor().GetLargest(); cursorDistance > 0.45F) {
			m_CursorAngle = m_MenuController->GetAnalogCursor().GetAbsRadAngle();

			while (m_CursorAngle < 0) { m_CursorAngle += c_TwoPI; }

			float pieSliceAreaEnd;
			for (const PieSlice *pieSlice : m_CurrentSlices) {
				pieSliceAreaEnd = pieSlice->GetAreaStart() + pieSlice->GetAreaArc();
				if ((pieSlice->GetAreaStart() <= m_CursorAngle && pieSliceAreaEnd > m_CursorAngle) || (pieSliceAreaEnd > c_TwoPI && m_CursorAngle >= 0 && m_CursorAngle < (pieSliceAreaEnd - c_TwoPI))) {
					SelectPieSlice(pieSlice);
					break;
				}
			}
			return true;
		} else if (cursorDistance > 0.35 && cursorDistance <= 0.45) {
			m_HoveredSlice = nullptr;
			m_BGBitmapNeedsRedrawing = true;
			return false;
		}

		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PieMenuGUI::HandleNonMouseInput() {
		std::map<ControlState, PieSlice *> controlStateSlices = {
			{ControlState::PRESS_UP, &m_UpSlice},
			{ControlState::PRESS_LEFT, &m_LeftSlice},
			{ControlState::PRESS_DOWN, &m_DownSlice},
			{ControlState::PRESS_RIGHT, &m_RightSlice}
		};
		std::map<ControlState, std::pair<std::pair<float, float>, bool>> controlStateSelectionZoomAngles = {
			{ControlState::PRESS_UP, {{m_LeftSlice.GetMidAngle(), m_RightSlice.GetMidAngle()}, true}},
			{ControlState::PRESS_LEFT, {{m_DownSlice.GetMidAngle(), m_UpSlice.GetMidAngle()}, false}},
			{ControlState::PRESS_DOWN, {{m_RightSlice.GetMidAngle(), m_LeftSlice.GetMidAngle()}, false}},
			{ControlState::PRESS_RIGHT, {{m_UpSlice.GetMidAngle(), m_DownSlice.GetMidAngle()}, true}}
		};
		std::map<ControlState, std::pair<std::pair<float, float>, bool>> controlStateSelectionStepAngles = {
			{ControlState::PRESS_UP, {{m_UpSlice.GetMidAngle(), m_DownSlice.GetMidAngle()}, true}},
			{ControlState::PRESS_LEFT, {{m_LeftSlice.GetMidAngle(), 0}, false}},
			{ControlState::PRESS_DOWN, {{m_DownSlice.GetMidAngle(), m_UpSlice.GetMidAngle()}, false}},
			{ControlState::PRESS_RIGHT, {{0, m_LeftSlice.GetMidAngle()}, true}}
		};

		for (const auto &[controlState, pieSlice] : controlStateSlices) {
			if (m_MenuController->IsState(controlState)) {
				const auto &[firstSelectionZoomAngle, secondSelectionZoomAngle] = controlStateSelectionZoomAngles.at(controlState).first;
				bool shouldZoom = !m_HoveredSlice;
				shouldZoom |= controlStateSelectionZoomAngles.at(controlState).second ? (m_CursorAngle > firstSelectionZoomAngle && m_CursorAngle < secondSelectionZoomAngle) : (m_CursorAngle > firstSelectionZoomAngle || m_CursorAngle < secondSelectionZoomAngle);

				if (shouldZoom) {
					return SelectPieSlice(pieSlice, true);
				} else if (m_HoveredSlice != pieSlice) {
					const auto &[firstSelectionStepAngle, secondSelectionStepAngle] = controlStateSelectionStepAngles.at(controlState).first;
					int stepDirection = controlStateSelectionStepAngles.at(controlState).second ?
						((m_CursorAngle > firstSelectionStepAngle && m_CursorAngle < secondSelectionStepAngle) ? -1 : 1) :
						((m_CursorAngle > firstSelectionStepAngle || m_CursorAngle < secondSelectionStepAngle) ? -1 : 1);

					std::vector<PieSlice *>::iterator sliceToSelect = std::find(m_CurrentSlices.begin(), m_CurrentSlices.end(), m_HoveredSlice) + stepDirection;
					if (sliceToSelect >= m_CurrentSlices.end()) {
						sliceToSelect = m_CurrentSlices.begin();
					} else if (sliceToSelect < m_CurrentSlices.begin()) {
						sliceToSelect = m_CurrentSlices.end() - 1;
					}

					if ((*sliceToSelect) != nullptr) {
						return SelectPieSlice((*sliceToSelect), true);
					}
				}
			}
		}

		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::UpdateSliceActivation() {
		if (m_MenuController->IsState(PRESS_PRIMARY) || (m_HoveredSlice != m_AlreadyActivatedSlice && m_MenuController->IsState(RELEASE_SECONDARY))) {
			m_HoverTimer.Reset();
			m_ActivatedSlice = m_HoveredSlice->IsEnabled() ? m_HoveredSlice : m_ActivatedSlice;
			m_AlreadyActivatedSlice = m_ActivatedSlice;

			SoundContainer *soundToPlay = m_HoveredSlice->IsEnabled() ? g_GUISound.SlicePickedSound() : g_GUISound.DisabledPickedSound();
			soundToPlay->Play();
		}

		if (GetPieCommand() == PieSlice::PieSliceIndex::PSI_SCRIPTED) {
			// TODO: Investigate reloading the file each time. I think it's needed cause this stuff isn't in PresetMan, so this is the only way for it to be reloadable. To test, have script on slice, edit it and see what happens with and without this. Also, make this support not having an actor I guess.
			g_LuaMan.RunScriptFile(m_ActivatedSlice->GetScriptPath());

			std::string functionString = m_ActivatedSlice->GetFunctionName() + "(";
			if (m_AffectedObject) {
				g_LuaMan.SetTempEntity(m_AffectedObject);
				const Actor *affectedObjectAsActor = static_cast<Actor *>(m_AffectedObject);
				functionString += affectedObjectAsActor ? "ToActor(" : "";
				functionString += "LuaMan.TempEntity";
				functionString += affectedObjectAsActor ? ")" : "";
			}
			functionString += ");";
			g_LuaMan.RunScriptString(functionString);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::UpdatePredrawnMenuBackgroundBitmap() {
		int centerX = m_BGBitmap->w / 2;
		int centerY = m_BGBitmap->h / 2;

		/// <summary>
		/// Lambda for drawing separators when in Line IconSeparatorMode.
		/// It draws four separator lines between each PieSlice so the resulting separation will be at least 2 pixels thick, regardless of the PieSlice's angles.
		/// </summary>
		auto DrawLineIconSeparator = [this, &centerX, &centerY](const PieSlice *slice, Vector &separator) {
			separator.SetXY(static_cast<float>(m_InnerRadius + m_BackgroundThickness + m_BackgroundSeparatorSize), 0).RadRotate(slice->GetAreaStart());
			line(m_BGBitmap, centerX, centerY, centerX + separator.GetCeilingIntX(), centerY + separator.GetCeilingIntY(), m_BackgroundBorderColor);
			line(m_BGBitmap, centerX + 1, centerY, centerX + 1 + separator.GetCeilingIntX(), centerY + separator.GetCeilingIntY(), m_BackgroundBorderColor);
			line(m_BGBitmap, centerX, centerY + 1, centerX + separator.GetCeilingIntX(), centerY + 1 + separator.GetCeilingIntY(), m_BackgroundBorderColor);
			line(m_BGBitmap, centerX + 1, centerY + 1, centerX + 1 + separator.GetCeilingIntX(), centerY + 1 + separator.GetCeilingIntY(), m_BackgroundBorderColor);
		};

		/// <summary>
		/// Lambda for drawing separators when in Circle IconSeparatorMode. It draws background circles for each PieSlice.
		/// </summary>
		auto DrawCircleIconSeparator = [this, &centerX, &centerY](const PieSlice *slice, Vector &positionOffset) {
			positionOffset = Vector(static_cast<float>(m_InnerRadius) + (static_cast<float>(m_BackgroundThickness) / 2.0F), 0).RadRotate(slice->GetMidAngle());
			circlefill(m_BGBitmap, centerX + positionOffset.GetFloorIntX(), centerY + positionOffset.GetFloorIntY(), m_BackgroundSeparatorSize, m_BackgroundBorderColor);
			circlefill(m_BGBitmap, centerX + positionOffset.GetFloorIntX(), centerY + positionOffset.GetFloorIntY(), m_BackgroundSeparatorSize - 1, slice == m_HoveredSlice && slice->IsEnabled() ? m_SelectedItemBackgroundColor : m_BackgroundColor);
		};

		clear_to_color(m_BGBitmap, g_MaskColor);
		circlefill(m_BGBitmap, centerX, centerY, m_InnerRadius + m_BackgroundThickness, m_BackgroundColor);
		circlefill(m_BGBitmap, centerX, centerY, m_InnerRadius, g_MaskColor);

		if (m_EnabledState == EnabledState::Enabled) {
			Vector separatorVector;
			switch (m_IconSeparatorMode) {
				case IconSeparatorMode::Line:
					for (const PieSlice *slice : m_CurrentSlices) { DrawLineIconSeparator(slice, separatorVector); }

					if (m_HoveredSlice && m_HoveredSlice->IsEnabled()) {
						separatorVector.SetXY(static_cast<float>(m_InnerRadius + (m_BackgroundThickness / 2)), 0.0F).RadRotate(m_HoveredSlice->GetMidAngle());
						floodfill(m_BGBitmap, centerX + separatorVector.GetFloorIntX(), centerY + separatorVector.GetFloorIntY(), m_SelectedItemBackgroundColor);
					}
					break;
				case IconSeparatorMode::Circle:
					for (const PieSlice *slice : m_CurrentSlices) {
						if (slice->GetType() != PieSlice::PieSliceIndex::PSI_NONE) { DrawCircleIconSeparator(slice, separatorVector); }
					}
					break;
				default:
					RTEAbort("Invalid icon separator mode in PieMenuGUI.");
			}
		}

		m_BGBitmapNeedsRedrawing = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//TODO Need to investigate this stuff, I don't fully understand what it does and maybe it's mostly undesirable since we'll want to be able to draw pie menus partly off the screen
	void PieMenuGUI::CalculateDrawPosition(const BITMAP *targetBitmap, const Vector &targetPos, Vector &drawPos) const {
		drawPos = m_CenterPos - targetPos;
		if (!targetPos.IsZero()) {
			const Box *nearestBox = nullptr;

			Box screenBox(targetPos, static_cast<float>(targetBitmap->w), static_cast<float>(targetBitmap->h));
			std::list<Box> wrappedBoxes;
			bool withinAnyBox = false;
			float distance = std::numeric_limits<float>::max();
			float shortestDist = std::numeric_limits<float>::max();
			//TODO under what conditions would the pie menu not be on the screen and, if that's the case, would we still want to draw it? Try to remove this in next pass of PieMenu changes, or replace it with more standard wrapping handling.
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

		// Adjust the draw position so that the menu will always be drawn fully inside the player's screen
		int menuDrawRadius = m_InnerRadius + m_BackgroundThickness + 2 + m_LargeFont->GetFontHeight();
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
		Vector sliceIconOffset;

		for (const PieSlice *slice : m_CurrentSlices) {
			sliceIcon = slice->GetAppropriateIcon(slice == m_HoveredSlice);

			if (sliceIcon) {
				sliceIconOffset = Vector(static_cast<float>(m_InnerRadius + (m_BackgroundThickness / 2)), 0).RadRotate(slice->GetMidAngle()) + Vector(1.0F - static_cast<float>(sliceIcon->w / 2), 1.0F - static_cast<float>(sliceIcon->h / 2));
				draw_sprite(targetBitmap, sliceIcon, drawPos.GetFloorIntX() + sliceIconOffset.GetFloorIntX(), drawPos.GetFloorIntY() + sliceIconOffset.GetFloorIntY());
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::DrawPieCursorAndSliceDescriptions(BITMAP *targetBitmap, const Vector &drawPos) const {
		Vector cursorPos = Vector(static_cast<float>(m_InnerRadius)/* - s_CursorBitmap->w*/, 0.0F).RadRotate(m_CursorAngle);
		pivot_sprite(targetBitmap, s_CursorBitmap, drawPos.GetFloorIntX() + cursorPos.GetFloorIntX(), drawPos.GetFloorIntY() + cursorPos.GetFloorIntY(), s_CursorBitmap->w / 2, s_CursorBitmap->h / 2, ftofix((m_CursorAngle / c_PI) * -128));

		// Align text center, left or right respectively, based on which side of the menu the hovered slice is on.
		AllegroBitmap allegroBitmap(targetBitmap);
		Vector textPos = Vector(static_cast<float>(m_InnerRadius + m_BackgroundThickness + m_LargeFont->GetFontHeight()) * 0.5F, 0.0F).RadRotate(m_HoveredSlice->GetMidAngle()) - Vector(0.0F, static_cast<float>(m_LargeFont->GetFontHeight()) * 0.45F);
		if (m_HoveredSlice == &m_UpSlice || m_HoveredSlice == &m_DownSlice) {
			m_LargeFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() + textPos.GetFloorIntX(), drawPos.GetFloorIntY() + textPos.GetFloorIntY(), m_HoveredSlice->GetDescription().c_str(), GUIFont::Centre);
		} else if (m_CursorAngle < c_HalfPI || m_CursorAngle > c_PI + c_HalfPI) {
			m_LargeFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() + textPos.GetFloorIntX(), drawPos.GetFloorIntY() + textPos.GetFloorIntY(), m_HoveredSlice->GetDescription().c_str(), GUIFont::Left);
		} else {
			m_LargeFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() + textPos.GetFloorIntX(), drawPos.GetFloorIntY() + textPos.GetFloorIntY(), m_HoveredSlice->GetDescription().c_str(), GUIFont::Right);
		}
	}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PieMenuGUI::SelectPieSlice(const PieSlice *pieSliceToSelect, bool moveCursorToSlice) {
		if (pieSliceToSelect == nullptr || pieSliceToSelect == m_HoveredSlice) {
			return false;
		}

		m_HoveredSlice = pieSliceToSelect;

		m_BGBitmapNeedsRedrawing = true;
		if (moveCursorToSlice) { m_CursorAngle = m_HoveredSlice->GetMidAngle(); }

		SoundContainer *soundToPlay = pieSliceToSelect->IsEnabled() ? g_GUISound.HoverChangeSound() : g_GUISound.HoverDisabledSound();
		soundToPlay->Play();
		return true;
	}
}
