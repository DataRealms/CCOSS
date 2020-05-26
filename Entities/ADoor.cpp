#include "ADoor.h"
#include "AtomGroup.h"
#include "Attachable.h"
#include "Matrix.h"
#include "SLTerrain.h"

namespace RTE {

	ConcreteClassInfo(ADoor, Actor, 0)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::Clear() {
		m_InitialSpriteAnimDuration = m_SpriteAnimDuration;
		m_Sensors.clear();
		m_SensorTimer.Reset();
		m_SensorInterval = 1000;
		m_Door = 0;
		m_DoorState = CLOSED;
		m_ClosedByDefault = true;
		m_OpenOffset.Reset();
		m_ClosedOffset.Reset();
		m_OpenAngle = 0;
		m_ClosedAngle = 0;
		m_ChangedDirectionAfterStop = false;
		m_DoorMoveTimer.Reset();
		m_DoorMoveTime = 0;
		m_DoorMoveStopTime = 0;
		m_ResumeAfterStop = false;
		m_ResetDefaultTimer.Reset();
		m_ResetDefaultDelay = 8000;
		m_DrawMaterialLayerWhenOpen = true;
		m_DrawMaterialLayerWhenClosed = true;
		m_DoorMaterialDrawn = false;
		m_MaterialDrawOverride = false;
		m_LastDoorMaterialPos.Reset();
		m_DoorMoveStartSound.Reset();
		m_DoorMoveSound.Reset();
		m_DoorDirectionChangeSound.Reset();
		m_DoorMoveEndSound.Reset();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ADoor::Create(const ADoor &reference) {
		Actor::Create(reference);

		if (reference.m_Door) {
			m_Door = dynamic_cast<Attachable *>(reference.m_Door->Clone());
			AddAttachable(m_Door, m_ClosedOffset, true);
		}
		m_OpenOffset = reference.m_OpenOffset;
		m_ClosedOffset = reference.m_ClosedOffset;
		m_OpenAngle = reference.m_OpenAngle;
		m_ClosedAngle = reference.m_ClosedAngle;
		m_DoorState = reference.m_DoorState;
		m_DoorMoveTime = reference.m_DoorMoveTime;
		m_DoorMoveStartSound = reference.m_DoorMoveStartSound;
		m_DoorMoveSound = reference.m_DoorMoveSound;
		m_DoorDirectionChangeSound = reference.m_DoorDirectionChangeSound;
		m_DoorMoveEndSound = reference.m_DoorMoveEndSound;
		m_ClosedByDefault = reference.m_ClosedByDefault;
		m_ResetDefaultDelay = reference.m_ResetDefaultDelay;

		// Set the initial door state to the opposite of the default, so it'll go to the default first thing.
		// This will look cool, demonstrate that it's a movable door and will force the drawing of the default state of door material.
		m_DoorState = reference.m_ClosedByDefault ? OPEN : CLOSED;

		for (const ADSensor &sensor : reference.m_Sensors) {
			m_Sensors.push_back(sensor);
		}
		m_SensorInterval = reference.m_SensorInterval;
		m_DrawMaterialLayerWhenOpen = reference.m_DrawMaterialLayerWhenOpen;
		m_DrawMaterialLayerWhenClosed = reference.m_DrawMaterialLayerWhenClosed;
		m_MaterialDrawOverride = reference.m_MaterialDrawOverride;
		m_DoorMaterialID = reference.m_DoorMaterialID;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ADoor::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "Door") {
			delete m_Door;
			m_Door = new Attachable;
			reader >> m_Door;
			m_DoorMaterialID = m_Door->GetMaterial()->GetIndex();
		} else if (propName == "OpenOffset") {
			reader >> m_OpenOffset;
		} else if (propName == "ClosedOffset") {
			reader >> m_ClosedOffset;
		} else if (propName == "OpenClosedOffset") {
			reader >> m_OpenOffset;
			m_ClosedOffset = m_OpenOffset;
		} else if (propName == "OpenAngle") {
			Matrix rotation;
			reader >> rotation;
			m_OpenAngle = rotation.GetRadAngle();
		} else if (propName == "ClosedAngle") {
			Matrix rotation;
			reader >> rotation;
			m_ClosedAngle = rotation.GetRadAngle();
		} else if (propName == "OpenClosedAngle") {
			Matrix rotation;
			reader >> rotation;
			m_OpenAngle = rotation.GetRadAngle();
			m_ClosedAngle = rotation.GetRadAngle();
		} else if (propName == "DoorMoveTime") {
			reader >> m_DoorMoveTime;
		} else if (propName == "ClosedByDefault") {
			reader >> m_ClosedByDefault;
		} else if (propName == "ResetDefaultDelay") {
			reader >> m_ResetDefaultDelay;
		} else if (propName == "SensorInterval") {
			reader >> m_SensorInterval;
		} else if (propName == "AddSensor") {
			ADSensor sensor;
			reader >> sensor;
			m_Sensors.push_back(sensor);
		} else if (propName == "DrawMaterialLayerWhenOpen") {
			reader >> m_DrawMaterialLayerWhenOpen;
		} else if (propName == "DrawMaterialLayerWhenClosed") {
			reader >> m_DrawMaterialLayerWhenClosed;
		} else if (propName == "DoorMoveStartSound") {
			reader >> m_DoorMoveStartSound;
		} else if (propName == "DoorMoveSound") {
			reader >> m_DoorMoveSound;
		} else if (propName == "DoorDirectionChangeSound") {
			reader >> m_DoorDirectionChangeSound;
		} else if (propName == "DoorMoveEndSound") {
			reader >> m_DoorMoveEndSound;
		} else {
			// See if the base class(es) can find a match instead
			return Actor::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ADoor::Save(Writer &writer) const {
		Actor::Save(writer);

		writer.NewProperty("Door");
		writer << m_Door;
		writer.NewProperty("OpenOffset");
		writer << m_OpenOffset;
		writer.NewProperty("ClosedOffset");
		writer << m_ClosedOffset;
		writer.NewProperty("OpenAngle");
		writer << Matrix(m_OpenAngle);
		writer.NewProperty("ClosedAngle");
		writer << Matrix(m_ClosedAngle);
		writer.NewProperty("DoorDelay");
		writer << m_DoorMoveTime;
		writer.NewProperty("ClosedByDefault");
		writer << m_ClosedByDefault;
		writer.NewProperty("ResetDefaultDelay");
		writer << m_ResetDefaultDelay;
		writer.NewProperty("SensorInterval");
		writer << m_SensorInterval;
		for (const ADSensor &sensor : m_Sensors) {
			writer.NewProperty("AddSensor");
			writer << sensor;
		}
		writer.NewProperty("DrawMaterialLayerWhenOpen");
		writer << m_DrawMaterialLayerWhenOpen;
		writer.NewProperty("DrawMaterialLayerWhenClosed");
		writer << m_DrawMaterialLayerWhenClosed;
		writer.NewProperty("DoorMoveStartSound");
		writer << m_DoorMoveStartSound;
		writer.NewProperty("DoorMoveSound");
		writer << m_DoorMoveSound;
		writer.NewProperty("DoorDirectionChangeSound");
		writer << m_DoorDirectionChangeSound;
		writer.NewProperty("DoorMoveEndSound");
		writer << m_DoorMoveEndSound;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::Destroy(bool notInherited) {
		m_DoorMoveStartSound.Stop();
		m_DoorMoveSound.Stop();
		m_DoorDirectionChangeSound.Stop();
		m_DoorMoveEndSound.Stop();
		delete m_Door;
		if (!notInherited) { Actor::Destroy(); }

		for (ADSensor &sensor : m_Sensors) {
			sensor.Destroy();
		}
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float ADoor::GetMass() const {
		float totalMass = Actor::GetMass();
		if (m_Door) { totalMass += m_Door->GetMass(); }
		return totalMass;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::GetMOIDs(std::vector<MOID> &MOIDs) const {
		if (m_Door) { m_Door->GetMOIDs(MOIDs); }
		Actor::GetMOIDs(MOIDs);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::SetID(const MOID newID) {
		Actor::SetID(newID);
		if (m_Door) { m_Door->SetID(newID); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ADoor::IsOnScenePoint(Vector &scenePoint) const {
		return ((m_Door && m_Door->IsOnScenePoint(scenePoint)) || Actor::IsOnScenePoint(scenePoint));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::DrawDoorMaterial() {
		if (!m_Door || m_MaterialDrawOverride || !g_SceneMan.GetTerrain() || !g_SceneMan.GetTerrain()->GetMaterialBitmap()) {
			return;
		}

		// Erase the previous drawing if it already exists
		if (m_DoorMaterialDrawn) {
			int fillX = m_LastDoorMaterialPos.GetFloorIntX();
			int fillY = m_LastDoorMaterialPos.GetFloorIntY();

			if (g_SceneMan.GetTerrMatter(fillX, fillY) != g_MaterialAir) { floodfill(g_SceneMan.GetTerrain()->GetMaterialBitmap(), fillX, fillY, g_MaterialAir); }
		}

		// Now draw the material silhouette to the terrain
		Vector notUsed;
		m_Door->Draw(g_SceneMan.GetTerrain()->GetMaterialBitmap(), notUsed, g_DrawMaterial, true);
		m_LastDoorMaterialPos = m_Door->GetPos();
		m_DoorMaterialDrawn = true;

		// Register that we changed the material layer of the terrain
		g_SceneMan.GetTerrain()->AddUpdatedMaterialArea(m_Door->GetBoundingBox());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ADoor::EraseDoorMaterial() {
		m_DoorMaterialDrawn = false;

		if (!m_Door || !g_SceneMan.GetTerrain() || !g_SceneMan.GetTerrain()->GetMaterialBitmap()) {
			return false;
		}

		int fillX = m_LastDoorMaterialPos.GetFloorIntX();
		int fillY = m_LastDoorMaterialPos.GetFloorIntY();

		if (g_SceneMan.GetTerrMatter(fillX, fillY) != g_MaterialAir) {
			floodfill(g_SceneMan.GetTerrain()->GetMaterialBitmap(), fillX, fillY, g_MaterialAir);
			// Register that we changed the material layer of the terrain
			g_SceneMan.GetTerrain()->AddUpdatedMaterialArea(m_Door->GetBoundingBox());
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::MaterialDrawOverride(bool enable) {
		if (!g_SceneMan.GetTerrain() || !g_SceneMan.GetTerrain()->GetMaterialBitmap()) {
			return;
		}
		if (enable) {
			// Erase the material temporarily if we have drawn it and the override isn't already in effect
			if (m_DoorMaterialDrawn && m_MaterialDrawOverride != enable) {
				int fillX = m_LastDoorMaterialPos.GetFloorIntX();
				int fillY = m_LastDoorMaterialPos.GetFloorIntY();

				if (g_SceneMan.GetTerrMatter(fillX, fillY) != g_MaterialAir) {
					floodfill(g_SceneMan.GetTerrain()->GetMaterialBitmap(), fillX, fillY, g_MaterialAir);
					// Register that we changed the material layer of the terrain
					g_SceneMan.GetTerrain()->AddUpdatedMaterialArea(m_Door->GetBoundingBox());
				}
			}
			m_MaterialDrawOverride = enable;
		} else {
			// Draw the door back if we were indeed temporarily suppressing it before
			if (m_DoorMaterialDrawn && m_MaterialDrawOverride != enable) {
				Vector notUsed;
				m_Door->Draw(g_SceneMan.GetTerrain()->GetMaterialBitmap(), notUsed, g_DrawMaterial, true);
				// Register that we changed the material layer of the terrain
				g_SceneMan.GetTerrain()->AddUpdatedMaterialArea(m_Door->GetBoundingBox());
			}
			m_MaterialDrawOverride = enable;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ADoor::RemoveAnyRandomWounds(int amount) {
		float damage = 0;
		for (int i = 0; i < amount; i++) {
			// Fill the list of damaged bodyparts
			std::vector<MOSRotating *> bodyParts;
			if (GetWoundCount() > 0) { bodyParts.push_back(this); }
			if (m_Door && m_Door->GetWoundCount()) { bodyParts.push_back(m_Door); }

			// Stop removing wounds if there are not any left
			if (bodyParts.size() == 0) {
				break;
			}

			int partIndex = RangeRand(0, bodyParts.size() - 1);
			MOSRotating * part = bodyParts[partIndex];
			damage += part->RemoveWounds(1);
		}
		return damage;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::GibThis(Vector impactImpulse, float internalBlast, MovableObject *ignoreMO) {
		if (m_Door && m_Door->IsAttached()) {
			// Remove the material representation of the door, if any
			EraseDoorMaterial();
			m_Door->DeepCheck(true);
			m_Door->SetPinStrength(0);
			m_Door->SetVel(m_Vel + m_Door->GetParentOffset() * PosRand());
			m_Door->SetAngularVel(NormalRand());
			g_MovableMan.AddParticle(m_Door);
			RemoveAttachable(m_Door);
			m_Door = 0;
		}
		Actor::GibThis(impactImpulse, internalBlast, ignoreMO);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::UpdateChildMOIDs(vector<MovableObject *> &MOIDIndex, MOID rootMOID, bool makeNewMOID) {
		if (m_Door) { m_Door->UpdateMOID(MOIDIndex, m_RootMOID, makeNewMOID); }
		Actor::UpdateChildMOIDs(MOIDIndex, m_RootMOID, makeNewMOID);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::OpenDoor() {
		if (m_DoorState == STOPPED) {
			SharedDoorControls();
			m_DoorState = CLOSING;
			m_ChangedDirectionAfterStop = (m_DoorState == m_DoorStateOnStop) ? false : true;
		}
		if (m_DoorState == CLOSED || m_DoorState == CLOSING) {
			SharedDoorControls();
			m_DoorState = OPENING;
		}
		m_ResetDefaultTimer.Reset();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::CloseDoor() {
		if (m_DoorState == STOPPED) {
			SharedDoorControls();
			m_DoorState = OPENING;
			m_ChangedDirectionAfterStop = (m_DoorState == m_DoorStateOnStop) ? false : true;
		}
		if (m_DoorState == OPEN || m_DoorState == OPENING) {
			SharedDoorControls();
			m_DoorState = CLOSING;
		}
		m_ResetDefaultTimer.Reset();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::StopDoor() {
		if (m_DoorState == OPENING || m_DoorState == CLOSING) {
			if (m_DrawMaterialLayerWhenOpen || m_DrawMaterialLayerWhenClosed) { DrawDoorMaterial(); }
			m_DoorMoveStopTime = m_DoorMoveTime - m_DoorMoveTimer.GetElapsedSimTimeMS();
			m_DoorStateOnStop = m_DoorState;
			m_DoorMoveSound.Stop();
			m_DoorMoveEndSound.Play(m_Pos);
			m_DoorState = STOPPED;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::SharedDoorControls() {
		if (m_DoorState == OPEN || m_DoorState == CLOSED) {
			m_DoorMoveStartSound.Play(m_Pos);
			m_DoorMoveTimer.Reset();
			if (m_DoorMaterialDrawn) { EraseDoorMaterial(); }
			if (m_Door) { m_Door->DeepCheck(true); }
		} else if (m_DoorState == OPENING || m_DoorState == CLOSING) {
			m_DoorMoveSound.Stop();
			if (!m_ResumeAfterStop) {
				m_DoorDirectionChangeSound.Play(m_Pos);
				m_DoorMoveTimer.SetElapsedSimTimeMS(m_DoorMoveTime - m_DoorMoveTimer.GetElapsedSimTimeMS());
			} else {
				m_DoorMoveStartSound.Play(m_Pos);
				m_DoorMoveTimer.SetElapsedSimTimeMS(m_ChangedDirectionAfterStop ? m_DoorMoveTime - m_DoorMoveStopTime : m_DoorMoveStopTime);
				m_ChangedDirectionAfterStop = false;
				m_ResumeAfterStop = false;
			}
		} else if (m_DoorState == STOPPED) {
			if (m_DoorMaterialDrawn) { EraseDoorMaterial(); }
			m_ResumeAfterStop = true;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::Update() {
		Actor::Update();

		if (m_Door && m_Door->IsAttached()) {
			if (m_DoorState != STOPPED && m_SensorTimer.IsPastSimMS(m_SensorInterval)) { UpdateSensors(); }
			UpdateDoorAttachable();
		}

		if (m_Door && !ApplyAttachableForces(m_Door)) {
			// Remove the material representation if the door fell off
			EraseDoorMaterial();
			// Start the spinning out of control animation for the motor, start it slow
			m_SpriteAnimDuration *= 4;
			m_Door = 0;
		}

		if (m_SpriteAnimMode == LOOPWHENOPENCLOSE && m_FrameCount > 1 && (m_DoorState == OPENING || m_DoorState == CLOSING) && m_SpriteAnimTimer.IsPastSimMS(m_SpriteAnimDuration)) {
			m_Frame = (m_Frame + 1) % m_FrameCount;
			m_SpriteAnimTimer.Reset();
		}

		// Lose health when door is lost, spinning out of control until grinds to halt
		if (!m_Door && m_Status != DYING && m_Status != DEAD) {
			m_SpriteAnimMode = ALWAYSLOOP;
			m_SpriteAnimDuration = LERP(0, m_MaxHealth, 10, m_InitialSpriteAnimDuration, m_Health);

			// TODO: Figure out how to pitch this correctly so it goes up as SpriteAnimDuration goes down.
			/*
			if (!m_DoorMoveSound.IsBeingPlayed()) { m_DoorMoveSound.Play(m_Pos); }
			g_AudioMan.SetSoundPitch(&m_DoorMoveSound, LERP(0, m_InitialSpriteAnimDuration, 0, 5, m_SpriteAnimDuration));
			*/

			m_Health -= 0.4F;
		}
		if (m_Status == DEAD) { GibThis(Vector(), 50); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::UpdateSensors() {		
		const Actor *foundActor = 0;
		bool anySensorInput = false;

		for (ADSensor &sensor : m_Sensors) {
			foundActor = sensor.SenseActor(m_Pos, m_Rotation, m_HFlipped, m_MOID);
			if (foundActor && foundActor->IsControllable()) {
				anySensorInput = true;
				// Look for enemy actors first and break the loop so we don't accidentally open the door for a friendly actor is enemies are nearby.
				if (foundActor->GetTeam() != m_Team) {
					CloseDoor();
					break;
				} else if (foundActor->GetTeam() == m_Team) {					
					OpenDoor();
				}
			}
		}
		// If no sensor input, return to default state if it's time. This won't override any actions that the sensors started
		if (!anySensorInput && m_ResetDefaultTimer.IsPastSimMS(m_ResetDefaultDelay)) {
			if (m_ClosedByDefault && m_DoorState == OPEN) {
				CloseDoor();
			} else if (!m_ClosedByDefault && m_DoorState == CLOSED) {
				OpenDoor();
			}
		}
		m_SensorTimer.Reset();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::UpdateDoorAttachable() {
		m_Door->SetHFlipped(m_HFlipped);
		switch (m_DoorState) {
			case OPEN:
				m_Door->SetJointPos(m_Pos + m_OpenOffset.GetXFlipped(m_HFlipped) * m_Rotation);
				m_Door->SetRotAngle(m_Rotation.GetRadAngle() + (m_OpenAngle * GetFlipFactor()));
				break;
			case OPENING:
				if (!m_DoorMoveSound.IsBeingPlayed()) { m_DoorMoveSound.Play(m_Pos); }

				if (m_DoorMoveTimer.IsPastSimMS(m_DoorMoveTime)) {
					m_ResetDefaultTimer.Reset();

					m_Door->SetJointPos(m_Pos + m_OpenOffset.GetXFlipped(m_HFlipped) * m_Rotation);
					m_Door->SetRotAngle(m_Rotation.GetRadAngle() + (m_OpenAngle * GetFlipFactor()));

					// Draw the material representation of the door to the terrain in the open state
					if (m_DrawMaterialLayerWhenOpen) { DrawDoorMaterial(); }

					m_DoorMoveSound.Stop();
					m_DoorMoveEndSound.Play(m_Pos);

					m_DoorState = OPEN;
				} else {
					// Find interpolated positions and angles for opening
					Vector openingOffset(LERP(0, m_DoorMoveTime, m_ClosedOffset.m_X, m_OpenOffset.m_X, m_DoorMoveTimer.GetElapsedSimTimeMS()), LERP(0, m_DoorMoveTime, m_ClosedOffset.m_Y, m_OpenOffset.m_Y, m_DoorMoveTimer.GetElapsedSimTimeMS()));
					// TODO: Make this work across rotation 0
					float openingAngle = LERP(0, m_DoorMoveTime, m_ClosedAngle, m_OpenAngle, m_DoorMoveTimer.GetElapsedSimTimeMS());

					m_Door->SetJointPos(m_Pos + openingOffset.GetXFlipped(m_HFlipped) * m_Rotation);
					m_Door->SetRotAngle(m_Rotation.GetRadAngle() + (openingAngle * GetFlipFactor()));

					// Do the deep check to clear away any terrain debris when the door is moving but only after a little while so it doesn't take a chunk out of the ground before being snapped into place
					if (m_DoorMoveTimer.IsPastSimMS(50)) { m_Door->DeepCheck(true); }
				}
				break;
			case CLOSED:
				m_Door->SetJointPos(m_Pos + m_ClosedOffset.GetXFlipped(m_HFlipped) * m_Rotation);
				m_Door->SetRotAngle(m_Rotation.GetRadAngle() + (m_ClosedAngle * GetFlipFactor()));
				break;
			case CLOSING:
				if (!m_DoorMoveSound.IsBeingPlayed()) { m_DoorMoveSound.Play(m_Pos); }

				if (m_DoorMoveTimer.IsPastSimMS(m_DoorMoveTime)) {
					m_ResetDefaultTimer.Reset();
					
					m_Door->SetJointPos(m_Pos + m_ClosedOffset.GetXFlipped(m_HFlipped) * m_Rotation);
					m_Door->SetRotAngle(m_Rotation.GetRadAngle() + (m_ClosedAngle * GetFlipFactor()));

					// Draw the material representation of the door to the terrain in the closed state
					if (m_DrawMaterialLayerWhenClosed) { DrawDoorMaterial(); }

					m_DoorMoveSound.Stop();
					m_DoorMoveEndSound.Play(m_Pos);

					m_DoorState = CLOSED;
				} else {
					// Find interpolated positions and angles for closing
					Vector closingOffset(LERP(0, m_DoorMoveTime, m_OpenOffset.m_X, m_ClosedOffset.m_X, m_DoorMoveTimer.GetElapsedSimTimeMS()), LERP(0, m_DoorMoveTime, m_OpenOffset.m_Y, m_ClosedOffset.m_Y, m_DoorMoveTimer.GetElapsedSimTimeMS()));
					// TODO: Make this work across rotation 0
					float closingAngle = LERP(0, m_DoorMoveTime, m_OpenAngle, m_ClosedAngle, m_DoorMoveTimer.GetElapsedSimTimeMS());

					m_Door->SetJointPos(m_Pos + closingOffset.GetXFlipped(m_HFlipped) * m_Rotation);
					m_Door->SetRotAngle(m_Rotation.GetRadAngle() + (closingAngle * GetFlipFactor()));

					// Do the deep check to clear away any terrain debris when the door is moving but only after a little while so it doesn't take a chunk out of the ground before being snapped into place
					if (m_DoorMoveTimer.IsPastSimMS(50)) { m_Door->DeepCheck(true); }
				}
				break;
			case STOPPED:
				break;
			default:
				RTEAbort("Undefined door state value passed in. See DoorState enumeration for defined values.");
				break;
		}
		m_Door->Update();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::Draw(BITMAP *targetBitmap, const Vector &targetPos, DrawMode mode, bool onlyPhysical) const {
		// Override color drawing with flash, if requested.
		DrawMode realMode = (mode == g_DrawColor && m_FlashWhiteMS) ? g_DrawWhite : mode;

		if (m_Door && m_Door->IsAttached()) {
			if (!m_Door->IsDrawnAfterParent()) {
				m_Door->Draw(targetBitmap, targetPos, realMode, onlyPhysical);
				Actor::Draw(targetBitmap, targetPos, mode, onlyPhysical);
			} else {
				Actor::Draw(targetBitmap, targetPos, mode, onlyPhysical);
				m_Door->Draw(targetBitmap, targetPos, realMode, onlyPhysical);
			}
		} else {
			Actor::Draw(targetBitmap, targetPos, mode, onlyPhysical);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::DrawHUD(BITMAP *targetBitmap, const Vector &targetPos, int whichScreen, bool playerControlled) {
		if (!m_HUDVisible) {
			return;
		}
		// Only draw if the team viewing this is on the same team OR has seen the space where this is located
		int viewingTeam = g_ActivityMan.GetActivity()->GetTeamOfPlayer(g_ActivityMan.GetActivity()->PlayerOfScreen(whichScreen));
		if (viewingTeam != m_Team && viewingTeam != Activity::NOTEAM && g_SceneMan.IsUnseen(m_Pos.m_X, m_Pos.m_Y, viewingTeam)) {
			return;
		}
	}
}