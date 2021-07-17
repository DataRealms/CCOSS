#ifndef _RTELUABINDENTITIES_
#define _RTELUABINDENTITIES_

#include "LuaEntityAdapters.h"

#include "Entity.h"
#include "MetaPlayer.h"

namespace RTE {

	/// <summary>
	/// 
	/// </summary>
	struct EntityLuaBindings {

		LuaBindingRegisterFunctionForType(Entity) {
			return luabind::class_<Entity>("Entity")
				.def("Clone", &CloneEntity)
				.def("Reset", &Entity::Reset)
				.def(luabind::tostring(luabind::const_self))
				.property("ClassName", &Entity::GetClassName)
				.property("PresetName", &Entity::GetPresetName, &Entity::SetPresetName)
				.property("Description", &Entity::GetDescription, &Entity::SetDescription)
				.def("GetModuleAndPresetName", &Entity::GetModuleAndPresetName)
				.property("IsOriginalPreset", &Entity::IsOriginalPreset)
				.property("ModuleID", &Entity::GetModuleID)
				.property("RandomWeight", &Entity::GetRandomWeight)
				.def("AddToGroup", &Entity::AddToGroup)
				.def("IsInGroup", (bool (Entity::*)(const std::string &))&Entity::IsInGroup);
		}

		LuaBindingRegisterFunctionForType(ACDropShip) {
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

		LuaBindingRegisterFunctionForType(ACrab) {
			return ConcreteTypeLuaClassDefinition(ACrab, Actor)
				// These are all private/protected so they can't be bound, need to consider making them public.
				.enum_("MovementState")[
					luabind::value("STAND", 0 /*ACrab::MovementState::STAND*/),
					luabind::value("WALK", 1 /*ACrab::MovementState::WALK*/),
					luabind::value("JUMP", 2 /*ACrab::MovementState::JUMP*/),
					luabind::value("DISLODGE", 3 /*ACrab::MovementState::DISLODGE*/),
					luabind::value("MOVEMENTSTATECOUNT", 4 /*ACrab::MovementState::MOVEMENTSTATECOUNT*/)
				]
				.enum_("Side")[
					// Doesn't have qualifier
					luabind::value("LEFTSIDE", 0 /*ACrab::LEFTSIDE*/),
					luabind::value("RIGHTSIDE", 1 /*ACrab::RIGHTSIDE*/),
					luabind::value("SIDECOUNT", 2 /*ACrab::SIDECOUNT*/)
				]
				.enum_("Layer")[
				// Doesn't have qualifier
				luabind::value("FGROUND", 0 /*ACrab::FGROUND*/),
				luabind::value("BGROUND", 1 /*ACrab::BGROUND*/)
				]
				.enum_("DeviceHandlingState")[
					luabind::value("STILL", 0 /*ACrab::DeviceHandlingState::STILL*/),
					luabind::value("POINTING", 1 /*ACrab::DeviceHandlingState::POINTING*/),
					luabind::value("SCANNING", 2 /*ACrab::DeviceHandlingState::SCANNING*/),
					luabind::value("AIMING", 3 /*ACrab::DeviceHandlingState::AIMING*/),
					luabind::value("FIRING", 4 /*ACrab::DeviceHandlingState::FIRING*/),
					luabind::value("THROWING", 5 /*ACrab::DeviceHandlingState::THROWING*/),
					luabind::value("DIGGING", 6 /*ACrab::DeviceHandlingState::DIGGING*/)
				]
				.enum_("SweepState")[
					luabind::value("NOSWEEP", 0 /*ACrab::SweepState::NOSWEEP*/),
					luabind::value("SWEEPINGUP", 1 /*ACrab::SweepState::SWEEPINGUP*/),
					luabind::value("SWEEPUPPAUSE", 2 /*ACrab::SweepState::SWEEPUPPAUSE*/),
					luabind::value("SWEEPINGDOWN", 3 /*ACrab::SweepState::SWEEPINGDOWN*/),
					luabind::value("SWEEPDOWNPAUSE", 4 /*ACrab::SweepState::SWEEPDOWNPAUSE*/)
				]
				.enum_("DigState")[
					luabind::value("NOTDIGGING", 0 /*ACrab::DigState::NOTDIGGING*/),
					luabind::value("PREDIG", 1 /*ACrab::DigState::PREDIG*/),
					luabind::value("STARTDIG", 2 /*ACrab::DigState::STARTDIG*/),
					luabind::value("TUNNELING", 3 /*ACrab::DigState::TUNNELING*/),
					luabind::value("FINISHINGDIG", 4 /*ACrab::DigState::FINISHINGDIG*/),
					luabind::value("PAUSEDIGGER", 5 /*ACrab::DigState::PAUSEDIGGER*/)
				]
				.enum_("JumpState")[
					luabind::value("NOTJUMPING", 0 /*ACrab::JumpState::NOTJUMPING*/),
					luabind::value("FORWARDJUMP", 1 /*ACrab::JumpState::FORWARDJUMP*/),
					luabind::value("PREJUMP", 2 /*ACrab::JumpState::PREJUMP*/),
					luabind::value("UPJUMP", 3 /*ACrab::JumpState::UPJUMP*/),
					luabind::value("APEXJUMP", 4 /*ACrab::JumpState::APEXJUMP*/),
					luabind::value("LANDJUMP", 5 /*ACrab::JumpState::LANDJUMP*/)
				]
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
				.def("ReloadFirearm", &ACrab::ReloadFirearm)
				.def("IsWithinRange", &ACrab::IsWithinRange)
				.def("Look", &ACrab::Look)
				.def("LookForMOs", &ACrab::LookForMOs)
				.def("IsOnScenePoint", &ACrab::IsOnScenePoint)
				.def("GetLimbPath", &ACrab::GetLimbPath)
				.property("LimbPathPushForce", &ACrab::GetLimbPathPushForce, &ACrab::SetLimbPathPushForce)
				.def("GetLimbPathSpeed", &ACrab::GetLimbPathSpeed)
				.def("SetLimbPathSpeed", &ACrab::SetLimbPathSpeed);
		}

		LuaBindingRegisterFunctionForType(ACraft) {
			return AbstractTypeLuaClassDefinition(ACraft, Actor)
				.enum_("HatchState")[
					luabind::value("CLOSED", ACraft::HatchState::CLOSED),
					luabind::value("OPENING", ACraft::HatchState::OPENING),
					luabind::value("OPEN", ACraft::HatchState::OPEN),
					luabind::value("CLOSING", ACraft::HatchState::CLOSING),
					luabind::value("HatchStateCount", ACraft::HatchState::HatchStateCount)
				]
				.enum_("Side")[
					// Doesn't have qualifier
					luabind::value("RIGHT", ACraft::RIGHT),
					luabind::value("LEFT", ACraft::LEFT)
				]
				// These are all private/protected so they can't be bound, need to consider making them public.
				.enum_("CraftDeliverySequence")[
					luabind::value("FALL", 0 /*ACraft::CraftDeliverySequence::FALL*/),
					luabind::value("LAND", 1 /*ACraft::CraftDeliverySequence::LAND*/),
					luabind::value("STANDBY", 2 /*ACraft::CraftDeliverySequence::STANDBY*/),
					luabind::value("UNLOAD", 3 /*ACraft::CraftDeliverySequence::UNLOAD*/),
					luabind::value("LAUNCH", 4 /*ACraft::CraftDeliverySequence::LAUNCH*/),
					luabind::value("UNSTICK", 5 /*ACraft::CraftDeliverySequence::UNSTICK*/)
				]
				.enum_("AltitudeMoveState")[
					luabind::value("HOVER", 0 /*ACraft::AltitudeMoveState::HOVER*/),
					luabind::value("DESCEND", 1 /*ACraft::AltitudeMoveState::DESCEND*/),
					luabind::value("ASCEND", 2 /*ACraft::AltitudeMoveState::ASCEND*/)
				]
				.def("OpenHatch", &ACraft::OpenHatch)
				.def("CloseHatch", &ACraft::CloseHatch)
				.property("HatchState", &ACraft::GetHatchState)
				.property("HatchOpenSound", &ACraft::GetHatchOpenSound, &ACraftSetHatchOpenSound)
				.property("HatchCloseSound", &ACraft::GetHatchCloseSound, &ACraftSetHatchCloseSound)
				.property("CrashSound", &ACraft::GetCrashSound, &ACraftSetCrashSound)
				.property("MaxPassengers", &ACraft::GetMaxPassengers)
				.property("DeliveryDelayMultiplier", &ACraft::GetDeliveryDelayMultiplier);
		}

		LuaBindingRegisterFunctionForType(ACRocket) {
			return ConcreteTypeLuaClassDefinition(ACRocket, ACraft)
				// These are all private/protected so they can't be bound, need to consider making them public.
				.enum_("LandingGearState")[
					luabind::value("RAISED", 0 /*ACRocket::LandingGearState::RAISED*/),
					luabind::value("LOWERED", 1 /*ACRocket::LandingGearState::LOWERED*/),
					luabind::value("LOWERING", 2 /*ACRocket::LandingGearState::LOWERING*/),
					luabind::value("RAISING", 3 /*ACRocket::LandingGearState::RAISING*/),
					luabind::value("GearStateCount", 4 /*ACRocket::LandingGearState::GearStateCount*/)
				]
				.property("RightLeg", &ACRocket::GetRightLeg, &ACRocketSetRightLeg)
				.property("LeftLeg", &ACRocket::GetLeftLeg, &ACRocketSetLeftLeg)
				.property("MainEngine", &ACRocket::GetMainThruster, &ACRocketSetMainThruster)
				.property("LeftEngine", &ACRocket::GetLeftThruster, &ACRocketSetLeftThruster)
				.property("RightEngine", &ACRocket::GetRightThruster, &ACRocketSetRightThruster)
				.property("LeftThruster", &ACRocket::GetULeftThruster, &ACRocketSetULeftThruster)
				.property("RightThruster", &ACRocket::GetURightThruster, &ACRocketSetURightThruster)
				.property("GearState", &ACRocket::GetGearState);
		}

		LuaBindingRegisterFunctionForType(Actor) {
			return ConcreteTypeLuaClassDefinition(Actor, MOSRotating)
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
				]
				.def(luabind::constructor<>())
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
				.def("AddInventoryItem", &Actor::AddInventoryItem, luabind::adopt(_2))
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
				.def_readwrite("MovePath", &Actor::m_MovePath, luabind::return_stl_iterator)
				.def_readwrite("Inventory", &Actor::m_Inventory, luabind::return_stl_iterator)
				.def("SetAlarmPoint", &Actor::AlarmPoint)
				.def("GetAlarmPoint", &Actor::GetAlarmPoint)
				.property("AimDistance", &Actor::GetAimDistance, &Actor::SetAimDistance)
				.property("SightDistance", &Actor::GetSightDistance, &Actor::SetSightDistance);
		}

		LuaBindingRegisterFunctionForType(ADoor) {
			return ConcreteTypeLuaClassDefinition(ADoor, Actor)
				.enum_("DoorState")[
					luabind::value("CLOSED", ADoor::DoorState::CLOSED),
					luabind::value("OPENING", ADoor::DoorState::OPENING),
					luabind::value("OPEN", ADoor::DoorState::OPEN),
					luabind::value("CLOSING", ADoor::DoorState::CLOSING),
					luabind::value("STOPPED", ADoor::DoorState::STOPPED)
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
				.property("DoorMoveEndSound", &ADoor::GetDoorMoveEndSound, &ADoorSetDoorMoveEndSound);
		}

		LuaBindingRegisterFunctionForType(AEmitter) {
			return ConcreteTypeLuaClassDefinition(AEmitter, Attachable)
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
				.def_readwrite("Emissions", &AEmitter::m_EmissionList, luabind::return_stl_iterator);
		}

		LuaBindingRegisterFunctionForType(AHuman) {
			return ConcreteTypeLuaClassDefinition(AHuman, Actor)
				// These are all private/protected so they can't be bound, need to consider making them public.
				.enum_("UpperBodyState")[
					luabind::value("WEAPON_READY", 0 /*AHuman::UpperBodyState::WEAPON_READY*/),
					luabind::value("AIMING_SHARP", 1 /*AHuman::UpperBodyState::AIMING_SHARP*/),
					luabind::value("HOLSTERING_BACK", 2 /*AHuman::UpperBodyState::HOLSTERING_BACK*/),
					luabind::value("HOLSTERING_BELT", 3 /*AHuman::UpperBodyState::HOLSTERING_BELT*/),
					luabind::value("DEHOLSTERING_BACK", 4 /*AHuman::UpperBodyState::DEHOLSTERING_BACK*/),
					luabind::value("DEHOLSTERING_BELT", 5 /*AHuman::UpperBodyState::DEHOLSTERING_BELT*/),
					luabind::value("THROWING_PREP", 6 /*AHuman::UpperBodyState::THROWING_PREP*/),
					luabind::value("THROWING_RELEASE", 7 /*AHuman::UpperBodyState::THROWING_RELEASE*/)
				]
				.enum_("MovementState")[
					luabind::value("NOMOVE", 0 /*AHuman::MovementState::NOMOVE*/),
					luabind::value("STAND", 1 /*AHuman::MovementState::STAND*/),
					luabind::value("WALK", 2 /*AHuman::MovementState::WALK*/),
					luabind::value("CROUCH", 3 /*AHuman::MovementState::CROUCH*/),
					luabind::value("CRAWL", 4 /*AHuman::MovementState::CRAWL*/),
					luabind::value("ARMCRAWL", 5 /*AHuman::MovementState::ARMCRAWL*/),
					luabind::value("CLIMB", 6 /*AHuman::MovementState::CLIMB*/),
					luabind::value("JUMP", 7 /*AHuman::MovementState::JUMP*/),
					luabind::value("DISLODGE", 8 /*AHuman::MovementState::DISLODGE*/),
					luabind::value("MOVEMENTSTATECOUNT", 9 /*AHuman::MovementState::MOVEMENTSTATECOUNT*/)
				]
				.enum_("ProneState")[
					luabind::value("NOTPRONE", 0 /*AHuman::ProneState::NOTPRONE*/),
					luabind::value("GOPRONE", 1 /*AHuman::ProneState::GOPRONE*/),
					luabind::value("PRONE", 2 /*AHuman::ProneState::PRONE*/),
					luabind::value("PRONESTATECOUNT", 3 /*AHuman::ProneState::PRONESTATECOUNT*/)
				]
				.enum_("Layer")[
					// Doesn't have qualifier
					luabind::value("FGROUND", 0 /*AHuman::FGROUND*/),
					luabind::value("BGROUND", 1 /*AHuman::BGROUND*/)
				]
				.enum_("DeviceHandlingState")[
					luabind::value("STILL", 0 /*AHuman::DeviceHandlingState::STILL*/),
					luabind::value("POINTING", 1 /*AHuman::DeviceHandlingState::POINTING*/),
					luabind::value("SCANNING", 2 /*AHuman::DeviceHandlingState::SCANNING*/),
					luabind::value("AIMING", 3 /*AHuman::DeviceHandlingState::AIMING*/),
					luabind::value("FIRING", 4 /*AHuman::DeviceHandlingState::FIRING*/),
					luabind::value("THROWING", 5 /*AHuman::DeviceHandlingState::THROWING*/),
					luabind::value("DIGGING", 6 /*AHuman::DeviceHandlingState::DIGGING*/)
				]
				.enum_("SweepState")[
					luabind::value("NOSWEEP", 0 /*AHuman::SweepState::NOSWEEP*/),
					luabind::value("SWEEPINGUP", 1 /*AHuman::SweepState::SWEEPINGUP*/),
					luabind::value("SWEEPUPPAUSE", 2 /*AHuman::SweepState::SWEEPUPPAUSE*/),
					luabind::value("SWEEPINGDOWN", 3 /*AHuman::SweepState::SWEEPINGDOWN*/),
					luabind::value("SWEEPDOWNPAUSE", 4 /*AHuman::SweepState::SWEEPDOWNPAUSE*/)
				]
				.enum_("DigState")[
					luabind::value("NOTDIGGING", 0 /*AHuman::DigState::NOTDIGGING*/),
					luabind::value("PREDIG", 1 /*AHuman::DigState::PREDIG*/),
					luabind::value("STARTDIG", 2 /*AHuman::DigState::STARTDIG*/),
					luabind::value("TUNNELING", 3 /*AHuman::DigState::TUNNELING*/),
					luabind::value("FINISHINGDIG", 4 /*AHuman::DigState::FINISHINGDIG*/),
					luabind::value("PAUSEDIGGER", 5 /*AHuman::DigState::PAUSEDIGGER*/)
				]
				.enum_("JumpState")[
					luabind::value("NOTJUMPING", 0 /*AHuman::JumpState::NOTJUMPING*/),
					luabind::value("FORWARDJUMP", 1 /*AHuman::JumpState::FORWARDJUMP*/),
					luabind::value("PREJUMP", 2 /*AHuman::JumpState::PREJUMP*/),
					luabind::value("UPJUMP", 3 /*AHuman::JumpState::UPJUMP*/),
					luabind::value("APEXJUMP", 4 /*AHuman::JumpState::APEXJUMP*/),
					luabind::value("LANDJUMP", 5 /*AHuman::JumpState::LANDJUMP*/)
				]
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
				.def("SetRotAngleTarget", &AHuman::SetRotAngleTarget);
		}

		LuaBindingRegisterFunctionForType(Arm) {
			return ConcreteTypeLuaClassDefinition(Arm, Attachable)
				.property("HeldDevice", &Arm::GetHeldMO)
				.property("MaxLength", &Arm::GetMaxLength)
				.property("IdleOffset", &Arm::GetIdleOffset, &Arm::SetIdleOffset)
				.property("GripStrength", &Arm::GetGripStrength, &Arm::SetGripStrength)
				.property("HandPos", &Arm::GetHandPos, &Arm::SetHandPos);
		}

		LuaBindingRegisterFunctionForType(Attachable) {
			return ConcreteTypeLuaClassDefinition(Attachable, MOSRotating)
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
				.property("CanCollideWithTerrain", &Attachable::CanCollideWithTerrain);
		}

		LuaBindingRegisterFunctionForType(Deployment) {
			return AbstractTypeLuaClassDefinition(Deployment, SceneObject)
				.def("CreateDeployedActor", (Actor * (Deployment::*)())&Deployment::CreateDeployedActor, luabind::adopt(luabind::result))
				.def("CreateDeployedObject", (SceneObject * (Deployment::*)())&Deployment::CreateDeployedObject, luabind::adopt(luabind::result))
				.def("GetLoadoutName", &Deployment::GetLoadoutName)
				.property("SpawnRadius", &Deployment::GetSpawnRadius)
				.property("ID", &Deployment::GetID)
				.property("HFlipped", &Deployment::IsHFlipped);
		}

		LuaBindingRegisterFunctionForType(Emission) {
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

		LuaBindingRegisterFunctionForType(GlobalScript) {
			return AbstractTypeLuaClassDefinition(GlobalScript, Entity)
				.def("Deactivate", &GlobalScript::Deactivate);
		}

		LuaBindingRegisterFunctionForType(HDFirearm) {
			return ConcreteTypeLuaClassDefinition(HDFirearm, HeldDevice)
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
				.property("RecoilTransmission", &HDFirearm::GetJointStiffness, &HDFirearm::SetJointStiffness);
		}

		LuaBindingRegisterFunctionForType(HeldDevice) {
			return ConcreteTypeLuaClassDefinition(HeldDevice, Attachable)
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
				.def("SetSupported", &HeldDevice::SetSupported);
		}

		LuaBindingRegisterFunctionForType(Leg) {
			return ConcreteTypeLuaClassDefinition(Leg, Attachable)
				.property("Foot", &Leg::GetFoot, &LegSetFoot);
		}

		LuaBindingRegisterFunctionForType(LimbPath) {
			return luabind::class_<LimbPath>("LimbPath")
				.property("StartOffset", &LimbPath::GetStartOffset, &LimbPath::SetStartOffset)
				.property("SegmentCount", &LimbPath::GetSegCount)
				.def("GetSegment", &LimbPath::GetSegment);
		}

		LuaBindingRegisterFunctionForType(Magazine) {
			return ConcreteTypeLuaClassDefinition(Magazine, Attachable)
				.property("NextRound", &Magazine::GetNextRound)
				.property("RoundCount", &Magazine::GetRoundCount, &Magazine::SetRoundCount)
				.property("IsEmpty", &Magazine::IsEmpty)
				.property("IsFull", &Magazine::IsFull)
				.property("IsOverHalfFull", &Magazine::IsOverHalfFull)
				.property("Capacity", &Magazine::GetCapacity)
				.property("Discardable", &Magazine::IsDiscardable);
		}

		LuaBindingRegisterFunctionForType(Material) {
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

		LuaBindingRegisterFunctionForType(MetaPlayer) {
			return luabind::class_<MetaPlayer>("MetaPlayer")
				.def(luabind::constructor<>())
				.property("NativeTechModule", &MetaPlayer::GetNativeTechModule)
				.property("ForeignCostMultiplier", &MetaPlayer::GetForeignCostMultiplier)
				.property("NativeCostMultiplier", &MetaPlayer::GetNativeCostMultiplier)
				.property("InGamePlayer", &MetaPlayer::GetInGamePlayer)
				.property("BrainPoolCount", &MetaPlayer::GetBrainPoolCount, &MetaPlayer::SetBrainPoolCount)
				.def("ChangeBrainPoolCount", &MetaPlayer::ChangeBrainPoolCount);
		}

		LuaBindingRegisterFunctionForType(MOPixel) {
			return ConcreteTypeLuaClassDefinition(MOPixel, MovableObject)
				.property("TrailLength", &MOPixel::GetTrailLength, &MOPixel::SetTrailLength);
		}

		LuaBindingRegisterFunctionForType(MOSParticle) {
			return ConcreteTypeLuaClassDefinition(MOSParticle, MOSprite);
		}

		LuaBindingRegisterFunctionForType(MOSprite) {
			return AbstractTypeLuaClassDefinition(MOSprite, MovableObject)
				.enum_("SpriteAnimMode")[
					luabind::value("NOANIM", MOSprite::SpriteAnimMode::NOANIM),
					luabind::value("ALWAYSLOOP", MOSprite::SpriteAnimMode::ALWAYSLOOP),
					luabind::value("ALWAYSRANDOM", MOSprite::SpriteAnimMode::ALWAYSRANDOM),
					luabind::value("ALWAYSPINGPONG", MOSprite::SpriteAnimMode::ALWAYSPINGPONG),
					luabind::value("LOOPWHENMOVING", MOSprite::SpriteAnimMode::LOOPWHENMOVING),
					luabind::value("LOOPWHENOPENCLOSE", MOSprite::SpriteAnimMode::LOOPWHENOPENCLOSE),
					luabind::value("PINGPONGOPENCLOSE", MOSprite::SpriteAnimMode::PINGPONGOPENCLOSE),
					luabind::value("OVERLIFETIME", MOSprite::SpriteAnimMode::OVERLIFETIME),
					luabind::value("ONCOLLIDE", MOSprite::SpriteAnimMode::ONCOLLIDE)
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
				.def("GetExitWoundPresetName", &MOSprite::GetExitWoundPresetName);
		}

		LuaBindingRegisterFunctionForType(MOSRotating) {
			return ConcreteTypeLuaClassDefinition(MOSRotating, MOSprite)
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
				.def("AddEmitter", (void (MOSRotating::*)(Attachable *attachableToAdd))&MOSRotating::AddAttachable, luabind::adopt(_2))
				.def("AddEmitter", (void (MOSRotating::*)(Attachable *attachableToAdd, const Vector &parentOffset))&MOSRotating::AddAttachable, luabind::adopt(_2))
				.def("RemoveEmitter", (bool (MOSRotating::*)(Attachable *attachableToRemove))&MOSRotating::RemoveAttachable)
				.def("RemoveEmitter", (bool (MOSRotating::*)(long uniqueIDOfAttachableToRemove))&MOSRotating::RemoveAttachable)
				.def_readonly("Attachables", &MOSRotating::m_Attachables, luabind::return_stl_iterator)
				.def_readonly("Wounds", &MOSRotating::m_Wounds, luabind::return_stl_iterator);
		}

		LuaBindingRegisterFunctionForType(MovableObject) {
			return AbstractTypeLuaClassDefinition(MovableObject, SceneObject)
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
				.property("HitWhatParticleUniqueID", &MovableObject::HitWhatParticleUniqueID);
		}

		LuaBindingRegisterFunctionForType(PEmitter) {
			return ConcreteTypeLuaClassDefinition(PEmitter, MOSParticle)
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
				.def_readwrite("Emissions", &PEmitter::m_EmissionList, luabind::return_stl_iterator);
		}

		LuaBindingRegisterFunctionForType(Round) {
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

		LuaBindingRegisterFunctionForType(Scene) {
			return ConcreteTypeLuaClassDefinition(Scene, Entity)
				.enum_("PlacedObjectSets")[
					luabind::value("PLACEONLOAD", Scene::PlacedObjectSets::PLACEONLOAD),
					luabind::value("BLUEPRINT", Scene::PlacedObjectSets::BLUEPRINT),
					luabind::value("AIPLAN", Scene::PlacedObjectSets::AIPLAN),
					luabind::value("PLACEDSETSCOUNT", Scene::PlacedObjectSets::PLACEDSETSCOUNT)
				]
				.property("Location", &Scene::GetLocation, &Scene::SetLocation)
				//.property("Terrain", &Scene::GetTerrain)
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
				.def_readwrite("ScenePath", &Scene::m_ScenePath, luabind::return_stl_iterator)
				.def_readwrite("Deployments", &Scene::m_Deployments, luabind::return_stl_iterator)
				.property("ScenePathSize", &Scene::GetScenePathSize);
		}

		LuaBindingRegisterFunctionForType(SceneArea) {
			return luabind::class_<Scene::Area>("Area")
				.def(luabind::constructor<>())
				.def(luabind::constructor<std::string>())
				.def(luabind::constructor<const Scene::Area &>())
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
				.def("GetRandomPoint", &Scene::Area::GetRandomPoint);
		}

		LuaBindingRegisterFunctionForType(SceneObject) {
			return AbstractTypeLuaClassDefinition(SceneObject, Entity)
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
				.def("IsOnScenePoint", &SceneObject::IsOnScenePoint);
		}

		LuaBindingRegisterFunctionForType(SoundContainer) {
			return ConcreteTypeLuaClassDefinition(SoundContainer, Entity)
				.def(luabind::constructor<>())
				.enum_("SoundOverlapMode")[
					luabind::value("OVERLAP", SoundContainer::SoundOverlapMode::OVERLAP),
					luabind::value("RESTART", SoundContainer::SoundOverlapMode::RESTART),
					luabind::value("IGNORE_PLAY", SoundContainer::SoundOverlapMode::IGNORE_PLAY)
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
				.def("FadeOut", &SoundContainer::FadeOut);
		}

		LuaBindingRegisterFunctionForType(SoundSet) {
			return luabind::class_<SoundSet>("SoundSet")
				.def(luabind::constructor<>())
				.enum_("SoundSelectionCycleMode")[
					luabind::value("RANDOM", SoundSet::SoundSelectionCycleMode::RANDOM),
					luabind::value("FORWARDS", SoundSet::SoundSelectionCycleMode::FORWARDS),
					luabind::value("ALL", SoundSet::SoundSelectionCycleMode::ALL)
				]
				.property("SoundSelectionCycleMode", &SoundSet::GetSoundSelectionCycleMode, &SoundSet::SetSoundSelectionCycleMode)
				.def_readonly("SubSoundSets", &SoundSet::m_SubSoundSets, luabind::return_stl_iterator)
				.def("HasAnySounds", &SoundSet::HasAnySounds)
				.def("SelectNextSounds", &SoundSet::SelectNextSounds)
				.def("AddSound", (void (SoundSet:: *)(std::string const &soundFilePath)) &SoundSet::AddSound)
				.def("AddSound", (void (SoundSet:: *)(std::string const &soundFilePath, const Vector &offset, float minimumAudibleDistance, float attenuationStartDistance)) &SoundSet::AddSound)
				.def("AddSoundSet", &SoundSet::AddSoundSet);
		}

		LuaBindingRegisterFunctionForType(TDExplosive) {
			return ConcreteTypeLuaClassDefinition(TDExplosive, ThrownDevice)
				.property("IsAnimatedManually", &TDExplosive::IsAnimatedManually, &TDExplosive::SetAnimatedManually);
		}

		LuaBindingRegisterFunctionForType(TerrainObject) {
			return ConcreteTypeLuaClassDefinition(TerrainObject, SceneObject)
				.def("GetBitmapOffset", &TerrainObject::GetBitmapOffset)
				.def("GetBitmapWidth", &TerrainObject::GetBitmapWidth)
				.def("GetBitmapHeight", &TerrainObject::GetBitmapHeight);
		}

		LuaBindingRegisterFunctionForType(ThrownDevice) {
			return ConcreteTypeLuaClassDefinition(ThrownDevice, HeldDevice)
				.property("MinThrowVel", &ThrownDevice::GetMinThrowVel, &ThrownDevice::SetMinThrowVel)
				.property("MaxThrowVel", &ThrownDevice::GetMaxThrowVel, &ThrownDevice::SetMaxThrowVel);
		}

		LuaBindingRegisterFunctionForType(Turret) {
			return ConcreteTypeLuaClassDefinition(Turret, Attachable)
				.property("MountedDevice", &Turret::GetMountedDevice, &TurretSetMountedDevice);
		}
	};
}
#endif