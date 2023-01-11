#ifndef _RTEThreadMan_
#define _RTEThreadMan_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            ThreadMan.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the ThreadMan class.
// Project:         Retro Terrain Engine
// Author(s):       
//                  
//                  

#include "Singleton.h"

#include "Activity.h"
#include "SLTerrain.h"

#include <atomic>

#define g_ThreadMan ThreadMan::Instance()

#include "BS_thread_pool.hpp"

namespace RTE
{
    struct RenderableGameState {
        RenderableGameState();
        ~RenderableGameState();

        std::unique_ptr<SLTerrain> m_Terrain = nullptr;
        std::unique_ptr<Activity> m_Activity = std::make_unique<Activity>();
    };

    /// <summary>
    /// The manager for any long-running background threads (i.e the simulation thread), and managing communication between threads.
    /// </summary>
    class ThreadMan: public Singleton<ThreadMan> {
    public:

        ThreadMan() { Clear(); }
        virtual ~ThreadMan() { Destroy(); }

        virtual int Initialize();

        void Destroy();

        /// <summary>
		/// Updates the state of this ThreadMan. Supposed to be done every render frame.
		/// </summary>
		void Update();

        /// <summary>
        /// Notifies us that the next sim update will be drawn, and transfers the 
        /// required information from the current simulation update to a drawable format.
        /// </summary>
        void TransferSimStateToRenderer();

        /// <summary>
        /// Gets a game state we can safely modify from the simulation thread.
        /// TODO_MULTITHREAD this isn't actually safe to modify unless we're in TransferSimStateToRenderer.
        /// Either remove this (ideally some sort of draw queue to this object, and remove all draws from sim)
        /// Or find a way to synchronize this.
        /// </summary>
        RenderableGameState& GetModifiableGameState() { return *m_GameStateModifiable; };

        /// <summary>
        /// Get a game state we can safely read from the render thread.
        /// </summary>
        const RenderableGameState& GetDrawableGameState() const { return *m_GameStateDrawable; };

        /// <summary>
        /// Queue a function that will be ran in the sim thread
        /// </summary>
        void QueueInSimulationThread(std::function<void(void)> funcToRun);

        /// <summary>
        /// Run all queued functions for the sim thread
        /// </summary>
        void RunSimulationThreadFunctions();

        /// <summary>
        /// Get the mutex that is taken while MO lists are being manipulated by MovableMan
        /// </summary>
        std::mutex& GetMODeletedMutex() { return m_MODeletedMutex; }

        virtual const std::string & GetClassName() const { return m_ClassName; }

    protected:

        // Member variables
        static const std::string m_ClassName;

    private:

		std::unique_ptr<RenderableGameState> m_GameStateModifiable; //!< Current game state game state that sim can update (owned)
		std::unique_ptr<RenderableGameState> m_GameStateDrawable; //!< Stable game state that we are drawing (owned)
		std::mutex m_GameStateCopyMutex; //!< Mutex to ensure we can't swap our rendering game state while it's being copied to.
		std::mutex m_MODeletedMutex; //!< Mutex to ensure MO lists aren't being modified while we loop through them.
		std::atomic<bool> m_NewSimFrame; //!< Whether we have a new sim frame ready to draw.
		//bool m_IsDrawingNewFrame; //!< Whether we are currently drawing a new render frame..

        std::vector<std::function<void(void)>> m_SimFunctions;
		std::mutex m_SimFunctionMutex; //!< Mutex to ensure that functions are added to the sim thread safely

        void Clear();

        // Disallow the use of some implicit methods.
        ThreadMan(const ThreadMan &reference);
        ThreadMan & operator=(const ThreadMan &rhs);

    };

} // namespace RTE

#endif // File
