#include "TerrainFrosting.h"
#include "SLTerrain.h"
#include "SceneMan.h"

namespace RTE {

	const std::string TerrainFrosting::c_ClassName = "TerrainFrosting";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TerrainFrosting::Clear() {
		m_TargetMaterial.Reset();
		m_FrostingMaterial.Reset();
		m_MinThickness = 5;
		m_MaxThickness = 5;
		m_InAirOnly = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TerrainFrosting::Create(const TerrainFrosting &reference) {
		m_TargetMaterial = reference.m_TargetMaterial;
		m_FrostingMaterial = reference.m_FrostingMaterial;
		m_MinThickness = reference.m_MinThickness;
		m_MaxThickness = reference.m_MaxThickness;
		m_InAirOnly = reference.m_InAirOnly;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TerrainFrosting::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "TargetMaterial") {
			reader >> m_TargetMaterial;
		} else if (propName == "FrostingMaterial") {
			reader >> m_FrostingMaterial;
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

		writer.NewPropertyWithValue("TargetMaterial", m_TargetMaterial);
		writer.NewPropertyWithValue("FrostingMaterial", m_FrostingMaterial);
		writer.NewPropertyWithValue("MinThickness", m_MinThickness);
		writer.NewPropertyWithValue("MaxThickness", m_MaxThickness);
		writer.NewPropertyWithValue("InAirOnly", m_InAirOnly);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TerrainFrosting::ApplyFrosting(SLTerrain *terrain) const {
		BITMAP *matBitmap = terrain->GetBitmap();
		BITMAP *fgColorBitmap = terrain->GetFGColorBitmap();

		int targetId = m_TargetMaterial.GetIndex();
		bool targetFound = false;
		bool applyingFrosting = false;
		int thickness = 0;

		// Try to get the color texture of the frosting material. If fail, we'll use the color instead
		BITMAP *frostingTexture = m_FrostingMaterial.GetTexture();
		if (frostingTexture) { acquire_bitmap(frostingTexture); }

		acquire_bitmap(matBitmap);
		acquire_bitmap(fgColorBitmap);

		for (int xPos = 0; xPos < matBitmap->w; ++xPos) {
			int thicknessGoal = GetThicknessSample();

			// Work upward from the bottom of each column
			for (int yPos = matBitmap->h - 1; yPos >= 0; --yPos) {
				// Read which material the current pixel represents
				int matIndex = _getpixel(matBitmap, xPos, yPos);

				// We've encountered the target material! Prepare to apply frosting as soon as it ends!
				if (!targetFound && matIndex == targetId) {
					targetFound = true;
					thickness = 0;
				} else if (targetFound && matIndex != targetId && thickness <= thicknessGoal) {
					// Target material has ended! See if we should start putting on the frosting
					applyingFrosting = true;
					targetFound = false;
				}

				// If time to put down frosting pixels, then do so IF there is air, OR we're set to ignore what we're overwriting
				if (applyingFrosting && (matIndex == g_MaterialAir || !m_InAirOnly) && thickness <= thicknessGoal) {
					// Get the color either from the frosting material's texture or the solid color
					int pixelColor = frostingTexture ? _getpixel(frostingTexture, xPos % frostingTexture->w, yPos % frostingTexture->h) : m_FrostingMaterial.GetColor().GetIndex();

					// Put the frosting pixel color on the FG color layer
					_putpixel(fgColorBitmap, xPos, yPos, pixelColor);
					// Put the material ID pixel on the material layer
					_putpixel(matBitmap, xPos, yPos, m_FrostingMaterial.GetIndex());

					// Keep track of the applied thickness
					thickness++;
				} else {
					applyingFrosting = false;
				}
			}
		}
		if (frostingTexture) { release_bitmap(frostingTexture); }

		release_bitmap(matBitmap);
		release_bitmap(fgColorBitmap);
	}
}