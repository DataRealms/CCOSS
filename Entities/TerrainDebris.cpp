#include "TerrainDebris.h"
#include "SLTerrain.h"

namespace RTE {

	ConcreteClassInfo(TerrainDebris, Entity, 0)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TerrainDebris::Clear() {
		m_DebrisFile.Reset();
		m_Bitmaps.clear();
		m_BitmapCount = 0;
		m_Material.Reset();
		m_TargetMaterial.Reset();
		m_DebrisPlacementMode = DebrisPlacementModes::NoPlacementRestrictions;
		m_OnlyBuried = false;
		m_MinDepth = 0;
		m_MaxDepth = 10;
		m_Density = 0.01F;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TerrainDebris::Create() {
		Entity::Create();

		m_DebrisFile.GetAsAnimation(m_Bitmaps, m_BitmapCount);
		RTEAssert(!m_Bitmaps.empty() && m_Bitmaps.at(0), "Failed to load debris bitmaps during TerrainDebris::Create!");

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TerrainDebris::Create(const TerrainDebris &reference) {
		Entity::Create(reference);

		m_DebrisFile = reference.m_DebrisFile;
		m_Bitmaps.clear();
		m_Bitmaps = reference.m_Bitmaps;
		m_BitmapCount = reference.m_BitmapCount;
		m_Material = reference.m_Material;
		m_TargetMaterial = reference.m_TargetMaterial;
		m_DebrisPlacementMode = reference.m_DebrisPlacementMode;
		m_OnlyBuried = reference.m_OnlyBuried;
		m_MinDepth = reference.m_MinDepth;
		m_MaxDepth = reference.m_MaxDepth;
		m_Density = reference.m_Density;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TerrainDebris::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "DebrisFile") {
			reader >> m_DebrisFile;
		} else if (propName == "DebrisPieceCount") {
			reader >> m_BitmapCount;
			m_Bitmaps.reserve(m_BitmapCount);
		} else if (propName == "DebrisMaterial") {
			reader >> m_Material;
		} else if (propName == "TargetMaterial") {
			reader >> m_TargetMaterial;
		} else if (propName == "DebrisPlacementMode") {
			m_DebrisPlacementMode = static_cast<DebrisPlacementModes>(std::stoi(reader.ReadPropValue()));
			if (m_DebrisPlacementMode < DebrisPlacementModes::NoPlacementRestrictions || m_DebrisPlacementMode > DebrisPlacementModes::OnOverhangAndCavityOverhang) { reader.ReportError("Invalid "); }
		} else if (propName == "OnlyBuried") {
			reader >> m_OnlyBuried;
		} else if (propName == "MinDepth") {
			reader >> m_MinDepth;
		} else if (propName == "MaxDepth") {
			reader >> m_MaxDepth;
		} else if (propName == "DensityPerMeter") {
			reader >> m_Density;
		} else {
			return Entity::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TerrainDebris::Save(Writer &writer) const {
		Entity::Save(writer);

		writer.NewProperty("DebrisFile");
		writer << m_DebrisFile;
		writer.NewProperty("DebrisPieceCount");
		writer << m_BitmapCount;
		writer.NewProperty("DebrisMaterial");
		writer << m_Material;
		writer.NewProperty("TargetMaterial");
		writer << m_TargetMaterial;
		writer.NewProperty("DebrisPlacementMode");
		writer << m_DebrisPlacementMode;
		writer.NewProperty("OnlyBuried");
		writer << m_OnlyBuried;
		writer.NewProperty("MinDepth");
		writer << m_MinDepth;
		writer.NewProperty("MaxDepth");
		writer << m_MaxDepth;
		writer.NewProperty("DensityPerMeter");
		writer << m_Density;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool TerrainDebris::GetPiecePlacementPosition(SLTerrain *terrain, Box &buriedCheckBox) const {
		BITMAP *matBitmap = terrain->GetMaterialBitmap();
		int posX = RandomNum(0, matBitmap->w);
		int depth = RandomNum(m_MinDepth, m_MaxDepth);
		int buriedDepthOffset = m_OnlyBuried ? static_cast<int>(buriedCheckBox.GetHeight() * 0.6F) : 0;
		int prevMaterialCheckPixel = -1;

		bool scanForOverhang = m_DebrisPlacementMode == DebrisPlacementModes::OnOverhangOnly || m_DebrisPlacementMode == DebrisPlacementModes::OnCavityOverhangOnly || m_DebrisPlacementMode == DebrisPlacementModes::OnOverhangAndCavityOverhang;

		// For overhangs scan from the bottom so it's easier to detect.
		for (int surfacePosY = 0, overhangPosY = matBitmap->h - 1; surfacePosY < matBitmap->h && overhangPosY > 0; surfacePosY++, overhangPosY--) {
			int posY = scanForOverhang ? overhangPosY : surfacePosY;
			int materialCheckPixel = _getpixel(matBitmap, posX, posY);
			if (materialCheckPixel != MaterialColorKeys::g_MaterialAir) {
				// TODO: Adding depth here will properly place debris only on target material, rather than any random pixel within depth range from the original target material pixel.
				int depthAdjustedPosY = posY;// + (depth * (scanForOverhang ? -1 : 1));
				if (scanForOverhang ? (depthAdjustedPosY > 0) : (depthAdjustedPosY < matBitmap->h)) {
					// TODO: Enable the depth check once multi TargetMaterial debris is supported.
					//materialCheckPixel = _getpixel(matBitmap, posX, depthAdjustedPosY);
					if (MaterialPixelIsValidTarget(materialCheckPixel, prevMaterialCheckPixel)) {
						surfacePosY += (depth + buriedDepthOffset);
						overhangPosY -= (depth + buriedDepthOffset);
						buriedCheckBox.SetCenter(Vector(static_cast<float>(posX), static_cast<float>(scanForOverhang ? overhangPosY : surfacePosY)));
						if (!m_OnlyBuried || terrain->IsBoxBuried(buriedCheckBox)) {
							return true;
						}
					}
				}
			}
			prevMaterialCheckPixel = materialCheckPixel;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool TerrainDebris::MaterialPixelIsValidTarget(int materialCheckPixel, int prevMaterialCheckPixel) const {
		bool checkResult = true;

		if (materialCheckPixel != m_TargetMaterial.GetIndex()) {
			checkResult = false;
		} else {
			switch (m_DebrisPlacementMode) {
				case DebrisPlacementModes::OnSurfaceOnly:
				case DebrisPlacementModes::OnOverhangOnly:
					if (prevMaterialCheckPixel != MaterialColorKeys::g_MaterialAir) { checkResult = false; }
					break;
				case DebrisPlacementModes::OnCavitySurfaceOnly:
				case DebrisPlacementModes::OnCavityOverhangOnly:
					if (prevMaterialCheckPixel != MaterialColorKeys::g_MaterialCavity) { checkResult = false; }
					break;
				case DebrisPlacementModes::OnSurfaceAndCavitySurface:
				case DebrisPlacementModes::OnOverhangAndCavityOverhang:
					if (prevMaterialCheckPixel != MaterialColorKeys::g_MaterialAir && prevMaterialCheckPixel != MaterialColorKeys::g_MaterialCavity) { checkResult = false; }
					break;
				default:
					// No placement restrictions, pixel just needs to be of target material.
					break;
			}
		}
		return checkResult;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TerrainDebris::ApplyDebris(SLTerrain *terrain) {
		RTEAssert(!m_Bitmaps.empty() && m_BitmapCount > 0, "No bitmaps loaded for terrain debris during TerrainDebris::ApplyDebris!");

		int possiblePieceToPlaceCount = static_cast<int>((static_cast<float>(terrain->GetMaterialBitmap()->w) * c_MPP) * m_Density);
		std::vector<std::pair<int, Vector>> piecesToPlace;
		piecesToPlace.reserve(possiblePieceToPlaceCount);

		// Reference. Do not remove.
		//acquire_bitmap(fgColorBitmap);
		//acquire_bitmap(matBitmap);

		for (int piece = 0; piece < possiblePieceToPlaceCount; ++piece) {
			int pieceBitmapIndex = RandomNum(0, m_BitmapCount - 1);
			RTEAssert(pieceBitmapIndex >= 0 && pieceBitmapIndex < m_BitmapCount, "Bitmap index was out of bounds during TerrainDebris::ApplyDebris!");
			Box buriedCheckBox(Vector(), static_cast<float>(m_Bitmaps.at(pieceBitmapIndex)->w), static_cast<float>(m_Bitmaps.at(pieceBitmapIndex)->h));
			if (GetPiecePlacementPosition(terrain, buriedCheckBox)) { piecesToPlace.emplace_back(pieceBitmapIndex, buriedCheckBox.GetCorner()); }
		}
		for (const auto &[bitmapIndex, position] : piecesToPlace) {
			draw_sprite(terrain->GetFGColorBitmap(), m_Bitmaps.at(bitmapIndex), position.GetFloorIntX(), position.GetFloorIntY());
			draw_character_ex(terrain->GetMaterialBitmap(), m_Bitmaps.at(bitmapIndex), position.GetFloorIntX(), position.GetFloorIntY(), m_Material.GetIndex(), -1);
		}

		// Reference. Do not remove.
		//release_bitmap(fgColorBitmap);
		//release_bitmap(matBitmap);
	}
}