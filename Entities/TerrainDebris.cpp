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
		if (Entity::Create() < 0) {
			return -1;
		}
		m_DebrisFile.GetAsAnimation(m_Bitmaps, m_BitmapCount);
		RTEAssert(!m_Bitmaps.empty() && m_Bitmaps.at(0), "Failed to load debris bitmaps!");

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TerrainDebris::Create(const TerrainDebris &reference) {
		Entity::Create(reference);

		m_DebrisFile = reference.m_DebrisFile;
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
		RTEAssert(!m_Bitmaps.empty() && m_BitmapCount > 0, "No bitmaps loaded for terrain debris!");

		BITMAP *terrBitmap = terrain->GetFGColorBitmap();
		BITMAP *matBitmap = terrain->GetMaterialBitmap();

		// How many pieces of debris we're spreading out.
		unsigned int terrainWidth = terrBitmap->w;
		unsigned int pieceCount = static_cast<int>((static_cast<float>(terrainWidth) * c_MPP) * m_Density);

		// First is index in the bitmap array, Second is blit location
		std::list<std::pair<int, Vector>> piecesToPlace;
		Vector location;
		Box pieceBox;

		unsigned char checkPixel;

		acquire_bitmap(terrBitmap);
		acquire_bitmap(matBitmap);

		for (unsigned int piece = 0; piece < pieceCount; ++piece) {
			bool place = false;
			unsigned int currentBitmap = RandomNum<int>(0, m_BitmapCount - 1);
			RTEAssert(currentBitmap >= 0 && currentBitmap < m_BitmapCount, "Bitmap index is out of bounds!");

			pieceBox.SetWidth(static_cast<float>(m_Bitmaps.at(currentBitmap)->w));
			pieceBox.SetHeight(static_cast<float>(m_Bitmaps.at(currentBitmap)->h));

			int x = RandomNum<int>(0, terrainWidth);
			int y = 0;
			int depth = RandomNum(m_MinDepth, m_MaxDepth);

			while (y < terrBitmap->h) {
				// Find the air-terrain boundary
				for (; y < terrBitmap->h; ++y) {
					checkPixel = _getpixel(matBitmap, x, y);
					// Check for terrain hit
					if (checkPixel != g_MaterialAir) {
						if (checkPixel == m_TargetMaterial.GetIndex()) {
							place = true;
							break;
						// If we didn't hit target material, but are specified to, then don't place
						} else if (m_OnlyOnSurface) {
							place = false;
							break;
						}
					}
				}
				if (!place) {
					break;
				}
				// The target locations are on the center of the objects; if supposed to be buried, move down so it is
				y += depth + static_cast<int>(m_OnlyBuried ? pieceBox.GetHeight() * 0.6F : 0);
				pieceBox.SetCenter(Vector(static_cast<float>(x), static_cast<float>(y)));

				// Make sure we're not trying to place something into a cave or other air pocket
				if (!g_SceneMan.GetTerrain()->IsAirPixel(x, y) && (!m_OnlyBuried || g_SceneMan.GetTerrain()->IsBoxBuried(pieceBox))) {
					// Do delayed drawing so that we don't end up placing things on top of each other
					piecesToPlace.emplace_back(currentBitmap, pieceBox.GetCorner());
					break;
				}
			}
		}

		for (const auto &[pieceFrameNum, piecePos] : piecesToPlace) {
			// Draw the color sprite onto the terrain color layer.
			draw_sprite(terrBitmap, m_Bitmaps.at(pieceFrameNum), piecePos.GetFloorIntX(), piecePos.GetFloorIntY());
			// Draw the material representation onto the terrain's material layer
			draw_character_ex(matBitmap, m_Bitmaps.at(pieceFrameNum), piecePos.GetFloorIntX(), piecePos.GetFloorIntY(), m_Material.GetIndex(), -1);
		}

		release_bitmap(terrBitmap);
		release_bitmap(matBitmap);
		terrBitmap = nullptr;
		matBitmap = nullptr;
	}
}