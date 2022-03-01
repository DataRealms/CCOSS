#ifndef _RTELUAADAPTERSENTITIES_
#define _RTELUAADAPTERSENTITIES_

#include "ConsoleMan.h"
#include "PresetMan.h"

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
#include "GlobalScript.h"

#include "GameActivity.h"
#include "GAScripted.h"

namespace RTE {

#pragma region Entity Lua Adapter Macros
	/// <summary>
	/// Convenience macro to generate preset clone-create adapter functions that will return the exact pre-cast types, so we don't have to do: myNewActor = ToActor(PresetMan:GetPreset("AHuman", "Soldier Light", "All")):Clone()
	/// But can instead do: myNewActor = CreateActor("Soldier Light", "All");
	/// Or even: myNewActor = CreateActor("Soldier Light");
	/// Or for a randomly selected Preset within a group: myNewActor = RandomActor("Light Troops");
	/// </summary>
	#define LuaEntityCreate(TYPE) \
		static TYPE * Create##TYPE(std::string preseName, std::string moduleName) { \
			const Entity *entityPreset = g_PresetMan.GetEntityPreset(#TYPE, preseName, moduleName); \
			if (!entityPreset) { \
				g_ConsoleMan.PrintString(std::string("ERROR: There is no ") + std::string(#TYPE) + std::string(" of the Preset name \"") + preseName + std::string("\" defined in the \"") + moduleName + std::string("\" Data Module!")); \
				return nullptr; \
			} \
			return dynamic_cast<TYPE *>(entityPreset->Clone()); \
		} \
		static TYPE * Create##TYPE(std::string preset) { \
			return Create##TYPE(preset, "All"); \
		} \
		static TYPE * Random##TYPE(std::string groupName, int moduleSpaceID) { \
			const Entity *entityPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech(groupName, #TYPE, moduleSpaceID); \
			if (!entityPreset) { entityPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech(groupName, #TYPE, g_PresetMan.GetModuleID("Base.rte")); } \
			if (!entityPreset) { entityPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech("Any", #TYPE, moduleSpaceID); } \
			if (!entityPreset) { \
				g_ConsoleMan.PrintString(std::string("WARNING: Could not find any ") + std::string(#TYPE) + std::string(" defined in a Group called \"") + groupName + std::string("\" in module ") + g_PresetMan.GetDataModuleName(moduleSpaceID) + "!");	\
				return nullptr; \
			} \
			return dynamic_cast<TYPE *>(entityPreset->Clone()); \
		} \
		static TYPE * Random##TYPE(std::string groupName, std::string dataModuleName) { \
			int moduleSpaceID = g_PresetMan.GetModuleID(dataModuleName); \
			const Entity *entityPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech(groupName, #TYPE, moduleSpaceID); \
			if (!entityPreset) { entityPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech(groupName, #TYPE, g_PresetMan.GetModuleID("Base.rte")); } \
			if (!entityPreset) { entityPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech("Any", #TYPE, moduleSpaceID); } \
			if (!entityPreset) { \
				g_ConsoleMan.PrintString(std::string("WARNING: Could not find any ") + std::string(#TYPE) + std::string(" defined in a Group called \"") + groupName + std::string("\" in module ") + dataModuleName + "!"); \
				return nullptr; \
			} \
			return dynamic_cast<TYPE *>(entityPreset->Clone()); \
		} \
		static TYPE * Random##TYPE(std::string groupName) { \
			return Random##TYPE(groupName, "All"); \
		}

	/// <summary>
	/// Convenience macro to generate a preset clone adapter function for a type.
	/// </summary>
	#define LuaEntityClone(TYPE) \
		static TYPE * Clone##TYPE(const TYPE *thisEntity) { \
			if (thisEntity) { \
				return dynamic_cast<TYPE *>(thisEntity->Clone()); \
			} \
			g_ConsoleMan.PrintString(std::string("ERROR: Tried to clone a ") + std::string(#TYPE) + std::string(" reference that is nil!")); \
			return nullptr; \
		}

	/// <summary>
	/// Convenience macro to generate type casting adapter functions for a type.
	/// </summary>
	#define LuaEntityCast(TYPE) \
		static TYPE * To##TYPE(Entity *entity) { \
			TYPE *targetType = dynamic_cast<TYPE *>(entity); \
			if (!targetType) { g_ConsoleMan.PrintString(std::string("ERROR: Tried to convert a non-") + std::string(#TYPE) + std::string(" Entity reference to an ") + std::string(#TYPE) + std::string(" reference!")); } \
			return targetType; \
		} \
		static const TYPE * ToConst##TYPE(const Entity *entity) { \
			const TYPE *targetType = dynamic_cast<const TYPE *>(entity); \
			if (!targetType) { g_ConsoleMan.PrintString(std::string("ERROR: Tried to convert a non-") + std::string(#TYPE) + std::string(" Entity reference to an ") + std::string(#TYPE) + std::string(" reference!")); } \
			return targetType; \
		} \
		static bool Is##TYPE(Entity *entity) { \
			return dynamic_cast<TYPE *>(entity) ? true : false; \
		}

	/// <summary>
	/// Special handling for passing ownership through properties. If you try to pass null to this normally, LuaJIT crashes.
	/// This handling avoids that, and is a bit safer since there's no actual ownership transfer from Lua to C++.
	/// </summary>
	#define LuaPropertyOwnershipSafetyFaker(OBJECTTYPE, PROPERTYTYPE, SETTERFUNCTION) \
		static void OBJECTTYPE##SETTERFUNCTION(OBJECTTYPE *luaSelfObject, PROPERTYTYPE *objectToSet) { \
			luaSelfObject->SETTERFUNCTION(objectToSet ? dynamic_cast<PROPERTYTYPE *>(objectToSet->Clone()) : nullptr); \
		}
#pragma endregion

#pragma region Entity Lua Adapters
	// TODO this is a temporary fix for lua PresetName setting causing scripts to have to rerun. It should be replaced with a DisplayName property someday.
	static void SetPresetName(Entity *selfObject, const std::string &presetName) {
		selfObject->SetPresetName(presetName, true);
	}

	// These methods are needed to specially handle removing attachables with Lua in order to avoid memory leaks. They have silly names cause LuaBind otherwise makes it difficult to pass values to them properly.
	// Eventually RemoveAttachable should return the removed attachable, making this whole thing no longer unsafe and these methods unnecessary (there's a TODO in MOSRotating.h for it).
	static Attachable * RemoveAttachableFromParentLuaSafe1(Attachable *luaSelfObject) {
		if (luaSelfObject->IsAttached()) {
			return luaSelfObject->GetParent()->RemoveAttachable(luaSelfObject);
		}
		return luaSelfObject;
	}
	static Attachable * RemoveAttachableFromParentLuaSafe2(Attachable *luaSelfObject, bool addToMovableMan, bool addBreakWounds) {
		if (luaSelfObject->IsAttached()) {
			return luaSelfObject->GetParent()->RemoveAttachable(luaSelfObject, addToMovableMan, addBreakWounds);
		}
		return luaSelfObject;
	}

	static void TurretAddMountedFirearm(Turret *luaSelfObject, HDFirearm *newMountedDevice) {
		luaSelfObject->AddMountedDevice(newMountedDevice);
	}

	static void GibThis(MOSRotating *luaSelfObject) {
		luaSelfObject->GibThis();
	}

	static float GetTotalValue(const SceneObject *luaSelfObject, int nativeModule, float foreignMult) {
		return luaSelfObject->GetTotalValue(nativeModule, foreignMult, 1.0F);
	}

	LuaEntityCreate(SoundContainer);
	LuaEntityCreate(Attachable);
	LuaEntityCreate(Arm);
	LuaEntityCreate(Leg);
	LuaEntityCreate(AEmitter);
	LuaEntityCreate(Turret);
	LuaEntityCreate(Actor);
	LuaEntityCreate(ADoor);
	LuaEntityCreate(AHuman);
	LuaEntityCreate(ACrab);
	LuaEntityCreate(ACraft);
	LuaEntityCreate(ACDropShip);
	LuaEntityCreate(ACRocket);
	LuaEntityCreate(MOSParticle);
	LuaEntityCreate(MOSRotating);
	LuaEntityCreate(MOPixel);
	LuaEntityCreate(Scene);
	LuaEntityCreate(HeldDevice);
	LuaEntityCreate(Round);
	LuaEntityCreate(Magazine);
	LuaEntityCreate(HDFirearm);
	LuaEntityCreate(ThrownDevice);
	LuaEntityCreate(TDExplosive);
	LuaEntityCreate(TerrainObject);
	LuaEntityCreate(PEmitter);

	LuaEntityClone(Entity);
	LuaEntityClone(SoundContainer);
	LuaEntityClone(SceneObject);
	LuaEntityClone(MovableObject);
	LuaEntityClone(Attachable);
	LuaEntityClone(Arm);
	LuaEntityClone(Leg);
	LuaEntityClone(Emission);
	LuaEntityClone(AEmitter);
	LuaEntityClone(Turret);
	LuaEntityClone(Actor);
	LuaEntityClone(ADoor);
	LuaEntityClone(AHuman);
	LuaEntityClone(ACrab);
	LuaEntityClone(ACraft);
	LuaEntityClone(ACDropShip);
	LuaEntityClone(ACRocket);
	LuaEntityClone(MOSParticle);
	LuaEntityClone(MOSRotating);
	LuaEntityClone(MOPixel);
	LuaEntityClone(Scene);
	LuaEntityClone(HeldDevice);
	LuaEntityClone(Round);
	LuaEntityClone(Magazine);
	LuaEntityClone(HDFirearm);
	LuaEntityClone(ThrownDevice);
	LuaEntityClone(TDExplosive);
	LuaEntityClone(TerrainObject);
	LuaEntityClone(PEmitter);

	LuaEntityCast(Entity);
	LuaEntityCast(SoundContainer);
	LuaEntityCast(SceneObject);
	LuaEntityCast(MovableObject);
	LuaEntityCast(Attachable);
	LuaEntityCast(Arm);
	LuaEntityCast(Leg);
	LuaEntityCast(Emission);
	LuaEntityCast(AEmitter);
	LuaEntityCast(Turret);
	LuaEntityCast(Actor);
	LuaEntityCast(ADoor);
	LuaEntityCast(AHuman);
	LuaEntityCast(ACrab);
	LuaEntityCast(ACraft);
	LuaEntityCast(ACDropShip);
	LuaEntityCast(ACRocket);
	LuaEntityCast(MOSParticle);
	LuaEntityCast(MOSRotating);
	LuaEntityCast(MOPixel);
	LuaEntityCast(MOSprite);
	LuaEntityCast(Scene);
	LuaEntityCast(Deployment);
	LuaEntityCast(GameActivity);
	LuaEntityCast(GlobalScript);
	LuaEntityCast(GAScripted);
	LuaEntityCast(HeldDevice);
	LuaEntityCast(Round);
	LuaEntityCast(Magazine);
	LuaEntityCast(HDFirearm);
	LuaEntityCast(ThrownDevice);
	LuaEntityCast(TDExplosive);
	LuaEntityCast(TerrainObject);
	LuaEntityCast(PEmitter);

	LuaPropertyOwnershipSafetyFaker(MOSRotating, SoundContainer, SetGibSound);
	LuaPropertyOwnershipSafetyFaker(Attachable, AEmitter, SetBreakWound);
	LuaPropertyOwnershipSafetyFaker(Attachable, AEmitter, SetParentBreakWound);
	LuaPropertyOwnershipSafetyFaker(AEmitter, Attachable, SetFlash);
	LuaPropertyOwnershipSafetyFaker(AEmitter, SoundContainer, SetEmissionSound);
	LuaPropertyOwnershipSafetyFaker(AEmitter, SoundContainer, SetBurstSound);
	LuaPropertyOwnershipSafetyFaker(AEmitter, SoundContainer, SetEndSound);
	LuaPropertyOwnershipSafetyFaker(ADoor, Attachable, SetDoor);
	LuaPropertyOwnershipSafetyFaker(Leg, Attachable, SetFoot);
	LuaPropertyOwnershipSafetyFaker(Actor, SoundContainer, SetBodyHitSound);
	LuaPropertyOwnershipSafetyFaker(Actor, SoundContainer, SetAlarmSound);
	LuaPropertyOwnershipSafetyFaker(Actor, SoundContainer, SetPainSound);
	LuaPropertyOwnershipSafetyFaker(Actor, SoundContainer, SetDeathSound);
	LuaPropertyOwnershipSafetyFaker(Actor, SoundContainer, SetDeviceSwitchSound);
	LuaPropertyOwnershipSafetyFaker(ADoor, SoundContainer, SetDoorMoveStartSound);
	LuaPropertyOwnershipSafetyFaker(ADoor, SoundContainer, SetDoorMoveSound);
	LuaPropertyOwnershipSafetyFaker(ADoor, SoundContainer, SetDoorDirectionChangeSound);
	LuaPropertyOwnershipSafetyFaker(ADoor, SoundContainer, SetDoorMoveEndSound);
	LuaPropertyOwnershipSafetyFaker(AHuman, Attachable, SetHead);
	LuaPropertyOwnershipSafetyFaker(AHuman, AEmitter, SetJetpack);
	LuaPropertyOwnershipSafetyFaker(AHuman, Arm, SetFGArm);
	LuaPropertyOwnershipSafetyFaker(AHuman, Arm, SetBGArm);
	LuaPropertyOwnershipSafetyFaker(AHuman, Leg, SetFGLeg);
	LuaPropertyOwnershipSafetyFaker(AHuman, Leg, SetBGLeg);
	LuaPropertyOwnershipSafetyFaker(AHuman, Attachable, SetFGFoot);
	LuaPropertyOwnershipSafetyFaker(AHuman, Attachable, SetBGFoot);
	LuaPropertyOwnershipSafetyFaker(AHuman, SoundContainer, SetStrideSound);
	LuaPropertyOwnershipSafetyFaker(ACrab, Turret, SetTurret);
	LuaPropertyOwnershipSafetyFaker(ACrab, AEmitter, SetJetpack);
	LuaPropertyOwnershipSafetyFaker(ACrab, Leg, SetLeftFGLeg);
	LuaPropertyOwnershipSafetyFaker(ACrab, Leg, SetLeftBGLeg);
	LuaPropertyOwnershipSafetyFaker(ACrab, Leg, SetRightFGLeg);
	LuaPropertyOwnershipSafetyFaker(ACrab, Leg, SetRightBGLeg);
	LuaPropertyOwnershipSafetyFaker(ACrab, SoundContainer, SetStrideSound);
	LuaPropertyOwnershipSafetyFaker(Turret, HeldDevice, SetFirstMountedDevice);
	LuaPropertyOwnershipSafetyFaker(ACraft, SoundContainer, SetHatchOpenSound);
	LuaPropertyOwnershipSafetyFaker(ACraft, SoundContainer, SetHatchCloseSound);
	LuaPropertyOwnershipSafetyFaker(ACraft, SoundContainer, SetCrashSound);
	LuaPropertyOwnershipSafetyFaker(ACDropShip, AEmitter, SetRightThruster);
	LuaPropertyOwnershipSafetyFaker(ACDropShip, AEmitter, SetLeftThruster);
	LuaPropertyOwnershipSafetyFaker(ACDropShip, AEmitter, SetURightThruster);
	LuaPropertyOwnershipSafetyFaker(ACDropShip, AEmitter, SetULeftThruster);
	LuaPropertyOwnershipSafetyFaker(ACDropShip, Attachable, SetRightHatch);
	LuaPropertyOwnershipSafetyFaker(ACDropShip, Attachable, SetLeftHatch);
	LuaPropertyOwnershipSafetyFaker(ACRocket, Leg, SetRightLeg);
	LuaPropertyOwnershipSafetyFaker(ACRocket, Leg, SetLeftLeg);
	LuaPropertyOwnershipSafetyFaker(ACRocket, AEmitter, SetMainThruster);
	LuaPropertyOwnershipSafetyFaker(ACRocket, AEmitter, SetLeftThruster);
	LuaPropertyOwnershipSafetyFaker(ACRocket, AEmitter, SetRightThruster);
	LuaPropertyOwnershipSafetyFaker(ACRocket, AEmitter, SetULeftThruster);
	LuaPropertyOwnershipSafetyFaker(ACRocket, AEmitter, SetURightThruster);
	LuaPropertyOwnershipSafetyFaker(HDFirearm, Magazine, SetMagazine);
	LuaPropertyOwnershipSafetyFaker(HDFirearm, Attachable, SetFlash);
	LuaPropertyOwnershipSafetyFaker(HDFirearm, SoundContainer, SetPreFireSound);
	LuaPropertyOwnershipSafetyFaker(HDFirearm, SoundContainer, SetFireSound);
	LuaPropertyOwnershipSafetyFaker(HDFirearm, SoundContainer, SetFireEchoSound);
	LuaPropertyOwnershipSafetyFaker(HDFirearm, SoundContainer, SetActiveSound);
	LuaPropertyOwnershipSafetyFaker(HDFirearm, SoundContainer, SetDeactivationSound);
	LuaPropertyOwnershipSafetyFaker(HDFirearm, SoundContainer, SetEmptySound);
	LuaPropertyOwnershipSafetyFaker(HDFirearm, SoundContainer, SetReloadStartSound);
	LuaPropertyOwnershipSafetyFaker(HDFirearm, SoundContainer, SetReloadEndSound);
#pragma endregion
}
#endif