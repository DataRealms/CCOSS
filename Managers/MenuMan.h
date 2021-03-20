#ifndef _RTEMENUMAN_
#define _RTEMENUMAN_

#include "Singleton.h"

#define g_MenuMan MenuMan::Instance()

namespace RTE {

	/// <summary>
	/// 
	/// </summary>
	class MenuMan : public Singleton<MenuMan> {

	public:

#pragma region Creation
		/// <summary>
		/// 
		/// </summary>
		MenuMan();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// 
		/// </summary>
		~MenuMan();
#pragma endregion

	private:

	};
}
#endif