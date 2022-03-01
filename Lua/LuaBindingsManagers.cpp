#include "LuaBindingRegisterDefinitions.h"
#include "LuaAdapters.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, ActivityMan) {
		return luabind::class_<ActivityMan>("ActivityManager")

		.property("DefaultActivityType", &ActivityMan::GetDefaultActivityType, &ActivityMan::SetDefaultActivityType)
		.property("DefaultActivityName", &ActivityMan::GetDefaultActivityName, &ActivityMan::SetDefaultActivityName)

		.def("SetStartActivity", &ActivityMan::SetStartActivity, luabind::adopt(_2)) // Transfers ownership of the Activity to start into the ActivityMan, adopts ownership (_1 is the this ptr)
		.def("GetStartActivity", &ActivityMan::GetStartActivity)
		.def("GetActivity", &ActivityMan::GetActivity)
		.def("StartActivity", (int (ActivityMan::*)(Activity *))&ActivityMan::StartActivity, luabind::adopt(_2)) // Transfers ownership of the Activity to start into the ActivityMan, adopts ownership (_1 is the this ptr)
		.def("StartActivity", (int (ActivityMan::*)(const std::string &, const std::string &))&ActivityMan::StartActivity)
		.def("RestartActivity", &ActivityMan::RestartActivity)
		.def("PauseActivity", &ActivityMan::PauseActivity)
		.def("EndActivity", &ActivityMan::EndActivity)
		.def("ActivityRunning", &ActivityMan::ActivityRunning)
		.def("ActivityPaused", &ActivityMan::ActivityPaused);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, AudioMan) {
		return luabind::class_<AudioMan>("AudioManager")

		.property("MusicVolume", &AudioMan::GetMusicVolume, &AudioMan::SetMusicVolume)
		.property("SoundsVolume", &AudioMan::GetSoundsVolume, &AudioMan::SetSoundsVolume)

		.def("StopAll", &AudioMan::StopAll)
		.def("GetGlobalPitch", &AudioMan::GetGlobalPitch)
		.def("IsMusicPlaying", &AudioMan::IsMusicPlaying)
		.def("SetTempMusicVolume", &AudioMan::SetTempMusicVolume)
		.def("GetMusicPosition", &AudioMan::GetMusicPosition)
		.def("SetMusicPosition", &AudioMan::SetMusicPosition)
		.def("SetMusicPitch", &AudioMan::SetMusicPitch)
		.def("StopMusic", &AudioMan::StopMusic)
		.def("PlayMusic", &AudioMan::PlayMusic)
		.def("PlayNextStream", &AudioMan::PlayNextStream)
		.def("StopMusic", &AudioMan::StopMusic)
		.def("QueueMusicStream", &AudioMan::QueueMusicStream)
		.def("QueueSilence", &AudioMan::QueueSilence)
		.def("ClearMusicQueue", &AudioMan::ClearMusicQueue)
		.def("PlaySound", (SoundContainer *(AudioMan:: *)(const std::string &filePath)) &AudioMan::PlaySound, luabind::adopt(luabind::result))
		.def("PlaySound", (SoundContainer *(AudioMan:: *)(const std::string &filePath, const Vector &position)) &AudioMan::PlaySound, luabind::adopt(luabind::result))
		.def("PlaySound", (SoundContainer *(AudioMan:: *)(const std::string &filePath, const Vector &position, int player)) &AudioMan::PlaySound, luabind::adopt(luabind::result));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, ConsoleMan) {
		return luabind::class_<ConsoleMan>("ConsoleManager")

		.def("PrintString", &ConsoleMan::PrintString)
		.def("SaveInputLog", &ConsoleMan::SaveInputLog)
		.def("SaveAllText", &ConsoleMan::SaveAllText)
		.def("Clear", &ConsoleMan::ClearLog);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, FrameMan) {
		return luabind::class_<FrameMan>("FrameManager")

		.property("PlayerScreenWidth", &FrameMan::GetPlayerScreenWidth)
		.property("PlayerScreenHeight", &FrameMan::GetPlayerScreenHeight)

		.def("LoadPalette", &FrameMan::LoadPalette)
		.def("SetScreenText", &FrameMan::SetScreenText)
		.def("ClearScreenText", &FrameMan::ClearScreenText)
		.def("FadeInPalette", &FrameMan::FadeInPalette)
		.def("FadeOutPalette", &FrameMan::FadeOutPalette)
		.def("SaveScreenToPNG", &FrameMan::SaveScreenToPNG)
		.def("SaveBitmapToPNG", &FrameMan::SaveBitmapToPNG)
		.def("FlashScreen", &FrameMan::FlashScreen)
		.def("CalculateTextHeight", &FrameMan::CalculateTextHeight)
		.def("CalculateTextWidth", &FrameMan::CalculateTextWidth);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, MetaMan) {
		return luabind::class_<MetaMan>("MetaManager")

		.property("GameName", &MetaMan::GetGameName, &MetaMan::SetGameName)
		.property("PlayerTurn", &MetaMan::GetPlayerTurn)
		.property("PlayerCount", &MetaMan::GetPlayerCount)

		.def_readwrite("Players", &MetaMan::m_Players, luabind::return_stl_iterator)

		.def("GetTeamOfPlayer", &MetaMan::GetTeamOfPlayer)
		.def("GetPlayer", &MetaMan::GetPlayer)
		.def("GetMetaPlayerOfInGamePlayer", &MetaMan::GetMetaPlayerOfInGamePlayer);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, MovableMan) {
		return luabind::class_<MovableMan>("MovableManager")

		.property("MaxDroppedItems", &MovableMan::GetMaxDroppedItems, &MovableMan::SetMaxDroppedItems)

		.def_readwrite("Actors", &MovableMan::m_Actors, luabind::return_stl_iterator)
		.def_readwrite("Items", &MovableMan::m_Items, luabind::return_stl_iterator)
		.def_readwrite("Particles", &MovableMan::m_Particles, luabind::return_stl_iterator)
		.def_readwrite("AddedActors", &MovableMan::m_AddedActors, luabind::return_stl_iterator)
		.def_readwrite("AddedItems", &MovableMan::m_AddedItems, luabind::return_stl_iterator)
		.def_readwrite("AddedParticles", &MovableMan::m_AddedParticles, luabind::return_stl_iterator)
		.def_readwrite("AlarmEvents", &MovableMan::m_AlarmEvents, luabind::return_stl_iterator)
		.def_readwrite("AddedAlarmEvents", &MovableMan::m_AddedAlarmEvents, luabind::return_stl_iterator)

		.def("GetMOFromID", &MovableMan::GetMOFromID)
		.def("FindObjectByUniqueID", &MovableMan::FindObjectByUniqueID)
		.def("GetMOIDCount", &MovableMan::GetMOIDCount)
		.def("GetTeamMOIDCount", &MovableMan::GetTeamMOIDCount)
		.def("PurgeAllMOs", &MovableMan::PurgeAllMOs)
		.def("GetNextActorInGroup", &MovableMan::GetNextActorInGroup)
		.def("GetPrevActorInGroup", &MovableMan::GetPrevActorInGroup)
		.def("GetNextTeamActor", &MovableMan::GetNextTeamActor)
		.def("GetPrevTeamActor", &MovableMan::GetPrevTeamActor)
		.def("GetClosestTeamActor", &MovableMan::GetClosestTeamActor)
		.def("GetClosestEnemyActor", &MovableMan::GetClosestEnemyActor)
		.def("GetFirstTeamActor", &MovableMan::GetFirstTeamActor)
		.def("GetClosestActor", &MovableMan::GetClosestActor)
		.def("GetClosestBrainActor", &MovableMan::GetClosestBrainActor)
		.def("GetFirstBrainActor", &MovableMan::GetFirstBrainActor)
		.def("GetClosestOtherBrainActor", &MovableMan::GetClosestOtherBrainActor)
		.def("GetFirstOtherBrainActor", &MovableMan::GetFirstOtherBrainActor)
		.def("GetUnassignedBrain", &MovableMan::GetUnassignedBrain)
		.def("GetParticleCount", &MovableMan::GetParticleCount)
		.def("GetSplashRatio", &MovableMan::GetSplashRatio)
		.def("SortTeamRoster", &MovableMan::SortTeamRoster)
		.def("ChangeActorTeam", &MovableMan::ChangeActorTeam)
		.def("RemoveActor", &MovableMan::RemoveActor)
		.def("RemoveItem", &MovableMan::RemoveItem)
		.def("RemoveParticle", &MovableMan::RemoveParticle)
		.def("ValidMO", &MovableMan::ValidMO)
		.def("IsActor", &MovableMan::IsActor)
		.def("IsDevice", &MovableMan::IsDevice)
		.def("IsParticle", &MovableMan::IsParticle)
		.def("IsOfActor", &MovableMan::IsOfActor)
		.def("GetRootMOID", &MovableMan::GetRootMOID)
		.def("RemoveMO", &MovableMan::RemoveMO)
		.def("KillAllTeamActors", &MovableMan::KillAllTeamActors)
		.def("KillAllEnemyActors", &MovableMan::KillAllEnemyActors)
		.def("OpenAllDoors", &MovableMan::OpenAllDoors)
		.def("IsParticleSettlingEnabled", &MovableMan::IsParticleSettlingEnabled)
		.def("EnableParticleSettling", &MovableMan::EnableParticleSettling)
		.def("IsMOSubtractionEnabled", &MovableMan::IsMOSubtractionEnabled)

		.def("AddMO", &AddMO, luabind::adopt(_2))
		.def("AddActor", &AddActor, luabind::adopt(_2))
		.def("AddItem", &AddItem, luabind::adopt(_2))
		.def("AddParticle", &AddParticle, luabind::adopt(_2));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, PostProcessMan) {
		return luabind::class_<PostProcessMan>("PostProcessManager")

		.def("RegisterPostEffect", &PostProcessMan::RegisterPostEffect);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, PresetMan) {
		return luabind::class_<PresetMan>("PresetManager")

		.def_readwrite("Modules", &PresetMan::m_pDataModules, luabind::return_stl_iterator)

		.def("LoadDataModule", (bool (PresetMan::*)(std::string))&PresetMan::LoadDataModule)
		.def("GetDataModule", &PresetMan::GetDataModule)
		.def("GetModuleID", &PresetMan::GetModuleID)
		.def("GetModuleIDFromPath", &PresetMan::GetModuleIDFromPath)
		.def("GetTotalModuleCount", &PresetMan::GetTotalModuleCount)
		.def("GetOfficialModuleCount", &PresetMan::GetOfficialModuleCount)
		.def("AddPreset", &PresetMan::AddEntityPreset)
		.def("GetPreset", (const Entity *(PresetMan::*)(std::string, std::string, int))&PresetMan::GetEntityPreset)
		.def("GetPreset", (const Entity *(PresetMan::*)(std::string, std::string, std::string))&PresetMan::GetEntityPreset)
		.def("GetLoadout", (Actor * (PresetMan::*)(std::string, std::string, bool))&PresetMan::GetLoadout, luabind::adopt(luabind::result))
		.def("GetLoadout", (Actor * (PresetMan::*)(std::string, int, bool))&PresetMan::GetLoadout, luabind::adopt(luabind::result))
		.def("GetRandomOfGroup", &PresetMan::GetRandomOfGroup)
		.def("GetRandomOfGroupInModuleSpace", &PresetMan::GetRandomOfGroupInModuleSpace)
		.def("GetEntityDataLocation", &PresetMan::GetEntityDataLocation)
		.def("ReadReflectedPreset", &PresetMan::ReadReflectedPreset)
		.def("ReloadAllScripts", &PresetMan::ReloadAllScripts);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, PrimitiveMan) {
		return luabind::class_<PrimitiveMan>("PrimitiveManager")

		.def("DrawLinePrimitive", (void (PrimitiveMan::*)(const Vector &start, const Vector &end, unsigned char color))&PrimitiveMan::DrawLinePrimitive)
		.def("DrawLinePrimitive", (void (PrimitiveMan::*)(int player, const Vector &start, const Vector &end, unsigned char color))&PrimitiveMan::DrawLinePrimitive)
		.def("DrawArcPrimitive", (void (PrimitiveMan::*)(const Vector &pos, float startAngle, float endAngle, int radius, unsigned char color))&PrimitiveMan::DrawArcPrimitive)
		.def("DrawArcPrimitive", (void (PrimitiveMan::*)(const Vector &pos, float startAngle, float endAngle, int radius, unsigned char color, int thickness))&PrimitiveMan::DrawArcPrimitive)
		.def("DrawArcPrimitive", (void (PrimitiveMan::*)(int player, const Vector &pos, float startAngle, float endAngle, int radius, unsigned char color))&PrimitiveMan::DrawArcPrimitive)
		.def("DrawArcPrimitive", (void (PrimitiveMan::*)(int player, const Vector &pos, float startAngle, float endAngle, int radius, unsigned char color, int thickness))&PrimitiveMan::DrawArcPrimitive)
		.def("DrawSplinePrimitive", (void (PrimitiveMan::*)(const Vector &start, const Vector &guideA, const Vector &guideB, const Vector &end, unsigned char color))&PrimitiveMan::DrawSplinePrimitive)
		.def("DrawSplinePrimitive", (void (PrimitiveMan::*)(int player, const Vector &start, const Vector &guideA, const Vector &guideB, const Vector &end, unsigned char color))&PrimitiveMan::DrawSplinePrimitive)
		.def("DrawBoxPrimitive", (void (PrimitiveMan::*)(const Vector &start, const Vector &end, unsigned char color))&PrimitiveMan::DrawBoxPrimitive)
		.def("DrawBoxPrimitive", (void (PrimitiveMan::*)(int player, const Vector &start, const Vector &end, unsigned char color))&PrimitiveMan::DrawBoxPrimitive)
		.def("DrawBoxFillPrimitive", (void (PrimitiveMan::*)(const Vector &start, const Vector &end, unsigned char color))&PrimitiveMan::DrawBoxFillPrimitive)
		.def("DrawBoxFillPrimitive", (void (PrimitiveMan::*)(int player, const Vector &start, const Vector &end, unsigned char color))&PrimitiveMan::DrawBoxFillPrimitive)
		.def("DrawRoundedBoxPrimitive", (void (PrimitiveMan::*)(const Vector &start, const Vector &end, int cornerRadius, unsigned char color))&PrimitiveMan::DrawRoundedBoxPrimitive)
		.def("DrawRoundedBoxPrimitive", (void (PrimitiveMan::*)(int player, const Vector &start, const Vector &end, int cornerRadius, unsigned char color))&PrimitiveMan::DrawRoundedBoxPrimitive)
		.def("DrawRoundedBoxFillPrimitive", (void (PrimitiveMan::*)(const Vector &start, const Vector &end, int cornerRadius, unsigned char color))&PrimitiveMan::DrawRoundedBoxFillPrimitive)
		.def("DrawRoundedBoxFillPrimitive", (void (PrimitiveMan::*)(int player, const Vector &start, const Vector &end, int cornerRadius, unsigned char color))&PrimitiveMan::DrawRoundedBoxFillPrimitive)
		.def("DrawCirclePrimitive", (void (PrimitiveMan::*)(const Vector & pos, int radius, unsigned char color))&PrimitiveMan::DrawCirclePrimitive)
		.def("DrawCirclePrimitive", (void (PrimitiveMan::*)(int player, const Vector &pos, int radius, unsigned char color))&PrimitiveMan::DrawCirclePrimitive)
		.def("DrawCircleFillPrimitive", (void (PrimitiveMan::*)(const Vector &pos, int radius, unsigned char color))&PrimitiveMan::DrawCircleFillPrimitive)
		.def("DrawCircleFillPrimitive", (void (PrimitiveMan::*)(int player, const Vector &pos, int radius, unsigned char color))&PrimitiveMan::DrawCircleFillPrimitive)
		.def("DrawEllipsePrimitive", (void (PrimitiveMan::*)(const Vector &pos, int horizRadius, int vertRadius, unsigned char color))&PrimitiveMan::DrawEllipsePrimitive)
		.def("DrawEllipsePrimitive", (void (PrimitiveMan::*)(int player, const Vector &pos, int horizRadius, int vertRadius, unsigned char color))&PrimitiveMan::DrawEllipsePrimitive)
		.def("DrawEllipseFillPrimitive", (void (PrimitiveMan::*)(const Vector &pos, int horizRadius, int vertRadius, unsigned char color))&PrimitiveMan::DrawEllipseFillPrimitive)
		.def("DrawEllipseFillPrimitive", (void (PrimitiveMan::*)(int player, const Vector &pos, int horizRadius, int vertRadius, unsigned char color))&PrimitiveMan::DrawEllipseFillPrimitive)
		.def("DrawTrianglePrimitive", (void (PrimitiveMan::*)(const Vector &pointA, const Vector &pointB, const Vector &pointC, unsigned char color))&PrimitiveMan::DrawTrianglePrimitive)
		.def("DrawTrianglePrimitive", (void (PrimitiveMan::*)(int player, const Vector &pointA, const Vector &pointB, const Vector &pointC, unsigned char color))&PrimitiveMan::DrawTrianglePrimitive)
		.def("DrawTriangleFillPrimitive", (void (PrimitiveMan::*)(const Vector &pointA, const Vector &pointB, const Vector &pointC, unsigned char color))&PrimitiveMan::DrawTriangleFillPrimitive)
		.def("DrawTriangleFillPrimitive", (void (PrimitiveMan::*)(int player, const Vector &pointA, const Vector &pointB, const Vector &pointC, unsigned char color))&PrimitiveMan::DrawTriangleFillPrimitive)
		.def("DrawTextPrimitive", (void (PrimitiveMan::*)(const Vector &start, const std::string &text, bool isSmall, int alignment))&PrimitiveMan::DrawTextPrimitive)
		.def("DrawTextPrimitive", (void (PrimitiveMan::*)(int player, const Vector &start, const std::string &text, bool isSmall, int alignment))&PrimitiveMan::DrawTextPrimitive)
		.def("DrawBitmapPrimitive", (void (PrimitiveMan::*)(const Vector &start, Entity *entity, float rotAngle, int frame))&PrimitiveMan::DrawBitmapPrimitive)
		.def("DrawBitmapPrimitive", (void (PrimitiveMan::*)(const Vector &start, Entity *entity, float rotAngle, int frame, bool hFlipped, bool vFlipped))&PrimitiveMan::DrawBitmapPrimitive)
		.def("DrawBitmapPrimitive", (void (PrimitiveMan::*)(int player, const Vector &start, Entity *entity, float rotAngle, int frame))&PrimitiveMan::DrawBitmapPrimitive)
		.def("DrawBitmapPrimitive", (void (PrimitiveMan::*)(int player, const Vector &start, Entity *entity, float rotAngle, int frame, bool hFlipped, bool vFlipped))&PrimitiveMan::DrawBitmapPrimitive)
		.def("DrawIconPrimitive", (void (PrimitiveMan::*)(const Vector &start, Entity *entity))&PrimitiveMan::DrawIconPrimitive)
		.def("DrawIconPrimitive", (void (PrimitiveMan::*)(int player, const Vector &start, Entity *entity))&PrimitiveMan::DrawIconPrimitive);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, SceneMan) {
		return luabind::class_<SceneMan>("SceneManager")

		.property("Scene", &SceneMan::GetScene)
		.property("SceneDim", &SceneMan::GetSceneDim)
		.property("SceneWidth", &SceneMan::GetSceneWidth)
		.property("SceneHeight", &SceneMan::GetSceneHeight)
		.property("SceneWrapsX", &SceneMan::SceneWrapsX)
		.property("SceneWrapsY", &SceneMan::SceneWrapsY)
		.property("LayerDrawMode", &SceneMan::GetLayerDrawMode, &SceneMan::SetLayerDrawMode)
		.property("GlobalAcc", &SceneMan::GetGlobalAcc)
		.property("OzPerKg", &SceneMan::GetOzPerKg)
		.property("KgPerOz", &SceneMan::GetKgPerOz)

		.def("LoadScene", (int (SceneMan::*)(std::string, bool, bool))&SceneMan::LoadScene)
		.def("LoadScene", (int (SceneMan::*)(std::string, bool))&SceneMan::LoadScene)
		.def("GetOffset", &SceneMan::GetOffset)
		.def("SetOffset", (void (SceneMan::*)(const Vector &, int))&SceneMan::SetOffset)
		.def("SetOffsetX", &SceneMan::SetOffsetX)
		.def("SetOffsetY", &SceneMan::SetOffsetY)
		.def("GetScreenOcclusion", &SceneMan::GetScreenOcclusion)
		.def("SetScreenOcclusion", &SceneMan::SetScreenOcclusion)
		.def("GetTerrain", &SceneMan::GetTerrain)
		.def("GetMaterial", &SceneMan::GetMaterial)
		.def("GetMaterialFromID", &SceneMan::GetMaterialFromID)
		.def("GetTerrMatter", &SceneMan::GetTerrMatter)
		.def("GetMOIDPixel", &SceneMan::GetMOIDPixel)
		.def("SetLayerDrawMode", &SceneMan::SetLayerDrawMode)
		.def("SetScroll", &SceneMan::SetScroll)
		.def("SetScrollTarget", &SceneMan::SetScrollTarget)
		.def("GetScrollTarget", &SceneMan::GetScrollTarget)
		.def("TargetDistanceScalar", &SceneMan::TargetDistanceScalar)
		.def("CheckOffset", &SceneMan::CheckOffset)
		.def("LoadUnseenLayer", &SceneMan::LoadUnseenLayer)
		.def("MakeAllUnseen", &SceneMan::MakeAllUnseen)
		.def("AnythingUnseen", &SceneMan::AnythingUnseen)
		.def("GetUnseenResolution", &SceneMan::GetUnseenResolution)
		.def("IsUnseen", &SceneMan::IsUnseen)
		.def("RevealUnseen", &SceneMan::RevealUnseen)
		.def("RevealUnseenBox", &SceneMan::RevealUnseenBox)
		.def("RestoreUnseen", &SceneMan::RestoreUnseen)
		.def("RestoreUnseenBox", &SceneMan::RestoreUnseenBox)
		.def("CastSeeRay", &SceneMan::CastSeeRay)
		.def("CastUnseeRay", &SceneMan::CastUnseeRay)
		.def("CastUnseenRay", &SceneMan::CastUnseenRay)
		.def("CastMaterialRay", (bool (SceneMan::*)(const Vector &, const Vector &, unsigned char, Vector &, int, bool))&SceneMan::CastMaterialRay)
		.def("CastMaterialRay", (float (SceneMan::*)(const Vector &, const Vector &, unsigned char, int))&SceneMan::CastMaterialRay)
		.def("CastNotMaterialRay", (bool (SceneMan::*)(const Vector &, const Vector &, unsigned char, Vector &, int, bool))&SceneMan::CastNotMaterialRay)
		.def("CastNotMaterialRay", (float (SceneMan::*)(const Vector &, const Vector &, unsigned char, int, bool))&SceneMan::CastNotMaterialRay)
		.def("CastStrengthSumRay", &SceneMan::CastStrengthSumRay)
		.def("CastMaxStrengthRay", &SceneMan::CastMaxStrengthRay)
		.def("CastStrengthRay", &SceneMan::CastStrengthRay)
		.def("CastWeaknessRay", &SceneMan::CastWeaknessRay)
		.def("CastMORay", &SceneMan::CastMORay)
		.def("CastFindMORay", &SceneMan::CastFindMORay)
		.def("CastObstacleRay", &SceneMan::CastObstacleRay)
		.def("GetLastRayHitPos", &SceneMan::GetLastRayHitPos)
		.def("FindAltitude", &SceneMan::FindAltitude)
		.def("MovePointToGround", &SceneMan::MovePointToGround)
		.def("IsWithinBounds", &SceneMan::IsWithinBounds)
		.def("ForceBounds", (bool (SceneMan::*)(int &, int &))&SceneMan::ForceBounds)
		.def("ForceBounds", (bool (SceneMan::*)(Vector &))&SceneMan::ForceBounds)//, out_value(_2))
		.def("WrapPosition", (bool (SceneMan::*)(int &, int &))&SceneMan::WrapPosition)
		.def("WrapPosition", (bool (SceneMan::*)(Vector &))&SceneMan::WrapPosition)//, out_value(_2))
		.def("SnapPosition", &SceneMan::SnapPosition)
		.def("ShortestDistance", &SceneMan::ShortestDistance)
		.def("ObscuredPoint", (bool (SceneMan::*)(Vector &, int))&SceneMan::ObscuredPoint)//, out_value(_2))
		.def("ObscuredPoint", (bool (SceneMan::*)(int, int, int))&SceneMan::ObscuredPoint)
		.def("AddSceneObject", &SceneMan::AddSceneObject, luabind::adopt(_2))
		.def("AddTerrainObject", &SceneMan::AddTerrainObject, luabind::adopt(_2))
		.def("CheckAndRemoveOrphans", (int (SceneMan::*)(int, int, int, int, bool))&SceneMan::RemoveOrphans);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, SettingsMan) {
		return luabind::class_<SettingsMan>("SettingsManager")

		.property("PrintDebugInfo", &SettingsMan::PrintDebugInfo, &SettingsMan::SetPrintDebugInfo)
		.property("RecommendedMOIDCount", &SettingsMan::RecommendedMOIDCount)
		.property("ShowEnemyHUD", &SettingsMan::ShowEnemyHUD);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, TimerMan) {
		return luabind::class_<TimerMan>("TimerManager")

		.property("TimeScale", &TimerMan::GetTimeScale, &TimerMan::SetTimeScale)
		.property("RealToSimCap", &TimerMan::GetRealToSimCap, &TimerMan::SetRealToSimCap)
		.property("DeltaTimeTicks", &TimerMan::GetDeltaTimeTicks, &TimerMan::SetDeltaTimeTicks)
		.property("DeltaTimeSecs", &TimerMan::GetDeltaTimeSecs, &TimerMan::SetDeltaTimeSecs)
		.property("DeltaTimeMS", &TimerMan::GetDeltaTimeMS)
		.property("OneSimUpdatePerFrame", &TimerMan::IsOneSimUpdatePerFrame, &TimerMan::SetOneSimUpdatePerFrame)

		.property("TicksPerSecond", &GetTicksPerSecond)

		.def("TimeForSimUpdate", &TimerMan::TimeForSimUpdate)
		.def("DrawnSimUpdate", &TimerMan::DrawnSimUpdate);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(ManagerLuaBindings, UInputMan) {
		return luabind::class_<UInputMan>("UInputManager")

		.property("FlagAltState", &UInputMan::FlagAltState)
		.property("FlagCtrlState", &UInputMan::FlagCtrlState)
		.property("FlagShiftState", &UInputMan::FlagShiftState)

		.def("GetInputDevice", &UInputMan::GetInputDevice)
		.def("ElementPressed", &UInputMan::ElementPressed)
		.def("ElementReleased", &UInputMan::ElementReleased)
		.def("ElementHeld", &UInputMan::ElementHeld)
		.def("KeyPressed", &UInputMan::KeyPressed)
		.def("KeyReleased", &UInputMan::KeyReleased)
		.def("KeyHeld", &UInputMan::KeyHeld)
		.def("WhichKeyHeld", &UInputMan::WhichKeyHeld)
		.def("MouseButtonPressed", &UInputMan::MouseButtonPressed)
		.def("MouseButtonReleased", &UInputMan::MouseButtonReleased)
		.def("MouseButtonHeld", &UInputMan::MouseButtonHeld)
		.def("MouseWheelMoved", &UInputMan::MouseWheelMoved)
		.def("JoyButtonPressed", &UInputMan::JoyButtonPressed)
		.def("JoyButtonReleased", &UInputMan::JoyButtonReleased)
		.def("JoyButtonHeld", &UInputMan::JoyButtonHeld)
		.def("WhichJoyButtonPressed", &UInputMan::WhichJoyButtonPressed)
		.def("JoyDirectionPressed", &UInputMan::JoyDirectionPressed)
		.def("JoyDirectionReleased", &UInputMan::JoyDirectionReleased)
		.def("JoyDirectionHeld", &UInputMan::JoyDirectionHeld)
		.def("AnalogMoveValues", &UInputMan::AnalogMoveValues)
		.def("AnalogAimValues", &UInputMan::AnalogAimValues)
		.def("SetMouseValueMagnitude", &UInputMan::SetMouseValueMagnitude)
		.def("AnalogAxisValue", &UInputMan::AnalogAxisValue)
		.def("AnalogStickValues", &UInputMan::AnalogStickValues)
		.def("MouseUsedByPlayer", &UInputMan::MouseUsedByPlayer)
		.def("AnyMouseButtonPress", &UInputMan::AnyMouseButtonPress)
		.def("GetMouseMovement", &UInputMan::GetMouseMovement)
		.def("DisableMouseMoving", &UInputMan::DisableMouseMoving)
		.def("SetMousePos", &UInputMan::SetMousePos)
		.def("ForceMouseWithinBox", &UInputMan::ForceMouseWithinBox)
		.def("AnyKeyPress", &UInputMan::AnyKeyPress)
		.def("AnyJoyInput", &UInputMan::AnyJoyInput)
		.def("AnyJoyPress", &UInputMan::AnyJoyPress)
		.def("AnyJoyButtonPress", &UInputMan::AnyJoyButtonPress)
		.def("AnyInput", &UInputMan::AnyKeyOrJoyInput)
		.def("AnyPress", &UInputMan::AnyPress)
		.def("AnyStartPress", &UInputMan::AnyStartPress)

		.def("MouseButtonPressed", &MouseButtonPressed)
		.def("MouseButtonReleased", &MouseButtonReleased)
		.def("MouseButtonHeld", &MouseButtonHeld);
	}
}