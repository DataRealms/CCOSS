#ifndef _RTEGENERICSAVEDDATA_
#define _RTEGENERICSAVEDDATA_

#include "Serializable.h"

namespace RTE {

	/// <summary>
	/// Helper class to save generic data.
	/// </summary>
	class GenericSavedData : public Serializable {

		/// <summary>
		/// Helper class to save generic string data.
		/// </summary>
		class GenericSavedStrings : public Serializable {

		public:

			SerializableClassNameGetter;
			SerializableOverrideMethods;

			/// <summary>
			/// Constructor method used to instantiate a GenericSavedStrings object in system memory and make it ready for use.
			/// </summary>
			GenericSavedStrings() = default;

			/// <summary>
			///	Constructor method used to instantiate a GenericSavedStrings object to be identical to another, by deep copy, and make it ready for use.
			/// </summary>
			/// <param name="reference">A reference to the GenericSavedStrings to deep copy.</param>
			GenericSavedStrings(const GenericSavedStrings &reference) = default;

			std::unordered_map<std::string, std::string> m_Data; //!< Stored string data.

		private:

			static const std::string c_ClassName; //!< A string with the friendly formatted type name of this object.
		};

		/// <summary>
		/// Helper class to save generic number data.
		/// </summary>
		class GenericSavedNumbers : public Serializable {

		public:

			SerializableClassNameGetter;
			SerializableOverrideMethods;

			/// <summary>
			/// Constructor method used to instantiate a GenericSavedNumbers object in system memory and make it ready for use.
			/// </summary>
			GenericSavedNumbers() = default;

			/// <summary>
			///	Constructor method used to instantiate a GenericSavedNumbers object to be identical to another, by deep copy, and make it ready for use.
			/// </summary>
			/// <param name="reference">A reference to the GenericSavedNumbers to deep copy.</param>
			GenericSavedNumbers(const GenericSavedNumbers &reference) = default;

			std::unordered_map<std::string, float> m_Data; //!< Stored number data.

		private:

			static const std::string c_ClassName; //!< A string with the friendly formatted type name of this object.
		};

	public:

		SerializableClassNameGetter;
		SerializableOverrideMethods;

		/// <summary>
		/// Constructor method used to instantiate a GenericSavedData object in system memory and make it ready for use.
		/// </summary>
		GenericSavedData() = default;

		/// <summary>
		///	Constructor method used to instantiate a GenericSavedData object to be identical to another, by deep copy, and make it ready for use.
		/// </summary>
		/// <param name="reference">A reference to the GenericSavedData to deep copy.</param>
		GenericSavedData(const GenericSavedData &reference) = default;

		GenericSavedStrings m_SavedStrings; //!< Stored string data.
		GenericSavedNumbers m_SavedNumbers; //!< Stored number data.

	private:

		static const std::string c_ClassName; //!< A string with the friendly formatted type name of this object.
	};
}
#endif