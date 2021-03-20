#ifndef _RTETITLESCREEN_
#define _RTETITLESCREEN_

namespace RTE {

	/// <summary>
	/// 
	/// </summary>
	class TitleScreen {

	public:

#pragma region Creation
		/// <summary>
		/// 
		/// </summary>
		TitleScreen();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// 
		/// </summary>
		~TitleScreen();
#pragma endregion

#pragma region Concrete Methods

#pragma endregion

	private:

		/// <summary>
		/// Clears all the member variables of this TitleScreen, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		TitleScreen(const TitleScreen &reference) = delete;
		TitleScreen & operator=(const TitleScreen &rhs) = delete;
	};
}
#endif