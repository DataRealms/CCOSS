#include "InventoryMenuGUI.h"

#include "FrameMan.h"
#include "UInputMan.h"

#include "Controller.h"
#include "AHuman.h"
#include "ContentFile.h"

#include "GUI/GUI.h"
#include "GUI/GUIFont.h"
#include "GUI/AllegroBitmap.h"

namespace RTE {

	const Vector InventoryMenuGUI::c_CarouselBoxMaxSize(50, 32);
	const Vector InventoryMenuGUI::c_CarouselBoxMinSize(40, 24);
	const Vector InventoryMenuGUI::c_CarouselBoxSizeStep = (c_CarouselBoxMaxSize - c_CarouselBoxMinSize) / (c_ItemsPerRow / 2);
	const int InventoryMenuGUI::c_CarouselBoxCornerRadius = ((c_CarouselBoxMaxSize.GetFloorIntY() - c_CarouselBoxMinSize.GetFloorIntY()) / 2) - 1;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InventoryMenuGUI::CarouselItemBox::GetIconsAndMass(std::vector<BITMAP *> &itemIcons, float &totalItemMass, const std::vector<MovableObject *> *equippedItems) const {
		if (IsForEquippedItems) {
			itemIcons.reserve(equippedItems->size());
			for (const MovableObject *equippedItem : *equippedItems) {
				if (equippedItem) {
					itemIcons.push_back(equippedItem->GetGraphicalIcon());
					totalItemMass += equippedItem->GetMass();
				}
			}
		} else {
			itemIcons.push_back(Item->GetGraphicalIcon());
			totalItemMass += Item->GetMass();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InventoryMenuGUI::Clear() {
		m_SmallFont = nullptr;
		m_LargeFont = nullptr;

		m_ActivityPlayerController = nullptr;
		m_InventoryActor = nullptr;
		m_InventoryActorIsHuman = false;

		m_MenuMode = MenuMode::Carousel;
		m_DisplayOnly = false;

		m_EnabledState = EnabledState::Disabled;
		m_EnableDisableProgress = 0;
		m_CenterPos.Reset();

		m_CarouselDrawEmptyBoxes = true;
		m_CarouselBackgroundTransparent = true;
		m_CarouselBackgroundBoxColor = 4;
		m_CarouselBackgroundBoxBorderSize = Vector(2, 1);
		m_CarouselBackgroundBoxBorderColor = g_MaskColor;
		
		m_CarouselAnimationDirection = CarouselAnimationDirection::None;
		m_CarouselAnimationProgress = 0;
		for (std::unique_ptr<CarouselItemBox> &carouselItemBox : m_CarouselItemBoxes) { carouselItemBox = nullptr; }
		m_CarouselExitingItemBox.reset();

		m_CarouselBGBitmap = nullptr;
		m_CarouselBitmap = nullptr;

		m_InventoryObjectRows.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int InventoryMenuGUI::Create(Controller *activityPlayerController, Actor *inventoryActor, MenuMode menuMode) {
		RTEAssert(activityPlayerController, "No controller sent to InventoryMenuGUI on creation!");
		RTEAssert(c_ItemsPerRow % 2 == 1, "Don't you dare use an even number of items per inventory row, you filthy animal!");

		m_SmallFont = g_FrameMan.GetSmallFont();
		m_LargeFont = g_FrameMan.GetLargeFont();

		m_ActivityPlayerController = activityPlayerController;
		SetInventoryActor(inventoryActor);
		m_MenuMode = menuMode;

		if (!m_CarouselBGBitmap || !m_CarouselBitmap) {
			for (std::unique_ptr<CarouselItemBox> &carouselItemBox : m_CarouselItemBoxes) {
				carouselItemBox = std::make_unique<CarouselItemBox>();
				carouselItemBox->IsForEquippedItems = false;
			}
			std::size_t equippedItemIndex = c_ItemsPerRow / 2;
			Vector currentBoxSize = c_CarouselBoxMaxSize;
			int carouselBitmapWidth = 0;
			for (int i = equippedItemIndex; i >= 0; i--) {
				m_CarouselItemBoxes.at(i)->FullSize = currentBoxSize;
				m_CarouselItemBoxes.at(i)->IconCenterPosition.SetY(c_CarouselBoxMaxSize.GetY() / 2 + static_cast<float>(m_SmallFont->GetFontHeight() / 2));
				carouselBitmapWidth += currentBoxSize.GetFloorIntX();
				if (i != equippedItemIndex) {
					m_CarouselItemBoxes.at((equippedItemIndex * 2) - i)->FullSize = currentBoxSize;
					m_CarouselItemBoxes.at((equippedItemIndex * 2) - i)->IconCenterPosition.SetY(c_CarouselBoxMaxSize.GetY() / 2 + static_cast<float>(m_SmallFont->GetFontHeight() / 2));
					carouselBitmapWidth += currentBoxSize.GetFloorIntX();
				} else {
					m_CarouselItemBoxes.at(i)->IsForEquippedItems = true;
				}
				currentBoxSize -= c_CarouselBoxSizeStep;
			}

			m_CarouselExitingItemBox = std::make_unique<CarouselItemBox>();
			m_CarouselExitingItemBox->IsForEquippedItems = false;
			m_CarouselExitingItemBox->IconCenterPosition.SetY(c_CarouselBoxMaxSize.GetY() / 2 + static_cast<float>(m_SmallFont->GetFontHeight() / 2));
			m_CarouselExitingItemBox->FullSize = c_CarouselBoxMinSize - c_CarouselBoxSizeStep;

			m_CarouselBGBitmap = std::unique_ptr<BITMAP>(create_bitmap_ex(8, carouselBitmapWidth, c_CarouselBoxMaxSize.GetFloorIntY() + m_SmallFont->GetFontHeight() / 2));
			m_CarouselBitmap = std::unique_ptr<BITMAP>(create_bitmap_ex(8, carouselBitmapWidth, c_CarouselBoxMaxSize.GetFloorIntY() + m_SmallFont->GetFontHeight() / 2));
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InventoryMenuGUI::SetEnabled(bool enable) {
		if (!m_ActivityPlayerController || !m_InventoryActor) {
			return;
		}

		bool enabledStateDoesNotMatchInput = (enable && m_EnabledState != EnabledState::Enabled && m_EnabledState != EnabledState::Enabling) || (!enable && m_EnabledState != EnabledState::Disabled && m_EnabledState != EnabledState::Disabling);
		if (enabledStateDoesNotMatchInput) {
			m_EnabledState = enable ? EnabledState::Enabling : EnabledState::Disabling;

			if (m_MenuMode == MenuMode::Full || m_MenuMode == MenuMode::Transfer) {
				if (m_ActivityPlayerController && m_ActivityPlayerController->IsMouseControlled()) {
					g_UInputMan.SetMouseValueMagnitude(0);
					m_ActivityPlayerController->m_AnalogCursor = Vector();
				}
				SoundContainer *soundToPlay = enable ? g_GUISound.EnterMenuSound() : g_GUISound.ExitMenuSound();
				soundToPlay->Play();
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InventoryMenuGUI::SetInventoryActor(Actor *newInventoryActor) {
		m_InventoryActor = newInventoryActor;
		if (m_InventoryActor) {
			m_InventoryActorIsHuman = dynamic_cast<AHuman *>(m_InventoryActor);
			m_CenterPos = m_InventoryActor->GetAboveHUDPos() - Vector(0, c_ItemBoxSize);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InventoryMenuGUI::Update() {
		if (IsEnabled() && (!m_ActivityPlayerController || !m_InventoryActor)) { m_EnabledState = EnabledState::Disabling; }
		if (IsEnablingOrDisabling()) { UpdateEnablingAndDisablingProgress(); }

		if (!IsDisabled()) {
			if (const AHuman *inventoryActorAsAHuman = m_InventoryActorIsHuman ? dynamic_cast<AHuman *>(m_InventoryActor) : nullptr) {
				m_InventoryActorEquippedItems.clear();
				m_InventoryActorEquippedItems.reserve(2);
				if (inventoryActorAsAHuman->GetEquippedItem()) { m_InventoryActorEquippedItems.push_back(inventoryActorAsAHuman->GetEquippedItem()); }
				if (inventoryActorAsAHuman->GetEquippedBGItem()) { m_InventoryActorEquippedItems.push_back(inventoryActorAsAHuman->GetEquippedBGItem()); }
			}
			if (m_InventoryActorEquippedItems.empty() && m_InventoryActor && m_InventoryActor->IsInventoryEmpty()) { m_EnabledState = EnabledState::Disabling; }

			switch (m_MenuMode) {
				case MenuMode::Carousel:
					UpdateCarouselMode();
					break;
				case MenuMode::Full:
					UpdateFullMode();
					break;
				case MenuMode::Transfer:
					UpdateTransferMode();
					break;
				default:
					RTEAbort("Unsupported InventoryMenuGUI MenuMode during Update " + std::to_string(static_cast<int>(m_MenuMode)));
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InventoryMenuGUI::UpdateEnablingAndDisablingProgress() {
		float maxProgressChange = 0.15F;
		if (m_EnabledState == EnabledState::Enabling) {
			m_EnableDisableProgress = std::fminf(m_EnableDisableProgress + maxProgressChange, EaseOut(0, 1.0F, std::fmaxf(0.01F, m_EnableDisableProgress)));
			if (m_EnableDisableProgress >= 0.99F) { m_EnabledState = EnabledState::Enabled; }
		} else {
			m_EnableDisableProgress = std::fmaxf(m_EnableDisableProgress - maxProgressChange, 1.0F - EaseOut(0, 1.0F, 1.0F - std::fminf(0.99F, m_EnableDisableProgress)));
			if (m_EnableDisableProgress <= 0.01F) { m_EnabledState = EnabledState::Disabled; }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InventoryMenuGUI::UpdateCarouselMode() {
		for (const std::unique_ptr<CarouselItemBox> &carouselItemBox : m_CarouselItemBoxes) { carouselItemBox->Item = nullptr; }
		if (const std::deque<MovableObject *> *inventory = m_InventoryActor->GetInventory(); !inventory->empty()) {
			int leftSideItemCount = std::min(static_cast<int>(std::floorf(static_cast<float>(inventory->size()) / 2)), c_ItemsPerRow / 2);
			int rightSideItemCount = std::min(static_cast<int>(std::ceilf(static_cast<float>(inventory->size()) / 2)), c_ItemsPerRow / 2);
			
			int carouselIndex = 0;
			std::vector<MovableObject *> temporaryLeftSideItemsForProperOrdering;
			temporaryLeftSideItemsForProperOrdering.reserve(leftSideItemCount);
			if (leftSideItemCount > 0) {
				std::for_each(inventory->crbegin(), inventory->crbegin() + leftSideItemCount, [&temporaryLeftSideItemsForProperOrdering](MovableObject *carouselItem) {
					temporaryLeftSideItemsForProperOrdering.emplace_back(carouselItem);
				});
			}
			std::for_each(temporaryLeftSideItemsForProperOrdering.crbegin(), temporaryLeftSideItemsForProperOrdering.crend(), [this, &carouselIndex, &leftSideItemCount](MovableObject *carouselItem) {
				m_CarouselItemBoxes.at(carouselIndex + (c_ItemsPerRow / 2) - leftSideItemCount)->Item = carouselItem;
				carouselIndex++;
			});
			carouselIndex = c_ItemsPerRow / 2 + 1;
			if (rightSideItemCount > 0) {
				std::for_each(inventory->cbegin(), inventory->cbegin() + rightSideItemCount, [this, &carouselIndex](MovableObject *carouselItem) {
					m_CarouselItemBoxes.at(carouselIndex)->Item = carouselItem;
					carouselIndex++;
				});
			}

			if (m_InventoryActor->GetController()->IsState(ControlState::WEAPON_CHANGE_PREV)) {
				m_CarouselAnimationDirection = CarouselAnimationDirection::Right;
				m_CarouselAnimationProgress = 0;
				m_CarouselExitingItemBox->Item = m_CarouselItemBoxes.at(m_CarouselItemBoxes.size() - 1)->Item;
			} else if (m_InventoryActor->GetController()->IsState(ControlState::WEAPON_CHANGE_NEXT)) {
				m_CarouselAnimationDirection = CarouselAnimationDirection::Left;
				m_CarouselAnimationProgress = 0;
				m_CarouselExitingItemBox->Item = m_CarouselItemBoxes.at(0)->Item;
			}

			if (m_CarouselAnimationDirection != CarouselAnimationDirection::None) {
				m_CarouselAnimationProgress += 0.1F;
				if (m_CarouselAnimationProgress >= 1.0F) {
					m_CarouselAnimationDirection = CarouselAnimationDirection::None;
					m_CarouselAnimationProgress = 0;
					m_CarouselExitingItemBox->Item = nullptr;
				}
			}
		}
		UpdateCarouselItemBoxSizesAndPositions();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InventoryMenuGUI::UpdateCarouselItemBoxSizesAndPositions() {
		float halfMassFontHeight = static_cast<float>(m_SmallFont->GetFontHeight() / 2);
		int carouselIndex = 0;
		float directionalAnimationProgress = m_CarouselAnimationProgress * static_cast<float>(m_CarouselAnimationDirection);
		float currentBoxHorizontalOffset = (directionalAnimationProgress <= 0 ? directionalAnimationProgress : -(1.0F - directionalAnimationProgress)) * m_CarouselExitingItemBox->FullSize.GetX();

		/// <summary>
		/// Lambda to do all the repetitive boilerplate of setting up a carousel item box.
		/// </summary>
		auto SetupCarouselItemBox = [this, &halfMassFontHeight, &carouselIndex, &currentBoxHorizontalOffset](const std::unique_ptr<CarouselItemBox> &carouselItemBox, bool leftSideRoundedAndBordered, bool rightSideRoundedAndBordered) {
			carouselItemBox->CurrentSize = carouselItemBox->FullSize;
			if (carouselIndex == -1) {
				carouselItemBox->CurrentSize += c_CarouselBoxSizeStep * (1.0F - m_CarouselAnimationProgress);
			} else if (m_CarouselAnimationDirection != CarouselAnimationDirection::None) {
				int animationStartSizeIndex = carouselIndex - static_cast<int>(m_CarouselAnimationDirection);
				if (animationStartSizeIndex < 0 || animationStartSizeIndex >= m_CarouselItemBoxes.size()) {
					carouselItemBox->CurrentSize -= c_CarouselBoxSizeStep * (1.0F - m_CarouselAnimationProgress);
				} else {
					carouselItemBox->CurrentSize += (m_CarouselItemBoxes.at(animationStartSizeIndex)->FullSize - carouselItemBox->CurrentSize) * (1.0F - m_CarouselAnimationProgress);
				}
			}
			carouselItemBox->Pos.SetXY(currentBoxHorizontalOffset, halfMassFontHeight + (c_CarouselBoxMaxSize.GetY() - carouselItemBox->CurrentSize.GetY()) / 2);
			carouselItemBox->IconCenterPosition.SetX(carouselItemBox->Pos.GetX() + (carouselItemBox->CurrentSize.GetX() / 2));
			carouselItemBox->RoundedAndBorderedSides = {leftSideRoundedAndBordered, rightSideRoundedAndBordered};
			carouselIndex++;
			currentBoxHorizontalOffset += carouselItemBox->CurrentSize.GetX();
		};

		if (m_CarouselAnimationDirection == CarouselAnimationDirection::Left) {
			carouselIndex = -1;
			SetupCarouselItemBox(m_CarouselExitingItemBox, true, false);
		}

		for (const std::unique_ptr<CarouselItemBox> &carouselItemBox : m_CarouselItemBoxes) {
			std::pair<bool, bool> roundedAndBorderedSides;
			if (carouselIndex < c_ItemsPerRow / 2) {
				roundedAndBorderedSides = {true, m_CarouselAnimationDirection == CarouselAnimationDirection::Left && carouselIndex == (c_ItemsPerRow / 2) - 1 && directionalAnimationProgress > -0.5F};
			} else if (carouselIndex == c_ItemsPerRow / 2) {
				roundedAndBorderedSides = {directionalAnimationProgress >= 0 || directionalAnimationProgress <= -0.5F, directionalAnimationProgress <= 0 || directionalAnimationProgress >= 0.5F};
			} else {
				roundedAndBorderedSides = {m_CarouselAnimationDirection == CarouselAnimationDirection::Right && carouselIndex == (c_ItemsPerRow / 2) + 1 && directionalAnimationProgress < 0.5F, true};
			}
			SetupCarouselItemBox(carouselItemBox, roundedAndBorderedSides.first, roundedAndBorderedSides.second);
		}

		if (m_CarouselAnimationDirection == CarouselAnimationDirection::Right) {
			carouselIndex = -1;
			SetupCarouselItemBox(m_CarouselExitingItemBox, false, true);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InventoryMenuGUI::UpdateFullMode() {
		//TODO Make Full mode
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InventoryMenuGUI::UpdateTransferMode() {
		//TODO Make Transfer mode
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InventoryMenuGUI::Draw(BITMAP *targetBitmap, const Vector &targetPos) const {
		Vector drawPos = m_CenterPos - targetPos;

		switch (m_MenuMode) {
			case MenuMode::Carousel:
				if (!m_InventoryActor || m_InventoryActor->IsInventoryEmpty() && m_InventoryActorEquippedItems.empty()) {
					return;
				}
				DrawCarouselMode(targetBitmap, drawPos);
				break;
			case MenuMode::Full:
				break;
			case MenuMode::Transfer:
				break;
			default:
				RTEAbort("Unsupported InventoryMenuGUI MenuMode during Draw " + std::to_string(static_cast<int>(m_MenuMode)));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InventoryMenuGUI::DrawCarouselMode(BITMAP *targetBitmap, const Vector &drawPos) const {
		clear_to_color(m_CarouselBitmap.get(), g_MaskColor);
		clear_to_color(m_CarouselBGBitmap.get(), g_MaskColor);
		AllegroBitmap carouselAllegroBitmap(m_CarouselBitmap.get());

		for (const std::unique_ptr<CarouselItemBox> &carouselItemBox : m_CarouselItemBoxes) {
			if (carouselItemBox->Item || (carouselItemBox->IsForEquippedItems && !m_InventoryActorEquippedItems.empty())) {
				DrawCarouselItemBoxBackground(*carouselItemBox);
				DrawCarouselItemBoxForeground(*carouselItemBox, &carouselAllegroBitmap);
			} else if (m_CarouselDrawEmptyBoxes) {
				DrawCarouselItemBoxBackground(*carouselItemBox);
				m_SmallFont->DrawAligned(&carouselAllegroBitmap, carouselItemBox->IconCenterPosition.GetFloorIntX(), carouselItemBox->IconCenterPosition.GetFloorIntY() - (m_SmallFont->GetFontHeight() / 2), "Empty", GUIFont::Centre);
			}
		}
		if (m_CarouselAnimationDirection != CarouselAnimationDirection::None && (m_CarouselExitingItemBox->Item || m_CarouselDrawEmptyBoxes)) {
			if (m_CarouselExitingItemBox->Item) {
				DrawCarouselItemBoxBackground(*m_CarouselExitingItemBox);
				DrawCarouselItemBoxForeground(*m_CarouselExitingItemBox, &carouselAllegroBitmap);
			} else if (m_CarouselDrawEmptyBoxes) {
				DrawCarouselItemBoxBackground(*m_CarouselExitingItemBox);
				m_SmallFont->DrawAligned(&carouselAllegroBitmap, m_CarouselExitingItemBox->IconCenterPosition.GetFloorIntX(), m_CarouselExitingItemBox->IconCenterPosition.GetFloorIntY() - (m_SmallFont->GetFontHeight() / 2), "Empty", GUIFont::Centre);
			}
		}
		if (IsEnablingOrDisabling()) {
			int hiddenAreaHalfWidth = static_cast<int>((1.0F - m_EnableDisableProgress) * static_cast<float>(m_CarouselBitmap->w / 2));
			rectfill(m_CarouselBitmap.get(), 0, 0, hiddenAreaHalfWidth, m_CarouselBitmap->h, g_MaskColor);
			rectfill(m_CarouselBitmap.get(), m_CarouselBitmap->w - hiddenAreaHalfWidth, 0, m_CarouselBitmap->w, m_CarouselBitmap->h, g_MaskColor);
			rectfill(m_CarouselBGBitmap.get(), 0, 0, hiddenAreaHalfWidth, m_CarouselBGBitmap->h, g_MaskColor);
			rectfill(m_CarouselBGBitmap.get(), m_CarouselBGBitmap->w - hiddenAreaHalfWidth, 0, m_CarouselBGBitmap->w, m_CarouselBGBitmap->h, g_MaskColor);
		}

		if (m_CarouselBackgroundTransparent && !g_FrameMan.IsInMultiplayerMode()) {
			g_FrameMan.SetTransTable(MoreTrans);
			draw_trans_sprite(targetBitmap, m_CarouselBGBitmap.get(), drawPos.GetFloorIntX() - m_CarouselBGBitmap->w / 2, drawPos.GetFloorIntY() - m_CarouselBGBitmap->h / 2);
			draw_sprite(targetBitmap, m_CarouselBitmap.get(), drawPos.GetFloorIntX() - m_CarouselBitmap->w / 2, drawPos.GetFloorIntY() - m_CarouselBitmap->h / 2);
		} else {
			draw_sprite(m_CarouselBGBitmap.get(), m_CarouselBitmap.get(), 0, 0);
			draw_sprite(targetBitmap, m_CarouselBGBitmap.get(), drawPos.GetFloorIntX() - m_CarouselBGBitmap->w / 2, drawPos.GetFloorIntY() - m_CarouselBGBitmap->h / 2);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InventoryMenuGUI::DrawCarouselItemBoxBackground(const CarouselItemBox &itemBoxToDraw) const {
		auto DrawBox = [](BITMAP *targetBitmap, const Vector &boxTopLeftCorner, const Vector &boxBottomRightCorner, int color, bool roundedLeftSide, bool roundedRightSide) {
			if (roundedLeftSide) {
				circlefill(targetBitmap, boxTopLeftCorner.GetFloorIntX() + c_CarouselBoxCornerRadius, boxTopLeftCorner.GetFloorIntY() + c_CarouselBoxCornerRadius, c_CarouselBoxCornerRadius, color);
				circlefill(targetBitmap, boxTopLeftCorner.GetFloorIntX() + c_CarouselBoxCornerRadius, boxBottomRightCorner.GetFloorIntY() - c_CarouselBoxCornerRadius, c_CarouselBoxCornerRadius, color);
				rectfill(targetBitmap, boxTopLeftCorner.GetFloorIntX(), boxTopLeftCorner.GetFloorIntY() + c_CarouselBoxCornerRadius, boxTopLeftCorner.GetFloorIntX() + c_CarouselBoxCornerRadius, boxBottomRightCorner.GetFloorIntY() - c_CarouselBoxCornerRadius, color);
			}
			if (roundedRightSide) {
				circlefill(targetBitmap, boxBottomRightCorner.GetFloorIntX() - c_CarouselBoxCornerRadius, boxTopLeftCorner.GetFloorIntY() + c_CarouselBoxCornerRadius, c_CarouselBoxCornerRadius, color);
				circlefill(targetBitmap, boxBottomRightCorner.GetFloorIntX() - c_CarouselBoxCornerRadius, boxBottomRightCorner.GetFloorIntY() - c_CarouselBoxCornerRadius, c_CarouselBoxCornerRadius, color);
				rectfill(targetBitmap, boxBottomRightCorner.GetFloorIntX() - c_CarouselBoxCornerRadius, boxTopLeftCorner.GetFloorIntY() + c_CarouselBoxCornerRadius, boxBottomRightCorner.GetFloorIntX(), boxBottomRightCorner.GetFloorIntY() - c_CarouselBoxCornerRadius, color);
			}
			rectfill(targetBitmap, boxTopLeftCorner.GetFloorIntX() + (roundedLeftSide ? c_CarouselBoxCornerRadius : 0), boxTopLeftCorner.GetFloorIntY(), boxBottomRightCorner.GetFloorIntX() - (roundedRightSide ? c_CarouselBoxCornerRadius : 0), boxBottomRightCorner.GetFloorIntY(), color);
		};

		Vector spriteZeroIndexSizeOffset(1, 1);
		if (!m_CarouselBackgroundBoxBorderSize.IsZero()) {
			DrawBox(m_CarouselBGBitmap.get(), itemBoxToDraw.Pos, itemBoxToDraw.Pos + itemBoxToDraw.CurrentSize - spriteZeroIndexSizeOffset, m_CarouselBackgroundBoxBorderColor, itemBoxToDraw.RoundedAndBorderedSides.first, itemBoxToDraw.RoundedAndBorderedSides.second);
		}
		DrawBox(m_CarouselBGBitmap.get(), itemBoxToDraw.Pos + (itemBoxToDraw.RoundedAndBorderedSides.first ? m_CarouselBackgroundBoxBorderSize : Vector(0, m_CarouselBackgroundBoxBorderSize.GetY())), itemBoxToDraw.Pos + itemBoxToDraw.CurrentSize - spriteZeroIndexSizeOffset - (itemBoxToDraw.RoundedAndBorderedSides.second ? m_CarouselBackgroundBoxBorderSize : Vector(0, m_CarouselBackgroundBoxBorderSize.GetY())), m_CarouselBackgroundBoxColor, itemBoxToDraw.RoundedAndBorderedSides.first, itemBoxToDraw.RoundedAndBorderedSides.second);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InventoryMenuGUI::DrawCarouselItemBoxForeground(const CarouselItemBox &itemBoxToDraw, AllegroBitmap *carouselAllegroBitmap) const {
		std::vector<BITMAP *> itemIcons;
		float totalItemMass = 0;
		itemBoxToDraw.GetIconsAndMass(itemIcons, totalItemMass, &m_InventoryActorEquippedItems);

		Vector spriteZeroIndexSizeOffset(1, 1);
		Vector multiItemDrawOffset = Vector(c_MultipleItemInBoxOffset * static_cast<float>(itemIcons.size() - 1), -c_MultipleItemInBoxOffset * static_cast<float>(itemIcons.size() - 1));
		Vector iconMaxSize = itemBoxToDraw.CurrentSize - Vector(c_MinimumItemPadding * 2, c_MinimumItemPadding * 2) - spriteZeroIndexSizeOffset;
		if (itemBoxToDraw.RoundedAndBorderedSides.first) { iconMaxSize.SetX(iconMaxSize.GetX() - m_CarouselBackgroundBoxBorderSize.GetX()); }
		if (itemBoxToDraw.RoundedAndBorderedSides.second) { iconMaxSize.SetX(iconMaxSize.GetX() - m_CarouselBackgroundBoxBorderSize.GetX()); }
		std::for_each(itemIcons.crbegin(), itemIcons.crend(), [this, &itemBoxToDraw, &multiItemDrawOffset, &iconMaxSize](BITMAP *iconToDraw) {
			if (Vector sizeRatios(static_cast<float>(iconToDraw->w - 1 + (multiItemDrawOffset.GetFloorIntX() / 2)) / iconMaxSize.GetX(), static_cast<float>(iconToDraw->h - 1 + (multiItemDrawOffset.GetFloorIntY() / 2)) / iconMaxSize.GetY()); sizeRatios.GetX() > 1.0F || sizeRatios.GetY() > 1.0F) {
				stretch_sprite(m_CarouselBitmap.get(), iconToDraw,
					itemBoxToDraw.IconCenterPosition.GetFloorIntX() - static_cast<int>((itemBoxToDraw.RoundedAndBorderedSides.first ? std::floorf : std::ceilf)(static_cast<float>(iconToDraw->w) / (sizeRatios.GetLargest() * 2.0F))) + multiItemDrawOffset.GetFloorIntX() + (itemBoxToDraw.RoundedAndBorderedSides.first ? m_CarouselBackgroundBoxBorderSize.GetFloorIntX() / 2 : 0) - (itemBoxToDraw.RoundedAndBorderedSides.second ? m_CarouselBackgroundBoxBorderSize.GetFloorIntX() / 2 : 0),
					itemBoxToDraw.IconCenterPosition.GetFloorIntY() - static_cast<int>(static_cast<float>(iconToDraw->h) / (sizeRatios.GetLargest() * 2.0F)) + multiItemDrawOffset.GetFloorIntY(),
					static_cast<int>((itemBoxToDraw.RoundedAndBorderedSides.first ? std::ceilf : std::floorf)(static_cast<float>(iconToDraw->w) / sizeRatios.GetLargest())), static_cast<int>(static_cast<float>(iconToDraw->h) / sizeRatios.GetLargest()));
			} else {
				draw_sprite(m_CarouselBitmap.get(), iconToDraw, itemBoxToDraw.IconCenterPosition.GetFloorIntX() - (iconToDraw->w / 2) + multiItemDrawOffset.GetFloorIntX(), itemBoxToDraw.IconCenterPosition.GetFloorIntY() - (iconToDraw->h / 2) + multiItemDrawOffset.GetFloorIntY());
			}
			multiItemDrawOffset -= Vector(c_MultipleItemInBoxOffset, -c_MultipleItemInBoxOffset);
		});

		std::string massString = RoundFloatToPrecision(std::fminf(999, totalItemMass), 0) + (totalItemMass > 999 ? "+ " : " ") + "KG";
		m_SmallFont->DrawAligned(carouselAllegroBitmap, itemBoxToDraw.IconCenterPosition.GetFloorIntX(), itemBoxToDraw.IconCenterPosition.GetFloorIntY() - ((itemBoxToDraw.CurrentSize.GetFloorIntY() + m_SmallFont->GetFontHeight()) / 2) + 1, massString.c_str(), GUIFont::Centre);
	}
}