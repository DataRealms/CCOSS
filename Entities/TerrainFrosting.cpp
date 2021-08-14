#include "TerrainFrosting.h"
#include "SLTerrain.h"

namespace RTE {

	const std::string TerrainFrosting::c_ClassName = "TerrainFrosting";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TerrainFrosting::Clear() {
		m_FrostingMaterial.Reset();
		m_TargetMaterial.Reset();
		m_MinThickness = 5;
		m_MaxThickness = 5;
		m_InAirOnly = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TerrainFrosting::Create(const TerrainFrosting &reference) {
		m_FrostingMaterial = reference.m_FrostingMaterial;
		m_TargetMaterial = reference.m_TargetMaterial;
		m_MinThickness = reference.m_MinThickness;
		m_MaxThickness = reference.m_MaxThickness;
		m_InAirOnly = reference.m_InAirOnly;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TerrainFrosting::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "FrostingMaterial") {
			reader >> m_FrostingMaterial;
		} else if (propName == "TargetMaterial") {
			reader >> m_TargetMaterial;
		} else if (propName == "MinThickness") {
			reader >> m_MinThickness;
		} else if (propName == "MaxThickness") {
			reader >> m_MaxThickness;
		} else if (propName == "InAirOnly") {
			reader >> m_InAirOnly;
		} else {
			return Serializable::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TerrainFrosting::Save(Writer &writer) const {
		Serializable::Save(writer);

		writer.NewPropertyWithValue("FrostingMaterial", m_FrostingMaterial);
		writer.NewPropertyWithValue("TargetMaterial", m_TargetMaterial);
		writer.NewPropertyWithValue("MinThickness", m_MinThickness);
		writer.NewPropertyWithValue("MaxThickness", m_MaxThickness);
		writer.NewPropertyWithValue("InAirOnly", m_InAirOnly);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TerrainFrosting::ApplyFrosting(SLTerrain *terrain) const {
		// Try to get the texture of the frosting material. If there is none, the color index will be used instead.
		BITMAP *frostingTexture = m_FrostingMaterial.GetTexture();
		BITMAP *fgColorBitmap = terrain->GetFGColorBitmap();
		BITMAP *matBitmap = terrain->GetBitmap();

		bool targetMatFound = false;
		bool applyingFrosting = false;
		int appliedThickness = 0;

		// Reference. Do not remove.
		//acquire_bitmap(matBitmap);
		//acquire_bitmap(fgColorBitmap);
		//if (frostingTexture) { acquire_bitmap(frostingTexture); }

		for (int xPos = 0; xPos < matBitmap->w; ++xPos) {
			int thicknessGoal = m_MinThickness + RandomNum(0, m_MaxThickness - m_MinThickness);

			// Work upward from the bottom of each column.
			for (int yPos = matBitmap->h - 1; yPos >= 0; --yPos) {
				int matIndex = _getpixel(matBitmap, xPos, yPos);

				if (!targetMatFound && matIndex == m_TargetMaterial.GetIndex()) {
					targetMatFound = true;
					appliedThickness = 0;
				} else if (targetMatFound && matIndex != m_TargetMaterial.GetIndex() && appliedThickness <= thicknessGoal) {
					// Target material has ended! See if we should start putting on the frosting.
					targetMatFound = false;
					applyingFrosting = true;
				}
				if (applyingFrosting && (matIndex == MaterialColorKeys::g_MaterialAir || !m_InAirOnly) && appliedThickness <= thicknessGoal) {
					_putpixel(fgColorBitmap, xPos, yPos, frostingTexture ? _getpixel(frostingTexture, xPos % frostingTexture->w, yPos % frostingTexture->h) : m_FrostingMaterial.GetColor().GetIndex());
					_putpixel(matBitmap, xPos, yPos, m_FrostingMaterial.GetIndex());
					appliedThickness++;
				} else {
					applyingFrosting = false;
				}
			}
		}
		// Reference. Do not remove.
		//if (frostingTexture) { release_bitmap(frostingTexture); }
		//release_bitmap(fgColorBitmap);
		//release_bitmap(matBitmap);
	}
}