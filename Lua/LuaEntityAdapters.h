#ifndef _RTELUAENTITYADAPTERS_
#define _RTELUAENTITYADAPTERS_

#include "LuaMacros.h"

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// These are expanded by the preprocessor to all the different cloning function definitions.
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////
	// Preset clone adapters that will return the exact pre-cast types so we don't have to do:
	// myNewActor = ToActor(PresetMan:GetPreset("AHuman", "Soldier Light", "All")):Clone()
	// but can instead do:
	// myNewActor = CreateActor("Soldier Light", "All");
	// or even:
	// myNewActor = CreateActor("Soldier Light");
	// or for a randomly selected Preset within a group:
	// myNewActor = RandomActor("Light Troops");
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// These are expanded by the preprocessor to all the different casting function definitions named: To[Type]()
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

	LuaPropertyOwnershipSafetyFaker(Turret, HeldDevice, SetMountedDevice);

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AddMO(MovableMan &movableMan, MovableObject *movableObject) {
		if (movableMan.ValidMO(movableObject)) {
			g_ConsoleMan.PrintString("ERROR: Tried to add a MovableObject that already exists in the simulation! " + movableObject->GetPresetName());
		} else {
			movableMan.AddMO(movableObject);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AddActor(MovableMan &movableMan, Actor *actor) {
		if (movableMan.IsActor(actor)) {
			g_ConsoleMan.PrintString("ERROR: Tried to add an Actor that already exists in the simulation!" + actor->GetPresetName());
		} else {
			movableMan.AddActor(actor);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AddItem(MovableMan &movableMan, MovableObject *item) {
		if (movableMan.ValidMO(item)) {
			g_ConsoleMan.PrintString("ERROR: Tried to add an Item that already exists in the simulation!" + item->GetPresetName());
		} else {
			movableMan.AddItem(item);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AddParticle(MovableMan &movableMan, MovableObject *particle) {
		if (movableMan.ValidMO(particle)) {
			g_ConsoleMan.PrintString("ERROR: Tried to add a Particle that already exists in the simulation!" + particle->GetPresetName());
		} else {
			movableMan.AddParticle(particle);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	These methods are needed to specially handling removing attachables with Lua in order to avoid memory leaks. They have silly names cause luabind otherwise makes it difficult to pass values to them properly.
	Eventually RemoveAttachable should return the removed attachable, making this whole thing no longer unsafe and these methods unnecessary (there's a TODO in MOSRotating.h for it).
	*/
	bool RemoveAttachableLuaSafe4(MOSRotating *luaSelfObject, Attachable *attachable, bool addToMovableMan, bool addBreakWounds) {
		if (!addToMovableMan && !attachable->IsSetToDelete()) {
			attachable->SetToDelete();
		}
		return luaSelfObject->RemoveAttachable(attachable, addToMovableMan, addBreakWounds);
	}

	bool RemoveAttachableLuaSafe3(MOSRotating *luaSelfObject, Attachable *attachable) {
		return RemoveAttachableLuaSafe4(luaSelfObject, attachable, false, false);
	}

	bool RemoveAttachableLuaSafe2(MOSRotating *luaSelfObject, long attachableUniqueID, bool addToMovableMan, bool addBreakWounds) {
		MovableObject *attachableAsMovableObject = g_MovableMan.FindObjectByUniqueID(attachableUniqueID);
		if (attachableAsMovableObject) {
			return RemoveAttachableLuaSafe4(luaSelfObject, dynamic_cast<Attachable *>(attachableAsMovableObject), addToMovableMan, addBreakWounds);
		}
		return false;
	}

	bool RemoveAttachableLuaSafe1(MOSRotating *luaSelfObject, long attachableUniqueID) {
		return RemoveAttachableLuaSafe2(luaSelfObject, attachableUniqueID, false, false);
	}

	bool RemoveAttachableFromParentLuaSafe1(Attachable *luaSelfObject) {
		if (luaSelfObject->IsAttached()) {
			return RemoveAttachableLuaSafe4(luaSelfObject->GetParent(), luaSelfObject, false, false);
		}
		return false;
	}

	bool RemoveAttachableFromParentLuaSafe2(Attachable *luaSelfObject, bool addToMovableMan, bool addBreakWounds) {
		if (luaSelfObject->IsAttached()) {
			return RemoveAttachableLuaSafe4(luaSelfObject->GetParent(), luaSelfObject, addToMovableMan, addBreakWounds);
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GibThis(MOSRotating *pThis) {
		pThis->GibThis();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double NormalRand() {
		return RandomNormalNum<double>();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double PosRand() {
		return RandomNum<double>();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double LuaRand(double num) {
		return RandomNum<double>(1, num);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// Explicit deletion of any Entity instance that Lua owns. It will probably be handled by the GC, but this makes it instantaneous.
	/// </summary>
	/// <param name="entityToDelete">The Entity to delete.</param>
	void DeleteEntity(Entity *entityToDelete) {
		delete entityToDelete;
		entityToDelete = nullptr;
	}
}
#endif