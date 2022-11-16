#ifndef _RTELUABINDOBJECTWRAPPER_
#define _RTELUABINDOBJECTWRAPPER_

namespace luabind::adl {
	class object;
}

namespace RTE {

	/// <summary>
	/// A wrapper for luabind objects, to avoid include problems with luabind.
	/// </summary>
	class LuabindObjectWrapper {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used for LuabindObjectWrapper.
		/// </summary>
		LuabindObjectWrapper() = default;

		/// <summary>
		/// Constructor method used to instantiate a LuabindObjectWrapper object in system memory.
		/// </summary>
		explicit LuabindObjectWrapper(luabind::adl::object *luabindObject) : m_LuabindObject(luabindObject) {}
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a LuabindObjectWrapper object before deletion from system memory.
		/// </summary>
		~LuabindObjectWrapper() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the LuabindObjectWrapper object.
		/// </summary>
		void Destroy();
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the wrapper's luabind object. Ownership is NOT transferred!
		/// </summary>
		/// <returns>The wrapper's luabind object.</returns>
		luabind::adl::object * GetLuabindObject() const { return m_LuabindObject; }

		/// <summary>
		/// Sets the wrapper's luabind object. Ownership is transferred!
		/// </summary>
		/// <param name="newLuabindObject">The new luabind object for the wrapper.</param>
		void SetLuabindObject(luabind::adl::object *newLuabindObject) { m_LuabindObject = newLuabindObject; }
#pragma endregion

	private:

		luabind::adl::object *m_LuabindObject; //!< The luabind object this is wrapping.
	};
}
#endif