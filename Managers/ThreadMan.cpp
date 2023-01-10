//////////////////////////////////////////////////////////////////////////////////////////
// File:            ThreadMan.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the ThreadMan class.
// Project:         Retro Terrain Engine
// Author(s):       
//                  
//                  

#include "ThreadMan.h"

#include "Scene.h"
#include "SceneLayer.h"
#include "Vector.h"

#include "ActivityMan.h"
#include "SceneMan.h"
#include "MovableMan.h"
#include "TimerMan.h"

using namespace std;

namespace RTE
{

RenderableGameState::RenderableGameState() {
    BITMAP *pBitmap = create_bitmap_ex(8, g_SceneMan.GetSceneWidth(), g_SceneMan.GetSceneHeight());
    clear_to_color(pBitmap, g_MaskColor);
    m_pMOColorLayer = std::make_unique<SceneLayerTracked>();
    m_pMOColorLayer->Create(pBitmap, true, Vector(), g_SceneMan.SceneWrapsX(), g_SceneMan.SceneWrapsY(), Vector(1.0, 1.0));
}

RenderableGameState::~RenderableGameState() {}

const std::string ThreadMan::m_ClassName = "ThreadMan";

void ThreadMan::Clear() {
	m_GameStateModifiable.reset();
	m_GameStateDrawable.reset();
	m_NewSimFrame = false;
}

int ThreadMan::Initialize() {
    Clear();
    m_GameStateModifiable.reset(new RenderableGameState());
	m_GameStateDrawable.reset(new RenderableGameState());
    
	return 0;
}

void ThreadMan::Update() {
    if (m_NewSimFrame) {
        std::lock_guard<std::mutex> lock(m_GameStateCopyMutex);

        std::swap(m_GameStateDrawable, m_GameStateModifiable);
        m_NewSimFrame = false;
    }
}

void ThreadMan::TransferSimStateToRenderer() {
    std::lock_guard<std::mutex> lock(m_GameStateCopyMutex);

    // Copy game state into our current buffer
    // TODO_MULTITHREAD: Figure out something better/faster...
    // It's especially annoying that the MO draw itself takes place on the simulation thread
    // Perhaps we can cache draw requests and pass them onto draw?
    m_GameStateModifiable->m_Activity.reset(dynamic_cast<Activity*>(g_ActivityMan.GetActivity()->Clone()));
    m_GameStateModifiable->m_Terrain.reset(dynamic_cast<SLTerrain*>(g_SceneMan.GetScene()->GetTerrain()->Clone()));

    // TODO_MULTITHREAD: add post processing effects to RenderableGameState
    // Clear the effects list for this frame
    //m_PostScreenEffects.clear();

    // this loses draws earlier in the update loop, we can do draws, with atom travel
    // And yes, that's pretttttty broken.
    // TODO_MULTIHREAD fix
    g_ThreadMan.GetModifiableGameState().m_pMOColorLayer->ClearBitmap(g_MaskColor);
    g_MovableMan.Draw(m_GameStateModifiable->m_pMOColorLayer->GetBitmap());

    // Mark that we have a new sim frame, so we can swap rendered game state at the start of the new render
    g_TimerMan.FulfillDrawRequest(); // TODO_MULTITHREAD
    m_NewSimFrame = true;
}

void ThreadMan::Destroy() {
    Clear();
}

} // namespace RTE