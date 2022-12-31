#pragma once
#ifndef _RTEGAMEVERSION_
#define _RTEGAMEVERSION_

#include "System/Semver200/semver200.h"

namespace RTE {

#pragma region Game Version
	static const version::Semver200_version c_GameVersion = version::Semver200_version(c_VersionString);
#pragma endregion

}

#endif // !_RTEGAMEVERSION_
