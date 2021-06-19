#include "ADoor.h"
#include "AtomGroup.h"
#include "Attachable.h"
#include "Matrix.h"
#include "SLTerrain.h"
#include "PresetMan.h"

namespace RTE {

	ConcreteClassInfo(ADoor, Actor, 20)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::Clear() {
		m_InitialSpriteAnimDuration = 0;
		m_Sensors.clear();
		m_SensorTimer.Reset();
		m_SensorInterval = 1000;
		m_Door = 0;
		m_DoorState = CLOSED;
		m_DoorStateOnStop = CLOSED;
		m_ClosedByDefault = true;
		m_OpenOffset.Reset();
		m_ClosedOffset.Reset();
		m_OpenAngle = 0;
		m_ClosedAngle = 0;
		m_DoorMoveTimer.Reset();
		m_DoorMoveTime = 0;
		m_ResumeAfterStop = false;
		m_ChangedDirectionAfterStop = false;
		m_DoorMoveStopTime = 0;
		m_ResetToDefaultStateTimer.Reset();
		m_ResetToDefaultStateDelay = 5000;
		m_DrawMaterialLayerWhenOpen = true;
		m_DrawMaterialLayerWhenClosed = true;
		m_DoorMaterialID = g_MaterialDoor;
		m_DoorMaterialDrawn = false;
		m_DoorMaterialTempErased = false;
		m_LastDoorMaterialPos.Reset();
		m_DoorMoveStartSound = nullptr;
		m_DoorMoveSound = nullptr;
		m_DoorDirectionChangeSound = nullptr;
		m_DoorMoveEndSound = nullptr;

		// NOTE: This special override of a parent class member variable avoids needing an extra variable to avoid overwriting INI values.
		m_CanBeSquished = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ADoor::Create(const ADoor &reference) {
		if (reference.m_Door) {
			m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_Door->GetUniqueID());
			SetDoor(dynamic_cast<Attachable *>(reference.m_Door->Clone()));
		}

		Actor::Create(reference);

		m_InitialSpriteAnimDuration = reference.m_SpriteAnimDuration;

		for (const ADSensor &sensor : reference.m_Sensors) {
			m_Sensors.push_back(sensor);
		}
		m_SensorInterval = reference.m_SensorInterval;

		// Set the initial door state to the opposite of default so it'll move to default when spawned and draw the door material layer.
		m_DoorState = reference.m_ClosedByDefault ? OPEN : CLOSED;

		m_ClosedByDefault = reference.m_ClosedByDefault;
		m_OpenOffset = reference.m_OpenOffset;
		m_ClosedOffset = reference.m_ClosedOffset;
		m_OpenAngle = reference.m_OpenAngle;
		m_ClosedAngle = reference.m_ClosedAngle;
		m_DoorMoveTime = reference.m_DoorMoveTime;
		m_ResetToDefaultStateDelay = reference.m_ResetToDefaultStateDelay;
		m_DrawMaterialLayerWhenOpen = reference.m_DrawMaterialLayerWhenOpen;
		m_DrawMaterialLayerWhenClosed = reference.m_DrawMaterialLayerWhenClosed;
		m_DoorMaterialID = reference.m_DoorMaterialID;
		m_DoorMaterialTempErased = reference.m_DoorMaterialTempErased;
		if (reference.m_DoorMoveStartSound) { m_DoorMoveStartSound = dynamic_cast<SoundContainer*>(reference.m_DoorMoveStartSound->Clone()); }
		if (reference.m_DoorMoveSound) { m_DoorMoveSound = dynamic_cast<SoundContainer*>(reference.m_DoorMoveSound->Clone()); }
		if (reference.m_DoorDirectionChangeSound) { m_DoorDirectionChangeSound = dynamic_cast<SoundContainer*>(reference.m_DoorDirectionChangeSound->Clone()); }
		if (reference.m_DoorMoveEndSound) { m_DoorMoveEndSound = dynamic_cast<SoundContainer*>(reference.m_DoorMoveEndSound->Clone()); }

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ADoor::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "Door") {
			SetDoor(dynamic_cast<Attachable *>(g_PresetMan.ReadReflectedPreset(reader)));
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
			if (m_OpenAngle < 0) { reader.ReportError("Door OpenAngle cannot be less than 0."); }
		} else if (propName == "ClosedAngle") {
			Matrix rotation;
			reader >> rotation;
			m_ClosedAngle = rotation.GetRadAngle();
			if (m_ClosedAngle < 0) { reader.ReportError("Door ClosedAngle cannot be less than 0."); }
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
			reader >> m_ResetToDefaultStateDelay;
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
			m_DoorMoveStartSound = new SoundContainer;
			reader >> m_DoorMoveStartSound;
		} else if (propName == "DoorMoveSound") {
			m_DoorMoveSound = new SoundContainer;
			reader >> m_DoorMoveSound;
		} else if (propName == "DoorDirectionChangeSound") {
			m_DoorDirectionChangeSound = new SoundContainer;
			reader >> m_DoorDirectionChangeSound;
		} else if (propName == "DoorMoveEndSound") {
			m_DoorMoveEndSound = new SoundContainer;
			reader >> m_DoorMoveEndSound;
		} else {
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
		writer << m_ResetToDefaultStateDelay;
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
		if (m_DoorMoveStartSound) { m_DoorMoveStartSound->Stop(); }
		if (m_DoorMoveSound) { m_DoorMoveSound->Stop(); }
		if (m_DoorDirectionChangeSound) { m_DoorDirectionChangeSound->Stop(); }
		if (m_DoorMoveEndSound) { m_DoorMoveEndSound->Stop(); }
		if (!notInherited) { Actor::Destroy(); }

		delete m_DoorMoveStartSound;
		delete m_DoorMoveSound;
		delete m_DoorDirectionChangeSound;
		delete m_DoorMoveEndSound;

		for (ADSensor &sensor : m_Sensors) {
			sensor.Destroy();
		}
		if (m_DoorMaterialDrawn) { EraseDoorMaterial(); }
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::SetDoor(Attachable *newDoor) {
		if (m_Door && m_Door->IsAttached()) { RemoveAndDeleteAttachable(m_Door); }
		if (newDoor == nullptr) {
			m_Door = nullptr;
		} else {
			m_Door = newDoor;
			AddAttachable(newDoor);

			m_HardcodedAttachableUniqueIDsAndSetters.insert({newDoor->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
				dynamic_cast<ADoor *>(parent)->SetDoor(attachable);
			}});

			m_Door->SetInheritsRotAngle(false);
			m_DoorMaterialID = m_Door->GetMaterial()->GetIndex();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::DrawDoorMaterial() {
		if (!m_Door || m_DoorMaterialTempErased || !g_SceneMan.GetTerrain() || !g_SceneMan.GetTerrain()->GetMaterialBitmap()) {
			return;
		}
		if (m_DoorMaterialDrawn) { EraseDoorMaterial(false); }

		m_Door->Draw(g_SceneMan.GetTerrain()->GetMaterialBitmap(), Vector(), g_DrawDoor, true);
		m_LastDoorMaterialPos = m_Door->GetPos();
		m_DoorMaterialDrawn = true;

		g_SceneMan.GetTerrain()->AddUpdatedMaterialArea(m_Door->GetBoundingBox());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ADoor::EraseDoorMaterial(bool updateMaterialArea, bool keepMaterialDrawnFlag) {
		if (!keepMaterialDrawnFlag) { m_DoorMaterialDrawn = false; }

		if (!g_SceneMan.GetTerrain() || !g_SceneMan.GetTerrain()->GetMaterialBitmap()) {
			return false;
		}

		int fillX = m_LastDoorMaterialPos.GetFloorIntX();
		int fillY = m_LastDoorMaterialPos.GetFloorIntY();

		if (g_SceneMan.GetTerrMatter(fillX, fillY) != g_MaterialAir) {
			floodfill(g_SceneMan.GetTerrain()->GetMaterialBitmap(), fillX, fillY, g_MaterialAir);
			if (m_Door && updateMaterialArea) { g_SceneMan.GetTerrain()->AddUpdatedMaterialArea(m_Door->GetBoundingBox()); }
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
			if (m_DoorMaterialDrawn && m_DoorMaterialTempErased != enable) { EraseDoorMaterial(true, true); }
		} else {
			// Draw the door back if we were indeed temporarily suppressing it before
			if (m_DoorMaterialDrawn && m_DoorMaterialTempErased != enable) {
				m_Door->Draw(g_SceneMan.GetTerrain()->GetMaterialBitmap(), Vector(), g_DrawDoor, true);
				g_SceneMan.GetTerrain()->AddUpdatedMaterialArea(m_Door->GetBoundingBox());
			}
		}
		m_DoorMaterialTempErased = enable;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::GibThis(const Vector &impactImpulse, MovableObject *movableObjectToIgnore) {
		if (m_Door && m_Door->IsAttached()) {
			EraseDoorMaterial();
			m_Door->DeepCheck(true);
			m_Door->SetPinStrength(0);
		}
		Actor::GibThis(impactImpulse, movableObjectToIgnore);
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
		m_ResetToDefaultStateTimer.Reset();
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
		m_ResetToDefaultStateTimer.Reset();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::StopDoor() {
		if (m_DoorState == OPENING || m_DoorState == CLOSING) {
			if (m_DrawMaterialLayerWhenOpen || m_DrawMaterialLayerWhenClosed) { DrawDoorMaterial(); }
			m_DoorMoveStopTime = m_DoorMoveTime - m_DoorMoveTimer.GetElapsedSimTimeMS();
			m_DoorStateOnStop = m_DoorState;
			if (m_DoorMoveSound) { m_DoorMoveSound->Stop(); }
			if (m_DoorMoveEndSound) { m_DoorMoveEndSound->Play(m_Pos); }
			m_DoorState = STOPPED;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::SharedDoorControls() {
		if (m_DoorState == OPEN || m_DoorState == CLOSED) {
			if (m_DoorMoveStartSound) { m_DoorMoveStartSound->Play(m_Pos); }
			m_DoorMoveTimer.Reset();
			if (m_DoorMaterialDrawn) { EraseDoorMaterial(); }
			if (m_Door) { m_Door->DeepCheck(true); }
		} else if (m_DoorState == OPENING || m_DoorState == CLOSING) {
			if (m_DoorMoveSound) { m_DoorMoveSound->Stop(); }
			if (!m_ResumeAfterStop) {
				if (m_DoorDirectionChangeSound) { m_DoorDirectionChangeSound->Play(m_Pos); }
				m_DoorMoveTimer.SetElapsedSimTimeMS(m_DoorMoveTime - m_DoorMoveTimer.GetElapsedSimTimeMS());
			} else {
				if (m_DoorMoveStartSound) { m_DoorMoveStartSound->Play(m_Pos); }
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

		if (m_Door && m_Door->IsAttached()) {
			if (m_DoorState != STOPPED && m_SensorTimer.IsPastSimMS(m_SensorInterval)) { UpdateSensors(); }
			UpdateDoorAttachableActions();
		}

		Actor::Update();

		if (!m_Door) {
			EraseDoorMaterial();
			// Start the spinning out of control animation for the motor, start it slow
			m_SpriteAnimDuration *= 4;
		}

		if (m_SpriteAnimMode == LOOPWHENOPENCLOSE && m_FrameCount > 1 && (m_DoorState == OPENING || m_DoorState == CLOSING) && m_SpriteAnimTimer.IsPastSimMS(m_SpriteAnimDuration)) {
			m_Frame = (m_Frame + 1) % m_FrameCount;
			m_SpriteAnimTimer.Reset();
		}

		// Lose health when door is lost, spinning out of control until grinds to halt
		if (!m_Door && m_Status != DYING && m_Status != DEAD) {
			m_SpriteAnimMode = ALWAYSLOOP;
			m_SpriteAnimDuration = LERP(0, m_MaxHealth, 10, m_InitialSpriteAnimDuration, m_Health);

			if (m_DoorMoveSound) {
				if (!m_DoorMoveSound->IsBeingPlayed()) { m_DoorMoveSound->Play(m_Pos); }
				m_DoorMoveSound->SetPitch(LERP(10, m_InitialSpriteAnimDuration, 2, 1, m_SpriteAnimDuration));
			}

			m_Health -= 0.4F;
		}
		if (m_Status == DEAD) { GibThis(); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::UpdateSensors() {		
		const Actor *foundActor = 0;
		bool anySensorInput = false;

		for (ADSensor &sensor : m_Sensors) {
			foundActor = sensor.SenseActor(m_Pos, m_Rotation, m_HFlipped, m_MOID);
			if (foundActor && foundActor->IsControllable()) {
				anySensorInput = true;
				// If a sensor has found an enemy Actor, close the door and break so we don't accidentally open it for a friendly Actor.
				if (foundActor->GetTeam() != m_Team) {
					CloseDoor();
					break;
				} else if (foundActor->GetTeam() == m_Team) {					
					OpenDoor();
				}
			}
		}
		if (!anySensorInput && m_ResetToDefaultStateTimer.IsPastSimMS(m_ResetToDefaultStateDelay)) {
			if (m_ClosedByDefault && m_DoorState == OPEN) {
				CloseDoor();
			} else if (!m_ClosedByDefault && m_DoorState == CLOSED) {
				OpenDoor();
			}
		}
		m_SensorTimer.Reset();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::UpdateDoorAttachableActions() {
		Vector startOffset;
		Vector endOffset;
		float startAngle;
		float endAngle;

		if (m_DoorState == OPEN || m_DoorState == OPENING) {
			startOffset = m_ClosedOffset;
			endOffset = m_OpenOffset;
			startAngle = m_ClosedAngle;
			endAngle = m_OpenAngle;
		} else if (m_DoorState == CLOSED || m_DoorState == CLOSING) {
			startOffset = m_OpenOffset;
			endOffset = m_ClosedOffset;
			startAngle = m_OpenAngle;
			endAngle = m_ClosedAngle;
		}

		if (m_DoorState == OPEN || m_DoorState == CLOSED) {
			m_Door->SetParentOffset(endOffset);
			m_Door->SetRotAngle(m_Rotation.GetRadAngle() + (endAngle * static_cast<float>(GetFlipFactor())));
		} else if (m_DoorState == OPENING || m_DoorState == CLOSING) {
			if (m_DoorMoveSound && !m_DoorMoveSound->IsBeingPlayed()) { m_DoorMoveSound->Play(m_Pos); }

			if (m_DoorMoveTimer.IsPastSimMS(m_DoorMoveTime)) {
				m_ResetToDefaultStateTimer.Reset();

				m_Door->SetParentOffset(endOffset);
				m_Door->SetRotAngle(m_Rotation.GetRadAngle() + (endAngle * static_cast<float>(GetFlipFactor())));

				if (m_DoorMoveSound) { m_DoorMoveSound->Stop(); }
				if (m_DoorMoveEndSound) { m_DoorMoveEndSound->Play(m_Pos); }

				if (m_DoorState == OPENING) {
					if (m_DrawMaterialLayerWhenOpen) { DrawDoorMaterial(); }
					m_DoorState = OPEN;
				} else if (m_DoorState == CLOSING) {
					if (m_DrawMaterialLayerWhenClosed) { DrawDoorMaterial(); }
					m_DoorState = CLOSED;
				}
			} else {
				Vector updatedOffset(LERP(0, m_DoorMoveTime, startOffset.m_X, endOffset.m_X, m_DoorMoveTimer.GetElapsedSimTimeMS()), LERP(0, m_DoorMoveTime, startOffset.m_Y, endOffset.m_Y, m_DoorMoveTimer.GetElapsedSimTimeMS()));
				// TODO: Make this work across rotation 0. Probably the best solution would be to setup an angle LERP that properly handles the 2PI border and +- angles.
				float updatedAngle = LERP(0, m_DoorMoveTime, startAngle, endAngle, m_DoorMoveTimer.GetElapsedSimTimeMS());

				m_Door->SetParentOffset(updatedOffset);
				m_Door->SetRotAngle(m_Rotation.GetRadAngle() + (updatedAngle * GetFlipFactor()));

				// Clear away any terrain debris when the door is moving but only after a short delay so it doesn't take a chunk out of the ground
				if (m_DoorMoveTimer.IsPastSimMS(50)) { m_Door->DeepCheck(true); }
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::DrawHUD(BITMAP *targetBitmap, const Vector &targetPos, int whichScreen, bool playerControlled) {
		if (!m_HUDVisible) {
			return;
		}
		// Only draw if the team viewing this is on the same team OR has seen the space where this is located
		int viewingTeam = g_ActivityMan.GetActivity()->GetTeamOfPlayer(g_ActivityMan.GetActivity()->PlayerOfScreen(whichScreen));
		if (viewingTeam != m_Team && viewingTeam != Activity::NoTeam && g_SceneMan.IsUnseen(m_Pos.m_X, m_Pos.m_Y, viewingTeam)) {
			return;
		}
	}
}