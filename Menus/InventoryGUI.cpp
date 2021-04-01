#include "InventoryGUI.h"

#include "AHuman.h"

#include "GUI/GUIFont.h"
#include "GUI/AllegroBitmap.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InventoryGUI::Clear() {
	m_Actor = 0;
	m_BGBitmap = 0;
	m_MassDisplayFont = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int InventoryGUI::Create(Actor *inventoryActor) {
	m_Actor = inventoryActor;
	if (!m_BGBitmap) {
		m_BGBitmap = create_bitmap_ex(8, MAX(s_InventoryIconMaxWidth, s_InventoryIconMaxHeight) + (s_InventoryEquippedIconHighlightWidth * 2), MAX(s_InventoryIconMaxWidth, s_InventoryIconMaxHeight) + (s_InventoryEquippedIconHighlightWidth * 2));
	}
	m_MassDisplayFont = g_FrameMan.GetSmallFont();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InventoryGUI::Update() {
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InventoryGUI::Draw(BITMAP *targetBitmap, const Vector &targetPos) const {
	if (m_PieEnabled == ENABLED && m_Actor && g_MovableMan.IsActor(m_Actor) && dynamic_cast<AHuman *>(m_Actor)) {
		const std::deque<MovableObject *> *inventory = m_Actor->GetInventory();
		const AHuman *actorAsHuman = dynamic_cast<AHuman *>(m_Actor);
		MovableObject *equippedItem = actorAsHuman != nullptr ? actorAsHuman->GetEquippedItem() : nullptr;
		if (inventory->empty() && equippedItem == nullptr) {
			return;
		}

		std::vector<MovableObject *> organizedInventory;
		organizedInventory.reserve(inventory->size() + 1);
		std::copy(inventory->begin() + inventory->size() / 2, inventory->end(), std::back_inserter(organizedInventory));
		organizedInventory.push_back(equippedItem);
		std::copy(inventory->begin(), inventory->begin() + inventory->size() / 2, std::back_inserter(organizedInventory));

		std::pair<float, float> inventoryObjectRowRemainders;
		std::vector<std::array<MovableObject *, s_InventoryIconsPerRow>> inventoryObjectRows;
		Vector inventoryIconSize = equippedItem == nullptr ? Vector() : Vector(equippedItem->GetGraphicalIcon()->w, equippedItem->GetGraphicalIcon()->h);
		TransformOrganizedInventoryIntoRowsAndCalculateRemaindersAndIconSizes(organizedInventory, inventoryObjectRowRemainders, inventoryObjectRows, inventoryIconSize);

		DrawInventoryItemIconsAndMassIndicators(targetBitmap, targetPos, organizedInventory.size(), equippedItem, inventoryObjectRowRemainders, inventoryObjectRows, inventoryIconSize);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InventoryGUI::TransformOrganizedInventoryIntoRowsAndCalculateRemaindersAndIconSizes(const std::vector<MovableObject *> &organizedInventory, std::pair<float, float> &inventoryObjectRowRemainders, std::vector<std::array<MovableObject *, s_InventoryIconsPerRow>> &inventoryObjectRows, Vector &inventoryIconSize) const {
	const float remainderFloat = static_cast<float>((organizedInventory.size() - s_InventoryIconsPerRow) % (s_InventoryIconsPerRow * 2)) / 2;
	inventoryObjectRowRemainders = {
		(organizedInventory.size() > s_InventoryIconsPerRow) ? std::ceil(remainderFloat) : 0,
		(organizedInventory.size() > s_InventoryIconsPerRow) ? std::floor(remainderFloat) : 0
	};

	int currentInsertIndex = 0;
	std::array<MovableObject *, s_InventoryIconsPerRow> currentRow;
	std::fill(currentRow.begin(), currentRow.end(), nullptr);

	for (MovableObject *inventoryObject : organizedInventory) {
		currentInsertIndex %= s_InventoryIconsPerRow;

		if (inventoryObjectRows.size() == 0 && currentInsertIndex == 0 && inventoryObjectRowRemainders.first > 0) {
			currentInsertIndex = s_InventoryIconsPerRow - inventoryObjectRowRemainders.first;
		}
		currentRow[currentInsertIndex] = inventoryObject;
		currentInsertIndex++;

		inventoryIconSize.SetXY(
			MIN(s_InventoryIconMaxWidth, MAX(inventoryIconSize.GetX(), inventoryObject->GetGraphicalIcon()->w)),
			MIN(s_InventoryIconMaxHeight, MAX(inventoryIconSize.GetY(), inventoryObject->GetGraphicalIcon()->h))
		);

		if (currentInsertIndex == s_InventoryIconsPerRow || inventoryObject == organizedInventory.at(organizedInventory.size() - 1)) {
			inventoryObjectRows.push_back(currentRow);
			std::fill(currentRow.begin(), currentRow.end(), nullptr);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InventoryGUI::DrawInventoryItemIconsAndMassIndicators(BITMAP *targetBitmap, const Vector &drawPos, float totalOrganizedInventorySize, const MovableObject *equippedItem, std::pair<float, float> inventoryObjectRowRemainders, const std::vector<std::array<MovableObject *, s_InventoryIconsPerRow>> &inventoryObjectRows, const Vector &inventoryIconSize) const {
	Vector inventoryDrawPos(
		drawPos.GetX() - ((s_InventoryIconsPerRow / 2) * (inventoryIconSize.GetX() + s_InventoryIconSpacer) + (inventoryIconSize.GetX() / 2)),
		drawPos.GetY() - (m_EnabledRadius + m_Thickness) - (inventoryObjectRows.size() * (inventoryIconSize.GetY() + m_MassDisplayFont->GetFontHeight() + 10))
	);
	float currentRowUnfilledSpots;

	for (const std::array<MovableObject *, s_InventoryIconsPerRow> &inventoryObjectRow : inventoryObjectRows) {
		currentRowUnfilledSpots = 0;
		if (inventoryObjectRows.size() == 1) {
			currentRowUnfilledSpots = s_InventoryIconsPerRow - totalOrganizedInventorySize;
		} else if (inventoryObjectRowRemainders.first > 0 && inventoryObjectRow == (*inventoryObjectRows.begin())) {
			currentRowUnfilledSpots = s_InventoryIconsPerRow - inventoryObjectRowRemainders.first;
		} else if (inventoryObjectRowRemainders.second > 0 && inventoryObjectRow == (*(inventoryObjectRows.end() - 1))) {
			currentRowUnfilledSpots = s_InventoryIconsPerRow - inventoryObjectRowRemainders.second;
		}
		DrawInventoryItemIconRow(targetBitmap, Vector(inventoryDrawPos.GetX() + (currentRowUnfilledSpots / 2) * (inventoryIconSize.GetX() + s_InventoryIconSpacer), inventoryDrawPos.GetY()), equippedItem, inventoryObjectRow, inventoryIconSize);

		inventoryDrawPos.SetY(inventoryDrawPos.GetY() + inventoryIconSize.GetY() + m_MassDisplayFont->GetFontHeight() + 10);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InventoryGUI::DrawInventoryItemIconRow(BITMAP *targetBitmap, Vector &currentInventoryIconDrawPos, const MovableObject *equippedItem, const std::array<MovableObject *, s_InventoryIconsPerRow> &inventoryObjectRow, const Vector &inventoryIconSize) const {
	BITMAP *currentInventoryIcon;
	Vector currentInventoryIconSize;
	float currentInventoryIconWidthHalfDifference;

	AllegroBitmap targetAllegroBitmap(targetBitmap);
	std::string massString;

	int i = -3;
	for (MovableObject *currentInventoryObject : inventoryObjectRow) {
		i++;
		if (currentInventoryObject == nullptr) {
			continue;
		}
		currentInventoryIcon = currentInventoryObject->GetGraphicalIcon();
		if (currentInventoryIcon == nullptr) {
			currentInventoryIconDrawPos.SetX(currentInventoryIconDrawPos.GetX() + inventoryIconSize.GetX() + s_InventoryIconSpacer);
			continue;
		}

		currentInventoryIconSize.SetXY(currentInventoryIcon->w, currentInventoryIcon->h);
		if (currentInventoryIconSize.GetX() > inventoryIconSize.GetX() || currentInventoryIconSize.GetY() > inventoryIconSize.GetY()) {
			currentInventoryIconSize /= (currentInventoryIconSize / inventoryIconSize).GetLargest();
		}

		currentInventoryIconWidthHalfDifference = (inventoryIconSize.GetX() - currentInventoryIconSize.GetX()) / 2;
		currentInventoryIconDrawPos.SetX(currentInventoryIconDrawPos.GetX() + currentInventoryIconWidthHalfDifference);
		/*if (false && currentInventoryObject == equippedItem) {
			clear_to_color(m_EquippedItemBGBitmap, g_MaskColor);
			stretch_sprite(m_EquippedItemBGBitmap, currentInventoryIcon, 0, 0, currentInventoryIconSize.GetX() + (s_InventoryEquippedIconHighlightWidth * 2), currentInventoryIconSize.GetY() + (s_InventoryEquippedIconHighlightWidth * 2));
			draw_character_ex(targetBitmap, m_EquippedItemBGBitmap, currentInventoryIconDrawPos.GetX() - s_InventoryEquippedIconHighlightWidth, currentInventoryIconDrawPos.GetY() - s_InventoryEquippedIconHighlightWidth, g_BlackColor, -1);
			clear_to_color(m_EquippedItemBGBitmap, g_MaskColor);
			stretch_sprite(m_EquippedItemBGBitmap, currentInventoryIcon, 0, 0, currentInventoryIconSize.GetX() + (s_InventoryEquippedIconHighlightWidth * 2) - 2, currentInventoryIconSize.GetY() + (s_InventoryEquippedIconHighlightWidth * 2 - 2));
			draw_character_ex(targetBitmap, m_EquippedItemBGBitmap, currentInventoryIconDrawPos.GetX() - s_InventoryEquippedIconHighlightWidth, currentInventoryIconDrawPos.GetY() - s_InventoryEquippedIconHighlightWidth, g_WhiteColor, -1);
		}*/
		if (currentInventoryIcon->w > inventoryIconSize.GetX() || currentInventoryIcon->h > inventoryIconSize.GetY()) {
			stretch_sprite(targetBitmap, currentInventoryIcon, currentInventoryIconDrawPos.GetX(), currentInventoryIconDrawPos.GetY(), currentInventoryIconSize.GetX(), currentInventoryIconSize.GetY());
		} else {
			//draw_sprite(targetBitmap, currentInventoryIcon, currentInventoryIconDrawPos.GetX(), currentInventoryIconDrawPos.GetY());
			float xMult = i == 0 ? 1 : (i % 2 == 0 ? 0.8 : 0.9);
			float yMult = xMult;
			float yOffset = i == 0 ? 0 : 0;// : (i % 2 == 0 ? -3 : -1.5);
			stretch_sprite(targetBitmap, currentInventoryIcon, currentInventoryIconDrawPos.GetX(), currentInventoryIconDrawPos.GetY() + yOffset, currentInventoryIconSize.GetX() * xMult, currentInventoryIconSize.GetY() * yMult);
		}
		massString = std::to_string(std::min(999, static_cast<int>(std::round(currentInventoryObject->GetMass())))) + (currentInventoryObject->GetMass() > 999 ? "+" : " ") + "KG";
		//g_FrameMan.GetSmallFont()->DrawAligned(&targetAllegroBitmap, currentInventoryIconDrawPos.GetX() + (currentInventoryIconSize.GetX() / 2), currentInventoryIconDrawPos.GetY() - (inventoryIconSize.GetY() / 2) - (m_InventoryMassDisplayFont->GetFontHeight() / 2) - s_InventoryIconSpacer, massString.c_str(), GUIFont::Centre);
		currentInventoryIconDrawPos.SetXY(currentInventoryIconDrawPos.GetX() - currentInventoryIconWidthHalfDifference + inventoryIconSize.GetX() + s_InventoryIconSpacer, currentInventoryIconDrawPos.GetY());
	}
}