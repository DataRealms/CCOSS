// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Entity) {
		return luabind::class_<Entity>("Entity")

		.def(luabind::tostring(luabind::const_self))

		.property("ClassName", &Entity::GetClassName)
		.property("PresetName", &Entity::GetPresetName, &LuaAdaptersEntity::SetPresetName)
		.property("Description", &Entity::GetDescription, &Entity::SetDescription)
		.property("IsOriginalPreset", &Entity::IsOriginalPreset)
		.property("ModuleID", &Entity::GetModuleID)
		.property("ModuleName", &Entity::GetModuleName)
		.property("RandomWeight", &Entity::GetRandomWeight)
		.property("Groups", &Entity::GetGroups, luabind::return_stl_iterator)

		.def("Clone", &LuaAdaptersEntityClone::CloneEntity)
		.def("Reset", &Entity::Reset)
		.def("GetModuleAndPresetName", &Entity::GetModuleAndPresetName)
		.def("AddToGroup", &Entity::AddToGroup)
		.def("RemoveFromGroup", &Entity::RemoveFromGroup)
		.def("IsInGroup", &Entity::IsInGroup);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ACDropShip) {
		return ConcreteTypeLuaClassDefinition(ACDropShip, ACraft)

		.property("RightEngine", &ACDropShip::GetRightThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACDropShipSetRightThruster)
		.property("LeftEngine", &ACDropShip::GetLeftThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACDropShipSetLeftThruster)
		.property("RightThruster", &ACDropShip::GetURightThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACDropShipSetURightThruster)
		.property("LeftThruster", &ACDropShip::GetULeftThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACDropShipSetULeftThruster)
		.property("RightHatch", &ACDropShip::GetRightHatch, &LuaAdaptersPropertyOwnershipSafetyFaker::ACDropShipSetRightHatch)
		.property("LeftHatch", &ACDropShip::GetLeftHatch, &LuaAdaptersPropertyOwnershipSafetyFaker::ACDropShipSetLeftHatch)
		.property("MaxEngineAngle", &ACDropShip::GetMaxEngineAngle, &ACDropShip::SetMaxEngineAngle)
		.property("LateralControlSpeed", &ACDropShip::GetLateralControlSpeed, &ACDropShip::SetLateralControlSpeed)
		.property("LateralControl", &ACDropShip::GetLateralControl)
		.property("HoverHeightModifier", &ACDropShip::GetHoverHeightModifier, &ACDropShip::SetHoverHeightModifier)

		.def("DetectObstacle", &ACDropShip::DetectObstacle)
		.def("GetAltitude", &ACDropShip::GetAltitude);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ACrab) {
		return ConcreteTypeLuaClassDefinition(ACrab, Actor)

		.def(luabind::constructor<>())

		.property("Turret", &ACrab::GetTurret, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetTurret)
		.property("Jetpack", &ACrab::GetJetpack, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetJetpack)
		.property("LeftFGLeg", &ACrab::GetLeftFGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetLeftFGLeg)
		.property("LeftBGLeg", &ACrab::GetLeftBGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetLeftBGLeg)
		.property("RightFGLeg", &ACrab::GetRightFGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetRightFGLeg)
		.property("RightBGLeg", &ACrab::GetRightBGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetRightBGLeg)
		.property("StrideSound", &ACrab::GetStrideSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ACrabSetStrideSound)
		.property("StrideFrame", &ACrab::StrideFrame)
		.property("EquippedItem", &ACrab::GetEquippedItem)
		.property("FirearmIsReady", &ACrab::FirearmIsReady)
		.property("FirearmIsEmpty", &ACrab::FirearmIsEmpty)
		.property("FirearmNeedsReload", &ACrab::FirearmNeedsReload)
		.property("FirearmIsSemiAuto", &ACrab::FirearmIsSemiAuto)
		.property("FirearmActivationDelay", &ACrab::FirearmActivationDelay)
		.property("LimbPathPushForce", &ACrab::GetLimbPathPushForce, &ACrab::SetLimbPathPushForce)
		.property("AimRangeUpperLimit", &ACrab::GetAimRangeUpperLimit, &ACrab::SetAimRangeUpperLimit)
		.property("AimRangeLowerLimit", &ACrab::GetAimRangeLowerLimit, &ACrab::SetAimRangeLowerLimit)

		.def("ReloadFirearms", &ACrab::ReloadFirearms)
		.def("IsWithinRange", &ACrab::IsWithinRange)
		.def("Look", &ACrab::Look)
		.def("LookForMOs", &ACrab::LookForMOs)
		.def("GetLimbPath", &ACrab::GetLimbPath)
		.def("GetLimbPathSpeed", &ACrab::GetLimbPathSpeed)
		.def("SetLimbPathSpeed", &ACrab::SetLimbPathSpeed)

		.enum_("Side")[
			luabind::value("LEFTSIDE", ACrab::Side::LEFTSIDE),
			luabind::value("RIGHTSIDE", ACrab::Side::RIGHTSIDE),
			luabind::value("SIDECOUNT", ACrab::Side::SIDECOUNT)
		]
		.enum_("Layer")[
			luabind::value("FGROUND", ACrab::Layer::FGROUND),
			luabind::value("BGROUND", ACrab::Layer::BGROUND)
		]
		.enum_("DeviceHandlingState")[
			luabind::value("STILL", ACrab::DeviceHandlingState::STILL),
			luabind::value("POINTING", ACrab::DeviceHandlingState::POINTING),
			luabind::value("SCANNING", ACrab::DeviceHandlingState::SCANNING),
			luabind::value("AIMING", ACrab::DeviceHandlingState::AIMING),
			luabind::value("FIRING", ACrab::DeviceHandlingState::FIRING),
			luabind::value("THROWING", ACrab::DeviceHandlingState::THROWING),
			luabind::value("DIGGING", ACrab::DeviceHandlingState::DIGGING)
		]
		.enum_("SweepState")[
			luabind::value("NOSWEEP", ACrab::SweepState::NOSWEEP),
			luabind::value("SWEEPINGUP", ACrab::SweepState::SWEEPINGUP),
			luabind::value("SWEEPUPPAUSE", ACrab::SweepState::SWEEPUPPAUSE),
			luabind::value("SWEEPINGDOWN", ACrab::SweepState::SWEEPINGDOWN),
			luabind::value("SWEEPDOWNPAUSE", ACrab::SweepState::SWEEPDOWNPAUSE)
		]
		.enum_("DigState")[
			luabind::value("NOTDIGGING", ACrab::DigState::NOTDIGGING),
			luabind::value("PREDIG", ACrab::DigState::PREDIG),
			luabind::value("STARTDIG", ACrab::DigState::STARTDIG),
			luabind::value("TUNNELING", ACrab::DigState::TUNNELING),
			luabind::value("FINISHINGDIG", ACrab::DigState::FINISHINGDIG),
			luabind::value("PAUSEDIGGER", ACrab::DigState::PAUSEDIGGER)
		]
		.enum_("JumpState")[
			luabind::value("NOTJUMPING", ACrab::JumpState::NOTJUMPING),
			luabind::value("FORWARDJUMP", ACrab::JumpState::FORWARDJUMP),
			luabind::value("PREJUMP", ACrab::JumpState::PREUPJUMP),
			luabind::value("UPJUMP", ACrab::JumpState::UPJUMP),
			luabind::value("APEXJUMP", ACrab::JumpState::APEXJUMP),
			luabind::value("LANDJUMP", ACrab::JumpState::LANDJUMP)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ACraft) {
		return AbstractTypeLuaClassDefinition(ACraft, Actor)

		.property("HatchState", &ACraft::GetHatchState)
		.property("HatchOpenSound", &ACraft::GetHatchOpenSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ACraftSetHatchOpenSound)
		.property("HatchCloseSound", &ACraft::GetHatchCloseSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ACraftSetHatchCloseSound)
		.property("CrashSound", &ACraft::GetCrashSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ACraftSetCrashSound)
		.property("MaxPassengers", &ACraft::GetMaxPassengers)
		.property("DeliveryDelayMultiplier", &ACraft::GetDeliveryDelayMultiplier)
		.property("ScuttleOnDeath", &ACraft::GetScuttleOnDeath, &ACraft::SetScuttleOnDeath)
		.property("HatchDelay", &ACraft::GetHatchDelay, &ACraft::SetHatchDelay)

		.def("OpenHatch", &ACraft::OpenHatch)
		.def("CloseHatch", &ACraft::CloseHatch)

		.enum_("HatchState")[
			luabind::value("CLOSED", ACraft::HatchState::CLOSED),
			luabind::value("OPENING", ACraft::HatchState::OPENING),
			luabind::value("OPEN", ACraft::HatchState::OPEN),
			luabind::value("CLOSING", ACraft::HatchState::CLOSING),
			luabind::value("HatchStateCount", ACraft::HatchState::HatchStateCount)
		]
		.enum_("Side")[
			luabind::value("RIGHT", ACraft::Side::RIGHT),
			luabind::value("LEFT", ACraft::Side::LEFT)
		]

		.enum_("CraftDeliverySequence")[
			luabind::value("FALL", ACraft::CraftDeliverySequence::FALL),
			luabind::value("LAND", ACraft::CraftDeliverySequence::LAND),
			luabind::value("STANDBY", ACraft::CraftDeliverySequence::STANDBY),
			luabind::value("UNLOAD", ACraft::CraftDeliverySequence::UNLOAD),
			luabind::value("LAUNCH", ACraft::CraftDeliverySequence::LAUNCH),
			luabind::value("UNSTICK", ACraft::CraftDeliverySequence::UNSTICK)
		]
		.enum_("AltitudeMoveState")[
			luabind::value("HOVER", ACraft::AltitudeMoveState::HOVER),
			luabind::value("DESCEND", ACraft::AltitudeMoveState::DESCEND),
			luabind::value("ASCEND", ACraft::AltitudeMoveState::ASCEND)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ACRocket) {
		return ConcreteTypeLuaClassDefinition(ACRocket, ACraft)

		.property("RightLeg", &ACRocket::GetRightLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetRightLeg)
		.property("LeftLeg", &ACRocket::GetLeftLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetLeftLeg)
		.property("MainEngine", &ACRocket::GetMainThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetMainThruster)
		.property("LeftEngine", &ACRocket::GetLeftThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetLeftThruster)
		.property("RightEngine", &ACRocket::GetRightThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetRightThruster)
		.property("LeftThruster", &ACRocket::GetULeftThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetULeftThruster)
		.property("RightThruster", &ACRocket::GetURightThruster, &LuaAdaptersPropertyOwnershipSafetyFaker::ACRocketSetURightThruster)
		.property("GearState", &ACRocket::GetGearState)

		.enum_("LandingGearState")[
			luabind::value("RAISED", ACRocket::LandingGearState::RAISED),
			luabind::value("LOWERED", ACRocket::LandingGearState::LOWERED),
			luabind::value("LOWERING", ACRocket::LandingGearState::LOWERING),
			luabind::value("RAISING", ACRocket::LandingGearState::RAISING),
			luabind::value("GearStateCount", ACRocket::LandingGearState::GearStateCount)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Actor) {
		return ConcreteTypeLuaClassDefinition(Actor, MOSRotating)

		.def(luabind::constructor<>())

		.property("PlayerControllable", &Actor::IsPlayerControllable, &Actor::SetPlayerControllable)
		.property("BodyHitSound", &Actor::GetBodyHitSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ActorSetBodyHitSound)
		.property("AlarmSound", &Actor::GetAlarmSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ActorSetAlarmSound)
		.property("PainSound", &Actor::GetPainSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ActorSetPainSound)
		.property("DeathSound", &Actor::GetDeathSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ActorSetDeathSound)
		.property("DeviceSwitchSound", &Actor::GetDeviceSwitchSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ActorSetDeviceSwitchSound)
		.property("ImpulseDamageThreshold", &Actor::GetTravelImpulseDamage, &Actor::SetTravelImpulseDamage)
		.property("StableRecoveryDelay", &Actor::GetStableRecoverDelay, &Actor::SetStableRecoverDelay)
		.property("Status", &Actor::GetStatus, &Actor::SetStatus)
		.property("Health", &Actor::GetHealth, &Actor::SetHealth)
		.property("PrevHealth", &Actor::GetPrevHealth)
		.property("MaxHealth", &Actor::GetMaxHealth, &Actor::SetMaxHealth)
		.property("InventoryMass", &Actor::GetInventoryMass)
		.property("GoldCarried", &Actor::GetGoldCarried, &Actor::SetGoldCarried)
		.property("AimRange", &Actor::GetAimRange, &Actor::SetAimRange)
		.property("CPUPos", &Actor::GetCPUPos)
		.property("EyePos", &Actor::GetEyePos)
		.property("HolsterOffset", &Actor::GetHolsterOffset, &Actor::SetHolsterOffset)
		.property("ReloadOffset", &Actor::GetReloadOffset, &Actor::SetReloadOffset)
		.property("ViewPoint", &Actor::GetViewPoint, &Actor::SetViewPoint)
		.property("ItemInReach", &Actor::GetItemInReach, &Actor::SetItemInReach)
		.property("SharpAimProgress", &Actor::GetSharpAimProgress)
		.property("Height", &Actor::GetHeight)
		.property("AIMode", &Actor::GetAIMode, &Actor::SetAIMode)
		.property("DeploymentID", &Actor::GetDeploymentID)
		.property("PassengerSlots", &Actor::GetPassengerSlots, &Actor::SetPassengerSlots)
		.property("Perceptiveness", &Actor::GetPerceptiveness, &Actor::SetPerceptiveness)
		.property("PainThreshold", &Actor::GetPainThreshold, &Actor::SetPainThreshold)
		.property("CanRevealUnseen", &Actor::GetCanRevealUnseen, &Actor::SetCanRevealUnseen)
		.property("InventorySize", &Actor::GetInventorySize)
		.property("MaxInventoryMass", &Actor::GetMaxInventoryMass)
		.property("MovePathSize", &Actor::GetMovePathSize)
		.property("MovePathEnd", &Actor::GetMovePathEnd)
		.property("IsWaitingOnNewMovePath", &Actor::IsWaitingOnNewMovePath)
		.property("AimDistance", &Actor::GetAimDistance, &Actor::SetAimDistance)
		.property("SightDistance", &Actor::GetSightDistance, &Actor::SetSightDistance)
		.property("PieMenu", &Actor::GetPieMenu, &LuaAdaptersPropertyOwnershipSafetyFaker::ActorSetPieMenu)
		.property("AIBaseDigStrength", &Actor::GetAIBaseDigStrength, &Actor::SetAIBaseDigStrength)
		.property("DigStrength", &Actor::EstimateDigStrength)
		.property("SceneWaypoints", &LuaAdaptersActor::GetSceneWaypoints, luabind::adopt(luabind::return_value) + luabind::return_stl_iterator)
		.property("LimbPushForcesAndCollisionsDisabled", &Actor::GetLimbPushForcesAndCollisionsDisabled, &Actor::SetLimbPushForcesAndCollisionsDisabled)
		.property("MoveProximityLimit", &Actor::GetMoveProximityLimit, &Actor::SetMoveProximityLimit)

		.def_readwrite("MOMoveTarget", &Actor::m_pMOMoveTarget)
		.def_readwrite("MovePath", &Actor::m_MovePath, luabind::return_stl_iterator)
		.def_readwrite("Inventory", &Actor::m_Inventory, luabind::return_stl_iterator)

		.def("GetController", &Actor::GetController)
		.def("IsPlayerControlled", &Actor::IsPlayerControlled)
		.def("IsControllable", &Actor::IsControllable)
		.def("SetControllerMode", &Actor::SetControllerMode)
		.def("SwapControllerModes", &Actor::SwapControllerModes)
		.def("GetStableVelocityThreshold", &Actor::GetStableVel)
		.def("SetStableVelocityThreshold", (void (Actor::*)(float, float))&Actor::SetStableVel)
		.def("SetStableVelocityThreshold", (void (Actor::*)(Vector))&Actor::SetStableVel)
		.def("GetAimAngle", &Actor::GetAimAngle)
		.def("SetAimAngle", &Actor::SetAimAngle)
		.def("HasObject", &Actor::HasObject)
		.def("HasObjectInGroup", &Actor::HasObjectInGroup)
		.def("IsWithinRange", &Actor::IsWithinRange)
		.def("AddGold", &Actor::AddGold)
		.def("AddHealth", &Actor::AddHealth)
		.def("IsStatus", &Actor::IsStatus)
		.def("IsDead", &Actor::IsDead)
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
		.def("AddInventoryItem", &Actor::AddInventoryItem, luabind::adopt(_2))
		.def("RemoveInventoryItem", (void (Actor::*)(const std::string &))&Actor::RemoveInventoryItem)
		.def("RemoveInventoryItem", (void (Actor::*)(const std::string &, const std::string &))&Actor::RemoveInventoryItem)
		.def("RemoveInventoryItemAtIndex", &Actor::RemoveInventoryItemAtIndex, luabind::adopt(luabind::return_value))
		.def("SwapNextInventory", &Actor::SwapNextInventory)
		.def("SwapPrevInventory", &Actor::SwapPrevInventory)
		.def("DropAllInventory", &Actor::DropAllInventory)
		.def("DropAllGold", &Actor::DropAllGold)
		.def("IsInventoryEmpty", &Actor::IsInventoryEmpty)
		.def("DrawWaypoints", &Actor::DrawWaypoints)
		.def("SetMovePathToUpdate", &Actor::SetMovePathToUpdate)
		.def("UpdateMovePath", &Actor::UpdateMovePath)
		.def("SetAlarmPoint", &Actor::AlarmPoint)
		.def("GetAlarmPoint", &Actor::GetAlarmPoint)
		.def("IsOrganic", &Actor::IsOrganic)
		.def("IsMechanical", &Actor::IsMechanical)

		.enum_("Status")[
			luabind::value("STABLE", Actor::Status::STABLE),
			luabind::value("UNSTABLE", Actor::Status::UNSTABLE),
			luabind::value("INACTIVE", Actor::Status::INACTIVE),
			luabind::value("DYING", Actor::Status::DYING),
			luabind::value("DEAD", Actor::Status::DEAD)
		]
		.enum_("MovementState")[
			luabind::value("NOMOVE", Actor::MovementState::NOMOVE),
			luabind::value("STAND", Actor::MovementState::STAND),
			luabind::value("WALK", Actor::MovementState::WALK),
			luabind::value("JUMP", Actor::MovementState::JUMP),
			luabind::value("DISLODGE", Actor::MovementState::DISLODGE),
			luabind::value("CROUCH", Actor::MovementState::CROUCH),
			luabind::value("CRAWL", Actor::MovementState::CRAWL),
			luabind::value("ARMCRAWL", Actor::MovementState::ARMCRAWL),
			luabind::value("CLIMB", Actor::MovementState::CLIMB),
			luabind::value("MOVEMENTSTATECOUNT", Actor::MovementState::MOVEMENTSTATECOUNT)
		]
		.enum_("AIMode")[
			luabind::value("AIMODE_NONE", Actor::AIMode::AIMODE_NONE),
			luabind::value("AIMODE_SENTRY", Actor::AIMode::AIMODE_SENTRY),
			luabind::value("AIMODE_PATROL", Actor::AIMode::AIMODE_PATROL),
			luabind::value("AIMODE_GOTO", Actor::AIMode::AIMODE_GOTO),
			luabind::value("AIMODE_BRAINHUNT", Actor::AIMode::AIMODE_BRAINHUNT),
			luabind::value("AIMODE_GOLDDIG", Actor::AIMode::AIMODE_GOLDDIG),
			luabind::value("AIMODE_RETURN", Actor::AIMode::AIMODE_RETURN),
			luabind::value("AIMODE_STAY", Actor::AIMode::AIMODE_STAY),
			luabind::value("AIMODE_SCUTTLE", Actor::AIMode::AIMODE_SCUTTLE),
			luabind::value("AIMODE_DELIVER", Actor::AIMode::AIMODE_DELIVER),
			luabind::value("AIMODE_BOMB", Actor::AIMode::AIMODE_BOMB),
			luabind::value("AIMODE_SQUAD", Actor::AIMode::AIMODE_SQUAD),
			luabind::value("AIMODE_COUNT", Actor::AIMode::AIMODE_COUNT)
		]
		.enum_("ActionState")[
			luabind::value("MOVING", Actor::ActionState::MOVING),
			luabind::value("MOVING_FAST", Actor::ActionState::MOVING_FAST),
			luabind::value("FIRING", Actor::ActionState::FIRING),
			luabind::value("ActionStateCount", Actor::ActionState::ActionStateCount)
		]
		.enum_("AimState")[
			luabind::value("AIMSTILL", Actor::AimState::AIMSTILL),
			luabind::value("AIMUP", Actor::AimState::AIMUP),
			luabind::value("AIMDOWN", Actor::AimState::AIMDOWN),
			luabind::value("AimStateCount", Actor::AimState::AimStateCount)
		]
		.enum_("LateralMoveState")[
			luabind::value("LAT_STILL", Actor::LateralMoveState::LAT_STILL),
			luabind::value("LAT_LEFT", Actor::LateralMoveState::LAT_LEFT),
			luabind::value("LAT_RIGHT", Actor::LateralMoveState::LAT_RIGHT)
		]
		.enum_("ObstacleState")[
			luabind::value("PROCEEDING", Actor::ObstacleState::PROCEEDING),
			luabind::value("BACKSTEPPING", Actor::ObstacleState::BACKSTEPPING),
			luabind::value("DIGPAUSING", Actor::ObstacleState::DIGPAUSING),
			luabind::value("JUMPING", Actor::ObstacleState::JUMPING),
			luabind::value("SOFTLANDING", Actor::ObstacleState::SOFTLANDING)
		]
		.enum_("TeamBlockState")[
			luabind::value("NOTBLOCKED", Actor::TeamBlockState::NOTBLOCKED),
			luabind::value("BLOCKED", Actor::TeamBlockState::BLOCKED),
			luabind::value("IGNORINGBLOCK", Actor::TeamBlockState::IGNORINGBLOCK),
			luabind::value("FOLLOWWAIT", Actor::TeamBlockState::FOLLOWWAIT)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ADoor) {
		return ConcreteTypeLuaClassDefinition(ADoor, Actor)

		.property("Door", &ADoor::GetDoor, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoor)
		.property("DoorMoveStartSound", &ADoor::GetDoorMoveStartSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoorMoveStartSound)
		.property("DoorMoveSound", &ADoor::GetDoorMoveSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoorMoveSound)
		.property("DoorDirectionChangeSound", &ADoor::GetDoorDirectionChangeSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoorDirectionChangeSound)
		.property("DoorMoveEndSound", &ADoor::GetDoorMoveEndSound, &LuaAdaptersPropertyOwnershipSafetyFaker::ADoorSetDoorMoveEndSound)

		.def("GetDoorState", &ADoor::GetDoorState)
		.def("OpenDoor", &ADoor::OpenDoor)
		.def("CloseDoor", &ADoor::CloseDoor)
		.def("StopDoor", &ADoor::StopDoor)
		.def("ResetSensorTimer", &ADoor::ResetSensorTimer)
		.def("SetClosedByDefault", &ADoor::SetClosedByDefault)

		.enum_("DoorState")[
			luabind::value("CLOSED", ADoor::DoorState::CLOSED),
			luabind::value("OPENING", ADoor::DoorState::OPENING),
			luabind::value("OPEN", ADoor::DoorState::OPEN),
			luabind::value("CLOSING", ADoor::DoorState::CLOSING),
			luabind::value("STOPPED", ADoor::DoorState::STOPPED)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, AEmitter) {
		return ConcreteTypeLuaClassDefinition(AEmitter, Attachable)

		.property("EmissionSound", &AEmitter::GetEmissionSound, &LuaAdaptersPropertyOwnershipSafetyFaker::AEmitterSetEmissionSound)
		.property("BurstSound", &AEmitter::GetBurstSound, &LuaAdaptersPropertyOwnershipSafetyFaker::AEmitterSetBurstSound)
		.property("EndSound", &AEmitter::GetEndSound, &LuaAdaptersPropertyOwnershipSafetyFaker::AEmitterSetEndSound)
		.property("BurstScale", &AEmitter::GetBurstScale, &AEmitter::SetBurstScale)
		.property("EmitAngle", &AEmitter::GetEmitAngle, &AEmitter::SetEmitAngle)
		.property("GetThrottle", &AEmitter::GetThrottle, &AEmitter::SetThrottle)
		.property("Throttle", &AEmitter::GetThrottle, &AEmitter::SetThrottle)
		.property("ThrottleFactor", &AEmitter::GetThrottleFactor)
		.property("NegativeThrottleMultiplier", &AEmitter::GetNegativeThrottleMultiplier, &AEmitter::SetNegativeThrottleMultiplier)
		.property("PositiveThrottleMultiplier", &AEmitter::GetPositiveThrottleMultiplier, &AEmitter::SetPositiveThrottleMultiplier)
		.property("BurstSpacing", &AEmitter::GetBurstSpacing, &AEmitter::SetBurstSpacing)
		.property("BurstDamage", &AEmitter::GetBurstDamage, &AEmitter::SetBurstDamage)
		.property("EmitterDamageMultiplier", &AEmitter::GetEmitterDamageMultiplier, &AEmitter::SetEmitterDamageMultiplier)
		.property("EmitCount", &AEmitter::GetEmitCount)
		.property("EmitCountLimit", &AEmitter::GetEmitCountLimit, &AEmitter::SetEmitCountLimit)
		.property("EmitDamage", &AEmitter::GetEmitDamage, &AEmitter::SetEmitDamage)
		.property("EmitOffset", &AEmitter::GetEmitOffset, &AEmitter::SetEmitOffset)
		.property("Flash", &AEmitter::GetFlash, &LuaAdaptersPropertyOwnershipSafetyFaker::AEmitterSetFlash)
		.property("FlashScale", &AEmitter::GetFlashScale, &AEmitter::SetFlashScale)
		.property("TotalParticlesPerMinute", &AEmitter::GetTotalParticlesPerMinute)
		.property("TotalBurstSize", &AEmitter::GetTotalBurstSize)

		.def_readwrite("Emissions", &AEmitter::m_EmissionList, luabind::return_stl_iterator)

		.def("IsEmitting", &AEmitter::IsEmitting)
		.def("WasEmitting", &AEmitter::WasEmitting)
		.def("EnableEmission", &AEmitter::EnableEmission)
		.def("GetEmitVector", &AEmitter::GetEmitVector)
		.def("GetRecoilVector", &AEmitter::GetRecoilVector)
		.def("EstimateImpulse", &AEmitter::EstimateImpulse)
		.def("TriggerBurst", &AEmitter::TriggerBurst)
		.def("IsSetToBurst", &AEmitter::IsSetToBurst)
		.def("CanTriggerBurst", &AEmitter::CanTriggerBurst)
		.def("GetScaledThrottle", &AEmitter::GetScaledThrottle)
		.def("JustStartedEmitting", &AEmitter::JustStartedEmitting);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, AEJetpack) {
		return ConcreteTypeLuaClassDefinition(AEJetpack, AEmitter)

		.property("JetpackType", &AEJetpack::GetJetpackType, &AEJetpack::SetJetpackType)
		.property("JetTimeTotal", &AEJetpack::GetJetTimeTotal, &AEJetpack::SetJetTimeTotal)
		.property("JetTimeLeft", &AEJetpack::GetJetTimeLeft)
		.property("JetReplenishRate", &AEJetpack::GetJetReplenishRate, &AEJetpack::SetJetReplenishRate)
		.property("MinimumFuelRatio", &AEJetpack::GetMinimumFuelRatio, &AEJetpack::SetMinimumFuelRatio)
		.property("JetAngleRange", &AEJetpack::GetJetAngleRange, &AEJetpack::SetJetAngleRange)
		.property("CanAdjustAngleWhileFiring", &AEJetpack::GetCanAdjustAngleWhileFiring, &AEJetpack::SetCanAdjustAngleWhileFiring)
		.property("AdjustsThrottleForWeight", &AEJetpack::GetAdjustsThrottleForWeight, &AEJetpack::SetAdjustsThrottleForWeight)

		.enum_("JetpackType")[
			luabind::value("Standard", AEJetpack::Standard),
			luabind::value("JumpPack", AEJetpack::JumpPack)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, AHuman) {
		return ConcreteTypeLuaClassDefinition(AHuman, Actor)

		.def(luabind::constructor<>())

		.property("Head", &AHuman::GetHead, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetHead)
		.property("Jetpack", &AHuman::GetJetpack, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetJetpack)
		.property("FGArm", &AHuman::GetFGArm, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetFGArm)
		.property("BGArm", &AHuman::GetBGArm, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetBGArm)
		.property("FGLeg", &AHuman::GetFGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetFGLeg)
		.property("BGLeg", &AHuman::GetBGLeg, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetBGLeg)
		.property("FGFoot", &AHuman::GetFGFoot, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetFGFoot)
		.property("BGFoot", &AHuman::GetBGFoot, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetBGFoot)
		.property("StrideSound", &AHuman::GetStrideSound, &LuaAdaptersPropertyOwnershipSafetyFaker::AHumanSetStrideSound)
		.property("UpperBodyState", &AHuman::GetUpperBodyState, &AHuman::SetUpperBodyState)
		.property("MovementState", &AHuman::GetMovementState, &AHuman::SetMovementState)
		.property("ProneState", &AHuman::GetProneState, &AHuman::SetProneState)
		.property("ThrowPrepTime", &AHuman::GetThrowPrepTime, &AHuman::SetThrowPrepTime)
		.property("ThrowProgress", &AHuman::GetThrowProgress)
		.property("EquippedItem", &AHuman::GetEquippedItem)
		.property("EquippedBGItem", &AHuman::GetEquippedBGItem)
		.property("EquippedMass", &AHuman::GetEquippedMass)
		.property("FirearmIsReady", &AHuman::FirearmIsReady)
		.property("ThrowableIsReady", &AHuman::ThrowableIsReady)
		.property("FirearmIsEmpty", &AHuman::FirearmIsEmpty)
		.property("FirearmNeedsReload", &AHuman::FirearmNeedsReload)
		.property("FirearmIsSemiAuto", &AHuman::FirearmIsSemiAuto)
		.property("FirearmActivationDelay", &AHuman::FirearmActivationDelay)
		.property("LimbPathPushForce", &AHuman::GetLimbPathPushForce, &AHuman::SetLimbPathPushForce)
		.property("IsClimbing", &AHuman::IsClimbing)
		.property("StrideFrame", &AHuman::StrideFrame)
		.property("ArmSwingRate", &AHuman::GetArmSwingRate, &AHuman::SetArmSwingRate)
		.property("DeviceArmSwayRate", &AHuman::GetDeviceArmSwayRate, &AHuman::SetDeviceArmSwayRate)

		.def("EquipFirearm", &AHuman::EquipFirearm)
		.def("EquipThrowable", &AHuman::EquipThrowable)
		.def("EquipDiggingTool", &AHuman::EquipDiggingTool)
		.def("EquipShield", &AHuman::EquipShield)
		.def("EquipShieldInBGArm", &AHuman::EquipShieldInBGArm)
		.def("EquipDeviceInGroup", &AHuman::EquipDeviceInGroup)
		.def("EquipNamedDevice", (bool (AHuman::*)(const std::string &, bool))&AHuman::EquipNamedDevice)
		.def("EquipNamedDevice", (bool (AHuman::*)(const std::string &, const std::string &, bool))&AHuman::EquipNamedDevice)
		.def("EquipLoadedFirearmInGroup", &AHuman::EquipLoadedFirearmInGroup)
		.def("UnequipFGArm", &AHuman::UnequipFGArm)
		.def("UnequipBGArm", &AHuman::UnequipBGArm)
		.def("UnequipArms", &AHuman::UnequipArms)
		.def("ReloadFirearms", &LuaAdaptersAHuman::ReloadFirearms)
		.def("ReloadFirearms", &AHuman::ReloadFirearms)
		.def("FirearmsAreReloading", &AHuman::FirearmsAreReloading)
		.def("IsWithinRange", &AHuman::IsWithinRange)
		.def("Look", &AHuman::Look)
		.def("LookForGold", &AHuman::LookForGold)
		.def("LookForMOs", &AHuman::LookForMOs)
		.def("GetLimbPath", &AHuman::GetLimbPath)
		.def("GetLimbPathSpeed", &AHuman::GetLimbPathSpeed)
		.def("SetLimbPathSpeed", &AHuman::SetLimbPathSpeed)
		.def("GetRotAngleTarget", &AHuman::GetRotAngleTarget)
		.def("SetRotAngleTarget", &AHuman::SetRotAngleTarget)
		.def("GetWalkAngle", &AHuman::GetWalkAngle)
		.def("SetWalkAngle", &AHuman::SetWalkAngle)

		.enum_("UpperBodyState")[
			luabind::value("WEAPON_READY", AHuman::UpperBodyState::WEAPON_READY),
			luabind::value("AIMING_SHARP", AHuman::UpperBodyState::AIMING_SHARP),
			luabind::value("HOLSTERING_BACK", AHuman::UpperBodyState::HOLSTERING_BACK),
			luabind::value("HOLSTERING_BELT", AHuman::UpperBodyState::HOLSTERING_BELT),
			luabind::value("DEHOLSTERING_BACK", AHuman::UpperBodyState::DEHOLSTERING_BACK),
			luabind::value("DEHOLSTERING_BELT", AHuman::UpperBodyState::DEHOLSTERING_BELT),
			luabind::value("THROWING_PREP", AHuman::UpperBodyState::THROWING_PREP),
			luabind::value("THROWING_RELEASE",AHuman::UpperBodyState::THROWING_RELEASE)
		]
		.enum_("ProneState")[
			luabind::value("NOTPRONE", AHuman::ProneState::NOTPRONE),
			luabind::value("GOPRONE", AHuman::ProneState::GOPRONE),
			luabind::value("PRONE", AHuman::ProneState::PRONE),
			luabind::value("PRONESTATECOUNT", AHuman::ProneState::PRONESTATECOUNT)
		]
		.enum_("Layer")[
			luabind::value("FGROUND", AHuman::Layer::FGROUND),
			luabind::value("BGROUND", AHuman::Layer::BGROUND)
		]
		.enum_("DeviceHandlingState")[
			luabind::value("STILL", AHuman::DeviceHandlingState::STILL),
			luabind::value("POINTING", AHuman::DeviceHandlingState::POINTING),
			luabind::value("SCANNING", AHuman::DeviceHandlingState::SCANNING),
			luabind::value("AIMING", AHuman::DeviceHandlingState::AIMING),
			luabind::value("FIRING", AHuman::DeviceHandlingState::FIRING),
			luabind::value("THROWING", AHuman::DeviceHandlingState::THROWING),
			luabind::value("DIGGING", AHuman::DeviceHandlingState::DIGGING)
		]
		.enum_("SweepState")[
			luabind::value("NOSWEEP", AHuman::SweepState::NOSWEEP),
			luabind::value("SWEEPINGUP", AHuman::SweepState::SWEEPINGUP),
			luabind::value("SWEEPUPPAUSE", AHuman::SweepState::SWEEPUPPAUSE),
			luabind::value("SWEEPINGDOWN", AHuman::SweepState::SWEEPINGDOWN),
			luabind::value("SWEEPDOWNPAUSE", AHuman::SweepState::SWEEPDOWNPAUSE)
		]
		.enum_("DigState")[
			luabind::value("NOTDIGGING", AHuman::DigState::NOTDIGGING),
			luabind::value("PREDIG", AHuman::DigState::PREDIG),
			luabind::value("STARTDIG", AHuman::DigState::STARTDIG),
			luabind::value("TUNNELING", AHuman::DigState::TUNNELING),
			luabind::value("FINISHINGDIG", AHuman::DigState::FINISHINGDIG),
			luabind::value("PAUSEDIGGER", AHuman::DigState::PAUSEDIGGER)
		]
		.enum_("JumpState")[
			luabind::value("NOTJUMPING", AHuman::JumpState::NOTJUMPING),
			luabind::value("FORWARDJUMP", AHuman::JumpState::FORWARDJUMP),
			luabind::value("PREJUMP", AHuman::JumpState::PREUPJUMP),
			luabind::value("UPJUMP", AHuman::JumpState::UPJUMP),
			luabind::value("APEXJUMP", AHuman::JumpState::APEXJUMP),
			luabind::value("LANDJUMP", AHuman::JumpState::LANDJUMP)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Arm) {
		return ConcreteTypeLuaClassDefinition(Arm, Attachable)

			.property("MaxLength", &Arm::GetMaxLength)
			.property("MoveSpeed", &Arm::GetMoveSpeed, &Arm::SetMoveSpeed)

			.property("HandIdleOffset", &Arm::GetHandIdleOffset, &Arm::SetHandIdleOffset)

			.property("HandPos", &Arm::GetHandPos, &Arm::SetHandPos)
			.property("HasAnyHandTargets", &Arm::HasAnyHandTargets)
			.property("NumberOfHandTargets", &Arm::GetNumberOfHandTargets)
			.property("NextHandTargetDescription", &Arm::GetNextHandTargetDescription)
			.property("NextHandTargetPosition", &Arm::GetNextHandTargetPosition)
			.property("HandHasReachedCurrentTarget", &Arm::GetHandHasReachedCurrentTarget)

			.property("GripStrength", &Arm::GetGripStrength, &Arm::SetGripStrength)
			.property("ThrowStrength", &Arm::GetThrowStrength, &Arm::SetThrowStrength)

			.property("HeldDevice", &Arm::GetHeldDevice, &LuaAdaptersPropertyOwnershipSafetyFaker::ArmSetHeldDevice)
			.property("SupportedHeldDevice", &Arm::GetHeldDeviceThisArmIsTryingToSupport)

			.def("AddHandTarget", (void (Arm::*)(const std::string &description, const Vector &handTargetPositionToAdd))&Arm::AddHandTarget)
			.def("AddHandTarget", (void (Arm::*)(const std::string &description, const Vector &handTargetPositionToAdd, float delayAtTarget))&Arm::AddHandTarget)
			.def("RemoveNextHandTarget", &Arm::RemoveNextHandTarget)
			.def("ClearHandTargets", &Arm::ClearHandTargets);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Attachable) {
		return ConcreteTypeLuaClassDefinition(Attachable, MOSRotating)

		.property("ParentOffset", &Attachable::GetParentOffset, &Attachable::SetParentOffset)
		.property("JointStrength", &Attachable::GetJointStrength, &Attachable::SetJointStrength)
		.property("JointStiffness", &Attachable::GetJointStiffness, &Attachable::SetJointStiffness)
		.property("JointOffset", &Attachable::GetJointOffset, &Attachable::SetJointOffset)
		.property("JointPos", &Attachable::GetJointPos)
		.property("DeleteWhenRemovedFromParent", &Attachable::GetDeleteWhenRemovedFromParent, &Attachable::SetDeleteWhenRemovedFromParent)
		.property("GibWhenRemovedFromParent", &Attachable::GetGibWhenRemovedFromParent, &Attachable::SetGibWhenRemovedFromParent)
		.property("ApplyTransferredForcesAtOffset", &Attachable::GetApplyTransferredForcesAtOffset, &Attachable::SetApplyTransferredForcesAtOffset)
		.property("BreakWound", &Attachable::GetBreakWound, &LuaAdaptersPropertyOwnershipSafetyFaker::AttachableSetBreakWound)
		.property("ParentBreakWound", &Attachable::GetParentBreakWound, &LuaAdaptersPropertyOwnershipSafetyFaker::AttachableSetParentBreakWound)
		.property("InheritsHFlipped", &Attachable::InheritsHFlipped, &Attachable::SetInheritsHFlipped)
		.property("InheritsRotAngle", &Attachable::InheritsRotAngle, &Attachable::SetInheritsRotAngle)
		.property("InheritedRotAngleOffset", &Attachable::GetInheritedRotAngleOffset, &Attachable::SetInheritedRotAngleOffset)
		.property("AtomSubgroupID", &Attachable::GetAtomSubgroupID)
		.property("CollidesWithTerrainWhileAttached", &Attachable::GetCollidesWithTerrainWhileAttached, &Attachable::SetCollidesWithTerrainWhileAttached)
		.property("IgnoresParticlesWhileAttached", &Attachable::GetIgnoresParticlesWhileAttached, &Attachable::SetIgnoresParticlesWhileAttached)
		.property("CanCollideWithTerrain", &Attachable::CanCollideWithTerrain)
		.property("DrawnAfterParent", &Attachable::IsDrawnAfterParent, &Attachable::SetDrawnAfterParent)
		.property("InheritsFrame", &Attachable::InheritsFrame, &Attachable::SetInheritsFrame)

		.def("IsAttached", &Attachable::IsAttached)
		.def("IsAttachedTo", &Attachable::IsAttachedTo)

		.def("RemoveFromParent", &LuaAdaptersAttachable::RemoveFromParent1, luabind::adopt(luabind::return_value))
		.def("RemoveFromParent", &LuaAdaptersAttachable::RemoveFromParent2, luabind::adopt(luabind::return_value));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Deployment) {
		return AbstractTypeLuaClassDefinition(Deployment, SceneObject)

		.property("ID", &Deployment::GetID)
		.property("HFlipped", &Deployment::IsHFlipped)
		.property("SpawnRadius", &Deployment::GetSpawnRadius)

		.def("GetLoadoutName", &Deployment::GetLoadoutName)
		.def("CreateDeployedActor", (Actor * (Deployment::*)())&Deployment::CreateDeployedActor, luabind::adopt(luabind::result))
		.def("CreateDeployedObject", (SceneObject * (Deployment::*)())&Deployment::CreateDeployedObject, luabind::adopt(luabind::result));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Emission) {
		return AbstractTypeLuaClassDefinition(Emission, Entity)

		.property("ParticlesPerMinute", &Emission::GetRate, &Emission::SetRate)
		.property("MinVelocity", &Emission::GetMinVelocity, &Emission::SetMinVelocity)
		.property("MaxVelocity", &Emission::GetMaxVelocity, &Emission::SetMaxVelocity)
		.property("PushesEmitter", &Emission::PushesEmitter, &Emission::SetPushesEmitter)
		.property("LifeVariation", &Emission::GetLifeVariation, &Emission::SetLifeVariation)
		.property("BurstSize", &Emission::GetBurstSize, &Emission::SetBurstSize)
		.property("Spread", &Emission::GetSpread, &Emission::SetSpread)
		.property("Offset", &Emission::GetOffset, &Emission::SetOffset)

		.def("ResetEmissionTimers", &Emission::ResetEmissionTimers);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Gib) {
		return luabind::class_<Gib>("Gib")

		.property("ParticlePreset", &Gib::GetParticlePreset, &Gib::SetParticlePreset)
		.property("MinVelocity", &Gib::GetMinVelocity, &Gib::SetMinVelocity)
		.property("MaxVelocity", &Gib::GetMaxVelocity, &Gib::SetMaxVelocity)
		.property("SpreadMode", &Gib::GetSpreadMode, &Gib::SetSpreadMode)

		.def_readwrite("Offset", &Gib::m_Offset)
		.def_readwrite("Count", &Gib::m_Count)
		.def_readwrite("Spread", &Gib::m_Spread)
		.def_readwrite("LifeVariation", &Gib::m_LifeVariation)
		.def_readwrite("InheritsVel", &Gib::m_InheritsVel)
		.def_readwrite("IgnoresTeamHits", &Gib::m_IgnoresTeamHits)

		.enum_("SpreadMode")[
			luabind::value("SpreadRandom", Gib::SpreadMode::SpreadRandom),
			luabind::value("SpreadEven", Gib::SpreadMode::SpreadEven),
			luabind::value("SpreadSpiral", Gib::SpreadMode::SpreadSpiral)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, GlobalScript) {
		return AbstractTypeLuaClassDefinition(GlobalScript, Entity)

		.def("Deactivate", &LuaAdaptersGlobalScript::Deactivate);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, HDFirearm) {
		return ConcreteTypeLuaClassDefinition(HDFirearm, HeldDevice)

		.property("ReloadEndOffset", &HDFirearm::GetReloadEndOffset, &HDFirearm::SetReloadEndOffset)
		.property("RateOfFire", &HDFirearm::GetRateOfFire, &HDFirearm::SetRateOfFire)
		.property("MSPerRound", &HDFirearm::GetMSPerRound)
		.property("FullAuto", &HDFirearm::IsFullAuto, &HDFirearm::SetFullAuto)
		.property("Reloadable", &HDFirearm::IsReloadable, &HDFirearm::SetReloadable)
		.property("DualReloadable", &HDFirearm::IsDualReloadable, &HDFirearm::SetDualReloadable)
		.property("OneHandedReloadTimeMultiplier", &HDFirearm::GetOneHandedReloadTimeMultiplier, &HDFirearm::SetOneHandedReloadTimeMultiplier)
		.property("ReloadAngle", &HDFirearm::GetReloadAngle, &HDFirearm::SetReloadAngle)
		.property("OneHandedReloadAngle", &HDFirearm::GetOneHandedReloadAngle, &HDFirearm::SetOneHandedReloadAngle)
		.property("CurrentReloadAngle", &HDFirearm::GetCurrentReloadAngle)
		.property("RoundInMagCount", &HDFirearm::GetRoundInMagCount)
		.property("RoundInMagCapacity", &HDFirearm::GetRoundInMagCapacity)
		.property("Magazine", &HDFirearm::GetMagazine, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetMagazine)
		.property("Flash", &HDFirearm::GetFlash, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetFlash)
		.property("PreFireSound", &HDFirearm::GetPreFireSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetPreFireSound)
		.property("FireSound", &HDFirearm::GetFireSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetFireSound)
		.property("FireEchoSound", &HDFirearm::GetFireEchoSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetFireEchoSound)
		.property("ActiveSound", &HDFirearm::GetActiveSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetActiveSound)
		.property("DeactivationSound", &HDFirearm::GetDeactivationSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetDeactivationSound)
		.property("EmptySound", &HDFirearm::GetEmptySound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetEmptySound)
		.property("ReloadStartSound", &HDFirearm::GetReloadStartSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetReloadStartSound)
		.property("ReloadEndSound", &HDFirearm::GetReloadEndSound, &LuaAdaptersPropertyOwnershipSafetyFaker::HDFirearmSetReloadEndSound)
		.property("ActivationDelay", &HDFirearm::GetActivationDelay, &HDFirearm::SetActivationDelay)
		.property("DeactivationDelay", &HDFirearm::GetDeactivationDelay, &HDFirearm::SetDeactivationDelay)
		.property("BaseReloadTime", &HDFirearm::GetBaseReloadTime, &HDFirearm::SetBaseReloadTime)
		.property("ReloadTime", &HDFirearm::GetReloadTime)
		.property("ReloadProgress", &HDFirearm::GetReloadProgress)
		.property("ShakeRange", &HDFirearm::GetShakeRange, &HDFirearm::SetShakeRange)
		.property("SharpShakeRange", &HDFirearm::GetSharpShakeRange, &HDFirearm::SetSharpShakeRange)
		.property("NoSupportFactor", &HDFirearm::GetNoSupportFactor, &HDFirearm::SetNoSupportFactor)
		.property("ParticleSpreadRange", &HDFirearm::GetParticleSpreadRange, &HDFirearm::SetParticleSpreadRange)
		.property("ShellVelVariation", &HDFirearm::GetShellVelVariation, &HDFirearm::SetShellVelVariation)
		.property("FiredOnce", &HDFirearm::FiredOnce)
		.property("FiredFrame", &HDFirearm::FiredFrame)
		.property("CanFire", &HDFirearm::CanFire)
		.property("RoundsFired", &HDFirearm::RoundsFired)
		.property("IsAnimatedManually", &HDFirearm::IsAnimatedManually, &HDFirearm::SetAnimatedManually)
		.property("RecoilTransmission", &HDFirearm::GetJointStiffness, &HDFirearm::SetJointStiffness)

		.def("GetAIFireVel", &HDFirearm::GetAIFireVel)
		.def("GetAIBulletLifeTime", &HDFirearm::GetAIBulletLifeTime)
		.def("GetBulletAccScalar", &HDFirearm::GetBulletAccScalar)
		.def("GetAIBlastRadius", &HDFirearm::GetAIBlastRadius)
		.def("GetAIPenetration", &HDFirearm::GetAIPenetration)
		.def("CompareTrajectories", &HDFirearm::CompareTrajectories)
		.def("GetNextMagazineName", &HDFirearm::GetNextMagazineName)
		.def("SetNextMagazineName", &HDFirearm::SetNextMagazineName);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, HeldDevice) {
		return ConcreteTypeLuaClassDefinition(HeldDevice, Attachable)

		.property("SupportPos", &HeldDevice::GetSupportPos)
		.property("MagazinePos", &HeldDevice::GetMagazinePos)
		.property("MuzzlePos", &HeldDevice::GetMuzzlePos)
		.property("MuzzleOffset", &HeldDevice::GetMuzzleOffset, &HeldDevice::SetMuzzleOffset)
		.property("StanceOffset", &HeldDevice::GetStanceOffset, &HeldDevice::SetStanceOffset)
		.property("SharpStanceOffset", &HeldDevice::GetSharpStanceOffset, &HeldDevice::SetSharpStanceOffset)
		.property("SharpLength", &HeldDevice::GetSharpLength, &HeldDevice::SetSharpLength)
		.property("SharpLength", &HeldDevice::GetSharpLength, &HeldDevice::SetSharpLength)
		.property("Supportable", &HeldDevice::IsSupportable, &HeldDevice::SetSupportable)
		.property("SupportOffset", &HeldDevice::GetSupportOffset, &HeldDevice::SetSupportOffset)
		.property("UseSupportOffsetWhileReloading", &HeldDevice::GetUseSupportOffsetWhileReloading, &HeldDevice::SetUseSupportOffsetWhileReloading)
		.property("HasPickupLimitations", &HeldDevice::HasPickupLimitations)
		.property("UnPickupable", &HeldDevice::IsUnPickupable, &HeldDevice::SetUnPickupable)
		.property("GripStrengthMultiplier", &HeldDevice::GetGripStrengthMultiplier, &HeldDevice::SetGripStrengthMultiplier)
		.property("Supported", &HeldDevice::GetSupported, &HeldDevice::SetSupported)
		.property("GetsHitByMOsWhenHeld", &HeldDevice::GetsHitByMOsWhenHeld, &HeldDevice::SetGetsHitByMOsWhenHeld)
		.property("VisualRecoilMultiplier", &HeldDevice::GetVisualRecoilMultiplier, &HeldDevice::SetVisualRecoilMultiplier)

		.def("IsBeingHeld", &HeldDevice::IsBeingHeld)
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
		.def("IsEmpty", &HeldDevice::IsEmpty)
		.def("IsPickupableBy", &HeldDevice::IsPickupableBy)
		.def("AddPickupableByPresetName", &HeldDevice::AddPickupableByPresetName)
		.def("RemovePickupableByPresetName", &HeldDevice::RemovePickupableByPresetName);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Leg) {
		return ConcreteTypeLuaClassDefinition(Leg, Attachable)

		.property("Foot", &Leg::GetFoot, &LuaAdaptersPropertyOwnershipSafetyFaker::LegSetFoot)
		.property("MoveSpeed", &Leg::GetMoveSpeed, &Leg::SetMoveSpeed);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, LimbPath) {
		return luabind::class_<LimbPath>("LimbPath")

		.property("StartOffset", &LimbPath::GetStartOffset, &LimbPath::SetStartOffset)
		.property("SegmentCount", &LimbPath::GetSegCount)

		.def("GetSegment", &LimbPath::GetSegment);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Magazine) {
		return ConcreteTypeLuaClassDefinition(Magazine, Attachable)

		.property("NextRound", &Magazine::GetNextRound)
		.property("RoundCount", &Magazine::GetRoundCount, &Magazine::SetRoundCount)
		.property("IsEmpty", &Magazine::IsEmpty)
		.property("IsFull", &Magazine::IsFull)
		.property("IsOverHalfFull", &Magazine::IsOverHalfFull)
		.property("Capacity", &Magazine::GetCapacity)
		.property("Discardable", &Magazine::IsDiscardable);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Material) {
		return luabind::class_<Material, Entity>("Material")

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
		.property("IsScrap", &Material::IsScrap);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MetaPlayer) {
		return luabind::class_<MetaPlayer>("MetaPlayer")

		.def(luabind::constructor<>())

		.property("NativeTechModule", &MetaPlayer::GetNativeTechModule)
		.property("ForeignCostMultiplier", &MetaPlayer::GetForeignCostMultiplier)
		.property("NativeCostMultiplier", &MetaPlayer::GetNativeCostMultiplier)
		.property("InGamePlayer", &MetaPlayer::GetInGamePlayer)
		.property("BrainPoolCount", &MetaPlayer::GetBrainPoolCount, &MetaPlayer::SetBrainPoolCount)

		.def("ChangeBrainPoolCount", &MetaPlayer::ChangeBrainPoolCount);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MOPixel) {
		return ConcreteTypeLuaClassDefinition(MOPixel, MovableObject)

		.property("TrailLength", &MOPixel::GetTrailLength, &MOPixel::SetTrailLength)
		.property("Staininess", &MOPixel::GetStaininess, &MOPixel::SetStaininess);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MOSParticle) {
		return ConcreteTypeLuaClassDefinition(MOSParticle, MOSprite);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MOSprite) {
		return AbstractTypeLuaClassDefinition(MOSprite, MovableObject)

		.property("Diameter", &MOSprite::GetDiameter)
		.property("BoundingBox", &MOSprite::GetBoundingBox)
		.property("FrameCount", &MOSprite::GetFrameCount)
		.property("SpriteOffset", &MOSprite::GetSpriteOffset, &MOSprite::SetSpriteOffset)
		.property("HFlipped", &MOSprite::IsHFlipped, &MOSprite::SetHFlipped)
		.property("FlipFactor", &MOSprite::GetFlipFactor)
		.property("RotAngle", &MOSprite::GetRotAngle, &MOSprite::SetRotAngle)
		.property("PrevRotAngle", &MOSprite::GetPrevRotAngle)
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
		.def("GetIconWidth", &MOSprite::GetIconWidth)
		.def("GetIconHeight", &MOSprite::GetIconHeight)
		.def("SetEntryWound", &MOSprite::SetEntryWound)
		.def("SetExitWound", &MOSprite::SetExitWound)
		.def("GetEntryWoundPresetName", &MOSprite::GetEntryWoundPresetName)
		.def("GetExitWoundPresetName", &MOSprite::GetExitWoundPresetName)

		.enum_("SpriteAnimMode")[
			luabind::value("NOANIM", SpriteAnimMode::NOANIM),
			luabind::value("ALWAYSLOOP", SpriteAnimMode::ALWAYSLOOP),
			luabind::value("ALWAYSRANDOM", SpriteAnimMode::ALWAYSRANDOM),
			luabind::value("ALWAYSPINGPONG", SpriteAnimMode::ALWAYSPINGPONG),
			luabind::value("LOOPWHENACTIVE", SpriteAnimMode::LOOPWHENACTIVE),
			luabind::value("LOOPWHENOPENCLOSE", SpriteAnimMode::LOOPWHENOPENCLOSE),
			luabind::value("PINGPONGOPENCLOSE", SpriteAnimMode::PINGPONGOPENCLOSE),
			luabind::value("OVERLIFETIME", SpriteAnimMode::OVERLIFETIME),
			luabind::value("ONCOLLIDE", SpriteAnimMode::ONCOLLIDE)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MOSRotating) {
		return ConcreteTypeLuaClassDefinition(MOSRotating, MOSprite)

		//.property("Material", &MOSRotating::GetMaterial)
		.property("IndividualRadius", &MOSRotating::GetIndividualRadius)
		.property("IndividualDiameter", &MOSRotating::GetIndividualDiameter)
		.property("IndividualMass", &MOSRotating::GetIndividualMass)
		.property("RecoilForce", &MOSRotating::GetRecoilForce)
		.property("RecoilOffset", &MOSRotating::GetRecoilOffset)
		.property("TravelImpulse", &MOSRotating::GetTravelImpulse, &MOSRotating::SetTravelImpulse)
		.property("GibWoundLimit", (int (MOSRotating:: *)() const) &MOSRotating::GetGibWoundLimit, &MOSRotating::SetGibWoundLimit)
		.property("GibSound", &MOSRotating::GetGibSound, &LuaAdaptersPropertyOwnershipSafetyFaker::MOSRotatingSetGibSound)
		.property("GibImpulseLimit", &MOSRotating::GetGibImpulseLimit, &MOSRotating::SetGibImpulseLimit)
		.property("WoundCountAffectsImpulseLimitRatio", &MOSRotating::GetWoundCountAffectsImpulseLimitRatio)
		.property("GibAtEndOfLifetime", &MOSRotating::GetGibAtEndOfLifetime, &MOSRotating::SetGibAtEndOfLifetime)
		.property("DamageMultiplier", &MOSRotating::GetDamageMultiplier, &MOSRotating::SetDamageMultiplier)
		.property("WoundCount", (int (MOSRotating:: *)() const) &MOSRotating::GetWoundCount)
		.property("OrientToVel", &MOSRotating::GetOrientToVel, &MOSRotating::SetOrientToVel)

		.def_readonly("Attachables", &MOSRotating::m_Attachables, luabind::return_stl_iterator)
		.def_readonly("Wounds", &MOSRotating::m_Wounds, luabind::return_stl_iterator)
		.def_readonly("Gibs", &MOSRotating::m_Gibs, luabind::return_stl_iterator)

		.def("AddRecoil", &MOSRotating::AddRecoil)
		.def("SetRecoil", &MOSRotating::SetRecoil)
		.def("IsRecoiled", &MOSRotating::IsRecoiled)
		.def("EnableDeepCheck", &MOSRotating::EnableDeepCheck)
		.def("ForceDeepCheck", &MOSRotating::ForceDeepCheck)
		.def("GibThis", &MOSRotating::GibThis)
		.def("MoveOutOfTerrain", &MOSRotating::MoveOutOfTerrain)
		.def("FlashWhite", &MOSRotating::FlashWhite)
		.def("GetGibWoundLimit", (int (MOSRotating:: *)() const) &MOSRotating::GetGibWoundLimit)
		.def("GetGibWoundLimit", (int (MOSRotating:: *)(bool positiveDamage, bool negativeDamage, bool noDamage) const) &MOSRotating::GetGibWoundLimit)
		.def("GetWoundCount", (int (MOSRotating:: *)() const) &MOSRotating::GetWoundCount)
		.def("GetWoundCount", (int (MOSRotating:: *)(bool positiveDamage, bool negativeDamage, bool noDamage) const) &MOSRotating::GetWoundCount)
		.def("GetWounds", &LuaAdaptersMOSRotating::GetWounds1, luabind::adopt(luabind::return_value) + luabind::return_stl_iterator)
		.def("GetWounds", &LuaAdaptersMOSRotating::GetWounds2, luabind::adopt(luabind::return_value) + luabind::return_stl_iterator)
		.def("AddWound", &MOSRotating::AddWound, luabind::adopt(_2))
		.def("RemoveWounds", (float (MOSRotating:: *)(int numberOfWoundsToRemove)) &MOSRotating::RemoveWounds)
		.def("RemoveWounds", (float (MOSRotating:: *)(int numberOfWoundsToRemove, bool positiveDamage, bool negativeDamage, bool noDamage)) &MOSRotating::RemoveWounds)
		.def("IsOnScenePoint", &MOSRotating::IsOnScenePoint)
		.def("EraseFromTerrain", &MOSRotating::EraseFromTerrain)
		.def("AddAttachable", (void (MOSRotating::*)(Attachable *attachableToAdd))&MOSRotating::AddAttachable, luabind::adopt(_2))
		.def("AddAttachable", (void (MOSRotating::*)(Attachable *attachableToAdd, const Vector &parentOffset))&MOSRotating::AddAttachable, luabind::adopt(_2))
		.def("RemoveAttachable", (Attachable *(MOSRotating:: *)(long uniqueIDOfAttachableToRemove)) &MOSRotating::RemoveAttachable, luabind::adopt(luabind::return_value))
		.def("RemoveAttachable", (Attachable *(MOSRotating:: *)(long uniqueIDOfAttachableToRemove, bool addToMovableMan, bool addBreakWounds)) &MOSRotating::RemoveAttachable, luabind::adopt(luabind::return_value))
		.def("RemoveAttachable", (Attachable *(MOSRotating:: *)(Attachable *attachableToRemove))&MOSRotating::RemoveAttachable, luabind::adopt(luabind::return_value))
		.def("RemoveAttachable", (Attachable *(MOSRotating:: *)(Attachable *attachableToRemove, bool addToMovableMan, bool addBreakWounds)) &MOSRotating::RemoveAttachable)
		.def("AddEmitter", (void (MOSRotating::*)(Attachable *attachableToAdd))&MOSRotating::AddAttachable, luabind::adopt(_2))
		.def("AddEmitter", (void (MOSRotating::*)(Attachable *attachableToAdd, const Vector &parentOffset))&MOSRotating::AddAttachable, luabind::adopt(_2))
		.def("RemoveEmitter", (Attachable *(MOSRotating:: *)(long uniqueIDOfAttachableToRemove)) &MOSRotating::RemoveAttachable, luabind::adopt(luabind::return_value))
		.def("RemoveEmitter", (Attachable *(MOSRotating:: *)(long uniqueIDOfAttachableToRemove, bool addToMovableMan, bool addBreakWounds)) &MOSRotating::RemoveAttachable, luabind::adopt(luabind::return_value))
		.def("RemoveEmitter", (Attachable *(MOSRotating:: *)(Attachable *attachableToRemove)) &MOSRotating::RemoveAttachable, luabind::adopt(luabind::return_value))
		.def("RemoveEmitter", (Attachable *(MOSRotating:: *)(Attachable *attachableToRemove, bool addToMovableMan, bool addBreakWounds)) &MOSRotating::RemoveAttachable, luabind::adopt(luabind::return_value))

		.def("GibThis", &LuaAdaptersMOSRotating::GibThis);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MovableObject) {
		return AbstractTypeLuaClassDefinition(MovableObject, SceneObject)

		.property("Material", &MovableObject::GetMaterial)
		.property("Mass", &MovableObject::GetMass, &MovableObject::SetMass)
		.property("Pos", &MovableObject::GetPos, &MovableObject::SetPosLuaBinding)
		.property("Vel", &MovableObject::GetVel, &MovableObject::SetVel)
		.property("PrevPos", &MovableObject::GetPrevPos)
		.property("PrevVel", &MovableObject::GetPrevVel)
		.property("DistanceTravelled", &MovableObject::GetDistanceTravelled)
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
		.property("HasEverBeenAddedToMovableMan", &MovableObject::HasEverBeenAddedToMovableMan)
		.property("AboveHUDPos", &MovableObject::GetAboveHUDPos)
		.property("HitsMOs", &MovableObject::HitsMOs, &MovableObject::SetToHitMOs)
		.property("GetsHitByMOs", &MovableObject::GetsHitByMOs, &MovableObject::SetToGetHitByMOs)
		.property("IgnoresTeamHits", &MovableObject::IgnoresTeamHits, &MovableObject::SetIgnoresTeamHits)
		.property("IgnoresWhichTeam", &MovableObject::IgnoresWhichTeam)
		.property("IgnoreTerrain", &MovableObject::IgnoreTerrain, &MovableObject::SetIgnoreTerrain)
		.property("IgnoresActorHits", &MovableObject::GetIgnoresActorHits, &MovableObject::SetIgnoresActorHits)
		.property("ToSettle", &MovableObject::ToSettle, &MovableObject::SetToSettle)
		.property("ToDelete", &MovableObject::ToDelete, &MovableObject::SetToDelete)
		.property("MissionCritical", &MovableObject::IsMissionCritical, &MovableObject::SetMissionCritical)
		.property("HUDVisible", &MovableObject::GetHUDVisible, &MovableObject::SetHUDVisible)
		.property("PinStrength", &MovableObject::GetPinStrength, &MovableObject::SetPinStrength)
		.property("RestThreshold", &MovableObject::GetRestThreshold, &MovableObject::SetRestThreshold)
		.property("DamageOnCollision", &MovableObject::DamageOnCollision, &MovableObject::SetDamageOnCollision)
		.property("DamageOnPenetration", &MovableObject::DamageOnPenetration, &MovableObject::SetDamageOnPenetration)
		.property("WoundDamageMultiplier", &MovableObject::WoundDamageMultiplier, &MovableObject::SetWoundDamageMultiplier)
		.property("HitWhatMOID", &MovableObject::HitWhatMOID)
		.property("HitWhatTerrMaterial", &MovableObject::HitWhatTerrMaterial)
		.property("HitWhatParticleUniqueID", &MovableObject::HitWhatParticleUniqueID)
		.property("ApplyWoundDamageOnCollision", &MovableObject::GetApplyWoundDamageOnCollision, &MovableObject::SetApplyWoundDamageOnCollision)
		.property("ApplyWoundBurstDamageOnCollision", &MovableObject::GetApplyWoundBurstDamageOnCollision, &MovableObject::SetApplyWoundBurstDamageOnCollision)
		.property("SimUpdatesBetweenScriptedUpdates", &MovableObject::GetSimUpdatesBetweenScriptedUpdates, &MovableObject::SetSimUpdatesBetweenScriptedUpdates)

		.def("GetParent", (MOSRotating * (MovableObject::*)())&MovableObject::GetParent)
		.def("GetParent", (const MOSRotating * (MovableObject::*)() const)&MovableObject::GetParent)
		.def("GetRootParent", (MovableObject * (MovableObject::*)())&MovableObject::GetRootParent)
		.def("GetRootParent", (const MovableObject * (MovableObject::*)() const)&MovableObject::GetRootParent)
		.def("ReloadScripts", &MovableObject::ReloadScripts)
		.def("HasScript", &LuaAdaptersMovableObject::HasScript)
		.def("AddScript", &LuaAdaptersMovableObject::AddScript)
		.def("ScriptEnabled", &MovableObject::ScriptEnabled)
		.def("EnableScript", &LuaAdaptersMovableObject::EnableScript)
		.def("DisableScript", &LuaAdaptersMovableObject::DisableScript)
		.def("EnableOrDisableAllScripts", &MovableObject::EnableOrDisableAllScripts)
		.def("GetStringValue", &MovableObject::GetStringValue)
		.def("GetEncodedStringValue", &MovableObject::GetEncodedStringValue)
		.def("GetNumberValue", &MovableObject::GetNumberValue)
		.def("GetObjectValue", &MovableObject::GetObjectValue)
		.def("SetStringValue", &MovableObject::SetStringValue)
		.def("SetEncodedStringValue", &MovableObject::SetEncodedStringValue)
		.def("SetNumberValue", &MovableObject::SetNumberValue)
		.def("SetObjectValue", &MovableObject::SetObjectValue)
		.def("RemoveStringValue", &MovableObject::RemoveStringValue)
		.def("RemoveNumberValue", &MovableObject::RemoveNumberValue)
		.def("RemoveObjectValue", &MovableObject::RemoveObjectValue)
		.def("StringValueExists", &MovableObject::StringValueExists)
		.def("NumberValueExists", &MovableObject::NumberValueExists)
		.def("ObjectValueExists", &MovableObject::ObjectValueExists)
		.def("GetAltitude", &MovableObject::GetAltitude)
		.def("GetWhichMOToNotHit", &MovableObject::GetWhichMOToNotHit)
		.def("SetWhichMOToNotHit", &MovableObject::SetWhichMOToNotHit)
		.def("IsSetToDelete", &MovableObject::IsSetToDelete)
		.def("IsMissionCritical", &MovableObject::IsMissionCritical)
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
		.def("RestDetection", &MovableObject::RestDetection)
		.def("NotResting", &MovableObject::NotResting)
		.def("IsAtRest", &MovableObject::IsAtRest)
		.def("MoveOutOfTerrain", &MovableObject::MoveOutOfTerrain)
		.def("RotateOffset", &MovableObject::RotateOffset)
		.def("SendMessage", &LuaAdaptersMovableObject::SendMessage1)
		.def("SendMessage", &LuaAdaptersMovableObject::SendMessage2)
		.def("RequestSyncedUpdate", &MovableObject::RequestSyncedUpdate);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, PEmitter) {
		return ConcreteTypeLuaClassDefinition(PEmitter, MOSParticle)

		.property("BurstScale", &PEmitter::GetBurstScale, &PEmitter::SetBurstScale)
		.property("EmitAngle", &PEmitter::GetEmitAngle, &PEmitter::SetEmitAngle)
		.property("GetThrottle", &PEmitter::GetThrottle, &PEmitter::SetThrottle)
		.property("Throttle", &PEmitter::GetThrottle, &PEmitter::SetThrottle)
		.property("ThrottleFactor", &PEmitter::GetThrottleFactor)
		.property("BurstSpacing", &PEmitter::GetBurstSpacing, &PEmitter::SetBurstSpacing)
		.property("EmitCountLimit", &PEmitter::GetEmitCountLimit, &PEmitter::SetEmitCountLimit)
		.property("FlashScale", &PEmitter::GetFlashScale, &PEmitter::SetFlashScale)

		.def_readwrite("Emissions", &PEmitter::m_EmissionList, luabind::return_stl_iterator)

		.def("IsEmitting", &PEmitter::IsEmitting)
		.def("WasEmitting", &PEmitter::WasEmitting)
		.def("EnableEmission", &PEmitter::EnableEmission)
		.def("GetEmitVector", &PEmitter::GetEmitVector)
		.def("GetRecoilVector", &PEmitter::GetRecoilVector)
		.def("EstimateImpulse", &PEmitter::EstimateImpulse)
		.def("TriggerBurst", &PEmitter::TriggerBurst)
		.def("IsSetToBurst", &PEmitter::IsSetToBurst)
		.def("CanTriggerBurst", &PEmitter::CanTriggerBurst)
		.def("JustStartedEmitting", &PEmitter::JustStartedEmitting);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, PieSlice) {
		return ConcreteTypeLuaClassDefinition(PieSlice, Entity)

		.property("Type", &PieSlice::GetType, &PieSlice::SetType)
		.property("Direction", &PieSlice::GetDirection, &PieSlice::SetDirection)
		.property("CanBeMiddleSlice", &PieSlice::GetCanBeMiddleSlice, &PieSlice::SetCanBeMiddleSlice)
		.property("OriginalSource", &PieSlice::GetOriginalSource)
		.property("Enabled", &PieSlice::IsEnabled, &PieSlice::SetEnabled)

		.property("ScriptPath", &PieSlice::GetScriptPath, &PieSlice::SetScriptPath)
		.property("FunctionName", &PieSlice::GetFunctionName, &PieSlice::SetFunctionName)
		.property("SubPieMenu", &PieSlice::GetSubPieMenu, &PieSlice::SetSubPieMenu)

		.property("DrawFlippedToMatchAbsoluteAngle", &PieSlice::GetDrawFlippedToMatchAbsoluteAngle, &PieSlice::SetDrawFlippedToMatchAbsoluteAngle)

		.def("ReloadScripts", &PieSlice::ReloadScripts)

		.enum_("SliceType")[
			luabind::value("NoType", PieSlice::SliceType::NoType),
			luabind::value("Pickup", PieSlice::SliceType::Pickup),
			luabind::value("Drop", PieSlice::SliceType::Drop),
			luabind::value("NextItem", PieSlice::SliceType::NextItem),
			luabind::value("PreviousItem", PieSlice::SliceType::PreviousItem),
			luabind::value("Reload", PieSlice::SliceType::Reload),
			luabind::value("BuyMenu", PieSlice::SliceType::BuyMenu),
			luabind::value("Stats", PieSlice::SliceType::Stats),
			luabind::value("Map", PieSlice::SliceType::Map),
			luabind::value("FormSquad", PieSlice::SliceType::FormSquad),
			luabind::value("Ceasefire", PieSlice::SliceType::Ceasefire),
			luabind::value("Sentry", PieSlice::SliceType::Sentry),
			luabind::value("Patrol", PieSlice::SliceType::Patrol),
			luabind::value("BrainHunt", PieSlice::SliceType::BrainHunt),
			luabind::value("GoldDig", PieSlice::SliceType::GoldDig),
			luabind::value("GoTo", PieSlice::SliceType::GoTo),
			luabind::value("Return", PieSlice::SliceType::Return),
			luabind::value("Stay", PieSlice::SliceType::Stay),
			luabind::value("Deliver", PieSlice::SliceType::Deliver),
			luabind::value("Scuttle", PieSlice::SliceType::Scuttle),
			luabind::value("Done", PieSlice::SliceType::EditorDone),
			luabind::value("Load", PieSlice::SliceType::EditorLoad),
			luabind::value("Save", PieSlice::SliceType::EditorSave),
			luabind::value("New", PieSlice::SliceType::EditorNew),
			luabind::value("Pick", PieSlice::SliceType::EditorPick),
			luabind::value("Move", PieSlice::SliceType::EditorMove),
			luabind::value("Remove", PieSlice::SliceType::EditorRemove),
			luabind::value("InFront", PieSlice::SliceType::EditorInFront),
			luabind::value("Behind", PieSlice::SliceType::EditorBehind),
			luabind::value("ZoomIn", PieSlice::SliceType::EditorZoomIn),
			luabind::value("ZoomOut", PieSlice::SliceType::EditorZoomOut),
			luabind::value("Team1", PieSlice::SliceType::EditorTeam1),
			luabind::value("Team2", PieSlice::SliceType::EditorTeam2),
			luabind::value("Team3", PieSlice::SliceType::EditorTeam3),
			luabind::value("Team4", PieSlice::SliceType::EditorTeam4)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, PieMenu) {
		return ConcreteTypeLuaClassDefinition(PieMenu, Entity)

		.property("Owner", &PieMenu::GetOwner)
		.property("Controller", &PieMenu::GetController)
		.property("AffectedObject", &PieMenu::GetAffectedObject)
		.property("Pos", &PieMenu::GetPos)
		.property("RotAngle", &PieMenu::GetRotAngle, &PieMenu::SetRotAngle)
		.property("FullInnerRadius", &PieMenu::GetFullInnerRadius, &PieMenu::SetFullInnerRadius)

		.property("PieSlices", &PieMenu::GetPieSlices, luabind::return_stl_iterator)

		.def("IsSubPieMenu", &PieMenu::IsSubPieMenu)

		.def("IsEnabled", &PieMenu::IsEnabled)
		.def("IsEnabling", &PieMenu::IsEnabling)
		.def("IsDisabling", &PieMenu::IsDisabling)
		.def("IsEnablingOrDisabling", &PieMenu::IsEnablingOrDisabling)
		.def("IsVisible", &PieMenu::IsVisible)
		.def("HasSubPieMenuOpen", &PieMenu::HasSubPieMenuOpen)

		.def("SetAnimationModeToNormal", &PieMenu::SetAnimationModeToNormal)
		.def("DoDisableAnimation", &PieMenu::DoDisableAnimation)
		.def("Wobble", &PieMenu::Wobble)
		.def("FreezeAtRadius", &PieMenu::FreezeAtRadius)

		.def("GetPieCommand", &PieMenu::GetPieCommand)
		.def("GetFirstPieSliceByPresetName", &PieMenu::GetFirstPieSliceByPresetName)
		.def("GetFirstPieSliceByType", &PieMenu::GetFirstPieSliceByType)
		.def("AddPieSlice", &PieMenu::AddPieSlice, luabind::adopt(_2))
		.def("AddPieSlice", &LuaAdaptersPieMenu::AddPieSlice, luabind::adopt(_2))
		.def("AddPieSliceIfPresetNameIsUnique", &PieMenu::AddPieSliceIfPresetNameIsUnique)
		.def("AddPieSliceIfPresetNameIsUnique", &LuaAdaptersPieMenu::AddPieSliceIfPresetNameIsUnique1)
		.def("AddPieSliceIfPresetNameIsUnique", &LuaAdaptersPieMenu::AddPieSliceIfPresetNameIsUnique2)
		.def("RemovePieSlice", &PieMenu::RemovePieSlice, luabind::adopt(luabind::return_value))
		.def("RemovePieSlicesByPresetName", &PieMenu::RemovePieSlicesByPresetName)
		.def("RemovePieSlicesByType", &PieMenu::RemovePieSlicesByType)
		.def("RemovePieSlicesByOriginalSource", &PieMenu::RemovePieSlicesByOriginalSource)
		.def("ReplacePieSlice", &PieMenu::ReplacePieSlice, luabind::adopt(luabind::result) + luabind::adopt(_3));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Round) {
		return ConcreteTypeLuaClassDefinition(Round, Entity)

		.property("NextParticle", &Round::GetNextParticle)
		.property("Shell", &Round::GetShell)
		.property("FireVel", &Round::GetFireVel)
		.property("InheritsFirerVelocity", &Round::GetInheritsFirerVelocity)
		.property("ShellVel", &Round::GetShellVel)
		.property("Separation", &Round::GetSeparation)
		.property("ParticleCount", &Round::ParticleCount)
		.property("AILifeTime", &Round::GetAILifeTime)
		.property("AIFireVel", &Round::GetAIFireVel)
		.property("IsEmpty", &Round::IsEmpty);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Scene) {
		return ConcreteTypeLuaClassDefinition(Scene, Entity)

		.property("Location", &Scene::GetLocation, &Scene::SetLocation)
		//.property("Terrain", &Scene::GetTerrain)
		.property("Dimensions", &Scene::GetDimensions)
		.property("Width", &Scene::GetWidth)
		.property("Height", &Scene::GetHeight)
		.property("WrapsX", &Scene::WrapsX)
		.property("WrapsY", &Scene::WrapsY)
		.property("TeamOwnership", &Scene::GetTeamOwnership, &Scene::SetTeamOwnership)
		.property("GlobalAcc", &Scene::GetGlobalAcc, &Scene::SetGlobalAcc)
		.property("ScenePathSize", &Scene::GetScenePathSize)

		.def_readwrite("Deployments", &Scene::m_Deployments, luabind::return_stl_iterator)

		.def_readonly("BackgroundLayers", &Scene::m_BackLayerList, luabind::return_stl_iterator)

		.def("GetScenePath", &Scene::GetScenePath, luabind::return_stl_iterator)
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
		.def_readwrite("Areas", &Scene::m_AreaList, luabind::return_stl_iterator)
		.def("SetArea", &Scene::SetArea)
		.def("HasArea", &Scene::HasArea)
		.def("GetArea", (Scene::Area * (Scene:: *)(const std::string &areaName)) &Scene::GetArea)
		.def("GetOptionalArea", &Scene::GetOptionalArea)
		.def("WithinArea", &Scene::WithinArea)
		.def("AddNavigatableArea", &Scene::AddNavigatableArea)
		.def("ClearNavigatableAreas", &Scene::ClearNavigatableAreas)
		.def("ResetPathFinding", &Scene::ResetPathFinding)
		.def("UpdatePathFinding", &Scene::UpdatePathFinding)
		.def("PathFindingUpdated", &Scene::PathFindingUpdated)
		.def("CalculatePath", &LuaAdaptersScene::CalculatePath1)
		.def("CalculatePath", &LuaAdaptersScene::CalculatePath2)
		.def("CalculatePathAsync", &LuaAdaptersScene::CalculatePathAsync1)
		.def("CalculatePathAsync", &LuaAdaptersScene::CalculatePathAsync2)

		.enum_("PlacedObjectSets")[
			luabind::value("PLACEONLOAD", Scene::PlacedObjectSets::PLACEONLOAD),
			luabind::value("BLUEPRINT", Scene::PlacedObjectSets::BLUEPRINT),
			luabind::value("AIPLAN", Scene::PlacedObjectSets::AIPLAN),
			luabind::value("PLACEDSETSCOUNT", Scene::PlacedObjectSets::PLACEDSETSCOUNT)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SceneArea) {
		return luabind::class_<Scene::Area>("Area")

		.def(luabind::constructor<>())
		.def(luabind::constructor<std::string>())
		.def(luabind::constructor<const Scene::Area &>())

		.property("ClassName", &Scene::Area::GetClassName)
		.property("Name", &Scene::Area::GetName)
		.property("FirstBox", &Scene::Area::GetFirstBox)
		.property("Center", &Scene::Area::GetCenterPoint)
		.property("RandomPoint", &Scene::Area::GetRandomPoint)

		.def("Reset", &Scene::Area::Reset)
		.def_readwrite("Boxes", &Scene::Area::m_BoxList, luabind::return_stl_iterator)
		.def("AddBox", &Scene::Area::AddBox)
		.def("RemoveBox", &Scene::Area::RemoveBox)
		.def("HasNoArea", &Scene::Area::HasNoArea)
		.def("IsInside", &Scene::Area::IsInside)
		.def("IsInsideX", &Scene::Area::IsInsideX)
		.def("IsInsideY", &Scene::Area::IsInsideY)
		.def("GetBoxInside", &Scene::Area::GetBoxInside)
		.def("RemoveBoxInside", &Scene::Area::RemoveBoxInside)
		.def("GetCenterPoint", &Scene::Area::GetCenterPoint)
		.def("GetRandomPoint", &Scene::Area::GetRandomPoint);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SceneLayer) {
		return luabind::class_<SceneLayer, Entity>("SceneLayer");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SceneObject) {
		return AbstractTypeLuaClassDefinition(SceneObject, Entity)
		.property("Pos", &SceneObject::GetPos, &SceneObject::SetPosLuaBinding)
		.property("HFlipped", &SceneObject::IsHFlipped, &SceneObject::SetHFlipped)
		.property("RotAngle", &SceneObject::GetRotAngle, &SceneObject::SetRotAngle)
		.property("Team", &SceneObject::GetTeam, &SceneObject::SetTeam)
		.property("PlacedByPlayer", &SceneObject::GetPlacedByPlayer, &SceneObject::SetPlacedByPlayer)
		.property("Buyable", &SceneObject::IsBuyable)
		
		.property("BuyableMode", &LuaAdaptersSceneObject::GetBuyableMode)
		
		.def("IsOnScenePoint", &SceneObject::IsOnScenePoint)
		.def("GetGoldValue", &SceneObject::GetGoldValueOld)
		.def("GetGoldValue", &SceneObject::GetGoldValue)
		.def("SetGoldValue", &SceneObject::SetGoldValue)
		.def("GetGoldValueString", &SceneObject::GetGoldValueString)
		
		.def("GetTotalValue", &SceneObject::GetTotalValue)
		.def("GetTotalValue", &LuaAdaptersSceneObject::GetTotalValue)
		
		.enum_("BuyableMode")[
			luabind::value("NORESTRICTIONS", static_cast<int>(SceneObject::BuyableMode::NoRestrictions)),
			luabind::value("BUYMENUONLY", static_cast<int>(SceneObject::BuyableMode::BuyMenuOnly)),
			luabind::value("OBJECTPICKERONLY", static_cast<int>(SceneObject::BuyableMode::ObjectPickerOnly)),
			luabind::value("SCRIPTONLY", static_cast<int>(SceneObject::BuyableMode::ScriptOnly))];
		
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SLBackground) {
		return luabind::class_<SLBackground, SceneLayer>("SLBackground")

		.property("Frame", &SLBackground::GetFrame, &SLBackground::SetFrame)
		.property("SpriteAnimMode", &SLBackground::GetSpriteAnimMode, &SLBackground::SetSpriteAnimMode)
		.property("SpriteAnimDuration", &SLBackground::GetSpriteAnimDuration, &SLBackground::SetSpriteAnimDuration)
		.property("IsAnimatedManually", &SLBackground::IsAnimatedManually, &SLBackground::SetAnimatedManually)
		.property("AutoScrollX", &SLBackground::GetAutoScrollX, &SLBackground::SetAutoScrollX)
		.property("AutoScrollY", &SLBackground::GetAutoScrollY, &SLBackground::SetAutoScrollY)
		.property("AutoScrollInterval", &SLBackground::GetAutoScrollStepInterval, &SLBackground::SetAutoScrollStepInterval)
		.property("AutoScrollStep", &SLBackground::GetAutoScrollStep, &SLBackground::SetAutoScrollStep)
		.property("AutoScrollStepX", &SLBackground::GetAutoScrollStepX, &SLBackground::SetAutoScrollStepX)
		.property("AutoScrollStepY", &SLBackground::GetAutoScrollStepY, &SLBackground::SetAutoScrollStepY)

		.def("IsAutoScrolling", &SLBackground::IsAutoScrolling);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SoundContainer) {
		return ConcreteTypeLuaClassDefinition(SoundContainer, Entity)

		.def(luabind::constructor<>())

		.property("SoundOverlapMode", &SoundContainer::GetSoundOverlapMode, &SoundContainer::SetSoundOverlapMode)
		.property("BusRouting", &SoundContainer::GetBusRouting, &SoundContainer::SetBusRouting)
		.property("Immobile", &SoundContainer::IsImmobile, &SoundContainer::SetImmobile)
		.property("AttenuationStartDistance", &SoundContainer::GetAttenuationStartDistance, &SoundContainer::SetAttenuationStartDistance)
		.property("CustomPanValue", &SoundContainer::GetCustomPanValue, &SoundContainer::SetCustomPanValue)
		.property("PanningStrengthMultiplier", &SoundContainer::GetPanningStrengthMultiplier, &SoundContainer::SetPanningStrengthMultiplier)
		.property("Loops", &SoundContainer::GetLoopSetting, &SoundContainer::SetLoopSetting)
		.property("Priority", &SoundContainer::GetPriority, &SoundContainer::SetPriority)
		.property("AffectedByGlobalPitch", &SoundContainer::IsAffectedByGlobalPitch, &SoundContainer::SetAffectedByGlobalPitch)
		.property("Pos", &SoundContainer::GetPosition, &SoundContainer::SetPosition)
		.property("Volume", &SoundContainer::GetVolume, &SoundContainer::SetVolume)
		.property("Pitch", &SoundContainer::GetPitch, &SoundContainer::SetPitch)
		.property("PitchVariation", &SoundContainer::GetPitchVariation, &SoundContainer::SetPitchVariation)

		.def("HasAnySounds", &SoundContainer::HasAnySounds)
		.def("GetTopLevelSoundSet", &SoundContainer::GetTopLevelSoundSet)
		.def("SetTopLevelSoundSet", &SoundContainer::SetTopLevelSoundSet)
		.def("IsBeingPlayed", &SoundContainer::IsBeingPlayed)
		.def("Play", (bool (SoundContainer:: *)()) &SoundContainer::Play)
		.def("Play", (bool (SoundContainer:: *)(const int player)) &SoundContainer::Play)
		.def("Play", (bool (SoundContainer:: *)(const Vector &position)) &SoundContainer::Play)
		.def("Play", (bool (SoundContainer:: *)(const Vector &position, int player)) &SoundContainer::Play)
		.def("Stop", (bool (SoundContainer:: *)()) &SoundContainer::Stop)
		.def("Stop", (bool (SoundContainer:: *)(int player)) &SoundContainer::Stop)
		.def("Restart", (bool (SoundContainer:: *)()) &SoundContainer::Restart)
		.def("Restart", (bool (SoundContainer:: *)(int player)) &SoundContainer::Restart)
		.def("FadeOut", &SoundContainer::FadeOut)
		
		.enum_("BusRouting")[
			luabind::value("SFX", SoundContainer::BusRouting::SFX),
			luabind::value("UI", SoundContainer::BusRouting::UI),
			luabind::value("MUSIC", SoundContainer::BusRouting::MUSIC)
		]
		
		.enum_("SoundOverlapMode")[
			luabind::value("OVERLAP", SoundContainer::SoundOverlapMode::OVERLAP),
			luabind::value("RESTART", SoundContainer::SoundOverlapMode::RESTART),
			luabind::value("IGNORE_PLAY", SoundContainer::SoundOverlapMode::IGNORE_PLAY)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SoundSet) {
		return luabind::class_<SoundSet>("SoundSet")

		.def(luabind::constructor<>())

		.property("SoundSelectionCycleMode", &SoundSet::GetSoundSelectionCycleMode, &SoundSet::SetSoundSelectionCycleMode)

		.def_readonly("SubSoundSets", &SoundSet::m_SubSoundSets, luabind::return_stl_iterator)

		.def("HasAnySounds", &SoundSet::HasAnySounds)
		.def("SelectNextSounds", &SoundSet::SelectNextSounds)
		.def("AddSound", (void (SoundSet:: *)(const std::string &soundFilePath)) &SoundSet::AddSound)
		.def("AddSound", (void (SoundSet:: *)(const std::string &soundFilePath, const Vector &offset, float minimumAudibleDistance, float attenuationStartDistance)) &SoundSet::AddSound)
		.def("RemoveSound", (bool (SoundSet:: *)(const std::string &soundFilePath)) &SoundSet::RemoveSound)
		.def("RemoveSound", (bool (SoundSet:: *)(const std::string &soundFilePath, bool removeFromSubSoundSets)) &SoundSet::RemoveSound)
		.def("AddSoundSet", &SoundSet::AddSoundSet)

		.enum_("SoundSelectionCycleMode")[
			luabind::value("RANDOM", SoundSet::SoundSelectionCycleMode::RANDOM),
			luabind::value("FORWARDS", SoundSet::SoundSelectionCycleMode::FORWARDS),
			luabind::value("ALL", SoundSet::SoundSelectionCycleMode::ALL)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, TDExplosive) {
		return ConcreteTypeLuaClassDefinition(TDExplosive, ThrownDevice)

		.property("IsAnimatedManually", &TDExplosive::IsAnimatedManually, &TDExplosive::SetAnimatedManually);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, TerrainObject) {
		return ConcreteTypeLuaClassDefinition(TerrainObject, SceneObject)

		.def("GetBitmapOffset", &TerrainObject::GetBitmapOffset)
		.def("GetBitmapWidth", &TerrainObject::GetBitmapWidth)
		.def("GetBitmapHeight", &TerrainObject::GetBitmapHeight);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ThrownDevice) {
		return ConcreteTypeLuaClassDefinition(ThrownDevice, HeldDevice)

		.property("MinThrowVel", &ThrownDevice::GetMinThrowVel, &ThrownDevice::SetMinThrowVel)
		.property("MaxThrowVel", &ThrownDevice::GetMaxThrowVel, &ThrownDevice::SetMaxThrowVel)
		.property("StartThrowOffset", &ThrownDevice::GetStartThrowOffset, &ThrownDevice::SetStartThrowOffset)
		.property("EndThrowOffset", &ThrownDevice::GetEndThrowOffset, &ThrownDevice::SetEndThrowOffset)

		.def("GetCalculatedMaxThrowVelIncludingArmThrowStrength", &ThrownDevice::GetCalculatedMaxThrowVelIncludingArmThrowStrength);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Turret) {
		return ConcreteTypeLuaClassDefinition(Turret, Attachable)

		.property("MountedDevice", &Turret::GetFirstMountedDevice, &LuaAdaptersPropertyOwnershipSafetyFaker::TurretSetFirstMountedDevice)
		.property("MountedDeviceRotationOffset", &Turret::GetMountedDeviceRotationOffset, &Turret::SetMountedDeviceRotationOffset)

		.def("GetMountedDevices", &Turret::GetMountedDevices, luabind::return_stl_iterator)
		.def("AddMountedDevice", &Turret::AddMountedDevice, luabind::adopt(_2))
		.def("AddMountedDevice", &LuaAdaptersTurret::AddMountedFirearm, luabind::adopt(_2));
	}
}
