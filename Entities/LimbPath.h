#ifndef _RTELIMBPATH_
#define _RTELIMBPATH_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            LimbPath.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the LimbPath class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Entity.h"
#include "Vector.h"
#include "ActivityMan.h"
#include "Atom.h"

namespace RTE
{

#define SPEEDCOUNT 3

enum Speed
{
    SLOW = 0,
    NORMAL,
    FAST
};


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           LimbPath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A set of Vector:s making up a motion path for a AtomGroup's limb. The
//                  path is continuous.
// Parent(s):       Entity.
// Class history:   05/25/2001 LimbPath created.

class LimbPath:
    public Entity
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EntityAllocation(LimbPath)
SerializableOverrideMethods
ClassInfoGetters

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     LimbPath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a LimbPath object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    LimbPath() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~LimbPath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a LimbPath object before deletion
//                  from system memory.
// Arguments:       None.

	~LimbPath() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the LimbPath object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

   int Create() override;

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the LimbPath object ready for use.
// Arguments:       A Vector specifying starting point of this LimbPath, relative
//                  to the owning RTEActor's origin.
//                  An int specifying how many segments there are in the following
//                  segment array. This MUST match the actual size of the array!
//                  An array of Vectors that hold the desired path segments to use.
//                  A float specifying the constant travel speed the limb traveling this
//                  LimbPath should have, in m/s.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(const Vector &startPoint,
                       const unsigned int segCount = 1,
                       const Vector *aSegArray = new Vector,
                       const float travelSpeed = 1.0);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a LimbPath to be identical to another, by deep copy.
// Arguments:       A reference to the LimbPath to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const LimbPath &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire LimbPath, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() override { Clear(); Entity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the LimbPath object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    void Destroy(bool notInherited = false) override;

    /// <summary>
    /// Gets the coordinates where the limb should start at the start of the LimbPath cycle, relative to the owning AtomGroup's local origin.
    /// </summary>
    /// <returns>A Vector with the start position.</returns>
    const Vector & GetStartOffset() const { return m_Start; }

    /// <summary>
    /// Sets the coordinates where the limb should start at the start of the LimbPath cycle, relative to the owning AtomGroup's local origin.
    /// </summary>
    /// <param name="newStartOffset">A Vector with the new start offset.</param>
    void SetStartOffset(const Vector &newStartOffset) { m_Start = newStartOffset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSegCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of Vector:s the internal array of 'waypoints' or
//                  segments of this LimbPath.
// Arguments:       None.
// Return value:    An int with he count.

    unsigned int GetSegCount() const { return m_Segments.size(); }


    /// <summary>
    /// Gets a pointer to the segment at the given index. Ownership is NOT transferred.
    /// </summary>
    /// <param name="segmentIndex">The index of the segment to get.</param>
    /// <returns>A pointer to the segment at the given index. Ownership is NOT transferred.</returns>
    Vector *GetSegment(int segmentIndex) { if (segmentIndex >= 0 && segmentIndex < m_Segments.size()) { return &m_Segments.at(segmentIndex); } return nullptr;}

    /// <summary>
    /// Gets whether or not foot collisions should be disabled, i.e. the limbpath's progress is greater than the FootCollisionsDisabledSegment value.
    /// </summary>
    /// <returns>Whether or not foot collisions should be disabled for this limbpath at its current progress.</returns>
    bool FootCollisionsShouldBeDisabled() const { return m_FootCollisionsDisabledSegment >= 0 && GetSegCount() - GetCurrentSegmentNumber() <= m_FootCollisionsDisabledSegment; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSegProgress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets how far the limb was last reported to be away form the current
//                  segment target/waypoint.
// Arguments:       None.
// Return value:    A normalized float describing the progress made toward the current
//                  segment last frame. 0.5 means it was half the length of the current
//                  segment away from it.

    float GetSegProgress() const { return m_SegProgress; }


    
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetProgressPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the APPROXIMATE scene position that the limb was reported to be
//                  last frame. This really shouldn't be used by external clients.
// Arguments:       None.
// Return value:    A Vector with the APPROXIAMTE scene/world coordinates of the limb as
//                  reported last.

    Vector GetProgressPos();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCurrentSegTarget
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the scene/world position target that the current segment represents.
// Arguments:       None.
// Return value:    A vector with the scene position of the current segment target.

    Vector GetCurrentSegTarget();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCurrentVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the velocity of the current position on the path.
// Arguments:       The current world coordinate position of the Limb.
// Return value:    A Vector with the current move velocity.

    Vector GetCurrentVel(const Vector &limbPos);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSpeed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the speed that a limb traveling this LimbPath should have.
// Arguments:       None.
// Return value:    A float describing the speed in m/s.

    float GetSpeed() const { return m_TravelSpeed[m_WhichSpeed]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSpeed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the speed that a limb traveling this LimbPath should have for the specified preset.
// Arguments:       Predefined speed preset to set the value for.
// Return value:    A float describing the speed in m/s.

	float GetSpeed(int speedPreset) const { if (speedPreset == SLOW || speedPreset == NORMAL || speedPreset == FAST) return m_TravelSpeed[speedPreset]; else return 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPushForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the force that a limb traveling this LimbPath can push against
//                  stuff in the scene with. It will increase to the double if progress
//                  isn't made on the segment.
// Arguments:       None.
// Return value:    The currently set force maximum, in kg * m/s^2.

    float GetPushForce() const { return m_PushForce + (m_PushForce * (m_SegTimer.GetElapsedSimTimeMS() / 500)); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPushForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets thedefault, unaltered force that a limb traveling this LimbPath can push against
//                  stuff in the scene with.
// Arguments:       None.
// Return value:    The default set force maximum, in kg * m/s^2.

	float GetDefaultPushForce() const { return m_PushForce; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNextTimeChunk
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the time needed to get to the target waypoint of the current
//                  segment at the current speed, if there are no obstacles. The chunk
//                  will not exceed the remaining time left on the frame, and will deduct
//                  itself from the remaining frame time tally (originally set by
//                  SetFrameTime()).
// Arguments:       The current world coordinate position of the Limb.
// Return value:    A float describing the time chunk in seconds.

    float GetNextTimeChunk(const Vector &limbPos);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalPathTime
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total time that this entire path should take to travel along
//                  with the current speed setting, including the start segments.
// Arguments:       None.
// Return value:    The total time (ms) this should take to travel along, if unobstructed.

    float GetTotalPathTime() const { return ((m_TotalLength * c_MPP) / m_TravelSpeed[m_WhichSpeed]) * 1000; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRegularPathTime
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total time that this path should take to travel along
//                  with the current speed setting, NOT including the start segments.
// Arguments:       None.
// Return value:    The total time (ms) this should take to travel along, if unobstructed.

    float GetRegularPathTime() const { return ((m_RegularLength * c_MPP) / m_TravelSpeed[m_WhichSpeed]) * 1000; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalTimeProgress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ratio of time since the path was restarted and the total time
//                  it should take to travel along the path with the current speed setting,
//                  including the start segments.
// Arguments:       None.
// Return value:    A positive scalar ratio showing the progress. 0 - 1.0 and beyond.
//                  If the path has ended, but not been reset, 0 is returned.

    float GetTotalTimeProgress() const { return m_Ended ? 0 : (m_PathTimer.GetElapsedSimTimeMS() / GetTotalPathTime()); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRegularTimeProgress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ratio of time since the path was restarted and the total time
//                  it should take to travel along the path with the current speed setting,
//                  NOT including the start segments.
// Arguments:       None.
// Return value:    A positive scalar ratio showing the progress. 0 - 1.0 and beyond.
//                  If the path has ended, but not been reset, 0 is returned.

    float GetRegularTimeProgress() const { return m_Ended ? 0 : (m_PathTimer.GetElapsedSimTimeMS() / GetRegularPathTime()); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReportProgress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Used to report how much progress was made to getting the limb close to
//                  the target (the current segment waypoint).
// Arguments:       The new limb position in world coords.
// Return value:    None.

    void ReportProgress(const Vector &limbPos);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalProgress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a value representing the total progress that has been made on
//                  this entire path. If the path has ended, 0.0 is returned.
// Arguments:       None.
// Return value:    A float indicating the total progress made on the entire path, from
//                  0.0 to 1.0. If the path has ended, 0.0 is returned.

    float GetTotalProgress()  const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRegularProgress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a value representing the progress that has been made on the
//                  regular part of this path, ie averythign except the starting segments.
//                  If progress has not been made past the starting segments, < 0 will
//                  be returned. If the path has ended, 0.0 is returned.
// Arguments:       None.
// Return value:    A float indicating the total progress made on the regular path, from
//                  0.0 to 1.0. If the path has ended, 0.0 is returned.

    float GetRegularProgress() const;

    /// <summary>
    /// Gets the current segment as a number, rather than an iterator.
    /// </summary>
    /// <returns>The current segment as a number.</returns>
    int GetCurrentSegmentNumber() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSegments
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a new array of 'waypoints' or segments of this LimbPath.
// Arguments:       An int specifying how many segments there are in the following
//                  segment array. This MUST match the actual size of the array!
//                  A pointer to the new Vector array.
// Return value:    None.

//    void SetSegments(const unsigned int segCount, const Vector *newSegments);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetCurrentSeg
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current seg pointer to whichever segment in the segment deque.
// Arguments:       An int that is an index to a valid element of the internal segment array.
// Return value:    None.

//    void SetCurrentSeg(unsigned int currentSeg) { m_CurrentSegment = currentSeg; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSpeed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the speed that a limb traveling this LimbPath should have to one
//                  of the three predefined speed settings.
// Arguments:       An int specifying which discrete speed setting to use from the Speed
//                  enumeration.
// Return value:    None.

    void SetSpeed(int newSpeed);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OverrideSpeed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the speed that a limb traveling this LimbPath with the specified preset should have.
// Arguments:       An int specifying which discrete speed setting to use from the Speed
//                  enumeration. New limb travel speed value.
// Return value:    None.

	void OverrideSpeed(int speedPreset, float newSpeed);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OverridePushForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the force that a limb traveling this LimbPath can push against
//                  stuff in the scene with.
// Arguments:       The new push force maximum, in kg * m/s^2.
// Return value:    None.

	void OverridePushForce(float newForce) { m_PushForce = newForce; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetFrameTime
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the amount of time that will be used by the limb to travel every
//                  frame. Defined in seconds.
// Arguments:       A float describing the time in s.
// Return value:    None.

    void SetFrameTime(float newFrameTime) { m_TimeLeft = newFrameTime; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetHFlip
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this path is flipped horizontally or not. If being
//                  flipped the path automatically restarts.
// Arguments:       A bool telling this path to be flipped or not.
// Return value:    None.

    void SetHFlip(bool hflipped)
    {
        m_HFlipped = hflipped;
/*
        if (m_HFlipped != hflipped) {
            // Gotta flip before restart.
            m_HFlipped = hflipped;
            Restart();
        }
        else
            m_HFlipped = hflipped;
*/
        m_Rotation.SetXFlipped(m_HFlipped);
    }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetJointPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Informs this LimbPath of the absolute world coordinates of its owning
//                  Actor's limb's joint for this frame. Needs to be done before
//                  travelling anyhting along this path each frame.
// Arguments:       A Vector with the updated joint position info.
// Return value:    None.

    void SetJointPos(const Vector &jointPos) { m_JointPos = jointPos; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetJointVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Informs this LimbPath of the current velocity  of its owning Actor's
//                  limb's joint for this frame. Needs to be done before travelling
//                  anyhting along this path each frame.
// Arguments:       A Vector with the updated joint velocity info.
// Return value:    None.

    void SetJointVel(const Vector &jointVel) { m_JointVel = jointVel; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetRotation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Informs this LimbPath of the current rotation of its owning Actor's
//                  for this frame. Needs to be done before travelling
//                  anyhting along this path each frame.
// Arguments:       A Matrix with the updated rotation info.
// Return value:    None.

    void SetRotation(const Matrix &rotation) { m_Rotation = rotation; m_Rotation.SetXFlipped(m_HFlipped); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FrameDone
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns if GetNextMoveVec() have to be called again or not on this
//                  frame. If the last call didn't use up all the time moving on the
//                  current segment because it ended, this will return false. Then
//                  GetNextMoveVec() needs to be called at least one more time this frame.
// Arguments:       None.
// Return value:    A bool with the answer.

    bool FrameDone() const { return m_TimeLeft <= 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PathEnded
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the last call to ProgressMade() completed the
//                  entire path. Use Restart() to start the path over.
// Arguments:       None.
// Return value:    A bool with the answer.

    bool PathEnded() const { return m_Ended; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PathIsAtStart
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the path has been restarted without making any
//                  progress yet.
// Arguments:       None.
// Return value:    A bool with the answer.

    bool PathIsAtStart() const { return m_CurrentSegment == m_Segments.begin() && m_SegProgress == 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Terminate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this LimbPath's progress to its end.
// Arguments:       None.
// Return value:    None.

    void Terminate();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Restart
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Restarts the position tracking of the limb that travels along this
//                  LimbPath.
// Arguments:       None.
// Return value:    None.

    void Restart();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RestartFree
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Restarts the position tracking of the limb that travels along this
//                  LimbPath at a point which does not contain terrain. In doing this,
//                  a list of potential starting segments are checked and the first to
//                  yield a starting position that is not in terrain will be picked.
//                  If none of the candidate starting segments are free of terrain,
//                  the last one in the list will be picked and false will be returned
//                  here. The passed in limbPos Vector will be set to teh new position of
//                  the restarted path, if a free spot is found.
// Arguments:       Limb scene pos which will be set to the new reset position if a free
//                  spot was found.
//                  The root MOID to ignore when looking for a free position.
//                  To enable ignoring of all MOIDs associated with an object of a specific
//                  team which also has team ignoring enabled itself.
// Return value:    Whether a starting segment that yielded a starting pos free of terrain
//                  was found or not.

    bool RestartFree(Vector &limbPos, MOID MOIDToIgnore = g_NoMOID, int ignoreTeam = Activity::NoTeam);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsInitialized
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicated whether this path is has been created and had some data set
//                  yet.
// Arguments:       None.
// Return value:    Whether this has been Create:ed yet.

    bool IsInitialized() const { return !m_Start.IsZero() || !m_Segments.empty(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsStaticPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicated whether this path is in fact just a single point to where
//                  the limb will always be ordered to move toward. IE a standing still
//                  type limb movement.
// Arguments:       None.
// Return value:    None.

    bool IsStaticPoint() const { return m_Segments.empty(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this LimbPath's current graphical debug representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  The color to draw the path's pixels as.
// Return value:    None.

	void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), unsigned char color = 34) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    static Entity::ClassInfo m_sClass;

    // The starting point of the path.
    Vector m_Start;

    // The number of starting segments, counting into the path from its beginning,
    // that upon restart of this path will be tried in reverse order till one which
    // yields a starting position that is clear of terrain is found.
    int m_StartSegCount;

    // Array containing the actual 'waypoints' or segments for the path.
    std::deque<Vector> m_Segments;

    // The iterator to the segment of the path that the limb ended up on the end of
	std::deque<Vector>::iterator m_CurrentSegment;

    int m_FootCollisionsDisabledSegment; //!< The segment after which foot collisions will be disabled for this limbpath, if it's for legs.

    // Normalized measure of how far the limb has progressed toward the
    // current segment's target. 0.0 means its farther away than the
    // magnitude of the entire segment. 0.5 means it's half the mag of the segment
    // away from the target.
    float m_SegProgress;

    // The constant speed that the limb traveling this path has in m/s.
    float m_TravelSpeed[SPEEDCOUNT];
    // The current speed setting.
    int m_WhichSpeed;

    // The max force that a limb travelling along this path can push.
    // In kg * m/(s^2)
    float m_PushForce;

    // The latest known position of the owning actor's joint in world coordinates.
    Vector m_JointPos;
    // The latest known velocity of the owning actor's joint in world coordinates.
    Vector m_JointVel;
    // The latest known rotation of the owning actor in world coordinates.
    Matrix m_Rotation;

    // If GetNextTimeSeg() couldn't use up all frame time because the current segment
    // ended,this var stores the remainder of time that should be used to progress
    // on the next segment during the same frame.
    float m_TimeLeft;

    // Times the amount of sim time spent since the last path traversal was started
    Timer m_PathTimer;
    // Times the amount of sim time spent pursuing the current segment's target.
    Timer m_SegTimer;

    // Total length of this LimbPath, including the alternative starting segments, in pixel units
    float m_TotalLength;
    // Length of this LimbPath, excluding the alternative starting segments.
    float m_RegularLength;
    bool m_SegmentDone;
    bool m_Ended;
    bool m_HFlipped;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this LimbPath, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

};

} // namespace RTE

#endif // File