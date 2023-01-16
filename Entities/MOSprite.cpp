//////////////////////////////////////////////////////////////////////////////////////////
// File:            MOSprite.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the MOSprite class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "MOSprite.h"

#include "AEmitter.h"
#include "PresetMan.h"
#include "ThreadMan.h"

namespace RTE {

AbstractClassInfo(MOSprite, MovableObject);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MOSprite, effectively
//                  resetting the members of this abstraction level only.

void MOSprite::Clear()
{
    m_SpriteFile.Reset();
    m_aSprite.clear();
	m_IconFile.Reset();
	m_GraphicalIcon = nullptr;
    m_FrameCount = 1;
    m_SpriteOffset.Reset();
    m_Frame = 0;
    m_SpriteAnimMode = NOANIM;
    m_SpriteAnimDuration = 500;
    m_SpriteAnimTimer.Reset();
    m_SpriteAnimIsReversingFrames = false;
    m_HFlipped = false;
    m_SpriteRadius = 1.0F;
    m_SpriteDiameter = 2.0F;
    m_Rotation.Reset();
    m_PrevRotation.Reset();
    m_AngularVel = 0;
    m_PrevAngVel = 0;
	m_AngOscillations = 0;
    m_SettleMaterialDisabled = false;
    m_pEntryWound = 0;
    m_pExitWound = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MOSprite object ready for use.

int MOSprite::Create()
{
    if (MovableObject::Create() < 0)
        return -1;

    // Post-process reading
	m_aSprite.clear();
    m_SpriteFile.GetAsAnimation(m_aSprite, m_FrameCount);

    if (!m_aSprite.empty() && m_aSprite[0])
    {
        // Set default sprite offset
        if (m_SpriteOffset.IsZero()) { m_SpriteOffset.SetXY(static_cast<float>(-m_aSprite[0]->w) / 2.0F, static_cast<float>(-m_aSprite[0]->h) / 2.0F); }

        // Calc maximum dimensions from the Pos, based on the sprite
        float maxX = std::max(std::fabs(m_SpriteOffset.GetX()), std::fabs(static_cast<float>(m_aSprite[0]->w) + m_SpriteOffset.GetX()));
        float maxY = std::max(std::fabs(m_SpriteOffset.GetY()), std::fabs(static_cast<float>(m_aSprite[0]->h) + m_SpriteOffset.GetY()));
        m_SpriteRadius = std::sqrt((maxX * maxX) + (maxY * maxY));
        m_SpriteDiameter = m_SpriteRadius * 2.0F;
    }
    else
        return -1;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MOSprite object ready for use.

int MOSprite::Create(ContentFile spriteFile,
                     const int frameCount,
                     const float mass,
                     const Vector &position,
                     const Vector &velocity,
                     const unsigned long lifetime)
{
    MovableObject::Create(mass, position, velocity, 0, 0, lifetime);

    m_SpriteFile = spriteFile;
    m_FrameCount = frameCount;
	m_aSprite.clear();
    m_SpriteFile.GetAsAnimation(m_aSprite, m_FrameCount);
    m_SpriteOffset.SetXY(static_cast<float>(-m_aSprite[0]->w) / 2.0F, static_cast<float>(-m_aSprite[0]->h) / 2.0F);

    m_HFlipped = false;

    // Calc maximum dimensions from the Pos, based on the sprite
    float maxX = std::max(std::fabs(m_SpriteOffset.GetX()), std::fabs(static_cast<float>(m_aSprite[0]->w) + m_SpriteOffset.GetX()));
    float maxY = std::max(std::fabs(m_SpriteOffset.GetY()), std::fabs(static_cast<float>(m_aSprite[0]->h) + m_SpriteOffset.GetY()));
    m_SpriteRadius = std::sqrt((maxX * maxX) + (maxY * maxY));
    m_SpriteDiameter = m_SpriteRadius * 2.0F;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a MOSprite to be identical to another, by deep copy.

int MOSprite::Create(const MOSprite &reference)
{
    MovableObject::Create(reference);

    if (reference.m_aSprite.empty())
        return -1;

    m_SpriteFile = reference.m_SpriteFile;
	m_IconFile = reference.m_IconFile;
	m_GraphicalIcon = m_IconFile.GetAsBitmap();

    m_FrameCount = reference.m_FrameCount;
    m_Frame = reference.m_Frame;
	m_aSprite = reference.m_aSprite;
    m_SpriteOffset = reference.m_SpriteOffset;
    m_SpriteAnimMode = reference.m_SpriteAnimMode;
    m_SpriteAnimDuration = reference.m_SpriteAnimDuration;
    m_HFlipped = reference.m_HFlipped;
    m_SpriteRadius = reference.m_SpriteRadius;
    m_SpriteDiameter = reference.m_SpriteDiameter;

    m_Rotation = reference.m_Rotation;
    m_PrevRotation = reference.m_PrevRotation;
    m_AngularVel = reference.m_AngularVel;
    m_SettleMaterialDisabled = reference.m_SettleMaterialDisabled;
    m_pEntryWound = reference.m_pEntryWound;
    m_pExitWound = reference.m_pExitWound;
//    if (reference.m_pExitWound)  Not doing anymore since we're not owning
//        m_pExitWound = dynamic_cast<AEmitter *>(reference.m_pExitWound->Clone());

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int MOSprite::ReadProperty(const std::string_view &propName, Reader &reader) {
	StartPropertyList(return MovableObject::ReadProperty(propName, reader));
    
    MatchProperty("SpriteFile", { reader >> m_SpriteFile; });
	MatchProperty("IconFile", {
		reader >> m_IconFile;
		m_GraphicalIcon = m_IconFile.GetAsBitmap();
	});
	MatchProperty("FrameCount", {
		reader >> m_FrameCount;
		m_aSprite.reserve(m_FrameCount);
	});
	MatchProperty("SpriteOffset", { reader >> m_SpriteOffset; });
    MatchProperty("SpriteAnimMode",
    {
//        string mode;
//        reader >> mode;
        int mode;
        reader >> mode;
        m_SpriteAnimMode = (SpriteAnimMode)mode;
/*
        if (mode == "NOANIM")
            m_SpriteAnimMode = NOANIM;
        else if (mode == "ALWAYSLOOP")
            m_SpriteAnimMode = ALWAYSLOOP;
        else if (mode == "ALWAYSPINGPONG")
            m_SpriteAnimMode = ALWAYSPINGPONG;
        else if (mode == "LOOPWHENACTIVE")
            m_SpriteAnimMode = LOOPWHENACTIVE;
        else
            Abort
*/
    });
    MatchProperty("SpriteAnimDuration", { reader >> m_SpriteAnimDuration; });
    MatchProperty("HFlipped", { reader >> m_HFlipped; });
    MatchProperty("Rotation", { reader >> m_Rotation; });
    MatchProperty("AngularVel", { reader >> m_AngularVel; });
    MatchProperty("SettleMaterialDisabled", { reader >> m_SettleMaterialDisabled; });
    MatchProperty("EntryWound", { m_pEntryWound = dynamic_cast<const AEmitter *>(g_PresetMan.GetEntityPreset(reader)); });
    MatchProperty("ExitWound", { m_pExitWound = dynamic_cast<const AEmitter *>(g_PresetMan.GetEntityPreset(reader)); });

    EndPropertyList;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetEntryWound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets entry wound emitter for this MOSprite
void MOSprite::SetEntryWound(std::string presetName, std::string moduleName)
{
	if (presetName == "")
		m_pEntryWound = 0;
	else
		m_pEntryWound = dynamic_cast<const AEmitter *>(g_PresetMan.GetEntityPreset("AEmitter", presetName, moduleName));
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetExitWound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets exit wound emitter for this MOSprite
void MOSprite::SetExitWound(std::string presetName, std::string moduleName)
{
	if (presetName == "")
		m_pExitWound = 0;
	else
		m_pExitWound = dynamic_cast<const AEmitter *>(g_PresetMan.GetEntityPreset("AEmitter", presetName, moduleName));
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetEntryWoundPresetName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns entry wound emitter preset name for this MOSprite

std::string MOSprite::GetEntryWoundPresetName() const
{
	return m_pEntryWound ? m_pEntryWound->GetPresetName() : ""; 
};

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetExitWoundPresetName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns exit wound emitter preset name for this MOSprite

std::string MOSprite::GetExitWoundPresetName() const
{ 
	return m_pExitWound ? m_pExitWound->GetPresetName() : ""; 
};

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this MOSprite with a Writer for
//                  later recreation with Create(Reader &reader);

int MOSprite::Save(Writer &writer) const
{
    MovableObject::Save(writer);
// TODO: Make proper save system that knows not to save redundant data!
/*
    writer.NewProperty("SpriteFile");
    writer << m_SpriteFile;
    writer.NewProperty("FrameCount");
    writer << m_FrameCount;
    writer.NewProperty("SpriteOffset");
    writer << m_SpriteOffset;
    writer.NewProperty("SpriteAnimMode");
    writer << m_SpriteAnimMode;
    writer.NewProperty("SpriteAnimDuration");
    writer << m_SpriteAnimDuration;
    writer.NewProperty("HFlipped");
    writer << m_HFlipped;
    writer.NewProperty("Rotation");
    writer << m_Rotation.GetRadAngle();
    writer.NewProperty("AngularVel");
    writer << m_AngularVel;
    writer.NewProperty("SettleMaterialDisabled");
    writer << m_SettleMaterialDisabled;
    writer.NewProperty("EntryWound");
    writer << m_pEntryWound;
    writer.NewProperty("ExitWound");
    writer << m_pExitWound;
*/
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the MOSprite object.

void MOSprite::Destroy(bool notInherited)
{
//    delete m_pEntryWound; Not doing this anymore since we're not owning
//    delete m_pExitWound;

    if (!notInherited)
        MovableObject::Destroy();
    Clear();
}

bool MOSprite::HitTestAtPixel(int pixelX, int pixelY) const {
    if (!GetsHitByMOs() || GetRootParent()->GetTraveling()) {
        return false;
    }

    Vector distanceBetweenTestPositionAndMO = g_SceneMan.ShortestDistance(m_Pos, Vector(static_cast<float>(pixelX), static_cast<float>(pixelY)));
    if (distanceBetweenTestPositionAndMO.MagnitudeIsGreaterThan(m_SpriteRadius)) {
        return false;
    }

    // Check the scene position in the current local space of the MO, accounting for Position, Sprite Offset, Angle and HFlipped.
    //TODO Account for Scale as well someday, maybe.
    Matrix rotation = m_Rotation; // <- Copy to non-const variable so / operator overload works.
    Vector entryPos = (distanceBetweenTestPositionAndMO / rotation).GetXFlipped(m_HFlipped) - m_SpriteOffset;
    int localX = entryPos.GetFloorIntX();
    int localY = entryPos.GetFloorIntY();

    BITMAP* sprite = m_aSprite[m_Frame];
    return is_inside_bitmap(sprite, localX, localY, 0) && _getpixel(sprite, localX, localY) != ColorKeys::g_MaskColor;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetFrame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Hard-sets the frame this sprite is supposed to show.

void MOSprite::SetFrame(unsigned int newFrame)
{
    if (newFrame < 0)
        newFrame = 0;
    if (newFrame >= m_FrameCount)
        newFrame = m_FrameCount - 1;

    m_Frame = newFrame;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetNextFrame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Hard-sets the frame this sprite is supposed to show, to the
//                  consecutive one after the current one. If currently the last fame is
//                  this will set it to the be the first, looping the animation.

bool MOSprite::SetNextFrame()
{
    if (++m_Frame >= m_FrameCount)
    {
        m_Frame = 0;
        return true;
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsOnScenePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this' current graphical representation overlaps
//                  a point in absolute scene coordinates.

bool MOSprite::IsOnScenePoint(Vector &scenePoint) const
{
    if (!m_aSprite[m_Frame])
        return false;
// TODO: TAKE CARE OF WRAPPING
/*
    // Take care of wrapping situations
    bitmapPos = m_Pos + m_BitmapOffset;
    Vector aScenePoint[4];
    aScenePoint[0] = scenePoint;
    int passes = 1;

    // See if need to double draw this across the scene seam if we're being drawn onto a scenewide bitmap
    if (targetPos.IsZero())
    {
        if (g_SceneMan.SceneWrapsX())
        {
            if (bitmapPos.m_X < m_pFGColor->w)
            {
                aScenePoint[passes] = aScenePoint[0];
                aScenePoint[passes].m_X += g_SceneMan.GetSceneWidth();
                passes++;
            }
            else if (aScenePoint[0].m_X > pTargetBitmap->w - m_pFGColor->w)
            {
                aScenePoint[passes] = aScenePoint[0];
                aScenePoint[passes].m_X -= g_SceneMan.GetSceneWidth();
                passes++;
            }
        }
        if (g_SceneMan.SceneWrapsY())
        {
            
        }
    }

    // Check all the passes needed
    for (int i = 0; i < passes; ++i)
    {
        if (IsWithinBox(aScenePoint[i], m_Pos + m_BitmapOffset, m_pFGColor->w, m_pFGColor->h))
        {
            if (getpixel(m_pFGColor, aScenePoint[i].m_X, aScenePoint[i].m_Y) != g_MaskColor ||
               (m_pBGColor && getpixel(m_pBGColor, aScenePoint[i].m_X, aScenePoint[i].m_Y) != g_MaskColor) ||
               (m_pMaterial && getpixel(m_pMaterial, aScenePoint[i].m_X, aScenePoint[i].m_Y) != g_MaterialAir))
               return true;
        }
    }
*/
    if (WithinBox(scenePoint, m_Pos.m_X - m_SpriteRadius, m_Pos.m_Y - m_SpriteRadius, m_Pos.m_X + m_SpriteRadius, m_Pos.m_Y + m_SpriteRadius))
    {
        // Get scene point in object's relative space
        Vector spritePoint = scenePoint - m_Pos;
        spritePoint.FlipX(m_HFlipped);
        // Check over overlap
        int pixel = getpixel(m_aSprite[m_Frame], spritePoint.m_X - m_SpriteOffset.m_X, spritePoint.m_Y - m_SpriteOffset.m_Y);
        // Check that it isn't outside the bitmap, and not of the key color
        if (pixel != -1 && pixel != g_MaskColor)
           return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RotateOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Rotates a vector offset from this MORotating's position according to
//                  the rotate angle and flipping.

Vector MOSprite::RotateOffset(const Vector &offset) const
{
    Vector rotOff(offset.GetXFlipped(m_HFlipped));
    rotOff *= const_cast<Matrix &>(m_Rotation);
    return rotOff;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UnRotateOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Rotates a vector offset from this MORotating's position according to
//                  the NEGATIVE rotate angle and takes flipping into account.

Vector MOSprite::UnRotateOffset(const Vector &offset) const
{
    Vector rotOff(offset.GetXFlipped(m_HFlipped));
    rotOff /= const_cast<Matrix &>(m_Rotation);
    return rotOff;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Pure v. method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MOSprite. Supposed to be done every frame.

void MOSprite::Update() {
	MovableObject::Update();

    // Check that the sprite has enough frames to even have an animation and override the setting if not
	if (m_FrameCount > 1) {
		// If animation mode is set to something other than ALWAYSLOOP but only has 2 frames, override it because it's pointless
		if ((m_SpriteAnimMode == ALWAYSRANDOM || m_SpriteAnimMode == ALWAYSPINGPONG) && m_FrameCount == 2) {
			m_SpriteAnimMode = ALWAYSLOOP;
		} else if (m_SpriteAnimMode == OVERLIFETIME) {
			// If animation mode is set to over lifetime but lifetime is unlimited, override to always loop otherwise it will never animate.
			if (m_Lifetime == 0) {
				m_SpriteAnimMode = ALWAYSLOOP;
			} else {
				double lifeTimeFrame = static_cast<double>(m_FrameCount) * (m_AgeTimer.GetElapsedSimTimeMS() / static_cast<double>(m_Lifetime));
				m_Frame = static_cast<int>(std::floor(lifeTimeFrame));
				if (m_Frame >= m_FrameCount) { m_Frame = m_FrameCount - 1; }
				return;
			}
		}
	} else {
		m_SpriteAnimMode = NOANIM;
	}

    // Animate the sprite, if applicable
	double frameTime = m_SpriteAnimDuration / m_FrameCount;
    while (m_SpriteAnimTimer.GetElapsedSimTimeMS() > frameTime) {
        unsigned int prevFrame = m_Frame;
        double newTime = frameTime > 0 ? m_SpriteAnimTimer.GetElapsedSimTimeMS() - frameTime : 0;
        m_SpriteAnimTimer.SetElapsedSimTimeMS(newTime);
		switch (m_SpriteAnimMode) {
		    case ALWAYSLOOP:
			    m_Frame = ((m_Frame + 1) % m_FrameCount);
			    break;
		    case ALWAYSRANDOM:
			    while (m_Frame == prevFrame) {
					m_Frame = RandomNum<int>(0, m_FrameCount - 1);
			    }
			    break;
		    case ALWAYSPINGPONG:
			    if (m_Frame == m_FrameCount - 1) {
				    m_SpriteAnimIsReversingFrames = true;
			    } else if (m_Frame == 0) {
				    m_SpriteAnimIsReversingFrames = false;
			    }
			    m_SpriteAnimIsReversingFrames ? m_Frame-- : m_Frame++;
			    break;
		    default:
			    break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this MOSprite's current graphical representation to a
//                  BITMAP of choice.

void MOSprite::Draw(BITMAP * targetBitmap,
                    const Vector &targetPos,
                    DrawMode mode,
                    bool onlyPhysical) const
{
    BITMAP *currentFrame = m_aSprite[m_Frame];
    if (!currentFrame) {
        RTEAbort("Sprite frame pointer is null when drawing MOSprite!");
    }

    // Apply offsets and positions.
    Vector spriteOffset;
    if (m_HFlipped) {
        spriteOffset.SetXY(-(currentFrame->w + m_SpriteOffset.m_X), m_SpriteOffset.m_Y);
    } else {
        spriteOffset = m_SpriteOffset;
    }

    Vector prevSpritePos(m_PrevPos + spriteOffset - targetPos);
    Vector spritePos(m_Pos + spriteOffset - targetPos);

    if (mode == g_DrawMOID) {
        g_SceneMan.RegisterMOIDDrawing(m_MOID, spritePos.GetX(), spritePos.GetY(), spritePos.GetX() + currentFrame->w, spritePos.GetY() + currentFrame->h);
		return;
	}

    bool hFlipped = m_HFlipped;
    bool wrapDoubleDraw = m_WrapDoubleDraw;

    auto renderFunc = [=](float interpolationAmount) {
        BITMAP* pTargetBitmap = targetBitmap;
        Vector renderPos = g_SceneMan.Lerp(0.0F, 1.0F, prevSpritePos, spritePos, interpolationAmount);
        if (targetBitmap == nullptr) {
            pTargetBitmap = g_ThreadMan.GetRenderTarget();
            renderPos -= g_ThreadMan.GetRenderOffset();
        }

        // Take care of wrapping situations
        std::array<Vector, 4> drawPositions = { renderPos };
        int drawPasses = 1;
        if (g_SceneMan.SceneWrapsX()) {
            if (renderPos.IsZero() && wrapDoubleDraw) {
                if (spritePos.GetFloorIntX() < currentFrame->w) {
                    drawPositions[drawPasses] = spritePos;
                    drawPositions[drawPasses].m_X += static_cast<float>(pTargetBitmap->w);
                    drawPasses++;
                } else if (spritePos.GetFloorIntX() > pTargetBitmap->w - currentFrame->w) {
                    drawPositions[drawPasses] = spritePos;
                    drawPositions[drawPasses].m_X -= static_cast<float>(pTargetBitmap->w);
                    drawPasses++;
                }
            } else if (wrapDoubleDraw) {
                if (renderPos.m_X < 0) {
                    drawPositions[drawPasses] = drawPositions[0];
                    drawPositions[drawPasses].m_X += static_cast<float>(g_SceneMan.GetSceneWidth());
                    drawPasses++;
                }
                if (renderPos.GetFloorIntX() + pTargetBitmap->w > g_SceneMan.GetSceneWidth()) {
                    drawPositions[drawPasses] = drawPositions[0];
                    drawPositions[drawPasses].m_X -= static_cast<float>(g_SceneMan.GetSceneWidth());
                    drawPasses++;
                }
            }
        }
        if (g_SceneMan.SceneWrapsY()) {
            if (renderPos.IsZero() && wrapDoubleDraw) {
                if (spritePos.GetFloorIntY() < currentFrame->h) {
                    drawPositions[drawPasses] = spritePos;
                    drawPositions[drawPasses].m_Y += static_cast<float>(pTargetBitmap->h);
                    drawPasses++;
                } else if (spritePos.GetFloorIntY() > pTargetBitmap->h - currentFrame->h) {
                    drawPositions[drawPasses] = spritePos;
                    drawPositions[drawPasses].m_Y -= static_cast<float>(pTargetBitmap->h);
                    drawPasses++;
                }
            } else if (wrapDoubleDraw) {
                if (renderPos.m_Y < 0) {
                    drawPositions[drawPasses] = drawPositions[0];
                    drawPositions[drawPasses].m_Y += static_cast<float>(g_SceneMan.GetSceneHeight());
                    drawPasses++;
                }
                if (renderPos.GetFloorIntY() + pTargetBitmap->h > g_SceneMan.GetSceneHeight()) {
                    drawPositions[drawPasses] = drawPositions[0];
                    drawPositions[drawPasses].m_Y -= static_cast<float>(g_SceneMan.GetSceneHeight());
                    drawPasses++;
                }
            }
        }

        for (int i = 0; i < drawPasses; ++i)
        {
            int spriteX = drawPositions[i].GetFloorIntX();
            int spriteY = drawPositions[i].GetFloorIntY();
            switch (mode) {
                case g_DrawMaterial:
                    RTEAbort("Ordered to draw an MOSprite in its material, which is not possible!");
                    break;
                case g_DrawWhite:
                    draw_character_ex(pTargetBitmap, currentFrame, spriteX, spriteY, g_WhiteColor, -1);
                    break;
                    break;
                case g_DrawTrans:
                    draw_trans_sprite(pTargetBitmap, currentFrame, spriteX, spriteY);
                    break;
                case g_DrawAlpha:
                    set_alpha_blender();
                    draw_trans_sprite(pTargetBitmap, currentFrame, spriteX, spriteY);
                    break;
                default:
                    if (!hFlipped) {
                        draw_sprite(pTargetBitmap, currentFrame, spriteX, spriteY);
                    } else {
                        draw_sprite_h_flip(pTargetBitmap, currentFrame, spriteX, spriteY);
                    }
            }
        }
    };

    if (targetBitmap == nullptr) {
        g_ThreadMan.GetSimRenderQueue().push_back(renderFunc);
    } else {
        renderFunc(1.0F);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MOSprite::NewFrame() {
    MovableObject::NewFrame();

    m_PrevRotation = m_Rotation;
    m_PrevAngVel = m_AngularVel;
}


} // namespace RTE