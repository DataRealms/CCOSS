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
		m_Width = 0;
		m_Height = 0;
		m_LayerToDraw = LayerType::ForegroundLayer;
		m_FGColorLayer = nullptr;
		m_BGColorLayer = nullptr;
		m_DefaultBGTextureFile.Reset();
		m_TerrainFrostings.clear();
		m_TerrainDebris.clear();
		m_TerrainObjects.clear();
		m_UpdatedMaterialAreas.clear();
		m_OrbitDirection = Directions::Up;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLTerrain::Create() {
		SceneLayer::Create();

		m_Width = m_BitmapFile.GetImageWidth();
		m_Height = m_BitmapFile.GetImageHeight();

		if (!m_FGColorLayer.get()) { m_FGColorLayer = std::make_unique<SceneLayer>(); }
		if (!m_BGColorLayer.get()) { m_BGColorLayer = std::make_unique<SceneLayer>(); }

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLTerrain::Create(const SLTerrain &reference) {
		SceneLayer::Create(reference);

		m_Width = reference.m_Width;
		m_Height = reference.m_Height;

		// Copy the layers but not the layer BITMAPs because they will be loaded later by LoadData.
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

		m_OrbitDirection = reference.m_OrbitDirection;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLTerrain::ReadProperty(const std::string_view &propName, Reader &reader) {
		StartPropertyList(return SceneLayer::ReadProperty(propName, reader));
		
		MatchProperty("BackgroundTexture", { reader >> m_DefaultBGTextureFile; });
		MatchProperty("FGColorLayer", {
			m_FGColorLayer = std::make_unique<SceneLayer>();
			reader >> m_FGColorLayer.get();
		});
		MatchProperty("BGColorLayer", {
			m_BGColorLayer = std::make_unique<SceneLayer>();
			reader >> m_BGColorLayer.get();
		});
		MatchProperty("AddTerrainFrosting", {
			std::unique_ptr<TerrainFrosting> terrainFrosting = std::make_unique<TerrainFrosting>();
			reader >> terrainFrosting.get();
			m_TerrainFrostings.emplace_back(terrainFrosting.release());
		});
		MatchProperty("AddTerrainDebris", {
			std::unique_ptr<TerrainDebris> terrainDebris = std::make_unique<TerrainDebris>();
			reader >> terrainDebris.get();
			m_TerrainDebris.emplace_back(terrainDebris.release());
		});
		MatchProperty("PlaceTerrainObject", {
			std::unique_ptr<TerrainObject> terrainObject = std::make_unique<TerrainObject>();
			reader >> terrainObject.get();
			m_TerrainObjects.emplace_back(terrainObject.release());
		});
		MatchProperty("OrbitDirection", {
			std::string orbitDirection;
			reader >> orbitDirection;
			if (orbitDirection == "Up") {
				m_OrbitDirection = Directions::Up;
			} else if (orbitDirection == "Down") {
				m_OrbitDirection = Directions::Down;
			} else if (orbitDirection == "Left") {
				m_OrbitDirection = Directions::Left;
			} else if (orbitDirection == "Right") {
				m_OrbitDirection = Directions::Right;
			} else {
				reader.ReportError("Unknown OrbitDirection '" + orbitDirection + "'!");
			}
		});
		
		EndPropertyList;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLTerrain::Save(Writer &writer) const {
		SceneLayer::Save(writer);

		// Only write the background texture info if the background itself is not saved out as a file already, since saved, pre-rendered bitmaps don't need texturing.
		if (m_BGColorLayer->IsLoadedFromDisk()) {
			writer.NewPropertyWithValue("BGColorLayer", m_BGColorLayer.get());
		} else {
			writer.NewPropertyWithValue("BackgroundTexture", m_DefaultBGTextureFile);
		}

		// Only write the procedural parameters if the foreground itself is not saved out as a file already, since saved, pre-rendered bitmaps don't need procedural generation.
		if (m_FGColorLayer->IsLoadedFromDisk()) {
			writer.NewPropertyWithValue("FGColorLayer", m_FGColorLayer.get());
		} else {
			for (const TerrainFrosting *terrainFrosting : m_TerrainFrostings) {
				writer.NewPropertyWithValue("AddTerrainFrosting", terrainFrosting);
			}
			for (const TerrainDebris *terrainDebris : m_TerrainDebris) {
				writer.NewPropertyWithValue("AddTerrainDebris", terrainDebris);
			}
			for (const TerrainObject *terrainObject : m_TerrainObjects) {
				// Write out only what is needed to place a copy of this in the Terrain
				writer.NewProperty("PlaceTerrainObject");
				writer.ObjectStart(terrainObject->GetClassName());
				writer.NewPropertyWithValue("CopyOf", terrainObject->GetModuleAndPresetName());
				writer.NewPropertyWithValue("Position", terrainObject->GetPos());
				writer.ObjectEnd();
			}
		}

		writer.NewProperty("OrbitDirection");
		switch (m_OrbitDirection) {
		default:
		case Directions::Up:
			writer << "Up";
			break;
		case Directions::Down:
			writer << "Down";
			break;
		case Directions::Left:
			writer << "Left";
			break;
		case Directions::Right:
			writer << "Right";
			break;
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// TODO: Break this down and refactor.
	void SLTerrain::TexturizeTerrain() {
		BITMAP *defaultBGLayerTexture = m_DefaultBGTextureFile.GetAsBitmap();

		BITMAP* fgLayerTexture = m_FGColorLayer->GetBitmap();
		BITMAP* bgLayerTexture = m_BGColorLayer->GetBitmap();

		const std::array<Material *, c_PaletteEntriesNumber> &materialPalette = g_SceneMan.GetMaterialPalette();
		const std::array<unsigned char, c_PaletteEntriesNumber> &materialMappings = g_PresetMan.GetDataModule(m_BitmapFile.GetDataModuleID())->GetAllMaterialMappings();

		std::array<BITMAP *, c_PaletteEntriesNumber> materialFGTextures;
		materialFGTextures.fill(nullptr);
		std::array<BITMAP *, c_PaletteEntriesNumber> materialBGTextures;
		materialBGTextures.fill(nullptr);
		std::array<int, c_PaletteEntriesNumber> materialColors;
		materialColors.fill(0);

		// We want to multithread this, however parallel fors only work on container types
		// This is sorta ugly, but a necessary evil for now :)
		std::vector<int> rows(m_MainBitmap->h); // we loop through h first, because we want each thread to have sequential memory that they're touching
		std::iota(std::begin(rows), std::end(rows), 0);

		// Go through each pixel on the main bitmap, which contains all the material pixels loaded from the bitmap.
		// Place texture pixels on the FG layer corresponding to the materials on the main material bitmap.
		std::for_each(std::execution::par_unseq, std::begin(rows), std::end(rows),
			[&](int yPos) {
				for (int xPos = 0; xPos < m_MainBitmap->w; ++xPos) {
					int matIndex = _getpixel(m_MainBitmap, xPos, yPos);

					// Map any materials defined in this data module but initially collided with other material ID's and thus were displaced to other ID's.
					if (materialMappings[matIndex] != 0) {
						// Assign the mapping and put it onto the material bitmap too.
						matIndex = materialMappings[matIndex];
						_putpixel(m_MainBitmap, xPos, yPos, matIndex);
					}

					RTEAssert(matIndex >= 0 && matIndex < c_PaletteEntriesNumber, "Invalid material index!");

					// Validate the material, or fallback to default material.
					const Material* material = materialPalette[matIndex] ? materialPalette[matIndex] : materialPalette[MaterialColorKeys::g_MaterialOutOfBounds];

					BITMAP* fgTexture = materialFGTextures[matIndex];
					BITMAP* bgTexture = materialBGTextures[matIndex];

					// If haven't read a pixel of this material before, then get its texture so we can quickly access it.
					if (!fgTexture && material->GetFGTexture()) {
						fgTexture = materialFGTextures[matIndex] = material->GetFGTexture();
					}

					if (!bgTexture && material->GetBGTexture()) {
						bgTexture = materialBGTextures[matIndex] = material->GetBGTexture();
					}

					int fgPixelColor = 0;

					// If actually no texture for the material, then use the material's solid color instead.
					if (!fgTexture) {
						if (materialColors[matIndex] == 0) { 
							materialColors[matIndex] = material->GetColor().GetIndex(); 
						}
						fgPixelColor = materialColors[matIndex];
					} else {
						fgPixelColor = _getpixel(fgTexture, xPos % fgTexture->w, yPos % fgTexture->h);
					}
					_putpixel(fgLayerTexture, xPos, yPos, fgPixelColor);

					int bgPixelColor = 0;
					if (matIndex == 0) {
						bgPixelColor = ColorKeys::g_MaskColor;
					} else {
						if (!bgTexture) {
							bgPixelColor = _getpixel(defaultBGLayerTexture, xPos % defaultBGLayerTexture->w, yPos % defaultBGLayerTexture->h);
						} else {
							bgPixelColor = _getpixel(bgTexture, xPos % bgTexture->w, yPos% bgTexture->h);
						}
					}

					_putpixel(bgLayerTexture, xPos, yPos, bgPixelColor);
				}
			});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLTerrain::LoadData() {
		SceneLayer::LoadData();

		RTEAssert(m_FGColorLayer.get(), "Terrain's foreground layer not instantiated before trying to load its data!");
		RTEAssert(m_BGColorLayer.get(), "Terrain's background layer not instantiated before trying to load its data!");

		if (m_FGColorLayer->IsLoadedFromDisk() && m_BGColorLayer->IsLoadedFromDisk()) {
			m_FGColorLayer->LoadData();
			m_BGColorLayer->LoadData();
		} else {
			m_FGColorLayer->Destroy();
			m_FGColorLayer->Create(create_bitmap_ex(8, m_MainBitmap->w, m_MainBitmap->h), true, m_Offset, m_WrapX, m_WrapY, m_ScrollInfo);

			m_BGColorLayer->Destroy();
			m_BGColorLayer->Create(create_bitmap_ex(8, m_MainBitmap->w, m_MainBitmap->h), true, m_Offset, m_WrapX, m_WrapY, m_ScrollInfo);

			TexturizeTerrain();

			for (const TerrainFrosting *terrainFrosting : m_TerrainFrostings) {
				terrainFrosting->FrostTerrain(this);
			}
			for (TerrainDebris *terrainDebris : m_TerrainDebris) {
				terrainDebris->ScatterOnTerrain(this);
			}
			for (TerrainObject *terrainObject : m_TerrainObjects) {
				terrainObject->PlaceOnTerrain(this);
			}
			CleanAir();
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLTerrain::SaveData(const std::string &pathBase, bool doAsyncSaves) {
		if (pathBase.empty()) {
			return -1;
		}
		SceneLayer::SaveData(pathBase + " Mat.png", doAsyncSaves);
		m_FGColorLayer->SaveData(pathBase + " FG.png", doAsyncSaves);
		m_BGColorLayer->SaveData(pathBase + " BG.png", doAsyncSaves);
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

	bool SLTerrain::IsAirPixel(const int pixelX, const int pixelY) const {
		int checkPixel = GetPixel(pixelX, pixelY);
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

	void SLTerrain::CleanAir() {
		std::vector<int> rows(m_MainBitmap->h); // we loop through h first, because we want each thread to have sequential memory that they're touching
		std::iota(std::begin(rows), std::end(rows), 0);

		std::for_each(std::execution::par_unseq, std::begin(rows), std::end(rows),
			[&](int yPos) {
				for (int xPos = 0; xPos < m_MainBitmap->w; ++xPos) {
					int matPixel = _getpixel(m_MainBitmap, xPos, yPos);
					if (matPixel == MaterialColorKeys::g_MaterialCavity) {
						_putpixel(m_MainBitmap, xPos, yPos, MaterialColorKeys::g_MaterialAir);
						matPixel = MaterialColorKeys::g_MaterialAir;
					}
					if (matPixel == MaterialColorKeys::g_MaterialAir) { _putpixel(m_FGColorLayer->GetBitmap(), xPos, yPos, ColorKeys::g_MaskColor); }
				}
			});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLTerrain::CleanAirBox(const Box &box, bool wrapsX, bool wrapsY) {
		int width = m_MainBitmap->w;
		int height = m_MainBitmap->h;

		for (int y = box.m_Corner.GetFloorIntY(); y < static_cast<int>(box.m_Corner.GetY() + box.m_Height); ++y) {
			for (int x = box.m_Corner.GetFloorIntX(); x < static_cast<int>(box.m_Corner.GetX() + box.m_Width); ++x) {
				int wrappedX = x;
				int wrappedY = y;

				if (wrapsX) {
					if (wrappedX < 0) { wrappedX += width; }
					if (wrappedX >= width) { wrappedX -= width; }
				}
				if (wrapsY) {
					if (wrappedY < 0) { wrappedY += height; }
					if (wrappedY >= height) { wrappedY -= height; }
				}
				if (wrappedX >= 0 && wrappedX < width && wrappedY >= 0 && wrappedY < height) {
					int matPixel = _getpixel(m_MainBitmap, wrappedX, wrappedY);
					if (matPixel == MaterialColorKeys::g_MaterialCavity) {
						_putpixel(m_MainBitmap, wrappedX, wrappedY, MaterialColorKeys::g_MaterialAir);
						matPixel = MaterialColorKeys::g_MaterialAir;
					}
					if (matPixel == MaterialColorKeys::g_MaterialAir) { _putpixel(m_FGColorLayer->GetBitmap(), wrappedX, wrappedY, ColorKeys::g_MaskColor); }
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// TODO: OPTIMIZE THIS, IT'S A TIME HOG. MAYBE JSUT STAMP THE OUTLINE AND SAMPLE SOME RANDOM PARTICLES?
	std::deque<MOPixel *> SLTerrain::EraseSilhouette(BITMAP *sprite, const Vector &pos, const Vector &pivot, const Matrix &rotation, float scale, bool makeMOPs, int skipMOP, int maxMOPs) {
		RTEAssert(sprite, "Null BITMAP passed to SLTerrain::EraseSilhouette");

		int maxWidth = static_cast<int>(static_cast<float>(sprite->w + std::abs(pivot.GetFloorIntX() - (sprite->w / 2))) * scale);
		int maxHeight = static_cast<int>(static_cast<float>(sprite->h + std::abs(pivot.GetFloorIntY() - (sprite->h / 2))) * scale);
		int maxDiameter = static_cast<int>(std::sqrt(static_cast<float>(maxWidth * maxWidth + maxHeight * maxHeight)) * 2.0F);
		int skipCount = skipMOP;

		BITMAP *tempBitmap = g_SceneMan.GetIntermediateBitmapForSettlingIntoTerrain(maxDiameter);
		clear_bitmap(tempBitmap);
		pivot_scaled_sprite(tempBitmap, sprite, tempBitmap->w / 2, tempBitmap->h / 2, pivot.GetFloorIntX(), pivot.GetFloorIntY(), ftofix(rotation.GetAllegroAngle()), ftofix(scale));

		std::deque<MOPixel *> dislodgedMOPixels;

		// Test intersection between color pixels of the test bitmap and non-air pixels of the terrain, then generate and collect MOPixels that represent the terrain overlap and clear the same pixels out of the terrain.
		for (int testY = 0; testY < tempBitmap->h; ++testY) {
			for (int testX = 0; testX < tempBitmap->w; ++testX) {
				int terrX = pos.GetFloorIntX() - (tempBitmap->w / 2) + testX;
				int terrY = pos.GetFloorIntY() - (tempBitmap->h / 2) + testY;

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
					if (makeMOPs && matPixel != MaterialColorKeys::g_MaterialAir && colorPixel != ColorKeys::g_MaskColor && ++skipCount > skipMOP && dislodgedMOPixels.size() < maxMOPs) {
						skipCount = 0;
						const Material *sceneMat = g_SceneMan.GetMaterialFromID(matPixel);
						const Material *spawnMat = sceneMat->GetSpawnMaterial() ? g_SceneMan.GetMaterialFromID(sceneMat->GetSpawnMaterial()) : sceneMat;

						std::unique_ptr<Atom> terrainPixelAtom = std::make_unique<Atom>(Vector(), spawnMat->GetIndex(), nullptr, colorPixel, 2);
						std::unique_ptr<MOPixel> terrainPixel = std::make_unique<MOPixel>(colorPixel, spawnMat->GetPixelDensity(), Vector(static_cast<float>(terrX), static_cast<float>(terrY)), Vector(), terrainPixelAtom.release(), 0);
#ifndef RELEASE_BUILD
						terrainPixel->SetDescription("Dislodged Terrain Pixel from Material " + std::to_string(sceneMat->GetIndex()));
#endif
						terrainPixel->SetToHitMOs(false);
						dislodgedMOPixels.emplace_back(terrainPixel.release());
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
		// TODO: improve fit/tightness of box here.
		m_UpdatedMaterialAreas.emplace_back(Box(pos - pivot, static_cast<float>(maxWidth), static_cast<float>(maxHeight)));

		return dislodgedMOPixels;
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