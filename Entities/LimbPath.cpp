//////////////////////////////////////////////////////////////////////////////////////////
// File:            LimbPath.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the LimbPath class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "LimbPath.h"
#include "PresetMan.h"
#include "SLTerrain.h"

namespace RTE {

ConcreteClassInfo(LimbPath, Entity, 20);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this LimbPath, effectively
//                  resetting the members of this abstraction level only.

void LimbPath::Clear()
{
    m_Start.Reset();
    m_StartSegCount = 0;
    m_Segments.clear();
//    m_CurrentSegment = 0;
    m_FootCollisionsDisabledSegment = -1;
    m_SegProgress = 0.0;
    for (int i = 0; i < SPEEDCOUNT; ++i)
        m_TravelSpeed[i] = 0.0;
    m_WhichSpeed = NORMAL;
    m_PushForce = 0.0;
    m_JointPos.Reset();
    m_JointVel.Reset();
    m_Rotation.Reset();
    m_TimeLeft = 0.0;
    m_PathTimer.Reset();
    m_SegTimer.Reset();
    m_TotalLength = 0.0;
    m_RegularLength = 0.0;
    m_SegmentDone = false;
    m_Ended = true;
    m_HFlipped = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the LimbPath object ready for use.

int LimbPath::Create()
{
    // Read all the properties
    if (Entity::Create() < 0)
        return -1;

    if (m_Segments.size() > 0)
        m_CurrentSegment = m_Segments.begin();
    else
        m_CurrentSegment = m_Segments.end();

    Terminate();

    return 0;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the LimbPath object ready for use.

int LimbPath::Create(const Vector &startPoint,
                     const unsigned int segCount,
                     const Vector *aSegArray,
                     const float travelSpeed)
{
    m_StartPoint = startPoint;
    m_SegCount = segCount;
    m_TravelSpeed = travelSpeed;

    m_Segments = new Vector[m_SegCount];

    if (aSegArray)
    {
        for (int i = 0; i < m_SegCount; ++i)
            m_Segments[i] = aSegArray[i];
    }

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a LimbPath to be identical to another, by deep copy.

int LimbPath::Create(const LimbPath &reference)
{
    Entity::Create(reference);

    m_Start = reference.m_Start;
    m_StartSegCount = reference.m_StartSegCount;

    deque<Vector>::const_iterator itr;
    for (itr = reference.m_Segments.begin(); itr != reference.m_Segments.end(); ++itr)
        m_Segments.push_back(*itr);

    if (m_Segments.size() > 0)
        m_CurrentSegment = m_Segments.begin();
    else
        m_CurrentSegment = m_Segments.end();

    m_FootCollisionsDisabledSegment = reference.m_FootCollisionsDisabledSegment;

    m_SegProgress = reference.m_SegProgress;
    for (int i = 0; i < SPEEDCOUNT; ++i)
        m_TravelSpeed[i] = reference.m_TravelSpeed[i];
    m_PushForce = reference.m_PushForce;
    m_TimeLeft = reference.m_TimeLeft;
    m_TotalLength = reference.m_TotalLength;
    m_RegularLength = reference.m_RegularLength;
    m_SegmentDone = reference.m_SegmentDone;
    m_HFlipped = reference.m_HFlipped;

    Terminate();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int LimbPath::ReadProperty(const std::string_view &propName, Reader &reader)
{
    if (propName == "StartOffset")
        reader >> m_Start;
    else if (propName == "StartSegCount")
        reader >> m_StartSegCount;
    else if (propName == "AddSegment")
    {
        Vector segment;
        reader >> segment;
        m_Segments.push_back(segment);
        m_TotalLength += segment.GetMagnitude();
        if (m_Segments.size() >= m_StartSegCount)
            m_RegularLength += segment.GetMagnitude();
    } else if (propName == "EndSegCount") {
        reader >> m_FootCollisionsDisabledSegment;
    }
	else if (propName == "SlowTravelSpeed")
	{
		reader >> m_TravelSpeed[SLOW];
		//m_TravelSpeed[SLOW] = m_TravelSpeed[SLOW] * 2;
	}
	else if (propName == "NormalTravelSpeed")
	{
		reader >> m_TravelSpeed[NORMAL];
		//m_TravelSpeed[NORMAL] = m_TravelSpeed[NORMAL] * 2;
	}
	else if (propName == "FastTravelSpeed")
	{
		reader >> m_TravelSpeed[FAST];
		//m_TravelSpeed[FAST] = m_TravelSpeed[FAST] * 2;
	}
	else if (propName == "PushForce")
	{
		reader >> m_PushForce;
		//m_PushForce = m_PushForce / 1.5;
	}
    else
        return Entity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this LimbPath with a Writer for
//                  later recreation with Create(Reader &reader);

int LimbPath::Save(Writer &writer) const
{
    Entity::Save(writer);

    writer.NewProperty("StartOffset");
    writer << m_Start;
    writer.NewProperty("StartSegCount");
    writer << m_StartSegCount;
    for (deque<Vector>::const_iterator itr = m_Segments.begin(); itr != m_Segments.end(); ++itr)
    {
        writer.NewProperty("AddSegment");
        writer << *itr;
    }
    writer.NewProperty("SlowTravelSpeed");
    writer << m_TravelSpeed[SLOW];
    writer.NewProperty("NormalTravelSpeed");
    writer << m_TravelSpeed[NORMAL];
    writer.NewProperty("FastTravelSpeed");
    writer << m_TravelSpeed[FAST];
    writer.NewProperty("PushForce");
    writer << m_PushForce;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the LimbPath object.

void LimbPath::Destroy(bool notInherited)
{
    
    if (!notInherited)
        Entity::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetProgressPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the APPROXIMATE scene position that the limb was reported to be
//                  last frame.

Vector LimbPath::GetProgressPos()
{
    if (IsStaticPoint())
        return m_JointPos.GetFloored() + m_Start * m_Rotation;

    // Start at the start
    Vector returnVec(m_JointPos.GetFloored() + m_Start * m_Rotation);

    // Add all the segments before the current one
	deque<Vector>::const_iterator itr;
    for (itr = m_Segments.begin(); itr != m_CurrentSegment; ++itr)
        returnVec += (*itr) * m_Rotation;

    // Add any from the progress made on the current one
    if (itr != m_Segments.end())
        returnVec += ((*(m_CurrentSegment)) * m_SegProgress) * m_Rotation;

    return returnVec;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCurrentSegTarget
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the scene/world position target that the current segment represents.

Vector LimbPath::GetCurrentSegTarget()
{
    if (IsStaticPoint())
        return m_JointPos.GetFloored() + m_Start * m_Rotation;

    Vector returnVec(m_JointPos.GetFloored() + m_Start * m_Rotation);
	deque<Vector>::const_iterator itr;

    for (itr = m_Segments.begin(); itr != m_CurrentSegment; ++itr)
        returnVec += (*itr) * m_Rotation;

    if (itr != m_Segments.end())
        returnVec += *(m_CurrentSegment) * m_Rotation;

    return returnVec;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCurrentVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the velocity of the current position on the path.
//                  Note that this should be called BEFORE GetNextVec() if the
//                  appropriate matching velocity is to be returned here. If the limb
//                  doesn't hit the end of a segment before the time chunk runs out,
//                  the returned move vector is limited by the time chunk.

Vector LimbPath::GetCurrentVel(const Vector &limbPos)
{
    Vector returnVel;
    Vector distVect = g_SceneMan.ShortestDistance(limbPos, GetCurrentSegTarget());
	float adjustedTravelSpeed = m_TravelSpeed[m_WhichSpeed] / (1.0F + m_JointVel.GetMagnitude() * 0.1F);

    if (IsStaticPoint())
    {
        returnVel = distVect * c_MPP / 0.020/* + m_JointVel*/;
        returnVel.CapMagnitude(adjustedTravelSpeed);
        returnVel += m_JointVel;

//        if (distVect.GetMagnitude() < 0.5)
//            returnVel *= 0.1;
    }
    else
    {
        returnVel.SetXY(adjustedTravelSpeed, 0);

        if (!distVect.IsZero())
            returnVel.AbsRotateTo(distVect);

        returnVel += m_JointVel;
    }

    return returnVel;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNextTimeChunk
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the time needed to get to the target waypoint of the current
//                  segment at the current speed, if there are no obstacles. The chunk
//                  will not exceed the remaining time left on the frame, and will deduct
//                  itself from the remaining frame time tally (originally set by
//                  SetFrameTime()).

float LimbPath::GetNextTimeChunk(const Vector &limbPos)
{
    float timeChunk;

    if (IsStaticPoint())
    {
        // Use all the time to get to the target point.
        timeChunk = m_TimeLeft;
        m_TimeLeft = 0.0;
    }
    else
    {
        Vector distance;
        // Figure out the distance, in meters, between the limb position and the target.
        distance = g_SceneMan.ShortestDistance(limbPos, GetCurrentSegTarget()) * c_MPP;
        // Add the distance needed to be traveled due to the joint velocity.
//        distance += m_JointVel * m_TimeLeft;

        // Figure out the time needed to get to the target at the current speed, if
        // there are no obstacles.
        timeChunk = distance.GetMagnitude() / (GetSpeed() + m_JointVel.GetMagnitude());
        // Cap the time segment off to what we have left, if needed.
        timeChunk = timeChunk > m_TimeLeft ? m_TimeLeft : timeChunk;
        // Deduct the time used to pushtravel from the total time left.
        m_TimeLeft -= timeChunk;
    }

    return timeChunk;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReportProgress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Used to report how much progress was made to getting the limb close to
//                  the target (the next segment start).

void LimbPath::ReportProgress(const Vector &limbPos)
{
    if (IsStaticPoint())
    {
        m_Ended = g_SceneMan.ShortestDistance(limbPos, GetCurrentSegTarget()).GetMagnitude() < 1.0;
    }
    else
    {
        // Check if we are sufficiently close to the target to start going after the next one.
        Vector distVec = g_SceneMan.ShortestDistance(limbPos, GetCurrentSegTarget());
        float distance = distVec.GetMagnitude();
        float segMag = (*(m_CurrentSegment)).GetMagnitude();
// TODO: Don't hardcode this!")
        if (distance < 1.5)
        {
            if (++(m_CurrentSegment) == m_Segments.end())
            {
                --(m_CurrentSegment);
                // Get normalized progress measure toward the target.
                m_SegProgress = distance > segMag ? 0.0 : (1.0 - (distance / segMag));
                m_Ended = true;
            }
            // Next segment!
            else
            {
                m_SegProgress = 0.0;
                m_SegTimer.Reset();
                m_Ended = false;
            }
        }
        else
        {
            m_SegProgress = distance > segMag ? 0.0 : (1.0 - (distance / segMag));
            m_Ended = false;
        }

        // Make sure we're not stuck on one segment, time that it isn't taking unreasonably long, and restart the path if it seems stuck
        if (!m_Ended && m_SegTimer.IsPastSimMS(((segMag * c_MPP) / GetSpeed()) * 1000 * 2))
//        if (!m_Ended && m_SegTimer.IsPastSimMS(333))
        {
            Terminate();
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalProgress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a value representing the total progress that has been made on
//                  this entire path. If the path has ended, 0.0 is returned.

float LimbPath::GetTotalProgress() const
{
    if (m_Ended || IsStaticPoint())
        return 0.0;

    float prog = 0;
    for (deque<Vector>::const_iterator itr = m_Segments.begin(); itr != m_CurrentSegment; ++itr)
        prog += itr->GetMagnitude();

    prog += (*(m_CurrentSegment)).GetMagnitude() * m_SegProgress;
    return prog / m_TotalLength;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRegularProgress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a value representing the progress that has been made on the
//                  regular part of this path, ie averythign except the starting segments.
//                  If progress has not been made past the starting segments, < 0 will
//                  be returned. If the path has ended, 0.0 is returned.

float LimbPath::GetRegularProgress() const
{
    if (m_Ended || IsStaticPoint())
        return 0.0;

    float prog = m_RegularLength - m_TotalLength;
    for (deque<Vector>::const_iterator itr = m_Segments.begin(); itr != m_CurrentSegment; ++itr)
        prog += itr->GetMagnitude();
    prog += (*(m_CurrentSegment)).GetMagnitude() * m_SegProgress;

    return prog / m_RegularLength;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LimbPath::GetCurrentSegmentNumber() const {
    int progress = 0;
    if (!m_Ended && !IsStaticPoint()) {
        for (deque<Vector>::const_iterator itr = m_Segments.begin(); itr != m_CurrentSegment; ++itr) {
            progress++;
        }
    }
    return progress;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSpeed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the speed that a limb traveling this LimbPath should have to one
//                  of the three predefined speed settings.

void LimbPath::SetSpeed(int newSpeed)
{
    if (newSpeed <= SLOW)
        m_WhichSpeed = SLOW;
    else if (newSpeed >= FAST)
        m_WhichSpeed = FAST;
    else
        m_WhichSpeed = NORMAL;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OverrideSpeed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the speed that a limb traveling this LimbPath with the specified preset should have.

void LimbPath::OverrideSpeed(int speedPreset, float newSpeed)
{
	if (speedPreset == SLOW || speedPreset == FAST || speedPreset == NORMAL)
	{
		m_TravelSpeed[m_WhichSpeed] = newSpeed;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Terminate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this LimbPath's progress to its end.

void LimbPath::Terminate()
{
    if (IsStaticPoint()) {
        m_Ended = true;
    }
    else {
        m_CurrentSegment = --(m_Segments.end());
        m_SegProgress = 1.0;
        m_Ended = true;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Restart
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Restarts the position tracking of the limb that travels along this
//                  LimbPath.

void LimbPath::Restart()
{
    m_CurrentSegment = m_Segments.begin();
    m_PathTimer.Reset();
    m_SegTimer.Reset();
    m_SegProgress = 0;
    m_Ended = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RestartFree
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Restarts the position tracking of the limb that travels along this
//                  LimbPath at a point which does not contain terrain. In doing this,
//                  a list of potential starting segments are checked and the first to
//                  yield a starting position that is not in terrain will be picked.
//                  If none of the candidate starting segments are free of terrain,
//                  the last one in the list will be picked and false will be returned
//                  here. The passed in limbPos Vector will be set to the new position of
//                  the restarted path, if a free spot is found.

bool LimbPath::RestartFree(Vector &limbPos, MOID MOIDToIgnore, int ignoreTeam)
{
    deque<Vector>::iterator prevSeg = m_CurrentSegment;
    float prevProg = m_SegProgress;
    m_SegProgress = 0;
    bool found = false;
    float result = 0;

    g_SceneMan.GetTerrain()->LockBitmaps();
    acquire_bitmap(g_SceneMan.GetMOIDBitmap());
    
    if (IsStaticPoint())
	{
		Vector notUsed;
        Vector targetPos = m_JointPos.GetFloored() + m_Start * m_Rotation;
        Vector beginPos = targetPos;
// TODO: don't hardcode the beginpos
        beginPos.m_Y -= 24;

        result = g_SceneMan.CastObstacleRay(beginPos, targetPos - beginPos, notUsed, limbPos, MOIDToIgnore, ignoreTeam, g_MaterialGrass);

        // Only indicate that we found free position if there were any free pixels encountered
        if (result < 0 || result > 0)
            found = true;
    }
    else
	{
		Vector notUsed;
		
        // Start at the very beginning of the path
        m_CurrentSegment = m_Segments.begin();

        // Find the first start segment that has an obstacle on it
        int i = 0;
        for (; i < m_StartSegCount; ++i)
        {
            result = g_SceneMan.CastObstacleRay(GetProgressPos(), (*m_CurrentSegment) * m_Rotation, notUsed, limbPos, MOIDToIgnore, ignoreTeam, g_MaterialGrass);

            // If we found an obstacle after the first pixel, report the current segment as the starting one and that there is free space here
            if (result > 0)
            {
                // Set accurate segment progress
// TODO: See if this is a good idea, or if we should just set it to 0 and set limbPos to the start of current segment
                m_SegProgress = g_SceneMan.ShortestDistance(GetProgressPos(), limbPos).GetMagnitude() / (*m_CurrentSegment).GetMagnitude();
                limbPos = GetProgressPos();
//                m_SegProgress = 0;
                m_Ended = false;
                found = true;
                break;
            }
            // If obstacle was found on first pixel, report last segment as restarting pos, if there was a last segment
            else if (result == 0 && m_CurrentSegment != m_Segments.begin())
            {
                // Use last segment
                --(m_CurrentSegment);
                limbPos = GetProgressPos();
                m_SegProgress = 0;
                m_Ended = false;
                found = true;
                break;
            }
            // If obstacle was found on the first pixel of the first segment, then just report that we couldn't find any free space
            else if (result == 0 && m_CurrentSegment == m_Segments.begin())
            {
                found = false;
                break;
            }

            // Check next segment, and quit if it's the end
            if (++(m_CurrentSegment) == m_Segments.end())
            {
                found = false;
                break;
            }
        }

        // If we couldn't find any obstacles on the starting segments, then set it to the first non-starting seg and report success
        if (!found && i == m_StartSegCount && m_CurrentSegment != m_Segments.end())
        {
            limbPos = GetProgressPos();
            m_SegProgress = 0;
            m_Ended = false;
            found = true;
        }
    }
    release_bitmap(g_SceneMan.GetMOIDBitmap());
    g_SceneMan.GetTerrain()->UnlockBitmaps();

    if (found)
    {
        m_PathTimer.Reset();
        m_SegTimer.Reset();
        return true;
    }

    // Failed to find free space, so set back to old state
    m_CurrentSegment = prevSeg;
    m_SegProgress = prevProg;
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this LimbPath's current graphical debug representation to a
//                  BITMAP of choice.

void LimbPath::Draw(BITMAP *pTargetBitmap,
                    const Vector &targetPos,
                    unsigned char color) const
{
    acquire_bitmap(pTargetBitmap);
    Vector prevPoint = m_JointPos.GetFloored() + (m_Start * m_Rotation) - targetPos;
    Vector nextPoint = prevPoint;
    for (deque<Vector>::const_iterator itr = m_Segments.begin(); itr != m_Segments.end(); ++itr)
    {
        nextPoint += (*itr) * m_Rotation;
        line(pTargetBitmap, prevPoint.m_X, prevPoint.m_Y, nextPoint.m_X, nextPoint.m_Y, color);
        prevPoint += (*itr) * m_Rotation;
    }

    release_bitmap(pTargetBitmap);
}

} // namespace RTE