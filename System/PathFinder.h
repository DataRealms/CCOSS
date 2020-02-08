#ifndef _RTEPATHFINDER_
#define _RTEPATHFINDER_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            PathFinder.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the PathFinder class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Vector.h"
#include "Box.h"
#include "SceneMan.h"
#include "System/MicroPather/micropather.h"

using namespace micropather;

namespace RTE
{

class Scene;


//////////////////////////////////////////////////////////////////////////////////////////
// Struct:          PathNode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Contains everything related to a node on the path grid used by PathFinder.
// Parent(s):       None.
// Class history:   09/23/2007 PathNode created.

struct PathNode
{
    // Absolute position of the center of this node in the scene
    Vector m_Pos;
    // Whether this has been updated since last call to Reset the pather
    bool m_IsChanged;
    // Pointers to all adjacent nodes. These are not owned, and may be 0 if adjacent to non-wrapping scene border
    PathNode *m_pUp;
    PathNode *m_pRight;
    PathNode *m_pDown;
    PathNode *m_pLeft;
    PathNode *m_pUpRight;
    PathNode *m_pRightDown;
    PathNode *m_pDownLeft;
    PathNode *m_pLeftUp;
    // Costs to get to each of the adjacent nodes
    float m_UpCost;
    float m_RightCost;
    float m_DownCost;
    float m_LeftCost;
    float m_UpRightCost;
    float m_RightDownCost;
    float m_DownLeftCost;
    float m_LeftUpCost;

    PathNode(Vector pos) { m_Pos = pos;
                           m_pUp = m_pRight = m_pDown = m_pLeft = m_pUpRight = m_pRightDown = m_pDownLeft = m_pLeftUp = 0;
                           // Costs are infinite unless recalculated as otherwise
                           m_UpCost = m_RightCost = m_DownCost = m_LeftCost = m_UpRightCost = m_RightDownCost = m_DownLeftCost = m_LeftUpCost = FLT_MAX; }
};


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           PathFinder
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A class encapsulating and implementing the MicroPather A* pathfinding
//                  library.
// Parent(s):       Graph, a MicroPather pure abstract class.
// Class history:   09/23/2007 PathFinder created.

class PathFinder:
    public Graph
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     PathFinder
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a PathFinder object.
// Arguments:       The scene to be pathing within.
//                  The width and height in scene pixels that of each node should represent.
//                  The block size that the node cache is allocated from. Should be about
//                  a fourth of the total number of nodes.

    PathFinder(Scene *pScene, int nodeDimension = 20, unsigned int allocate = 2000) { Clear(); Create(pScene, nodeDimension, allocate); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~PathFinder
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a PathFinder object before deletion.
// Arguments:       None.

    virtual ~PathFinder() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the PathFinder object ready for use.
// Arguments:       The scene to be pathing within.
//                  The width and height in scene pixels that of each node should represent.
//                  The block size that the node cache is allocated from. Should be about
//                  a fourth of the total number of nodes.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(Scene *pScene, int nodeDimension = 20, unsigned int allocate = 2000);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets RGB of this PathFinder to zero.
// Arguments:       None.
// Return value:    None.

    void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) this PathFinder object.
// Arguments:       None.
// Return value:    None.

    virtual void Destroy();

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetR
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the red value of this PathFinder.
// Arguments:       A unsigned char value that the R value will be set to, between 0 and 255.0
// Return value:    None.

    void SetR(unsigned char newR) { m_R = newR; m_Index = 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetIndex
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the entry in the current color palette that most closely matches
//                  this PathFinder's RGB values.
// Arguments:       None.
// Return value:    The color entry index number.

    unsigned char GetIndex() const { return m_Index; }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RecalculateAllCosts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Recalculates all the costs between all the nodes by tracing lines
//                  in the material layer and summing all the material strengths for each
//                  encountered pixel. Also resets the pather itself.
// Arguments:       None.
// Return value:    None.

    void RecalculateAllCosts();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RecalculateAreaCosts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Recalculates the costs between all the nodes touching a list of specific
//                  rectangular areas (which will be wrapped). Also resets the pather itself.
// Arguments:       The list of Box:es representing the updated areas.
// Return value:    None.

    void RecalculateAreaCosts(const std::list<Box> &boxList);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculatePath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates and returns the least difficult path between two points on
//                  the current scene.
// Arguments:       Start and end positions on the scene to find the path between.
//                  A list which will be filled out with waypoints between the start and end.
//                  The total minimum difficulty cost calculated between the two points on
//                  the scene.
// Return value:    Success or failure, expressed as SOLVED, NO_SOLUTION, or START_END_SAME.

    int CalculatePath(Vector start, Vector end, std::list<Vector> &pathResult, float &totalCostResult, float digStrength = 1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LeastCostEstimate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation of the abstract interface of Graph. Gets the least
//                  possible cost to get from node A to B, if it all was air.
// Arguments:       Pointer to node to start from. OINT.
//                  Node to end up at. OINT.
// Return value:    The cost of the absolutely fastest possible way between the two points,
//                  as if traveled through air all the way.
//                  

    virtual float LeastCostEstimate(void *pStartState, void *pEndState);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AdjacentCost
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation of the abstract interface of Graph. Gets the cost to go
//                  to any adjacent node of the one passed in.
// Arguments:       Pointer to node to get to cost of all adjacents for. OINT.
//                  An empty vector which will be filled out with all the valid nodes adjacent
//                  to the one passed in. If at non-wrapping edge of seam, those non existent
//                  nodes won't be added.
// Return value:    None.

    virtual void AdjacentCost(void *pState, std::vector<micropather::StateCost> *pAdjacentList);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PrintStateInfo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation of the abstract interface of Graph. This function is
//                  only used in DEBUG mode - it dumps output to stdout. Since void* aren't
//                  really human readable, this will print out some consice info without
//                  an ending newline.
// Arguments:       The state to print out info about.
// Return value:    None.

    virtual void PrintStateInfo(void *pState);


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CostAlongLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Helper function for calculating the real actual cost of giong in a
//                  straight line between any two points on the scene. It takes into account
//                  distance traveled, as well as the strength of the materials the line has
//                  to pass through. UPDATE: newer version also goes through parallel lines
//                  offset to each side from the main one.
// Arguments:       The two points to go between.
// Return value:    The cost value.

    //float CostAlongLine(const Vector &start, const Vector &end) { float matCost = g_SceneMan.CastStrengthSumRay(start, end, 0, g_MaterialDoor); return g_SceneMan.ShortestDistance(start, end).GetMagnitude() + (matCost * matCost * matCost * matCost); }
    float CostAlongLine(const Vector &start, const Vector &end) { return g_SceneMan.CastMaxStrengthRay(start, end, 0); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateNodeCosts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Helper function for updating all the values of cost edges going out from
//                  a specific node. This does NOT update the pather, which is required
//                  before solving more paths after calling this.
// Arguments:       The node to update all costs of. OINT. It's safe to pass 0 here.
// Return value:    None.

    void UpdateNodeCosts(PathNode *pNode);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateNodeCostsInBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Helper function for updating all the values of cost edges crossed by
//                  a specific box. This does NOT update the pather, which is required
//                  before solving more paths after calling this. Also it does NOT wrap the
//                  box coming in here, only truncates it!
// Arguments:       The Box of which all edges it touches should be recalculated.
// Return value:    None.

    void UpdateNodeCostsInBox(Box &box);


    // The array of PathNodes representing the grid on the scene. The nodes are owned by this
    std::vector<std::vector<PathNode *> > m_NodeGrid;
    // The width and height of each node, in pixels on the scene
    int m_NodeDimension;
    // What material strength the search is capable of digging trough.
    float m_DigStrenght;
    // The actual pathing object that does the pathfinding work. Owned.
    MicroPather *m_pPather;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this PathFinder, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


};

} // namespace RTE

#endif // File