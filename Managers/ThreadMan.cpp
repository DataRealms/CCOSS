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

#include "ActivityMan.h"
#include "SceneMan.h"
#include "MovableMan.h"

using namespace std;

namespace RTE
{

const std::string ThreadMan::m_ClassName = "ThreadMan";

void ThreadMan::Clear() {
	m_GameState.reset();
	m_GameStateBack.reset();
	m_NewSimFrame = false;
}

int ThreadMan::Initialize() {
    m_GameState.reset(new RenderableGameState());
	m_GameStateBack.reset(new RenderableGameState());
    
	return 0;
}

void ThreadMan::Update() {
    if (m_NewSimFrame) {
        std::lock_guard<std::mutex> lock(m_GameStateCopyMutex);
        std::swap(m_GameState, m_GameStateBack);
        g_SceneMan.SwapMOColorBitmap();
        m_NewSimFrame = false;
    }
}

void ThreadMan::NewSimFrameToDraw()
{
    std::lock_guard<std::mutex> lock(m_GameStateCopyMutex);

    // Copy game state into our current buffer
    // TODO_MULTITHREAD: Figure out something better/faster...
    // It's especially annoying that the MO draw itself takes place on the simulation thread
    // Perhaps we can cache draw requests and pass them onto draw?
    m_GameState->m_Activity.reset(dynamic_cast<Activity*>(g_ActivityMan.GetActivity()->Clone()));
    m_GameState->m_Terrain.reset(dynamic_cast<SLTerrain*>(g_SceneMan.GetScene()->GetTerrain()->Clone()));

    // TODO_MULTITHREAD: add post processing effects to RenderableGameState
    // Clear the effects list for this frame
    //m_PostScreenEffects.clear();

    g_SceneMan.ClearMOColorLayer();
    g_MovableMan.Draw(g_SceneMan.GetMOColorBitmap());

    // Mark that we have a new sim frame, so we can swap rendered game state at the start of the new render
    m_NewSimFrame = true;
}

void ThreadMan::Destroy() {
    Clear();
}

} // namespace RTE