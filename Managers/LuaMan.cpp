#include "LuaMan.h"

#include "ACDropShip.h"
#include "ACrab.h"
#include "ACraft.h"
#include "ACRocket.h"
#include "Actor.h"
#include "ActorEditor.h"
#include "ADoor.h"
#include "AEmitter.h"
#include "Emission.h"
#include "AHuman.h"
#include "Arm.h"
#include "AtomGroup.h"
#include "Attachable.h"
#include "HDFirearm.h"
#include "HeldDevice.h"
#include "Leg.h"
#include "LimbPath.h"
#include "Magazine.h"
#include "Material.h"
#include "MOSParticle.h"
#include "MOPixel.h"
#include "MOSprite.h"
#include "MOSRotating.h"
#include "Scene.h"
#include "SLTerrain.h"
#include "TerrainObject.h"
#include "SoundContainer.h"
#include "TDExplosive.h"
#include "ThrownDevice.h"
#include "Turret.h"
#include "PEmitter.h"

#include "DataModule.h"
#include "GAScripted.h"
#include "Box.h"
#include "BuyMenuGUI.h"
#include "SceneEditorGUI.h"
#include "MetaPlayer.h"
#include "GUIBanner.h"

#include "MetaMan.h"
#include "ConsoleMan.h"
#include "PresetMan.h"
#include "PrimitiveMan.h"
#include "UInputMan.h"
#include "SettingsMan.h"

#include "lua.hpp"

// LuaBind
#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1
#include "luabind/luabind.hpp"
#include "luabind/operator.hpp"
#include "luabind/copy_policy.hpp"
#include "luabind/adopt_policy.hpp"
#include "luabind/out_value_policy.hpp"
#include "luabind/iterator_policy.hpp"
#include "luabind/return_reference_to_policy.hpp"
// Boost
//#include "boost/detail/shared_ptr_nmt.hpp"
//#include "boost/shared_ptr.hpp"

using namespace luabind;

/// <summary>
/// Special callback function for adding file name and line number to error messages when calling functions incorrectly.
/// </summary>
/// <param name="pState">The Lua master state.</param>
/// <returns>An error signal, 1, so Lua correctly reports that there's been an error.</returns>
int AddFileAndLineToError(lua_State* pState) {
    lua_Debug luaDebug;
    if (lua_getstack(pState, 2, &luaDebug) > 0) {
        lua_getinfo(pState, "Sln", &luaDebug);
        std::string errorString = lua_tostring(pState, -1);

        if (errorString.find(".lua") != std::string::npos) {
            lua_pushstring(pState, errorString.c_str());
        } else {
            std::stringstream messageStream;
            messageStream << ((luaDebug.name == nullptr || strstr(luaDebug.name, ".rte") == nullptr) ? luaDebug.short_src : luaDebug.name);
            messageStream << ":" << luaDebug.currentline << ": " << errorString;
            lua_pushstring(pState, messageStream.str().c_str());
        }
    }

   return 1;
}


// From LuaBind documentation:
// You also have to supply two functions for your smart pointer.
// One that returns the type of const version of the smart pointer type (boost::shared_ptr<const A> in this case).
// And one function that extracts the raw pointer from the smart pointer.
// The first function is needed because luabind has to allow the non-const -> conversion when passing values from Lua to C++.
// The second function is needed when Lua calls member functions on held types, the this pointer must be a raw pointer, it is also needed to allow the smart_pointer -> raw_pointer conversion from Lua to C++.

namespace luabind
{
    template<class T>
    T * get_pointer(boost::shared_ptr<T> &p)
    {
        return p.get();
    }

// From LuaBind documentation:
// IMPORTANT
// get_const_holder() has been removed.
// Automatic conversions between smart_ptr<X> and smart_ptr<X const> no longer work.
    /*template<class A>
    boost::shared_ptr<const A> * get_const_holder(boost::shared_ptr<A> *)
    {
        return 0;
    }*/
}


namespace RTE
{

// Can't have global enums in the master state so we use this dummy struct as a class and register the enums under it.
struct enum_wrapper {
	// Nested structs for each enum because we can't register enum_wrapper multiple times under a different name.
	// We're doing this so we can access each enum separately by name rather than having all of them accessed from a shared name.
	// If this proves to be a hassle then we can easily revert to the shared name access by registering everything under enum_wrapper.
	struct input_device {};
	struct input_elements {};
	struct mouse_buttons {};
	struct joy_buttons {};
	struct joy_directions {};
};





//////////////////////////////////////////////////////////////////////////////////////////
// Explicit deletion of any Entity instance that Lua owns.. it will probably be handled
// by the GC, but this makes it instantaneous

void DeleteEntity(Entity *pEntity)
{
    delete pEntity;
    pEntity = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Other misc adapters to eliminate/emulate default parameters etc


double NormalRand() { return RandomNormalNum<double>(); }
double PosRand() { return RandomNum<double>(); }



/*
//////////////////////////////////////////////////////////////////////////////////////////
// Wrapper for the GAScripted so we can derive new classes from it purely in lua:
//
// "It is also possible to derive Lua classes from C++ classes, and override virtual functions with Lua functions.
// To do this we have to create a wrapper class for our C++ base class.
// This is the class that will hold the Lua object when we instantiate a Lua class"

struct GAScriptedWrapper:
    GAScripted,
    wrap_base
{
    GAScriptedWrapper(): GAScripted() { ; }

    // Passing in the path of the script file that defines, in Lua, the GAScripted-derived class
//    virtual int Create(const GAScripted &reference) { return call<int>("Create", reference); }
    virtual Entity * Clone(Entity *pCloneTo = 0) const { return call<Entity *>("Clone", pCloneTo); }
    virtual int Start() { call<int>("Start"); }
    virtual void Pause(bool pause) { call<void>("Pause", pause); }
    virtual void End() { call<void>("End"); }
    virtual void Update() { call<void>("Update"); }

//    static int static_Create(GAScripted *pSelf, const GAScripted &reference) { return pSelf->GAScripted::Create(reference); }
    static Entity * static_Clone(GAScripted *pSelf, Entity *pCloneTo = 0) { return pSelf->GAScripted::Clone(pCloneTo); }
    static int static_Start(GAScripted *pSelf) { return pSelf->GAScripted::Start(); }
    static void static_Pause(GAScripted *pSelf, bool pause) { return pSelf->GAScripted::Pause(pause); }
    static void static_End(GAScripted *pSelf) { return pSelf->GAScripted::End(); }
    static void static_Update(GAScripted *pSelf) { return pSelf->GAScripted::Update(); }
};
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LuaMan::Clear()
{
    m_pMasterState = 0;
    m_LastError.clear();
// TODO: is this the best way to give ID's.. won't ever be reset?
    m_NextPresetID = 0;
    m_NextObjectID = 0;
    m_pTempEntity = 0;
    m_TempEntityVector.clear();
    m_TempEntityVector.shrink_to_fit();

	//Clear files list
	for (int i = 0; i < MAX_OPEN_FILES; ++i)
		m_Files[i] = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LuaMan::Initialize() {
    m_pMasterState = luaL_newstate();
    // Attach the master state to LuaBind
    luabind::open(m_pMasterState);

	const luaL_Reg libsToLoad[] = {
		{ LUA_COLIBNAME, luaopen_base },
		{ LUA_LOADLIBNAME, luaopen_package },
		{ LUA_TABLIBNAME, luaopen_table },
		{ LUA_STRLIBNAME, luaopen_string },
		{ LUA_MATHLIBNAME, luaopen_math },
		{ LUA_DBLIBNAME, luaopen_debug },
		{ LUA_JITLIBNAME, luaopen_jit },
		{ NULL, NULL } // End of array
	};

	for (const luaL_Reg *lib = libsToLoad; lib->func; lib++) {
		lua_pushcfunction(m_pMasterState, lib->func);
		lua_pushstring(m_pMasterState, lib->name);
		lua_call(m_pMasterState, 1, 0);
	}

	// LuaJIT should start automatically after we load the library but we're making sure it did anyway.
	if (!luaJIT_setmode(m_pMasterState, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON)) { RTEAbort("Failed to initialize LuaJIT!"); }

    // From LuaBind documentation:
    // As mentioned in the Lua documentation, it is possible to pass an error handler function to lua_pcall().
    // Luabind makes use of lua_pcall() internally when calling member functions and free functions.
    // It is possible to set the error handler function that Luabind will use globally:
    //set_pcall_callback(&AddFileAndLineToError); //NOTE: This seems to do nothing

    // Declare all useful classes in the master state
    module(m_pMasterState)
    [

        class_<Scene::Area>("Area")
            .def(constructor<>())
            .def(constructor<string>())
            .def(constructor<const Scene::Area &>())
            .def("Reset", &Scene::Area::Reset)
            .property("ClassName", &Scene::Area::GetClassName)
            .property("Name", &Scene::Area::GetName)
            .def("AddBox", &Scene::Area::AddBox)
            .def("HasNoArea", &Scene::Area::HasNoArea)
            .def("IsInside", &Scene::Area::IsInside)
            .def("IsInsideX", &Scene::Area::IsInsideX)
            .def("IsInsideY", &Scene::Area::IsInsideY)
            .def("GetBoxInside", &Scene::Area::GetBoxInside)
            .def("RemoveBoxInside", &Scene::Area::RemoveBoxInside)
            .def("GetCenterPoint", &Scene::Area::GetCenterPoint)
            .def("GetRandomPoint", &Scene::Area::GetRandomPoint),

        class_<Entity/*, boost::shared_ptr<Entity> */>("Entity")
            .def("Clone", &CloneEntity)
            .def("Reset", &Entity::Reset)
            .def(tostring(const_self))
            .property("ClassName", &Entity::GetClassName)
            .property("PresetName", &Entity::GetPresetName, &Entity::SetPresetName)
            .property("Description", &Entity::GetDescription, &Entity::SetDescription)
            .def("GetModuleAndPresetName", &Entity::GetModuleAndPresetName)
            .property("IsOriginalPreset", &Entity::IsOriginalPreset)
            .property("ModuleID", &Entity::GetModuleID)
			.property("RandomWeight", &Entity::GetRandomWeight)
            .def("AddToGroup", &Entity::AddToGroup)
            .def("IsInGroup", (bool (Entity::*)(const string &))&Entity::IsInGroup),

        CONCRETELUABINDING(SoundContainer, Entity)
			.def(constructor<>())
            .enum_("SoundOverlapMode")[
                value("OVERLAP", SoundContainer::SoundOverlapMode::OVERLAP),
                value("RESTART", SoundContainer::SoundOverlapMode::RESTART),
                value("IGNORE_PLAY", SoundContainer::SoundOverlapMode::IGNORE_PLAY)
            ]
            .property("SoundOverlapMode", &SoundContainer::GetSoundOverlapMode, &SoundContainer::SetSoundOverlapMode)
            .property("Immobile", &SoundContainer::IsImmobile, &SoundContainer::SetImmobile)
            .property("AttenuationStartDistance", &SoundContainer::GetAttenuationStartDistance, &SoundContainer::SetAttenuationStartDistance)
            .property("Loops", &SoundContainer::GetLoopSetting, &SoundContainer::SetLoopSetting)
            .property("Priority", &SoundContainer::GetPriority, &SoundContainer::SetPriority)
            .property("AffectedByGlobalPitch", &SoundContainer::IsAffectedByGlobalPitch, &SoundContainer::SetAffectedByGlobalPitch)
            .property("Pos", &SoundContainer::GetPosition, &SoundContainer::SetPosition)
            .property("Volume", &SoundContainer::GetVolume, &SoundContainer::SetVolume)
            .property("Pitch", &SoundContainer::GetPitch, &SoundContainer::SetPitch)
            .def("HasAnySounds", &SoundContainer::HasAnySounds)
            .def("GetTopLevelSoundSet", &SoundContainer::GetTopLevelSoundSet)
			.def("IsBeingPlayed", &SoundContainer::IsBeingPlayed)
            .def("Play", (bool (SoundContainer:: *)()) &SoundContainer::Play)
            .def("Play", (bool (SoundContainer:: *)(const int player)) &SoundContainer::Play)
            .def("Play", (bool (SoundContainer:: *)(const Vector &position)) &SoundContainer::Play)
            .def("Play", (bool (SoundContainer:: *)(const Vector &position, int player)) &SoundContainer::Play)
            .def("Stop", (bool (SoundContainer:: *)()) &SoundContainer::Stop)
            .def("Stop", (bool (SoundContainer:: *)(int player)) &SoundContainer::Stop)
            .def("Restart", (bool (SoundContainer:: *)()) &SoundContainer::Restart)
            .def("Restart", (bool (SoundContainer:: *)(int player)) &SoundContainer::Restart)
            .def("FadeOut", &SoundContainer::FadeOut),

        class_<SoundSet>("SoundSet")
            .def(constructor<>())
            .enum_("SoundSelectionCycleMode")[
                value("RANDOM", SoundSet::SoundSelectionCycleMode::RANDOM),
                value("FORWARDS", SoundSet::SoundSelectionCycleMode::FORWARDS),
                value("ALL", SoundSet::SoundSelectionCycleMode::ALL)
            ]
            .property("SoundSelectionCycleMode", &SoundSet::GetSoundSelectionCycleMode, &SoundSet::SetSoundSelectionCycleMode)
            .def_readonly("SubSoundSets", &SoundSet::m_SubSoundSets, return_stl_iterator)
            .def("HasAnySounds", &SoundSet::HasAnySounds)
            .def("SelectNextSounds", &SoundSet::SelectNextSounds)
            .def("AddSound", (void (SoundSet:: *)(std::string const &soundFilePath)) &SoundSet::AddSound)
            .def("AddSound", (void (SoundSet:: *)(std::string const &soundFilePath, const Vector &offset, float minimumAudibleDistance, float attenuationStartDistance)) &SoundSet::AddSound)
            .def("AddSoundSet", &SoundSet::AddSoundSet),

        class_<LimbPath>("LimbPath")
            .property("StartOffset", &LimbPath::GetStartOffset, &LimbPath::SetStartOffset)
            .property("SegmentCount", &LimbPath::GetSegCount)
            .def("GetSegment", &LimbPath::GetSegment),

        ABSTRACTLUABINDING(SceneObject, Entity)
            .property("Pos", &SceneObject::GetPos, &SceneObject::SetPos)
            .property("HFlipped", &SceneObject::IsHFlipped, &SceneObject::SetHFlipped)
            .property("RotAngle", &SceneObject::GetRotAngle, &SceneObject::SetRotAngle)
            .property("Team", &SceneObject::GetTeam, &SceneObject::SetTeam)
            .property("PlacedByPlayer", &SceneObject::GetPlacedByPlayer, &SceneObject::SetPlacedByPlayer)
            .def("GetGoldValue", &SceneObject::GetGoldValueOld)
            .def("GetGoldValue", &SceneObject::GetGoldValue)
			.def("SetGoldValue", &SceneObject::SetGoldValue)
			.def("GetGoldValueString", &SceneObject::GetGoldValueString)
            .def("GetTotalValue", &SceneObject::GetTotalValueOld)
            .def("GetTotalValue", &SceneObject::GetTotalValue)
            .property("IsBuyable", &SceneObject::IsBuyable)
            .def("IsOnScenePoint", &SceneObject::IsOnScenePoint),

        ABSTRACTLUABINDING(MovableObject, SceneObject)
			.def("GetParent", (MOSRotating * (MovableObject::*)())&MovableObject::GetParent)
			.def("GetParent", (const MOSRotating * (MovableObject::*)() const)&MovableObject::GetParent)
			.def("GetRootParent", (MovableObject * (MovableObject::*)())&MovableObject::GetRootParent)
			.def("GetRootParent", (const MovableObject * (MovableObject::*)() const)&MovableObject::GetRootParent)
			.property("Material", &MovableObject::GetMaterial)
            .def("ReloadScripts", (int (MovableObject:: *)()) &MovableObject::ReloadScripts)
            .def("HasScript", &MovableObject::HasScript)
            .def("AddScript", &MovableObject::AddScript)
            .def("ScriptEnabled", &MovableObject::ScriptEnabled)
            .def("EnableScript", &MovableObject::EnableScript)
            .def("DisableScript", &MovableObject::DisableScript)
            .def("EnableOrDisableAllScripts", &MovableObject::EnableOrDisableAllScripts)
            .property("Mass", &MovableObject::GetMass, &MovableObject::SetMass)
            .property("Pos", &MovableObject::GetPos, &MovableObject::SetPos)
            .property("Vel", &MovableObject::GetVel, &MovableObject::SetVel)
			.property("PrevPos", &MovableObject::GetPrevPos)
			.property("PrevVel", &MovableObject::GetPrevVel)
            .property("AngularVel", &MovableObject::GetAngularVel, &MovableObject::SetAngularVel)
            .property("Radius", &MovableObject::GetRadius)
            .property("Diameter", &MovableObject::GetDiameter)
            .property("Scale", &MovableObject::GetScale, &MovableObject::SetScale)
            .property("EffectRotAngle", &MovableObject::GetEffectRotAngle, &MovableObject::SetEffectRotAngle)
            .property("GlobalAccScalar", &MovableObject::GetGlobalAccScalar, &MovableObject::SetGlobalAccScalar)
            .property("AirResistance", &MovableObject::GetAirResistance, &MovableObject::SetAirResistance)
            .property("AirThreshold", &MovableObject::GetAirThreshold, &MovableObject::SetAirThreshold)
            .property("Age", &MovableObject::GetAge, &MovableObject::SetAge)
            .property("Lifetime", &MovableObject::GetLifetime, &MovableObject::SetLifetime)
            .property("ID", &MovableObject::GetID)
            .property("UniqueID", &MovableObject::GetUniqueID)
            .property("RootID", &MovableObject::GetRootID)
            .property("MOIDFootprint", &MovableObject::GetMOIDFootprint)
            .property("Sharpness", &MovableObject::GetSharpness, &MovableObject::SetSharpness)
            .def("GetAltitude", &MovableObject::GetAltitude)
            .property("AboveHUDPos", &MovableObject::GetAboveHUDPos)
            .property("HitsMOs", &MovableObject::HitsMOs, &MovableObject::SetToHitMOs)
            .property("GetsHitByMOs", &MovableObject::GetsHitByMOs, &MovableObject::SetToGetHitByMOs)
            .property("IgnoresTeamHits", &MovableObject::IgnoresTeamHits, &MovableObject::SetIgnoresTeamHits)
            .property("IgnoresWhichTeam", &MovableObject::IgnoresWhichTeam)
			.property("IgnoreTerrain", &MovableObject::IgnoreTerrain, &MovableObject::SetIgnoreTerrain)
            .def("GetWhichMOToNotHit", &MovableObject::GetWhichMOToNotHit)
			.def("SetWhichMOToNotHit", &MovableObject::SetWhichMOToNotHit)
            .property("ToSettle", &MovableObject::ToSettle, &MovableObject::SetToSettle)
            .property("ToDelete", &MovableObject::ToDelete, &MovableObject::SetToDelete)
            .def("IsSetToDelete", &MovableObject::IsSetToDelete)
			.property("MissionCritical", &MovableObject::IsMissionCritical, &MovableObject::SetMissionCritical)
			.def("IsMissionCritical", &MovableObject::IsMissionCritical)
			.property("HUDVisible", &MovableObject::GetHUDVisible, &MovableObject::SetHUDVisible)
            .def("IsGeneric", &MovableObject::IsGeneric)
            .def("IsActor", &MovableObject::IsActor)
            .def("IsDevice", &MovableObject::IsDevice)
            .def("IsHeldDevice", &MovableObject::IsHeldDevice)
            .def("IsThrownDevice", &MovableObject::IsThrownDevice)
            .def("IsGold", &MovableObject::IsGold)
            .def("IsThrownDevice", &MovableObject::IsThrownDevice)
            .def("HasObject", &MovableObject::HasObject)
            .def("HasObjectInGroup", &MovableObject::HasObjectInGroup)
            .def("AddForce", &MovableObject::AddForce)
            .def("AddAbsForce", &MovableObject::AddAbsForce)
            .def("AddImpulseForce", &MovableObject::AddImpulseForce)
            .def("AddAbsImpulseForce", &MovableObject::AddAbsImpulseForce)
            .def("ClearForces", &MovableObject::ClearForces)
            .def("ClearImpulseForces", &MovableObject::ClearImpulseForces)
			.def("GetForcesCount", &MovableObject::GetForcesCount)
			.def("GetForceVector", &MovableObject::GetForceVector)
			.def("GetForceOffset", &MovableObject::GetForceOffset)
			.def("SetForceVector", &MovableObject::SetForceVector)
			.def("SetForceOffset", &MovableObject::SetForceOffset)
			.def("GetImpulsesCount", &MovableObject::GetImpulsesCount)
			.def("GetImpulseVector", &MovableObject::GetImpulseVector)
			.def("GetImpulseOffset", &MovableObject::GetImpulseOffset)
			.def("SetImpulseVector", &MovableObject::SetImpulseVector)
			.def("SetImpulseOffset", &MovableObject::SetImpulseOffset)
            .property("PinStrength", &MovableObject::GetPinStrength, &MovableObject::SetPinStrength)
            .def("RestDetection", &MovableObject::RestDetection)
            .def("NotResting", &MovableObject::NotResting)
            .def("IsAtRest", &MovableObject::IsAtRest)
            .def("MoveOutOfTerrain", &MovableObject::MoveOutOfTerrain)
            .def("RotateOffset", &MovableObject::RotateOffset)
			.property("DamageOnCollision", &MovableObject::DamageOnCollision, &MovableObject::SetDamageOnCollision)
			.property("DamageOnPenetration", &MovableObject::DamageOnPenetration, &MovableObject::SetDamageOnPenetration)
			.property("WoundDamageMultiplier", &MovableObject::WoundDamageMultiplier, &MovableObject::SetWoundDamageMultiplier)
			.property("HitWhatMOID", &MovableObject::HitWhatMOID)
			.property("HitWhatTerrMaterial", &MovableObject::HitWhatTerrMaterial)
			.property("ProvidesPieMenuContext", &MovableObject::ProvidesPieMenuContext, &MovableObject::SetProvidesPieMenuContext)
			.property("HitWhatParticleUniqueID", &MovableObject::HitWhatParticleUniqueID),

		class_<Material, Entity>("Material")
			.property("ID", &Material::GetIndex)
			.property("Restitution", &Material::GetRestitution)
			.property("Bounce", &Material::GetRestitution)
			.property("Friction", &Material::GetFriction)
			.property("Stickiness", &Material::GetStickiness)
			.property("Strength", &Material::GetIntegrity)
			.property("StructuralIntegrity", &Material::GetIntegrity)
			.property("DensityKGPerVolumeL", &Material::GetVolumeDensity)
			.property("DensityKGPerPixel", &Material::GetPixelDensity)
			.property("SettleMaterial", &Material::GetSettleMaterial)
			.property("SpawnMaterial", &Material::GetSpawnMaterial)
			.property("TransformsInto", &Material::GetSpawnMaterial)
			.property("IsScrap", &Material::IsScrap),

        CONCRETELUABINDING(MOPixel, MovableObject)
			.property("TrailLength", &MOPixel::GetTrailLength, &MOPixel::SetTrailLength),

        CONCRETELUABINDING(TerrainObject, SceneObject)
            .def("GetBitmapOffset", &TerrainObject::GetBitmapOffset)
            .def("GetBitmapWidth", &TerrainObject::GetBitmapWidth)
            .def("GetBitmapHeight", &TerrainObject::GetBitmapHeight),

		ABSTRACTLUABINDING(MOSprite, MovableObject)
			.enum_("SpriteAnimMode")[
				value("NOANIM", MOSprite::SpriteAnimMode::NOANIM),
				value("ALWAYSLOOP", MOSprite::SpriteAnimMode::ALWAYSLOOP),
				value("ALWAYSRANDOM", MOSprite::SpriteAnimMode::ALWAYSRANDOM),
				value("ALWAYSPINGPONG", MOSprite::SpriteAnimMode::ALWAYSPINGPONG),
				value("LOOPWHENMOVING", MOSprite::SpriteAnimMode::LOOPWHENMOVING),
				value("LOOPWHENOPENCLOSE", MOSprite::SpriteAnimMode::LOOPWHENOPENCLOSE),
				value("PINGPONGOPENCLOSE", MOSprite::SpriteAnimMode::PINGPONGOPENCLOSE),
				value("OVERLIFETIME", MOSprite::SpriteAnimMode::OVERLIFETIME),
				value("ONCOLLIDE", MOSprite::SpriteAnimMode::ONCOLLIDE)
			]
            .property("Diameter", &MOSprite::GetDiameter)
            .property("BoundingBox", &MOSprite::GetBoundingBox)
            .property("FrameCount", &MOSprite::GetFrameCount)
            .property("SpriteOffset", &MOSprite::GetSpriteOffset, &MOSprite::SetSpriteOffset)
            .property("HFlipped", &MOSprite::IsHFlipped, &MOSprite::SetHFlipped)
            .property("FlipFactor", &MOSprite::GetFlipFactor)
            .property("RotAngle", &MOSprite::GetRotAngle, &MOSprite::SetRotAngle)
            .property("AngularVel", &MOSprite::GetAngularVel, &MOSprite::SetAngularVel)
            .property("Frame", &MOSprite::GetFrame, &MOSprite::SetFrame)
            .property("SpriteAnimMode", &MOSprite::GetSpriteAnimMode, &MOSprite::SetSpriteAnimMode)
            .property("SpriteAnimDuration", &MOSprite::GetSpriteAnimDuration, &MOSprite::SetSpriteAnimDuration)
            .def("SetNextFrame", &MOSprite::SetNextFrame)
            .def("IsTooFast", &MOSprite::IsTooFast)
            .def("IsOnScenePoint", &MOSprite::IsOnScenePoint)
            .def("RotateOffset", &MOSprite::RotateOffset)
            .def("UnRotateOffset", &MOSprite::UnRotateOffset)
            .def("FacingAngle", &MOSprite::FacingAngle)
            .def("GetSpriteWidth", &MOSprite::GetSpriteWidth)
            .def("GetSpriteHeight", &MOSprite::GetSpriteHeight)
			.def("SetEntryWound", &MOSprite::SetEntryWound)
			.def("SetExitWound", &MOSprite::SetExitWound)
			.def("GetEntryWoundPresetName", &MOSprite::GetEntryWoundPresetName)
			.def("GetExitWoundPresetName", &MOSprite::GetExitWoundPresetName),

        CONCRETELUABINDING(MOSParticle, MOSprite),

        CONCRETELUABINDING(MOSRotating, MOSprite)
            /*.property("Material", &MOSRotating::GetMaterial)*/
            .property("IndividualRadius", &MOSRotating::GetIndividualRadius)
            .property("IndividualDiameter", &MOSRotating::GetIndividualDiameter)
            .property("IndividualMass", &MOSRotating::GetIndividualMass)
            .property("RecoilForce", &MOSRotating::GetRecoilForce)
            .property("RecoilOffset", &MOSRotating::GetRecoilOffset)
			.property("TravelImpulse", &MOSRotating::GetTravelImpulse, &MOSRotating::SetTravelImpulse)
			.property("GibWoundLimit", (int (MOSRotating:: *)() const) &MOSRotating::GetGibWoundLimit, &MOSRotating::SetGibWoundLimit)
			.property("GibSound", &MOSRotating::GetGibSound, &MOSRotatingSetGibSound)
			.property("GibImpulseLimit", &MOSRotating::GetGibImpulseLimit, &MOSRotating::SetGibImpulseLimit)
			.property("DamageMultiplier", &MOSRotating::GetDamageMultiplier, &MOSRotating::SetDamageMultiplier)
			.property("WoundCount", (int (MOSRotating:: *)() const) &MOSRotating::GetWoundCount)
			.property("OrientToVel", &MOSRotating::GetOrientToVel, &MOSRotating::SetOrientToVel)
            .def("AddRecoil", &MOSRotating::AddRecoil)
            .def("SetRecoil", &MOSRotating::SetRecoil)
            .def("IsRecoiled", &MOSRotating::IsRecoiled)
            .def("EnableDeepCheck", &MOSRotating::EnableDeepCheck)
            .def("ForceDeepCheck", &MOSRotating::ForceDeepCheck)
            .def("GibThis", &MOSRotating::GibThis)
            // Free function bound as member function to emulate default variables
            .def("GibThis", &GibThis)
            .def("MoveOutOfTerrain", &MOSRotating::MoveOutOfTerrain)
			.def("GetGibWoundLimit", (int (MOSRotating:: *)() const) &MOSRotating::GetGibWoundLimit)
            .def("GetGibWoundLimit", (int (MOSRotating:: *)(bool positiveDamage, bool negativeDamage, bool noDamage) const) &MOSRotating::GetGibWoundLimit)
			.def("GetWoundCount", (int (MOSRotating:: *)() const) &MOSRotating::GetWoundCount)
			.def("GetWoundCount", (int (MOSRotating:: *)(bool positiveDamage, bool negativeDamage, bool noDamage) const) &MOSRotating::GetWoundCount)
			.def("AddWound", &MOSRotating::AddWound, adopt(_2))
			.def("RemoveWounds", (float (MOSRotating:: *)(int numberOfWoundsToRemove)) &MOSRotating::RemoveWounds)
            .def("RemoveWounds", (float (MOSRotating:: *)(int numberOfWoundsToRemove, bool positiveDamage, bool negativeDamage, bool noDamage)) &MOSRotating::RemoveWounds)
            .def("IsOnScenePoint", &MOSRotating::IsOnScenePoint)
            .def("EraseFromTerrain", &MOSRotating::EraseFromTerrain)
            .def("GetStringValue", &MOSRotating::GetStringValue)
            .def("GetNumberValue", &MOSRotating::GetNumberValue)
            .def("GetObjectValue", &MOSRotating::GetObjectValue)
            .def("SetStringValue", &MOSRotating::SetStringValue)
            .def("SetNumberValue", &MOSRotating::SetNumberValue)
            .def("SetObjectValue", &MOSRotating::SetObjectValue)
            .def("RemoveStringValue", &MOSRotating::RemoveStringValue)
            .def("RemoveNumberValue", &MOSRotating::RemoveNumberValue)
            .def("RemoveObjectValue", &MOSRotating::RemoveObjectValue)
            .def("StringValueExists", &MOSRotating::StringValueExists)
            .def("NumberValueExists", &MOSRotating::NumberValueExists)
            .def("ObjectValueExists", &MOSRotating::ObjectValueExists)
            .def("AddAttachable", (void (MOSRotating::*)(Attachable *attachableToAdd))&MOSRotating::AddAttachable, adopt(_2))
            .def("AddAttachable", (void (MOSRotating::*)(Attachable *attachableToAdd, const Vector &parentOffset))&MOSRotating::AddAttachable, adopt(_2))
            .def("RemoveAttachable", &RemoveAttachableLuaSafe1)
            .def("RemoveAttachable", &RemoveAttachableLuaSafe2)
            .def("RemoveAttachable", &RemoveAttachableLuaSafe3)
            .def("RemoveAttachable", &RemoveAttachableLuaSafe4)
            /*
            .def("RemoveAttachable", (bool (MOSRotating:: *)(long uniqueIDOfAttachableToRemove)) &MOSRotating::RemoveAttachable)
            .def("RemoveAttachable", (bool (MOSRotating:: *)(long uniqueIDOfAttachableToRemove, bool addToMovableMan, bool addBreakWounds)) &MOSRotating::RemoveAttachable)
            .def("RemoveAttachable", (bool (MOSRotating::*)(Attachable *attachableToRemove))&MOSRotating::RemoveAttachable)
            .def("RemoveAttachable", (bool (MOSRotating:: *)(Attachable *attachableToRemove, bool addToMovableMan, bool addBreakWounds)) &MOSRotating::RemoveAttachable)
            */
			.def("AddEmitter", (void (MOSRotating::*)(Attachable *attachableToAdd))&MOSRotating::AddAttachable, adopt(_2))
			.def("AddEmitter", (void (MOSRotating::*)(Attachable *attachableToAdd, const Vector &parentOffset))&MOSRotating::AddAttachable, adopt(_2))
			.def("RemoveEmitter", (bool (MOSRotating::*)(Attachable *attachableToRemove))&MOSRotating::RemoveAttachable)
			.def("RemoveEmitter", (bool (MOSRotating::*)(long uniqueIDOfAttachableToRemove))&MOSRotating::RemoveAttachable)
			.def_readonly("Attachables", &MOSRotating::m_Attachables, return_stl_iterator)
			.def_readonly("Wounds", &MOSRotating::m_Wounds, return_stl_iterator),

        CONCRETELUABINDING(Attachable, MOSRotating)
            .def("IsAttached", &Attachable::IsAttached)
            .def("IsAttachedTo", &Attachable::IsAttachedTo)
            .def("RemoveFromParent", &RemoveAttachableFromParentLuaSafe1)
            .def("RemoveFromParent", &RemoveAttachableFromParentLuaSafe2)
			.property("ParentOffset", &Attachable::GetParentOffset, &Attachable::SetParentOffset)
            .def("IsDrawnAfterParent", &Attachable::IsDrawnAfterParent)
            .property("JointStrength", &Attachable::GetJointStrength, &Attachable::SetJointStrength)
            .property("JointStiffness", &Attachable::GetJointStiffness, &Attachable::SetJointStiffness)
            .property("JointOffset", &Attachable::GetJointOffset, &Attachable::SetJointOffset)
            .property("ApplyTransferredForcesAtOffset", &Attachable::GetApplyTransferredForcesAtOffset, &Attachable::SetApplyTransferredForcesAtOffset)
            .property("BreakWound", &Attachable::GetBreakWound, &AttachableSetBreakWound)
            .property("ParentBreakWound", &Attachable::GetParentBreakWound, &AttachableSetParentBreakWound)
            .property("InheritsHFlipped", &Attachable::InheritsHFlipped, &Attachable::SetInheritsHFlipped)
			.property("InheritsRotAngle", &Attachable::InheritsRotAngle, &Attachable::SetInheritsRotAngle)
            .property("InheritedRotAngleOffset", &Attachable::GetInheritedRotAngleOffset, &Attachable::SetInheritedRotAngleOffset)
            .property("AtomSubgroupID", &Attachable::GetAtomSubgroupID)
			.property("CollidesWithTerrainWhileAttached", &Attachable::GetCollidesWithTerrainWhileAttached, &Attachable::SetCollidesWithTerrainWhileAttached)
            .property("CanCollideWithTerrain", &Attachable::CanCollideWithTerrain),

		ABSTRACTLUABINDING(Emission, Entity)
			.property("ParticlesPerMinute", &Emission::GetRate, &Emission::SetRate)
			.property("MinVelocity", &Emission::GetMinVelocity, &Emission::SetMinVelocity)
			.property("MaxVelocity", &Emission::GetMaxVelocity, &Emission::SetMaxVelocity)
			.property("PushesEmitter", &Emission::PushesEmitter, &Emission::SetPushesEmitter)
			.property("LifeVariation", &Emission::GetLifeVariation, &Emission::SetLifeVariation)
			.property("BurstSize", &Emission::GetBurstSize, &Emission::SetBurstSize)
			.property("Spread", &Emission::GetSpread, &Emission::SetSpread)
			.property("Offset", &Emission::GetOffset, &Emission::SetOffset)
			.def("ResetEmissionTimers", &Emission::ResetEmissionTimers),

        CONCRETELUABINDING(AEmitter, Attachable)
            .def("IsEmitting", &AEmitter::IsEmitting)
            .def("EnableEmission", &AEmitter::EnableEmission)
			.property("EmissionSound", &AEmitter::GetEmissionSound, &AEmitterSetEmissionSound)
			.property("BurstSound", &AEmitter::GetBurstSound, &AEmitterSetBurstSound)
			.property("EndSound", &AEmitter::GetEndSound, &AEmitterSetEndSound)
            .property("BurstScale", &AEmitter::GetBurstScale, &AEmitter::SetBurstScale)
            .property("EmitAngle", &AEmitter::GetEmitAngle, &AEmitter::SetEmitAngle)
            .property("GetThrottle", &AEmitter::GetThrottle, &AEmitter::SetThrottle)
            .property("Throttle", &AEmitter::GetThrottle, &AEmitter::SetThrottle)
            .property("BurstSpacing", &AEmitter::GetBurstSpacing, &AEmitter::SetBurstSpacing)
			.property("BurstDamage", &AEmitter::GetBurstDamage, &AEmitter::SetBurstDamage)
			.property("EmitterDamageMultiplier", &AEmitter::GetEmitterDamageMultiplier, &AEmitter::SetEmitterDamageMultiplier)
			.property("EmitCountLimit", &AEmitter::GetEmitCountLimit, &AEmitter::SetEmitCountLimit)
			.property("EmitDamage", &AEmitter::GetEmitDamage, &AEmitter::SetEmitDamage)
            .property("Flash", &AEmitter::GetFlash, &AEmitterSetFlash)
			.property("FlashScale", &AEmitter::GetFlashScale, &AEmitter::SetFlashScale)
			.def("GetEmitVector", &AEmitter::GetEmitVector)
            .def("GetRecoilVector", &AEmitter::GetRecoilVector)
            .def("EstimateImpulse", &AEmitter::EstimateImpulse)
            .def("TriggerBurst", &AEmitter::TriggerBurst)
            .def("IsSetToBurst", &AEmitter::IsSetToBurst)
            .def("CanTriggerBurst", &AEmitter::CanTriggerBurst)
			.def_readwrite("Emissions", &AEmitter::m_EmissionList, return_stl_iterator),

		CONCRETELUABINDING(PEmitter, MOSParticle)
			.def("IsEmitting", &PEmitter::IsEmitting)
			.def("EnableEmission", &PEmitter::EnableEmission)
			.property("BurstScale", &PEmitter::GetBurstScale, &PEmitter::SetBurstScale)
			.property("EmitAngle", &PEmitter::GetEmitAngle, &PEmitter::SetEmitAngle)
			.property("GetThrottle", &PEmitter::GetThrottle, &PEmitter::SetThrottle)
			.property("Throttle", &PEmitter::GetThrottle, &PEmitter::SetThrottle)
			.property("BurstSpacing", &PEmitter::GetBurstSpacing, &PEmitter::SetBurstSpacing)
			.property("EmitCountLimit", &PEmitter::GetEmitCountLimit, &PEmitter::SetEmitCountLimit)
			.property("FlashScale", &PEmitter::GetFlashScale, &PEmitter::SetFlashScale)
			.def("GetEmitVector", &PEmitter::GetEmitVector)
			.def("GetRecoilVector", &PEmitter::GetRecoilVector)
			.def("EstimateImpulse", &PEmitter::EstimateImpulse)
			.def("TriggerBurst", &PEmitter::TriggerBurst)
			.def("IsSetToBurst", &PEmitter::IsSetToBurst)
			.def("CanTriggerBurst", &PEmitter::CanTriggerBurst)
			.def_readwrite("Emissions", &PEmitter::m_EmissionList, return_stl_iterator),

		CONCRETELUABINDING(Actor, MOSRotating)
			.enum_("Status")[
				value("STABLE", Actor::Status::STABLE),
				value("UNSTABLE", Actor::Status::UNSTABLE),
				value("INACTIVE", Actor::Status::INACTIVE),
				value("DYING", Actor::Status::DYING),
				value("DEAD", Actor::Status::DEAD)
			]
			.enum_("AIMode")[
				value("AIMODE_NONE", Actor::AIMode::AIMODE_NONE),
				value("AIMODE_SENTRY", Actor::AIMode::AIMODE_SENTRY),
				value("AIMODE_PATROL", Actor::AIMode::AIMODE_PATROL),
				value("AIMODE_GOTO", Actor::AIMode::AIMODE_GOTO),
				value("AIMODE_BRAINHUNT", Actor::AIMode::AIMODE_BRAINHUNT),
				value("AIMODE_GOLDDIG", Actor::AIMode::AIMODE_GOLDDIG),
				value("AIMODE_RETURN", Actor::AIMode::AIMODE_RETURN),
				value("AIMODE_STAY", Actor::AIMode::AIMODE_STAY),
				value("AIMODE_SCUTTLE", Actor::AIMode::AIMODE_SCUTTLE),
				value("AIMODE_DELIVER", Actor::AIMode::AIMODE_DELIVER),
				value("AIMODE_BOMB", Actor::AIMode::AIMODE_BOMB),
				value("AIMODE_SQUAD", Actor::AIMode::AIMODE_SQUAD),
				value("AIMODE_COUNT", Actor::AIMode::AIMODE_COUNT)
			]
			.enum_("ActionState")[
				value("MOVING", Actor::ActionState::MOVING),
				value("MOVING_FAST", Actor::ActionState::MOVING_FAST),
				value("FIRING", Actor::ActionState::FIRING),
				value("ActionStateCount", Actor::ActionState::ActionStateCount)
			]
			.enum_("AimState")[
				value("AIMSTILL", Actor::AimState::AIMSTILL),
				value("AIMUP", Actor::AimState::AIMUP),
				value("AIMDOWN", Actor::AimState::AIMDOWN),
				value("AimStateCount", Actor::AimState::AimStateCount)
			]
			.enum_("LateralMoveState")[
				value("LAT_STILL", Actor::LateralMoveState::LAT_STILL),
				value("LAT_LEFT", Actor::LateralMoveState::LAT_LEFT),
				value("LAT_RIGHT", Actor::LateralMoveState::LAT_RIGHT)
			]
			.enum_("ObstacleState")[
				value("PROCEEDING", Actor::ObstacleState::PROCEEDING),
				value("BACKSTEPPING", Actor::ObstacleState::BACKSTEPPING),
				value("DIGPAUSING", Actor::ObstacleState::DIGPAUSING),
				value("JUMPING", Actor::ObstacleState::JUMPING),
				value("SOFTLANDING", Actor::ObstacleState::SOFTLANDING)
			]
			.enum_("TeamBlockState")[
				value("NOTBLOCKED", Actor::TeamBlockState::NOTBLOCKED),
				value("BLOCKED", Actor::TeamBlockState::BLOCKED),
				value("IGNORINGBLOCK", Actor::TeamBlockState::IGNORINGBLOCK),
				value("FOLLOWWAIT", Actor::TeamBlockState::FOLLOWWAIT)
			]
            .def(constructor<>())
            .def("GetController", &Actor::GetController)
            .def("IsPlayerControlled", &Actor::IsPlayerControlled)
            .def("IsControllable", &Actor::IsControllable)
            .def("SetControllerMode", &Actor::SetControllerMode)
            .def("SwapControllerModes", &Actor::SwapControllerModes)
			.property("BodyHitSound", &Actor::GetBodyHitSound, &ActorSetBodyHitSound)
			.property("AlarmSound", &Actor::GetAlarmSound, &ActorSetAlarmSound)
			.property("PainSound", &Actor::GetPainSound, &ActorSetPainSound)
			.property("DeathSound", &Actor::GetDeathSound, &ActorSetDeathSound)
			.property("DeviceSwitchSound", &Actor::GetDeviceSwitchSound, &ActorSetDeviceSwitchSound)
			.property("ImpulseDamageThreshold", &Actor::GetTravelImpulseDamage, &Actor::SetTravelImpulseDamage)
			.def("GetStableVelocityThreshold", &Actor::GetStableVel)
			.def("SetStableVelocityThreshold", (void (Actor::*)(float, float))&Actor::SetStableVel)
			.def("SetStableVelocityThreshold", (void (Actor::*)(Vector))&Actor::SetStableVel)
            .property("Status", &Actor::GetStatus, &Actor::SetStatus)
            .property("Health", &Actor::GetHealth, &Actor::SetHealth)
            .property("PrevHealth", &Actor::GetPrevHealth)
            .property("MaxHealth", &Actor::GetMaxHealth, &Actor::SetMaxHealth)
            .property("InventoryMass", &Actor::GetInventoryMass)
            .property("GoldCarried", &Actor::GetGoldCarried, &Actor::SetGoldCarried)
            .property("AimRange", &Actor::GetAimRange, &Actor::SetAimRange)
            .def("GetAimAngle", &Actor::GetAimAngle)
            .def("SetAimAngle", &Actor::SetAimAngle)
            .def("HasObject", &Actor::HasObject)
            .def("HasObjectInGroup", &Actor::HasObjectInGroup)
            .property("CPUPos", &Actor::GetCPUPos)
            .property("EyePos", &Actor::GetEyePos)
            .property("HolsterOffset", &Actor::GetHolsterOffset, &Actor::SetHolsterOffset)
            .property("ViewPoint", &Actor::GetViewPoint, &Actor::SetViewPoint)
            .property("ItemInReach", &Actor::GetItemInReach, &Actor::SetItemInReach)
            .property("Height", &Actor::GetHeight)
            .def("IsWithinRange", &Actor::IsWithinRange)
            .def("AddHealth", &Actor::AddHealth)
            .def("IsStatus", &Actor::IsStatus)
            .def("IsDead", &Actor::IsDead)
            .property("AIMode", &Actor::GetAIMode, &Actor::SetAIMode)
            .property("DeploymentID", &Actor::GetDeploymentID)
            .property("PassengerSlots", &Actor::GetPassengerSlots, &Actor::SetPassengerSlots)
			.def("AddAISceneWaypoint", &Actor::AddAISceneWaypoint)
            .def("AddAIMOWaypoint", &Actor::AddAIMOWaypoint)
            .def("ClearAIWaypoints", &Actor::ClearAIWaypoints)
            .def("GetLastAIWaypoint", &Actor::GetLastAIWaypoint)
            .def("GetAIMOWaypointID", &Actor::GetAIMOWaypointID)
            .def("GetWaypointListSize", &Actor::GetWaypointsSize)
            .def("ClearMovePath", &Actor::ClearMovePath)
            .def("AddToMovePathBeginning", &Actor::AddToMovePathBeginning)
            .def("AddToMovePathEnd", &Actor::AddToMovePathEnd)
            .def("RemoveMovePathBeginning", &Actor::RemoveMovePathBeginning)
            .def("RemoveMovePathEnd", &Actor::RemoveMovePathEnd)
            .property("Perceptiveness", &Actor::GetPerceptiveness, &Actor::SetPerceptiveness)
			.property("CanRevealUnseen", &Actor::GetCanRevealUnseen, &Actor::SetCanRevealUnseen)
            .def("AddInventoryItem", &Actor::AddInventoryItem, adopt(_2))
            .def("RemoveInventoryItem", &Actor::RemoveInventoryItem)
            .def("SwapNextInventory", &Actor::SwapNextInventory)
            .def("SwapPrevInventory", &Actor::SwapPrevInventory)
            .def("DropAllInventory", &Actor::DropAllInventory)
            .property("InventorySize", &Actor::GetInventorySize)
            .def("IsInventoryEmpty", &Actor::IsInventoryEmpty)
            .property("MaxInventoryMass", &Actor::GetMaxInventoryMass)
            .def("FlashWhite", &Actor::FlashWhite)
            .def("DrawWaypoints", &Actor::DrawWaypoints)
            .def("SetMovePathToUpdate", &Actor::SetMovePathToUpdate)
            .def("UpdateMovePath", &Actor::UpdateMovePath)
            .property("MovePathSize", &Actor::GetMovePathSize)
            .def_readwrite("MOMoveTarget", &Actor::m_pMOMoveTarget)
            .def_readwrite("MovePath", &Actor::m_MovePath, return_stl_iterator)
            .def_readwrite("Inventory", &Actor::m_Inventory, return_stl_iterator)
            .def("SetAlarmPoint", &Actor::AlarmPoint)
            .def("GetAlarmPoint", &Actor::GetAlarmPoint)
            .property("AimDistance", &Actor::GetAimDistance, &Actor::SetAimDistance)
			.property("SightDistance", &Actor::GetSightDistance, &Actor::SetSightDistance),

		CONCRETELUABINDING(ADoor, Actor)
			.enum_("DoorState")[
				value("CLOSED", ADoor::DoorState::CLOSED),
				value("OPENING", ADoor::DoorState::OPENING),
				value("OPEN", ADoor::DoorState::OPEN),
				value("CLOSING", ADoor::DoorState::CLOSING),
				value("STOPPED", ADoor::DoorState::STOPPED)
			]
            .property("Door", &ADoor::GetDoor, &ADoorSetDoor)
			.def("GetDoorState", &ADoor::GetDoorState)
			.def("OpenDoor", &ADoor::OpenDoor)
			.def("CloseDoor", &ADoor::CloseDoor)
			.def("StopDoor", &ADoor::StopDoor)
			.def("SetClosedByDefault", &ADoor::SetClosedByDefault)
			.property("DoorMoveStartSound", &ADoor::GetDoorMoveStartSound, &ADoorSetDoorMoveStartSound)
			.property("DoorMoveSound", &ADoor::GetDoorMoveSound, &ADoorSetDoorMoveSound)
			.property("DoorDirectionChangeSound", &ADoor::GetDoorDirectionChangeSound, &ADoorSetDoorDirectionChangeSound)
			.property("DoorMoveEndSound", &ADoor::GetDoorMoveEndSound, &ADoorSetDoorMoveEndSound),

		CONCRETELUABINDING(Arm, Attachable)
            .property("HeldDevice", &Arm::GetHeldMO)
			.property("MaxLength", &Arm::GetMaxLength)
			.property("IdleOffset", &Arm::GetIdleOffset, &Arm::SetIdleOffset)
            .property("GripStrength", &Arm::GetGripStrength, &Arm::SetGripStrength)
			.property("HandPos", &Arm::GetHandPos, &Arm::SetHandPos),

        CONCRETELUABINDING(Leg, Attachable)
            .property("Foot", &Leg::GetFoot, &LegSetFoot),

		CONCRETELUABINDING(AHuman, Actor)
			// These are all private/protected so they can't be bound, need to consider making them public.
			.enum_("UpperBodyState")[
				value("WEAPON_READY", 0 /*AHuman::UpperBodyState::WEAPON_READY*/),
				value("AIMING_SHARP", 1 /*AHuman::UpperBodyState::AIMING_SHARP*/),
				value("HOLSTERING_BACK", 2 /*AHuman::UpperBodyState::HOLSTERING_BACK*/),
				value("HOLSTERING_BELT", 3 /*AHuman::UpperBodyState::HOLSTERING_BELT*/),
				value("DEHOLSTERING_BACK", 4 /*AHuman::UpperBodyState::DEHOLSTERING_BACK*/),
				value("DEHOLSTERING_BELT", 5 /*AHuman::UpperBodyState::DEHOLSTERING_BELT*/),
				value("THROWING_PREP", 6 /*AHuman::UpperBodyState::THROWING_PREP*/),
				value("THROWING_RELEASE", 7 /*AHuman::UpperBodyState::THROWING_RELEASE*/)
			]
			.enum_("MovementState")[
				value("NOMOVE", 0 /*AHuman::MovementState::NOMOVE*/),
				value("STAND", 1 /*AHuman::MovementState::STAND*/),
				value("WALK", 2 /*AHuman::MovementState::WALK*/),
				value("CROUCH", 3 /*AHuman::MovementState::CROUCH*/),
				value("CRAWL", 4 /*AHuman::MovementState::CRAWL*/),
				value("ARMCRAWL", 5 /*AHuman::MovementState::ARMCRAWL*/),
				value("CLIMB", 6 /*AHuman::MovementState::CLIMB*/),
				value("JUMP", 7 /*AHuman::MovementState::JUMP*/),
				value("DISLODGE", 8 /*AHuman::MovementState::DISLODGE*/),
				value("MOVEMENTSTATECOUNT", 9 /*AHuman::MovementState::MOVEMENTSTATECOUNT*/)
			]
			.enum_("ProneState")[
				value("NOTPRONE", 0 /*AHuman::ProneState::NOTPRONE*/),
				value("GOPRONE", 1 /*AHuman::ProneState::GOPRONE*/),
				value("PRONE", 2 /*AHuman::ProneState::PRONE*/),
				value("PRONESTATECOUNT", 3 /*AHuman::ProneState::PRONESTATECOUNT*/)
			]		
			.enum_("Layer")[
				// Doesn't have qualifier
				value("FGROUND", 0 /*AHuman::FGROUND*/),
				value("BGROUND", 1 /*AHuman::BGROUND*/)
			]
			.enum_("DeviceHandlingState")[
				value("STILL", 0 /*AHuman::DeviceHandlingState::STILL*/),
				value("POINTING", 1 /*AHuman::DeviceHandlingState::POINTING*/),
				value("SCANNING", 2 /*AHuman::DeviceHandlingState::SCANNING*/),
				value("AIMING", 3 /*AHuman::DeviceHandlingState::AIMING*/),
				value("FIRING", 4 /*AHuman::DeviceHandlingState::FIRING*/),
				value("THROWING", 5 /*AHuman::DeviceHandlingState::THROWING*/),
				value("DIGGING", 6 /*AHuman::DeviceHandlingState::DIGGING*/)
			]
			.enum_("SweepState")[
				value("NOSWEEP", 0 /*AHuman::SweepState::NOSWEEP*/),
				value("SWEEPINGUP", 1 /*AHuman::SweepState::SWEEPINGUP*/),
				value("SWEEPUPPAUSE", 2 /*AHuman::SweepState::SWEEPUPPAUSE*/),
				value("SWEEPINGDOWN", 3 /*AHuman::SweepState::SWEEPINGDOWN*/),
				value("SWEEPDOWNPAUSE", 4 /*AHuman::SweepState::SWEEPDOWNPAUSE*/)
			]
			.enum_("DigState")[
				value("NOTDIGGING", 0 /*AHuman::DigState::NOTDIGGING*/),
				value("PREDIG", 1 /*AHuman::DigState::PREDIG*/),
				value("STARTDIG", 2 /*AHuman::DigState::STARTDIG*/),
				value("TUNNELING", 3 /*AHuman::DigState::TUNNELING*/),
				value("FINISHINGDIG", 4 /*AHuman::DigState::FINISHINGDIG*/),
				value("PAUSEDIGGER", 5 /*AHuman::DigState::PAUSEDIGGER*/)
			]
			.enum_("JumpState")[
				value("NOTJUMPING", 0 /*AHuman::JumpState::NOTJUMPING*/),
				value("FORWARDJUMP", 1 /*AHuman::JumpState::FORWARDJUMP*/),
				value("PREJUMP", 2 /*AHuman::JumpState::PREJUMP*/),
				value("UPJUMP", 3 /*AHuman::JumpState::UPJUMP*/),
				value("APEXJUMP", 4 /*AHuman::JumpState::APEXJUMP*/),
				value("LANDJUMP", 5 /*AHuman::JumpState::LANDJUMP*/)
			]
            .def(constructor<>())
            .property("Head", &AHuman::GetHead, &AHumanSetHead)
            .property("Jetpack", &AHuman::GetJetpack, &AHumanSetJetpack)
            .property("FGArm", &AHuman::GetFGArm, &AHumanSetFGArm)
            .property("BGArm", &AHuman::GetBGArm, &AHumanSetBGArm)
            .property("FGLeg", &AHuman::GetFGLeg, &AHumanSetFGLeg)
            .property("BGLeg", &AHuman::GetBGLeg, &AHumanSetBGLeg)
			.property("FGFoot", &AHuman::GetFGFoot, &AHumanSetFGFoot)
			.property("BGFoot", &AHuman::GetBGFoot, &AHumanSetBGFoot)
			.property("StrideSound", &AHuman::GetStrideSound, &AHumanSetStrideSound)
			.property("JetTimeTotal", &AHuman::GetJetTimeTotal, &AHuman::SetJetTimeTotal)
            .property("JetTimeLeft", &AHuman::GetJetTimeLeft, &AHuman::SetJetTimeLeft)
			.property("JetAngleRange", &AHuman::GetJetAngleRange, &AHuman::SetJetAngleRange)
			.property("ThrowPrepTime", &AHuman::GetThrowPrepTime, &AHuman::SetThrowPrepTime)
            .def("EquipFirearm", &AHuman::EquipFirearm)
            .def("EquipThrowable", &AHuman::EquipThrowable)
            .def("EquipDiggingTool", &AHuman::EquipDiggingTool)
            .def("EquipShield", &AHuman::EquipShield)
            .def("EquipShieldInBGArm", &AHuman::EquipShieldInBGArm)
            .def("EquipDeviceInGroup", &AHuman::EquipDeviceInGroup)
            .def("EquipNamedDevice", &AHuman::EquipNamedDevice)
            .def("EquipLoadedFirearmInGroup", &AHuman::EquipLoadedFirearmInGroup)
            .def("UnequipBGArm", &AHuman::UnequipBGArm)
            .property("EquippedItem", &AHuman::GetEquippedItem)
            .property("EquippedBGItem", &AHuman::GetEquippedBGItem)
            .property("FirearmIsReady", &AHuman::FirearmIsReady)
            .property("ThrowableIsReady", &AHuman::ThrowableIsReady)
            .property("FirearmIsEmpty", &AHuman::FirearmIsEmpty)
            .property("FirearmNeedsReload", &AHuman::FirearmNeedsReload)
            .property("FirearmIsSemiAuto", &AHuman::FirearmIsSemiAuto)
            .property("FirearmActivationDelay", &AHuman::FirearmActivationDelay)
            .def("ReloadFirearms", &AHuman::ReloadFirearms)
            .def("IsWithinRange", &AHuman::IsWithinRange)
            .def("Look", &AHuman::Look)
            .def("LookForGold", &AHuman::LookForGold)
            .def("LookForMOs", &AHuman::LookForMOs)
            .def("IsOnScenePoint", &AHuman::IsOnScenePoint)
            .def("GetLimbPath", &AHuman::GetLimbPath)
			.property("LimbPathPushForce", &AHuman::GetLimbPathPushForce, &AHuman::SetLimbPathPushForce)
			.def("GetLimbPathSpeed", &AHuman::GetLimbPathSpeed)
			.def("SetLimbPathSpeed", &AHuman::SetLimbPathSpeed)
			.def("GetRotAngleTarget", &AHuman::GetRotAngleTarget)
			.def("SetRotAngleTarget", &AHuman::SetRotAngleTarget),
        
		CONCRETELUABINDING(ACrab, Actor)
			// These are all private/protected so they can't be bound, need to consider making them public.
			.enum_("MovementState")[
				value("STAND", 0 /*ACrab::MovementState::STAND*/),
				value("WALK", 1 /*ACrab::MovementState::WALK*/),
				value("JUMP", 2 /*ACrab::MovementState::JUMP*/),
				value("DISLODGE", 3 /*ACrab::MovementState::DISLODGE*/),
				value("MOVEMENTSTATECOUNT", 4 /*ACrab::MovementState::MOVEMENTSTATECOUNT*/)
			]
			.enum_("Side")[
				// Doesn't have qualifier
				value("LEFTSIDE", 0 /*ACrab::LEFTSIDE*/),
				value("RIGHTSIDE", 1 /*ACrab::RIGHTSIDE*/),
				value("SIDECOUNT", 2 /*ACrab::SIDECOUNT*/)
			]
			.enum_("Layer")[
				// Doesn't have qualifier
				value("FGROUND", 0 /*ACrab::FGROUND*/),
				value("BGROUND", 1 /*ACrab::BGROUND*/)
			]
			.enum_("DeviceHandlingState")[
				value("STILL", 0 /*ACrab::DeviceHandlingState::STILL*/),
				value("POINTING", 1 /*ACrab::DeviceHandlingState::POINTING*/),
				value("SCANNING", 2 /*ACrab::DeviceHandlingState::SCANNING*/),
				value("AIMING", 3 /*ACrab::DeviceHandlingState::AIMING*/),
				value("FIRING", 4 /*ACrab::DeviceHandlingState::FIRING*/),
				value("THROWING", 5 /*ACrab::DeviceHandlingState::THROWING*/),
				value("DIGGING", 6 /*ACrab::DeviceHandlingState::DIGGING*/)
			]
			.enum_("SweepState")[
				value("NOSWEEP", 0 /*ACrab::SweepState::NOSWEEP*/),
				value("SWEEPINGUP", 1 /*ACrab::SweepState::SWEEPINGUP*/),
				value("SWEEPUPPAUSE", 2 /*ACrab::SweepState::SWEEPUPPAUSE*/),
				value("SWEEPINGDOWN", 3 /*ACrab::SweepState::SWEEPINGDOWN*/),
				value("SWEEPDOWNPAUSE", 4 /*ACrab::SweepState::SWEEPDOWNPAUSE*/)
			]
			.enum_("DigState")[
				value("NOTDIGGING", 0 /*ACrab::DigState::NOTDIGGING*/),
				value("PREDIG", 1 /*ACrab::DigState::PREDIG*/),
				value("STARTDIG", 2 /*ACrab::DigState::STARTDIG*/),
				value("TUNNELING", 3 /*ACrab::DigState::TUNNELING*/),
				value("FINISHINGDIG", 4 /*ACrab::DigState::FINISHINGDIG*/),
				value("PAUSEDIGGER", 5 /*ACrab::DigState::PAUSEDIGGER*/)
			]
			.enum_("JumpState")[
				value("NOTJUMPING", 0 /*ACrab::JumpState::NOTJUMPING*/),
				value("FORWARDJUMP", 1 /*ACrab::JumpState::FORWARDJUMP*/),
				value("PREJUMP", 2 /*ACrab::JumpState::PREJUMP*/),
				value("UPJUMP", 3 /*ACrab::JumpState::UPJUMP*/),
				value("APEXJUMP", 4 /*ACrab::JumpState::APEXJUMP*/),
				value("LANDJUMP", 5 /*ACrab::JumpState::LANDJUMP*/)
			]
            .def(constructor<>())
            .property("Turret", &ACrab::GetTurret, &ACrabSetTurret)
            .property("Jetpack", &ACrab::GetJetpack, &ACrabSetJetpack)
            .property("LeftFGLeg", &ACrab::GetLeftFGLeg, &ACrabSetLeftFGLeg)
            .property("LeftBGLeg", &ACrab::GetLeftBGLeg, &ACrabSetLeftBGLeg)
            .property("RightFGLeg", &ACrab::GetRightFGLeg, &ACrabSetRightFGLeg)
            .property("RightBGLeg", &ACrab::GetRightBGLeg, &ACrabSetRightBGLeg)
			.property("StrideSound", &ACrab::GetStrideSound, &ACrabSetStrideSound)
			.property("JetTimeTotal", &ACrab::GetJetTimeTotal, &ACrab::SetJetTimeTotal)
            .property("JetTimeLeft", &ACrab::GetJetTimeLeft)
            .property("EquippedItem", &ACrab::GetEquippedItem)
            .property("FirearmIsReady", &ACrab::FirearmIsReady)
            .property("FirearmIsEmpty", &ACrab::FirearmIsEmpty)
            .property("FirearmNeedsReload", &ACrab::FirearmNeedsReload)
            .property("FirearmIsSemiAuto", &ACrab::FirearmIsSemiAuto)
            .property("FirearmActivationDelay", &ACrab::FirearmActivationDelay)
            .def("ReloadFirearm", &ACrab::ReloadFirearm)
            .def("IsWithinRange", &ACrab::IsWithinRange)
            .def("Look", &ACrab::Look)
            .def("LookForMOs", &ACrab::LookForMOs)
            .def("IsOnScenePoint", &ACrab::IsOnScenePoint)
            .def("GetLimbPath", &ACrab::GetLimbPath)
			.property("LimbPathPushForce", &ACrab::GetLimbPathPushForce, &ACrab::SetLimbPathPushForce)
			.def("GetLimbPathSpeed", &ACrab::GetLimbPathSpeed)
			.def("SetLimbPathSpeed", &ACrab::SetLimbPathSpeed),

        CONCRETELUABINDING(Turret, Attachable)
			.property("MountedDevice", &Turret::GetMountedDevice, &TurretSetMountedDevice),

		ABSTRACTLUABINDING(ACraft, Actor)
			.enum_("HatchState")[
				value("CLOSED", ACraft::HatchState::CLOSED),
				value("OPENING", ACraft::HatchState::OPENING),
				value("OPEN", ACraft::HatchState::OPEN),
				value("CLOSING", ACraft::HatchState::CLOSING),
				value("HatchStateCount", ACraft::HatchState::HatchStateCount)
			]
			.enum_("Side")[
				// Doesn't have qualifier
				value("RIGHT", ACraft::RIGHT),
				value("LEFT", ACraft::LEFT)
			]
			// These are all private/protected so they can't be bound, need to consider making them public.
			.enum_("CraftDeliverySequence")[
				value("FALL", 0 /*ACraft::CraftDeliverySequence::FALL*/),
				value("LAND", 1 /*ACraft::CraftDeliverySequence::LAND*/),
				value("STANDBY", 2 /*ACraft::CraftDeliverySequence::STANDBY*/),
				value("UNLOAD", 3 /*ACraft::CraftDeliverySequence::UNLOAD*/),
				value("LAUNCH", 4 /*ACraft::CraftDeliverySequence::LAUNCH*/),
				value("UNSTICK", 5 /*ACraft::CraftDeliverySequence::UNSTICK*/)
			]
			.enum_("AltitudeMoveState")[
				value("HOVER", 0 /*ACraft::AltitudeMoveState::HOVER*/),
				value("DESCEND", 1 /*ACraft::AltitudeMoveState::DESCEND*/),
				value("ASCEND", 2 /*ACraft::AltitudeMoveState::ASCEND*/)
			]
            .def("OpenHatch", &ACraft::OpenHatch)
            .def("CloseHatch", &ACraft::CloseHatch)
            .property("HatchState", &ACraft::GetHatchState)
			.property("HatchOpenSound", &ACraft::GetHatchOpenSound, &ACraftSetHatchOpenSound)
			.property("HatchCloseSound", &ACraft::GetHatchCloseSound, &ACraftSetHatchCloseSound)
			.property("CrashSound", &ACraft::GetCrashSound, &ACraftSetCrashSound)
            .property("MaxPassengers", &ACraft::GetMaxPassengers)
            .property("DeliveryDelayMultiplier", &ACraft::GetDeliveryDelayMultiplier),

        CONCRETELUABINDING(ACDropShip, ACraft)
            .property("RightEngine", &ACDropShip::GetRightThruster, &ACDropShipSetRightThruster)
            .property("LeftEngine", &ACDropShip::GetLeftThruster, &ACDropShipSetLeftThruster)
            .property("RightThruster", &ACDropShip::GetURightThruster, &ACDropShipSetURightThruster)
            .property("LeftThruster", &ACDropShip::GetULeftThruster, &ACDropShipSetULeftThruster)
			.property("RightHatch", &ACDropShip::GetRightHatch, &ACDropShipSetRightHatch)
			.property("LeftHatch", &ACDropShip::GetLeftHatch, &ACDropShipSetLeftHatch)
			.property("MaxEngineAngle", &ACDropShip::GetMaxEngineAngle, &ACDropShip::SetMaxEngineAngle)
			.property("LateralControlSpeed", &ACDropShip::GetLateralControlSpeed, &ACDropShip::SetLateralControlSpeed)
			.property("LateralControl", &ACDropShip::GetLateralControl)
			.def("DetectObstacle", &ACDropShip::DetectObstacle)
            .def("GetAltitude", &ACDropShip::GetAltitude),

		CONCRETELUABINDING(ACRocket, ACraft)
			// These are all private/protected so they can't be bound, need to consider making them public.
			.enum_("LandingGearState")[
				value("RAISED", 0 /*ACRocket::LandingGearState::RAISED*/),
				value("LOWERED", 1 /*ACRocket::LandingGearState::LOWERED*/),
				value("LOWERING", 2 /*ACRocket::LandingGearState::LOWERING*/),
				value("RAISING", 3 /*ACRocket::LandingGearState::RAISING*/),
				value("GearStateCount", 4 /*ACRocket::LandingGearState::GearStateCount*/)
			]
            .property("RightLeg", &ACRocket::GetRightLeg, &ACRocketSetRightLeg)
            .property("LeftLeg", &ACRocket::GetLeftLeg, &ACRocketSetLeftLeg)
			.property("MainEngine", &ACRocket::GetMainThruster, &ACRocketSetMainThruster)
			.property("LeftEngine", &ACRocket::GetLeftThruster, &ACRocketSetLeftThruster)
			.property("RightEngine", &ACRocket::GetRightThruster, &ACRocketSetRightThruster)
			.property("LeftThruster", &ACRocket::GetULeftThruster, &ACRocketSetULeftThruster)
			.property("RightThruster", &ACRocket::GetURightThruster, &ACRocketSetURightThruster)
			.property("GearState", &ACRocket::GetGearState),

        CONCRETELUABINDING(HeldDevice, Attachable)
            .property("SupportPos", &HeldDevice::GetSupportPos)
            .property("MagazinePos", &HeldDevice::GetMagazinePos)
            .property("MuzzlePos", &HeldDevice::GetMuzzlePos)
            .property("MuzzleOffset", &HeldDevice::GetMuzzleOffset, &HeldDevice::SetMuzzleOffset)
            .property("StanceOffset", &HeldDevice::GetStanceOffset, &HeldDevice::SetStanceOffset)
            .property("SharpStanceOffset", &HeldDevice::GetSharpStanceOffset, &HeldDevice::SetSharpStanceOffset)
            .property("SharpLength", &HeldDevice::GetSharpLength, &HeldDevice::SetSharpLength)
            .def("IsWeapon", &HeldDevice::IsWeapon)
            .def("IsTool", &HeldDevice::IsTool)
            .def("IsShield", &HeldDevice::IsShield)
            .def("IsDualWieldable", &HeldDevice::IsDualWieldable)
            .def("SetDualWieldable", &HeldDevice::SetDualWieldable)
            .def("IsOneHanded", &HeldDevice::IsOneHanded)
            .def("SetOneHanded", &HeldDevice::SetOneHanded)
            .def("Activate", &HeldDevice::Activate)
            .def("Deactivate", &HeldDevice::Deactivate)
            .def("Reload", &HeldDevice::Reload)
            .def("IsActivated", &HeldDevice::IsActivated)
            .def("IsReloading", &HeldDevice::IsReloading)
            .def("DoneReloading", &HeldDevice::DoneReloading)
            .def("NeedsReloading", &HeldDevice::NeedsReloading)
            .def("IsFull", &HeldDevice::IsFull)
			.property("SharpLength", &HeldDevice::GetSharpLength, &HeldDevice::SetSharpLength)
			.property("SupportOffset", &HeldDevice::GetSupportOffset, &HeldDevice::SetSupportOffset)
            .property("HasPickupLimitations", &HeldDevice::HasPickupLimitations)
            .property("UnPickupable", &HeldDevice::IsUnPickupable, &HeldDevice::SetUnPickupable)
            .def("IsPickupableBy", &HeldDevice::IsPickupableBy)
            .def("AddPickupableByPresetName", &HeldDevice::AddPickupableByPresetName)
            .def("RemovePickupableByPresetName", &HeldDevice::RemovePickupableByPresetName)
            .property("GripStrengthMultiplier", &HeldDevice::GetGripStrengthMultiplier, &HeldDevice::SetGripStrengthMultiplier)
			.def("SetSupported", &HeldDevice::SetSupported),

        CONCRETELUABINDING(Magazine, Attachable)
            .property("NextRound", &Magazine::GetNextRound)
            .property("RoundCount", &Magazine::GetRoundCount, &Magazine::SetRoundCount)
            .property("IsEmpty", &Magazine::IsEmpty)
            .property("IsFull", &Magazine::IsFull)
            .property("IsOverHalfFull", &Magazine::IsOverHalfFull)
            .property("Capacity", &Magazine::GetCapacity)
            .property("Discardable", &Magazine::IsDiscardable),

        CONCRETELUABINDING(Round, Entity)
            .property("NextParticle", &Round::GetNextParticle)
            .property("Shell", &Round::GetShell)
            .property("FireVel", &Round::GetFireVel)
            .property("ShellVel", &Round::GetShellVel)
            .property("Separation", &Round::GetSeparation)
            .property("ParticleCount", &Round::ParticleCount)
            .property("AILifeTime", &Round::GetAILifeTime)
            .property("AIFireVel", &Round::GetAIFireVel)
            .property("IsEmpty", &Round::IsEmpty),

        CONCRETELUABINDING(HDFirearm, HeldDevice)
            .property("RateOfFire", &HDFirearm::GetRateOfFire, &HDFirearm::SetRateOfFire)
			.property("FullAuto", &HDFirearm::IsFullAuto, &HDFirearm::SetFullAuto)
            .property("RoundInMagCount", &HDFirearm::GetRoundInMagCount)
            .property("Magazine", &HDFirearm::GetMagazine, &HDFirearmSetMagazine)
            .property("Flash", &HDFirearm::GetFlash, &HDFirearmSetFlash)
			.property("PreFireSound", &HDFirearm::GetPreFireSound, &HDFirearmSetPreFireSound)
			.property("FireSound", &HDFirearm::GetFireSound, &HDFirearmSetFireSound)
			.property("FireEchoSound", &HDFirearm::GetFireEchoSound, &HDFirearmSetFireEchoSound)
			.property("ActiveSound", &HDFirearm::GetActiveSound, &HDFirearmSetActiveSound)
			.property("DeactivationSound", &HDFirearm::GetDeactivationSound, &HDFirearmSetDeactivationSound)
			.property("EmptySound", &HDFirearm::GetEmptySound, &HDFirearmSetEmptySound)
			.property("ReloadStartSound", &HDFirearm::GetReloadStartSound, &HDFirearmSetReloadStartSound)
			.property("ReloadEndSound", &HDFirearm::GetReloadEndSound, &HDFirearmSetReloadEndSound)
            .property("ActivationDelay", &HDFirearm::GetActivationDelay, &HDFirearm::SetActivationDelay)
            .property("DeactivationDelay", &HDFirearm::GetDeactivationDelay, &HDFirearm::SetDeactivationDelay)
            .property("ReloadTime", &HDFirearm::GetReloadTime, &HDFirearm::SetReloadTime)
            .property("ShakeRange", &HDFirearm::GetShakeRange, &HDFirearm::SetShakeRange)
            .property("SharpShakeRange", &HDFirearm::GetSharpShakeRange, &HDFirearm::SetSharpShakeRange)
            .property("NoSupportFactor", &HDFirearm::GetNoSupportFactor, &HDFirearm::SetNoSupportFactor)
            .property("ParticleSpreadRange", &HDFirearm::GetParticleSpreadRange, &HDFirearm::SetParticleSpreadRange)
			.property("FiredOnce", &HDFirearm::FiredOnce)
			.property("FiredFrame", &HDFirearm::FiredFrame)
			.property("RoundsFired", &HDFirearm::RoundsFired)
            .def("GetAIFireVel", &HDFirearm::GetAIFireVel)
            .def("GetAIBulletLifeTime", &HDFirearm::GetAIBulletLifeTime)
            .def("GetBulletAccScalar", &HDFirearm::GetBulletAccScalar)
            .def("GetAIBlastRadius", &HDFirearm::GetAIBlastRadius)
            .def("GetAIPenetration", &HDFirearm::GetAIPenetration)
            .def("CompareTrajectories", &HDFirearm::CompareTrajectories)
            .def("SetNextMagazineName", &HDFirearm::SetNextMagazineName)
			.property("IsAnimatedManually", &HDFirearm::IsAnimatedManually, &HDFirearm::SetAnimatedManually)
			.property("RecoilTransmission", &HDFirearm::GetJointStiffness, &HDFirearm::SetJointStiffness),

        CONCRETELUABINDING(ThrownDevice, HeldDevice)
            .property("MinThrowVel", &ThrownDevice::GetMinThrowVel, &ThrownDevice::SetMinThrowVel)
            .property("MaxThrowVel", &ThrownDevice::GetMaxThrowVel, &ThrownDevice::SetMaxThrowVel),

        CONCRETELUABINDING(TDExplosive, ThrownDevice)
            .property("IsAnimatedManually", &TDExplosive::IsAnimatedManually, &TDExplosive::SetAnimatedManually),

		CONCRETELUABINDING(Scene, Entity)
			.enum_("PlacedObjectSets")[
				value("PLACEONLOAD", Scene::PlacedObjectSets::PLACEONLOAD),
				value("BLUEPRINT", Scene::PlacedObjectSets::BLUEPRINT),
				value("AIPLAN", Scene::PlacedObjectSets::AIPLAN),
				value("PLACEDSETSCOUNT", Scene::PlacedObjectSets::PLACEDSETSCOUNT)
			]
            .property("Location", &Scene::GetLocation, &Scene::SetLocation)
//            .property("Terrain", &Scene::GetTerrain)
            .property("Dimensions", &Scene::GetDimensions)
            .property("Width", &Scene::GetWidth)
            .property("Height", &Scene::GetHeight)
            .property("WrapsX", &Scene::WrapsX)
            .property("WrapsY", &Scene::WrapsY)
            .property("TeamOwnership", &Scene::GetTeamOwnership, &Scene::SetTeamOwnership)
            .def("GetBuildBudget", &Scene::GetBuildBudget)
            .def("SetBuildBudget", &Scene::SetBuildBudget)
            .def("IsScanScheduled", &Scene::IsScanScheduled)
            .def("SetScheduledScan", &Scene::SetScheduledScan)
            .def("ClearPlacedObjectSet", &Scene::ClearPlacedObjectSet)
            .def("PlaceResidentBrain", &Scene::PlaceResidentBrain)
            .def("PlaceResidentBrains", &Scene::PlaceResidentBrains)
            .def("RetrieveResidentBrains", &Scene::RetrieveResidentBrains)
            .def("GetResidentBrain", &Scene::GetResidentBrain)
            .def("SetResidentBrain", &Scene::SetResidentBrain)
            .def("SetArea", &Scene::SetArea)
            .def("HasArea", &Scene::HasArea)
            .def("GetArea", &Scene::GetArea)
			.def("GetOptionalArea", &Scene::GetOptionalArea)
			.def("WithinArea", &Scene::WithinArea)
            .property("GlobalAcc", &Scene::GetGlobalAcc, &Scene::SetGlobalAcc)
			.property("GlocalAcc", &Scene::GetGlobalAcc, &Scene::SetGlobalAcc)
			.def("ResetPathFinding", &Scene::ResetPathFinding)
            .def("UpdatePathFinding", &Scene::UpdatePathFinding)
            .def("PathFindingUpdated", &Scene::PathFindingUpdated)
            .def("CalculatePath", &Scene::CalculateScenePath)
            .def_readwrite("ScenePath", &Scene::m_ScenePath, return_stl_iterator)
			.def_readwrite("Deployments", &Scene::m_Deployments, return_stl_iterator)
			.property("ScenePathSize", &Scene::GetScenePathSize),

		ABSTRACTLUABINDING(Deployment, SceneObject)
			.def("CreateDeployedActor", (Actor * (Deployment::*)())&Deployment::CreateDeployedActor, adopt(result))
			.def("CreateDeployedObject", (SceneObject * (Deployment::*)())&Deployment::CreateDeployedObject, adopt(result))
			.def("GetLoadoutName", &Deployment::GetLoadoutName)
			.property("SpawnRadius", &Deployment::GetSpawnRadius)
			.property("ID", &Deployment::GetID)
			.property("HFlipped", &Deployment::IsHFlipped),

        class_<BuyMenuGUI>("BuyMenuGUI")
            .def("SetMetaPlayer", &BuyMenuGUI::SetMetaPlayer)
            .def("SetNativeTechModule", &BuyMenuGUI::SetNativeTechModule)
            .def("SetForeignCostMultiplier", &BuyMenuGUI::SetForeignCostMultiplier)
            .def("SetModuleExpanded", &BuyMenuGUI::SetModuleExpanded)
            .def("LoadAllLoadoutsFromFile", &BuyMenuGUI::LoadAllLoadoutsFromFile)
            .def("AddAllowedItem", &BuyMenuGUI::AddAllowedItem)
			.def("RemoveAllowedItem", &BuyMenuGUI::RemoveAllowedItem)
			.def("ClearAllowedItems", &BuyMenuGUI::ClearAllowedItems)
			.def("AddAlwaysAllowedItem", &BuyMenuGUI::AddAlwaysAllowedItem)
			.def("RemoveAlwaysAllowedItem", &BuyMenuGUI::RemoveAlwaysAllowedItem)
			.def("ClearAlwaysAllowedItems", &BuyMenuGUI::ClearAlwaysAllowedItems)
			.def("AddProhibitedItem", &BuyMenuGUI::AddProhibitedItem)
			.def("RemoveProhibitedItem", &BuyMenuGUI::RemoveProhibitedItem)
			.def("ClearProhibitedItems", &BuyMenuGUI::ClearProhibitedItems)
			.def("ForceRefresh", &BuyMenuGUI::ForceRefresh)
			.def("SetOwnedItemsAmount", &BuyMenuGUI::SetOwnedItemsAmount)
			.def("GetOwnedItemsAmount", &BuyMenuGUI::GetOwnedItemsAmount)
			.def("SetHeaderImage", &BuyMenuGUI::SetHeaderImage)
			.def("SetLogoImage", &BuyMenuGUI::SetLogoImage)
			.def("ClearCartList", &BuyMenuGUI::ClearCartList)
			.def("LoadDefaultLoadoutToCart", &BuyMenuGUI::LoadDefaultLoadoutToCart)
			.property("ShowOnlyOwnedItems", &BuyMenuGUI::GetOnlyShowOwnedItems, &BuyMenuGUI::SetOnlyShowOwnedItems)
			.property("EnforceMaxPassengersConstraint", &BuyMenuGUI::EnforceMaxPassengersConstraint, &BuyMenuGUI::SetEnforceMaxPassengersConstraint)
			.property("EnforceMaxMassConstraint", &BuyMenuGUI::EnforceMaxMassConstraint, &BuyMenuGUI::SetEnforceMaxMassConstraint),

		class_<SceneEditorGUI>("SceneEditorGUI")
			.enum_("EditorGUIMode")[
				value("INACTIVE", SceneEditorGUI::EditorGUIMode::INACTIVE),
				value("PICKINGOBJECT", SceneEditorGUI::EditorGUIMode::PICKINGOBJECT),
				value("ADDINGOBJECT", SceneEditorGUI::EditorGUIMode::ADDINGOBJECT),
				value("INSTALLINGBRAIN", SceneEditorGUI::EditorGUIMode::INSTALLINGBRAIN),
				value("PLACINGOBJECT", SceneEditorGUI::EditorGUIMode::PLACINGOBJECT),
				value("MOVINGOBJECT", SceneEditorGUI::EditorGUIMode::MOVINGOBJECT),
				value("DELETINGOBJECT", SceneEditorGUI::EditorGUIMode::DELETINGOBJECT),
				value("PLACEINFRONT", SceneEditorGUI::EditorGUIMode::PLACEINFRONT),
				value("PLACEBEHIND", SceneEditorGUI::EditorGUIMode::PLACEBEHIND),
				value("DONEEDITING", SceneEditorGUI::EditorGUIMode::DONEEDITING),
				value("EDITORGUIMODECOUNT", SceneEditorGUI::EditorGUIMode::EDITORGUIMODECOUNT)
			]
            .def("SetCursorPos", &SceneEditorGUI::SetCursorPos)
            .property("EditorMode", &SceneEditorGUI::GetEditorGUIMode, &SceneEditorGUI::SetEditorGUIMode)
            .def("GetCurrentObject", &SceneEditorGUI::GetCurrentObject)
            .def("SetCurrentObject", &SceneEditorGUI::SetCurrentObject)
            .def("SetModuleSpace", &SceneEditorGUI::SetModuleSpace)
            .def("SetNativeTechModule", &SceneEditorGUI::SetNativeTechModule)
            .def("SetForeignCostMultiplier", &SceneEditorGUI::SetForeignCostMultiplier)
            .def("TestBrainResidence", &SceneEditorGUI::TestBrainResidence),

		class_<Activity, Entity>("Activity")
			.enum_("Players")[
				value("PLAYER_NONE", Players::NoPlayer),
				value("PLAYER_1", Players::PlayerOne),
				value("PLAYER_2", Players::PlayerTwo),
				value("PLAYER_3", Players::PlayerThree),
				value("PLAYER_4", Players::PlayerFour),
				value("MAXPLAYERCOUNT", Players::MaxPlayerCount)
			]
			.enum_("ActivityState")[
				value("NOACTIVITY", Activity::ActivityState::NoActivity),
				value("NOTSTARTED", Activity::ActivityState::NotStarted),
				value("STARTING", Activity::ActivityState::Starting),
				value("EDITING", Activity::ActivityState::Editing),
				value("PREGAME", Activity::ActivityState::PreGame),
				value("RUNNING", Activity::ActivityState::Running),
				value("INERROR", Activity::ActivityState::HasError),
				value("OVER", Activity::ActivityState::Over)
			]
			.enum_("Team")[
				value("NOTEAM", Activity::Teams::NoTeam),
				value("TEAM_1", Activity::Teams::TeamOne),
				value("TEAM_2", Activity::Teams::TeamTwo),
				value("TEAM_3", Activity::Teams::TeamThree),
				value("TEAM_4", Activity::Teams::TeamFour),
				value("MAXTEAMCOUNT", Activity::Teams::MaxTeamCount)
			]
			.enum_("ViewState")[
				value("NORMAL", Activity::ViewState::Normal),
				value("OBSERVE", Activity::ViewState::Observe),
				value("DEATHWATCH", Activity::ViewState::DeathWatch),
				value("ACTORSELECT", Activity::ViewState::ActorSelect),
				value("AISENTRYPOINT", Activity::ViewState::AISentryPoint),
				value("AIPATROLPOINTS", Activity::ViewState::AIPatrolPoints),
				value("AIGOLDDIGPOINT", Activity::ViewState::AIGoldDigPoint),
				value("AIGOTOPOINT", Activity::ViewState::AIGoToPoint),
				value("LZSELECT", Activity::ViewState::LandingZoneSelect)
			]
			.enum_("DifficultySetting")[
				value("MINDIFFICULTY", Activity::DifficultySetting::MinDifficulty),
				value("CAKEDIFFICULTY", Activity::DifficultySetting::CakeDifficulty),
				value("EASYDIFFICULTY", Activity::DifficultySetting::EasyDifficulty),
				value("MEDIUMDIFFICULTY", Activity::DifficultySetting::MediumDifficulty),
				value("HARDDIFFICULTY", Activity::DifficultySetting::HardDifficulty),
				value("NUTSDIFFICULTY", Activity::DifficultySetting::NutsDifficulty),
				value("MAXDIFFICULTY", Activity::DifficultySetting::MaxDifficulty)
			]
			.enum_("AISkillSetting")[
				value("MINSKILL", Activity::AISkillSetting::MinSkill),
				value("INFERIORSKILL", Activity::AISkillSetting::InferiorSkill),
				value("DEFAULTSKILL", Activity::AISkillSetting::DefaultSkill),
				value("AVERAGESKILL", Activity::AISkillSetting::AverageSkill),
				value("GOODSKILL", Activity::AISkillSetting::GoodSkill),
				value("UNFAIRSKILL", Activity::AISkillSetting::UnfairSkill)	
			]
            .def(constructor<>())
            .property("ClassName", &Activity::GetClassName)
            .property("Description", &Activity::GetDescription)
            .property("InCampaignStage", &Activity::GetInCampaignStage, &Activity::SetInCampaignStage)
            .property("ActivityState", &Activity::GetActivityState, &Activity::SetActivityState)
            .property("SceneName", &Activity::GetSceneName, &Activity::SetSceneName)
            .property("PlayerCount", &Activity::GetPlayerCount)
            .def("DeactivatePlayer", &Activity::DeactivatePlayer)
            .def("PlayerActive", &Activity::PlayerActive)
            .def("PlayerHuman", &Activity::PlayerHuman)
            .property("HumanCount", &Activity::GetHumanCount)
            .property("TeamCount", &Activity::GetTeamCount)
            .def("TeamActive", &Activity::TeamActive)
            .def("GetTeamOfPlayer", &Activity::GetTeamOfPlayer)
            .def("SetTeamOfPlayer", &Activity::SetTeamOfPlayer)
            .def("PlayersInTeamCount", &Activity::PlayersInTeamCount)
            .def("ScreenOfPlayer", &Activity::ScreenOfPlayer)
            .def("GetViewState", &Activity::GetViewState)
            .def("SetViewState", &Activity::SetViewState)
            .def("GetPlayerBrain", &Activity::GetPlayerBrain)
            .def("SetPlayerBrain", &Activity::SetPlayerBrain)
            .def("PlayerHadBrain", &Activity::PlayerHadBrain)
            .def("SetBrainEvacuated", &Activity::SetBrainEvacuated)
            .def("BrainWasEvacuated", &Activity::BrainWasEvacuated)
            .def("IsAssignedBrain", &Activity::IsAssignedBrain)
            .def("IsBrainOfWhichPlayer", &Activity::IsBrainOfWhichPlayer)
            .def("IsOtherPlayerBrain", &Activity::IsOtherPlayerBrain)
            .def("HumanBrainCount", &Activity::HumanBrainCount)
            .def("AIBrainCount", &Activity::AIBrainCount)
            .def("GetControlledActor", &Activity::GetControlledActor)
            .def("SetTeamFunds", &Activity::SetTeamFunds)
            .def("GetTeamFunds", &Activity::GetTeamFunds)
            .def("SetTeamAISkill", &Activity::SetTeamAISkill)
            .def("GetTeamAISkill", &Activity::GetTeamAISkill)
            .def("ChangeTeamFunds", &Activity::ChangeTeamFunds)
            .def("TeamFundsChanged", &Activity::TeamFundsChanged)
            .def("ReportDeath", &Activity::ReportDeath)
            .def("GetTeamDeathCount", &Activity::GetTeamDeathCount)
            .def("Running", &Activity::IsRunning)
            .def("Paused", &Activity::IsPaused)
            .def("ActivityOver", &Activity::IsOver)
            .def("EnteredOrbit", &Activity::EnteredOrbit)
            .def("SwitchToActor", &Activity::SwitchToActor)
            .def("SwitchToNextActor", &Activity::SwitchToNextActor)
            .def("SwitchToPrevActor", &Activity::SwitchToPrevActor)
            .property("Difficulty", &Activity::GetDifficulty, &Activity::SetDifficulty)
            .def("IsHumanTeam", &Activity::IsHumanTeam)
            .def("ResetMessageTimer", &Activity::ResetMessageTimer),

		class_<GUIBanner>("GUIBanner")
			.enum_("AnimMode")[
				value("BLINKING", GUIBanner::AnimMode::BLINKING),
				value("FLYBYLEFTWARD", GUIBanner::AnimMode::FLYBYLEFTWARD),
				value("FLYBYRIGHTWARD", GUIBanner::AnimMode::FLYBYRIGHTWARD),
				value("ANIMMODECOUNT", GUIBanner::AnimMode::ANIMMODECOUNT)
			]
			.enum_("AnimState")[
				value("NOTSTARTED", GUIBanner::AnimState::NOTSTARTED),
				value("SHOWING", GUIBanner::AnimState::SHOWING),
				value("SHOW", GUIBanner::AnimState::SHOW),
				value("HIDING", GUIBanner::AnimState::HIDING),
				value("OVER", GUIBanner::AnimState::OVER),
				value("ANIMSTATECOUNT", GUIBanner::AnimState::ANIMSTATECOUNT)
			]
			.enum_("BannerColor")[
				value("RED", GameActivity::BannerColor::RED),
				value("YELLOW", GameActivity::BannerColor::YELLOW)
			]
            .property("BannerText", &GUIBanner::GetBannerText)
            .property("AnimState", &GUIBanner::GetAnimState)
            .def("IsVisible", &GUIBanner::IsVisible)
            .property("Kerning", &GUIBanner::GetKerning, &GUIBanner::SetKerning)
            .def("ShowText", &GUIBanner::ShowText)
            .def("HideText", &GUIBanner::HideText)
            .def("ClearText", &GUIBanner::ClearText),

        def("ToGameActivity", (GameActivity *(*)(Entity *))&ToGameActivity),
        def("ToGameActivity", (const GameActivity *(*)(const Entity *))&ToConstGameActivity),
		class_<GameActivity, Activity>("GameActivity")
			.enum_("ObjectiveArrowDir")[
				value("ARROWDOWN", GameActivity::ObjectiveArrowDir::ARROWDOWN),
				value("ARROWLEFT", GameActivity::ObjectiveArrowDir::ARROWLEFT),
				value("ARROWRIGHT", GameActivity::ObjectiveArrowDir::ARROWRIGHT),
				value("ARROWUP", GameActivity::ObjectiveArrowDir::ARROWUP)
			]
            .def(constructor<>())
            .def("SetObservationTarget", &GameActivity::SetObservationTarget)
            .def("SetDeathViewTarget", &GameActivity::SetDeathViewTarget)
            .def("SetLandingZone", &GameActivity::SetLandingZone)
            .def("GetLandingZone", &GameActivity::GetLandingZone)
            .def("SetActorSelectCursor", &GameActivity::SetActorSelectCursor)
            .def("GetBuyGUI", &GameActivity::GetBuyGUI)
            .def("GetEditorGUI", &GameActivity::GetEditorGUI)
            .property("WinnerTeam", &GameActivity::GetWinnerTeam, &GameActivity::SetWinnerTeam)
            .property("CPUTeam", &GameActivity::GetCPUTeam, &GameActivity::SetCPUTeam)
//            .def_readwrite("ActorCursor", &GameActivity::m_ActorCursor)
            .def_readwrite("CursorTimer", &GameActivity::m_CursorTimer)
            .def_readwrite("GameTimer", &GameActivity::m_GameTimer)
            .def_readwrite("GameOverTimer", &GameActivity::m_GameOverTimer)
            .def_readwrite("GameOverPeriod", &GameActivity::m_GameOverPeriod)
			.def("OtherTeam", &GameActivity::OtherTeam)
            .def("OneOrNoneTeamsLeft", &GameActivity::OneOrNoneTeamsLeft)
            .def("WhichTeamLeft", &GameActivity::WhichTeamLeft)
            .def("NoTeamLeft", &GameActivity::NoTeamLeft)
            // Backwards compat
            .def("OnlyOneTeamLeft", &GameActivity::OneOrNoneTeamsLeft)
            .def("GetBanner", &GameActivity::GetBanner)
            .def("SetLZArea", &GameActivity::SetLZArea)
            .def("GetLZArea", &GameActivity::GetLZArea)
            .def("SetBrainLZWidth", &GameActivity::SetBrainLZWidth)
            .def("GetBrainLZWidth", &GameActivity::GetBrainLZWidth)
			.def("GetActiveCPUTeamCount", &GameActivity::GetActiveCPUTeamCount)
			.def("GetActiveHumanTeamCount", &GameActivity::GetActiveHumanTeamCount)
			.def("AddObjectivePoint", &GameActivity::AddObjectivePoint)
            .def("YSortObjectivePoints", &GameActivity::YSortObjectivePoints)
            .def("ClearObjectivePoints", &GameActivity::ClearObjectivePoints)
            .def("AddOverridePurchase", &GameActivity::AddOverridePurchase)
            .def("SetOverridePurchaseList", (int (GameActivity::*)(const Loadout *, int))&GameActivity::SetOverridePurchaseList)
            .def("SetOverridePurchaseList", (int (GameActivity::*)(string, int))&GameActivity::SetOverridePurchaseList)
            .def("ClearOverridePurchase", &GameActivity::ClearOverridePurchase)
            .def("CreateDelivery", (bool (GameActivity::*)(int))&GameActivity::CreateDelivery)
            .def("CreateDelivery", (bool (GameActivity::*)(int, int))&GameActivity::CreateDelivery)
            .def("CreateDelivery", (bool (GameActivity::*)(int, int, Vector&))&GameActivity::CreateDelivery)
            .def("CreateDelivery", (bool (GameActivity::*)(int, int, Actor*))&GameActivity::CreateDelivery)
            .def("GetDeliveryCount", &GameActivity::GetDeliveryCount)
            .property("DeliveryDelay", &GameActivity::GetDeliveryDelay, &GameActivity::SetDeliveryDelay)
			.def("GetTeamTech", &GameActivity::GetTeamTech)
			.def("SetTeamTech", &GameActivity::SetTeamTech)
			.def("GetCrabToHumanSpawnRatio", &GameActivity::GetCrabToHumanSpawnRatio)
			.property("BuyMenuEnabled", &GameActivity::GetBuyMenuEnabled, &GameActivity::SetBuyMenuEnabled)
			.property("CraftsOrbitAtTheEdge", &GameActivity::GetCraftOrbitAtTheEdge, &GameActivity::SetCraftOrbitAtTheEdge)
            .def("TeamIsCPU", &GameActivity::TeamIsCPU)
            .def("GetStartingGold", &GameActivity::GetStartingGold)
            .def("GetFogOfWarEnabled", &GameActivity::GetFogOfWarEnabled)
            .def("UpdateEditing", &GameActivity::UpdateEditing)
            .def("DisableAIs", &GameActivity::DisableAIs)
            .def("InitAIs", &GameActivity::InitAIs)
            .def("AddPieMenuSlice", &GameActivity::AddPieMenuSlice)
            .def("AlterPieMenuSlice", &GameActivity::AlterPieMenuSlice)
            .def("RemovePieMenuSlice", &GameActivity::RemovePieMenuSlice)
			.def_readwrite("PieMenuSlices", &GameActivity::m_CurrentPieMenuSlices, return_stl_iterator),
		
        class_<PieSlice>("Slice")
			.enum_("Direction")[
				value("NONE", PieSlice::SliceDirection::NONE),
					value("UP", PieSlice::SliceDirection::UP),
					value("RIGHT", PieSlice::SliceDirection::RIGHT),
					value("DOWN", PieSlice::SliceDirection::DOWN),
					value("LEFT", PieSlice::SliceDirection::LEFT)
			]
			.enum_("PieSliceIndex")[
				value("PSI_NONE", PieSlice::PieSliceIndex::PSI_NONE),
				value("PSI_PICKUP", PieSlice::PieSliceIndex::PSI_PICKUP),
				value("PSI_DROP", PieSlice::PieSliceIndex::PSI_DROP),
				value("PSI_NEXTITEM", PieSlice::PieSliceIndex::PSI_NEXTITEM),
				value("PSI_PREVITEM", PieSlice::PieSliceIndex::PSI_PREVITEM),
				value("PSI_RELOAD", PieSlice::PieSliceIndex::PSI_RELOAD),
				value("PSI_BUYMENU", PieSlice::PieSliceIndex::PSI_BUYMENU),
				value("PSI_STATS", PieSlice::PieSliceIndex::PSI_STATS),
				value("PSI_MINIMAP", PieSlice::PieSliceIndex::PSI_MINIMAP),
				value("PSI_FORMSQUAD", PieSlice::PieSliceIndex::PSI_FORMSQUAD),
				value("PSI_CEASEFIRE", PieSlice::PieSliceIndex::PSI_CEASEFIRE),
				value("PSI_SENTRY", PieSlice::PieSliceIndex::PSI_SENTRY),
				value("PSI_PATROL", PieSlice::PieSliceIndex::PSI_PATROL),
				value("PSI_BRAINHUNT", PieSlice::PieSliceIndex::PSI_BRAINHUNT),
				value("PSI_GOLDDIG", PieSlice::PieSliceIndex::PSI_GOLDDIG),
				value("PSI_GOTO", PieSlice::PieSliceIndex::PSI_GOTO),
				value("PSI_RETURN", PieSlice::PieSliceIndex::PSI_RETURN),
				value("PSI_STAY", PieSlice::PieSliceIndex::PSI_STAY),
				value("PSI_DELIVER", PieSlice::PieSliceIndex::PSI_DELIVER),
				value("PSI_SCUTTLE", PieSlice::PieSliceIndex::PSI_SCUTTLE),
				value("PSI_DONE", PieSlice::PieSliceIndex::PSI_DONE),
				value("PSI_LOAD", PieSlice::PieSliceIndex::PSI_LOAD),
				value("PSI_SAVE", PieSlice::PieSliceIndex::PSI_SAVE),
				value("PSI_NEW", PieSlice::PieSliceIndex::PSI_NEW),
				value("PSI_PICK", PieSlice::PieSliceIndex::PSI_PICK),
				value("PSI_MOVE", PieSlice::PieSliceIndex::PSI_MOVE),
				value("PSI_REMOVE", PieSlice::PieSliceIndex::PSI_REMOVE),
				value("PSI_INFRONT", PieSlice::PieSliceIndex::PSI_INFRONT),
				value("PSI_BEHIND", PieSlice::PieSliceIndex::PSI_BEHIND),
				value("PSI_ZOOMIN", PieSlice::PieSliceIndex::PSI_ZOOMIN),
				value("PSI_ZOOMOUT", PieSlice::PieSliceIndex::PSI_ZOOMOUT),
				value("PSI_TEAM1", PieSlice::PieSliceIndex::PSI_TEAM1),
				value("PSI_TEAM2", PieSlice::PieSliceIndex::PSI_TEAM2),
				value("PSI_TEAM3", PieSlice::PieSliceIndex::PSI_TEAM3),
				value("PSI_TEAM4", PieSlice::PieSliceIndex::PSI_TEAM4),
				value("PSI_SCRIPTED", PieSlice::PieSliceIndex::PSI_SCRIPTED),
				value("PSI_COUNT", PieSlice::PieSliceIndex::PSI_COUNT)
			]
			.def(constructor<>())
			.property("FunctionName", &PieSlice::GetFunctionName)
			.property("Description", &PieSlice::GetDescription)
			.property("Type", &PieSlice::GetType)
			.property("Direction", &PieSlice::GetDirection),

        ABSTRACTLUABINDING(GlobalScript, Entity)
			.def("Deactivate", &GlobalScript::Deactivate),

        class_<MetaPlayer>("MetaPlayer")
            .def(constructor<>())
            .property("NativeTechModule", &MetaPlayer::GetNativeTechModule)
            .property("ForeignCostMultiplier", &MetaPlayer::GetForeignCostMultiplier)
            .property("NativeCostMultiplier", &MetaPlayer::GetNativeCostMultiplier)
            .property("InGamePlayer", &MetaPlayer::GetInGamePlayer)
            .property("BrainPoolCount", &MetaPlayer::GetBrainPoolCount, &MetaPlayer::SetBrainPoolCount)
            .def("ChangeBrainPoolCount", &MetaPlayer::ChangeBrainPoolCount),

        class_<AlarmEvent>("AlarmEvent")
            .def(constructor<>())
            .def(constructor<const Vector &, int, float>())
            .def_readwrite("ScenePos", &AlarmEvent::m_ScenePos)
            .def_readwrite("Team", &AlarmEvent::m_Team)
            .def_readwrite("Range", &AlarmEvent::m_Range),

        class_<LuaMan>("LuaManager")
            .property("TempEntity", &LuaMan::GetTempEntity)
            .def_readonly("TempEntities", &LuaMan::m_TempEntityVector, return_stl_iterator)
            .def("FileOpen", &LuaMan::FileOpen)
            .def("FileClose", &LuaMan::FileClose)
            .def("FileReadLine", &LuaMan::FileReadLine)
            .def("FileWriteLine", &LuaMan::FileWriteLine)
            .def("FileEOF", &LuaMan::FileEOF),

        // NOT a member function, so adopting _1 instead of the _2 for the first param, since there's no "this" pointer!!
        def("DeleteEntity", &DeleteEntity, adopt(_1)),
		def("RangeRand", (double(*)(double, double))& RandomNum),
		def("PosRand", &PosRand),
        def("NormalRand", &NormalRand),
        def("SelectRand", (int(*)(int, int)) &RandomNum),
        def("LERP", &LERP),
        def("EaseIn", &EaseIn),
        def("EaseOut", &EaseOut),
        def("EaseInOut", &EaseInOut),
        def("Clamp", &Limit),
		def("GetPPM", &GetPPM),
		def("GetMPP", &GetMPP),
		def("GetPPL", &GetPPL),
		def("GetLPP", &GetLPP),
		def("RoundFloatToPrecision", &RoundFloatToPrecision),

		class_<enum_wrapper::input_device>("InputDevice")
			.enum_("InputDevice")[
				value("DEVICE_KEYB_ONLY", InputDevice::DEVICE_KEYB_ONLY),
				value("DEVICE_MOUSE_KEYB", InputDevice::DEVICE_MOUSE_KEYB),
				value("DEVICE_GAMEPAD_1", InputDevice::DEVICE_GAMEPAD_1),
				value("DEVICE_GAMEPAD_2", InputDevice::DEVICE_GAMEPAD_2),
				value("DEVICE_GAMEPAD_3", InputDevice::DEVICE_GAMEPAD_3),
				value("DEVICE_GAMEPAD_4", InputDevice::DEVICE_GAMEPAD_4),
				value("DEVICE_COUNT", InputDevice::DEVICE_COUNT)
			],

		class_<enum_wrapper::input_elements>("InputElements")
			.enum_("InputElements")[
				value("INPUT_L_UP", InputElements::INPUT_L_UP),
				value("INPUT_L_DOWN", InputElements::INPUT_L_DOWN),
				value("INPUT_L_LEFT", InputElements::INPUT_L_LEFT),
				value("INPUT_L_RIGHT", InputElements::INPUT_L_RIGHT),
				value("INPUT_R_UP", InputElements::INPUT_R_UP),
				value("INPUT_R_DOWN", InputElements::INPUT_R_DOWN),
				value("INPUT_R_LEFT", InputElements::INPUT_R_LEFT),
				value("INPUT_R_RIGHT", InputElements::INPUT_R_RIGHT),
				value("INPUT_FIRE", InputElements::INPUT_FIRE),
				value("INPUT_AIM", InputElements::INPUT_AIM),
				value("INPUT_AIM_UP", InputElements::INPUT_AIM_UP),
				value("INPUT_AIM_DOWN", InputElements::INPUT_AIM_DOWN),
				value("INPUT_AIM_LEFT", InputElements::INPUT_AIM_LEFT),
				value("INPUT_AIM_RIGHT", InputElements::INPUT_AIM_RIGHT),
				value("INPUT_PIEMENU", InputElements::INPUT_PIEMENU),
				value("INPUT_JUMP", InputElements::INPUT_JUMP),
				value("INPUT_CROUCH", InputElements::INPUT_CROUCH),
				value("INPUT_NEXT", InputElements::INPUT_NEXT),
				value("INPUT_PREV", InputElements::INPUT_PREV),
				value("INPUT_START", InputElements::INPUT_START),
				value("INPUT_BACK", InputElements::INPUT_BACK),
				value("INPUT_COUNT", InputElements::INPUT_COUNT)
			],

		class_<enum_wrapper::mouse_buttons>("MouseButtons")
			.enum_("MouseButtons")[
				value("MOUSE_NONE", MouseButtons::MOUSE_NONE),
				value("MOUSE_LEFT", MouseButtons::MOUSE_LEFT),
				value("MOUSE_RIGHT", MouseButtons::MOUSE_RIGHT),
				value("MOUSE_MIDDLE", MouseButtons::MOUSE_MIDDLE),
				value("MAX_MOUSE_BUTTONS", MouseButtons::MAX_MOUSE_BUTTONS)
			],

		class_<enum_wrapper::joy_buttons>("JoyButtons")
			.enum_("JoyButtons")[
				value("JOY_NONE", JoyButtons::JOY_NONE),
				value("JOY_1", JoyButtons::JOY_1),
				value("JOY_2", JoyButtons::JOY_2),
				value("JOY_3", JoyButtons::JOY_3),
				value("JOY_4", JoyButtons::JOY_4),
				value("JOY_5", JoyButtons::JOY_5),
				value("JOY_6", JoyButtons::JOY_6),
				value("JOY_7", JoyButtons::JOY_7),
				value("JOY_8", JoyButtons::JOY_8),
				value("JOY_9", JoyButtons::JOY_9),
				value("JOY_10", JoyButtons::JOY_10),
				value("JOY_11", JoyButtons::JOY_11),
				value("JOY_12", JoyButtons::JOY_12),
				value("MAX_JOY_BUTTONS", JoyButtons::MAX_JOY_BUTTONS)
			],

		class_<enum_wrapper::joy_directions>("JoyDirections")
			.enum_("JoyDirections")[
				value("JOYDIR_ONE", JoyDirections::JOYDIR_ONE),
				value("JOYDIR_TWO", JoyDirections::JOYDIR_TWO)
			]
    ];

    // Assign the manager instances to globals in the lua master state
    globals(m_pMasterState)["TimerMan"] = &g_TimerMan;
    globals(m_pMasterState)["FrameMan"] = &g_FrameMan;
	globals(m_pMasterState)["PostProcessMan"] = &g_PostProcessMan;
	globals(m_pMasterState)["PrimitiveMan"] = &g_PrimitiveMan;
    globals(m_pMasterState)["PresetMan"] = &g_PresetMan;
    globals(m_pMasterState)["AudioMan"] = &g_AudioMan;
    globals(m_pMasterState)["UInputMan"] = &g_UInputMan;
    globals(m_pMasterState)["SceneMan"] = &g_SceneMan;
    globals(m_pMasterState)["ActivityMan"] = &g_ActivityMan;
    globals(m_pMasterState)["MetaMan"] = &g_MetaMan;
    globals(m_pMasterState)["MovableMan"] = &g_MovableMan;
    globals(m_pMasterState)["ConsoleMan"] = &g_ConsoleMan;
    globals(m_pMasterState)["LuaMan"] = &g_LuaMan;
    globals(m_pMasterState)["SettingsMan"] = &g_SettingsMan;

    luaL_dostring(m_pMasterState,
        // Override print() in the lua state to output to the console
        "print = function(toPrint) ConsoleMan:PrintString(\"PRINT: \" .. tostring(toPrint)); end;\n"
        // Add cls() as a shorcut to ConsoleMan:Clear()
        "cls = function() ConsoleMan:Clear(); end;"
        // Add package path to the defaults
        "package.path = package.path .. \";Base.rte/?.lua\";\n"
    );

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LuaMan::ClearUserModuleCache()
{
	luaL_dostring(m_pMasterState, "for m, n in pairs(package.loaded) do if type(n) == \"boolean\" then package.loaded[m] = nil end end");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LuaMan::Destroy()
{
    lua_close(m_pMasterState);

	//Close all opened files
	for (int i = 0; i < MAX_OPEN_FILES; ++i)
		FileClose(i);

    Clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LuaMan::SavePointerAsGlobal(void *pToSave, string globalName)
{
    // Push the pointer onto the Lua stack
    lua_pushlightuserdata(m_pMasterState, pToSave);
    // Pop and assign that pointer to a global var in the Lua state
    lua_setglobal(m_pMasterState, globalName.c_str());

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LuaMan::GlobalIsDefined(string globalName)
{
    // Get the var you want onto the stack so we can check it
    lua_getglobal(m_pMasterState, globalName.c_str());
    // Now report if it is nil/null or not
    bool isDefined = !lua_isnil(m_pMasterState, -1);
    // Pop the var so this operation is balanced and leaves the stack as it was
    lua_pop(m_pMasterState, 1);

    return isDefined;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LuaMan::TableEntryIsDefined(string tableName, string indexName)
{
    // Push the table onto the stack, checking if it even exists
    lua_getglobal(m_pMasterState, tableName.c_str());
    if (!lua_istable(m_pMasterState, -1))
    {
//        error(m_pMasterState, tableName + " is not a table when checking for the " + indexName + " within it.");
        // Clean up and report that there was nothing properly defined here
        lua_pop(m_pMasterState, 1);
        return false;
    }

    // Push the value at the requested index onto the stack so we can check if it's anything
    lua_getfield(m_pMasterState, -1, indexName.c_str());
    // Now report if it is nil/null or not
    bool isDefined = !lua_isnil(m_pMasterState, -1);
    // Pop both the var and the table so this operation is balanced and leaves the stack as it was
    lua_pop(m_pMasterState, 2);

    return isDefined;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LuaMan::ExpressionIsTrue(string expression, bool consoleErrors)
{
    if (expression.empty())
        return false;

    bool result = false;

    try
    {
        // Push the script string onto the stack so we can execute it, and then actually try to run it
        // Assign the result to a dedicated temp global variable
        if (luaL_dostring(m_pMasterState, (string("ExpressionResult = ") + expression + string(";")).c_str()))
        {
            // Retrieve and pop the error message off the stack
            m_LastError = string("When evaluating Lua expression: ") + lua_tostring(m_pMasterState, -1);
            lua_pop(m_pMasterState, 1);
            if (consoleErrors)
            {
                g_ConsoleMan.PrintString("ERROR: " + m_LastError);
                ClearErrors();
            }
            return false;
        }
    }
    catch(const std::exception &e)
    {
        m_LastError = string("When evaluating Lua expression: ") + e.what();
        if (consoleErrors)
        {
            g_ConsoleMan.PrintString("ERROR: " + m_LastError);
            ClearErrors();
        }
        return false;
    }

    // Get the result var onto the stack so we can check it
    lua_getglobal(m_pMasterState, "ExpressionResult");
    // Now report if it is nil/null or not
    result = lua_toboolean(m_pMasterState, -1);
    // Pop the result var so this operation is balanced and leaves the stack as it was
    lua_pop(m_pMasterState, 1);

    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LuaMan::RunScriptedFunction(const std::string &functionName, const std::string &selfObjectName, std::vector<std::string> variablesToSafetyCheck, std::vector<Entity *> functionEntityArguments, std::vector<std::string> functionLiteralArguments) {
    std::string scriptString = "";
    if (!variablesToSafetyCheck.empty()) {
        scriptString += "if ";
        for (const std::string &variableToSafetyCheck : variablesToSafetyCheck) {
            if (&variableToSafetyCheck != &variablesToSafetyCheck[0]) {
                scriptString += " and ";
            }
            scriptString += variableToSafetyCheck;
        }
        scriptString += " then ";
    }
    if (!functionEntityArguments.empty()) {
        scriptString += "local entityArguments = LuaMan.TempEntities; ";
    }
    scriptString += functionName + "(" + selfObjectName;
    if (!functionEntityArguments.empty()) {
        g_LuaMan.SetTempEntityVector(functionEntityArguments);
        for (const Entity *functionEntityArgument : functionEntityArguments) {
            scriptString += ", (To" + functionEntityArgument->GetClassName() + " and To" + functionEntityArgument->GetClassName() + "(entityArguments()) or entityArguments())";
        }
    }
    if (!functionLiteralArguments.empty()) {
        for (const std::string functionLiteralArgument : functionLiteralArguments) {
            scriptString += ", " + functionLiteralArgument;
        }
    }
    scriptString += ");";

    if (!variablesToSafetyCheck.empty()) { scriptString += " end;"; }
    
    return RunScriptString(scriptString);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LuaMan::RunScriptString(const std::string &scriptString, bool consoleErrors) {
    if (scriptString.empty()) {
        return -1;
    }
    int error = 0;

    lua_pushcfunction(m_pMasterState, &AddFileAndLineToError);
    try {
        // Load the script string onto the stack and then execute it with pcall. Pcall will call the file and line error handler if there's an error by pointing 2 up the stack to it.
        if (luaL_loadstring(m_pMasterState, scriptString.c_str()) || lua_pcall(m_pMasterState, 0, LUA_MULTRET, -2)) {
            // Retrieve the error message then pop it off the stack to clean it up
			m_LastError = lua_tostring(m_pMasterState, -1);
            lua_pop(m_pMasterState, 1);
            if (consoleErrors) {
                g_ConsoleMan.PrintString("ERROR: " + m_LastError);
                ClearErrors();
            }
            error = -1;
        }
    } catch(const std::exception &e) {
        m_LastError = e.what();
        if (consoleErrors) {
            g_ConsoleMan.PrintString("ERROR: " + m_LastError);
            ClearErrors();
        }
        error = -1;
    }

    // Pop the file and line error handler off the stack to clean it up
    lua_pop(m_pMasterState, 1);
        
    return error;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LuaMan::RunScriptFile(const std::string &filePath, bool consoleErrors) {
    if (filePath.empty()) {
        m_LastError = "Can't run a script file with an empty filepath!";
        return -1;
    }

	if (!System::PathExistsCaseSensitive(filePath)){
		m_LastError = "Script file: " + filePath + " doesn't exist!";
		if (consoleErrors) {
			g_ConsoleMan.PrintString("ERROR: " + m_LastError);
			ClearErrors();
		}
		return -1;
	}

    int error = 0;

    lua_pushcfunction(m_pMasterState, &AddFileAndLineToError);
    try {
        // Load the script file's contents onto the stack and then execute it with pcall. Pcall will call the file and line error handler if there's an error by pointing 2 up the stack to it.
        if (luaL_loadfile(m_pMasterState, filePath.c_str()) || lua_pcall(m_pMasterState, 0, LUA_MULTRET, -2)) {
            // Retrieve the error message then pop it off the stack
            m_LastError = lua_tostring(m_pMasterState, -1);
            lua_pop(m_pMasterState, 1);
            if (consoleErrors) {
                g_ConsoleMan.PrintString("ERROR: " + m_LastError);
                ClearErrors();
            }
            error = -1;
        }
    } catch(const std::exception &e) {
        m_LastError = e.what();
        if (consoleErrors) {
            g_ConsoleMan.PrintString("ERROR: " + m_LastError);
            ClearErrors();
        }
        error = -1;
    }

    // Pop the file and line error handler off the stack to clean it up
    lua_pop(m_pMasterState, 1);

    return error;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

string LuaMan::GetNewPresetID()
{
    // Generate the new ID
    char newID[64];
    std::snprintf(newID, sizeof(newID), "Pre%05i", m_NextPresetID);
    // Increment the ID so it will be diff for the next one (improve this primitive approach??)
    m_NextPresetID++;

    return string(newID);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

string LuaMan::GetNewObjectID()
{
    // Generate the new ID
    char newID[64];
    std::snprintf(newID, sizeof(newID), "Obj%05i", m_NextObjectID);
    // Increment the ID so it will be diff for the next one (improve this primitive approach??)
    m_NextObjectID++;

    return string(newID);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LuaMan::Update()
{
	lua_gc(m_pMasterState, LUA_GCSTEP, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LuaMan::FileOpen(std::string filename, std::string mode)
{
	int fl = -1;

	// Find some suitable file
	for (int i = 0; i < MAX_OPEN_FILES; ++i)
		if (m_Files[i] == 0)
		{
			fl = i;
			break;
		}

	if (fl == -1)
	{
		g_ConsoleMan.PrintString("Error: Can't open file, no more slots.");
		return -1;
	}

	//Check for path back-traversing and .rte extension. Everything is allowed to read or write only inside rte's
    string dotString = "..";
	string rteString = ".rte";

	string fullPath = System::GetWorkingDirectory() + filename;

	// Do not open paths with '..'
	if (fullPath.find(dotString) != string::npos)
		return -1;

	// Do not open paths that aren't written correctly
	if (!System::PathExistsCaseSensitive(std::filesystem::path(fullPath).lexically_normal().generic_string()))
		return -1;

	// Allow to edit files only inside .rte folders
	if (fullPath.find(rteString) == string::npos)
		return -1;

	// Open file and save handle
	FILE * f = fopen(fullPath.c_str(), mode.c_str());
	if (f)
	{
		m_Files[fl] = f;
		return fl;
	}

#ifdef _DEBUG
	g_ConsoleMan.PrintString("Error: Can't open file. " + fullPath);
#endif
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LuaMan::FileClose(int file)
{
	if (file > -1 && file < MAX_OPEN_FILES && m_Files[file])
	{
		fclose(m_Files[file]);
		m_Files[file] = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LuaMan::FileCloseAll()
{
	for (int file = 0 ; file < MAX_OPEN_FILES ; ++file)
		FileClose(file);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string LuaMan::FileReadLine(int file)
{
	if (file > -1 && file < MAX_OPEN_FILES && m_Files[file])
	{
		char buf[4096];
		fgets(buf, 4095, m_Files[file]);
		return (std::string(buf));
	}
	g_ConsoleMan.PrintString("Error: Tried to read a closed file, or read past EOF.");
	return "";
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LuaMan::FileWriteLine(int file, std::string line)
{
	if (file > -1 && file < MAX_OPEN_FILES && m_Files[file])
		fputs(line.c_str(), m_Files[file]);
	else
		g_ConsoleMan.PrintString("Error: Tried to write to a closed file.");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LuaMan::FileEOF(int file)
{
	if (file > -1 && file < MAX_OPEN_FILES && m_Files[file])
		if (!feof(m_Files[file]))
			return false;
	return true;
}
}
