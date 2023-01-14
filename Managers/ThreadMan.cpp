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
    
}

RenderableGameState::~RenderableGameState() {}

const std::string ThreadMan::m_ClassName = "ThreadMan";

void ThreadMan::Clear() {
	m_GameStateModifiable.reset();
	m_GameStateDrawable.reset();
	m_NewSimFrame = false;
    m_SimFunctions.clear();
    m_RenderTarget = nullptr;
    m_RenderOffset.Reset();
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
    // TODO_MULTITHREAD: Remove as much of this as possible...
    if (g_ActivityMan.IsInActivity()) {
        m_GameStateModifiable->m_Activity.reset(dynamic_cast<Activity*>(g_ActivityMan.GetActivity()->Clone()));
        m_GameStateModifiable->m_Terrain.reset(dynamic_cast<SLTerrain*>(g_SceneMan.GetScene()->GetTerrain()->Clone()));
    }
    m_GameStateModifiable->m_RenderQueue = m_SimRenderQueue;

    m_SimRenderQueue.clear();

    // TODO_MULTITHREAD: add post processing effects to RenderableGameState
    // Clear the effects list for this frame
    //m_PostScreenEffects.clear();

    // Mark that we have a new sim frame, so we can swap rendered game state at the start of the new render
    g_TimerMan.MarkNewSimUpdateComplete();
    m_NewSimFrame = true;
}

void ThreadMan::QueueInSimulationThread(std::function<void(void)> funcToRun) {
    std::lock_guard<std::mutex> lock(m_SimFunctionMutex);

    // If we're already queued, don't bother
    auto itr = std::find_if(m_SimFunctions.begin(), m_SimFunctions.end(),
        [&funcToRun](std::function<void(void)> &func) {
            return funcToRun.target<void(*)(void)>() == func.target<void(*)(void)>();
        });

    if (itr != m_SimFunctions.end()) {
        return;
    }

    m_SimFunctions.push_back(funcToRun);
}

void ThreadMan::RunSimulationThreadFunctions() {
    std::lock_guard<std::mutex> lock(m_SimFunctionMutex);
    for (auto& func : m_SimFunctions) {
        func();
    }
    m_SimFunctions.clear();
}

void ThreadMan::Destroy() {
    Clear();
}

} // namespace RTE