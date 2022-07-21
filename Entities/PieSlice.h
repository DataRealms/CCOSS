#ifndef _RTEPIESLICE_
#define _RTEPIESLICE_

#include "Constants.h"

#include "Icon.h"

namespace RTE {

	class PieMenu;
	class Actor;

	/// <summary>
	/// An individual PieSlice in a PieMenu.
	/// </summary>
	class PieSlice : public Entity {

	public:

		EntityAllocation(PieSlice);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// <summary>
		/// Enumeration for the types of PieSlices.
		/// </summary>
		enum Type {
			NoType = 0,
			// Inventory management
			Pickup,
			Drop,
			NextItem,
			PreviousItem,
			Reload,
			// Menu and GUI activations
			BuyMenu,
			FullInventory,
			Stats,
			Map,
			Ceasefire,
			// Squad
			FormSquad,
			// AI Modes
			AIModes,
			Sentry,
			Patrol,
			BrainHunt,
			GoldDig,
			GoTo,
			Return,
			Stay,
			Deliver,
			Scuttle,
			// Editor stuff
			Done,
			Load,
			SaveType,
			New,
			Pick,
			Move,
			Remove,
			InFront,
			Behind,
			ZoomIn,
			ZoomOut,
			Team1,
			Team2,
			Team3,
			Team4
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a PieSlice object in system memory. Create() should be called before using the object.
		/// </summary>
		PieSlice() { Clear(); }

		/// <summary>
		/// Makes the PieSlice object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override;

		/// <summary>
		/// Creates a PieSlice to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the PieSlice to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const PieSlice &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a PieSlice object before deletion from system memory.
		/// </summary>
		~PieSlice() override { Destroy(true); }

		/// <summary>
		/// Resets the entire Serializable, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the Type of this PieSlice.
		/// </summary>
		/// <returns>The Type of this PieSlice.</returns>
		Type GetType() const { return m_Type; }

		/// <summary>
		/// Sets the Type of this PieSlice.
		/// </summary>
		/// <param name="newType">The new Type of this PieSlice.</param>
		void SetType(Type newType) { m_Type = newType; }

		/// <summary>
		/// Gets the Direction of this PieSlice.
		/// </summary>
		/// <returns>The Direction of this PieSlice.</returns>
		Directions GetDirection() const { return m_Direction; }

		/// <summary>
		/// Sets the Direction of this PieSlice.
		/// </summary>
		/// <param name="newDirection">The new Direction of this PieSlice.</param>
		void SetDirection(Directions newDirection) { if (newDirection != Directions::None) { m_Direction = newDirection; } }

		/// <summary>
		/// Gets whether or not this PieSlice can be the middle PieSlice of a PieQuadrant.
		/// </summary>
		/// <returns>Whether or not this PieSlice can be the middle PieSlice of a PieQuadrant.</returns>
		bool GetCanBeMiddleSlice() const { return m_CanBeMiddleSlice; }

		/// <summary>
		/// Sets whether or not this PieSlice can be the middle PieSlice of a PieQuadrant.
		/// </summary>
		/// <param name="newCanBeMiddleSlice">Whether or not this PieSlice can be the middle PieSlice of a PieQuadrant.</param>
		void SetCanBeMiddleSlice(bool newCanBeMiddleSlice) { m_CanBeMiddleSlice = newCanBeMiddleSlice; }

		/// <summary>
		/// Gets the original Entity source of this PieSlice, if there is one.
		/// </summary>
		/// <returns>A pointer to the original Entity source of this PieSlice, if there is one.</returns>
		const Entity * GetOriginalSource() const { return m_OriginalSource; }

		/// <summary>
		/// Sets the original Entity source of this PieSlice.
		/// </summary>
		/// <param name="originalSource">A pointer to the original Entity source of this PieSlice.</param>
		void SetOriginalSource(const Entity *originalSource) { m_OriginalSource = originalSource; }

		/// <summary>
		/// Gets whether or not this PieSlice is enabled.
		/// </summary>
		/// <returns>Whether or not this PieSlice is enabled.</returns>
		bool IsEnabled() const { return m_Enabled; }

		/// <summary>
		/// Sets whether or not this PieSlice should be enabled.
		/// </summary>
		/// <param name="enabled">Whether or not this PieSlice should be enabled.</param>
		void SetEnabled(bool enabled) { m_Enabled = enabled; }

		/// <summary>
		/// Gets whether or not this PieSlice has a valid Icon.
		/// </summary>
		/// <returns>Whether or not this PieSlice has a valid Icon.</returns>
		bool HasIcon() const { return m_Icon && m_Icon->GetFrameCount() > 0; }

		/// <summary>
		/// Gets the appropriate icon frame for this PieSlice.
		/// </summary>
		/// <param name="sliceIsSelected">Whether or not this PieSlice is selected, which may affect which icon is appropriate.</param>
		/// <returns>The icon for this PieSlice.</returns>
		BITMAP * GetAppropriateIcon(bool sliceIsSelected = false) const;

		/// <summary>
		/// Sets the new Icon for this PieSlice. Ownership IS transferred.
		/// </summary>
		/// <param name="newIcon">The new Icon for this PieSlice.</param>
		void SetIcon(Icon *newIcon) { m_Icon = std::unique_ptr<Icon>(newIcon); }

		/// <summary>
		/// Gets the file path of the Lua file this PieSlice should run when activated, if any.
		/// </summary>
		/// <returns>The file path to the script file this PieSlice should load when activated.</returns>
		const std::string & GetScriptPath() const { return m_ScriptPath; }

		/// <summary>
		/// Sets the file path of the scripted file this PieSlice should run when activated.
		/// </summary>
		/// <param name="newScriptPath">The file path of the Lua file this PieSlice should run when activated.</param>
		void SetScriptPath(const std::string &newScriptPath) { m_ScriptPath = newScriptPath; }

		/// <summary>
		/// Gets the name of the Lua function to run when this PieSlice is activated.
		/// </summary>
		/// <returns>The name of the Lua function this PieSlice should execute when activated.</returns>
		const std::string & GetFunctionName() const { return m_FunctionName; }

		/// <summary>
		/// Sets the name of the Lua function to run when this PieSlice is activated as a scripted pie menu option.
		/// </summary>
		/// <param name="newFunctionName">The name of the Lua function to run when this PieSlice is activated.</param>
		void SetFunctionName(const std::string &newFunctionName) { m_FunctionName = newFunctionName; }

		//TODO Ideally this would be done with a weak_ptr but I'm not sure how it'll go with LuaMan. Try it out and see
		/// <summary>
		/// Gets the sub-PieMenu for this PieSlice if there is one. Ownership is NOT transferred.
		/// </summary>
		/// <returns>The sub-PieMenu for this PieSlice if there is one. Ownership is NOT transferred.</returns>
		PieMenu * GetSubPieMenu() const;

		/// <summary>
		/// Sets the sub-PieMenu for this PieSlice. Ownership IS transferred.
		/// </summary>
		/// <param name="newSubPieMenu">The new sub-PieMenu for this PieSlice. Ownership IS transferred.</param>
		void SetSubPieMenu(PieMenu *newSubPieMenu);
#pragma endregion

#pragma region Angle Getter and Setters
		/// <summary>
		/// Gets the start angle this PieSlice's area is set to be at in its pie menu.
		/// </summary>
		/// <returns>The start angle of this PieSlice's area.</returns>
		float GetStartAngle() const { return m_StartAngle; }

		/// <summary>
		/// Sets the start angle this PieSlice's area should be at in its pie menu.
		/// </summary>
		/// <param name="startAngle">The start angle to set for the PieSlice's area.</param>
		void SetStartAngle(float startAngle) { m_StartAngle = startAngle; RecalculateMidAngle(); }

		/// <summary>
		/// Gets the number of slots this PieSlice takes up.
		/// </summary>
		/// <returns>The number of slots this PieSlice takes up.</returns>
		int GetSlotCount() const { return m_SlotCount; }

		/// <summary>
		/// Sets the number of slots this PieSlice takes up.
		/// </summary>
		/// <param name="slotCount">The number of slots this PieSlice should take up.</param>
		void SetSlotCount(int slotCount) { m_SlotCount = std::max(1, slotCount); RecalculateMidAngle(); }

		/// <summary>
		/// Gets the mid angle this PieSlice's area is set to be at in its pie menu.
		/// </summary>
		/// <returns>The mid angle of this PieSlice's area.</returns>
		float GetMidAngle() const { return m_MidAngle; }

		/// <summary>
		/// Sets the mid angle this PieSlice's area should be at in its pie menu.
		/// </summary>
		/// <param name="midAngle">The mid angle to set for the PieSlice's area.</param>
		void SetMidAngle(float midAngle) { m_MidAngle = midAngle; }

		/// <summary>
		/// Gets whether or not this PieSlice should draw itself flipped to match its absolute angle (i.e. its angle accounting for its PieMenu's rotation).
		/// </summary>
		/// <returns>Whether or not this PieSlice should draw itself flipped to match its absolute angle.</returns>
		bool GetDrawFlippedToMatchAbsoluteAngle() const { return m_DrawFlippedToMatchAbsoluteAngle; }

		/// <summary>
		/// Sets whether or not this PieSlice should draw itself flipped to match its absolute angle (i.e. its angle accounting for its PieMenu's rotation).
		/// </summary>
		/// <param name="shouldDrawFlippedToMatchAbsoluteAngle">Whether or not this PieSlice should draw itself flipped to match its absolute angle.</param>
		void SetDrawFlippedToMatchAbsoluteAngle(bool shouldDrawFlippedToMatchAbsoluteAngle) { m_DrawFlippedToMatchAbsoluteAngle = shouldDrawFlippedToMatchAbsoluteAngle; }
#pragma endregion

	private:

		/// <summary>
		/// Custom deleter for PieMenu to avoid include problems with unique_ptr.
		/// </summary>
		struct PieMenuCustomDeleter {
			void operator()(PieMenu *pieMenu) const;
		};

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		Type m_Type; //!< The slice type, also used to determine the icon.
		Directions m_Direction; //!< The desired direction/location of this on the PieMenu.
		bool m_CanBeMiddleSlice; //!< Whether or not this PieSlice is allowed to be the middle slice. Defaults to true and should usually stay that way.
		const Entity *m_OriginalSource; //!< A pointer to the original source of this PieSlice, normally filled in when PieSlices are added to PieMenus by objects other than the PieMenu's owner, and nullptr otherwise.

		bool m_Enabled; //!< Whether this PieSlice is enabled or disabled and greyed out.
		std::unique_ptr<Icon> m_Icon; //!< The icon of this PieSlice.

		std::string m_ScriptPath; //!< Path to the script file this should run when activated.
		std::string m_FunctionName; //!< Name of the function in the script this should run when activated.
		std::unique_ptr<PieMenu, PieMenuCustomDeleter> m_SubPieMenu; //!< Unique pointer to the sub-PieMenu this should open when activated.

		float m_StartAngle; //!< The start angle of this PieSlice's area on the PieMenu, counted in radians from straight out right and going counter clockwise.
		int m_SlotCount; //!< The arc length of the PieSlice area, so that the icon should be drawn at the AreaStart + halfway of this.
		float m_MidAngle; //!< The mid angle of this PieSlice.

		bool m_DrawFlippedToMatchAbsoluteAngle; //!< Whether or not this PieSlice should draw flipped based on its absolute angle (i.e. its angle accounting for its pie menu's rotation)

		/// <summary>
		/// Recalculates this PieSlice's mid angle based on its start angle and slot count.
		/// </summary>
		void RecalculateMidAngle();

		/// <summary>
		/// Clears all the member variables of this PieSlice, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		PieSlice(const PieSlice &reference) = delete;
		PieSlice &operator=(const PieSlice &rhs) = delete;
	};
}
#endif
