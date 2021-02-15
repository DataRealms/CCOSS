#include "ADSensor.h"
#include "Actor.h"

namespace RTE {

	const std::string ADSensor::c_ClassName = "Sensor";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ADSensor::Clear() {
		m_StartOffset.Reset();
		m_SensorRay.Reset();
		m_Skip = 3;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ADSensor::Create(const ADSensor &reference) {
		m_StartOffset = reference.m_StartOffset;
		m_SensorRay = reference.m_SensorRay;
		m_Skip = reference.m_Skip;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ADSensor::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "StartOffset") {
			reader >> m_StartOffset;
		} else if (propName == "SensorRay") {
			reader >> m_SensorRay;
		} else if (propName == "SkipPixels") {
			reader >> m_Skip;
		} else {
			return Serializable::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ADSensor::Save(Writer &writer) const {
		Serializable::Save(writer);

		writer.NewProperty("StartOffset");
		writer << m_StartOffset;
		writer.NewProperty("SensorRay");
		writer << m_SensorRay;
		writer.NewProperty("SkipPixels");
		writer << m_Skip;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Actor * ADSensor::SenseActor(const Vector &doorPos, const Matrix &doorRot, bool doorHFlipped, MOID ignoreMOID) {
		Actor *sensedActor = 0;
		MOID foundMOID = g_SceneMan.CastMORay(doorPos + m_StartOffset.GetXFlipped(doorHFlipped) * doorRot, m_SensorRay.GetXFlipped(doorHFlipped) * doorRot, ignoreMOID, Activity::NoTeam, 0, true, m_Skip);

		if (foundMOID) {
			sensedActor = dynamic_cast<Actor *>(g_MovableMan.GetMOFromID(g_MovableMan.GetRootMOID(foundMOID)));

			// Reverse the ray direction if the sensed actor was not valid, to see if we hit anything else relevant.
			if (!sensedActor || !sensedActor->IsControllable()) {
				foundMOID = g_SceneMan.CastMORay(doorPos + (m_StartOffset.GetXFlipped(doorHFlipped) + m_SensorRay.GetXFlipped(doorHFlipped)) * doorRot, (-m_SensorRay.GetXFlipped(doorHFlipped)) * doorRot, ignoreMOID, Activity::NoTeam, 0, true, m_Skip);

				if (foundMOID) { sensedActor = dynamic_cast<Actor *>(g_MovableMan.GetMOFromID(g_MovableMan.GetRootMOID(foundMOID))); }
			}
		}
		return sensedActor;
	}
}