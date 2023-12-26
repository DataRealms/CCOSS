#include "ADoor.h"
#include "AtomGroup.h"
#include "Attachable.h"
#include "Matrix.h"
#include "SLTerrain.h"
#include "PresetMan.h"
#include "SettingsMan.h"

#include "tracy/Tracy.hpp"

namespace RTE {

	ConcreteClassInfo(ADoor, Actor, 20);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::Clear() {
		m_InitialSpriteAnimDuration = 0;
		m_Sensors.clear();
		m_SensorTimer.Reset();
		m_SensorInterval = 1000;
		m_Door = nullptr;
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
		m_DoorMaterialRedrawTimer.Reset();
		m_DoorMaterialRedrawTimer.SetSimTimeLimitMS(10000);
		// We need the door material layer to redraw right away when it's created, so set the door material redraw timer to be past its limit, thereby forcing an immediate redraw.
		m_DoorMaterialRedrawTimer.SetElapsedSimTimeMS(m_DoorMaterialRedrawTimer.GetSimTimeLimitMS() + 1);
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
		if (reference.m_Door) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_Door->GetUniqueID()); }

		Actor::Create(reference);

		if (reference.m_Door) { SetDoor(dynamic_cast<Attachable *>(reference.m_Door->Clone())); }

		m_InitialSpriteAnimDuration = reference.m_SpriteAnimDuration;

		for (const ADSensor &sensor : reference.m_Sensors) {
			m_Sensors.push_back(sensor);
		}
		m_SensorInterval = reference.m_SensorInterval;

		m_DoorState = reference.m_DoorState;
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

		if (reference.m_DoorMoveStartSound) { m_DoorMoveStartSound.reset(dynamic_cast<SoundContainer*>(reference.m_DoorMoveStartSound->Clone())); }
		if (reference.m_DoorMoveSound) { m_DoorMoveSound.reset(dynamic_cast<SoundContainer*>(reference.m_DoorMoveSound->Clone())); }
		if (reference.m_DoorDirectionChangeSound) { m_DoorDirectionChangeSound.reset(dynamic_cast<SoundContainer*>(reference.m_DoorDirectionChangeSound->Clone())); }
		if (reference.m_DoorMoveEndSound) { m_DoorMoveEndSound.reset(dynamic_cast<SoundContainer*>(reference.m_DoorMoveEndSound->Clone())); }

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ADoor::ReadProperty(const std::string_view &propName, Reader &reader) {
		StartPropertyList(return Actor::ReadProperty(propName, reader));
		
		MatchProperty("Door", { SetDoor(dynamic_cast<Attachable *>(g_PresetMan.ReadReflectedPreset(reader))); });
		MatchProperty("OpenOffset", { reader >> m_OpenOffset; });
		MatchProperty("ClosedOffset", { reader >> m_ClosedOffset; });
		MatchProperty("OpenClosedOffset", {
			reader >> m_OpenOffset;
			m_ClosedOffset = m_OpenOffset;
		});
		MatchProperty("OpenAngle", {
			Matrix rotation;
			reader >> rotation;
			m_OpenAngle = rotation.GetRadAngle();
			if (m_OpenAngle < 0) { reader.ReportError("Door OpenAngle cannot be less than 0."); }
		});
		MatchProperty("ClosedAngle", {
			Matrix rotation;
			reader >> rotation;
			m_ClosedAngle = rotation.GetRadAngle();
			if (m_ClosedAngle < 0) { reader.ReportError("Door ClosedAngle cannot be less than 0."); }
		});
		MatchProperty("OpenClosedAngle", {
			Matrix rotation;
			reader >> rotation;
			m_OpenAngle = rotation.GetRadAngle();
			m_ClosedAngle = rotation.GetRadAngle();
		});
		MatchProperty("DoorMoveTime", { reader >> m_DoorMoveTime; });
		MatchProperty("ClosedByDefault", { reader >> m_ClosedByDefault; });
		MatchProperty("ResetDefaultDelay", { reader >> m_ResetToDefaultStateDelay; });
		MatchProperty("SensorInterval", { reader >> m_SensorInterval; });
		MatchProperty("AddSensor", {
			ADSensor sensor;
			reader >> sensor;
			m_Sensors.push_back(sensor);
		});
		MatchProperty("DrawMaterialLayerWhenOpen", { reader >> m_DrawMaterialLayerWhenOpen; });
		MatchProperty("DrawMaterialLayerWhenClosed", { reader >> m_DrawMaterialLayerWhenClosed; });
		MatchProperty("DoorMoveStartSound", { m_DoorMoveStartSound.reset(dynamic_cast<SoundContainer *>(g_PresetMan.ReadReflectedPreset(reader))); });
		MatchProperty("DoorMoveSound", { m_DoorMoveSound.reset(dynamic_cast<SoundContainer *>(g_PresetMan.ReadReflectedPreset(reader))); });
		MatchProperty("DoorDirectionChangeSound", { m_DoorDirectionChangeSound.reset(dynamic_cast<SoundContainer *>(g_PresetMan.ReadReflectedPreset(reader))); });
		MatchProperty("DoorMoveEndSound", { m_DoorMoveEndSound.reset(dynamic_cast<SoundContainer *>(g_PresetMan.ReadReflectedPreset(reader))); });
		
		
		EndPropertyList;
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
		writer << m_DoorMoveStartSound.get();
		writer.NewProperty("DoorMoveSound");
		writer << m_DoorMoveSound.get();
		writer.NewProperty("DoorDirectionChangeSound");
		writer << m_DoorDirectionChangeSound.get();
		writer.NewProperty("DoorMoveEndSound");
		writer << m_DoorMoveEndSound.get();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::Destroy(bool notInherited) {
		if (m_DoorMoveStartSound) { m_DoorMoveStartSound->Stop(); }
		if (m_DoorMoveSound) { m_DoorMoveSound->Stop(); }
		if (m_DoorDirectionChangeSound) { m_DoorDirectionChangeSound->Stop(); }
		if (m_DoorMoveEndSound) { m_DoorMoveEndSound->Stop(); }
		if (!notInherited) { Actor::Destroy(); }

		for (ADSensor &sensor : m_Sensors) {
			sensor.Destroy();
		}
		if (m_DoorMaterialDrawn) {
			EraseDoorMaterial();
		}
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::SetDoor(Attachable *newDoor) {
		if (m_DoorMaterialDrawn) {
			RTEAssert(m_Door, "Door material drawn without an m_Door! This should've been cleared when the door was!");
			EraseDoorMaterial(); 
		}

		if (m_Door && m_Door->IsAttached()) { 
			RemoveAndDeleteAttachable(m_Door); 
		}

		m_Door = newDoor;
		if (m_Door) {
			AddAttachable(m_Door);

			m_HardcodedAttachableUniqueIDsAndSetters.insert({m_Door->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
				dynamic_cast<ADoor *>(parent)->SetDoor(attachable);
			}});

			m_Door->SetInheritsRotAngle(false);
			m_DoorMaterialID = m_Door->GetMaterial()->GetIndex();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::DrawDoorMaterial(bool disallowErasingMaterialBeforeDrawing, bool updateMaterialArea) {
		if (!m_Door || m_DoorMaterialTempErased || !g_SceneMan.GetTerrain() || !g_SceneMan.GetTerrain()->GetMaterialBitmap()) {
			return;
		}

		if (!disallowErasingMaterialBeforeDrawing && m_DoorMaterialDrawn) { 
			EraseDoorMaterial(updateMaterialArea); 
		}

		m_Door->Draw(g_SceneMan.GetTerrain()->GetMaterialBitmap(), Vector(), g_DrawDoor, true);
		m_LastDoorMaterialPos = m_Door->GetPos();
		m_DoorMaterialDrawn = true;

		if (updateMaterialArea) {
			g_SceneMan.GetTerrain()->AddUpdatedMaterialArea(m_Door->GetBoundingBox());
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ADoor::EraseDoorMaterial(bool updateMaterialArea) {
		if (!g_SceneMan.GetTerrain() || !g_SceneMan.GetTerrain()->GetMaterialBitmap()) {
			return false;
		}

		int fillX = m_LastDoorMaterialPos.GetFloorIntX();
		int fillY = m_LastDoorMaterialPos.GetFloorIntY();

		DrawDoorMaterial(true, updateMaterialArea);
		m_DoorMaterialDrawn = false;

		if (g_SceneMan.GetTerrMatter(fillX, fillY) != g_MaterialAir) {
			floodfill(g_SceneMan.GetTerrain()->GetMaterialBitmap(), fillX, fillY, g_MaterialAir);

			if (m_Door && updateMaterialArea) { 
				g_SceneMan.GetTerrain()->AddUpdatedMaterialArea(m_Door->GetBoundingBox()); 
			}

			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::TempEraseOrRedrawDoorMaterial(bool erase) {
		if (!g_SceneMan.GetTerrain() || !g_SceneMan.GetTerrain()->GetMaterialBitmap()) {
			return;
		}

		bool doorMaterialDrawnState = m_DoorMaterialDrawn;
		if (erase && m_DoorMaterialDrawn && !m_DoorMaterialTempErased) {
			m_DoorMaterialTempErased = erase;
			EraseDoorMaterial(false);
			m_DoorMaterialDrawn = doorMaterialDrawnState;
		} else if (!erase && m_DoorMaterialDrawn && m_DoorMaterialTempErased) {
			m_DoorMaterialTempErased = erase;
			DrawDoorMaterial(true, false);
			m_DoorMaterialDrawn = doorMaterialDrawnState;
		}
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

	void ADoor::CorrectAttachableAndWoundPositionsAndRotations() const {
		if (m_Door) {
			m_Door->SetParentOffset(m_ClosedByDefault ? m_ClosedOffset : m_OpenOffset);
			m_Door->SetRotAngle(m_Rotation.GetRadAngle() + (m_ClosedByDefault ? m_ClosedAngle : m_OpenAngle) * GetFlipFactor());
		}
		MOSRotating::CorrectAttachableAndWoundPositionsAndRotations();
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
			if (m_DrawMaterialLayerWhenOpen || m_DrawMaterialLayerWhenClosed) { DrawDoorMaterial(true); }
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
		ZoneScoped;

		if (m_Door) {
			if (m_DoorState != STOPPED && m_Status != Actor::Status::INACTIVE && m_SensorTimer.IsPastSimMS(m_SensorInterval)) { 
				UpdateSensors(); 
			}
			UpdateDoorAttachableActions();

			bool shouldDrawDoorMaterial = ((m_DrawMaterialLayerWhenOpen && m_DoorState == OPEN) ||
				(m_DrawMaterialLayerWhenClosed && m_DoorState == CLOSED) ||
				((m_DrawMaterialLayerWhenOpen || m_DrawMaterialLayerWhenClosed) && m_DoorState == STOPPED)) &&
				m_DoorMaterialRedrawTimer.IsPastSimTimeLimit();
			if (shouldDrawDoorMaterial) {
				DrawDoorMaterial(true);
				m_DoorMaterialRedrawTimer.Reset();
			}
		}

		Actor::Update();

		// Start the spinning out of control animation for the motor, start it slow
		if (!m_Door) { 
			m_SpriteAnimDuration *= 4; 
		}

		if (m_SpriteAnimMode == LOOPWHENOPENCLOSE && m_FrameCount > 1 && (m_DoorState == OPENING || m_DoorState == CLOSING) && m_SpriteAnimTimer.IsPastSimMS(m_SpriteAnimDuration)) {
			m_Frame = (m_Frame + 1) % m_FrameCount;
			m_SpriteAnimTimer.Reset();
		}

		// Lose health when door is lost, spinning out of control until grinds to halt
		if (!m_Door && m_Status != DYING && m_Status != DEAD) {
			m_SpriteAnimMode = ALWAYSLOOP;
			m_SpriteAnimDuration = static_cast<int>(Lerp(0, m_MaxHealth, 10.0F, static_cast<float>(m_InitialSpriteAnimDuration), m_Health));

			if (m_DoorMoveSound) {
				if (!m_DoorMoveSound->IsBeingPlayed()) { m_DoorMoveSound->Play(m_Pos); }
				m_DoorMoveSound->SetPitch(Lerp(10.0F, static_cast<float>(m_InitialSpriteAnimDuration), 2.0F, 1.0F, static_cast<float>(m_SpriteAnimDuration)));
			}

			m_Health -= 0.4F;
		}

		if (m_Status == DEAD) { 
			GibThis(); 
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::UpdateSensors() {		
		const Actor *foundActor = nullptr;
		bool anySensorInput = false;

		for (ADSensor &sensor : m_Sensors) {
			foundActor = sensor.SenseActor(m_Pos, m_Rotation, m_HFlipped, m_MOID);
			if (foundActor && foundActor->IsControllable()) {
				anySensorInput = true;

				if (m_Team == Activity::NoTeam) {
					OpenDoor();
					break;
				// If a sensor has found an enemy Actor, close the door and stop looking, so we don't accidentally open it for a friendly Actor.
				} else if (foundActor->GetTeam() != m_Team) {
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
			m_Door->SetRotAngle(m_Rotation.GetRadAngle() + (endAngle * GetFlipFactor()));
		} else if (m_DoorState == OPENING || m_DoorState == CLOSING) {
			if (m_DoorMoveSound && !m_DoorMoveSound->IsBeingPlayed()) { m_DoorMoveSound->Play(m_Pos); }

			if (m_DoorMoveTimer.IsPastSimMS(m_DoorMoveTime)) {
				m_ResetToDefaultStateTimer.Reset();

				m_Door->SetParentOffset(endOffset);
				m_Door->SetRotAngle(m_Rotation.GetRadAngle() + (endAngle * GetFlipFactor()));

				if (m_DoorMoveSound) { m_DoorMoveSound->Stop(); }
				if (m_DoorMoveEndSound) { m_DoorMoveEndSound->Play(m_Pos); }

				if (m_DoorState == OPENING) {
					if (m_DrawMaterialLayerWhenOpen) { 
						DrawDoorMaterial(); 
					}
					m_DoorState = OPEN;
				} else if (m_DoorState == CLOSING) {
					if (m_DrawMaterialLayerWhenClosed) { 
						DrawDoorMaterial(); 
					}
					m_DoorState = CLOSED;
				}
			} else {
				Vector updatedOffset(Lerp(0, m_DoorMoveTime, startOffset.m_X, endOffset.m_X, m_DoorMoveTimer.GetElapsedSimTimeMS()), Lerp(0, m_DoorMoveTime, startOffset.m_Y, endOffset.m_Y, m_DoorMoveTimer.GetElapsedSimTimeMS()));
				// TODO: Make this work across rotation 0. Probably the best solution would be to setup an angle Lerp that properly handles the 2PI border and +- angles.
				float updatedAngle = Lerp(0, m_DoorMoveTime, startAngle, endAngle, m_DoorMoveTimer.GetElapsedSimTimeMS());

				m_Door->SetParentOffset(updatedOffset);
				m_Door->SetRotAngle(m_Rotation.GetRadAngle() + (updatedAngle * GetFlipFactor()));

				// Clear away any terrain debris when the door is moving but only after a short delay so it doesn't take a chunk out of the ground.
				if (m_DoorMoveTimer.IsPastSimMS(50)) { 
					m_Door->DeepCheck(true); 
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADoor::DrawHUD(BITMAP *targetBitmap, const Vector &targetPos, int whichScreen, bool playerControlled) {
		m_HUDStack = -static_cast<int>(m_CharHeight) / 2;

		if (!m_HUDVisible) {
			return;
		}
		// Only draw if the team viewing this is on the same team OR has seen the space where this is located.
		int viewingTeam = g_ActivityMan.GetActivity()->GetTeamOfPlayer(g_ActivityMan.GetActivity()->PlayerOfScreen(whichScreen));
		if (viewingTeam != m_Team && viewingTeam != Activity::NoTeam && (!g_SettingsMan.ShowEnemyHUD() || g_SceneMan.IsUnseen(m_Pos.GetFloorIntX(), m_Pos.GetFloorIntY(), viewingTeam))) {
			return;
		}
	}
}