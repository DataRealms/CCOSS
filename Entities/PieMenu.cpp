#include "PieMenu.h"

#include "FrameMan.h"
#include "UInputMan.h"
#include "PresetMan.h"
#include "SettingsMan.h"

#include "AHuman.h"
#include "ContentFile.h"

#include "GUI.h"
#include "GUIFont.h"
#include "AllegroBitmap.h"

namespace RTE {

	ConcreteClassInfo(PieMenu, Entity, 20);

	const std::unordered_map<std::string, PieMenu::IconSeparatorMode> PieMenu::c_IconSeparatorModeMap = {
		{"Line", IconSeparatorMode::Line},
		{"Circle", IconSeparatorMode::Circle},
		{"Square", IconSeparatorMode::Square}
	};

	const std::unordered_map<ControlState, Directions> PieMenu::c_ControlStateDirections = {
		{ControlState::PRESS_UP, Directions::Up},
		{ControlState::PRESS_DOWN, Directions::Down},
		{ControlState::PRESS_LEFT, Directions::Left},
		{ControlState::PRESS_RIGHT, Directions::Right}
	};

	const std::unordered_map<Directions, Directions> PieMenu::c_OppositeDirections = {
		{Directions::Up, Directions::Down},
		{Directions::Down, Directions::Up},
		{Directions::Left, Directions::Right},
		{Directions::Right, Directions::Left}
	};

	const std::unordered_map<Directions, Directions> PieMenu::c_CounterClockwiseDirections = {
		{Directions::Up, Directions::Left},
		{Directions::Down, Directions::Right},
		{Directions::Left, Directions::Down},
		{Directions::Right, Directions::Up}
	};

	BITMAP * PieMenu::s_CursorBitmap = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenu::Clear() {
		m_LargeFont = nullptr;

		m_Owner = nullptr;
		m_MenuController = nullptr;
		m_AffectedObject = nullptr;
		m_DirectionIfSubPieMenu = Directions::None;
		m_MenuMode = MenuMode::Normal;
		m_CenterPos.Reset();
		m_Rotation.Reset();

		m_EnabledState = EnabledState::Disabled;
		m_EnableDisableAnimationTimer.Reset();
		m_HoverTimer.Reset();
		m_SubPieMenuHoverOpenTimer.Reset();
		m_SubPieMenuHoverOpenTimer.SetRealTimeLimitMS(g_SettingsMan.GetSubPieMenuHoverOpenDelay());

		m_IconSeparatorMode = IconSeparatorMode::Line;
		m_FullInnerRadius = c_DefaultFullRadius;
		m_BackgroundThickness = 16;
		m_BackgroundSeparatorSize = 2;
		m_DrawBackgroundTransparent = true;
		m_BackgroundColor = 4;
		m_BackgroundBorderColor = ColorKeys::g_MaskColor;
		m_SelectedItemBackgroundColor = ColorKeys::g_BlackColor;

		for (int i = 0; i < m_PieQuadrants.size(); i++) {
			m_PieQuadrants[i].Reset();
			m_PieQuadrants[i].m_Direction = static_cast<Directions>(i);
		}
		m_HoveredPieSlice = nullptr;
		m_ActivatedPieSlice = nullptr;
		m_AlreadyActivatedPieSlice = nullptr;
		m_CurrentPieSlices.clear();

		m_ActiveSubPieMenu = nullptr;

		m_WhilePieMenuOpenListeners.clear();

		m_CurrentInnerRadius = 0;
		m_CursorInVisiblePosition = false;
		m_CursorAngle = 0;

		m_BGBitmap = nullptr;
		m_BGRotationBitmap = nullptr;
		m_BGPieSlicesWithSubPieMenuBitmap = nullptr;
		m_BGBitmapNeedsRedrawing = true;
		m_BGPieSlicesWithSubPieMenuBitmapNeedsRedrawing = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PieMenu::Create() {
		if (!s_CursorBitmap) { s_CursorBitmap = ContentFile("Base.rte/GUIs/PieMenus/PieCursor.png").GetAsBitmap(); }

		if (!m_LargeFont) { m_LargeFont = g_FrameMan.GetLargeFont(); }

		if (!m_BGBitmap) { RecreateBackgroundBitmaps(); }

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PieMenu::Create(const PieMenu &reference) {
		Entity::Create(reference);

		m_LargeFont = reference.m_LargeFont;

		m_Owner = reference.m_Owner;
		m_MenuController = reference.m_MenuController;
		m_AffectedObject = reference.m_AffectedObject;
		m_DirectionIfSubPieMenu = reference.m_DirectionIfSubPieMenu;
		m_MenuMode = reference.m_MenuMode;
		m_CenterPos = reference.m_CenterPos;
		m_Rotation = reference.m_Rotation;

		m_EnabledState = reference.m_EnabledState;
		m_EnableDisableAnimationTimer = reference.m_EnableDisableAnimationTimer;
		m_HoverTimer = reference.m_HoverTimer;
		m_SubPieMenuHoverOpenTimer = reference.m_SubPieMenuHoverOpenTimer;

		m_IconSeparatorMode = reference.m_IconSeparatorMode;
		m_FullInnerRadius = reference.m_FullInnerRadius;
		m_BackgroundThickness = reference.m_BackgroundThickness;
		m_BackgroundSeparatorSize = reference.m_BackgroundSeparatorSize;
		m_DrawBackgroundTransparent = reference.m_DrawBackgroundTransparent;
		m_BackgroundColor = reference.m_BackgroundColor;
		m_BackgroundBorderColor = reference.m_BackgroundBorderColor;
		m_SelectedItemBackgroundColor = reference.m_SelectedItemBackgroundColor;

		for (int i = 0; i < m_PieQuadrants.size(); i++) {
			m_PieQuadrants[i].Create(reference.m_PieQuadrants[i], &reference, this);
			for (const PieSlice *pieSlice : m_PieQuadrants[i].GetFlattenedPieSlices()) {
				if (pieSlice->GetOriginalSource() != this) {
					m_PieQuadrants[i].RemovePieSlice(pieSlice);
				}
			}
		}

		m_CurrentInnerRadius = reference.m_CurrentInnerRadius;
		m_CursorInVisiblePosition = reference.m_CursorInVisiblePosition;
		m_CursorAngle = reference.m_CursorAngle;

		RecreateBackgroundBitmaps();

		RepopulateAndRealignCurrentPieSlices();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenu::Destroy(bool notInherited) {
		if (!notInherited) { Entity::Destroy(); }
		destroy_bitmap(m_BGBitmap);
		destroy_bitmap(m_BGRotationBitmap);
		destroy_bitmap(m_BGPieSlicesWithSubPieMenuBitmap);
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PieMenu::ReadProperty(const std::string_view &propName, Reader &reader) {
		StartPropertyList(return Entity::ReadProperty(propName, reader));
		
		MatchProperty("IconSeparatorMode", {
			std::string iconSeparatorModeString = reader.ReadPropValue();
			auto itr = c_IconSeparatorModeMap.find(iconSeparatorModeString);
			if (itr != c_IconSeparatorModeMap.end()) {
				m_IconSeparatorMode = itr->second;
			} else {
				try {
					m_IconSeparatorMode = static_cast<IconSeparatorMode>(std::stoi(iconSeparatorModeString));
				} catch (const std::invalid_argument &) {
					reader.ReportError("IconSeparatorMode " + iconSeparatorModeString + " is invalid.");
				}
			}
		});
		MatchProperty("FullInnerRadius", { reader >> m_FullInnerRadius; });
		MatchProperty("BackgroundThickness", { reader >> m_BackgroundThickness; });
		MatchProperty("BackgroundSeparatorSize", { reader >> m_BackgroundSeparatorSize; });
		MatchProperty("DrawBackgroundTransparent", { reader >> m_DrawBackgroundTransparent; });
		MatchProperty("BackgroundColor", { reader >> m_BackgroundColor; });
		MatchProperty("BackgroundBorderColor", { reader >> m_BackgroundBorderColor; });
		MatchProperty("SelectedItemBackgroundColor", { reader >> m_SelectedItemBackgroundColor; });
		MatchProperty("AddPieSlice", {
			if (m_CurrentPieSlices.size() == 4 * PieQuadrant::c_PieQuadrantSlotCount) {
				reader.ReportError("Pie menus cannot have more than " + std::to_string(4 * PieQuadrant::c_PieQuadrantSlotCount) + " slices. Use sub-pie menus to better organize your pie slices.");
			}
			if (!AddPieSlice(dynamic_cast<PieSlice *>(g_PresetMan.ReadReflectedPreset(reader)), this)) {
				reader.ReportError("Tried to add pie slice but that direction was full. Set direction to None if you don't care where the pie slice ends up.");
			}
		});

		EndPropertyList;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PieMenu::Save(Writer &writer) const {
		Entity::Save(writer);

		writer.NewPropertyWithValue("IconSeparatorMode", static_cast<int>(m_IconSeparatorMode));
		writer.NewPropertyWithValue("FullInnerRadius", m_FullInnerRadius);
		writer.NewPropertyWithValue("BackgroundThickness", m_BackgroundThickness);
		writer.NewPropertyWithValue("BackgroundSeparatorSize", m_BackgroundSeparatorSize);
		writer.NewPropertyWithValue("DrawBackgroundTransparent", m_DrawBackgroundTransparent);
		writer.NewPropertyWithValue("BackgroundColor", m_BackgroundColor);
		writer.NewPropertyWithValue("BackgroundBorderColor", m_BackgroundBorderColor);
		writer.NewPropertyWithValue("SelectedItemBackgroundColor", m_SelectedItemBackgroundColor);

		for (const PieSlice *pieSlice : m_CurrentPieSlices) {
			if (pieSlice->GetOriginalSource() == m_Owner) { writer.NewPropertyWithValue("AddPieSlice", pieSlice); }
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenu::SetOwner(Actor *newOwner) {
		RTEAssert((newOwner == nullptr) ? true : (newOwner->GetPieMenu() == this || IsSubPieMenu()), "Tried to set Pie Menu owning Actor to Actor with different Pie Menu.");
		if (m_Owner) {
			for (PieSlice *pieSlice : m_CurrentPieSlices) {
				if (pieSlice->GetOriginalSource() == m_Owner) { pieSlice->SetOriginalSource(newOwner); }
			}
			if (!IsSubPieMenu()) {
				RemoveWhilePieMenuOpenListener(m_Owner);
			}
		}
		if (newOwner && !IsSubPieMenu()) {
			AddWhilePieMenuOpenListener(newOwner, std::bind(&MovableObject::WhilePieMenuOpenListener, newOwner, this));
		}
		m_Owner = newOwner;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Controller * PieMenu::GetController() const {
		if (m_MenuController) {
			return m_MenuController;
		}
		if (!m_Owner) {
			RTEAbort("PieMenu " + GetPresetName() + " has no owner or menu Controller.");
			return nullptr;
		}
		return m_Owner->GetController();
	}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenu::SetPos(const Vector &newPos) {
		if (g_SceneMan.ShortestDistance(m_CenterPos, newPos, g_SceneMan.SceneWrapsX()).GetMagnitude() > 2.0F) { m_CenterPos = newPos; }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenu::SetEnabled(bool enable, bool playSounds) {
		m_MenuMode = MenuMode::Normal;

		bool enabledStateDoesNotMatchInput = (enable && m_EnabledState != EnabledState::Enabled && m_EnabledState != EnabledState::Enabling) || (!enable && m_EnabledState != EnabledState::Disabled && m_EnabledState != EnabledState::Disabling);
		if (enabledStateDoesNotMatchInput) {
			m_EnabledState = enable ? EnabledState::Enabling : EnabledState::Disabling;
			m_EnableDisableAnimationTimer.Reset();

			PrepareAnalogCursorForEnableOrDisable(enable);

			if (playSounds) {
				SoundContainer *soundToPlay = enable ? g_GUISound.PieMenuEnterSound() : g_GUISound.PieMenuExitSound();
				soundToPlay->Play();
			}

			if (!enable) {
				m_AlreadyActivatedPieSlice = nullptr;
				m_HoverTimer.SetRealTimeLimitMS(100);
				m_HoverTimer.Reset();
				SetHoveredPieSlice(nullptr);
				if (m_ActiveSubPieMenu) {
					m_ActivatedPieSlice = !m_ActivatedPieSlice ? m_ActiveSubPieMenu->GetActivatedPieSlice() : m_ActivatedPieSlice;
					m_ActiveSubPieMenu->SetEnabled(false, false);
					m_ActiveSubPieMenu = nullptr;
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const PieSlice * PieMenu::GetActivatedPieSlice() const {
		if (m_ActiveSubPieMenu) {
			return m_ActiveSubPieMenu->GetActivatedPieSlice();
		}
		return m_ActivatedPieSlice;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PieSlice::SliceType PieMenu::GetPieCommand() const {
		const PieSlice *activatedSlice = m_ActiveSubPieMenu ? m_ActiveSubPieMenu->GetActivatedPieSlice() : m_ActivatedPieSlice;
		return (activatedSlice == nullptr) ? PieSlice::SliceType::NoType : activatedSlice->GetType();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PieSlice * PieMenu::GetFirstPieSliceByPresetName(const std::string &presetName) const {
		for (PieSlice *pieSlice : m_CurrentPieSlices) {
			if (pieSlice->GetPresetName() == presetName) {
				return pieSlice;
			}
		}
		return nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PieSlice * PieMenu::GetFirstPieSliceByType(PieSlice::SliceType pieSliceType) const {
		for (PieSlice *pieSlice : m_CurrentPieSlices) {
			if (pieSlice->GetType() == pieSliceType) {
				return pieSlice;
			}
		}
		return nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PieMenu::AddPieSlice(PieSlice *pieSliceToAdd, const Entity *pieSliceOriginalSource, bool allowQuadrantOverflow) {
		Directions pieSliceDirection = pieSliceToAdd->GetDirection();
		if (pieSliceDirection != Directions::Any && !m_PieQuadrants.at(pieSliceDirection).m_Enabled) {
			pieSliceToAdd->SetDirection(Directions::Any);
			pieSliceDirection = Directions::Any;
		}

		bool sliceWasAdded = false;
		if (pieSliceDirection == Directions::Any) {
			PieQuadrant *leastFullPieQuadrant = nullptr;
			for (PieQuadrant &pieQuadrant : m_PieQuadrants) {
				if (pieQuadrant.m_Enabled && (!leastFullPieQuadrant || pieQuadrant.GetFlattenedPieSlices().size() < leastFullPieQuadrant->GetFlattenedPieSlices().size())) {
					leastFullPieQuadrant = &pieQuadrant;
				}
			}
			if (leastFullPieQuadrant) {
				sliceWasAdded = leastFullPieQuadrant->AddPieSlice(pieSliceToAdd);
			}
		} else {
			int desiredQuadrantIndex = static_cast<int>(pieSliceDirection);
			sliceWasAdded = m_PieQuadrants.at(desiredQuadrantIndex).AddPieSlice(pieSliceToAdd);
			while (!sliceWasAdded && allowQuadrantOverflow && desiredQuadrantIndex < m_PieQuadrants.size()) {
				desiredQuadrantIndex++;
				sliceWasAdded = m_PieQuadrants.at(desiredQuadrantIndex).AddPieSlice(pieSliceToAdd);
				pieSliceToAdd->SetDirection(static_cast<Directions>(desiredQuadrantIndex));
			}
		}

		if (!sliceWasAdded) {
			delete pieSliceToAdd;
		} else {
			pieSliceToAdd->SetOriginalSource(pieSliceOriginalSource);
			RepopulateAndRealignCurrentPieSlices();
		}
		return sliceWasAdded;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool PieMenu::AddPieSliceIfPresetNameIsUnique(PieSlice *pieSliceToAdd, const Entity *pieSliceOriginalSource, bool onlyCheckPieSlicesWithSameOriginalSource, bool allowQuadrantOverflow) {
		const std::string &pieSlicePresetName = pieSliceToAdd->GetPresetName();

		if (pieSlicePresetName == "None") {
			return AddPieSlice(dynamic_cast<PieSlice*>(pieSliceToAdd->Clone()), pieSliceOriginalSource, allowQuadrantOverflow);
		}

		bool pieSliceAlreadyExists = onlyCheckPieSlicesWithSameOriginalSource ? false : GetFirstPieSliceByPresetName(pieSlicePresetName) != nullptr;
		if (!pieSliceAlreadyExists) {
			for (const PieSlice *pieSlice : m_CurrentPieSlices) {
				if (pieSlice->GetOriginalSource() == pieSliceOriginalSource && pieSlice->GetPresetName() == pieSlicePresetName) {
					pieSliceAlreadyExists = true;
					break;
				}
			}
		}

		if (pieSliceAlreadyExists) {
			return false;
		}

		return AddPieSlice(dynamic_cast<PieSlice*>(pieSliceToAdd->Clone()), pieSliceOriginalSource, allowQuadrantOverflow);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    PieSlice * PieMenu::RemovePieSlice(const PieSlice *pieSliceToRemove) {
		PieSlice *removedPieSlice = nullptr;

		if (Directions sliceDirection = pieSliceToRemove->GetDirection(); sliceDirection > Directions::None) {
			if (sliceDirection == Directions::Any) {
				for (PieQuadrant &pieQuadrant : m_PieQuadrants) {
					removedPieSlice = pieQuadrant.RemovePieSlice(pieSliceToRemove);
					if (removedPieSlice) {
						break;
					}
				}
			} else {
				removedPieSlice = m_PieQuadrants.at(sliceDirection).RemovePieSlice(pieSliceToRemove);
			}
			if (removedPieSlice) {
				if (PieMenu *removedPieSliceSubPieMenu = removedPieSlice->GetSubPieMenu()) {
					removedPieSliceSubPieMenu->SetEnabled(false);
					removedPieSliceSubPieMenu->SetOwner(nullptr);
				}
				RepopulateAndRealignCurrentPieSlices();
			}
		}

		return removedPieSlice;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PieMenu::RemovePieSlicesByPresetName(const std::string &presetNameToRemoveBy) {
		bool anyPieSlicesRemoved = false;

		std::vector<const PieSlice *> pieSlicesToRemove;
		for (const PieSlice *pieSlice : m_CurrentPieSlices) {
			if (pieSlice->GetPresetName() == presetNameToRemoveBy) {
				pieSlicesToRemove.emplace_back(pieSlice);
				anyPieSlicesRemoved = true;
			}
		}

		for (const PieSlice *pieSliceToRemove : pieSlicesToRemove) {
			delete RemovePieSlice(pieSliceToRemove);
		}
		return anyPieSlicesRemoved;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PieMenu::RemovePieSlicesByType(PieSlice::SliceType pieSliceTypeToRemoveBy) {
		bool anyPieSlicesRemoved = false;

		std::vector<const PieSlice *> pieSlicesToRemove;
		for (const PieSlice *pieSlice : m_CurrentPieSlices) {
			if (pieSlice->GetType() == pieSliceTypeToRemoveBy) {
				pieSlicesToRemove.emplace_back(pieSlice);
				anyPieSlicesRemoved = true;
			}
		}
		for (const PieSlice *pieSliceToRemove : pieSlicesToRemove) {
			delete RemovePieSlice(pieSliceToRemove);
		}

		return anyPieSlicesRemoved;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PieMenu::RemovePieSlicesByOriginalSource(const Entity *originalSource) {
		bool anyPieSlicesRemoved = false;

		std::vector<const PieSlice *> pieSlicesToRemove;
		for (const PieSlice *pieSlice : m_CurrentPieSlices) {
			if (pieSlice->GetOriginalSource() == originalSource) {
				pieSlicesToRemove.emplace_back(pieSlice);
				anyPieSlicesRemoved = true;
			}
		}
		for (const PieSlice *pieSliceToRemove : pieSlicesToRemove) {
			delete RemovePieSlice(pieSliceToRemove);
		}

		return anyPieSlicesRemoved;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    PieSlice * PieMenu::ReplacePieSlice(const PieSlice *pieSliceToReplace, PieSlice *replacementPieSlice) {
		if (pieSliceToReplace == nullptr) {
			return nullptr;
		}
		if (replacementPieSlice == nullptr) {
			return RemovePieSlice(pieSliceToReplace);
		}

		PieSlice *replacedPieSlice = nullptr;

		auto DoPieSliceReplacementInPieQuadrant = [&replacedPieSlice, &pieSliceToReplace, &replacementPieSlice](PieQuadrant &pieQuadrant) {
			if (pieSliceToReplace == pieQuadrant.m_MiddlePieSlice.get()) {
				replacedPieSlice = pieQuadrant.m_MiddlePieSlice.release();
				pieQuadrant.m_MiddlePieSlice = std::unique_ptr<PieSlice>(replacementPieSlice);
			} else if (pieSliceToReplace == pieQuadrant.m_LeftPieSlices[0].get()) {
				replacedPieSlice = pieQuadrant.m_LeftPieSlices[0].release();
				pieQuadrant.m_LeftPieSlices[0] = std::unique_ptr<PieSlice>(replacementPieSlice);
			} else if (pieSliceToReplace == pieQuadrant.m_LeftPieSlices[1].get()) {
				replacedPieSlice = pieQuadrant.m_LeftPieSlices[1].release();
				pieQuadrant.m_LeftPieSlices[1] = std::unique_ptr<PieSlice>(replacementPieSlice);
			} else if (pieSliceToReplace == pieQuadrant.m_RightPieSlices[0].get()) {
				replacedPieSlice = pieQuadrant.m_RightPieSlices[0].release();
				pieQuadrant.m_RightPieSlices[0] = std::unique_ptr<PieSlice>(replacementPieSlice);
			} else if (pieSliceToReplace == pieQuadrant.m_RightPieSlices[1].get()) {
				replacedPieSlice = pieQuadrant.m_RightPieSlices[1].release();
				pieQuadrant.m_RightPieSlices[1] = std::unique_ptr<PieSlice>(replacementPieSlice);
			}
			RTEAssert(replacedPieSlice, "Tried to do PieSlice replacement in PieQuadrant, but PieSlice to replace was not found and removed from any PieQuadrant.");
		};

		if (Directions sliceDirection = pieSliceToReplace->GetDirection(); sliceDirection > Directions::None) {
			if (sliceDirection == Directions::Any) {
				for (PieQuadrant &pieQuadrant : m_PieQuadrants) {
					if (pieQuadrant.ContainsPieSlice(pieSliceToReplace)) {
						DoPieSliceReplacementInPieQuadrant(pieQuadrant);
						break;
					}
				}
			} else if (PieQuadrant &pieQuadrant = m_PieQuadrants[sliceDirection]; pieQuadrant.ContainsPieSlice(pieSliceToReplace)) {
				DoPieSliceReplacementInPieQuadrant(pieQuadrant);
			}
		}

		if (replacedPieSlice) {
			replacementPieSlice->SetOriginalSource(pieSliceToReplace->GetOriginalSource());
			replacementPieSlice->SetDirection(pieSliceToReplace->GetDirection());
			replacementPieSlice->SetCanBeMiddleSlice(pieSliceToReplace->GetCanBeMiddleSlice());
			replacementPieSlice->SetStartAngle(pieSliceToReplace->GetStartAngle());
			replacementPieSlice->SetSlotCount(pieSliceToReplace->GetSlotCount());
			replacementPieSlice->SetMidAngle(pieSliceToReplace->GetMidAngle());

			if (m_HoveredPieSlice == pieSliceToReplace) {
				m_HoveredPieSlice = replacementPieSlice;
			}
			if (m_ActivatedPieSlice == pieSliceToReplace) {
				m_ActivatedPieSlice = replacedPieSlice;
			}
			if (m_AlreadyActivatedPieSlice == pieSliceToReplace) {
				m_AlreadyActivatedPieSlice = replacementPieSlice;
			}

			RepopulateAndRealignCurrentPieSlices();
		}

		return replacedPieSlice;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void PieMenu::Update() {
		const Controller *controller = GetController();

		m_ActivatedPieSlice = nullptr;

		if (m_Owner) {
			SetPos(m_Owner->GetCPUPos());
		} else if (m_AffectedObject) {
			const Actor *affectedObjectAsActor = dynamic_cast<Actor *>(m_AffectedObject);
			SetPos(affectedObjectAsActor ? affectedObjectAsActor->GetCPUPos() : m_AffectedObject->GetPos());
		}

		if (m_MenuMode == MenuMode::Wobble) {
			UpdateWobbling();
		} else if (m_MenuMode == MenuMode::Freeze) {
			m_EnabledState = EnabledState::Enabling;
		} else if (m_EnabledState == EnabledState::Enabling || m_EnabledState == EnabledState::Disabling) {
			UpdateEnablingAndDisablingProgress();
		}

		if (controller->IsDisabled()) {
			SetEnabled(false);
			return;
		}

		if (m_AffectedObject && !g_MovableMan.ValidMO(m_AffectedObject)) { m_AffectedObject = nullptr; }

		if (m_MenuMode == MenuMode::Normal) {
			if (IsEnabled()) {
				for (const auto &[listenerObject, listenerFunction] : m_WhilePieMenuOpenListeners) { listenerFunction(); }

				bool anyInput = false;
				bool skipInputBecauseActiveSubPieMenuWasJustDisabled = false;
				if (m_ActiveSubPieMenu) {
					m_CursorAngle = m_HoveredPieSlice->GetMidAngle() + GetRotAngle();
					m_CursorInVisiblePosition = false;
					m_HoverTimer.Reset();
					if (m_ActiveSubPieMenu->IsVisible()) {
						m_ActiveSubPieMenu->Update();
					}
					if (!m_ActiveSubPieMenu->IsEnabled()) {
						m_ActivatedPieSlice = m_ActiveSubPieMenu->m_ActivatedPieSlice;
						Directions activeSubPieMenuDirection = m_ActiveSubPieMenu->m_DirectionIfSubPieMenu;
						m_ActiveSubPieMenu = nullptr;
						m_SubPieMenuHoverOpenTimer.Reset();
						m_HoverTimer.SetRealTimeLimitMS(2000);
						if (IsSubPieMenu()) {
							PrepareAnalogCursorForEnableOrDisable(true);
							m_CursorInVisiblePosition = true;
						} else {
							bool shouldClearHoveredSlice = controller->IsState(ControlState::PIE_MENU_ACTIVE_ANALOG);
							// If a keyboard-only sub-PieMenu is exited by going off the sides, the parent PieMenu should handle input so the next PieSlice can be naturally stepped to.
							if (activeSubPieMenuDirection != Directions::None) {
								for (const auto &[controlState, controlStateDirection] : c_ControlStateDirections) {
									if (controlStateDirection == c_OppositeDirections.at(activeSubPieMenuDirection) && controller->IsState(controlState)) {
										shouldClearHoveredSlice = true;
										break;
									}
								}
							}
							if (shouldClearHoveredSlice) {
								SetHoveredPieSlice(nullptr);
								skipInputBecauseActiveSubPieMenuWasJustDisabled = true;
							}
						}
					}
				}
				if (!m_ActiveSubPieMenu && !skipInputBecauseActiveSubPieMenuWasJustDisabled) {
					if (controller->IsState(PIE_MENU_ACTIVE_ANALOG)) {
						anyInput = HandleAnalogInput(controller->GetAnalogCursor());
					} else if (controller->IsState(PIE_MENU_ACTIVE_DIGITAL)) {
						anyInput = HandleDigitalInput();
					}
				}

				if (anyInput) {
					m_HoverTimer.Reset();
				}

				if (!IsSubPieMenu() && m_HoverTimer.IsPastRealTimeLimit()) {
					SetHoveredPieSlice(nullptr);
				}
			}

			if (m_HoveredPieSlice && m_EnabledState != EnabledState::Disabled && !m_ActiveSubPieMenu) { UpdateSliceActivation(); }

			if (!IsSubPieMenu()) { SetEnabled(controller->IsState(ControlState::PIE_MENU_ACTIVE)); }
		}

		if (m_BGBitmapNeedsRedrawing && m_EnabledState != EnabledState::Disabled) { UpdatePredrawnMenuBackgroundBitmap(); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenu::Draw(BITMAP *targetBitmap, const Vector &targetPos) const {
		Vector drawPos;
		CalculateDrawPosition(targetBitmap, targetPos, drawPos);

		if (m_EnabledState != EnabledState::Disabled) {
			if (m_DrawBackgroundTransparent && !g_FrameMan.IsInMultiplayerMode()) {
				g_FrameMan.SetTransTableFromPreset(TransparencyPreset::MoreTrans);
				draw_trans_sprite(targetBitmap, m_BGBitmap, drawPos.GetFloorIntX() - m_BGBitmap->w / 2, drawPos.GetFloorIntY() - m_BGBitmap->h / 2);
			} else {
				draw_sprite(targetBitmap, m_BGBitmap, drawPos.GetFloorIntX() - m_BGBitmap->w / 2, drawPos.GetFloorIntY() - m_BGBitmap->h / 2);
			}
		}

		if (m_EnabledState == EnabledState::Enabled) {
			DrawPieIcons(targetBitmap, drawPos);
			if (m_CursorInVisiblePosition) { DrawPieCursorAndPieSliceDescriptions(targetBitmap, drawPos); }
		}

		if (m_ActiveSubPieMenu) { m_ActiveSubPieMenu->Draw(targetBitmap, targetPos); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenu::UpdateWobbling() {
		float innerRadiusChange = static_cast<float>(m_EnableDisableAnimationTimer.GetElapsedRealTimeMS()) / 6.0F;

		m_BGBitmapNeedsRedrawing = true;
		m_CurrentInnerRadius += static_cast<int>(innerRadiusChange) * (m_EnabledState == EnabledState::Disabling ? -1 : 1);

		if (m_CurrentInnerRadius < 0) {
			m_EnabledState = EnabledState::Enabling;
		} else if (m_CurrentInnerRadius > m_FullInnerRadius / 2) {
			m_EnabledState = EnabledState::Disabling;
		}

		m_CurrentInnerRadius = std::clamp(m_CurrentInnerRadius, 0, m_FullInnerRadius / 2);
		m_EnableDisableAnimationTimer.Reset();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenu::UpdateEnablingAndDisablingProgress() {
		m_BGBitmapNeedsRedrawing = true;
		if (m_EnabledState == EnabledState::Enabling) {
			m_CurrentInnerRadius = static_cast<int>(Lerp(0.0F, static_cast<float>(c_EnablingDelay), 0.0F, static_cast<float>(m_FullInnerRadius), static_cast<float>(m_EnableDisableAnimationTimer.GetElapsedRealTimeMS())));
			if (IsSubPieMenu() || m_EnableDisableAnimationTimer.IsPastRealMS(c_EnablingDelay)) {
				m_EnabledState = EnabledState::Enabled;
				m_CurrentInnerRadius = m_FullInnerRadius;
				m_SubPieMenuHoverOpenTimer.Reset();
			}
		} else if (m_EnabledState == EnabledState::Disabling) {
			m_CurrentInnerRadius = static_cast<int>(Lerp(0.0F, static_cast<float>(c_EnablingDelay), static_cast<float>(m_FullInnerRadius), 0.0F, static_cast<float>(m_EnableDisableAnimationTimer.GetElapsedRealTimeMS())));
			if (IsSubPieMenu() || m_EnableDisableAnimationTimer.IsPastRealMS(c_EnablingDelay)) {
				m_EnabledState = EnabledState::Disabled;
				m_CurrentInnerRadius = 0;
				if (Actor *affectedObjectAsActor = dynamic_cast<Actor *>(m_AffectedObject)) { affectedObjectAsActor->FlashWhite(); }
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PieMenu::HandleAnalogInput(const Vector &input) {
		const Controller *controller = GetController();
		const PieSlice *pieSliceToSelect = nullptr;

		if (input.MagnitudeIsGreaterThan(0.5F)) {
			m_CursorInVisiblePosition = true;
			float normalizedCursorAngle = NormalizeAngleBetween0And2PI(input.GetAbsRadAngle());
			m_CursorAngle = normalizedCursorAngle;

			for (const PieSlice *pieSlice : m_CurrentPieSlices) {
				float absolutePieSliceStartAngle = pieSlice->GetStartAngle() + GetRotAngle();
				if (AngleWithinRange(m_CursorAngle, absolutePieSliceStartAngle, absolutePieSliceStartAngle + (PieQuadrant::c_PieSliceSlotSize * static_cast<float>(pieSlice->GetSlotCount())))) {
					pieSliceToSelect = pieSlice;
					break;
				}
			}
			SetHoveredPieSlice(pieSliceToSelect);
			m_HoverTimer.SetRealTimeLimitMS(controller->IsMouseControlled() ? 50 : 500);
			return true;
		} else if (IsSubPieMenu() && m_HoverTimer.IsPastRealTimeLimit()) {
			SetEnabled(false);
		}

		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PieMenu::HandleDigitalInput() {
		const Controller *controller = GetController();
		if (!controller) {
			return false;
		}

		// Don't allow our analog move to interfere with us - joystick-to-digital input is really awkward and doesn't feel nice
		if (controller->GetAnalogMove().MagnitudeIsGreaterThan(0.1F)) {
			// So instead use the analog input, but use our move instead of our cursor
			return HandleAnalogInput(controller->GetAnalogMove());
		}

		auto GetPieQuadrantContainingHoveredSlice = [this]() {
			if (m_HoveredPieSlice) {
				for (const PieQuadrant &pieQuadrant : m_PieQuadrants) {
					if (pieQuadrant.ContainsPieSlice(m_HoveredPieSlice)) {
						return &pieQuadrant;
					}
				}
			}
			return static_cast<const PieQuadrant *>(nullptr);
		};

		auto MoveToPieQuadrant = [this](const PieQuadrant &pieQuadrant, MoveToPieQuadrantMode moveToPieQuadrantMode = MoveToPieQuadrantMode::Middle) {
			if (pieQuadrant.GetFlattenedPieSlices().empty()) {
				SetHoveredPieSlice(nullptr);
				m_CursorInVisiblePosition = true;
				m_CursorAngle = c_DirectionsToRadiansMap.at(pieQuadrant.m_Direction) + GetRotAngle();
			} else if (moveToPieQuadrantMode == MoveToPieQuadrantMode::Start) {
				SetHoveredPieSlice(*(pieQuadrant.GetFlattenedPieSlices(true).begin()), true);
			} else if (moveToPieQuadrantMode == MoveToPieQuadrantMode::Middle) {
				SetHoveredPieSlice(pieQuadrant.m_MiddlePieSlice ? pieQuadrant.m_MiddlePieSlice.get() : pieQuadrant.m_LeftPieSlices[0].get(), true);
			} else if (moveToPieQuadrantMode == MoveToPieQuadrantMode::End) {
				SetHoveredPieSlice(*(pieQuadrant.GetFlattenedPieSlices(true).end() - 1), true);
			}
		};

		auto StepToNeighbouringPieSlice = [this, &MoveToPieQuadrant](const PieQuadrant *hoveredPieSlicePieQuadrant, Directions controlStateDirection, bool counterClockwiseMovement) {
			std::vector<PieSlice *> flattenedPieSlices = hoveredPieSlicePieQuadrant->GetFlattenedPieSlices(true);
			auto hoveredSliceIterator = std::find(flattenedPieSlices.begin(), flattenedPieSlices.end(), m_HoveredPieSlice);
			if ((counterClockwiseMovement && hoveredSliceIterator == flattenedPieSlices.end() - 1) || (!counterClockwiseMovement && hoveredSliceIterator == flattenedPieSlices.begin())) {
				if (m_PieQuadrants.at(controlStateDirection).m_Enabled) {
					MoveToPieQuadrant(m_PieQuadrants.at(controlStateDirection), counterClockwiseMovement ? MoveToPieQuadrantMode::Start : MoveToPieQuadrantMode::End);
				} else {
					SetEnabled(false);
				}
			} else {
				SetHoveredPieSlice(*(hoveredSliceIterator + (counterClockwiseMovement ? 1 : -1)), true);
			}
		};

		for (const auto &[controlState, controlStateDirection] : c_ControlStateDirections) {
			if (controller->IsState(controlState)) {
				const PieQuadrant &pieQuadrantAtControlStateDirection = m_PieQuadrants.at(controlStateDirection);
				if (!m_HoveredPieSlice && pieQuadrantAtControlStateDirection.m_Enabled) {
					MoveToPieQuadrant(pieQuadrantAtControlStateDirection);
				} else if (const PieQuadrant *hoveredPieSlicePieQuadrant = GetPieQuadrantContainingHoveredSlice()) {
					if (controlStateDirection == c_OppositeDirections.at(hoveredPieSlicePieQuadrant->m_Direction)) {
						if (IsSubPieMenu()) {
							SetEnabled(false);
						} else if (pieQuadrantAtControlStateDirection.m_Enabled) {
							MoveToPieQuadrant(pieQuadrantAtControlStateDirection);
						} else {
							g_GUISound.HoverDisabledSound()->Play();
						}
					} else if (hoveredPieSlicePieQuadrant->m_Direction == controlStateDirection) {
						if (m_HoveredPieSlice == pieQuadrantAtControlStateDirection.m_MiddlePieSlice.get()) {
							if (IsSubPieMenu()) {
								g_GUISound.HoverDisabledSound()->Play();
							} else {
								SetHoveredPieSlice(nullptr);
							}
						} else {
							bool hoveredPieSliceIsInPieQuadrantRightSide = (m_HoveredPieSlice == hoveredPieSlicePieQuadrant->m_RightPieSlices[0].get() || m_HoveredPieSlice == hoveredPieSlicePieQuadrant->m_RightPieSlices[1].get());
							std::vector<PieSlice *> flattenedPieSlices = hoveredPieSlicePieQuadrant->GetFlattenedPieSlices(true);
							auto hoveredPieSliceIterator = std::find(flattenedPieSlices.begin(), flattenedPieSlices.end(), m_HoveredPieSlice);
							SetHoveredPieSlice(*(hoveredPieSliceIterator + (hoveredPieSliceIsInPieQuadrantRightSide ? 1 : -1)), true);
						}
					} else {
						bool counterClockwiseMovement = controlStateDirection == c_CounterClockwiseDirections.at(hoveredPieSlicePieQuadrant->m_Direction);
						const std::array<std::unique_ptr<PieSlice>, PieQuadrant::c_PieQuadrantSlotCount / 2> &pieSlicesToMovePieQuadrantsFor = counterClockwiseMovement ? hoveredPieSlicePieQuadrant->m_RightPieSlices : hoveredPieSlicePieQuadrant->m_LeftPieSlices;
						if (pieQuadrantAtControlStateDirection.m_Enabled && (m_HoveredPieSlice == pieSlicesToMovePieQuadrantsFor[0].get() || m_HoveredPieSlice == pieSlicesToMovePieQuadrantsFor[1].get())) {
							MoveToPieQuadrant(pieQuadrantAtControlStateDirection);
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

	void PieMenu::UpdateSliceActivation() {
		const Controller *controller = GetController();
		if (controller && (controller->IsState(ControlState::PRESS_PRIMARY) || (m_HoveredPieSlice != m_AlreadyActivatedPieSlice && controller->IsState(ControlState::RELEASE_SECONDARY)))) {
			m_HoverTimer.Reset();
			m_ActivatedPieSlice = m_HoveredPieSlice->IsEnabled() ? m_HoveredPieSlice : m_ActivatedPieSlice;
			m_AlreadyActivatedPieSlice = m_ActivatedPieSlice;

			if (m_HoveredPieSlice->GetSubPieMenu() && controller->IsState(ControlState::RELEASE_SECONDARY)) {
				g_GUISound.UserErrorSound()->Play();
			} else {
				SoundContainer *soundToPlay = m_HoveredPieSlice->IsEnabled() ? g_GUISound.SlicePickedSound() : g_GUISound.DisabledPickedSound();
				soundToPlay->Play();
			}
		}

		if (IsEnabled()) {
			if ((m_ActivatedPieSlice && m_ActivatedPieSlice->GetSubPieMenu() != nullptr) || (m_HoveredPieSlice->GetSubPieMenu() && m_SubPieMenuHoverOpenTimer.IsPastRealTimeLimit())) {
				PreparePieSliceSubPieMenuForUse(m_ActivatedPieSlice ? m_ActivatedPieSlice : m_HoveredPieSlice);
				m_ActiveSubPieMenu = m_ActivatedPieSlice ? m_ActivatedPieSlice->GetSubPieMenu() : m_HoveredPieSlice->GetSubPieMenu();
				if (m_Owner) {
					m_ActiveSubPieMenu->SetOwner(m_Owner);
				} else if (m_MenuController) {
					m_ActiveSubPieMenu->SetMenuController(m_MenuController);
					m_ActiveSubPieMenu->SetPos(m_CenterPos);
				}
				m_ActiveSubPieMenu->SetEnabled(true);
				m_ActiveSubPieMenu->SetHoveredPieSlice(m_ActiveSubPieMenu->m_PieQuadrants.at(m_ActiveSubPieMenu->m_DirectionIfSubPieMenu).m_MiddlePieSlice.get(), true);
				m_ActiveSubPieMenu->m_HoverTimer.SetRealTimeLimitMS(2000);
				m_ActiveSubPieMenu->m_HoverTimer.Reset();
			} else if (m_ActivatedPieSlice && m_ActivatedPieSlice->GetLuabindFunctionObjectWrapper() && m_ActivatedPieSlice->GetLuabindFunctionObjectWrapper()->GetLuabindObject()) {
				if (const MovableObject *scriptTarget = m_Owner ? m_Owner : m_AffectedObject) {
					g_LuaMan.GetMasterScriptState().RunScriptFunctionObject(m_ActivatedPieSlice->GetLuabindFunctionObjectWrapper(), "", "", { scriptTarget, this, m_ActivatedPieSlice });
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenu::UpdatePredrawnMenuBackgroundBitmap() {
		int centerX = IsSubPieMenu() ? 0 : m_BGBitmap->w / 2;
		int centerY = IsSubPieMenu() ? 0 : m_BGBitmap->h / 2;
		if (m_DirectionIfSubPieMenu == Directions::Up || m_DirectionIfSubPieMenu == Directions::Left) { centerX = m_BGBitmap->w; }
		if (m_DirectionIfSubPieMenu == Directions::Up || m_DirectionIfSubPieMenu == Directions::Right) { centerY = m_BGBitmap->h; }
		if (m_DirectionIfSubPieMenu == Directions::Down) { centerX = -2; centerY = -2; }
		if (m_DirectionIfSubPieMenu == Directions::Left) { centerY = -2; }
		float subPieMenuRotationOffset = IsSubPieMenu() ? c_QuarterPI : 0;

		bool pieMenuNeedsToBeDrawnRotated = GetRotAngle() - subPieMenuRotationOffset != 0;
		BITMAP *bitmapToDrawTo = pieMenuNeedsToBeDrawnRotated ? m_BGRotationBitmap : m_BGBitmap;

		bool hasPieSliceWithSubPieMenu = false;
		for (const PieSlice *pieSlice : m_CurrentPieSlices) {
			if (pieSlice->GetSubPieMenu()) {
				hasPieSliceWithSubPieMenu = true;
				break;
			}
		}

		clear_to_color(bitmapToDrawTo, ColorKeys::g_MaskColor);
		circlefill(bitmapToDrawTo, centerX, centerY, m_CurrentInnerRadius + m_BackgroundThickness, m_BackgroundColor);
		circlefill(bitmapToDrawTo, centerX, centerY, m_CurrentInnerRadius, ColorKeys::g_MaskColor);

		if (m_EnabledState == EnabledState::Enabled) {
			if (hasPieSliceWithSubPieMenu) {
				clear_to_color(m_BGPieSlicesWithSubPieMenuBitmap, ColorKeys::g_MaskColor);
				circlefill(m_BGPieSlicesWithSubPieMenuBitmap, centerX, centerY, m_CurrentInnerRadius + m_BackgroundThickness + c_PieSliceWithSubPieMenuExtraThickness, m_BackgroundColor);
			}
			DrawBackgroundPieSliceSeparators(bitmapToDrawTo, centerX, centerY, subPieMenuRotationOffset);
			if (hasPieSliceWithSubPieMenu) {
				circlefill(m_BGPieSlicesWithSubPieMenuBitmap, centerX, centerY, m_CurrentInnerRadius + m_BackgroundThickness, ColorKeys::g_MaskColor);
				draw_sprite(m_BGPieSlicesWithSubPieMenuBitmap, bitmapToDrawTo, 0, 0);
				clear_to_color(bitmapToDrawTo, ColorKeys::g_MaskColor);
				draw_sprite(bitmapToDrawTo, m_BGPieSlicesWithSubPieMenuBitmap, 0, 0);
			}
		}

		if (bitmapToDrawTo != m_BGBitmap) {
			clear_to_color(m_BGBitmap, ColorKeys::g_MaskColor);
			float rotationAsAllegroAngle = ((GetRotAngle() - subPieMenuRotationOffset) / c_PI) * -128.0F;
			pivot_sprite(m_BGBitmap, bitmapToDrawTo, m_BGBitmap->w / 2, m_BGBitmap->h / 2, centerX, centerY, ftofix(rotationAsAllegroAngle));
		}
		m_BGBitmapNeedsRedrawing = false;
		m_BGPieSlicesWithSubPieMenuBitmapNeedsRedrawing = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenu::CalculateDrawPosition(const BITMAP *targetBitmap, const Vector &targetPos, Vector &drawPos) const {
		drawPos = m_CenterPos - targetPos;
		if (!targetPos.IsZero()) {
			const Box *nearestBox = nullptr;

			Box screenBox(targetPos, static_cast<float>(targetBitmap->w), static_cast<float>(targetBitmap->h));
			std::list<Box> wrappedBoxes;
			bool withinAnyBox = false;
			float distance = std::numeric_limits<float>::max();
			float shortestDist = std::numeric_limits<float>::max();
			//TODO under what conditions would the pie menu not be on the screen and, if that's the case, would we still want to draw it? Try to remove this in next pass of PieMenu changes, or replace it with more standard wrapping handling.
			// Note - offscreen piemenu is used for signaling selectable actors so it's currently desirable. Strategic mode won't want that clutter, so this can probably change then.
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
		int menuDrawRadius = m_CurrentInnerRadius + m_BackgroundThickness + 2 + m_LargeFont->GetFontHeight();
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

	void PieMenu::DrawPieIcons(BITMAP *targetBitmap, const Vector &drawPos) const {
		for (const PieSlice *pieSlice : m_CurrentPieSlices) {
			BITMAP *pieSliceIcon = pieSlice->GetAppropriateIcon(pieSlice == m_HoveredPieSlice);

			if (pieSliceIcon) {
				float pieSliceRotation = NormalizeAngleBetween0And2PI(pieSlice->GetMidAngle() + GetRotAngle());

				Vector pieSliceCenteringOffset(1.0F - static_cast<float>(pieSliceIcon->w / 2), 1.0F - static_cast<float>(pieSliceIcon->h / 2));
				if (GetRotAngle() == 0 && pieSlice == m_PieQuadrants[Directions::Right].m_MiddlePieSlice.get()) { pieSliceCenteringOffset.SetX(pieSliceCenteringOffset.GetX() - 1.0F); }
				if (GetRotAngle() == 0 && pieSlice == m_PieQuadrants[Directions::Down].m_MiddlePieSlice.get()) { pieSliceCenteringOffset.SetY(pieSliceCenteringOffset.GetY() - 1.0F); }

				Vector pieSliceIconOffset = Vector(static_cast<float>(m_CurrentInnerRadius + (m_BackgroundThickness / 2) + (pieSlice->GetSubPieMenu() && m_IconSeparatorMode == IconSeparatorMode::Line ? 2 : 0)), 0).RadRotate(pieSliceRotation) + pieSliceCenteringOffset;

				if (!pieSlice->GetDrawFlippedToMatchAbsoluteAngle() || (pieSliceRotation >= 0 && pieSliceRotation < (c_HalfPI + 0.001F))) {
					draw_sprite(targetBitmap, pieSliceIcon, drawPos.GetFloorIntX() + pieSliceIconOffset.GetFloorIntX(), drawPos.GetFloorIntY() + pieSliceIconOffset.GetFloorIntY());
				} else if (pieSliceRotation < (c_PI + 0.001F)) {
					draw_sprite_h_flip(targetBitmap, pieSliceIcon, drawPos.GetFloorIntX() + pieSliceIconOffset.GetFloorIntX(), drawPos.GetFloorIntY() + pieSliceIconOffset.GetFloorIntY());
				} else if (pieSliceRotation < (c_OneAndAHalfPI + 0.001F)) {
					draw_sprite_vh_flip(targetBitmap, pieSliceIcon, drawPos.GetFloorIntX() + pieSliceIconOffset.GetFloorIntX(), drawPos.GetFloorIntY() + pieSliceIconOffset.GetFloorIntY());
				} else {
					draw_sprite_v_flip(targetBitmap, pieSliceIcon, drawPos.GetFloorIntX() + pieSliceIconOffset.GetFloorIntX(), drawPos.GetFloorIntY() + pieSliceIconOffset.GetFloorIntY());
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenu::DrawPieCursorAndPieSliceDescriptions(BITMAP *targetBitmap, const Vector &drawPos) const {
		int nonLineSeparatorCorrection = m_IconSeparatorMode != IconSeparatorMode::Line ? -(m_BackgroundSeparatorSize) : 0;
		Vector cursorPos = Vector(static_cast<float>(m_CurrentInnerRadius + nonLineSeparatorCorrection), 0.0F).RadRotate(m_CursorAngle);
		pivot_sprite(targetBitmap, s_CursorBitmap, drawPos.GetFloorIntX() + cursorPos.GetFloorIntX(), drawPos.GetFloorIntY() + cursorPos.GetFloorIntY(), s_CursorBitmap->w / 2, s_CursorBitmap->h / 2, ftofix((m_CursorAngle / c_PI) * -128.0F));

		if (m_HoveredPieSlice) {
			float textRotation = NormalizeAngleBetween0And2PI(m_HoveredPieSlice->GetMidAngle() + GetRotAngle());
			Vector textCenteringOffset(0.0F, static_cast<float>(m_LargeFont->GetFontHeight()) * 0.45F);
			Vector textPos = Vector(static_cast<float>(m_CurrentInnerRadius + std::max(m_BackgroundThickness, m_BackgroundSeparatorSize) + (m_LargeFont->GetFontHeight() * 0.5)), 0).RadRotate(textRotation) - textCenteringOffset;

			AllegroBitmap allegroBitmap(targetBitmap);
			if (GetRotAngle() <= c_EighthPI && (m_HoveredPieSlice == m_PieQuadrants[Directions::Up].m_MiddlePieSlice.get() || m_HoveredPieSlice == m_PieQuadrants[Directions::Down].m_MiddlePieSlice.get())) {
				m_LargeFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() + textPos.GetFloorIntX(), drawPos.GetFloorIntY() + textPos.GetFloorIntY(), m_HoveredPieSlice->GetDescription().c_str(), GUIFont::Centre);
			} else if (textRotation < c_HalfPI - 0.01 || textRotation > c_PI + c_HalfPI + 0.01) {
				m_LargeFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() + textPos.GetFloorIntX(), drawPos.GetFloorIntY() + textPos.GetFloorIntY(), m_HoveredPieSlice->GetDescription().c_str(), GUIFont::Left);
			} else {
				m_LargeFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() + textPos.GetFloorIntX(), drawPos.GetFloorIntY() + textPos.GetFloorIntY(), m_HoveredPieSlice->GetDescription().c_str(), GUIFont::Right);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenu::RepopulateAndRealignCurrentPieSlices() {
		m_CurrentPieSlices.clear();
		for (PieQuadrant &pieQuadrant : m_PieQuadrants) {
			pieQuadrant.RealignPieSlices();
			if (pieQuadrant.m_MiddlePieSlice) { m_CurrentPieSlices.emplace_back(pieQuadrant.m_MiddlePieSlice.get()); }
		}
		for (PieQuadrant &pieQuadrant : m_PieQuadrants) {
			if (pieQuadrant.m_LeftPieSlices[0]) { m_CurrentPieSlices.emplace_back(pieQuadrant.m_LeftPieSlices[0].get()); }
		}
		for (PieQuadrant &pieQuadrant : m_PieQuadrants) {
			if (pieQuadrant.m_RightPieSlices[0]) { m_CurrentPieSlices.emplace_back(pieQuadrant.m_RightPieSlices[0].get()); }
		}
		for (PieQuadrant &pieQuadrant : m_PieQuadrants) {
			if (pieQuadrant.m_LeftPieSlices[1]) { m_CurrentPieSlices.emplace_back(pieQuadrant.m_LeftPieSlices[1].get()); }
		}
		for (PieQuadrant &pieQuadrant : m_PieQuadrants) {
			if (pieQuadrant.m_RightPieSlices[1]) { m_CurrentPieSlices.emplace_back(pieQuadrant.m_RightPieSlices[1].get()); }
		}
		ExpandPieSliceIntoEmptySpaceIfPossible();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenu::ExpandPieSliceIntoEmptySpaceIfPossible() {
		const std::unordered_map<Directions, Directions> nextDirectionForGivenDirection{ {Directions::Right, Directions::Up}, {Directions::Up, Directions::Left}, {Directions::Left, Directions::Down}, {Directions::Down, Directions::Right} };

		for (const PieQuadrant &pieQuadrant : m_PieQuadrants) {
			if (pieQuadrant.m_Enabled) {
				const std::unique_ptr<PieSlice> &leftMostPieSlice = !pieQuadrant.m_LeftPieSlices[1] ? pieQuadrant.m_LeftPieSlices[0] : pieQuadrant.m_LeftPieSlices[1];
				PieQuadrant &neighbouringPieQuadrant = m_PieQuadrants.at(nextDirectionForGivenDirection.at(pieQuadrant.m_Direction));

				if (leftMostPieSlice && leftMostPieSlice->GetSlotCount() == 1 && neighbouringPieQuadrant.m_Enabled && neighbouringPieQuadrant.m_SlotsForPieSlices[0] == nullptr) {
					leftMostPieSlice->SetSlotCount(leftMostPieSlice->GetSlotCount() + 1);
					neighbouringPieQuadrant.m_SlotsForPieSlices[0] = leftMostPieSlice.get();
				}
			}
		}
		m_BGBitmapNeedsRedrawing = true;
		m_BGPieSlicesWithSubPieMenuBitmapNeedsRedrawing = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenu::RecreateBackgroundBitmaps() {
		if (m_BGBitmap) {
			destroy_bitmap(m_BGBitmap);
			m_BGBitmap = nullptr;
		}
		if (m_BGRotationBitmap) {
			destroy_bitmap(m_BGRotationBitmap);
			m_BGRotationBitmap = nullptr;
		}
		if (m_BGPieSlicesWithSubPieMenuBitmap) {
			destroy_bitmap(m_BGPieSlicesWithSubPieMenuBitmap);
			m_BGPieSlicesWithSubPieMenuBitmap = nullptr;
		}

		int diameter = (m_FullInnerRadius + std::max(m_BackgroundThickness, m_BackgroundSeparatorSize) + (c_PieSliceWithSubPieMenuExtraThickness * 2)) * 2;

		m_BGBitmap = create_bitmap_ex(8, diameter, diameter);
		clear_to_color(m_BGBitmap, ColorKeys::g_MaskColor);
		m_BGRotationBitmap = create_bitmap_ex(8, diameter, diameter);
		clear_to_color(m_BGRotationBitmap, ColorKeys::g_MaskColor);
		m_BGPieSlicesWithSubPieMenuBitmap = create_bitmap_ex(8, diameter, diameter);
		clear_to_color(m_BGPieSlicesWithSubPieMenuBitmap, ColorKeys::g_MaskColor);

		m_BGBitmapNeedsRedrawing = true;
		m_BGPieSlicesWithSubPieMenuBitmapNeedsRedrawing = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenu::DrawBackgroundPieSliceSeparators(BITMAP *backgroundBitmapToDrawTo, int pieCircleCenterX, int pieCircleCenterY, float subPieMenuRotationOffset) const {
		switch (m_IconSeparatorMode) {
			case IconSeparatorMode::Line:
				for (const PieQuadrant &pieQuadrant : m_PieQuadrants) {
					float currentAngle = c_DirectionsToRadiansMap.at(pieQuadrant.m_Direction) - c_QuarterPI + subPieMenuRotationOffset;
					bool pieQuadrantIsEmpty = !pieQuadrant.m_Enabled || pieQuadrant.GetFlattenedPieSlices().empty();
					for (int currentSlot = 0; currentSlot < PieQuadrant::c_PieQuadrantSlotCount;) {
						if (const PieSlice *pieSliceInSlot = pieQuadrant.m_SlotsForPieSlices.at(currentSlot)) {
							// We don't draw lines if the current slot is the first slot, since a line will be drawn by the previous PieQuadrant's last PieSlice, or if it's a 2-size, since that means it's spreading over PieQuadrants and a line will be drawn at the next PieSlice's start.
							if (currentSlot > 0 || pieSliceInSlot->GetSlotCount() != 2) {
								currentAngle = pieSliceInSlot->GetStartAngle() + subPieMenuRotationOffset;
								DrawBackgroundPieSliceSeparator(backgroundBitmapToDrawTo, pieCircleCenterX, pieCircleCenterY, currentAngle, false, pieSliceInSlot->GetSubPieMenu());
								currentSlot += pieSliceInSlot->GetSlotCount();
								DrawBackgroundPieSliceSeparator(backgroundBitmapToDrawTo, pieCircleCenterX, pieCircleCenterY, currentAngle + (static_cast<float>(pieSliceInSlot->GetSlotCount()) * PieQuadrant::c_PieSliceSlotSize), pieSliceInSlot == m_HoveredPieSlice && m_HoveredPieSlice->IsEnabled(), pieSliceInSlot->GetSubPieMenu(), IsSubPieMenu() && currentSlot == PieQuadrant::c_PieQuadrantSlotCount);
								continue;
							}
						} else {
							if (!pieQuadrantIsEmpty && currentSlot == 0) { DrawBackgroundPieSliceSeparator(backgroundBitmapToDrawTo, pieCircleCenterX, pieCircleCenterY, currentAngle, false, false); }
							currentAngle += PieQuadrant::c_PieSliceSlotSize;
						}
						currentSlot++;
					}
				}
				for (const PieSlice *pieSlice : m_CurrentPieSlices) {
					if (!pieSlice->GetSubPieMenu()) {
						Vector floodfillPosition = Vector(static_cast<float>(m_CurrentInnerRadius + (m_BackgroundThickness / 2)), 0).RadRotate(pieSlice->GetMidAngle() + subPieMenuRotationOffset);
						floodfill(m_BGPieSlicesWithSubPieMenuBitmap, pieCircleCenterX + floodfillPosition.GetFloorIntX(), pieCircleCenterY + floodfillPosition.GetFloorIntY(), ColorKeys::g_MaskColor);
					}
				}
				break;
			case IconSeparatorMode::Circle:
			case IconSeparatorMode::Square:
				for (const PieSlice *pieSlice : m_CurrentPieSlices) {
					if (pieSlice->GetType() != PieSlice::SliceType::NoType) { DrawBackgroundPieSliceSeparator(backgroundBitmapToDrawTo, pieCircleCenterX, pieCircleCenterY, pieSlice->GetMidAngle() + subPieMenuRotationOffset, pieSlice == m_HoveredPieSlice && pieSlice->IsEnabled(), pieSlice->GetSubPieMenu()); }
				}
				break;
			default:
				RTEAbort("Invalid icon separator mode in PieMenu.");
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenu::DrawBackgroundPieSliceSeparator(BITMAP *backgroundBitmapToDrawTo, int pieCircleCenterX, int pieCircleCenterY, float rotAngle, bool isHoveredPieSlice, bool pieSliceHasSubPieMenu, bool drawHalfSizedSeparator) const {
		Vector separatorOffset;
		int backgroundSeparatorSize = m_BackgroundSeparatorSize;

		switch (m_IconSeparatorMode) {
			case IconSeparatorMode::Line:
				separatorOffset.SetXY(static_cast<float>(m_CurrentInnerRadius + m_BackgroundThickness + backgroundSeparatorSize), 0).RadRotate(rotAngle);
				line(backgroundBitmapToDrawTo, pieCircleCenterX, pieCircleCenterY, pieCircleCenterX + separatorOffset.GetCeilingIntX(), pieCircleCenterY + separatorOffset.GetCeilingIntY(), m_BackgroundBorderColor);
				if (!drawHalfSizedSeparator) { line(backgroundBitmapToDrawTo, pieCircleCenterX + 1, pieCircleCenterY, pieCircleCenterX + 1 + separatorOffset.GetCeilingIntX(), pieCircleCenterY + separatorOffset.GetCeilingIntY(), m_BackgroundBorderColor); }
				line(backgroundBitmapToDrawTo, pieCircleCenterX, pieCircleCenterY + 1, pieCircleCenterX + separatorOffset.GetCeilingIntX(), pieCircleCenterY + 1 + separatorOffset.GetCeilingIntY(), m_BackgroundBorderColor);
				if (!drawHalfSizedSeparator) { line(backgroundBitmapToDrawTo, pieCircleCenterX + 1, pieCircleCenterY + 1, pieCircleCenterX + 1 + separatorOffset.GetCeilingIntX(), pieCircleCenterY + 1 + separatorOffset.GetCeilingIntY(), m_BackgroundBorderColor); }

				if (pieSliceHasSubPieMenu) {
					separatorOffset.SetXY(static_cast<float>(m_CurrentInnerRadius + m_BackgroundThickness + backgroundSeparatorSize + c_PieSliceWithSubPieMenuExtraThickness), 0).RadRotate(rotAngle);
					line(m_BGPieSlicesWithSubPieMenuBitmap, pieCircleCenterX, pieCircleCenterY, pieCircleCenterX + separatorOffset.GetCeilingIntX(), pieCircleCenterY + separatorOffset.GetCeilingIntY(), m_BackgroundBorderColor);
					if (!drawHalfSizedSeparator) { line(m_BGPieSlicesWithSubPieMenuBitmap, pieCircleCenterX + 1, pieCircleCenterY, pieCircleCenterX + 1 + separatorOffset.GetCeilingIntX(), pieCircleCenterY + separatorOffset.GetCeilingIntY(), m_BackgroundBorderColor); }
					line(m_BGPieSlicesWithSubPieMenuBitmap, pieCircleCenterX, pieCircleCenterY + 1, pieCircleCenterX + separatorOffset.GetCeilingIntX(), pieCircleCenterY + 1 + separatorOffset.GetCeilingIntY(), m_BackgroundBorderColor);
					if (!drawHalfSizedSeparator) { line(m_BGPieSlicesWithSubPieMenuBitmap, pieCircleCenterX + 1, pieCircleCenterY + 1, pieCircleCenterX + 1 + separatorOffset.GetCeilingIntX(), pieCircleCenterY + 1 + separatorOffset.GetCeilingIntY(), m_BackgroundBorderColor); }
				}

				if (isHoveredPieSlice) {
					separatorOffset.SetXY(static_cast<float>(m_CurrentInnerRadius + (m_BackgroundThickness / 2)), 0).RadRotate(rotAngle - (PieQuadrant::c_PieSliceSlotSize / 2.0F));
					floodfill(backgroundBitmapToDrawTo, pieCircleCenterX + separatorOffset.GetFloorIntX(), pieCircleCenterY + separatorOffset.GetFloorIntY(), m_SelectedItemBackgroundColor);
					if (pieSliceHasSubPieMenu) { floodfill(m_BGPieSlicesWithSubPieMenuBitmap, pieCircleCenterX + separatorOffset.GetFloorIntX(), pieCircleCenterY + separatorOffset.GetFloorIntY(), m_SelectedItemBackgroundColor); }
				}
				break;
			case IconSeparatorMode::Circle:
			case IconSeparatorMode::Square:
				if (drawHalfSizedSeparator) { backgroundSeparatorSize /= 2; }
				separatorOffset.SetXY(static_cast<float>(m_CurrentInnerRadius) + (static_cast<float>(m_BackgroundThickness) / 2.0F), 0).RadRotate(rotAngle);
				if (pieSliceHasSubPieMenu) {
					if (m_IconSeparatorMode == IconSeparatorMode::Circle) {
						circlefill(backgroundBitmapToDrawTo, pieCircleCenterX + separatorOffset.GetFloorIntX(), pieCircleCenterY + separatorOffset.GetFloorIntY(), backgroundSeparatorSize + 2, m_BackgroundBorderColor);
						circlefill(backgroundBitmapToDrawTo, pieCircleCenterX + separatorOffset.GetFloorIntX(), pieCircleCenterY + separatorOffset.GetFloorIntY(), backgroundSeparatorSize + 1, m_BackgroundColor);
					} else if (m_IconSeparatorMode == IconSeparatorMode::Square) {
						rectfill(backgroundBitmapToDrawTo, pieCircleCenterX + separatorOffset.GetFloorIntX() - (backgroundSeparatorSize + 2), pieCircleCenterY + separatorOffset.GetFloorIntY() - (backgroundSeparatorSize + 2), pieCircleCenterX + separatorOffset.GetFloorIntX() + (backgroundSeparatorSize + 2), pieCircleCenterY + separatorOffset.GetFloorIntY() + (backgroundSeparatorSize + 2), m_BackgroundBorderColor);
						rectfill(backgroundBitmapToDrawTo, pieCircleCenterX + separatorOffset.GetFloorIntX() - (backgroundSeparatorSize + 1), pieCircleCenterY + separatorOffset.GetFloorIntY() - (backgroundSeparatorSize + 1), pieCircleCenterX + separatorOffset.GetFloorIntX() + (backgroundSeparatorSize + 1), pieCircleCenterY + separatorOffset.GetFloorIntY() + (backgroundSeparatorSize + 1), m_BackgroundColor);
					}
				}
				if (m_IconSeparatorMode == IconSeparatorMode::Circle) {
					circlefill(backgroundBitmapToDrawTo, pieCircleCenterX + separatorOffset.GetFloorIntX(), pieCircleCenterY + separatorOffset.GetFloorIntY(), backgroundSeparatorSize, m_BackgroundBorderColor);
					circlefill(backgroundBitmapToDrawTo, pieCircleCenterX + separatorOffset.GetFloorIntX(), pieCircleCenterY + separatorOffset.GetFloorIntY(), backgroundSeparatorSize - 1, isHoveredPieSlice ? m_SelectedItemBackgroundColor : m_BackgroundColor);
				} else if (m_IconSeparatorMode == IconSeparatorMode::Square) {
					rectfill(backgroundBitmapToDrawTo, pieCircleCenterX + separatorOffset.GetFloorIntX() - backgroundSeparatorSize, pieCircleCenterY + separatorOffset.GetFloorIntY() - backgroundSeparatorSize, pieCircleCenterX + separatorOffset.GetFloorIntX() + backgroundSeparatorSize, pieCircleCenterY + separatorOffset.GetFloorIntY() + backgroundSeparatorSize, m_BackgroundBorderColor);
					rectfill(backgroundBitmapToDrawTo, pieCircleCenterX + separatorOffset.GetFloorIntX() - (backgroundSeparatorSize - 1), pieCircleCenterY + separatorOffset.GetFloorIntY() - (backgroundSeparatorSize - 1), pieCircleCenterX + separatorOffset.GetFloorIntX() + (backgroundSeparatorSize - 1), pieCircleCenterY + separatorOffset.GetFloorIntY() + (backgroundSeparatorSize - 1), isHoveredPieSlice ? m_SelectedItemBackgroundColor : m_BackgroundColor);
				}
				break;
			default:
				RTEAbort("Invalid icon separator mode in PieMenu.");
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PieMenu::SetHoveredPieSlice(const PieSlice *pieSliceToSelect, bool moveCursorIconToSlice) {
		if (pieSliceToSelect == m_HoveredPieSlice) {
			return false;
		}

		m_HoveredPieSlice = pieSliceToSelect;
		m_SubPieMenuHoverOpenTimer.Reset();
		m_BGBitmapNeedsRedrawing = true;

		if (pieSliceToSelect) {
			if (moveCursorIconToSlice) {
				m_CursorInVisiblePosition = true;
				m_CursorAngle = GetRotAngle() + m_HoveredPieSlice->GetMidAngle();
			}

			SoundContainer *soundToPlay = pieSliceToSelect->IsEnabled() ? g_GUISound.HoverChangeSound() : g_GUISound.HoverDisabledSound();
			soundToPlay->Play();
		} else {
			m_CursorInVisiblePosition = false;
		}
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PieMenu::PreparePieSliceSubPieMenuForUse(const PieSlice *pieSliceWithSubPieMenu) const {
		PieMenu *subPieMenu = pieSliceWithSubPieMenu->GetSubPieMenu();
		subPieMenu->m_ActivatedPieSlice = nullptr;
		if (!subPieMenu || subPieMenu->IsSubPieMenu()) {
			return false;
		}
		for (const PieQuadrant &pieQuadrant : m_PieQuadrants) {
			if (pieQuadrant.ContainsPieSlice(pieSliceWithSubPieMenu)) {
				subPieMenu->m_DirectionIfSubPieMenu = pieQuadrant.m_Direction;
				break;
			}
		}

		float subPieMenuRotAngle = NormalizeAngleBetween0And2PI(pieSliceWithSubPieMenu->GetMidAngle() - c_DirectionsToRadiansMap.at(subPieMenu->m_DirectionIfSubPieMenu) + GetRotAngle());
		if (subPieMenuRotAngle < 0.0001F) { subPieMenuRotAngle = 0; }
		subPieMenu->SetRotAngle(subPieMenuRotAngle);
		subPieMenu->SetFullInnerRadius(m_FullInnerRadius + std::max(m_BackgroundThickness * 2, m_BackgroundSeparatorSize * 3) + c_PieSliceWithSubPieMenuExtraThickness);

		for (PieQuadrant &pieQuadrant : subPieMenu->m_PieQuadrants) {
			if (pieQuadrant.m_Direction != subPieMenu->m_DirectionIfSubPieMenu) { pieQuadrant.m_Enabled = false; }
		}
		std::vector<PieSlice *> existingPieSlices = subPieMenu->GetPieSlices();
		std::vector<PieSlice *> pieSlicesToReadd;
		pieSlicesToReadd.reserve(existingPieSlices.size());
		for (const PieSlice *existingPieSlice : existingPieSlices) {
			pieSlicesToReadd.emplace_back(subPieMenu->RemovePieSlice(existingPieSlice));
			pieSlicesToReadd.back()->SetDirection(Directions::Any);
		}
		for (PieSlice *pieSlice : pieSlicesToReadd) {
			subPieMenu->AddPieSlice(pieSlice, subPieMenu);
		}

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieMenu::PrepareAnalogCursorForEnableOrDisable(bool enable) const {
		if (Controller *controller = GetController(); controller && (controller->IsMouseControlled() || controller->IsGamepadControlled())) {
			if (!IsSubPieMenu()) {
				g_UInputMan.SetMouseValueMagnitude(0, controller->GetPlayer());
				controller->m_AnalogCursor.Reset();
			} else if (enable) {
				controller->SetAnalogCursorAngleLimits(GetRotAngle() + c_DirectionsToRadiansMap.at(m_DirectionIfSubPieMenu) - c_QuarterPI + (PieQuadrant::c_PieSliceSlotSize / 2.0F), GetRotAngle() + c_DirectionsToRadiansMap.at(m_DirectionIfSubPieMenu) + c_QuarterPI - (PieQuadrant::c_PieSliceSlotSize / 2.0F));
				if (!controller->m_AnalogCursor.IsZero()) {
					float mouseAngleToSet = GetRotAngle() + (m_HoveredPieSlice ? m_HoveredPieSlice->GetMidAngle() :  c_DirectionsToRadiansMap.at(m_DirectionIfSubPieMenu));
					g_UInputMan.SetMouseValueAngle(mouseAngleToSet, controller->GetPlayer());
					g_UInputMan.SetMouseValueMagnitude(0.75F, controller->GetPlayer());
					controller->m_AnalogCursor.SetAbsRadAngle(mouseAngleToSet);
					controller->m_AnalogCursor.SetMagnitude(0.75F);
				}
			} else {
				controller->ClearAnalogCursorAngleLimits();
			}
		}
	}
}
