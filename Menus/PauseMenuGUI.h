#ifndef _RTEPAUSEMENUGUI_
#define _RTEPAUSEMENUGUI_

namespace RTE {

	/// <summary>
	///
	/// </summary>
	class PauseMenuGUI {

	public:

	private:

		/// <summary>
		/// Clears all the member variables of this PauseMenuGUI, effectively resetting the members of this object.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		PauseMenuGUI(const PauseMenuGUI &reference) = delete;
		PauseMenuGUI & operator=(const PauseMenuGUI &rhs) = delete;
	};
}
#endif