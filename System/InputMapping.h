#ifndef _RTEINPUTMAPPING_
#define _RTEINPUTMAPPING_

#include "Serializable.h"

namespace RTE {

	/// <summary>
	/// A map between an input element and specific input device elements.
	/// </summary>
	class InputMapping : public Serializable {

	public:

		SerializableClassNameGetter
		SerializableOverrideMethods

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate an InputMapping object in system memory. Create() should be called before using the object.
		/// </summary>
		InputMapping() { Clear(); }

		/// <summary>
		/// Creates an InputMapping to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the InputMapping to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const InputMapping &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Resets the entire InputMapping, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the description of the input scheme preset that this element is part of, if any preset has been set for this element's scheme.
		/// </summary>
		/// <returns>The description associated with this element by the scheme preset, if any has been set. This string is empty otherwise.</returns>
		std::string GetPresetDescription() const { return m_PresetDescription; }

		/// <summary>
		/// Sets the description of the input scheme preset that this element is part of, if any preset has been set for this element's scheme.
		/// </summary>
		/// <param name="presetDescription">The description associated with this element by the scheme preset, if any has been set. This string should be empty otherwise.</param>
		void SetPresetDescription(const std::string &presetDescription) { m_PresetDescription = presetDescription; }
#pragma endregion

#pragma region Keyboard Getters and Setters
		/// <summary>
		/// Gets the keyboard key mapping.
		/// </summary>
		/// <returns>The keyboard key this is mapped to.</returns>
		int GetKey() const { return m_KeyMap; }

		/// <summary>
		/// Sets the keyboard button mapping.
		/// </summary>
		/// <param name="newKey">The scan code of the new key to map to.</param>
		void SetKey(int newKey) { m_KeyMap = newKey; }
#pragma endregion

#pragma region Mouse Getters and Setters
		/// <summary>
		/// Gets the mouse button mapping.
		/// </summary>
		/// <returns>The number of the mouse button this is mapped to.</returns>
		int GetMouseButton() const { return m_MouseButtonMap; }

		/// <summary>
		/// Sets the mouse button mapping.
		/// </summary>
		/// <param name="newButton">The number of the mouse button this should be mapped to.</param>
		void SetMouseButton(int newButton) { m_MouseButtonMap = newButton; }
#pragma endregion

#pragma region Joystick Getters and Setters
		/// <summary>
		/// Shows whether this is mapped to a joy direction or not.
		/// </summary>
		/// <returns>Joy direction mapped or not.</returns>
		bool JoyDirMapped() const { return m_DirectionMapped; }

		/// <summary>
		/// Gets the joystick stick number that this is mapped to.
		/// </summary>
		/// <returns>The direction, UInputMan::JOYDIR_ONE or UInputMan::JOYDIR_TWO.</returns>
		int GetDirection() const { return m_DirectionMap; }

		/// <summary>
		/// Sets the joystick direction mapping.
		/// </summary>
		/// <param name="newStick">The number of the stick this should be mapped to.</param>
		/// <param name="newAxis">The number of the axis this should be mapped to.</param>
		/// <param name="newDirection">The number of the direction this should be mapped to.</param>
		void SetDirection(int newStick, int newAxis, int newDirection) { m_DirectionMapped = true; m_StickMap = newStick; m_AxisMap = newAxis; m_DirectionMap = newDirection; }

		/// <summary>
		/// Gets the joystick button mapping.
		/// </summary>
		/// <returns>The number of the joystick button this is mapped to.</returns>
		int GetJoyButton() const { return m_JoyButtonMap; }

		/// <summary>
		/// Sets the joystick button mapping.
		/// </summary>
		/// <param name="newButton">The number of the joystick button this should be mapped to.</param>
		void SetJoyButton(int newButton) { m_JoyButtonMap = newButton; }

		/// <summary>
		/// Gets the joystick stick number that this is mapped to.
		/// </summary>
		/// <returns>The joystick stick number.</returns>
		int GetStick() const { return m_StickMap; }

		/// <summary>
		/// Gets the joystick axis number that this is mapped to.
		/// </summary>
		/// <returns>The joystick axis number.</returns>
		int GetAxis() const { return m_AxisMap; }
#pragma endregion

	protected:

		std::string m_PresetDescription; //!< The friendly description that is associated with the scheme preset element, if any is set.

		int m_KeyMap; //!< The keyboard key mapping.
		int m_MouseButtonMap; //!< The mouse button mapping.

		bool m_DirectionMapped; //!< Whether joystick direction mapping is enabled.

		int m_JoyButtonMap; //!< The joystick button mapping.
		int m_StickMap; //!< The joystick stick mapping, if any.
		int m_AxisMap; //!< The joystick axis mapping.
		int m_DirectionMap; //!< The joystick direction mapping.

	private:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		/// <summary>
		/// Clears all the member variables of this InputMapping, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif