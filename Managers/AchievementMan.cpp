/*
//TODO: Remake this into a nice offline achievement system eventually™.
// NO STUPID 999 GAMES THING!
// Possibly unlock special junk to use ingame.

#include "AchievementMan.h"
#include "ConsoleMan.h"

using namespace std;

namespace RTE
{

#ifdef STEAM_BUILD
AchievementMan::AchievementMan():
    m_OnUserStatsReceived(this, &AchievementMan::OnUserStatsReceived)
#else
    AchievementMan::AchievementMan()
#endif  // STEAM_BUILD
{
	m_Initialized = false;
#ifdef STEAM_BUILD
	m_pSteamUserStats = SteamUserStats();
	
	if (m_pSteamUserStats)
	{
		m_pSteamUserStats->RequestCurrentStats();
	}
#endif  // STEAM_BUILD
}

int AchievementMan::Create()
{
#ifdef STEAM_BUILD
	if (!m_pSteamUserStats || !m_pSteamUserStats->RequestCurrentStats())
	{
		g_ConsoleMan.PrintString("STEAM API: Failed to download stats!");
	}
#endif  // STEAM_BUILD

    return 0;
}

bool AchievementMan::IsAchievementUnlocked(const char *achievementName)
{
	bool unlocked = false;
#ifdef STEAM_BUILD
	if (m_pSteamUserStats && !m_pSteamUserStats->GetAchievement(achievementName, &unlocked))
	{
		g_ConsoleMan.PrintString("STEAM API: Failed to get achievement status!");
	}
#endif  // STEAM_BUILD

	return unlocked;
}

void AchievementMan::LockAchievement(const char *achievementName)
{
#ifdef STEAM_BUILD
	if (m_pSteamUserStats)
	{
		m_pSteamUserStats->ClearAchievement(achievementName);
		Sync();
	}
#endif  // STEAM_BUILD
}

void AchievementMan::UnlockAchievement(const char *achievementName)
{
#ifdef STEAM_BUILD
	if (m_pSteamUserStats)
	{
		m_pSteamUserStats->SetAchievement(achievementName);
		Sync();
	}
#endif  // STEAM_BUILD
}

#ifdef STEAM_BUILD
void AchievementMan::OnUserStatsReceived(UserStatsReceived_t *data)
{
	if (data->m_nGameID && data->m_eResult == k_EResultOK)
	{
		g_ConsoleMan.PrintString("STEAM API: Achievement status received!");
		m_Initialized = true;
		Sync();

		UnlockAchievement( "CC_BOOTUP" );
	}
}
#endif  // STEAM_BUILD

void AchievementMan::ProgressAchievement(const char *achievementName, int progress, int max)
{
#ifdef STEAM_BUILD
	if (!IsReady() || !m_pSteamUserStats)
	{
		g_ConsoleMan.PrintString("ACHIEVEMENTS: Tried to increase progress, but AchievementMan isn't ready!");
		return;
	}

	std::string name = achievementName + std::string("_PROGRESS");
	const char *statName = name.c_str();

	int current = 0;
	m_pSteamUserStats->GetStat(statName, &current);

	int val = progress + current;
	if (val >= max)
	{
		UnlockAchievement(achievementName);
		val = max;
	}

	m_pSteamUserStats->SetStat(statName, val);
#endif  // STEAM_BUILD
}

void AchievementMan::SetAchievementBit(const char *achievementName, int bit, int max)
{
#ifdef STEAM_BUILD
	if (!IsReady() || !m_pSteamUserStats)
	{
		g_ConsoleMan.PrintString("ACHIEVEMENTS: Tried to increase progress, but AchievementMan isn't ready!");
		return;
	}

	bit--;

	std::string name = achievementName + std::string("_BITFIELD");
	const char *statName = name.c_str();

	int current = 0;
	m_pSteamUserStats->GetStat(statName, &current);

	int val = current | 1 << bit;

	if (val == ~((~0) << max))
	{
		UnlockAchievement(achievementName);
	}

	m_pSteamUserStats->SetStat(statName, val);
#endif  // STEAM_BUILD
}

void AchievementMan::Sync()
{
#ifdef STEAM_BUILD
	if (m_pSteamUserStats)
	{
		m_pSteamUserStats->StoreStats();
	}
#endif  // STEAM_BUILD
}

} // namespace RTE
*/
