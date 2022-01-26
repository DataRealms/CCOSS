#include "PieMenuGUI.h"

#include "FrameMan.h"
#include "UInputMan.h"
#include "PresetMan.h"

#include "Controller.h"
#include "AHuman.h"
#include "ContentFile.h"

#include "GUI.h"
#include "GUIFont.h"
#include "AllegroBitmap.h"

namespace RTE {

	ConcreteClassInfo(PieMenuGUI, Entity, 20)

	BITMAP *PieMenuGUI::s_CursorBitmap;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::PieQuadrant::Reset() {
		Enabled = false;
		Direction = Directions::Right;
		MiddlePieSlice.reset();
		for (std::unique_ptr<PieSlice> &pieSlice : LeftPieSlices) { pieSlice.reset(); }
		for (std::unique_ptr<PieSlice> &pieSlice : RightPieSlices) { pieSlice.reset(); }
		SlotsForPieSlices.fill(nullptr);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::PieQuadrant::Create(const PieQuadrant &reference) {
		Enabled = reference.Enabled;
		Direction = reference.Direction;
		for (const PieSlice *pieSlice : reference.GetFlattenedSlices()) { AddPieSlice(dynamic_cast<PieSlice *>(pieSlice->Clone())); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector<PieSlice *> PieMenuGUI::PieQuadrant::GetFlattenedSlices(bool inCCWOrder) const {
		std::vector<PieSlice *> pieSlices;
		if (inCCWOrder) {
			for (auto pieSliceReverseIterator = RightPieSlices.crbegin(); pieSliceReverseIterator < RightPieSlices.crend(); pieSliceReverseIterator++) {
				if (*pieSliceReverseIterator) { pieSlices.emplace_back((*pieSliceReverseIterator).get()); }
			}
			if (MiddlePieSlice) { pieSlices.emplace_back(MiddlePieSlice.get()); }
			for (const std::unique_ptr<PieSlice> &pieSlice : LeftPieSlices) {
				if (pieSlice) { pieSlices.emplace_back(pieSlice.get()); }
			}
		} else {
			if (MiddlePieSlice) { pieSlices.emplace_back(MiddlePieSlice.get()); }
			const std::array<std::unique_ptr<PieSlice>, 2> &firstSliceArray = Direction == Directions::Left ? RightPieSlices : LeftPieSlices;
			const std::array<std::unique_ptr<PieSlice>, 2> &secondSliceArray = Direction == Directions::Left ? LeftPieSlices : RightPieSlices;
			for (int i = 0; i < firstSliceArray.size(); i++) {
				if (firstSliceArray.at(i)) { pieSlices.push_back(firstSliceArray.at(i).get()); }
				if (secondSliceArray.at(i)) { pieSlices.push_back(secondSliceArray.at(i).get()); }
			}
		}
		return pieSlices;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::PieQuadrant::RealignSlices() {
		SlotsForPieSlices.fill(nullptr);

		std::vector<PieSlice *> pieSlices = GetFlattenedSlices(true);
		int oddRoundedSliceCount = 2 * static_cast<int>(pieSlices.size() / 2) + 1;
		float angleOffset = NormalizeAngleBetween0And2PI(c_DirectionsToRadiansMap.at(Direction) - c_QuarterPI);
		
		int currentSlot = 0;
		for (PieSlice *pieSlice : pieSlices) {
			int sliceSlotCount = 1;
			if (oddRoundedSliceCount < c_QuadrantSlotCount && pieSlice == MiddlePieSlice.get()) { sliceSlotCount = oddRoundedSliceCount == 1 ? c_QuadrantSlotCount : c_QuadrantSlotCount - 2; }
			if ((currentSlot == 0 || SlotsForPieSlices.at(currentSlot - 1) == nullptr) && pieSlices.size() % 2 == 0) {
				currentSlot++;
				angleOffset += c_PieSliceSlotSize;
			}

			pieSlice->SetStartAngle(angleOffset);
			pieSlice->SetSlotCount(sliceSlotCount);

			for (int slot = currentSlot; slot < currentSlot + sliceSlotCount; slot++) {
				SlotsForPieSlices.at(slot) = pieSlice;
			}
			currentSlot += sliceSlotCount;
			angleOffset += c_PieSliceSlotSize * static_cast<float>(pieSlice->GetSlotCount());
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PieMenuGUI::PieQuadrant::AddPieSlice(PieSlice *pieSliceToAdd) {
		if (!pieSliceToAdd) {
			return false;
		}
		bool sliceWasAdded = false;
		if (!MiddlePieSlice && pieSliceToAdd->GetCanBeMiddleSlice()) {
			MiddlePieSlice = std::unique_ptr<PieSlice>(pieSliceToAdd);
			sliceWasAdded = true;
		}
		if (!sliceWasAdded && (!LeftPieSlices.at(1) || !RightPieSlices.at(1))) {
			bool bothSidesEqual = (!LeftPieSlices.at(0) && !RightPieSlices.at(0)) || (LeftPieSlices.at(0) && RightPieSlices.at(0)) || (LeftPieSlices.at(1) && RightPieSlices.at(1));
			bool leftSideHasMoreSlices = !bothSidesEqual && ((LeftPieSlices.at(0) && !RightPieSlices.at(0)) || LeftPieSlices.at(1));

			std::array<std::unique_ptr<PieSlice>, 2> &sliceArrayToAddTo = leftSideHasMoreSlices ? RightPieSlices : LeftPieSlices;
			sliceArrayToAddTo.at(sliceArrayToAddTo.at(0) ? 1 : 0) = std::unique_ptr<PieSlice>(pieSliceToAdd);
			sliceWasAdded = true;
		}

		return sliceWasAdded;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::PieQuadrant::RemovePieSlicesByOriginalSource(const Entity *originalSource) {
		if (MiddlePieSlice && MiddlePieSlice->GetOriginalSource() == originalSource) { MiddlePieSlice = nullptr; }
		for (std::unique_ptr<PieSlice> &pieSlice : LeftPieSlices) {
			if (pieSlice && pieSlice->GetOriginalSource() == originalSource) { pieSlice = nullptr; }
		}
		for (std::unique_ptr<PieSlice> &pieSlice : RightPieSlices) {
			if (pieSlice && pieSlice->GetOriginalSource() == originalSource) { pieSlice = nullptr; }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::Clear() {
		m_LargeFont = nullptr;

		m_Owner = nullptr;
		m_MenuController = nullptr;
		m_AffectedObject = nullptr;
		m_MenuMode = MenuMode::Normal;
		m_CenterPos.Reset();
		m_Rotation.Reset();

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
		m_CursorInVisiblePosition = false;
		m_CursorAngle = 0;

		m_BGBitmap = nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PieMenuGUI::Create() {
		if (!s_CursorBitmap) { s_CursorBitmap = ContentFile("Base.rte/GUIs/Skins/PieCursor.png").GetAsBitmap(); }

		if (!m_LargeFont) { m_LargeFont = g_FrameMan.GetLargeFont(); }

		if (!m_BGBitmap) {
			int diameter = (c_FullRadius + std::max(m_BackgroundThickness, m_BackgroundSeparatorSize) + 2) * 2;
			m_BGBitmap = create_bitmap_ex(8, diameter, diameter);
			clear_to_color(m_BGBitmap, g_MaskColor);
			m_BGBitmapNeedsRedrawing = true;
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PieMenuGUI::Create(const PieMenuGUI &reference) {
		Entity::Create(reference);

		m_LargeFont = reference.m_LargeFont;

		m_MenuController = reference.m_MenuController;
		m_AffectedObject = reference.m_AffectedObject;
		m_MenuMode = reference.m_MenuMode;
		m_CenterPos = reference.m_CenterPos;
		m_Rotation = reference.m_Rotation;

		m_IconSeparatorMode = reference.m_IconSeparatorMode;
		m_BackgroundThickness = reference.m_BackgroundThickness;
		m_BackgroundSeparatorSize = reference.m_BackgroundSeparatorSize;
		m_DrawBackgroundTransparent = reference.m_DrawBackgroundTransparent;
		m_BackgroundColor = reference.m_BackgroundColor;
		m_BackgroundBorderColor = reference.m_BackgroundBorderColor;
		m_SelectedItemBackgroundColor = reference.m_SelectedItemBackgroundColor;

		for (int i = 0; i < m_PieQuadrants.size(); i++) { m_PieQuadrants.at(i).Create(reference.m_PieQuadrants.at(i)); }

		m_InnerRadius = reference.m_InnerRadius;
		m_CursorInVisiblePosition = reference.m_CursorInVisiblePosition;
		m_CursorAngle = reference.m_CursorAngle;

		m_BGBitmap = create_bitmap_ex(8, reference.m_BGBitmap->w, reference.m_BGBitmap->h);
		clear_to_color(m_BGBitmap, g_MaskColor);
		m_BGBitmapNeedsRedrawing = true;

		ReloadCurrentPieSlices();
		ExpandPieSliceIntoEmptySpaceIfPossible();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::Destroy(bool notInherited) {
		if (!notInherited) { Entity::Destroy(); }
		destroy_bitmap(m_BGBitmap);
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::ResetSlices() {
		for (int i = 0; i < m_PieQuadrants.size(); i++) {
			m_PieQuadrants.at(i).Reset();
			m_PieQuadrants.at(i).Direction = static_cast<Directions>(i);
		}
		m_HoveredSlice = nullptr;
		m_ActivatedSlice = nullptr;
		m_AlreadyActivatedSlice = nullptr;
		m_CurrentSlices.clear();
		m_BGBitmapNeedsRedrawing = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PieMenuGUI::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "AddPieSlice") {
			if (m_CurrentSlices.size() == 4 * c_QuadrantSlotCount) {
				reader.ReportError("Pie menus cannot have more than " + std::to_string(4 * c_QuadrantSlotCount) + " slices. Use sub-pie menus to better organize your pie slices.");
			}
			if (!AddPieSlice(dynamic_cast<PieSlice *>(g_PresetMan.ReadReflectedPreset(reader)), m_Owner)) {
				reader.ReportError("Tried to add pie slice but that direction was full. Set direction to None if you don't care where the pie slice ends up.");
			}
		} else {
			return Entity::ReadProperty(propName, reader);
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PieMenuGUI::Save(Writer &writer) const {
		Entity::Save(writer);

		for (const PieSlice *pieSlice : m_CurrentSlices) {
			if (pieSlice->GetOriginalSource() == m_Owner) { writer.NewPropertyWithValue("AddPieSlice", pieSlice); }
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::SetOwner(Actor *newOwner) {
		RTEAssert(newOwner->GetPieMenu() == this, "Tried to set Pie Menu owning Actor to Actor with different Pie Menu.");
		for (PieSlice *pieSlice : m_CurrentSlices) {
			if (pieSlice->GetOriginalSource() == m_Owner) { pieSlice->SetOriginalSource(newOwner); }
		}
		m_Owner = newOwner;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Controller * PieMenuGUI::GetController() {
		if (m_MenuController) {
			return m_MenuController;
		}
		RTEAssert(m_Owner, "PieMenu " + GetPresetName() + " has no owner or menu Controller.");
		return m_Owner->GetController();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::SetEnabled(bool enable, bool playSounds) {
		m_MenuMode = MenuMode::Normal;

		bool enabledStateDoesNotMatchInput = (enable && m_EnabledState != EnabledState::Enabled && m_EnabledState != EnabledState::Enabling) || (!enable && m_EnabledState != EnabledState::Disabled && m_EnabledState != EnabledState::Disabling);
		if (enabledStateDoesNotMatchInput) {
			m_EnabledState = enable ? EnabledState::Enabling : EnabledState::Disabling;
			m_EnableDisableAnimationTimer.Reset();
			if (Controller *controller = GetController(); controller && controller->IsMouseControlled()) {
				g_UInputMan.SetMouseValueMagnitude(0, controller->GetPlayer());
				controller->m_AnalogCursor.Reset();
			}

			if (playSounds) {
				SoundContainer *soundToPlay = enable ? g_GUISound.PieMenuEnterSound() : g_GUISound.PieMenuExitSound();
				soundToPlay->Play();
			}

			if (enable) {
				for (const auto &[listenerObject, listenerFunction] : m_OnPieMenuOpenListeners) { listenerFunction(); }
			} else {
				for (const auto &[listenerObject, listenerFunction] : m_OnPieMenuCloseListeners) { listenerFunction(); }
				m_AlreadyActivatedSlice = nullptr;
				m_HoverTimer.SetRealTimeLimitMS(100);
				m_HoverTimer.Reset();
				if (m_ActivatedSlice &&m_ActivatedSlice->GetSubMenu()) { m_ActivatedSlice->GetSubMenu()->SetEnabled(false, false); }
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PieSlice *PieMenuGUI::GetPieSliceByType(PieSlice::PieSliceIndex pieSliceType) const {
		for (PieSlice *pieSlice : m_CurrentSlices) {
			if (pieSlice->GetType() == pieSliceType) { return pieSlice; }
		}
		return nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PieMenuGUI::AddPieSlice(PieSlice *newPieSlice, const Entity *sliceSource, bool allowQuadrantOverflow) {
		bool sliceWasAdded = false;
		if (newPieSlice->GetDirection() == Directions::Any) {
			PieQuadrant *leastFullPieQuadrant = nullptr;
			for (PieQuadrant &pieQuadrant : m_PieQuadrants) {
				if (!leastFullPieQuadrant || pieQuadrant.GetFlattenedSlices().size() < leastFullPieQuadrant->GetFlattenedSlices().size()) {
					leastFullPieQuadrant = &pieQuadrant;
				}
			}
			sliceWasAdded = leastFullPieQuadrant->AddPieSlice(newPieSlice);
		} else {
			int desiredQuadrantIndex = static_cast<int>(newPieSlice->GetDirection());
			sliceWasAdded = m_PieQuadrants.at(desiredQuadrantIndex).AddPieSlice(newPieSlice);
			while (!sliceWasAdded && allowQuadrantOverflow && desiredQuadrantIndex < m_PieQuadrants.size()) {
				desiredQuadrantIndex++;
				sliceWasAdded = m_PieQuadrants.at(desiredQuadrantIndex).AddPieSlice(newPieSlice);
			}
		}

		if (!sliceWasAdded) {
			delete newPieSlice;
		} else {
			newPieSlice->SetOriginalSource(sliceSource);
			ReloadCurrentPieSlices();
			ExpandPieSliceIntoEmptySpaceIfPossible();
		}
		return sliceWasAdded;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::Update() {
		m_ActivatedSlice = nullptr;

		if (m_Owner) {
			m_CenterPos = m_Owner->GetCPUPos();
		} else if (m_AffectedObject) {
			const Actor *affectedObjectAsActor = dynamic_cast<Actor *>(m_AffectedObject);
			m_CenterPos = (affectedObjectAsActor ? affectedObjectAsActor->GetCPUPos() : m_AffectedObject->GetPos());
		}

		if (m_MenuMode == MenuMode::Wobble) {
			UpdateWobbling();
		} else if (m_MenuMode == MenuMode::Freeze) {
			m_EnabledState = EnabledState::Enabling;
		} else if (m_EnabledState == EnabledState::Enabling || m_EnabledState == EnabledState::Disabling) {
			UpdateEnablingAndDisablingProgress();
		}

		if (GetController()->IsDisabled()) {
			return;
		}

		if (m_AffectedObject && !g_MovableMan.ValidMO(m_AffectedObject)) { m_AffectedObject = nullptr; }

		if (m_MenuMode == MenuMode::Normal) {
			if (IsEnabled() && m_ActivatedSlice && m_ActivatedSlice->GetSubMenu() != nullptr) {
				m_CursorAngle = m_ActivatedSlice->GetMidAngle() + GetRotAngle();
				m_HoverTimer.Reset();
			} else if (IsEnabled() && (HandleMouseInput() || HandleNonMouseInput())) {
				m_HoverTimer.Reset();
			} else if (m_HoverTimer.IsPastRealTimeLimit()) {
				m_HoveredSlice = nullptr;
				m_CursorInVisiblePosition = false;
				m_BGBitmapNeedsRedrawing = true;
			}

			if (m_HoveredSlice && m_EnabledState != EnabledState::Disabled) {
				UpdateSliceActivation();
				if (m_HoveredSlice->GetSubMenu()) {
					//TODO do submenu handling
				}
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
			if (m_CursorInVisiblePosition) { DrawPieCursorAndPieSliceDescriptions(targetBitmap, drawPos); }
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
		if (const Controller *controller = GetController()) {
			const PieSlice *pieSliceToSelect = nullptr;
			if (controller->GetAnalogCursor().GetLargest() > 0.45F) {
				m_CursorInVisiblePosition = true;
				m_CursorAngle = NormalizeAngleBetween0And2PI(controller->GetAnalogCursor().GetAbsRadAngle());

				float pieSliceAreaEnd;
				for (const PieSlice *pieSlice : m_CurrentSlices) {
					pieSliceAreaEnd = pieSlice->GetStartAngle() + (c_PieSliceSlotSize * static_cast<float>(pieSlice->GetSlotCount())) + GetRotAngle();
					if ((pieSlice->GetStartAngle() + GetRotAngle() <= m_CursorAngle && pieSliceAreaEnd > m_CursorAngle) || (pieSliceAreaEnd > c_TwoPI && m_CursorAngle >= 0 && m_CursorAngle < (pieSliceAreaEnd - c_TwoPI))) {
						pieSliceToSelect = pieSlice;
						break;
					}
				}
				SelectPieSlice(pieSliceToSelect);
				m_HoverTimer.SetRealTimeLimitMS(50);
				return true;
			}
		}

		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PieMenuGUI::HandleNonMouseInput() {
		const Controller *controller = GetController();
		if (!controller) {
			return false;
		}

		std::unordered_map<ControlState, Directions> controlStateDirections = {
			{ControlState::PRESS_UP, Directions::Up},
			{ControlState::PRESS_DOWN, Directions::Down},
			{ControlState::PRESS_LEFT, Directions::Left},
			{ControlState::PRESS_RIGHT, Directions::Right}
		};

		std::unordered_map<Directions, Directions> oppositeDirections = {
			{Directions::Up, Directions::Down},
			{Directions::Down, Directions::Up},
			{Directions::Left, Directions::Right},
			{Directions::Right, Directions::Left}
		};

		std::unordered_map<Directions, Directions> counterClockwiseDirections = {
			{Directions::Up, Directions::Left},
			{Directions::Down, Directions::Right},
			{Directions::Left, Directions::Down},
			{Directions::Right, Directions::Up}
		};

		auto GetPieQuadrantContainingHoveredSlice = [this]() {
			if (m_HoveredSlice) {
				for (const PieQuadrant &pieQuadrant : m_PieQuadrants) {
					if (pieQuadrant.ContainsSlice(m_HoveredSlice)) {
						return &pieQuadrant;
					}
				}
			}
			return static_cast<const PieQuadrant *>(nullptr);
		};

		auto ZoomToPieQuadrant = [this](const PieQuadrant &pieQuadrant) {
			if (pieQuadrant.GetFlattenedSlices().empty()) {
				SelectPieSlice(nullptr);
				m_CursorInVisiblePosition = true;
				m_CursorAngle = c_DirectionsToRadiansMap.at(pieQuadrant.Direction) + GetRotAngle();
			} else if (pieQuadrant.MiddlePieSlice) {
				SelectPieSlice(pieQuadrant.MiddlePieSlice.get(), true);
			} else {
				SelectPieSlice(pieQuadrant.LeftPieSlices.at(0).get(), true);
			}
		};

		auto StepToNeighbouringPieSlice = [this](const PieQuadrant *hoveredPieSlicePieQuadrant, Directions controlStateDirection, bool counterClockwiseMovement) {
			std::vector<PieSlice *> flattenedPieSlices = hoveredPieSlicePieQuadrant->GetFlattenedSlices(true);
			auto hoveredSliceIterator = std::find(flattenedPieSlices.begin(), flattenedPieSlices.end(), m_HoveredSlice);

			if ((counterClockwiseMovement && hoveredSliceIterator == flattenedPieSlices.end() - 1) || (!counterClockwiseMovement && hoveredSliceIterator == flattenedPieSlices.begin())) {
				std::vector<PieSlice *> pieQuadrantToMoveToFlattenedPieSlices = m_PieQuadrants.at(controlStateDirection).GetFlattenedSlices();
				if (pieQuadrantToMoveToFlattenedPieSlices.empty()) {
					SelectPieSlice(nullptr);
					m_CursorInVisiblePosition = true;
					m_CursorAngle = c_DirectionsToRadiansMap.at(controlStateDirection) + GetRotAngle();
				} else {
					SelectPieSlice(counterClockwiseMovement ? *(pieQuadrantToMoveToFlattenedPieSlices.begin()) : *(pieQuadrantToMoveToFlattenedPieSlices.end() - 1), true);
				}
			} else {
				SelectPieSlice(*(hoveredSliceIterator + (counterClockwiseMovement ? 1 : -1)), true);
			}
		};

		for (const auto &[controlState, controlStateDirection] : controlStateDirections) {
			if (controller->IsState(controlState)) {
				if (!m_HoveredSlice) {
					ZoomToPieQuadrant(m_PieQuadrants.at(controlStateDirection));
				} else if (const PieQuadrant *hoveredPieSlicePieQuadrant = GetPieQuadrantContainingHoveredSlice()) {
					if (controlStateDirection == oppositeDirections.at(hoveredPieSlicePieQuadrant->Direction)) {
						ZoomToPieQuadrant(m_PieQuadrants.at(controlStateDirection));
					} else if (controlStateDirection != hoveredPieSlicePieQuadrant->Direction || m_HoveredSlice != hoveredPieSlicePieQuadrant->MiddlePieSlice.get()) {
						bool counterClockwiseMovement = controlStateDirection == counterClockwiseDirections.at(hoveredPieSlicePieQuadrant->Direction);

						const std::array<std::unique_ptr<PieSlice>, c_QuadrantSlotCount / 2> &pieSlicesToZoomFor = counterClockwiseMovement ? hoveredPieSlicePieQuadrant->RightPieSlices : hoveredPieSlicePieQuadrant->LeftPieSlices;
						if (m_HoveredSlice == pieSlicesToZoomFor.at(0).get() || m_HoveredSlice == pieSlicesToZoomFor.at(1).get()) {
							ZoomToPieQuadrant(m_PieQuadrants.at(controlStateDirection));
						} else {
							StepToNeighbouringPieSlice(hoveredPieSlicePieQuadrant, controlStateDirection, counterClockwiseMovement);
						}
					}
				}
				m_HoverTimer.SetRealTimeLimitMS(2000);
				return true;
			}
		}

		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::UpdateSliceActivation() {
		const Controller *controller = GetController();
		if (controller && (controller->IsState(PRESS_PRIMARY) || (m_HoveredSlice != m_AlreadyActivatedSlice && controller->IsState(RELEASE_SECONDARY)))) {
			m_HoverTimer.Reset();
			m_ActivatedSlice = m_HoveredSlice->IsEnabled() ? m_HoveredSlice : m_ActivatedSlice;
			m_AlreadyActivatedSlice = m_ActivatedSlice;

			SoundContainer *soundToPlay = m_HoveredSlice->IsEnabled() ? g_GUISound.SlicePickedSound() : g_GUISound.DisabledPickedSound();
			soundToPlay->Play();
		}

		if (m_ActivatedSlice && !m_ActivatedSlice->GetScriptPath().empty() && !m_ActivatedSlice->GetFunctionName().empty()) {
			// TODO: Investigate reloading the file each time. I think it's needed cause this stuff isn't in PresetMan, so this is the only way for it to be reloadable. To test, have script on slice, edit it and see what happens with and without this. Also, make this support not having an actor I guess.
			if (MovableObject *scriptTarget = m_Owner ? m_Owner : m_AffectedObject) {
				g_LuaMan.RunScriptFile(m_ActivatedSlice->GetScriptPath());
				g_LuaMan.SetTempEntity(scriptTarget);
				std::string functionString = m_ActivatedSlice->GetFunctionName() + "(";
				bool scriptTargetIsActor = dynamic_cast<Actor *>(scriptTarget) != nullptr;
				functionString += scriptTargetIsActor ? "ToActor(" : "";
				functionString += "LuaMan.TempEntity";
				functionString += scriptTargetIsActor ? ")" : "";
				functionString += ");";
				g_LuaMan.RunScriptString(functionString);
			}
		} else if (m_ActivatedSlice && m_ActivatedSlice->GetSubMenu() != nullptr) {
			m_ActivatedSlice->GetSubMenu()->SetPos(m_CenterPos + Vector(static_cast<float>(m_InnerRadius + (m_BackgroundThickness * 2)), 0).RadRotate(m_ActivatedSlice->GetMidAngle() + GetRotAngle()));
			m_ActivatedSlice->GetSubMenu()->SetRotAngle(m_ActivatedSlice->GetMidAngle() + GetRotAngle());
			m_ActivatedSlice->GetSubMenu()->SetEnabled(true);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::UpdatePredrawnMenuBackgroundBitmap() {
		int centerX = m_BGBitmap->w / 2;
		int centerY = m_BGBitmap->h / 2;

		clear_to_color(m_BGBitmap, g_MaskColor);
		circlefill(m_BGBitmap, centerX, centerY, m_InnerRadius + m_BackgroundThickness, m_BackgroundColor);
		circlefill(m_BGBitmap, centerX, centerY, m_InnerRadius, g_MaskColor);


		if (m_EnabledState == EnabledState::Enabled) {
			Vector separator;

			/// <summary>
			/// Lambda for drawing separators when in Line IconSeparatorMode.
			/// It draws four separator lines between each PieSlice so the resulting separation will be at least 2 pixels thick, regardless of the PieSlice's angles.
			/// </summary>
			auto DrawLineIconSeparator = [this, &centerX, &centerY, &separator](float rotAngle) {
				separator.SetXY(static_cast<float>(m_InnerRadius + m_BackgroundThickness + m_BackgroundSeparatorSize), 0).RadRotate(rotAngle);
				line(m_BGBitmap, centerX, centerY, centerX + separator.GetCeilingIntX(), centerY + separator.GetCeilingIntY(), m_BackgroundBorderColor);
				line(m_BGBitmap, centerX + 1, centerY, centerX + 1 + separator.GetCeilingIntX(), centerY + separator.GetCeilingIntY(), m_BackgroundBorderColor);
				line(m_BGBitmap, centerX, centerY + 1, centerX + separator.GetCeilingIntX(), centerY + 1 + separator.GetCeilingIntY(), m_BackgroundBorderColor);
				line(m_BGBitmap, centerX + 1, centerY + 1, centerX + 1 + separator.GetCeilingIntX(), centerY + 1 + separator.GetCeilingIntY(), m_BackgroundBorderColor);
			};

			/// <summary>
			/// Lambda for drawing separators when in Circle IconSeparatorMode. It draws background circles for each PieSlice.
			/// </summary>
			auto DrawCircleIconSeparator = [this, &centerX, &centerY, &separator](const PieSlice *slice) {
				separator = Vector(static_cast<float>(m_InnerRadius) + (static_cast<float>(m_BackgroundThickness) / 2.0F), 0).RadRotate(slice->GetMidAngle() + GetRotAngle());
				circlefill(m_BGBitmap, centerX + separator.GetFloorIntX(), centerY + separator.GetFloorIntY(), m_BackgroundSeparatorSize, m_BackgroundBorderColor);
				circlefill(m_BGBitmap, centerX + separator.GetFloorIntX(), centerY + separator.GetFloorIntY(), m_BackgroundSeparatorSize - 1, slice == m_HoveredSlice && slice->IsEnabled() ? m_SelectedItemBackgroundColor : m_BackgroundColor);
			};

			switch (m_IconSeparatorMode) {
				case IconSeparatorMode::Line:
					const PieSlice *pieSliceInSlot;
					for (const PieQuadrant &pieQuadrant : m_PieQuadrants) {
						float currentAngle = c_DirectionsToRadiansMap.at(pieQuadrant.Direction) - c_QuarterPI;
						bool pieQuadrantIsEmpty = pieQuadrant.GetFlattenedSlices().empty();
						for (int currentSlot = 0; currentSlot < c_QuadrantSlotCount;) {
							pieSliceInSlot = pieQuadrant.SlotsForPieSlices.at(currentSlot);
							if (pieSliceInSlot) {
								if (currentSlot > 0 || pieSliceInSlot->GetSlotCount() != 2) {
									currentAngle = pieSliceInSlot->GetStartAngle();
									DrawLineIconSeparator(currentAngle);
									currentSlot += pieSliceInSlot->GetSlotCount();
									DrawLineIconSeparator(currentAngle + (static_cast<float>(pieSliceInSlot->GetSlotCount()) * c_PieSliceSlotSize));
									continue;
								}
							} else {
								if (!pieQuadrantIsEmpty || currentSlot == 0) { DrawLineIconSeparator(currentAngle); }
								currentAngle += c_PieSliceSlotSize;
							}
							currentSlot++;
						}
					}

					if (m_HoveredSlice && m_HoveredSlice->IsEnabled()) {
						separator.SetXY(static_cast<float>(m_InnerRadius + (m_BackgroundThickness / 2)), 0.0F).RadRotate(m_HoveredSlice->GetMidAngle() + GetRotAngle());
						floodfill(m_BGBitmap, centerX + separator.GetFloorIntX(), centerY + separator.GetFloorIntY(), m_SelectedItemBackgroundColor);
					}
					break;
				case IconSeparatorMode::Circle:
					for (const PieSlice *slice : m_CurrentSlices) {
						if (slice->GetType() != PieSlice::PieSliceIndex::PSI_NONE) { DrawCircleIconSeparator(slice); }
					}
					break;
				default:
					RTEAbort("Invalid icon separator mode in PieMenuGUI.");
			}
		}

		m_BGBitmapNeedsRedrawing = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
			// Note - offscreen piemenu is used for signalling selectable actors so it's currently desirable. Strategic mode won't want that clutter, so this can probably change then.
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
				sliceIconOffset = Vector(static_cast<float>(m_InnerRadius + (m_BackgroundThickness / 2)), 0).RadRotate(slice->GetMidAngle() + GetRotAngle()) + Vector(1.0F - static_cast<float>(sliceIcon->w / 2), 1.0F - static_cast<float>(sliceIcon->h / 2));
				draw_sprite(targetBitmap, sliceIcon, drawPos.GetFloorIntX() + sliceIconOffset.GetFloorIntX(), drawPos.GetFloorIntY() + sliceIconOffset.GetFloorIntY());
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::DrawPieCursorAndPieSliceDescriptions(BITMAP *targetBitmap, const Vector &drawPos) const {
		Vector cursorPos = Vector(static_cast<float>(m_InnerRadius), 0.0F).RadRotate(m_CursorAngle);
		pivot_sprite(targetBitmap, s_CursorBitmap, drawPos.GetFloorIntX() + cursorPos.GetFloorIntX(), drawPos.GetFloorIntY() + cursorPos.GetFloorIntY(), s_CursorBitmap->w / 2, s_CursorBitmap->h / 2, ftofix((m_CursorAngle / c_PI) * -128));

		if (m_HoveredSlice) {
			Vector textPos = Vector(static_cast<float>(m_InnerRadius + m_BackgroundThickness + (m_LargeFont->GetFontHeight() * 0.5)), 0).RadRotate(m_HoveredSlice->GetMidAngle() + GetRotAngle()) - Vector(0.0F, static_cast<float>(m_LargeFont->GetFontHeight()) * 0.45F);

			AllegroBitmap allegroBitmap(targetBitmap);
			if (m_HoveredSlice == m_PieQuadrants.at(Directions::Up).MiddlePieSlice.get() || m_HoveredSlice == m_PieQuadrants.at(Directions::Down).MiddlePieSlice.get()) {
				m_LargeFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() + textPos.GetFloorIntX(), drawPos.GetFloorIntY() + textPos.GetFloorIntY(), m_HoveredSlice->GetDescription().c_str(), GUIFont::Centre);
			} else if (m_CursorAngle < c_HalfPI || m_CursorAngle > c_PI + c_HalfPI) {
				m_LargeFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() + textPos.GetFloorIntX(), drawPos.GetFloorIntY() + textPos.GetFloorIntY(), m_HoveredSlice->GetDescription().c_str(), GUIFont::Left);
			} else {
				m_LargeFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() + textPos.GetFloorIntX(), drawPos.GetFloorIntY() + textPos.GetFloorIntY(), m_HoveredSlice->GetDescription().c_str(), GUIFont::Right);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::ReloadCurrentPieSlices() {
		m_CurrentSlices.clear();
		for (PieQuadrant &pieQuadrant : m_PieQuadrants) {
			pieQuadrant.RealignSlices();
			for (PieSlice *pieSlice : pieQuadrant.GetFlattenedSlices()) {
				m_CurrentSlices.push_back(pieSlice);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenuGUI::ExpandPieSliceIntoEmptySpaceIfPossible() {
		const std::unordered_map<Directions, Directions> nextDirectionForGivenDirection{ {Directions::Right, Directions::Up}, {Directions::Up, Directions::Left}, {Directions::Left, Directions::Down}, {Directions::Down, Directions::Right} };

		for (const PieQuadrant &pieQuadrant : m_PieQuadrants) {
			const std::unique_ptr<PieSlice> &leftMostPieSlice = !pieQuadrant.LeftPieSlices.at(1) ? pieQuadrant.LeftPieSlices.at(0) : pieQuadrant.LeftPieSlices.at(1);
			PieQuadrant &neighbouringPieQuadrant = m_PieQuadrants.at(nextDirectionForGivenDirection.at(pieQuadrant.Direction));
			
			if (leftMostPieSlice && leftMostPieSlice->GetSlotCount() == 1 && neighbouringPieQuadrant.SlotsForPieSlices.at(0) == nullptr) {
				leftMostPieSlice->SetSlotCount(leftMostPieSlice->GetSlotCount() + 1);
				neighbouringPieQuadrant.SlotsForPieSlices.at(0) = leftMostPieSlice.get();
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PieMenuGUI::SelectPieSlice(const PieSlice *pieSliceToSelect, bool moveCursorToSlice) {
		if (pieSliceToSelect == m_HoveredSlice) {
			return false;
		}

		m_HoveredSlice = pieSliceToSelect;
		m_BGBitmapNeedsRedrawing = true;

		if (pieSliceToSelect) {
			if (moveCursorToSlice) {
				m_CursorInVisiblePosition = true;
				m_CursorAngle = m_HoveredSlice->GetMidAngle() + GetRotAngle();
			}

			SoundContainer *soundToPlay = pieSliceToSelect->IsEnabled() ? g_GUISound.HoverChangeSound() : g_GUISound.HoverDisabledSound();
			soundToPlay->Play();
		}
		return true;
	}
}
