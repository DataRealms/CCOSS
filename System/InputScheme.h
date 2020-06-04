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

		SerializableOverrideMethods

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a InputScheme object in system memory. Create() should be called before using the object.
		/// </summary>
		InputScheme() { Clear(); }

		/// <summary>
		/// Creates a InputScheme to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the InputScheme to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const InputScheme &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a InputScheme object before deletion from system memory.
		/// </summary>
		virtual ~InputScheme() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the InputScheme object.
		/// </summary>
		void Destroy() { Clear(); }

		/// <summary>
		/// Resets the entire InputScheme, including its inherited members, to their default settings or values.
		/// </summary>
		virtual void Reset() { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the number of the device that this scheme is using.
		/// </summary>
		/// <returns>The device number of this scheme. See InputDevice enumeration.</returns>
		int GetDevice() const { return m_ActiveDevice; }

		/// <summary>
		/// Sets the device this scheme is supposed to use.
		/// </summary>
		/// <param name="activeDevice">The number of the device this scheme should use. See InputDevice enumeration.</param>
		void SetDevice(int activeDevice = 0) { m_ActiveDevice = activeDevice; }

		/// <summary>
		/// Gets the number of the last preset that this was set to.
		/// </summary>
		/// <returns>The last preset number set of this scheme. See InputPreset enumeration.</returns>
		int GetPreset() const { return m_SchemePreset; }

		/// <summary>
		/// Sets up a specific preset scheme that is sensible and recommended.
		/// </summary>
		/// <param name="schemePreset">The preset number to set the scheme to match. See InputPreset enumeration.</param>
		void SetPreset(int schemePreset = 0);

		/// <summary>
		/// Gets the InputMappings for this.
		/// </summary>
		/// <returns>The input mappings array, which is INPUT_COUNT large.</returns>
		InputMapping * GetInputMappings() { return m_InputMapping; }

		/// <summary>
		/// Get the deadzone value for this control scheme.
		/// </summary>
		/// <returns>Joystick dead zone from 0.0 to 1.0.</returns>
		float GetJoystickDeadzone() { return m_JoystickDeadzone; }

		/// <summary>
		/// Set the deadzone value for this control scheme.
		/// </summary>
		/// <param name="val">Joystick dead zone from 0.0 to 1.0.</param>
		void SetJoystickDeadzone(float val) { m_JoystickDeadzone = val; }

		/// <summary>
		/// Get the deadzone type for this control scheme.
		/// </summary>
		/// <returns>Joystick dead zone type this scheme is set to use.</returns>
		int GetJoystickDeadzoneType() { return m_JoystickDeadzoneType; }

		/// <summary>
		/// Set the deadzone type for this control scheme.
		/// </summary>
		/// <param name="val">Joystick dead zone type this scheme should use. See DeadZoneType enumeration.</param>
		void SetJoystickDeadzoneType(int val) { m_JoystickDeadzoneType = val; }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Sets some sensible default bindings for this.
		/// </summary>
		//void SetupDefaults();
#pragma endregion

#pragma region Class Info
		/// <summary>
		/// Gets the class name of this object.
		/// </summary>
		/// <returns>A string with the friendly-formatted type name of this object.</returns>
		virtual const std::string & GetClassName() const { return c_ClassName; }
#pragma endregion

	protected:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		int m_ActiveDevice; //!< The currently active device for this scheme.		
		int m_SchemePreset; //!< The preset this scheme was last set to, if any.

		float m_JoystickDeadzone; //!< How much of the input to treat like a deadzone input, not registered by the game.	
		int m_JoystickDeadzoneType; //!< Which deadzone type is used.

		InputMapping m_InputMapping[INPUT_COUNT]; //!< The device input element mappings.

	private:

		/// <summary>
		/// Clears all the member variables of this InputScheme, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif