// Make sure this file is always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaAdapterDefinitions.h"
#include "LuabindObjectWrapper.h"

namespace RTE {

	std::unordered_map<std::string, std::function<LuabindObjectWrapper * (Entity *, lua_State *)>> LuaAdaptersEntityCast::s_EntityToLuabindObjectCastFunctions = {};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LuaEntityCreateFunctionsDefinitionsForType(TYPE) \
	TYPE * LuaAdaptersEntityCreate::Create##TYPE(std::string preseName, std::string moduleName) { \
		const Entity *entityPreset = g_PresetMan.GetEntityPreset(#TYPE, preseName, moduleName); \
		if (!entityPreset) { \
			g_ConsoleMan.PrintString(std::string("ERROR: There is no ") + std::string(#TYPE) + std::string(" of the Preset name \"") + preseName + std::string("\" defined in the \"") + moduleName + std::string("\" Data Module!")); \
			return nullptr; \
		} \
		return dynamic_cast<TYPE *>(entityPreset->Clone()); \
	} \
	TYPE * LuaAdaptersEntityCreate::Create##TYPE(std::string preset) { \
		return Create##TYPE(preset, "All"); \
	} \
	TYPE * LuaAdaptersEntityCreate::Random##TYPE(std::string groupName, int moduleSpaceID) { \
		const Entity *entityPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech(groupName, #TYPE, moduleSpaceID); \
		if (!entityPreset) { entityPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech(groupName, #TYPE, g_PresetMan.GetModuleID("Base.rte")); } \
		if (!entityPreset) { entityPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech("Any", #TYPE, moduleSpaceID); } \
		if (!entityPreset) { \
			g_ConsoleMan.PrintString(std::string("WARNING: Could not find any ") + std::string(#TYPE) + std::string(" defined in a Group called \"") + groupName + std::string("\" in module ") + g_PresetMan.GetDataModuleName(moduleSpaceID) + "!");	\
			return nullptr; \
		} \
		return dynamic_cast<TYPE *>(entityPreset->Clone()); \
	} \
	TYPE * LuaAdaptersEntityCreate::Random##TYPE(std::string groupName, std::string dataModuleName) { \
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
	TYPE * LuaAdaptersEntityCreate::Random##TYPE(std::string groupName) { \
		return Random##TYPE(groupName, "All"); \
	}

	LuaEntityCreateFunctionsDefinitionsForType(SoundContainer);
	LuaEntityCreateFunctionsDefinitionsForType(Attachable);
	LuaEntityCreateFunctionsDefinitionsForType(Arm);
	LuaEntityCreateFunctionsDefinitionsForType(Leg);
	LuaEntityCreateFunctionsDefinitionsForType(AEmitter);
	LuaEntityCreateFunctionsDefinitionsForType(Turret);
	LuaEntityCreateFunctionsDefinitionsForType(Actor);
	LuaEntityCreateFunctionsDefinitionsForType(ADoor);
	LuaEntityCreateFunctionsDefinitionsForType(AHuman);
	LuaEntityCreateFunctionsDefinitionsForType(ACrab);
	LuaEntityCreateFunctionsDefinitionsForType(ACraft);
	LuaEntityCreateFunctionsDefinitionsForType(ACDropShip);
	LuaEntityCreateFunctionsDefinitionsForType(ACRocket);
	LuaEntityCreateFunctionsDefinitionsForType(MOSParticle);
	LuaEntityCreateFunctionsDefinitionsForType(MOSRotating);
	LuaEntityCreateFunctionsDefinitionsForType(MOPixel);
	LuaEntityCreateFunctionsDefinitionsForType(Scene);
	LuaEntityCreateFunctionsDefinitionsForType(HeldDevice);
	LuaEntityCreateFunctionsDefinitionsForType(Round);
	LuaEntityCreateFunctionsDefinitionsForType(Magazine);
	LuaEntityCreateFunctionsDefinitionsForType(HDFirearm);
	LuaEntityCreateFunctionsDefinitionsForType(ThrownDevice);
	LuaEntityCreateFunctionsDefinitionsForType(TDExplosive);
	LuaEntityCreateFunctionsDefinitionsForType(TerrainObject);
	LuaEntityCreateFunctionsDefinitionsForType(PEmitter);
	LuaEntityCreateFunctionsDefinitionsForType(PieSlice);
	LuaEntityCreateFunctionsDefinitionsForType(PieMenu);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LuaEntityCloneFunctionDefinitionForType(TYPE) \
	TYPE * LuaAdaptersEntityClone::Clone##TYPE(const TYPE *thisEntity) { \
		if (thisEntity) { \
			return dynamic_cast<TYPE *>(thisEntity->Clone()); \
		} \
		g_ConsoleMan.PrintString(std::string("ERROR: Tried to clone a ") + std::string(#TYPE) + std::string(" reference that is nil!")); \
		return nullptr; \
	}

	LuaEntityCloneFunctionDefinitionForType(Entity);
	LuaEntityCloneFunctionDefinitionForType(SoundContainer);
	LuaEntityCloneFunctionDefinitionForType(SceneObject);
	LuaEntityCloneFunctionDefinitionForType(MovableObject);
	LuaEntityCloneFunctionDefinitionForType(Attachable);
	LuaEntityCloneFunctionDefinitionForType(Arm);
	LuaEntityCloneFunctionDefinitionForType(Leg);
	LuaEntityCloneFunctionDefinitionForType(Emission);
	LuaEntityCloneFunctionDefinitionForType(AEmitter);
	LuaEntityCloneFunctionDefinitionForType(Turret);
	LuaEntityCloneFunctionDefinitionForType(Actor);
	LuaEntityCloneFunctionDefinitionForType(ADoor);
	LuaEntityCloneFunctionDefinitionForType(AHuman);
	LuaEntityCloneFunctionDefinitionForType(ACrab);
	LuaEntityCloneFunctionDefinitionForType(ACraft);
	LuaEntityCloneFunctionDefinitionForType(ACDropShip);
	LuaEntityCloneFunctionDefinitionForType(ACRocket);
	LuaEntityCloneFunctionDefinitionForType(MOSParticle);
	LuaEntityCloneFunctionDefinitionForType(MOSRotating);
	LuaEntityCloneFunctionDefinitionForType(MOPixel);
	LuaEntityCloneFunctionDefinitionForType(Scene);
	LuaEntityCloneFunctionDefinitionForType(HeldDevice);
	LuaEntityCloneFunctionDefinitionForType(Round);
	LuaEntityCloneFunctionDefinitionForType(Magazine);
	LuaEntityCloneFunctionDefinitionForType(HDFirearm);
	LuaEntityCloneFunctionDefinitionForType(ThrownDevice);
	LuaEntityCloneFunctionDefinitionForType(TDExplosive);
	LuaEntityCloneFunctionDefinitionForType(TerrainObject);
	LuaEntityCloneFunctionDefinitionForType(PEmitter);
	LuaEntityCloneFunctionDefinitionForType(PieSlice);
	LuaEntityCloneFunctionDefinitionForType(PieMenu);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LuaEntityCastFunctionsDefinitionsForType(TYPE) \
	TYPE * LuaAdaptersEntityCast::To##TYPE(Entity *entity) { \
		TYPE *targetType = dynamic_cast<TYPE *>(entity); \
		if (!targetType) { g_ConsoleMan.PrintString(std::string("ERROR: Tried to convert a non-") + std::string(#TYPE) + std::string(" Entity reference to an ") + std::string(#TYPE) + std::string(" reference! Entity was ") + (entity ? entity->GetPresetName() : "nil")); } \
		return targetType; \
	} \
	const TYPE * LuaAdaptersEntityCast::ToConst##TYPE(const Entity *entity) { \
		const TYPE *targetType = dynamic_cast<const TYPE *>(entity); \
		if (!targetType) { g_ConsoleMan.PrintString(std::string("ERROR: Tried to convert a non-") + std::string(#TYPE) + std::string(" Entity reference to an ") + std::string(#TYPE) + std::string(" reference! Entity was ") + (entity ? entity->GetPresetName() : "nil")); } \
		return targetType; \
	} \
	bool LuaAdaptersEntityCast::Is##TYPE(Entity *entity) { \
		return dynamic_cast<TYPE *>(entity) ? true : false; \
	} \
	LuabindObjectWrapper * LuaAdaptersEntityCast::ToLuabindObject##TYPE (Entity *entity, lua_State *luaState) { \
		return new LuabindObjectWrapper(new luabind::object(luaState, dynamic_cast<TYPE *>(entity)), ""); \
	} \
	/* Bullshit semi-hack to automatically populate the Luabind Object cast function map that is used in LuaMan::RunScriptFunctionObject */ \
	static const bool EntityToLuabindObjectCastMapAutoInserterForType##TYPE = []() { \
		LuaAdaptersEntityCast::s_EntityToLuabindObjectCastFunctions.try_emplace(std::string(#TYPE), &LuaAdaptersEntityCast::ToLuabindObject##TYPE); \
		return true; \
	}()

	LuaEntityCastFunctionsDefinitionsForType(Entity);
	LuaEntityCastFunctionsDefinitionsForType(SoundContainer);
	LuaEntityCastFunctionsDefinitionsForType(SceneObject);
	LuaEntityCastFunctionsDefinitionsForType(MovableObject);
	LuaEntityCastFunctionsDefinitionsForType(Attachable);
	LuaEntityCastFunctionsDefinitionsForType(Arm);
	LuaEntityCastFunctionsDefinitionsForType(Leg);
	LuaEntityCastFunctionsDefinitionsForType(Emission);
	LuaEntityCastFunctionsDefinitionsForType(AEmitter);
	LuaEntityCastFunctionsDefinitionsForType(Turret);
	LuaEntityCastFunctionsDefinitionsForType(Actor);
	LuaEntityCastFunctionsDefinitionsForType(ADoor);
	LuaEntityCastFunctionsDefinitionsForType(AHuman);
	LuaEntityCastFunctionsDefinitionsForType(ACrab);
	LuaEntityCastFunctionsDefinitionsForType(ACraft);
	LuaEntityCastFunctionsDefinitionsForType(ACDropShip);
	LuaEntityCastFunctionsDefinitionsForType(ACRocket);
	LuaEntityCastFunctionsDefinitionsForType(MOSParticle);
	LuaEntityCastFunctionsDefinitionsForType(MOSRotating);
	LuaEntityCastFunctionsDefinitionsForType(MOPixel);
	LuaEntityCastFunctionsDefinitionsForType(MOSprite);
	LuaEntityCastFunctionsDefinitionsForType(Scene);
	LuaEntityCastFunctionsDefinitionsForType(Deployment);
	LuaEntityCastFunctionsDefinitionsForType(GameActivity);
	LuaEntityCastFunctionsDefinitionsForType(GlobalScript);
	LuaEntityCastFunctionsDefinitionsForType(GAScripted);
	LuaEntityCastFunctionsDefinitionsForType(HeldDevice);
	LuaEntityCastFunctionsDefinitionsForType(Round);
	LuaEntityCastFunctionsDefinitionsForType(Magazine);
	LuaEntityCastFunctionsDefinitionsForType(HDFirearm);
	LuaEntityCastFunctionsDefinitionsForType(ThrownDevice);
	LuaEntityCastFunctionsDefinitionsForType(TDExplosive);
	LuaEntityCastFunctionsDefinitionsForType(TerrainObject);
	LuaEntityCastFunctionsDefinitionsForType(PEmitter);
	LuaEntityCastFunctionsDefinitionsForType(PieSlice);
	LuaEntityCastFunctionsDefinitionsForType(PieMenu);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LuaPropertyOwnershipSafetyFakerFunctionDefinition(OBJECTTYPE, PROPERTYTYPE, SETTERFUNCTION) \
	void LuaAdaptersPropertyOwnershipSafetyFaker::OBJECTTYPE##SETTERFUNCTION(OBJECTTYPE *luaSelfObject, PROPERTYTYPE *objectToSet) { \
		luaSelfObject->SETTERFUNCTION(objectToSet ? dynamic_cast<PROPERTYTYPE *>(objectToSet->Clone()) : nullptr); \
	}

	LuaPropertyOwnershipSafetyFakerFunctionDefinition(MOSRotating, SoundContainer, SetGibSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(Attachable, AEmitter, SetBreakWound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(Attachable, AEmitter, SetParentBreakWound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(AEmitter, Attachable, SetFlash);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(AEmitter, SoundContainer, SetEmissionSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(AEmitter, SoundContainer, SetBurstSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(AEmitter, SoundContainer, SetEndSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ADoor, Attachable, SetDoor);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(Arm, HeldDevice, SetHeldDevice);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(Leg, Attachable, SetFoot);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(Actor, PieMenu, SetPieMenu);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(Actor, SoundContainer, SetBodyHitSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(Actor, SoundContainer, SetAlarmSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(Actor, SoundContainer, SetPainSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(Actor, SoundContainer, SetDeathSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(Actor, SoundContainer, SetDeviceSwitchSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ADoor, SoundContainer, SetDoorMoveStartSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ADoor, SoundContainer, SetDoorMoveSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ADoor, SoundContainer, SetDoorDirectionChangeSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ADoor, SoundContainer, SetDoorMoveEndSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(AHuman, Attachable, SetHead);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(AHuman, AEmitter, SetJetpack);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(AHuman, Arm, SetFGArm);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(AHuman, Arm, SetBGArm);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(AHuman, Leg, SetFGLeg);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(AHuman, Leg, SetBGLeg);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(AHuman, Attachable, SetFGFoot);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(AHuman, Attachable, SetBGFoot);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(AHuman, SoundContainer, SetStrideSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACrab, Turret, SetTurret);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACrab, AEmitter, SetJetpack);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACrab, Leg, SetLeftFGLeg);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACrab, Leg, SetLeftBGLeg);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACrab, Leg, SetRightFGLeg);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACrab, Leg, SetRightBGLeg);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACrab, SoundContainer, SetStrideSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(Turret, HeldDevice, SetFirstMountedDevice);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACraft, SoundContainer, SetHatchOpenSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACraft, SoundContainer, SetHatchCloseSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACraft, SoundContainer, SetCrashSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACDropShip, AEmitter, SetRightThruster);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACDropShip, AEmitter, SetLeftThruster);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACDropShip, AEmitter, SetURightThruster);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACDropShip, AEmitter, SetULeftThruster);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACDropShip, Attachable, SetRightHatch);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACDropShip, Attachable, SetLeftHatch);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACRocket, Leg, SetRightLeg);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACRocket, Leg, SetLeftLeg);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACRocket, AEmitter, SetMainThruster);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACRocket, AEmitter, SetLeftThruster);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACRocket, AEmitter, SetRightThruster);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACRocket, AEmitter, SetULeftThruster);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(ACRocket, AEmitter, SetURightThruster);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(HDFirearm, Magazine, SetMagazine);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(HDFirearm, Attachable, SetFlash);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(HDFirearm, SoundContainer, SetPreFireSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(HDFirearm, SoundContainer, SetFireSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(HDFirearm, SoundContainer, SetFireEchoSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(HDFirearm, SoundContainer, SetActiveSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(HDFirearm, SoundContainer, SetDeactivationSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(HDFirearm, SoundContainer, SetEmptySound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(HDFirearm, SoundContainer, SetReloadStartSound);
	LuaPropertyOwnershipSafetyFakerFunctionDefinition(HDFirearm, SoundContainer, SetReloadEndSound);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaAdaptersEntity::SetPresetName(Entity *luaSelfObject, const std::string &presetName) {
		luaSelfObject->SetPresetName(presetName, true);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaAdaptersAHuman::ReloadFirearms(AHuman *luaSelfObject) {
		luaSelfObject->ReloadFirearms(false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float LuaAdaptersSceneObject::GetTotalValue(const SceneObject *luaSelfObject, int nativeModule, float foreignMult) {
		return luaSelfObject->GetTotalValue(nativeModule, foreignMult, 1.0F);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaAdaptersMovableObject::AddScript(MovableObject *luaSelfObject, const std::string &scriptPath) {
		switch (luaSelfObject->LoadScript(luaSelfObject->CorrectBackslashesInPath(scriptPath))) {
			case 0:
				return true;
			case -1:
				g_ConsoleMan.PrintString("ERROR: The script path " + scriptPath + " was empty.");
				break;
			case -2:
				g_ConsoleMan.PrintString("ERROR: The script path " + scriptPath + "  did not point to a valid file.");
				break;
			case -3:
				g_ConsoleMan.PrintString("ERROR: The script path " + scriptPath + " is already loaded onto this object.");
				break;
			case -4:
				g_ConsoleMan.PrintString("ERROR: Failed to do necessary setup to add scripts while attempting to add the script with path " + scriptPath + ". This has nothing to do with your script, please report it to a developer.");
				break;
			case -5:
				g_ConsoleMan.PrintString("ERROR: The file with script path " + scriptPath + " could not be run. Please check that this is a valid Lua file.");
				break;
			default:
				RTEAbort("Reached default case while adding script. This should never happen!");
				break;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaAdaptersMovableObject::EnableScript(MovableObject *luaSelfObject, const std::string &scriptPath) {
		return luaSelfObject->EnableOrDisableScript(scriptPath, true);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaAdaptersMovableObject::DisableScript(MovableObject *luaSelfObject, const std::string &scriptPath) {
		return luaSelfObject->EnableOrDisableScript(scriptPath, false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaAdaptersMOSRotating::GibThis(MOSRotating *luaSelfObject) {
		luaSelfObject->GibThis();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector<AEmitter *> * LuaAdaptersMOSRotating::GetWounds1(const MOSRotating *luaSelfObject) {
		return GetWounds2(luaSelfObject, true, false, false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector<AEmitter *> * LuaAdaptersMOSRotating::GetWounds2(const MOSRotating *luaSelfObject, 
	                                                                  bool includePositiveDamageAttachables,
	                                                                  bool includeNegativeDamageAttachables, 
	                                                                  bool includeNoDamageAttachables) {
		auto *wounds = new std::vector<AEmitter *>();

		GetWoundsImpl(luaSelfObject, includePositiveDamageAttachables, includeNegativeDamageAttachables, includeNoDamageAttachables, *wounds);
		return wounds;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaAdaptersMOSRotating::GetWoundsImpl(const MOSRotating *luaSelfObject, 
	                                           bool includePositiveDamageAttachables, 
	                                           bool includeNegativeDamageAttachables, 
	                                           bool includeNoDamageAttachables,
	                                           std::vector<AEmitter *> &wounds) {
		wounds.insert(wounds.end(), luaSelfObject->GetWoundList().begin(), luaSelfObject->GetWoundList().end());
		if (includePositiveDamageAttachables || includeNegativeDamageAttachables || includeNoDamageAttachables) {
			for (const Attachable *attachable : luaSelfObject->GetAttachables()) {
				bool attachableSatisfiesConditions = (includePositiveDamageAttachables && attachable->GetDamageMultiplier() > 0) ||
				                                     (includeNegativeDamageAttachables && attachable->GetDamageMultiplier() < 0) ||
				                                     (includeNoDamageAttachables && attachable->GetDamageMultiplier() == 0);

				if (attachableSatisfiesConditions) {
					GetWoundsImpl(attachable, includePositiveDamageAttachables, includeNegativeDamageAttachables, includeNoDamageAttachables, wounds);
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Attachable * LuaAdaptersAttachable::RemoveFromParent1(Attachable *luaSelfObject) {
		if (luaSelfObject->IsAttached()) {
			return luaSelfObject->GetParent()->RemoveAttachable(luaSelfObject);
		}
		return luaSelfObject;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Attachable * LuaAdaptersAttachable::RemoveFromParent2(Attachable *luaSelfObject, bool addToMovableMan, bool addBreakWounds) {
		if (luaSelfObject->IsAttached()) {
			return luaSelfObject->GetParent()->RemoveAttachable(luaSelfObject, addToMovableMan, addBreakWounds);
		}
		return luaSelfObject;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaAdaptersTurret::AddMountedFirearm(Turret *luaSelfObject, HDFirearm *newMountedDevice) {
		luaSelfObject->AddMountedDevice(newMountedDevice);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaAdaptersGlobalScript::Deactivate(GlobalScript *luaSelfObject) {
		luaSelfObject->SetActive(false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaAdaptersPieMenu::AddPieSlice(PieMenu *luaSelfObject, PieSlice *pieSliceToAdd, const Entity *pieSliceOriginalSource) {
		return luaSelfObject->AddPieSlice(pieSliceToAdd, pieSliceOriginalSource, false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaAdaptersPieMenu::AddPieSliceIfPresetNameIsUnique1(PieMenu *luaSelfObject, PieSlice *pieSliceToAdd, const Entity *pieSliceOriginalSource) {
		return luaSelfObject->AddPieSliceIfPresetNameIsUnique(pieSliceToAdd, pieSliceOriginalSource, false, false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaAdaptersPieMenu::AddPieSliceIfPresetNameIsUnique2(PieMenu *luaSelfObject, PieSlice *pieSliceToAdd, const Entity *pieSliceOriginalSource, bool onlyCheckPieSlicesWithSameOriginalSource) {
		return luaSelfObject->AddPieSliceIfPresetNameIsUnique(pieSliceToAdd, pieSliceOriginalSource, onlyCheckPieSlicesWithSameOriginalSource, false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaAdaptersMovableMan::AddMO(MovableMan &movableMan, MovableObject *movableObject) {
		if (movableMan.ValidMO(movableObject)) {
			g_ConsoleMan.PrintString("ERROR: Tried to add a MovableObject that already exists in the simulation! " + movableObject->GetPresetName());
		} else {
			movableMan.AddMO(movableObject);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaAdaptersMovableMan::AddActor(MovableMan &movableMan, Actor *actor) {
		if (movableMan.IsActor(actor)) {
			g_ConsoleMan.PrintString("ERROR: Tried to add an Actor that already exists in the simulation!" + actor->GetPresetName());
		} else {
			movableMan.AddActor(actor);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaAdaptersMovableMan::AddItem(MovableMan &movableMan, HeldDevice *item) {
		if (movableMan.ValidMO(dynamic_cast<MovableObject *>(item))) {
			g_ConsoleMan.PrintString("ERROR: Tried to add an Item that already exists in the simulation!" + item->GetPresetName());
		} else {
			movableMan.AddItem(item);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaAdaptersMovableMan::AddParticle(MovableMan &movableMan, MovableObject *particle) {
		if (movableMan.ValidMO(particle)) {
			g_ConsoleMan.PrintString("ERROR: Tried to add a Particle that already exists in the simulation!" + particle->GetPresetName());
		} else {
			movableMan.AddParticle(particle);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double LuaAdaptersTimerMan::GetDeltaTimeTicks(const TimerMan &timerMan) {
		return static_cast<double>(timerMan.GetDeltaTimeTicks());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double LuaAdaptersTimerMan::GetTicksPerSecond(const TimerMan &timerMan) {
		return static_cast<double>(timerMan.GetTicksPerSecond());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaAdaptersUInputMan::MouseButtonHeld(const UInputMan &uinputMan, int whichButton) {
		return uinputMan.MouseButtonHeld(Players::PlayerOne, whichButton);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaAdaptersUInputMan::MouseButtonPressed(const UInputMan &uinputMan, int whichButton) {
		return uinputMan.MouseButtonPressed(Players::PlayerOne, whichButton);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaAdaptersUInputMan::MouseButtonReleased(const UInputMan &uinputMan, int whichButton) {
		return uinputMan.MouseButtonReleased(Players::PlayerOne, whichButton);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaAdaptersPresetMan::ReloadEntityPreset1(PresetMan &presetMan, const std::string &presetName, const std::string &className, const std::string &moduleName) {
		return presetMan.ReloadEntityPreset(presetName, className, moduleName);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaAdaptersPresetMan::ReloadEntityPreset2(PresetMan &presetMan, const std::string &presetName, const std::string &className) {
		return ReloadEntityPreset1(presetMan, presetName, className, "");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaAdaptersPrimitiveMan::DrawPolygonPrimitive(PrimitiveMan &primitiveMan, const Vector &centerPos, int color, const luabind::object &verticesTable) {
		primitiveMan.DrawPolygonOrPolygonFillPrimitive(-1, centerPos, color, ConvertLuaTableToVectorOfType<Vector *>(verticesTable), false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaAdaptersPrimitiveMan::DrawPolygonPrimitiveForPlayer(PrimitiveMan &primitiveMan, int player, const Vector &centerPos, int color, const luabind::object &verticesTable) {
		primitiveMan.DrawPolygonOrPolygonFillPrimitive(player, centerPos, color, ConvertLuaTableToVectorOfType<Vector *>(verticesTable), false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaAdaptersPrimitiveMan::DrawPolygonFillPrimitive(PrimitiveMan &primitiveMan, const Vector &startPos, int color, const luabind::object &verticesTable) {
		primitiveMan.DrawPolygonOrPolygonFillPrimitive(-1, startPos, color, ConvertLuaTableToVectorOfType<Vector *>(verticesTable), true);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaAdaptersPrimitiveMan::DrawPolygonFillPrimitiveForPlayer(PrimitiveMan &primitiveMan, int player, const Vector &startPos, int color, const luabind::object &verticesTable) {
		primitiveMan.DrawPolygonOrPolygonFillPrimitive(player, startPos, color, ConvertLuaTableToVectorOfType<Vector *>(verticesTable), true);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaAdaptersPrimitiveMan::DrawPrimitivesWithTransparency(PrimitiveMan &primitiveMan, int transValue, const luabind::object &primitivesTable) {
		primitiveMan.SchedulePrimitivesForBlendedDrawing(DrawBlendMode::BlendTransparency, transValue, transValue, transValue, BlendAmountLimits::MinBlend, ConvertLuaTableToVectorOfType<GraphicalPrimitive *>(primitivesTable));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaAdaptersPrimitiveMan::DrawPrimitivesWithBlending(PrimitiveMan &primitiveMan, int blendMode, int blendAmount, const luabind::object &primitivesTable) {
		primitiveMan.SchedulePrimitivesForBlendedDrawing(static_cast<DrawBlendMode>(blendMode), blendAmount, blendAmount, blendAmount, blendAmount, ConvertLuaTableToVectorOfType<GraphicalPrimitive *>(primitivesTable));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaAdaptersPrimitiveMan::DrawPrimitivesWithBlendingPerChannel(PrimitiveMan &primitiveMan, int blendMode, int blendAmountR, int blendAmountG, int blendAmountB, int blendAmountA, const luabind::object &primitivesTable) {
		primitiveMan.SchedulePrimitivesForBlendedDrawing(static_cast<DrawBlendMode>(blendMode), blendAmountR, blendAmountG, blendAmountB, blendAmountA, ConvertLuaTableToVectorOfType<GraphicalPrimitive *>(primitivesTable));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float LuaAdaptersUtility::GetMPP() {
		return c_MPP;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float LuaAdaptersUtility::GetPPM() {
		return c_PPM;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float LuaAdaptersUtility::GetLPP() {
		return c_LPP;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float LuaAdaptersUtility::GetPPL() {
		return c_PPL;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float LuaAdaptersUtility::GetPathFindingDefaultDigStrength() {
		return c_PathFindingDefaultDigStrength;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaAdaptersUtility::DeleteEntity(Entity *entityToDelete) {
		delete entityToDelete;
		entityToDelete = nullptr;
	}
#pragma endregion
}