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
		m_OnlyOnSurface = false;
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
		m_OnlyOnSurface = reference.m_OnlyOnSurface;
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
		} else if (propName == "OnlyOnSurface") {
			reader >> m_OnlyOnSurface;
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
		writer.NewProperty("OnlyOnSurface");
		writer << m_OnlyOnSurface;
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

	void TerrainDebris::ApplyDebris(SLTerrain *terrain) {
		RTEAssert(!m_Bitmaps.empty() && m_BitmapCount > 0, "No bitmaps loaded for terrain debris during TerrainDebris::ApplyDebris!");

		BITMAP *fgColorBitmap = terrain->GetFGColorBitmap();
		BITMAP *matBitmap = terrain->GetMaterialBitmap();

		std::vector<std::pair<int, Vector>> piecesToPlace;
		int pieceToPlaceCount = static_cast<int>((static_cast<float>(fgColorBitmap->w) * c_MPP) * m_Density);
		piecesToPlace.reserve(pieceToPlaceCount);
		Box buriedCheckBox;

		// Reference. Do not remove.
		//acquire_bitmap(fgColorBitmap);
		//acquire_bitmap(matBitmap);

		for (int piece = 0; piece < pieceToPlaceCount; ++piece) {
			int currentBitmap = RandomNum(0, m_BitmapCount - 1);
			RTEAssert(currentBitmap >= 0 && currentBitmap < m_BitmapCount, "Bitmap index was out of bounds during TerrainDebris::ApplyDebris!");
			buriedCheckBox.SetWidth(static_cast<float>(m_Bitmaps.at(currentBitmap)->w));
			buriedCheckBox.SetHeight(static_cast<float>(m_Bitmaps.at(currentBitmap)->h));

			int posX = RandomNum(0, fgColorBitmap->w);
			int depth = RandomNum(m_MinDepth, m_MaxDepth);
			bool placePiece = false;

			for (int posY = 0; posY < matBitmap->h;) {
				int materialCheckPixel = _getpixel(matBitmap, posX, posY);
				if (materialCheckPixel != MaterialColorKeys::g_MaterialAir) {
					// TODO: Adding depth here will properly place debris only on target material, rather than any random pixel within depth range from the original target material pixel.
					// TODO: Enable the depth check once multi TargetMaterial debris is supported.
					int depthAdjustedPosY = posY /*+ depth*/;
					if (depthAdjustedPosY < matBitmap->h) {
						//materialCheckPixel = _getpixel(matBitmap, posX, depthAdjustedPosY);
						if (materialCheckPixel == m_TargetMaterial.GetIndex()) {
							placePiece = true;
						} else if (m_OnlyOnSurface) {
							// TODO: Fix OnlyOnSurface, doesn't do anything right now and I don't even get the logic here.
							placePiece = false;
						}
					}
				}
				if (placePiece) {
					posY += depth + (m_OnlyBuried ? static_cast<int>(buriedCheckBox.GetHeight() * 0.6F) : 0);
					buriedCheckBox.SetCenter(Vector(static_cast<float>(posX), static_cast<float>(posY)));
					if (!terrain->IsAirPixel(posX, posY) && (!m_OnlyBuried || terrain->IsBoxBuried(buriedCheckBox))) {
						piecesToPlace.emplace_back(currentBitmap, buriedCheckBox.GetCorner());
						break;
					}
				}
				posY++;
			}
		}
		for (const auto &[pieceFrameNum, piecePos] : piecesToPlace) {
			draw_sprite(fgColorBitmap, m_Bitmaps.at(pieceFrameNum), piecePos.GetFloorIntX(), piecePos.GetFloorIntY());
			draw_character_ex(matBitmap, m_Bitmaps.at(pieceFrameNum), piecePos.GetFloorIntX(), piecePos.GetFloorIntY(), m_Material.GetIndex(), -1);
		}

		// Reference. Do not remove.
		//release_bitmap(fgColorBitmap);
		//release_bitmap(matBitmap);
	}
}