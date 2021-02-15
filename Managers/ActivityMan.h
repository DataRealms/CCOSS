#ifndef _RTEACTIVITYMAN_
#define _RTEACTIVITYMAN_

#include "Singleton.h"
#include "Activity.h"

#define g_ActivityMan ActivityMan::Instance()

namespace RTE {

	/// <summary>
	/// The singleton manager of the Activities and rules of Cortex Command.
	/// </summary>
	class ActivityMan : public Singleton<ActivityMan> {
		friend class SettingsMan;

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate an ActivityMan object in system memory. Create() should be called before using the object.
		/// </summary>
		ActivityMan() { Clear(); }

		/// <summary>
		/// Makes the ActivityMan object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Initialize() { return 0; }
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up an ActivityMan object before deletion from system memory.
		/// </summary>
		~ActivityMan() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the ActivityMan object.
		/// </summary>
		void Destroy() { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the type name of the default Activity to be loaded if nothing else is available.
		/// </summary>
		/// <returns>The default Activity type name.</returns>
		std::string GetDefaultActivityType() const { return m_DefaultActivityType; }

		/// <summary>
		/// Sets the type name of the default Activity to be loaded if nothing else is available.
		/// </summary>
		/// <param name="defaultActivityType">The default Activity type name.</param>
		void SetDefaultActivityType(std::string defaultActivityType) { m_DefaultActivityType = defaultActivityType; }

		/// <summary>
		/// Gets the name of the default Activity to be loaded if nothing else is available.
		/// </summary>
		/// <returns>The default Activity preset name.</returns>
		std::string GetDefaultActivityName() const { return m_DefaultActivityName; }

		/// <summary>
		/// Sets the preset name of the default Activity to be loaded if nothing else is available.
		/// </summary>
		/// <param name="defaultActivityName">The default Activity preset name.</param>
		void SetDefaultActivityName(std::string defaultActivityName) { m_DefaultActivityName = defaultActivityName; }

		/// <summary>
		/// Gets the Activity that will be used in the next restart. Ownership is NOT transferred!
		/// </summary>
		/// <returns>The Activity to put into effect next time ResetActivity is called.</returns>
		Activity * GetStartActivity() const { return m_StartActivity; }

		/// <summary>
		/// Sets a new Activity to copy for next restart. You have to use RestartActivity to get it going. Ownership IS transferred!
		/// </summary>
		/// <param name="newActivity">The new Activity to put into effect next time ResetActivity is called.</param>
		void SetStartActivity(Activity *newActivity);

		/// <summary>
		/// Gets the current Activity in effect. Won't be what has been set by SetStartActivity unless RestartActivity has been called since.
		/// </summary>
		/// <returns>The current Activity in effect. Will be 0 if no Activity is going.</returns>
		Activity * GetActivity() const { return m_Activity; }

		/// <summary>
		/// Indicates whether the game is currently running or not (not editing, over or paused)
		/// </summary>
		/// <returns>Whether the game is running or not.</returns>
		bool ActivityRunning() const { return m_Activity ? m_Activity->IsRunning() : false; }

		/// <summary>
		/// Indicates whether the game is currently paused or not.
		/// </summary>
		/// <returns>Whether the game is paused or not.</returns>
		bool ActivityPaused() const { return m_Activity ? m_Activity->IsPaused() : true; }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Officially starts the Activity passed in. Ownership IS transferred!
		/// </summary>
		/// <param name="activity">The new activity to start.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.<returns>
		int StartActivity(Activity *activity);

		/// <summary>
		/// Officially gets and starts the Activity described.
		/// </summary>
		/// <param name="className">The class name of the Activity to start.</param>
		/// <param name="presetName">The PresetName of the Activity to start.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int StartActivity(std::string className, std::string presetName);

		/// <summary>
		/// Pauses/unpauses the game and saving/resuming in-game music if possible, or queuing default music if not.
		/// </summary>
		/// <param name="pause">Whether to pause the game or not.</param>
		void PauseActivity(bool pause = true);

		/// <summary>
		/// Completely restarts whatever Activity was last started.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int RestartActivity();

		/// <summary>
		/// Forces the current game's end.
		/// </summary>
		void EndActivity();

		/// <summary>
		/// Only updates Global Scripts of the current activity with LateUpdate flag enabled.
		/// </summary>
		void LateUpdateGlobalScripts();

		/// <summary>
		/// Updates the state of this and the current Activity. Supposed to be done every frame before drawing.
		/// </summary>
		void Update() { if (m_Activity) { m_Activity->Update(); } }
#pragma endregion

	protected:

		std::string m_DefaultActivityType; //!< The type name of the default Activity to be loaded if nothing else is available.
		std::string m_DefaultActivityName; //!< The preset name of the default Activity to be loaded if nothing else is available.
		
		Activity *m_Activity; //!< The current Activity in action. OWNED BY THIS!
		Activity *m_StartActivity; //!< The starting condition of the next Activity to be (re)started. OWNED BY THIS!
			
		std::string m_LastMusicPath; //!< Path to the last music stream being played.
		float m_LastMusicPos; //!< What the last position of the in-game music track was before pause, in seconds.

	private:

		/// <summary>
		/// Clears all the member variables of this ActivityMan, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		ActivityMan(const ActivityMan &reference) = delete;
		ActivityMan & operator=(const ActivityMan &rhs) = delete;
	};
}
#endif