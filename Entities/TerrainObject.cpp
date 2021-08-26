#include "TerrainObject.h"
#include "SceneMan.h"

namespace RTE {

	ConcreteClassInfo(TerrainObject, SceneObject, 0);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TerrainObject::Clear() {
		m_FGColorFile.Reset();
		m_FGColorBitmap = nullptr;
		m_BGColorFile.Reset();
		m_BGColorBitmap = nullptr;
		m_MaterialFile.Reset();
		m_MaterialBitmap = nullptr;
		m_BitmapOffset.Reset();
		m_OffsetDefined = false;
		m_ChildObjects.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TerrainObject::Create() {
		SceneObject::Create();

		if (!m_OffsetDefined) {
			const BITMAP *offsetCalcBitmap = nullptr;
			if (m_FGColorBitmap) {
				offsetCalcBitmap = m_FGColorBitmap;
			} else if (m_BGColorBitmap) {
				offsetCalcBitmap = m_BGColorBitmap;
			} else {
				offsetCalcBitmap = m_MaterialBitmap;
			}
			if (offsetCalcBitmap && offsetCalcBitmap->w > 24) { m_BitmapOffset.SetX(-(static_cast<float>(offsetCalcBitmap->w / 2))); }
			if (offsetCalcBitmap && offsetCalcBitmap->h > 24) { m_BitmapOffset.SetY(-(static_cast<float>(offsetCalcBitmap->h / 2))); }
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TerrainObject::Create(const TerrainObject &reference) {
		SceneObject::Create(reference);

		m_FGColorFile = reference.m_FGColorFile;
		m_FGColorBitmap = reference.m_FGColorBitmap;
		m_BGColorFile = reference.m_BGColorFile;
		m_BGColorBitmap = reference.m_BGColorBitmap;
		m_MaterialFile = reference.m_MaterialFile;
		m_MaterialBitmap = reference.m_MaterialBitmap;
		m_BitmapOffset = reference.m_BitmapOffset;
		m_OffsetDefined = reference.m_OffsetDefined;

		for (const SceneObject::SOPlacer &childObject : reference.m_ChildObjects) {
			m_ChildObjects.emplace_back(childObject);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TerrainObject::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "FGColorFile") {
			reader >> m_FGColorFile;
			m_FGColorBitmap = m_FGColorFile.GetAsBitmap();
		} else if (propName == "BGColorFile") {
			reader >> m_BGColorFile;
			m_BGColorBitmap = m_BGColorFile.GetAsBitmap();
		} else if (propName == "MaterialFile") {
			reader >> m_MaterialFile;
			m_MaterialBitmap = m_MaterialFile.GetAsBitmap();
		} else if (propName == "BitmapOffset") {
			reader >> m_BitmapOffset;
			m_OffsetDefined = true;
		} else if (propName == "AddChildObject") {
			SceneObject::SOPlacer newChildObject;
			reader >> newChildObject;
			newChildObject.SetTeam(m_Team);
			m_ChildObjects.emplace_back(newChildObject);
		} else {
			return SceneObject::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TerrainObject::Save(Writer &writer) const {
		SceneObject::Save(writer);

		writer.NewPropertyWithValue("FGColorFile", m_FGColorFile);
		writer.NewPropertyWithValue("BGColorFile", m_BGColorFile);
		writer.NewPropertyWithValue("MaterialFile", m_MaterialFile);

		if (m_OffsetDefined) { writer.NewPropertyWithValue("BitmapOffset", m_BitmapOffset); }

		for (const SOPlacer &childObject : m_ChildObjects) {
			writer.NewPropertyWithValue("AddChildObject", childObject);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP * TerrainObject::GetGraphicalIcon() const {
		if (!m_FGColorBitmap) {
			return m_BGColorBitmap;
		}

		// Checking if the FG has anything to show, if not, show the background layer instead
		int piece = m_FGColorBitmap->w / 10;
		if (getpixel(m_FGColorBitmap, m_FGColorBitmap->w / 2, m_FGColorBitmap->h / 2) != g_MaskColor || getpixel(m_FGColorBitmap, piece, piece) != g_MaskColor || getpixel(m_FGColorBitmap, m_FGColorBitmap->w - piece, piece) != g_MaskColor || getpixel(m_FGColorBitmap, piece, m_FGColorBitmap->h - piece) != g_MaskColor) {
			return m_FGColorBitmap;
		} else {
			return m_BGColorBitmap;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TerrainObject::SetTeam(int team) {
		SceneObject::SetTeam(team);
		for (SceneObject::SOPlacer &childObject : m_ChildObjects) {
			childObject.SetTeam(team);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool TerrainObject::IsOnScenePoint(Vector &scenePoint) const {
		if (!m_FGColorBitmap) {
			return false;
		}
		// TODO: TAKE CARE OF WRAPPING
		/*
		// Take care of wrapping situations
		bitmapPos = m_Pos + m_BitmapOffset;
		Vector aScenePoint[4];
		aScenePoint[0] = scenePoint;
		int passes = 1;

		// See if need to double draw this across the scene seam if we're being drawn onto a scene-wide bitmap
		if (targetPos.IsZero()) {
			if (g_SceneMan.SceneWrapsX()) {
				if (bitmapPos.m_X < m_FGColorBitmap->w) {
					aScenePoint[passes] = aScenePoint[0];
					aScenePoint[passes].m_X += g_SceneMan.GetSceneWidth();
					passes++;
				} else if (aScenePoint[0].m_X > targetBitmap->w - m_FGColorBitmap->w) {
					aScenePoint[passes] = aScenePoint[0];
					aScenePoint[passes].m_X -= g_SceneMan.GetSceneWidth();
					passes++;
				}
			}
			if (g_SceneMan.SceneWrapsY()) {

			}
		}

		// Check all the passes needed
		for (int i = 0; i < passes; ++i) {

			if (IsWithinBox(aScenePoint[i], m_Pos + m_BitmapOffset, m_FGColorBitmap->w, m_FGColorBitmap->h)) {
				if (getpixel(m_FGColorBitmap, aScenePoint[i].m_X, aScenePoint[i].m_Y) != g_MaskColor ||
					(m_BGColorBitmap && getpixel(m_BGColorBitmap, aScenePoint[i].m_X, aScenePoint[i].m_Y) != g_MaskColor) ||
					(m_MaterialBitmap && getpixel(m_MaterialBitmap, aScenePoint[i].m_X, aScenePoint[i].m_Y) != g_MaterialAir))
					return true;
			}
		}
		*/
		Vector bitmapPos = m_Pos + m_BitmapOffset;
		if (WithinBox(scenePoint, bitmapPos, m_FGColorBitmap->w, m_FGColorBitmap->h)) {
			// Scene point on the bitmap
			Vector bitmapPoint = scenePoint - bitmapPos;
			if (getpixel(m_FGColorBitmap, bitmapPoint.m_X, bitmapPoint.m_Y) != g_MaskColor || (m_BGColorBitmap && getpixel(m_BGColorBitmap, bitmapPoint.m_X, bitmapPoint.m_Y) != g_MaskColor) || (m_MaterialBitmap && getpixel(m_MaterialBitmap, bitmapPoint.m_X, bitmapPoint.m_Y) != g_MaterialAir)) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TerrainObject::Draw(BITMAP *targetBitmap, const Vector &targetPos, DrawMode mode, bool onlyPhysical) const {
		if (!m_FGColorBitmap) {
			RTEAbort("TerrainObject's bitmaps are null when drawing!");
		}

		// Take care of wrapping situations
		Vector aDrawPos[4];
		aDrawPos[0] = m_Pos + m_BitmapOffset - targetPos;
		int passes = 1;

		// See if need to double draw this across the scene seam if we're being drawn onto a scenewide bitmap
		if (targetPos.IsZero() && g_SceneMan.GetSceneWidth() <= targetBitmap->w) {
			if (aDrawPos[0].m_X < m_FGColorBitmap->w) {
				aDrawPos[passes] = aDrawPos[0];
				aDrawPos[passes].m_X += targetBitmap->w;
				passes++;
			} else if (aDrawPos[0].m_X > targetBitmap->w - m_FGColorBitmap->w) {
				aDrawPos[passes] = aDrawPos[0];
				aDrawPos[passes].m_X -= targetBitmap->w;
				passes++;
			}
		} else {
			// Only screenwide target bitmap, so double draw within the screen if the screen is straddling a scene seam
			if (g_SceneMan.SceneWrapsX()) {
				int sceneWidth = g_SceneMan.GetSceneWidth();
				if (targetPos.m_X < 0) {
					aDrawPos[passes] = aDrawPos[0];
					aDrawPos[passes].m_X -= sceneWidth;
					passes++;
				}
				if (targetPos.m_X + targetBitmap->w > sceneWidth) {
					aDrawPos[passes] = aDrawPos[0];
					aDrawPos[passes].m_X += sceneWidth;
					passes++;
				}
			}
		}

		// Draw all the passes needed
		for (int i = 0; i < passes; ++i) {
			if (mode == g_DrawColor) {
				masked_blit(m_BGColorBitmap, targetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), m_BGColorBitmap->w, m_BGColorBitmap->h);
				masked_blit(m_FGColorBitmap, targetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), m_FGColorBitmap->w, m_FGColorBitmap->h);
			} else if (mode == g_DrawMaterial) {
				masked_blit(m_MaterialBitmap, targetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), m_MaterialBitmap->w, m_MaterialBitmap->h);
			} else if (mode == g_DrawLess) {
				masked_blit(m_FGColorBitmap, targetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), m_FGColorBitmap->w, m_FGColorBitmap->h);
			} else if (mode == g_DrawTrans) {
				draw_trans_sprite(targetBitmap, m_FGColorBitmap, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY());
				draw_trans_sprite(targetBitmap, m_BGColorBitmap, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY());
			}
		}
	}
}