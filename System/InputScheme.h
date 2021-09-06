#ifndef _RTEINPUTSCHEME_
#define _RTEINPUTSCHEME_

#include "InputMapping.h"
#include "Constants.h"

namespace RTE {

	/// <summary>
	/// A complete input configuration scheme description for a single player.
	/// </summary>
	class InputScheme : public Serializable {

	public:

		SerializableClassNameGetter;
		SerializableOverrideMethods;

		/// <summary>
		/// Enumeration for different input scheme presets.
		/// </summary>
		enum InputPreset {
			NoPreset,
			PresetArrowKeys,
			PresetWASDKeys,
			PresetMouseWASDKeys,
			PresetGenericDPad,
			PresetGenericDualAnalog,
			PresetGamepadSNES,
			PresetGamepadDS4,
			PresetGamepadXbox360,
			InputPresetCount
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate an InputScheme object in system memory. Create() should be called before using the object.
		/// </summary>
		InputScheme() { Clear(); }

		/// <summary>
		/// Creates an InputScheme to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the InputScheme to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const InputScheme &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Resets the entire InputScheme, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); }

		/// <summary>
		/// Resets this InputScheme to the specified player's default input device and mappings.
		/// </summary>
		/// <param name="player">The preset player defaults this InputScheme should reset to.</param>
		void ResetToPlayerDefaults(Players player);
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the InputDevice that this scheme is using.
		/// </summary>
		/// <returns>The InputDevice of this scheme. See InputDevice enumeration.</returns>
		InputDevice GetDevice() const { return m_ActiveDevice; }

		/// <summary>
		/// Sets the InputDevice this scheme is supposed to use.
		/// </summary>
		/// <param name="activeDevice">The InputDevice this scheme should use. See InputDevice enumeration.</param>
		void SetDevice(InputDevice activeDevice = InputDevice::DEVICE_KEYB_ONLY) { m_ActiveDevice = activeDevice; }

		/// <summary>
		/// Gets the InputPreset that this scheme is using.
		/// </summary>
		/// <returns>The InputPreset of this scheme. See InputPreset enumeration.</returns>
		InputPreset GetPreset() const { return m_SchemePreset; }

		/// <summary>
		/// Sets up a specific preset scheme that is sensible and recommended.
		/// </summary>
		/// <param name="schemePreset">The preset number to set the scheme to match. See InputPreset enumeration.</param>
		void SetPreset(InputPreset schemePreset = InputPreset::NoPreset);

		/// <summary>
		/// Gets the InputMappings for this.
		/// </summary>
		/// <returns>The input mappings array, which is INPUT_COUNT large.</returns>
		std::array<InputMapping, InputElements::INPUT_COUNT> * GetInputMappings() { return &m_InputMappings; }
#pragma endregion

#pragma region Input Mapping Getters and Setters
		/// <summary>
		/// Gets the name of the key/mouse/joystick button/direction that a particular input element is mapped to.
		/// </summary>
		/// <param name="whichElement">Which input element to look up.</param>
		/// <returns>A string with the appropriate clear text description of the mapped thing.</returns>
		std::string GetMappingName(int whichElement) const;

		/// <summary>
		/// Gets which keyboard key is mapped to a specific input element.
		/// </summary>
		/// <param name="whichInput">Which input element to look up.</param>
		/// <returns>Which keyboard key is mapped to the specified element.</returns>
		int GetKeyMapping(int whichInput) const { return m_InputMappings.at(whichInput).GetKey(); }

		/// <summary>
		/// Sets a keyboard key as mapped to a specific input element.
		/// </summary>
		/// <param name="whichInput">Which input element to map to.</param>
		/// <param name="whichKey">The scan code of which keyboard key to map to above input element.</param>
		void SetKeyMapping(int whichInput, int whichKey) { m_InputMappings.at(whichInput).SetKey(whichKey); }

		/// <summary>
		/// Gets which joystick button is mapped to a specific input element.
		/// </summary>
		/// <param name="whichInput">Which input element to look up.</param>
		/// <returns>Which joystick button is mapped to the specified element.</returns>
		int GetJoyButtonMapping(int whichInput) const { return m_InputMappings.at(whichInput).GetJoyButton(); }

		/// <summary>
		/// Sets a joystick button as mapped to a specific input element.
		/// </summary>
		/// <param name="whichInput">Which input element to map to.</param>
		/// <param name="whichButton">Which joystick button to map to the specified input element.</param>
		void SetJoyButtonMapping(int whichInput, int whichButton) { m_InputMappings.at(whichInput).SetJoyButton(whichButton); }

		/// <summary>
		/// Get the deadzone value for this control scheme.
		/// </summary>
		/// <returns>Joystick dead zone from 0.0 to 1.0.</returns>
		float GetJoystickDeadzone() const { return m_JoystickDeadzone; }

		/// <summary>
		/// Set the deadzone value for this control scheme.
		/// </summary>
		/// <param name="deadzoneValue">Joystick dead zone from 0.0 to 1.0.</param>
		void SetJoystickDeadzone(float deadzoneValue) { m_JoystickDeadzone = deadzoneValue; }

		/// <summary>
		/// Get the DeadZoneType for this control scheme.
		/// </summary>
		/// <returns>The DeadZoneType this scheme is set to use. See DeadZoneType enumeration.</returns>
		DeadZoneType GetJoystickDeadzoneType() const { return m_JoystickDeadzoneType; }

		/// <summary>
		/// Set the DeadZoneType for this control scheme.
		/// </summary>
		/// <param name="deadzoneType">The DeadZoneType this scheme should use. See DeadZoneType enumeration.</param>
		void SetJoystickDeadzoneType(DeadZoneType deadzoneType) { m_JoystickDeadzoneType = deadzoneType; }
#pragma endregion

#pragma region Input Mapping Capture Handling
		/// <summary>
		/// Clears all mappings for a specific input element.
		/// </summary>
		/// <param name="whichInput">Which input element to clear all mappings of.</param>
		void ClearMapping(int whichInput) { m_InputMappings.at(whichInput).Reset(); }

		/// <summary>
		/// Checks for any key press this frame and creates an input mapping accordingly.
		/// </summary>
		/// <param name="whichInput">Which input element to map for.</param>
		/// <returns>Whether there were any key presses this frame and therefore whether a mapping was successfully captured or not.</returns>
		bool CaptureKeyMapping(int whichInput);

		/// <summary>
		/// Checks for any button or direction press this frame and creates an input mapping accordingly.
		/// </summary>
		/// <param name="whichJoy">Which joystick to scan for button and stick presses.</param>
		/// <param name="whichInput">Which input element to map for.</param>
		/// <returns>Whether there were any button or stick presses this frame and therefore whether a mapping was successfully captured or not.</returns>
		bool CaptureJoystickMapping(int whichJoy, int whichInput);
#pragma endregion

	protected:

		InputDevice m_ActiveDevice; //!< The currently active device for this scheme.
		InputPreset m_SchemePreset; //!< The preset this scheme was last set to, if any.

		DeadZoneType m_JoystickDeadzoneType; //!< Which deadzone type is used.
		float m_JoystickDeadzone; //!< How much of the input to treat as a deadzone input, i.e. one not registered by the game.

		std::array<InputMapping, InputElements::INPUT_COUNT> m_InputMappings; //!< The input element mappings of this InputScheme.

	private:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		/// <summary>
		/// Clears all the member variables of this InputScheme, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif