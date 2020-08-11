/*
//TODO: Remake this into a nice offline achievement system eventually™.
// NO STUPID 999 GAMES THING!
// Possibly unlock special junk to use ingame.

#ifndef _ACHIEVEMENTMAN_
#define _ACHIEVEMENTMAN_

#include "Singleton.h"
#include "Timer.h"
#define g_AchievementMan AchievementMan::Instance()

#ifdef STEAM_BUILD
    #include "steam_api.h"
#endif // STEAM_BUILD

#define CCSTEAMAPPID 209670

namespace RTE
{
class AchievementMan : public Singleton<AchievementMan>
{
public:

    AchievementMan();

    ~AchievementMan() { Sync(); };

	int Create();

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsAchievementUnlocked
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the current status of an achievement.
// Arguments:       None.
// Return value:    Whether the achievement is unlocked.

    bool IsAchievementUnlocked(const char *achievementName);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LockAchievement
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Marks an achievement as locked. For testing purposes only.
// Arguments:       Achievement to lock.
// Return value:    None.

    void LockAchievement(const char *achievementName);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UnlockAchievement
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Marks an achievement as unlocked.
// Arguments:       Achievement to unlock.
// Return value:    None.

    void UnlockAchievement(const char *achievementName);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ProgressAchievement
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Increases the progress of an achievement.
// Arguments:		The name of the achievement.
//					How much to progress it by.
//					The maximum progress of this achievement.
// Return value:	None.

	void ProgressAchievement(const char *achievementName, int progress, int max);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetAchievementBit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Increases the progress of a bitfield achievement.
//                  Keep in mind that this is stored internally as an integer.
// Arguments:		The name of the achievement.
//					Which bit to enable (1-indexed).
//					The amount of bits required to unlock this achievement.
// Return value:	None.

	void SetAchievementBit(const char *achievementName, int bit, int max);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsReady
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whether this manager is ready or not.
// Arguments:       None.
// Return value:    Whether this manager is ready.

	bool IsReady() { return m_Initialized; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Sync
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Uploads achievements & stats to Steam.
// Arguments:       None.
// Return value:    None.

    void Sync();

	void Destroy() { Sync(); };

#ifdef STEAM_BUILD
	STEAM_CALLBACK(AchievementMan, OnUserStatsReceived, UserStatsReceived_t, m_OnUserStatsReceived);
#endif // STEAM_BUILD

private:

#ifdef STEAM_BUILD
    ISteamUserStats *m_pSteamUserStats;
#endif // STEAM_BUILD
	bool m_Initialized;

};

} // namespace RTE

#endif // _ACHIEVEMENTMAN_
*/