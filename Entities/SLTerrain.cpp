#include "SLTerrain.h"
#include "TerrainFrosting.h"
#include "TerrainDebris.h"
#include "TerrainObject.h"
#include "SceneObject.h"
#include "MOSprite.h"
#include "MOPixel.h"
#include "Atom.h"
#include "DataModule.h"
#include "PresetMan.h"

namespace RTE {

	ConcreteClassInfo(SLTerrain, SceneLayer, 0);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::Clear() {
		m_LayerToDraw = LayerType::ForegroundLayer;
		m_FGColorLayer = nullptr;
		m_BGColorLayer = nullptr;
		m_DefaultBGTextureFile.Reset();
		m_TerrainFrostings.clear();
		m_TerrainDebris.clear();
		m_TerrainObjects.clear();
		m_UpdatedMateralAreas.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLTerrain::Create() {
		SceneLayer::Create();

		m_FGColorLayer = std::make_unique<SceneLayer>();
		m_BGColorLayer = std::make_unique<SceneLayer>();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLTerrain::Create(const SLTerrain &reference) {
		SceneLayer::Create(reference);

		// Don't copy the layers because they are loaded later by LoadData.
		m_FGColorLayer.reset(dynamic_cast<SceneLayer *>(reference.m_FGColorLayer->Clone()));
		m_BGColorLayer.reset(dynamic_cast<SceneLayer *>(reference.m_BGColorLayer->Clone()));

		m_DefaultBGTextureFile = reference.m_DefaultBGTextureFile;

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

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLTerrain::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "BackgroundTexture") {
			reader >> m_DefaultBGTextureFile;
		} else if (propName == "FGColorLayer") {
			m_FGColorLayer = std::make_unique<SceneLayer>();
			reader >> m_FGColorLayer.get();
		} else if (propName == "BGColorLayer") {
			m_BGColorLayer = std::make_unique<SceneLayer>();
			reader >> m_BGColorLayer.get();
		} else if (propName == "AddTerrainFrosting") {
			std::unique_ptr<TerrainFrosting> terrainFrosting = std::make_unique<TerrainFrosting>();
			reader >> terrainFrosting.get();
			m_TerrainFrostings.emplace_back(terrainFrosting.release());
		} else if (propName == "AddTerrainDebris") {
			std::unique_ptr<TerrainDebris> terrainDebris = std::make_unique<TerrainDebris>();
			reader >> terrainDebris.get();
			m_TerrainDebris.emplace_back(terrainDebris.release());
		} else if (propName == "PlaceTerrainObject") {
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

		// Only write the background texture info if the background itself is not saved out as a file already.
		if (m_BGColorLayer->IsFileData()) {
			writer.NewPropertyWithValue("BGColorLayer", m_BGColorLayer.get());
		} else {
			writer.NewPropertyWithValue("BackgroundTexture", m_DefaultBGTextureFile);
		}

		// Only if we haven't saved out the FG as a done and altered bitmap file should we save the procedural params here.
		if (m_FGColorLayer->IsFileData()) {
			writer.NewPropertyWithValue("FGColorLayer", m_FGColorLayer.get());
		} else {
			// Layer data is not saved into a bitmap file yet, so just write out the procedural params to build the terrain.
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
		if (!notInherited) { SceneLayer::Destroy(); }
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::TexturizeTerrain() {
		BITMAP *defaultBGLayerTexture = m_DefaultBGTextureFile.GetAsBitmap();

		const std::array<Material *, c_PaletteEntriesNumber> &materialPalette = g_SceneMan.GetMaterialPalette();
		const std::array<unsigned char, c_PaletteEntriesNumber> &materialMappings = g_PresetMan.GetDataModule(m_BitmapFile.GetDataModuleID())->GetAllMaterialMappings();

		std::array<BITMAP *, c_PaletteEntriesNumber> materialFGTextures;
		std::array<BITMAP *, c_PaletteEntriesNumber> materialBGTextures;
		materialFGTextures.fill(nullptr);
		materialBGTextures.fill(nullptr);
		std::array<int, c_PaletteEntriesNumber> materialColors;
		materialColors.fill(0);

		// Reference. Do not remove.
		//acquire_bitmap(m_MainBitmap);
		//acquire_bitmap(m_FGColorLayer->GetBitmap());
		//acquire_bitmap(m_BGColorLayer->GetBitmap());
		//acquire_bitmap(defaultBGLayerTexture);

		// Go through each pixel on the main bitmap, which contains all the material pixels loaded from the bitmap.
		// Place texture pixels on the FG layer corresponding to the materials on the main material bitmap.
		for (int xPos = 0; xPos < m_MainBitmap->w; ++xPos) {
			for (int yPos = 0; yPos < m_MainBitmap->h; ++yPos) {
				// Read which material the current pixel represents
				int matIndex = _getpixel(m_MainBitmap, xPos, yPos);
				// Map any materials defined in this data module but initially collided with other material ID's and thus were displaced to other ID's.
				if (materialMappings.at(matIndex) != 0) {
					// Assign the mapping and put it onto the material bitmap too.
					matIndex = materialMappings.at(matIndex);
					_putpixel(m_MainBitmap, xPos, yPos, matIndex);
				}

				// Validate the material, or default to default material.
				const Material *material = (matIndex >= 0 && matIndex < c_PaletteEntriesNumber && materialPalette.at(matIndex)) ? materialPalette.at(matIndex) : materialPalette.at(MaterialColorKeys::g_MaterialOutOfBounds);

				// If haven't read a pixel of this material before, then get its texture so we can quickly access it.
				if (!materialFGTextures.at(matIndex) && material->GetFGTexture()) {
					materialFGTextures.at(matIndex) = material->GetFGTexture();
					//acquire_bitmap(materialFGTextures.at(matIndex));
				}
				if (!materialBGTextures.at(matIndex) && material->GetBGTexture()) {
					materialBGTextures.at(matIndex) = material->GetBGTexture();
					//acquire_bitmap(materialBGTextures.at(matIndex));
				}

				int fgPixelColor = 0;
				// If actually no texture for the material, then use the material's solid color instead.
				if (!materialFGTextures.at(matIndex)) {
					if (materialColors.at(matIndex) == 0) { materialColors.at(matIndex) = material->GetColor().GetIndex(); }
					fgPixelColor = materialColors.at(matIndex);
				} else {
					fgPixelColor = _getpixel(materialFGTextures.at(matIndex), xPos % materialFGTextures.at(matIndex)->w, yPos % materialFGTextures.at(matIndex)->h);
				}
				_putpixel(m_FGColorLayer->GetBitmap(), xPos, yPos, fgPixelColor);

				int bgPixelColor = 0;
				if (matIndex == 0) {
					bgPixelColor = ColorKeys::g_MaskColor;
				} else {
					if (!materialBGTextures.at(matIndex)) {
						bgPixelColor = _getpixel(defaultBGLayerTexture, xPos % defaultBGLayerTexture->w, yPos % defaultBGLayerTexture->h);
					} else {
						bgPixelColor = _getpixel(materialBGTextures.at(matIndex), xPos % materialBGTextures.at(matIndex)->w, yPos % materialBGTextures.at(matIndex)->h);
					}
				}
				_putpixel(m_BGColorLayer->GetBitmap(), xPos, yPos, bgPixelColor);
			}
		}

		// Reference. Do not remove.
		//release_bitmap(m_MainBitmap);
		//release_bitmap(m_FGColorLayer->GetBitmap());
		//release_bitmap(m_BGColorLayer->GetBitmap());
		//release_bitmap(defaultBGLayerTexture);
		/*
		for (BITMAP *fgTextureBitmap : materialFGTextures) {
			release_bitmap(fgTextureBitmap);
		}
		for (BITMAP *bgTextureBitmap : materialBGTextures) {
			release_bitmap(bgTextureBitmap);
		}
		*/
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLTerrain::LoadData() {
		// Load the materials bitmap into the main bitmap.
		SceneLayer::LoadData();

		RTEAssert(m_FGColorLayer.get(), "Terrain's foreground layer not instantiated before trying to load its data!");
		RTEAssert(m_BGColorLayer.get(), "Terrain's background layer not instantiated before trying to load its data!");

		if (m_FGColorLayer->IsFileData() && m_BGColorLayer->IsFileData()) {
			m_FGColorLayer->LoadData();
			m_BGColorLayer->LoadData();
		} else {
			m_FGColorLayer->Destroy();
			m_FGColorLayer->Create(create_bitmap_ex(8, m_MainBitmap->w, m_MainBitmap->h), true, m_Offset, m_WrapX, m_WrapY, m_ScrollInfo);

			m_BGColorLayer->Destroy();
			m_BGColorLayer->Create(create_bitmap_ex(8, m_MainBitmap->w, m_MainBitmap->h), true, m_Offset, m_WrapX, m_WrapY, m_ScrollInfo);

			TexturizeTerrain();

			for (const TerrainFrosting *terrainFrosting : m_TerrainFrostings) {
				terrainFrosting->ApplyFrosting(this);
			}
			for (TerrainDebris *terrainDebris : m_TerrainDebris) {
				terrainDebris->ApplyDebris(this);
			}
			for (TerrainObject *terrainObject : m_TerrainObjects) {
				terrainObject->ApplyTerrainObject(this);
			}
			CleanAir();
		}
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
			return (layerType == LayerType::MaterialLayer) ? MaterialColorKeys::g_MaterialAir : ColorKeys::g_MaskColor;
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
		return checkPixel == MaterialColorKeys::g_MaterialAir || checkPixel == MaterialColorKeys::g_MaterialCavity;
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

	bool SLTerrain::ApplyObject(Entity *entity) {
		if (entity) {
			if (MovableObject *entityAsMovableObject = dynamic_cast<MovableObject *>(entity)) {
				return entityAsMovableObject->ApplyMovableObject(this);
			} else if (TerrainObject *entityAsTerrainObject = dynamic_cast<TerrainObject *>(entity)) {
				return entityAsTerrainObject->ApplyTerrainObject(this);
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// TODO: OPTIMIZE THIS, IT'S A TIME HOG. MAYBE JSUT STAMP THE OUTLINE AND SAMPLE SOME RANDOM PARTICLES?
	std::deque<MOPixel *> SLTerrain::EraseSilhouette(BITMAP *sprite, const Vector &pos, const Vector &pivot, const Matrix &rotation, float scale, bool makeMOPs, int skipMOP, int maxMOPs) {
		RTEAssert(sprite, "Null BITMAP passed to SLTerrain::EraseSilhouette");

		std::deque<MOPixel *> MOPDeque;

		// Find the maximum possible sized bitmap that the passed-in sprite will need
		int maxWidth = static_cast<int>(static_cast<float>(sprite->w + std::abs(pivot.GetFloorIntX() - (sprite->w / 2))) * scale);
		int maxHeight = static_cast<int>(static_cast<float>(sprite->h + std::abs(pivot.GetFloorIntY() - (sprite->h / 2))) * scale);
		int maxDiameter = static_cast<int>(std::sqrt(static_cast<float>(maxWidth * maxWidth + maxHeight * maxHeight)) * 2.0F);
		int skipCount = skipMOP;

		BITMAP *tempBitmap = g_SceneMan.GetIntermediateBitmapForSettlingIntoTerrain(maxDiameter);
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
					// Only add PixelMO if we're not due to skip any.
					if (makeMOPs && matPixel != MaterialColorKeys::g_MaterialAir && colorPixel != ColorKeys::g_MaskColor && ++skipCount > skipMOP && MOPDeque.size() < maxMOPs) {
						skipCount = 0;
						const Material *sceneMat = g_SceneMan.GetMaterialFromID(matPixel);
						const Material *spawnMat = sceneMat->GetSpawnMaterial() ? g_SceneMan.GetMaterialFromID(sceneMat->GetSpawnMaterial()) : sceneMat;
						// Create the MOPixel based off the Terrain data.
						std::unique_ptr<MOPixel> terrainPixel = std::make_unique<MOPixel>(colorPixel, spawnMat->GetPixelDensity(), Vector(terrX, terrY), Vector(), new Atom(Vector(), spawnMat->GetIndex(), 0, colorPixel, 2), 0);
						terrainPixel->SetToHitMOs(false);
						MOPDeque.emplace_back(terrainPixel.release());
					}

					// Clear the terrain pixels.
					if (matPixel != MaterialColorKeys::g_MaterialAir) { putpixel(m_MainBitmap, terrX, terrY, MaterialColorKeys::g_MaterialAir); }
					if (colorPixel != ColorKeys::g_MaskColor) {
						putpixel(m_FGColorLayer->GetBitmap(), terrX, terrY, ColorKeys::g_MaskColor);
						g_SceneMan.RegisterTerrainChange(terrX, terrY, 1, 1, ColorKeys::g_MaskColor, false);
					}
				}
			}
		}

		// Add a box to the updated areas list to show there's been change to the materials layer.
		// TODO: improve fit/tightness of box here.
		m_UpdatedMateralAreas.emplace_back(Box(pos - pivot, static_cast<float>(maxWidth), static_cast<float>(maxHeight)));

		return MOPDeque;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::ClearAllMaterial() {
		clear_to_color(m_MainBitmap, ColorKeys::g_MaskColor);
		clear_to_color(m_FGColorLayer->GetBitmap(), MaterialColorKeys::g_MaterialAir);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::CleanAir() {
		// Reference. Do not remove.
		//acquire_bitmap(m_MainBitmap);
		//acquire_bitmap(m_FGColorLayer->GetBitmap());

		int width = m_MainBitmap->w;
		int height = m_MainBitmap->h;

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				int matPixel = _getpixel(m_MainBitmap, x, y);
				if (matPixel == MaterialColorKeys::g_MaterialCavity) {
					_putpixel(m_MainBitmap, x, y, MaterialColorKeys::g_MaterialAir);
					matPixel = MaterialColorKeys::g_MaterialAir;
				}
				if (matPixel == MaterialColorKeys::g_MaterialAir) { _putpixel(m_FGColorLayer->GetBitmap(), x, y, ColorKeys::g_MaskColor); }
			}
		}
		// Reference. Do not remove.
		//release_bitmap(m_MainBitmap);
		//release_bitmap(m_FGColorLayer->GetBitmap());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::CleanAirBox(const Box &box, bool wrapsX, bool wrapsY) {
		// Reference. Do not remove.
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
					if (matPixel == MaterialColorKeys::g_MaterialCavity) {
						_putpixel(m_MainBitmap, wrapX, wrapY, MaterialColorKeys::g_MaterialAir);
						matPixel = MaterialColorKeys::g_MaterialAir;
					}
					if (matPixel == MaterialColorKeys::g_MaterialAir) { _putpixel(m_FGColorLayer->GetBitmap(), wrapX, wrapY, ColorKeys::g_MaskColor); }
				}
			}
		}
		// Reference. Do not remove.
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

	void SLTerrain::Draw(BITMAP *targetBitmap, Box &targetBox, bool offsetNeedsScrollRatioAdjustment) {
		switch (m_LayerToDraw) {
			case LayerType::MaterialLayer:
				SceneLayer::Draw(targetBitmap, targetBox);
				break;
			case LayerType::ForegroundLayer:
				m_FGColorLayer->Draw(targetBitmap, targetBox);
				break;
			case LayerType::BackgroundLayer:
				m_BGColorLayer->Draw(targetBitmap, targetBox);
				break;
			default:
				RTEAbort("Invalid LayerType was set to draw in SLTerrain::Draw!");
				break;
		}
	}
}