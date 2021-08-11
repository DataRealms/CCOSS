#include "SLTerrain.h"
#include "TerrainFrosting.h"
#include "TerrainDebris.h"
#include "TerrainObject.h"
#include "PresetMan.h"
#include "DataModule.h"
#include "SceneObject.h"
#include "MOPixel.h"
#include "MOSprite.h"
#include "Atom.h"

namespace RTE {

	ConcreteClassInfo(SLTerrain, SceneLayer, 0)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::Clear() {
		m_FGColorLayer = nullptr;
		m_BGColorLayer = nullptr;
		m_StructuralBitmap = nullptr;
		m_BGTextureFile.Reset();
		m_TerrainFrostings.clear();
		m_TerrainDebris.clear();
		m_TerrainObjects.clear();
		m_UpdatedMateralAreas.clear();
		m_DrawMaterial = false;
		m_NeedToClearFrostings = false;
		m_NeedToClearDebris = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLTerrain::Create() {
		if (SceneLayer::Create() < 0) {
			return -1;
		}
		m_FGColorLayer = std::make_unique<SceneLayer>();
		m_BGColorLayer = std::make_unique<SceneLayer>();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLTerrain::Create(const SLTerrain &reference) {
		SceneLayer::Create(reference);

		// Leave these because they are loaded late by LoadData
		m_FGColorLayer.reset(dynamic_cast<SceneLayer *>(reference.m_FGColorLayer->Clone()));
		m_BGColorLayer.reset(dynamic_cast<SceneLayer *>(reference.m_BGColorLayer->Clone()));
		m_BGTextureFile = reference.m_BGTextureFile;

		m_TerrainFrostings.clear();
		for (TerrainFrosting *terrainFrosting : reference.m_TerrainFrostings) {
			m_TerrainFrostings.emplace_back(terrainFrosting);
		}
		m_TerrainDebris.clear();
		for (TerrainDebris *terrainDebris : reference.m_TerrainDebris) {
			m_TerrainDebris.emplace_back(terrainDebris);
		}
		m_TerrainObjects.clear();
		for (TerrainObject *terrainObject : reference.m_TerrainObjects) {
			m_TerrainObjects.emplace_back(terrainObject);
		}

		m_DrawMaterial = reference.m_DrawMaterial;

		m_NeedToClearFrostings = true;
		m_NeedToClearDebris = true;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLTerrain::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "BackgroundTexture") {
			reader >> m_BGTextureFile;
		} else if (propName == "FGColorLayer") {
			m_FGColorLayer = std::make_unique<SceneLayer>();
			reader >> m_FGColorLayer.get();
		} else if (propName == "BGColorLayer") {
			m_BGColorLayer = std::make_unique<SceneLayer>();
			reader >> m_BGColorLayer.get();
		} else if (propName == "AddTerrainFrosting") {
			// Clear frostings if we derived them from some other SLTerrain object and then read another set from explicit terrain definition
			if (m_NeedToClearFrostings) {
				m_NeedToClearFrostings = false;
				m_TerrainFrostings.clear();
			}
			std::unique_ptr<TerrainFrosting> terrainFrosting = std::make_unique<TerrainFrosting>();
			reader >> terrainFrosting.get();
			m_TerrainFrostings.emplace_back(terrainFrosting.release());
		} else if (propName == "AddTerrainDebris") {
			// Clear debris if we derived them from some other SLTerrain object and then read another set from explicit terrain definition
			if (m_NeedToClearDebris) {
				m_NeedToClearDebris = false;
				m_TerrainDebris.clear();
			}
			std::unique_ptr<TerrainDebris> terrainDebris = std::make_unique<TerrainDebris>();
			reader >> terrainDebris.get();
			m_TerrainDebris.emplace_back(terrainDebris.release());
		} else if (propName == "AddTerrainObject" || propName == "PlaceTerrainObject") {
			std::unique_ptr<TerrainObject> terrainObject = std::make_unique<TerrainObject>();
			reader >> terrainObject.get();
			m_TerrainObjects.emplace_back(terrainObject.release());
		} else {
			return SceneLayer::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLTerrain::Save(Writer &writer) const {
		SceneLayer::Save(writer);

		// Only write the background texture info if the background itself is not saved out as a file already
		if (m_BGColorLayer->IsFileData()) {
			writer.NewPropertyWithValue("BGColorLayer", m_BGColorLayer.get());
		} else {
			writer.NewPropertyWithValue("BackgroundTexture", m_BGTextureFile);
		}

		// Only if we haven't saved out the FG as a done and altered bitmap file should we save the procedural params here
		if (m_FGColorLayer->IsFileData()) {
			writer.NewPropertyWithValue("FGColorLayer", m_FGColorLayer.get());
		} else {
			// Layer data is not saved into a bitmap file yet, so just write out the procedural params to build the terrain
			for (const TerrainFrosting *terrainFrosting : m_TerrainFrostings) {
				writer.NewPropertyWithValue("AddTerrainFrosting", terrainFrosting);
			}
			for (const TerrainDebris *terrainDebris : m_TerrainDebris) {
				writer.NewPropertyWithValue("AddTerrainDebris", terrainDebris);
			}
			for (const TerrainObject *terrainObject : m_TerrainObjects) {
				// Write out only what is needed to place a copy of this in the Terrain
				writer.NewProperty("AddTerrainObject");
				writer.ObjectStart(terrainObject->GetClassName());
				writer.NewPropertyWithValue("CopyOf", terrainObject->GetModuleAndPresetName());
				writer.NewPropertyWithValue("Position", terrainObject->GetPos());
				writer.ObjectEnd();
			}
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::Destroy(bool notInherited) {
		//destroy_bitmap(m_StructuralBitmap);
		if (!notInherited) { SceneLayer::Destroy(); }
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::TexturizeTerrain() {
		BITMAP *fgColorBitmap = m_FGColorLayer->GetBitmap();
		BITMAP *bgColorBitmap = m_BGColorLayer->GetBitmap();
		BITMAP *bgLayerTexture = m_BGTextureFile.GetAsBitmap();

		const std::array<Material *, c_PaletteEntriesNumber> &materialPalette = g_SceneMan.GetMaterialPalette();
		const std::array<unsigned char, c_PaletteEntriesNumber> &materialMappings = g_PresetMan.GetDataModule(m_BitmapFile.GetDataModuleID())->GetAllMaterialMappings();

		std::array<BITMAP *, c_PaletteEntriesNumber> materialTextures;
		materialTextures.fill(nullptr);
		std::array<int, c_PaletteEntriesNumber> materialColors;
		materialColors.fill(0);

		//acquire_bitmap(m_MainBitmap);
		//acquire_bitmap(fgColorBitmap);
		//acquire_bitmap(bgColorBitmap);
		//acquire_bitmap(bgLayerTexture);

		// Go through each pixel on the main bitmap, which contains all the material pixels loaded from the bitmap
		// Place texture pixels on the FG layer corresponding to the materials on the main material bitmap
		for (int xPos = 0; xPos < m_MainBitmap->w; ++xPos) {
			for (int yPos = 0; yPos < m_MainBitmap->h; ++yPos) {
				// Read which material the current pixel represents
				int matIndex = _getpixel(m_MainBitmap, xPos, yPos);
				// Map any materials defined in this data module but initially collided with other material ID's and thus were displaced to other ID's
				if (materialMappings.at(matIndex) != 0) {
					// Assign the mapping and put it onto the material bitmap too
					matIndex = materialMappings.at(matIndex);
					_putpixel(m_MainBitmap, xPos, yPos, matIndex);
				}

				// Validate the material, or default to default material
				const Material *material = (matIndex >= 0 && matIndex < c_PaletteEntriesNumber && materialPalette.at(matIndex)) ? materialPalette.at(matIndex) : materialPalette.at(g_MaterialDefault);

				// If haven't read a pixel of this material before, then get its texture so we can quickly access it
				if (!materialTextures.at(matIndex) && material->GetTexture()) {
					materialTextures.at(matIndex) = material->GetTexture();
					//acquire_bitmap(materialTextures.at(matIndex));
				}

				int pixelColor = 0;
				// If actually no texture for the material, then use the material's solid color instead
				if (!materialTextures.at(matIndex)) {
					if (materialColors.at(matIndex) == 0) { materialColors.at(matIndex) = material->GetColor().GetIndex(); }
					pixelColor = materialColors.at(matIndex);
				} else {
					pixelColor = _getpixel(materialTextures.at(matIndex), xPos % materialTextures.at(matIndex)->w, yPos % materialTextures.at(matIndex)->h);
				}
				_putpixel(fgColorBitmap, xPos, yPos, pixelColor);

				// Draw background texture on the background where this is stuff on the foreground
				if (bgLayerTexture && pixelColor != ColorKeys::g_MaskColor) {
					pixelColor = _getpixel(bgLayerTexture, xPos % bgLayerTexture->w, yPos % bgLayerTexture->h);
					_putpixel(bgColorBitmap, xPos, yPos, pixelColor);
				} else {
					_putpixel(bgColorBitmap, xPos, yPos, ColorKeys::g_MaskColor);
				}
			}
		}
		//release_bitmap(m_MainBitmap);
		//release_bitmap(fgColorBitmap);
		//release_bitmap(bgColorBitmap);
		//release_bitmap(bgLayerTexture);
		/*
		for (BITMAP *textureBitmap : materialTextures) {
			release_bitmap(textureBitmap);
		}
		*/
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLTerrain::LoadData() {
		// Load the materials bitmap into the main bitmap
		if (SceneLayer::LoadData()) {
			return -1;
		}
		RTEAssert(m_FGColorLayer.get(), "Terrain's foreground layer not instantiated before trying to load its data!");
		RTEAssert(m_BGColorLayer.get(), "Terrain's background layer not instantiated before trying to load its data!");

		if (m_FGColorLayer->IsFileData() && m_BGColorLayer->IsFileData()) {
			RTEAssert(m_FGColorLayer->LoadData() == 0, "Could not load the Foreground Color SceneLayer data from file, when a path was specified for it!");
			RTEAssert(m_BGColorLayer->LoadData() == 0, "Could not load the Background Color SceneLayer data from file, when a path was specified for it!");
		} else {
			m_FGColorLayer->Destroy();
			m_FGColorLayer->Create(create_bitmap_ex(8, m_MainBitmap->w, m_MainBitmap->h), true, m_Offset, m_WrapX, m_WrapY, m_ScrollRatio);

			m_BGColorLayer->Destroy();
			m_BGColorLayer->Create(create_bitmap_ex(8, m_MainBitmap->w, m_MainBitmap->h), true, m_Offset, m_WrapX, m_WrapY, m_ScrollRatio);

			/*
			// Structural integrity calc buffer bitmap
			destroy_bitmap(m_StructuralBitmap);
			m_StructuralBitmap = create_bitmap_ex(8, m_MainBitmap->w, m_MainBitmap->h);
			RTEAssert(m_StructuralBitmap, "Failed to allocate BITMAP in Terrain::Create");
			clear_bitmap(m_StructuralBitmap);
			*/

			TexturizeTerrain();

			for (const TerrainFrosting *terrainFrosting : m_TerrainFrostings) {
				terrainFrosting->ApplyFrosting(this);
			}
			for (TerrainDebris *terrainDebris : m_TerrainDebris) {
				terrainDebris->ApplyDebris(this);
			}
			for (TerrainObject *terrainObject : m_TerrainObjects) {
				ApplyTerrainObject(terrainObject);
			}
			CleanAir();
		}
		InitScrollRatios();
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLTerrain::SaveData(const std::string &pathBase) {
		if (pathBase.empty()) {
			return -1;
		}
		RTEAssert(SceneLayer::SaveData(pathBase + " Mat.bmp") == 0, "Failed to write the material bitmap data saving an SLTerrain!");
		RTEAssert(m_FGColorLayer->SaveData(pathBase + " FG.bmp") == 0,"Failed to write the FG color bitmap data saving an SLTerrain!");
		RTEAssert(m_BGColorLayer->SaveData(pathBase + " BG.bmp") == 0,"Failed to write the BG color bitmap data saving an SLTerrain!");
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLTerrain::ClearData() {
		RTEAssert(SceneLayer::ClearData() == 0, "Failed to clear material bitmap data of an SLTerrain!");
		RTEAssert(m_FGColorLayer && m_FGColorLayer->ClearData() == 0, "Failed to clear the foreground color bitmap data of an SLTerrain!");
		RTEAssert(m_BGColorLayer && m_BGColorLayer->ClearData() == 0, "Failed to clear the background color bitmap data of an SLTerrain!");
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLTerrain::GetPixelFromLayer(LayerType layerType, int pixelX, int pixelY) const {
		BITMAP *terrainLayer = nullptr;
		switch (layerType) {
			case LayerType::ForegroundLayer:
				terrainLayer = m_FGColorLayer->GetBitmap();
				break;
			case LayerType::BackgroundLayer:
				terrainLayer = m_BGColorLayer->GetBitmap();
				break;
			case LayerType::MaterialLayer:
				terrainLayer = m_MainBitmap;
				break;
			default:
				RTEAbort("Invalid LayerType passed into SLTerrain::GetPixelFromLayer!");
				break;
		}
		int posX = pixelX;
		int posY = pixelY;
		WrapPosition(posX, posY);

		// If it's still below or to the sides out of bounds after wrapping what is supposed to be wrapped, shit is out of bounds.
		if (posX < 0 || posX >= m_MainBitmap->w || posY < 0 || posY >= m_MainBitmap->h) {
			return (layerType == LayerType::MaterialLayer) ? g_MaterialAir : ColorKeys::g_MaskColor;
		}
		return _getpixel(terrainLayer, posX, posY);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::SetPixelOnLayer(LayerType layerType, int pixelX, int pixelY, int color) const {
		BITMAP *terrainLayer = nullptr;
		switch (layerType) {
			case LayerType::ForegroundLayer:
				terrainLayer = m_FGColorLayer->GetBitmap();
				break;
			case LayerType::BackgroundLayer:
				terrainLayer = m_BGColorLayer->GetBitmap();
				break;
			case LayerType::MaterialLayer:
				terrainLayer = m_MainBitmap;
				break;
			default:
				RTEAbort("Invalid LayerType passed into SLTerrain::SetPixelOnLayer!");
				break;
		}
		int posX = pixelX;
		int posY = pixelY;
		WrapPosition(posX, posY);

		if (posX < 0 || posX >= m_MainBitmap->w || posY < 0 || posY >= m_MainBitmap->h) {
			return;
		}
		_putpixel(terrainLayer, posX, posY, color);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SLTerrain::IsAirPixel(const int pixelX, const int pixelY) const {
		int posX = pixelX;
		int posY = pixelY;
		WrapPosition(posX, posY);

		if (posX < 0 || posX >= m_MainBitmap->w || posY < 0 || posY >= m_MainBitmap->h) {
			return true;
		}
		int checkPixel = _getpixel(m_MainBitmap, posX, posY);
		return checkPixel == g_MaterialAir || checkPixel == g_MaterialCavity;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SLTerrain::IsBoxBuried(const Box &checkBox) const {
		bool buried = true;
		buried = buried && !IsAirPixel(checkBox.GetCorner().GetFloorIntX(), checkBox.GetCorner().GetFloorIntY());
		buried = buried && !IsAirPixel(static_cast<int>(checkBox.GetCorner().GetX() + checkBox.GetWidth()), checkBox.GetCorner().GetFloorIntY());
		buried = buried && !IsAirPixel(checkBox.GetCorner().GetFloorIntX(), static_cast<int>(checkBox.GetCorner().GetY() + checkBox.GetHeight()));
		buried = buried && !IsAirPixel(static_cast<int>(checkBox.GetCorner().GetX() + checkBox.GetWidth()), static_cast<int>(checkBox.GetCorner().GetY() + checkBox.GetHeight()));
		return buried;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// TODO: OPTIMIZE THIS, IT'S A TIME HOG. MAYBE JSUT STAMP THE OUTLINE AND SAMPLE SOME RANDOM PARTICLES?
	std::deque<MOPixel *> SLTerrain::EraseSilhouette(BITMAP *sprite, Vector pos, Vector pivot, Matrix rotation, float scale, bool makeMOPs, int skipMOP, int maxMOPs) {
		RTEAssert(sprite, "Null BITMAP passed to SLTerrain::EraseSilhouette");

		std::deque<MOPixel *> MOPDeque;

		// Find the maximum possible sized bitmap that the passed-in sprite will need
		int maxWidth = static_cast<int>(static_cast<float>(sprite->w + std::abs(pivot.GetFloorIntX() - (sprite->w / 2))) * scale);
		int maxHeight = static_cast<int>(static_cast<float>(sprite->h + std::abs(pivot.GetFloorIntY() - (sprite->h / 2))) * scale);
		int maxDiameter = static_cast<int>(std::sqrt(static_cast<float>(maxWidth * maxWidth + maxHeight * maxHeight)) * 2.0F);
		int skipCount = skipMOP;

		BITMAP *tempBitmap = g_SceneMan.GetTempBitmap(maxDiameter);
		clear_bitmap(tempBitmap);
		pivot_scaled_sprite(tempBitmap, sprite, tempBitmap->w / 2, tempBitmap->h / 2, pivot.GetFloorIntX(), pivot.GetFloorIntY(), ftofix(rotation.GetAllegroAngle()), ftofix(scale));

		// Do the test of intersection between color pixels of the test bitmap and non-air pixels of the terrain
		// Generate and collect MOPixels that represent the terrain overlap and clear the same pixels out of the terrain
		for (int testY = 0; testY < tempBitmap->h; ++testY) {
			for (int testX = 0; testX < tempBitmap->w; ++testX) {
				int terrX = pos.GetFloorIntX() - (tempBitmap->w / 2) + testX;
				int terrY = pos.GetFloorIntY() - (tempBitmap->h / 2) + testY;

				// Make sure we're checking within bounds
				if (terrX < 0) {
					if (m_WrapX) {
						while (terrX < 0) {
							terrX += m_MainBitmap->w;
						}
					} else {
						continue;
					}
				}
				if (terrY < 0) {
					if (m_WrapY) {
						while (terrY < 0) {
							terrY += m_MainBitmap->h;
						}
					} else {
						continue;
					}
				}
				if (terrX >= m_MainBitmap->w) {
					if (m_WrapX) {
						terrX %= m_MainBitmap->w;
					} else {
						continue;
					}
				}
				if (terrY >= m_MainBitmap->h) {
					if (m_WrapY) {
						terrY %= m_MainBitmap->h;
					} else {
						continue;
					}
				}

				int matPixel = getpixel(m_MainBitmap, terrX, terrY);
				int colorPixel = getpixel(m_FGColorLayer->GetBitmap(), terrX, terrY);

				if (getpixel(tempBitmap, testX, testY) != ColorKeys::g_MaskColor) {
					// Only add PixelMO if we're not due to skip any
					if (makeMOPs && matPixel != g_MaterialAir && colorPixel != ColorKeys::g_MaskColor && ++skipCount > skipMOP && MOPDeque.size() < maxMOPs) {
						skipCount = 0;
						const Material *sceneMat = g_SceneMan.GetMaterialFromID(matPixel);
						const Material *spawnMat = sceneMat->GetSpawnMaterial() ? g_SceneMan.GetMaterialFromID(sceneMat->GetSpawnMaterial()) : sceneMat;
						// Create the MOPixel based off the Terrain data.
						std::unique_ptr<MOPixel> terrainPixel = std::make_unique<MOPixel>(colorPixel, spawnMat->GetPixelDensity(), Vector(terrX, terrY), Vector(), new Atom(Vector(), spawnMat->GetIndex(), 0, colorPixel, 2), 0);
						terrainPixel->SetToHitMOs(false);
						MOPDeque.emplace_back(terrainPixel.release());
					}

					// Clear the terrain pixels
					if (matPixel != g_MaterialAir) { putpixel(m_MainBitmap, terrX, terrY, g_MaterialAir); }
					if (colorPixel != ColorKeys::g_MaskColor) {
						putpixel(m_FGColorLayer->GetBitmap(), terrX, terrY, ColorKeys::g_MaskColor);
						g_SceneMan.RegisterTerrainChange(terrX, terrY, 1, 1, ColorKeys::g_MaskColor, false);
					}
				}
			}
		}

		// Add a box to the updated areas list to show there's been change to the materials layer
		// TODO: improve fit/tightness of box here
		m_UpdatedMateralAreas.emplace_back(Box(pos - pivot, static_cast<float>(maxWidth), static_cast<float>(maxHeight)));

		return MOPDeque;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SLTerrain::ApplyObject(Entity *entity) {
		if (entity) {
			if (MovableObject *entityAsMovableObject = dynamic_cast<MovableObject *>(entity)) {
				ApplyMovableObject(entityAsMovableObject);
				return true;
			} else if (TerrainObject *entityAsTerrainObject = dynamic_cast<TerrainObject *>(entity)) {
				ApplyTerrainObject(entityAsTerrainObject);
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::ApplyMovableObject(MovableObject *movableObject) {
		if (!movableObject) {
			return;
		}

		// Determine whether a sprite or just a pixel-based MO. If sprite, try to integrate it into the terrain, with terrain on top.
		if (const MOSprite *moSprite = dynamic_cast<MOSprite *>(movableObject)) {
			BITMAP *tempBitmap = g_SceneMan.GetTempBitmap(static_cast<int>(moSprite->GetDiameter()));

			// The position of the upper left corner of the temporary bitmap in the scene
			Vector bitmapScroll = moSprite->GetPos().GetFloored() - Vector(static_cast<float>(tempBitmap->w / 2), static_cast<float>(tempBitmap->w / 2));

			Box notUsed;

			// COLOR
			// Clear and draw the source sprite onto the temp bitmap
			clear_to_color(tempBitmap, ColorKeys::g_MaskColor);
			// Draw the actor and then the scene foreground to temp bitmap
			moSprite->Draw(tempBitmap, bitmapScroll, DrawMode::g_DrawColor, true);
			m_FGColorLayer->Draw(tempBitmap, notUsed, bitmapScroll);
			// Finally draw temporary bitmap to the Scene
			masked_blit(tempBitmap, GetFGColorBitmap(), 0, 0, bitmapScroll.GetFloorIntX(), bitmapScroll.GetFloorIntY(), tempBitmap->w, tempBitmap->h);

			// Register terrain change
			g_SceneMan.RegisterTerrainChange(bitmapScroll.GetFloorIntX(), bitmapScroll.GetFloorIntY(), tempBitmap->w, tempBitmap->h, ColorKeys::g_MaskColor, false);

			// TODO: centralize seam drawing!
			// Draw over seams
			if (g_SceneMan.SceneWrapsX()) {
				if (bitmapScroll.GetFloorIntX() < 0) {
					masked_blit(tempBitmap, GetFGColorBitmap(), 0, 0, bitmapScroll.GetFloorIntX() + g_SceneMan.GetSceneWidth(), bitmapScroll.GetFloorIntY(), tempBitmap->w, tempBitmap->h);
				} else if (bitmapScroll.GetFloorIntX() + tempBitmap->w > g_SceneMan.GetSceneWidth()) {
					masked_blit(tempBitmap, GetFGColorBitmap(), 0, 0, bitmapScroll.GetFloorIntX() - g_SceneMan.GetSceneWidth(), bitmapScroll.GetFloorIntY(), tempBitmap->w, tempBitmap->h);
				}
			}
			if (g_SceneMan.SceneWrapsY()) {
				if (bitmapScroll.GetFloorIntY() < 0) {
					masked_blit(tempBitmap, GetFGColorBitmap(), 0, 0, bitmapScroll.GetFloorIntX(), bitmapScroll.GetFloorIntY() + g_SceneMan.GetSceneHeight(), tempBitmap->w, tempBitmap->h);
				} else if (bitmapScroll.GetFloorIntY() + tempBitmap->h > g_SceneMan.GetSceneHeight()) {
					masked_blit(tempBitmap, GetFGColorBitmap(), 0, 0, bitmapScroll.GetFloorIntX(), bitmapScroll.GetFloorIntY() - g_SceneMan.GetSceneHeight(), tempBitmap->w, tempBitmap->h);
				}
			}

			// Material
			clear_to_color(tempBitmap, g_MaterialAir);
			// Draw the actor and then the scene material layer to temp bitmap
			moSprite->Draw(tempBitmap, bitmapScroll, DrawMode::g_DrawMaterial, true);
			SceneLayer::Draw(tempBitmap, notUsed, bitmapScroll);
			// Finally draw temporary bitmap to the Scene
			masked_blit(tempBitmap, GetMaterialBitmap(), 0, 0, bitmapScroll.GetFloorIntX(), bitmapScroll.GetFloorIntY(), tempBitmap->w, tempBitmap->h);
			// Add a box to the updated areas list to show there's been change to the materials layer
			m_UpdatedMateralAreas.emplace_back(Box(bitmapScroll, static_cast<float>(tempBitmap->w), static_cast<float>(tempBitmap->h)));
			// TODO: centralize seam drawing!
			// Draw over seams
			if (g_SceneMan.SceneWrapsX()) {
				if (bitmapScroll.GetFloorIntX() < 0) {
					masked_blit(tempBitmap, GetMaterialBitmap(), 0, 0, bitmapScroll.GetFloorIntX() + g_SceneMan.GetSceneWidth(), bitmapScroll.GetFloorIntY(), tempBitmap->w, tempBitmap->h);
				} else if (bitmapScroll.GetFloorIntX() + tempBitmap->w > g_SceneMan.GetSceneWidth()) {
					masked_blit(tempBitmap, GetMaterialBitmap(), 0, 0, bitmapScroll.GetFloorIntX() - g_SceneMan.GetSceneWidth(), bitmapScroll.GetFloorIntY(), tempBitmap->w, tempBitmap->h);
				}
			}
			if (g_SceneMan.SceneWrapsY()) {
				if (bitmapScroll.GetFloorIntY() < 0) {
					masked_blit(tempBitmap, GetMaterialBitmap(), 0, 0, bitmapScroll.GetFloorIntX(), bitmapScroll.GetFloorIntY() + g_SceneMan.GetSceneHeight(), tempBitmap->w, tempBitmap->h);
				} else if (bitmapScroll.GetFloorIntY() + tempBitmap->h > g_SceneMan.GetSceneHeight()) {
					masked_blit(tempBitmap, GetMaterialBitmap(), 0, 0, bitmapScroll.GetFloorIntX(), bitmapScroll.GetFloorIntY() - g_SceneMan.GetSceneHeight(), tempBitmap->w, tempBitmap->h);
				}
			}
		} else {
			// Not a big sprite, so just draw the representations
			movableObject->Draw(GetFGColorBitmap(), Vector(), DrawMode::g_DrawColor, true);
			// Register terrain change
			g_SceneMan.RegisterTerrainChange(movableObject->GetPos().GetFloorIntX(), movableObject->GetPos().GetFloorIntY(), 1, 1, DrawMode::g_DrawColor, false);

			movableObject->Draw(GetMaterialBitmap(), Vector(), DrawMode::g_DrawMaterial, true);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::ApplyTerrainObject(TerrainObject *terrainObject) {
		if (!terrainObject) {
			return;
		}
		Vector pos = terrainObject->GetPos() + terrainObject->GetBitmapOffset();

		// Do duplicate drawing if the terrain object straddles a wrapping border
		if (pos.GetFloorIntX() < 0) {
			draw_sprite(m_MainBitmap, terrainObject->GetMaterialBitmap(), pos.GetFloorIntX() + m_MainBitmap->w, pos.GetFloorIntY());
			draw_sprite(m_FGColorLayer->GetBitmap(), terrainObject->GetFGColorBitmap(), pos.GetFloorIntX() + m_FGColorLayer->GetBitmap()->w, pos.GetFloorIntY());
			if (terrainObject->HasBGColor()) { draw_sprite(m_BGColorLayer->GetBitmap(), terrainObject->GetBGColorBitmap(), pos.GetFloorIntX() + m_BGColorLayer->GetBitmap()->w, pos.GetFloorIntY()); }
		} else if (pos.GetFloorIntX() >= m_MainBitmap->w - terrainObject->GetFGColorBitmap()->w) {
			draw_sprite(m_MainBitmap, terrainObject->GetMaterialBitmap(), pos.GetFloorIntX() - m_MainBitmap->w, pos.GetFloorIntY());
			draw_sprite(m_FGColorLayer->GetBitmap(), terrainObject->GetFGColorBitmap(), pos.GetFloorIntX() - m_FGColorLayer->GetBitmap()->w, pos.GetFloorIntY());
			if (terrainObject->HasBGColor()) { draw_sprite(m_BGColorLayer->GetBitmap(), terrainObject->GetBGColorBitmap(), pos.GetFloorIntX() - m_BGColorLayer->GetBitmap()->w, pos.GetFloorIntY()); }
		}

		// Regular drawing
		draw_sprite(m_MainBitmap, terrainObject->GetMaterialBitmap(), pos.GetFloorIntX(), pos.GetFloorIntY());
		draw_sprite(m_FGColorLayer->GetBitmap(), terrainObject->GetFGColorBitmap(), pos.GetFloorIntX(), pos.GetFloorIntY());
		if (terrainObject->HasBGColor()) {
			draw_sprite(m_BGColorLayer->GetBitmap(), terrainObject->GetBGColorBitmap(), pos.GetFloorIntX(), pos.GetFloorIntY());
			g_SceneMan.RegisterTerrainChange(pos.GetFloorIntX(), pos.GetFloorIntY(), terrainObject->GetBitmapWidth(), terrainObject->GetBitmapHeight(), ColorKeys::g_MaskColor, true);
		}

		// Register terrain change
		g_SceneMan.RegisterTerrainChange(pos.GetFloorIntX(), pos.GetFloorIntY(), terrainObject->GetBitmapWidth(), terrainObject->GetBitmapHeight(), ColorKeys::g_MaskColor, false);

		// Add a box to the updated areas list to show there's been change to the materials layer
		m_UpdatedMateralAreas.emplace_back(Box(pos, static_cast<float>(terrainObject->GetMaterialBitmap()->w), static_cast<float>(terrainObject->GetMaterialBitmap()->h)));

		// Apply all the child objects of the TO, and first reapply the team so all its children are guaranteed to be on the same team!
		terrainObject->SetTeam(terrainObject->GetTeam());

		for (const SceneObject::SOPlacer &childObject : terrainObject->GetChildObjects()) {
			// TODO: check if we're placing a brain, and have it replace the resident brain of the scene!
			// Copy and apply, transferring ownership of the new copy into the application
			g_SceneMan.AddSceneObject(childObject.GetPlacedCopy(terrainObject));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::RegisterTerrainChange(const TerrainObject *terrainObject) const {
		if (terrainObject) {
			Vector pos = terrainObject->GetPos() + terrainObject->GetBitmapOffset();
			if (terrainObject->HasBGColor()) { g_SceneMan.RegisterTerrainChange(pos.GetFloorIntX(), pos.GetFloorIntY(), terrainObject->GetBitmapWidth(), terrainObject->GetBitmapHeight(), ColorKeys::g_MaskColor, true); }
			g_SceneMan.RegisterTerrainChange(pos.GetFloorIntX(), pos.GetFloorIntY(), terrainObject->GetBitmapWidth(), terrainObject->GetBitmapHeight(), ColorKeys::g_MaskColor, false);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::ClearAllMaterial() {
		clear_to_color(m_MainBitmap, ColorKeys::g_MaskColor);
		clear_to_color(m_FGColorLayer->GetBitmap(), g_MaterialAir);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::CleanAir() {
		//acquire_bitmap(m_MainBitmap);
		//acquire_bitmap(m_FGColorLayer->GetBitmap());

		int width = m_MainBitmap->w;
		int height = m_MainBitmap->h;

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				int matPixel = _getpixel(m_MainBitmap, x, y);
				if (matPixel == g_MaterialCavity) {
					_putpixel(m_MainBitmap, x, y, g_MaterialAir);
					matPixel = g_MaterialAir;
				}
				if (matPixel == g_MaterialAir) { _putpixel(m_FGColorLayer->GetBitmap(), x, y, ColorKeys::g_MaskColor); }
			}
		}

		//release_bitmap(m_MainBitmap);
		//release_bitmap(m_FGColorLayer->GetBitmap());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::CleanAirBox(const Box &box, bool wrapsX, bool wrapsY) {
		//acquire_bitmap(m_MainBitmap);
		//acquire_bitmap(m_FGColorLayer->GetBitmap());

		int width = m_MainBitmap->w;
		int height = m_MainBitmap->h;

		for (int y = box.m_Corner.GetFloorIntY(); y < static_cast<int>(box.m_Corner.GetY() + box.m_Height); ++y) {
			for (int x = box.m_Corner.GetFloorIntX(); x < static_cast<int>(box.m_Corner.GetX() + box.m_Width); ++x) {
				int wrapX = x;
				int wrapY = y;

				if (wrapsX) {
					if (wrapX < 0) { wrapX += width; }
					if (wrapX >= width) { wrapX -= width; }
				}
				if (wrapsY) {
					if (wrapY < 0) { wrapY += height; }
					if (wrapY >= height) { wrapY -= height; }
				}
				if (wrapX >= 0 && wrapX < width && wrapY >= 0 && wrapY < height) {
					int matPixel = _getpixel(m_MainBitmap, wrapX, wrapY);
					if (matPixel == g_MaterialCavity) {
						_putpixel(m_MainBitmap, wrapX, wrapY, g_MaterialAir);
						matPixel = g_MaterialAir;
					}
					if (matPixel == g_MaterialAir) { _putpixel(m_FGColorLayer->GetBitmap(), wrapX, wrapY, ColorKeys::g_MaskColor); }
				}
			}
		}
		//release_bitmap(m_MainBitmap);
		//release_bitmap(m_FGColorLayer->GetBitmap());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::Update() {
		SceneLayer::Update();

		m_FGColorLayer->SetOffset(m_Offset);
		m_BGColorLayer->SetOffset(m_Offset);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::Draw(BITMAP *targetBitmap, Box &targetBox, const Vector &scrollOverride) {
		if (m_DrawMaterial) {
			SceneLayer::Draw(targetBitmap, targetBox, scrollOverride);
		} else {
			m_FGColorLayer->Draw(targetBitmap, targetBox, scrollOverride);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::DrawBackground(BITMAP *targetBitmap, Box &targetBox, const Vector &scrollOverride) const {
		m_BGColorLayer->Draw(targetBitmap, targetBox, scrollOverride);
	}
}