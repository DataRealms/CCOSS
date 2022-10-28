#ifndef _RTEGLOBALSCRIPT_
#define _RTEGLOBALSCRIPT_

#include "PieSlice.h"

namespace RTE {

	class ACraft;

	/// <summary>
	/// The supporting object for a lua script that can be turned on and off by the player, and run independent of Activities.
	/// </summary>
	class GlobalScript : public Entity {
		friend struct EntityLuaBindings;

	public:

		EntityAllocation(GlobalScript);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a GlobalScript object in system memory. Create() should be called before using the object.
		/// </summary>
		GlobalScript() { Clear(); }

		/// <summary>
		/// Makes the GlobalScript object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override { return 0; }

		/// <summary>
		/// Creates an GlobalScript to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the GlobalScript to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const GlobalScript &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a GlobalScript object before deletion from system memory.
		/// </summary>
		~GlobalScript() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the GlobalScript object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override { if (!notInherited) { Entity::Destroy(); } Clear(); }

		/// <summary>
		/// Resets the entire GlobalScript, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); Entity::Reset(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets whether or not this GlobalScript is active. Active scripts can be deactivated automatically if it fails to execute it's Update function without errors to avoid filling the console with error messages.
		/// </summary>
		/// <returns>Whether or not this GlobalScript is active.</returns>
		bool IsActive() const { return m_IsActive; }

		/// <summary>
		/// Sets whether or not this GlobalScript should be active.
		/// </summary>
		/// <param name="active">Whether or not this GlobalScript should be active.</param>
		void SetActive(bool active) { m_IsActive = active; }

		/// <summary>
		/// Gets whether or not this GlobalScript should be updated late, i.e. after the standard MovableMan update.
		/// </summary>
		/// <returns>Whether or not this GlobalScript should be updated late.</returns>
		bool ShouldLateUpdate() const { return m_LateUpdate; }

		/// <summary>
		/// Gets the list of PieSlices this GlobalScript adds to any active Actor PieMenus.
		/// </summary>
		/// <returns>The list of PieSilces this GlobalScript adds to any active Actor PieMenus</returns>
		const std::vector<std::unique_ptr<PieSlice>> & GetPieSlicesToAdd() const { return m_PieSlicesToAdd; }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Reloads the Lua script specified by this GlobalScript. This will also update the original preset in the PresetMan with the update scripts so future objects spawned will use the new scripts.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int ReloadScripts() override;

		/// <summary>
		/// Starts this GlobalScript by running the appropriate Lua function.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Start();

		/// <summary>
		/// Pauses and unpauses this GlobalScript by running the appropriate Lua function.
		/// </summary>
		/// <param name="pause">Whether or not this GlobalScript should be paused.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Pause(bool pause = true) const;

		/// <summary>
		/// Ends this GlobalScript by running the appropriate Lua function
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int End() const;

		/// <summary>
		/// Handles when an ACraft has left the game scene and entered orbit by running the appropriate Lua function. Ownership is NOT transferred!
		/// </summary>
		/// <param name="orbitedCraft">The ACraft instance that entered orbit. Ownership is NOT transferred!</param>
		void HandleCraftEnteringOrbit(const ACraft *orbitedCraft) const;

		/// <summary>
		/// Updates the state of this GlobalScript every frame.
		/// </summary>
		void Update();
#pragma endregion

	private:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		std::string m_ScriptPath; //!< The path to the Lua script file that defines this' behaviors in update.
		std::string m_LuaClassName; //!< The name of the class (table) defining the logic of this in Lua, as specified in the script file.
		bool m_IsActive; //!< Whether this GlobalScript is currently allowed to run.
		bool m_LateUpdate; //!< Whether or not this GlobalScript should be updated late, i.e. after the standard MovableMan update.

		std::vector<std::unique_ptr<PieSlice>> m_PieSlicesToAdd; //!< A vector of PieSlices that should be added to any PieMenus opened while this GlobalScript is active.

		/// <summary>
		/// Clears all the member variables of this GlobalScript, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		GlobalScript(const GlobalScript &reference) = delete;
		GlobalScript &operator=(const GlobalScript &rhs) = delete;
	};
}
#endif