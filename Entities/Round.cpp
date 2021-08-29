#include "Round.h"
#include "PresetMan.h"
#include "MOPixel.h"

namespace RTE {

	ConcreteClassInfo(Round, Entity, 500);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Round::Clear() {
		m_Particle = 0;
		m_ParticleCount = 0;
		m_FireVel = 0;
		m_Separation = 0;
		m_LifeVariation = 0;
		m_Shell = 0;
		m_ShellVel = 0;
		m_FireSound.Reset();
		m_AILifeTime = 0;
		m_AIFireVel = -1;
		m_AIPenetration = -1;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Round::Create() {
		if (Entity::Create() < 0) {
			return -1;
		}

		if (m_AILifeTime == 0) {
			const MovableObject *bullet = GetNextParticle();
			if (bullet) { m_AILifeTime = bullet->GetLifetime(); }
		}
		if (m_AIFireVel < 0) { m_AIFireVel = m_FireVel; }

		if (m_AIPenetration < 0) {
			const MovableObject *bullet = GetNextParticle();
			m_AIPenetration = (bullet && dynamic_cast<const MOPixel *>(bullet)) ? bullet->GetMass() * bullet->GetSharpness() * m_AIFireVel : 0;
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Round::Create(const Round &reference) {
		Entity::Create(reference);

		m_Particle = reference.m_Particle;
		m_ParticleCount = reference.m_ParticleCount;
		m_FireVel = reference.m_FireVel;
		m_Separation = reference.m_Separation;
		m_LifeVariation = reference.m_LifeVariation;
		m_Shell = reference.m_Shell;
		m_ShellVel = reference.m_ShellVel;
		m_FireSound = reference.m_FireSound;
		m_AILifeTime = reference.m_AILifeTime;
		m_AIFireVel = reference.m_AIFireVel;
		m_AIPenetration = reference.m_AIPenetration;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Round::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "Particle") {
			m_Particle = dynamic_cast<const MovableObject *>(g_PresetMan.GetEntityPreset(reader));
			RTEAssert(m_Particle, "Stream suggests allocating an unallocable type in Round::Create!");
		} else if (propName == "ParticleCount") {
			reader >> m_ParticleCount;
		} else if (propName == "FireVelocity") {
			reader >> m_FireVel;
		} else if (propName == "Separation") {
			reader >> m_Separation;
		} else if (propName == "LifeVariation") {
			reader >> m_LifeVariation;
		} else if (propName == "Shell") {
			m_Shell = dynamic_cast<const MovableObject *>(g_PresetMan.GetEntityPreset(reader));
		} else if (propName == "ShellVelocity") {
			reader >> m_ShellVel;
		} else if (propName == "FireSound") {
			reader >> m_FireSound;
		} else if (propName == "AILifeTime") {
			reader >> m_AILifeTime;
		} else if (propName == "AIFireVel") {
			reader >> m_AIFireVel;
		} else if (propName == "AIPenetration") {
			reader >> m_AIPenetration;
		} else {
			return Entity::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Round::Save(Writer &writer) const {
		Entity::Save(writer);

		writer.NewProperty("Particle");
		writer << m_Particle;
		writer.NewProperty("ParticleCount");
		writer << m_ParticleCount;
		writer.NewProperty("FireVelocity");
		writer << m_FireVel;
		writer.NewProperty("Separation");
		writer << m_Separation;
		writer.NewProperty("LifeVariation");
		writer << m_LifeVariation;
		writer.NewProperty("Shell");
		writer << m_Shell;
		writer.NewProperty("ShellVelocity");
		writer << m_ShellVel;
		writer.NewProperty("FireSound");
		writer << m_FireSound;
		writer.NewProperty("AILifeTime");
		writer << m_AILifeTime;
		writer.NewProperty("AIFireVel");
		writer << m_AIFireVel;
		writer.NewProperty("AIPenetration");
		writer << m_AIPenetration;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	MovableObject * Round::PopNextParticle() {
		MovableObject *tempParticle = (m_ParticleCount > 0) ? dynamic_cast<MovableObject *>(m_Particle->Clone()) : 0;
		m_ParticleCount--;
		return tempParticle;
	}
}