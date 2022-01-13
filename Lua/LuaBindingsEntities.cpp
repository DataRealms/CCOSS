#include "LuaBindingRegisterDefinitions.h"
#include "LuaAdaptersEntities.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Entity) {
		return luabind::class_<Entity>("Entity")

		.def(luabind::tostring(luabind::const_self))

		.property("ClassName", &Entity::GetClassName)
		.property("PresetName", &Entity::GetPresetName, &SetPresetName)
		.property("Description", &Entity::GetDescription, &Entity::SetDescription)
		.property("IsOriginalPreset", &Entity::IsOriginalPreset)
		.property("ModuleID", &Entity::GetModuleID)
		.property("ModuleName", &Entity::GetModuleName)
		.property("RandomWeight", &Entity::GetRandomWeight)

		.def("Clone", &CloneEntity)
		.def("Reset", &Entity::Reset)
		.def("GetModuleAndPresetName", &Entity::GetModuleAndPresetName)
		.def("AddToGroup", &Entity::AddToGroup)
		.def("IsInGroup", &Entity::IsInGroup);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ACDropShip) {
		return ConcreteTypeLuaClassDefinition(ACDropShip, ACraft)

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
		.def("GetAltitude", &ACDropShip::GetAltitude);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, ACrab) {
		return ConcreteTypeLuaClassDefinition(ACrab, Actor)

		.def(luabind::constructor<>())

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
		.property("LimbPathPushForce", &ACrab::GetLimbPathPushForce, &ACrab::SetLimbPathPushForce)

		.def("ReloadFirearms", &ACrab::ReloadFirearms)
		.def("IsWithinRange", &ACrab::IsWithinRange)
		.def("Look", &ACrab::Look)
		.def("LookForMOs", &ACrab::LookForMOs)
		.def("IsOnScenePoint", &ACrab::IsOnScenePoint)
		.def("GetLimbPath", &ACrab::GetLimbPath)
		.def("GetLimbPathSpeed", &ACrab::GetLimbPathSpeed)
		.def("SetLimbPathSpeed", &ACrab::SetLimbPathSpeed)

		.enum_("MovementState")[
			luabind::value("STAND", ACrab::MovementState::STAND),
			luabind::value("WALK", ACrab::MovementState::WALK),
			luabind::value("JUMP", ACrab::MovementState::JUMP),
			luabind::value("DISLODGE", ACrab::MovementState::DISLODGE),
			luabind::value("MOVEMENTSTATECOUNT", ACrab::MovementState::MOVEMENTSTATECOUNT)
		]
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
		.property("HatchOpenSound", &ACraft::GetHatchOpenSound, &ACraftSetHatchOpenSound)
		.property("HatchCloseSound", &ACraft::GetHatchCloseSound, &ACraftSetHatchCloseSound)
		.property("CrashSound", &ACraft::GetCrashSound, &ACraftSetCrashSound)
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

		.property("RightLeg", &ACRocket::GetRightLeg, &ACRocketSetRightLeg)
		.property("LeftLeg", &ACRocket::GetLeftLeg, &ACRocketSetLeftLeg)
		.property("MainEngine", &ACRocket::GetMainThruster, &ACRocketSetMainThruster)
		.property("LeftEngine", &ACRocket::GetLeftThruster, &ACRocketSetLeftThruster)
		.property("RightEngine", &ACRocket::GetRightThruster, &ACRocketSetRightThruster)
		.property("LeftThruster", &ACRocket::GetULeftThruster, &ACRocketSetULeftThruster)
		.property("RightThruster", &ACRocket::GetURightThruster, &ACRocketSetURightThruster)
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

		.property("BodyHitSound", &Actor::GetBodyHitSound, &ActorSetBodyHitSound)
		.property("AlarmSound", &Actor::GetAlarmSound, &ActorSetAlarmSound)
		.property("PainSound", &Actor::GetPainSound, &ActorSetPainSound)
		.property("DeathSound", &Actor::GetDeathSound, &ActorSetDeathSound)
		.property("DeviceSwitchSound", &Actor::GetDeviceSwitchSound, &ActorSetDeviceSwitchSound)
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
		.property("ViewPoint", &Actor::GetViewPoint, &Actor::SetViewPoint)
		.property("ItemInReach", &Actor::GetItemInReach, &Actor::SetItemInReach)
		.property("Height", &Actor::GetHeight)
		.property("AIMode", &Actor::GetAIMode, &Actor::SetAIMode)
		.property("DeploymentID", &Actor::GetDeploymentID)
		.property("PassengerSlots", &Actor::GetPassengerSlots, &Actor::SetPassengerSlots)
		.property("Perceptiveness", &Actor::GetPerceptiveness, &Actor::SetPerceptiveness)
		.property("CanRevealUnseen", &Actor::GetCanRevealUnseen, &Actor::SetCanRevealUnseen)
		.property("InventorySize", &Actor::GetInventorySize)
		.property("MaxInventoryMass", &Actor::GetMaxInventoryMass)
		.property("MovePathSize", &Actor::GetMovePathSize)
		.property("AimDistance", &Actor::GetAimDistance, &Actor::SetAimDistance)
		.property("SightDistance", &Actor::GetSightDistance, &Actor::SetSightDistance)

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
		.def("RemoveInventoryItem", &Actor::RemoveInventoryItem)
		.def("SwapNextInventory", &Actor::SwapNextInventory)
		.def("SwapPrevInventory", &Actor::SwapPrevInventory)
		.def("DropAllInventory", &Actor::DropAllInventory)
		.def("IsInventoryEmpty", &Actor::IsInventoryEmpty)
		.def("FlashWhite", &Actor::FlashWhite)
		.def("DrawWaypoints", &Actor::DrawWaypoints)
		.def("SetMovePathToUpdate", &Actor::SetMovePathToUpdate)
		.def("UpdateMovePath", &Actor::UpdateMovePath)
		.def("SetAlarmPoint", &Actor::AlarmPoint)
		.def("GetAlarmPoint", &Actor::GetAlarmPoint)

		.enum_("Status")[
			luabind::value("STABLE", Actor::Status::STABLE),
			luabind::value("UNSTABLE", Actor::Status::UNSTABLE),
			luabind::value("INACTIVE", Actor::Status::INACTIVE),
			luabind::value("DYING", Actor::Status::DYING),
			luabind::value("DEAD", Actor::Status::DEAD)
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

		.property("Door", &ADoor::GetDoor, &ADoorSetDoor)
		.property("DoorMoveStartSound", &ADoor::GetDoorMoveStartSound, &ADoorSetDoorMoveStartSound)
		.property("DoorMoveSound", &ADoor::GetDoorMoveSound, &ADoorSetDoorMoveSound)
		.property("DoorDirectionChangeSound", &ADoor::GetDoorDirectionChangeSound, &ADoorSetDoorDirectionChangeSound)
		.property("DoorMoveEndSound", &ADoor::GetDoorMoveEndSound, &ADoorSetDoorMoveEndSound)

		.def("GetDoorState", &ADoor::GetDoorState)
		.def("OpenDoor", &ADoor::OpenDoor)
		.def("CloseDoor", &ADoor::CloseDoor)
		.def("StopDoor", &ADoor::StopDoor)
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

		.property("EmissionSound", &AEmitter::GetEmissionSound, &AEmitterSetEmissionSound)
		.property("BurstSound", &AEmitter::GetBurstSound, &AEmitterSetBurstSound)
		.property("EndSound", &AEmitter::GetEndSound, &AEmitterSetEndSound)
		.property("BurstScale", &AEmitter::GetBurstScale, &AEmitter::SetBurstScale)
		.property("EmitAngle", &AEmitter::GetEmitAngle, &AEmitter::SetEmitAngle)
		.property("GetThrottle", &AEmitter::GetThrottle, &AEmitter::SetThrottle)
		.property("Throttle", &AEmitter::GetThrottle, &AEmitter::SetThrottle)
		.property("NegativeThrottleMultiplier", &AEmitter::GetNegativeThrottleMultiplier, &AEmitter::SetNegativeThrottleMultiplier)
		.property("PositiveThrottleMultiplier", &AEmitter::GetPositiveThrottleMultiplier, &AEmitter::SetPositiveThrottleMultiplier)
		.property("BurstSpacing", &AEmitter::GetBurstSpacing, &AEmitter::SetBurstSpacing)
		.property("BurstDamage", &AEmitter::GetBurstDamage, &AEmitter::SetBurstDamage)
		.property("EmitterDamageMultiplier", &AEmitter::GetEmitterDamageMultiplier, &AEmitter::SetEmitterDamageMultiplier)
		.property("EmitCountLimit", &AEmitter::GetEmitCountLimit, &AEmitter::SetEmitCountLimit)
		.property("EmitDamage", &AEmitter::GetEmitDamage, &AEmitter::SetEmitDamage)
		.property("Flash", &AEmitter::GetFlash, &AEmitterSetFlash)
		.property("FlashScale", &AEmitter::GetFlashScale, &AEmitter::SetFlashScale)

		.def_readwrite("Emissions", &AEmitter::m_EmissionList, luabind::return_stl_iterator)

		.def("IsEmitting", &AEmitter::IsEmitting)
		.def("EnableEmission", &AEmitter::EnableEmission)
		.def("GetEmitVector", &AEmitter::GetEmitVector)
		.def("GetRecoilVector", &AEmitter::GetRecoilVector)
		.def("EstimateImpulse", &AEmitter::EstimateImpulse)
		.def("TriggerBurst", &AEmitter::TriggerBurst)
		.def("IsSetToBurst", &AEmitter::IsSetToBurst)
		.def("CanTriggerBurst", &AEmitter::CanTriggerBurst);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, AHuman) {
		return ConcreteTypeLuaClassDefinition(AHuman, Actor)

		.def(luabind::constructor<>())

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
		.property("ThrowProgress", &AHuman::GetThrowProgress)
		.property("EquippedItem", &AHuman::GetEquippedItem)
		.property("EquippedBGItem", &AHuman::GetEquippedBGItem)
		.property("FirearmIsReady", &AHuman::FirearmIsReady)
		.property("ThrowableIsReady", &AHuman::ThrowableIsReady)
		.property("FirearmIsEmpty", &AHuman::FirearmIsEmpty)
		.property("FirearmNeedsReload", &AHuman::FirearmNeedsReload)
		.property("FirearmIsSemiAuto", &AHuman::FirearmIsSemiAuto)
		.property("FirearmActivationDelay", &AHuman::FirearmActivationDelay)
		.property("LimbPathPushForce", &AHuman::GetLimbPathPushForce, &AHuman::SetLimbPathPushForce)

		.def("EquipFirearm", &AHuman::EquipFirearm)
		.def("EquipThrowable", &AHuman::EquipThrowable)
		.def("EquipDiggingTool", &AHuman::EquipDiggingTool)
		.def("EquipShield", &AHuman::EquipShield)
		.def("EquipShieldInBGArm", &AHuman::EquipShieldInBGArm)
		.def("EquipDeviceInGroup", &AHuman::EquipDeviceInGroup)
		.def("EquipNamedDevice", &AHuman::EquipNamedDevice)
		.def("EquipLoadedFirearmInGroup", &AHuman::EquipLoadedFirearmInGroup)
		.def("UnequipBGArm", &AHuman::UnequipBGArm)
		.def("ReloadFirearms", &AHuman::ReloadFirearms)
		.def("IsWithinRange", &AHuman::IsWithinRange)
		.def("Look", &AHuman::Look)
		.def("LookForGold", &AHuman::LookForGold)
		.def("LookForMOs", &AHuman::LookForMOs)
		.def("IsOnScenePoint", &AHuman::IsOnScenePoint)
		.def("GetLimbPath", &AHuman::GetLimbPath)
		.def("GetLimbPathSpeed", &AHuman::GetLimbPathSpeed)
		.def("SetLimbPathSpeed", &AHuman::SetLimbPathSpeed)
		.def("GetRotAngleTarget", &AHuman::GetRotAngleTarget)
		.def("SetRotAngleTarget", &AHuman::SetRotAngleTarget)

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
		.enum_("MovementState")[
			luabind::value("NOMOVE", AHuman::MovementState::NOMOVE),
			luabind::value("STAND", AHuman::MovementState::STAND),
			luabind::value("WALK", AHuman::MovementState::WALK),
			luabind::value("CROUCH", AHuman::MovementState::CROUCH),
			luabind::value("CRAWL", AHuman::MovementState::CRAWL),
			luabind::value("ARMCRAWL", AHuman::MovementState::ARMCRAWL),
			luabind::value("CLIMB", AHuman::MovementState::CLIMB),
			luabind::value("JUMP", AHuman::MovementState::JUMP),
			luabind::value("DISLODGE", AHuman::MovementState::DISLODGE),
			luabind::value("MOVEMENTSTATECOUNT", AHuman::MovementState::MOVEMENTSTATECOUNT)
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

		.property("HeldDevice", &Arm::GetHeldMO)
		.property("MaxLength", &Arm::GetMaxLength)
		.property("IdleOffset", &Arm::GetIdleOffset, &Arm::SetIdleOffset)
		.property("GripStrength", &Arm::GetGripStrength, &Arm::SetGripStrength)
		.property("ThrowStrength", &Arm::GetThrowStrength, &Arm::SetThrowStrength)
		.property("HandPos", &Arm::GetHandPos, &Arm::SetHandPos);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Attachable) {
		return ConcreteTypeLuaClassDefinition(Attachable, MOSRotating)

		.property("ParentOffset", &Attachable::GetParentOffset, &Attachable::SetParentOffset)
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
		.property("CanCollideWithTerrain", &Attachable::CanCollideWithTerrain)
		.property("DrawnAfterParent", &Attachable::IsDrawnAfterParent, &Attachable::SetDrawnAfterParent)
		.property("InheritsFrame", &Attachable::InheritsFrame, &Attachable::SetInheritsFrame)

		.def("IsAttached", &Attachable::IsAttached)
		.def("IsAttachedTo", &Attachable::IsAttachedTo)

		.def("RemoveFromParent", &RemoveAttachableFromParentLuaSafe1, luabind::adopt(luabind::return_value))
		.def("RemoveFromParent", &RemoveAttachableFromParentLuaSafe2, luabind::adopt(luabind::return_value));
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

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, GlobalScript) {
		return AbstractTypeLuaClassDefinition(GlobalScript, Entity)

		.def("Deactivate", &GlobalScript::Deactivate);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, HDFirearm) {
		return ConcreteTypeLuaClassDefinition(HDFirearm, HeldDevice)

		.property("RateOfFire", &HDFirearm::GetRateOfFire, &HDFirearm::SetRateOfFire)
		.property("FullAuto", &HDFirearm::IsFullAuto, &HDFirearm::SetFullAuto)
		.property("Reloadable", &HDFirearm::IsReloadable, &HDFirearm::SetReloadable)
		.property("RoundInMagCount", &HDFirearm::GetRoundInMagCount)
		.property("RoundInMagCapacity", &HDFirearm::GetRoundInMagCapacity)
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
		.property("ReloadProgress", &HDFirearm::GetReloadProgress)
		.property("ShakeRange", &HDFirearm::GetShakeRange, &HDFirearm::SetShakeRange)
		.property("SharpShakeRange", &HDFirearm::GetSharpShakeRange, &HDFirearm::SetSharpShakeRange)
		.property("NoSupportFactor", &HDFirearm::GetNoSupportFactor, &HDFirearm::SetNoSupportFactor)
		.property("ParticleSpreadRange", &HDFirearm::GetParticleSpreadRange, &HDFirearm::SetParticleSpreadRange)
		.property("ShellVelVariation", &HDFirearm::GetShellVelVariation, &HDFirearm::SetShellVelVariation)
		.property("FiredOnce", &HDFirearm::FiredOnce)
		.property("FiredFrame", &HDFirearm::FiredFrame)
		.property("RoundsFired", &HDFirearm::RoundsFired)
		.property("IsAnimatedManually", &HDFirearm::IsAnimatedManually, &HDFirearm::SetAnimatedManually)
		.property("RecoilTransmission", &HDFirearm::GetJointStiffness, &HDFirearm::SetJointStiffness)

		.def("GetAIFireVel", &HDFirearm::GetAIFireVel)
		.def("GetAIBulletLifeTime", &HDFirearm::GetAIBulletLifeTime)
		.def("GetBulletAccScalar", &HDFirearm::GetBulletAccScalar)
		.def("GetAIBlastRadius", &HDFirearm::GetAIBlastRadius)
		.def("GetAIPenetration", &HDFirearm::GetAIPenetration)
		.def("CompareTrajectories", &HDFirearm::CompareTrajectories)
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
		.property("SupportOffset", &HeldDevice::GetSupportOffset, &HeldDevice::SetSupportOffset)
		.property("HasPickupLimitations", &HeldDevice::HasPickupLimitations)
		.property("UnPickupable", &HeldDevice::IsUnPickupable, &HeldDevice::SetUnPickupable)
		.property("GripStrengthMultiplier", &HeldDevice::GetGripStrengthMultiplier, &HeldDevice::SetGripStrengthMultiplier)

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
		.def("IsPickupableBy", &HeldDevice::IsPickupableBy)
		.def("AddPickupableByPresetName", &HeldDevice::AddPickupableByPresetName)
		.def("RemovePickupableByPresetName", &HeldDevice::RemovePickupableByPresetName)
		.def("SetSupported", &HeldDevice::SetSupported);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Leg) {
		return ConcreteTypeLuaClassDefinition(Leg, Attachable)

		.property("Foot", &Leg::GetFoot, &LegSetFoot);
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

		.property("TrailLength", &MOPixel::GetTrailLength, &MOPixel::SetTrailLength);
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
		.def("SetEntryWound", &MOSprite::SetEntryWound)
		.def("SetExitWound", &MOSprite::SetExitWound)
		.def("GetEntryWoundPresetName", &MOSprite::GetEntryWoundPresetName)
		.def("GetExitWoundPresetName", &MOSprite::GetExitWoundPresetName)

		.enum_("SpriteAnimMode")[
			luabind::value("NOANIM", MOSprite::SpriteAnimMode::NOANIM),
			luabind::value("ALWAYSLOOP", MOSprite::SpriteAnimMode::ALWAYSLOOP),
			luabind::value("ALWAYSRANDOM", MOSprite::SpriteAnimMode::ALWAYSRANDOM),
			luabind::value("ALWAYSPINGPONG", MOSprite::SpriteAnimMode::ALWAYSPINGPONG),
			luabind::value("LOOPWHENACTIVE", MOSprite::SpriteAnimMode::LOOPWHENACTIVE),
			luabind::value("LOOPWHENOPENCLOSE", MOSprite::SpriteAnimMode::LOOPWHENOPENCLOSE),
			luabind::value("PINGPONGOPENCLOSE", MOSprite::SpriteAnimMode::PINGPONGOPENCLOSE),
			luabind::value("OVERLIFETIME", MOSprite::SpriteAnimMode::OVERLIFETIME),
			luabind::value("ONCOLLIDE", MOSprite::SpriteAnimMode::ONCOLLIDE)
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
		.property("GibSound", &MOSRotating::GetGibSound, &MOSRotatingSetGibSound)
		.property("GibImpulseLimit", &MOSRotating::GetGibImpulseLimit, &MOSRotating::SetGibImpulseLimit)
		.property("WoundCountAffectsImpulseLimitRatio", &MOSRotating::GetWoundCountAffectsImpulseLimitRatio)
		.property("DamageMultiplier", &MOSRotating::GetDamageMultiplier, &MOSRotating::SetDamageMultiplier)
		.property("WoundCount", (int (MOSRotating:: *)() const) &MOSRotating::GetWoundCount)
		.property("OrientToVel", &MOSRotating::GetOrientToVel, &MOSRotating::SetOrientToVel)

		.def_readonly("Attachables", &MOSRotating::m_Attachables, luabind::return_stl_iterator)
		.def_readonly("Wounds", &MOSRotating::m_Wounds, luabind::return_stl_iterator)

		.def("AddRecoil", &MOSRotating::AddRecoil)
		.def("SetRecoil", &MOSRotating::SetRecoil)
		.def("IsRecoiled", &MOSRotating::IsRecoiled)
		.def("EnableDeepCheck", &MOSRotating::EnableDeepCheck)
		.def("ForceDeepCheck", &MOSRotating::ForceDeepCheck)
		.def("GibThis", &MOSRotating::GibThis)
		.def("MoveOutOfTerrain", &MOSRotating::MoveOutOfTerrain)
		.def("GetGibWoundLimit", (int (MOSRotating:: *)() const) &MOSRotating::GetGibWoundLimit)
		.def("GetGibWoundLimit", (int (MOSRotating:: *)(bool positiveDamage, bool negativeDamage, bool noDamage) const) &MOSRotating::GetGibWoundLimit)
		.def("GetWoundCount", (int (MOSRotating:: *)() const) &MOSRotating::GetWoundCount)
		.def("GetWoundCount", (int (MOSRotating:: *)(bool positiveDamage, bool negativeDamage, bool noDamage) const) &MOSRotating::GetWoundCount)
		.def("AddWound", &MOSRotating::AddWound, luabind::adopt(_2))
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

		.def("GibThis", &GibThis); // Free function bound as member function to emulate default variables
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, MovableObject) {
		return AbstractTypeLuaClassDefinition(MovableObject, SceneObject)

		.property("Material", &MovableObject::GetMaterial)
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
		.property("AboveHUDPos", &MovableObject::GetAboveHUDPos)
		.property("HitsMOs", &MovableObject::HitsMOs, &MovableObject::SetToHitMOs)
		.property("GetsHitByMOs", &MovableObject::GetsHitByMOs, &MovableObject::SetToGetHitByMOs)
		.property("IgnoresTeamHits", &MovableObject::IgnoresTeamHits, &MovableObject::SetIgnoresTeamHits)
		.property("IgnoresWhichTeam", &MovableObject::IgnoresWhichTeam)
		.property("IgnoreTerrain", &MovableObject::IgnoreTerrain, &MovableObject::SetIgnoreTerrain)
		.property("ToSettle", &MovableObject::ToSettle, &MovableObject::SetToSettle)
		.property("ToDelete", &MovableObject::ToDelete, &MovableObject::SetToDelete)
		.property("MissionCritical", &MovableObject::IsMissionCritical, &MovableObject::SetMissionCritical)
		.property("HUDVisible", &MovableObject::GetHUDVisible, &MovableObject::SetHUDVisible)
		.property("PinStrength", &MovableObject::GetPinStrength, &MovableObject::SetPinStrength)
		.property("DamageOnCollision", &MovableObject::DamageOnCollision, &MovableObject::SetDamageOnCollision)
		.property("DamageOnPenetration", &MovableObject::DamageOnPenetration, &MovableObject::SetDamageOnPenetration)
		.property("WoundDamageMultiplier", &MovableObject::WoundDamageMultiplier, &MovableObject::SetWoundDamageMultiplier)
		.property("HitWhatMOID", &MovableObject::HitWhatMOID)
		.property("HitWhatTerrMaterial", &MovableObject::HitWhatTerrMaterial)
		.property("ProvidesPieMenuContext", &MovableObject::ProvidesPieMenuContext, &MovableObject::SetProvidesPieMenuContext)
		.property("HitWhatParticleUniqueID", &MovableObject::HitWhatParticleUniqueID)
		.property("ApplyWoundDamageOnCollision", &MovableObject::GetApplyWoundDamageOnCollision, &MovableObject::SetApplyWoundDamageOnCollision)
		.property("ApplyWoundBurstDamageOnCollision", &MovableObject::GetApplyWoundBurstDamageOnCollision, &MovableObject::SetApplyWoundBurstDamageOnCollision)

		.def("GetParent", (MOSRotating * (MovableObject::*)())&MovableObject::GetParent)
		.def("GetParent", (const MOSRotating * (MovableObject::*)() const)&MovableObject::GetParent)
		.def("GetRootParent", (MovableObject * (MovableObject::*)())&MovableObject::GetRootParent)
		.def("GetRootParent", (const MovableObject * (MovableObject::*)() const)&MovableObject::GetRootParent)
		.def("ReloadScripts", &MovableObject::ReloadScripts)
		.def("HasScript", &MovableObject::HasScript)
		.def("AddScript", &MovableObject::AddScript)
		.def("ScriptEnabled", &MovableObject::ScriptEnabled)
		.def("EnableScript", &MovableObject::EnableScript)
		.def("DisableScript", &MovableObject::DisableScript)
		.def("EnableOrDisableAllScripts", &MovableObject::EnableOrDisableAllScripts)
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
		.def("RotateOffset", &MovableObject::RotateOffset);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, PEmitter) {
		return ConcreteTypeLuaClassDefinition(PEmitter, MOSParticle)

		.property("BurstScale", &PEmitter::GetBurstScale, &PEmitter::SetBurstScale)
		.property("EmitAngle", &PEmitter::GetEmitAngle, &PEmitter::SetEmitAngle)
		.property("GetThrottle", &PEmitter::GetThrottle, &PEmitter::SetThrottle)
		.property("Throttle", &PEmitter::GetThrottle, &PEmitter::SetThrottle)
		.property("BurstSpacing", &PEmitter::GetBurstSpacing, &PEmitter::SetBurstSpacing)
		.property("EmitCountLimit", &PEmitter::GetEmitCountLimit, &PEmitter::SetEmitCountLimit)
		.property("FlashScale", &PEmitter::GetFlashScale, &PEmitter::SetFlashScale)

		.def_readwrite("Emissions", &PEmitter::m_EmissionList, luabind::return_stl_iterator)

		.def("IsEmitting", &PEmitter::IsEmitting)
		.def("EnableEmission", &PEmitter::EnableEmission)
		.def("GetEmitVector", &PEmitter::GetEmitVector)
		.def("GetRecoilVector", &PEmitter::GetRecoilVector)
		.def("EstimateImpulse", &PEmitter::EstimateImpulse)
		.def("TriggerBurst", &PEmitter::TriggerBurst)
		.def("IsSetToBurst", &PEmitter::IsSetToBurst)
		.def("CanTriggerBurst", &PEmitter::CanTriggerBurst);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Round) {
		return ConcreteTypeLuaClassDefinition(Round, Entity)

		.property("NextParticle", &Round::GetNextParticle)
		.property("Shell", &Round::GetShell)
		.property("FireVel", &Round::GetFireVel)
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

		.def_readwrite("ScenePath", &Scene::m_ScenePath, luabind::return_stl_iterator)
		.def_readwrite("Deployments", &Scene::m_Deployments, luabind::return_stl_iterator)

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
		.def("ResetPathFinding", &Scene::ResetPathFinding)
		.def("UpdatePathFinding", &Scene::UpdatePathFinding)
		.def("PathFindingUpdated", &Scene::PathFindingUpdated)
		.def("CalculatePath", &Scene::CalculateScenePath)

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

		.def("Reset", &Scene::Area::Reset)
		.def("AddBox", &Scene::Area::AddBox)
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

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SceneObject) {
		return AbstractTypeLuaClassDefinition(SceneObject, Entity)

		.property("Pos", &SceneObject::GetPos, &SceneObject::SetPos)
		.property("HFlipped", &SceneObject::IsHFlipped, &SceneObject::SetHFlipped)
		.property("RotAngle", &SceneObject::GetRotAngle, &SceneObject::SetRotAngle)
		.property("Team", &SceneObject::GetTeam, &SceneObject::SetTeam)
		.property("PlacedByPlayer", &SceneObject::GetPlacedByPlayer, &SceneObject::SetPlacedByPlayer)
		.property("IsBuyable", &SceneObject::IsBuyable)

		.def("IsOnScenePoint", &SceneObject::IsOnScenePoint)
		.def("GetGoldValue", &SceneObject::GetGoldValueOld)
		.def("GetGoldValue", &SceneObject::GetGoldValue)
		.def("SetGoldValue", &SceneObject::SetGoldValue)
		.def("GetGoldValueString", &SceneObject::GetGoldValueString)
		.def("GetTotalValue", &SceneObject::GetTotalValue)

		.def("GetTotalValue", &GetTotalValue);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, SoundContainer) {
		return ConcreteTypeLuaClassDefinition(SoundContainer, Entity)

		.def(luabind::constructor<>())

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
		.property("EndThrowOffset", &ThrownDevice::GetEndThrowOffset, &ThrownDevice::SetEndThrowOffset);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(EntityLuaBindings, Turret) {
		return ConcreteTypeLuaClassDefinition(Turret, Attachable)

		.property("MountedDevice", &Turret::GetFirstMountedDevice, &TurretSetFirstMountedDevice)
		.property("MountedDeviceRotationOffset", &Turret::GetMountedDeviceRotationOffset, &Turret::SetMountedDeviceRotationOffset)

		.def("GetMountedDevices", &Turret::GetMountedDevices, luabind::return_stl_iterator)
		.def("AddMountedDevice", &Turret::AddMountedDevice, luabind::adopt(_2))
		.def("AddMountedDevice", &TurretAddMountedFirearm, luabind::adopt(_2));
	}
}