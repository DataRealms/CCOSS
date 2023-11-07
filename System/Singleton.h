#ifndef _RTESINGLETON_
#define _RTESINGLETON_

#include "RTEError.h"

namespace RTE {

	template <typename Type>

	/// <summary>
	/// Anything derived from this class will adhere to the Singleton pattern.
	/// To convert any class into a Singleton, do the three following steps:
	/// 1. Publicly derive your class MyClass from Singleton.
	/// 2. Make sure to instantiate MyClass once before using it.
	/// 3. Call MyClass::Instance() to use the MyClass object from anywhere.
	/// </summary>
	class Singleton {

	public:

		/// <summary>
		/// Destructor method used to clean up a Singleton object before deletion.
		/// </summary>
		~Singleton() = default;

		/// <summary>
		/// Returns the sole instance of this Singleton.
		/// </summary>
		/// <returns>A reference to the sole instance of this Singleton.</returns>
		inline static Type & Instance() { return *s_Instance; }

		/// <summary>
		/// Constructs this Singleton.
		/// </summary>
		inline static void Construct() { s_Instance = new Type(); }

	protected:

		/// <summary>
		/// Constructor method used to instantiate a Singleton object.
		/// </summary>
		Singleton() { RTEAssert(!s_Instance, "Trying to create a second instance of a Singleton!"); }

	private:

		static Type *s_Instance; //!< Pointer to instance of this singleton.

		// Disallow the use of some implicit methods.
		Singleton(const Singleton &reference) = delete;
		Singleton & operator=(const Singleton &rhs) = delete;
	};

	template <typename Type> Type * Singleton<Type>::s_Instance = nullptr;
}
#endif