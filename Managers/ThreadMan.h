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
        std::unique_ptr<SceneLayerTracked> m_pMOColorLayer = nullptr;
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
        /// </summary>
        RenderableGameState& GetModifiableGameState() { return *m_GameStateModifiable; };

        /// <summary>
        /// Get a game state we can safely read from the render thread.
        /// </summary>
        const RenderableGameState& GetDrawableGameState() const { return *m_GameStateDrawable; };

        virtual const std::string & GetClassName() const { return m_ClassName; }

    protected:

        // Member variables
        static const std::string m_ClassName;

    private:

		std::unique_ptr<RenderableGameState> m_GameStateModifiable; //!< Current game state game state that sim can update (owned)
		std::unique_ptr<RenderableGameState> m_GameStateDrawable; //!< Stable game state that we are drawing (owned)
		std::mutex m_GameStateCopyMutex; //!< Mutex to ensure we can't swap our rendering game state while it's being copied to.
		std::atomic<bool> m_NewSimFrame; //!< Whether we have a new sim frame ready to draw.
		//bool m_IsDrawingNewFrame; //!< Whether we are currently drawing a new render frame..

        void Clear();

        // Disallow the use of some implicit methods.
        ThreadMan(const ThreadMan &reference);
        ThreadMan & operator=(const ThreadMan &rhs);

    };

} // namespace RTE

#endif // File
