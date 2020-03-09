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
		/// Constructor method used to instantiate a Singleton object.
		/// </summary>
		Singleton() {
			RTEAssert(!ms_Instance, "Trying to create a second instance of a Singleton");

			// Take nonexistent object sitting at address 0x1 in memory,
			// cast to both Singleton, and whatever is deriving, and with the
			// difference calculate the singleton's instance pointer.
			uintptr_t offset = (uintptr_t)(Type *)1 - (uintptr_t)(Singleton<Type> *)(Type *)1;
			ms_Instance = (Type *)((uintptr_t)this + offset);
		}

		/// <summary>
		/// Destructor method used to clean up a Singleton object before deletion.
		/// </summary>
		~Singleton() {
			RTEAssert(ms_Instance, "Trying to destruct nonexistent Singleton instance");
			ms_Instance = 0;
		}

		/// <summary>
		/// Returns this Singleton.
		/// </summary>
		/// <returns>A reference to the sole instance of this Singleton.</returns>
		static Type & Instance() {
			RTEAssert(ms_Instance, "Trying to use Singleton before instantiation");
			return *ms_Instance;
		}

	private:

		static Type *ms_Instance; //!< Pointer to instance of this singleton.
	};

	template <typename Type> Type * Singleton<Type>::ms_Instance = 0;
}
#endif