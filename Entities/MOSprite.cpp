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
#include "PresetMan.h"
#include "AEmitter.h"

namespace RTE {

AbstractClassInfo(MOSprite, MovableObject)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MOSprite, effectively
//                  resetting the members of this abstraction level only.

void MOSprite::Clear()
{
    m_SpriteFile.Reset();
    m_aSprite.clear();
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

    if (!m_aSprite.empty() && m_aSprite.at(0))
    {
        // Set default sprite offset
        if (m_SpriteOffset.IsZero()) { m_SpriteOffset.SetXY(static_cast<float>(-m_aSprite.at(0)->w) / 2.0F, static_cast<float>(-m_aSprite.at(0)->h) / 2.0F); }

        // Calc maximum dimensions from the Pos, based on the sprite
        float maxX = std::max(std::fabs(m_SpriteOffset.GetX()), std::fabs(static_cast<float>(m_aSprite.at(0)->w) + m_SpriteOffset.GetX()));
        float maxY = std::max(std::fabs(m_SpriteOffset.GetY()), std::fabs(static_cast<float>(m_aSprite.at(0)->h) + m_SpriteOffset.GetY()));
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
    m_SpriteOffset.SetXY(static_cast<float>(-m_aSprite.at(0)->w) / 2.0F, static_cast<float>(-m_aSprite.at(0)->h) / 2.0F);

    m_HFlipped = false;

    // Calc maximum dimensions from the Pos, based on the sprite
    float maxX = std::max(std::fabs(m_SpriteOffset.GetX()), std::fabs(static_cast<float>(m_aSprite.at(0)->w) + m_SpriteOffset.GetX()));
    float maxY = std::max(std::fabs(m_SpriteOffset.GetY()), std::fabs(static_cast<float>(m_aSprite.at(0)->h) + m_SpriteOffset.GetY()));
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

int MOSprite::ReadProperty(const std::string_view &propName, Reader &reader)
{
    if (propName == "SpriteFile")
        reader >> m_SpriteFile;
	else if (propName == "FrameCount") {
		reader >> m_FrameCount;
		m_aSprite.reserve(m_FrameCount);
	} else if (propName == "SpriteOffset")
        reader >> m_SpriteOffset;
    else if (propName == "SpriteAnimMode")
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
        else if (mode == "LOOPWHENMOVING")
            m_SpriteAnimMode = LOOPWHENMOVING;
        else
            Abort
*/
    }
    else if (propName == "SpriteAnimDuration")
        reader >> m_SpriteAnimDuration;
    else if (propName == "HFlipped")
        reader >> m_HFlipped;
    else if (propName == "Rotation")
        reader >> m_Rotation;
    else if (propName == "AngularVel")
        reader >> m_AngularVel;
    else if (propName == "SettleMaterialDisabled")
        reader >> m_SettleMaterialDisabled;
    else if (propName == "EntryWound")
        m_pEntryWound = dynamic_cast<const AEmitter *>(g_PresetMan.GetEntityPreset(reader));
    else if (propName == "ExitWound")
        m_pExitWound = dynamic_cast<const AEmitter *>(g_PresetMan.GetEntityPreset(reader));
    else
        return MovableObject::ReadProperty(propName, reader);

    return 0;
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

	// First, check that the sprite has enough frames to even have an animation and override the setting if not
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
	unsigned int frameTime = m_SpriteAnimDuration / m_FrameCount;
	unsigned int prevFrame = m_Frame;

	if (m_SpriteAnimTimer.GetElapsedSimTimeMS() > frameTime) {
		switch (m_SpriteAnimMode) {
		    case ALWAYSLOOP:
			    m_Frame = ((m_Frame + 1) % m_FrameCount);
                m_SpriteAnimTimer.Reset();
			    break;
		    case ALWAYSRANDOM:
			    while (m_Frame == prevFrame) {
					m_Frame = RandomNum<int>(0, m_FrameCount - 1);
			    }
                m_SpriteAnimTimer.Reset();
			    break;
		    case ALWAYSPINGPONG:
			    if (m_Frame == m_FrameCount - 1) {
				    m_SpriteAnimIsReversingFrames = true;
			    } else if (m_Frame == 0) {
				    m_SpriteAnimIsReversingFrames = false;
			    }
			    m_SpriteAnimIsReversingFrames ? m_Frame-- : m_Frame++;
                m_SpriteAnimTimer.Reset();
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

void MOSprite::Draw(BITMAP *pTargetBitmap,
                    const Vector &targetPos,
                    DrawMode mode,
                    bool onlyPhysical) const
{
    if (!m_aSprite[m_Frame])
        RTEAbort("Sprite frame pointer is null when drawing MOSprite!");

    // Apply offsets and positions.
    Vector spriteOffset;
    if (m_HFlipped)
        spriteOffset.SetXY(-(m_aSprite[m_Frame]->w + m_SpriteOffset.m_X), m_SpriteOffset.m_Y);
    else
        spriteOffset = m_SpriteOffset;

    Vector spritePos(m_Pos + spriteOffset - targetPos);

    // Take care of wrapping situations
    Vector aDrawPos[4];
    aDrawPos[0] = spritePos;
    int passes = 1;

    // Only bother with wrap drawing if the scene actually wraps around
    if (g_SceneMan.SceneWrapsX())
    {
        // See if need to double draw this across the scene seam if we're being drawn onto a scenewide bitmap
        if (targetPos.IsZero() && m_WrapDoubleDraw)
        {
            if (spritePos.m_X < m_aSprite[m_Frame]->w)
            {
                aDrawPos[passes] = spritePos;
                aDrawPos[passes].m_X += pTargetBitmap->w;
                passes++;
            }
            else if (spritePos.m_X > pTargetBitmap->w - m_aSprite[m_Frame]->w)
            {
                aDrawPos[passes] = spritePos;
                aDrawPos[passes].m_X -= pTargetBitmap->w;
                passes++;
            }
        }
        // Only screenwide target bitmap, so double draw within the screen if the screen is straddling a scene seam
        else if (m_WrapDoubleDraw)
        {
            if (targetPos.m_X < 0)
            {
                aDrawPos[passes] = aDrawPos[0];
                aDrawPos[passes].m_X -= g_SceneMan.GetSceneWidth();
                passes++;
            }
            if (targetPos.m_X + pTargetBitmap->w > g_SceneMan.GetSceneWidth())
            {
                aDrawPos[passes] = aDrawPos[0];
                aDrawPos[passes].m_X += g_SceneMan.GetSceneWidth();
                passes++;
            }
        }
    }

    for (int i = 0; i < passes; ++i)
    {
        if (mode == g_DrawMaterial) {
            RTEAbort("Ordered to draw an MOSprite in its material, which is not possible!");
        }
        else if (mode == g_DrawAir)
            draw_character_ex(pTargetBitmap, m_aSprite[m_Frame], aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), g_MaterialAir, -1);
        else if (mode == g_DrawMask)
            draw_character_ex(pTargetBitmap, m_aSprite[m_Frame], aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), g_MaskColor, -1);
        else if (mode == g_DrawWhite)
            draw_character_ex(pTargetBitmap, m_aSprite[m_Frame], aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), g_WhiteColor, -1);
        else if (mode == g_DrawMOID)
        {
            int spriteX = aDrawPos[i].GetFloorIntX();
            int spriteY = aDrawPos[i].GetFloorIntY();
            draw_character_ex(pTargetBitmap, m_aSprite[m_Frame], spriteX, spriteY, m_MOID, -1);
            g_SceneMan.RegisterMOIDDrawing(spriteX, spriteY, spriteX + m_aSprite[m_Frame]->w, spriteY + m_aSprite[m_Frame]->h);
		}
        else if (mode == g_DrawNoMOID)
            draw_character_ex(pTargetBitmap, m_aSprite[m_Frame], aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), g_NoMOID, -1);
        else if (mode == g_DrawTrans)
            draw_trans_sprite(pTargetBitmap, m_aSprite[m_Frame], aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY());
        else if (mode == g_DrawAlpha)
        {
            set_alpha_blender();
            draw_trans_sprite(pTargetBitmap, m_aSprite[m_Frame], aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY());
        }
        else {
            if (!m_HFlipped)
                draw_sprite(pTargetBitmap, m_aSprite[m_Frame], aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY());
            else
                draw_sprite_h_flip(pTargetBitmap, m_aSprite[m_Frame], aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY());
        }
    }
}

} // namespace RTE