#include "Atom.h"
#include "SLTerrain.h"
#include "MovableObject.h"
#include "MOSRotating.h"
#include "PresetMan.h"
#include "Actor.h"

namespace RTE {

	const std::string Atom::c_ClassName = "Atom";
	std::vector<void *> Atom::s_AllocatedPool;
	int Atom::s_PoolAllocBlockCount = 200;
	int Atom::s_InstancesInUse = 0;

	// This forms a circle around the Atom's offset center, to check for mask color pixels in order to determine the normal at the Atom's position.
	const int Atom::s_NormalChecks[c_NormalCheckCount][2] = { {0, -3}, {1, -3}, {2, -2}, {3, -1}, {3, 0}, {3, 1}, {2, 2}, {1, 3}, {0, 3}, {-1, 3}, {-2, 2}, {-3, 1}, {-3, 0}, {-3, -1}, {-2, -2}, {-1, -3} };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Atom::Clear() {
		m_Offset.Reset();
		m_OriginalOffset.Reset();
		m_Normal.Reset();
		m_Material = g_SceneMan.GetMaterialFromID(g_MaterialAir);
		m_SubgroupID = 0;
		m_MOHitsDisabled = false;
		m_TerrainHitsDisabled = false;
		m_OwnerMO = 0;
		m_IgnoreMOID = g_NoMOID;
		m_IgnoreMOIDs.clear();
		m_MOIDHit = g_NoMOID;
		m_TerrainMatHit = g_MaterialAir;
		m_LastHit.Reset();
		/*
		m_HitVel.Reset();
		m_HitRadius.Reset();
		m_HitImpulse.Reset();
		*/
		m_TrailColor.Reset();
		m_TrailLength = 0;
		m_NumPenetrations = 0;
		m_ChangedDir = true;
		m_ResultWrapped = false;
		m_PrevError = 0;
		m_StepRatio = 1.0F;
		m_SegProgress = 0.0F;

		m_IgnoreMOIDsByGroup = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Atom::Create(const Vector &offset, Material const *material, MovableObject *owner, Color trailColor, int trailLength) {
		m_Offset = m_OriginalOffset = offset;
		// Use the offset as normal for now
		m_Normal = m_Offset;
		m_Normal.Normalize();
		m_Material = material;
		m_OwnerMO = owner;
		m_TrailColor = trailColor;
		m_TrailLength = trailLength;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Atom::Create(const Atom &reference) {
		m_Offset = reference.m_Offset;
		m_OriginalOffset = reference.m_OriginalOffset;
		m_Normal = reference.m_Normal;
		m_Material = reference.m_Material;
		m_SubgroupID = reference.m_SubgroupID;
		m_TrailColor = reference.m_TrailColor;
		m_TrailLength = reference.m_TrailLength;

		// These need to be set manually by the new owner.
		m_OwnerMO = 0;
		m_IgnoreMOIDsByGroup = 0;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Atom::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "Offset") {
			reader >> m_Offset;
		} else if (propName == "OriginalOffset") {
			reader >> m_OriginalOffset;
		} else if (propName == "Material") {
			Material mat;
			mat.Reset();
			reader >> mat;
			m_Material = g_SceneMan.AddMaterialCopy(&mat);
			if (!m_Material) { RTEAbort("Failed to store material \"" + mat.GetPresetName() + "\". Aborting!"); }
		} else if (propName == "TrailColor") {
			reader >> m_TrailColor;
		} else if (propName == "TrailLength") {
			reader >> m_TrailLength;
		} else {
			return Serializable::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Atom::Save(Writer &writer) const {
		Serializable::Save(writer);

		writer.NewPropertyWithValue("Offset", m_Offset);
		writer.NewPropertyWithValue("OriginalOffset", m_OriginalOffset);
		writer.NewPropertyWithValue("Material", m_Material);
		writer.NewPropertyWithValue("TrailColor", m_TrailColor);
		writer.NewPropertyWithValue("TrailLength", m_TrailLength);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void * Atom::GetPoolMemory() {
		// If the pool is empty, then fill it up again with as many instances as we are set to
		if (s_AllocatedPool.empty()) { FillPool((s_PoolAllocBlockCount > 0) ? s_PoolAllocBlockCount : 10); }

		// Get the instance in the top of the pool and pop it off
		void *foundMemory = s_AllocatedPool.back();
		s_AllocatedPool.pop_back();

		RTEAssert(foundMemory, "Could not find an available instance in the pool, even after increasing its size!");

		// Keep track of the number of instances passed out
		s_InstancesInUse++;

		return foundMemory;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Atom::FillPool(int fillAmount) {
		// Default to the set block allocation size if fillAmount is 0
		if (fillAmount <= 0) { fillAmount = s_PoolAllocBlockCount; }

		// If concrete class, fill up the pool with pre-allocated memory blocks the size of the type
		if (fillAmount > 0) {
			// As many as we're asked to make
			for (int i = 0; i < fillAmount; ++i) {
				s_AllocatedPool.push_back(malloc(sizeof(Atom)));
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Atom::ReturnPoolMemory(void *returnedMemory) {
		if (!returnedMemory) {
			return false;
		}
		s_AllocatedPool.push_back(returnedMemory);

		// Keep track of the number of instances passed in
		s_InstancesInUse--;

		return s_InstancesInUse;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Atom::CalculateNormal(BITMAP *sprite, Vector spriteCenter) {
		RTEAssert(sprite, "Trying to set up Atom normal without passing in bitmap");

		// Can't set up a normal on an atom that doesn't have an offset from its parent's center
		if (m_Offset.IsZero()) {
			m_Normal.Reset();
			return false;
		}
		// See if the atom even ends up in the sprite at all
		Vector atomPos = spriteCenter + m_Offset;
		if (atomPos.m_X < 0 || atomPos.m_Y < 0 || atomPos.m_X >= sprite->w || atomPos.m_Y >= sprite->h) {
			return false;
		}
		// Go through all the check positions from the atom's position on the sprite
		m_Normal.Reset();
		int checkPixel = 0;
		for (int check = 0; check < c_NormalCheckCount; ++check) {
			// Establish the current integer position to check for nothingness on the sprite
			checkPixel = getpixel(sprite, atomPos.m_X + s_NormalChecks[check][X], atomPos.m_Y + s_NormalChecks[check][Y]);

			// If the pixel was outside of the bitmap, or on the key color, then that's a valid direction for normal, add it to the accumulated normal
			if (checkPixel < 0 || checkPixel == g_MaskColor) {
				m_Normal.m_X += s_NormalChecks[check][X];
				m_Normal.m_Y += s_NormalChecks[check][Y];
			}
		}
		m_Normal.Normalize();
		/*
		// Check whether the normal vector makes sense at all. It can't point against the offset, for example
		if (m_Normal.Dot(m_Offset) < 0)
		{
			// Abort and revert to offset-based normal
			m_Normal = m_Offset;
			m_Normal.Normalize();
			return false;
		}
		*/
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Atom::IsIgnoringMOID(MOID whichMOID) {
		if (whichMOID == m_IgnoreMOID) {
			return true;
		}
		const MovableObject *hitMO = g_MovableMan.GetMOFromID(whichMOID);
		hitMO = hitMO ? hitMO->GetRootParent() : 0;

		// First check if we are ignoring the team of the MO we hit, or if it's an AtomGroup and we're ignoring all those
		if (m_OwnerMO && hitMO) {
			if (m_OwnerMO->IgnoresTeamHits() && hitMO->IgnoresTeamHits() && m_OwnerMO->GetTeam() == hitMO->GetTeam()) {
				return true;
			}
			if ((m_OwnerMO->IgnoresAtomGroupHits() && dynamic_cast<const MOSRotating *>(hitMO)) || (hitMO->IgnoresAtomGroupHits() && dynamic_cast<const MOSRotating *>(m_OwnerMO))) {
				return true;
			}
		}
		// Now check for explicit ignore
		bool ignored = false;
		for (const MOID &moid : m_IgnoreMOIDs) {
			if (moid == whichMOID) {
				ignored = true;
				break;
			}
		}
		// Check in AtomGroup-owned list if it's assigned to this atom
		if (!ignored && m_IgnoreMOIDsByGroup) {
			for (const MOID &moid : *m_IgnoreMOIDsByGroup) {
				if (moid == whichMOID) {
					ignored = true;
					break;
				}
			}
		}
		return ignored;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Atom::MOHitResponse() {
		RTEAssert(m_OwnerMO, "Stepping an Atom without a parent MO!");

		if (m_OwnerMO->m_HitsMOs && m_MOIDHit != g_NoMOID /*&& IsIgnoringMOID(m_MOIDHit)*/) {
			m_LastHit.HitPoint.Reset();
			m_LastHit.BitmapNormal.Reset();
			bool hit[2];
			hit[X] = hit[Y] = false;
			bool validHit = true;

			// Check for the collision point in the dominant direction of travel.
			if (m_Delta[m_Dom] && ((m_Dom == X && g_SceneMan.GetMOIDPixel(m_HitPos[X], m_IntPos[Y]) != g_NoMOID) || (m_Dom == Y && g_SceneMan.GetMOIDPixel(m_IntPos[X], m_HitPos[Y]) != g_NoMOID))) {
				hit[m_Dom] = true;
				m_LastHit.HitPoint = (m_Dom == X) ? Vector(m_HitPos[X], m_IntPos[Y]) : Vector(m_IntPos[X], m_HitPos[Y]);
				m_LastHit.BitmapNormal[m_Dom] = -m_Increment[m_Dom];
			}

			// Check for the collision point in the submissive direction of travel.
			if (m_SubStepped && m_Delta[m_Sub] && ((m_Sub == X && g_SceneMan.GetMOIDPixel(m_HitPos[X], m_IntPos[Y]) != g_NoMOID) || (m_Sub == Y && g_SceneMan.GetMOIDPixel(m_IntPos[X], m_HitPos[Y]) != g_NoMOID))) {
				hit[m_Sub] = true;
				if (m_LastHit.HitPoint.IsZero()) {
					m_LastHit.HitPoint = (m_Sub == X) ? Vector(m_HitPos[X], m_IntPos[Y]) : Vector(m_IntPos[X], m_HitPos[Y]);
				} else {
					// We hit pixels in both sub and dom directions on the other MO, a corner hit.
					m_LastHit.HitPoint.SetXY(m_HitPos[X], m_HitPos[Y]);
				}
				m_LastHit.BitmapNormal[m_Sub] = -m_Increment[m_Sub];
			}

			// If neither the direct dominant or sub directions yielded a collision point, then that means we hit right on the corner edge of a pixel, and that is the collision point.
			if (!hit[m_Dom] && !hit[m_Sub]) {
				hit[m_Dom] = hit[m_Sub] = true;
				m_LastHit.HitPoint.SetXY(m_HitPos[X], m_HitPos[Y]);
				m_LastHit.BitmapNormal.SetXY(-m_Increment[X], -m_Increment[Y]);
			}
			m_LastHit.BitmapNormal.Normalize();

			if (!m_Normal.IsZero()) { m_LastHit.BitmapNormal = -m_OwnerMO->RotateOffset(m_Normal); }

			// Cancel collision response for this if it appears the collision is happening in the 'wrong' direction, meaning away from the center.
			// This happens when things are sunk into each other, and thus getting 'hooked' on each other
			if (m_LastHit.HitRadius[HITOR].Dot(m_LastHit.BitmapNormal) >= 0) {
				// Hitee hit radius and the normal presented to the hitor are facing each other! We are colliding in the wrong direction!
				validHit = false;
			}

			m_LastHit.Body[HITOR] = m_OwnerMO;
			m_LastHit.Body[HITEE] = g_MovableMan.GetMOFromID(m_MOIDHit);

#ifndef RELEASE_BUILD
			RTEAssert(m_LastHit.Body[HITEE], "Hitee MO is 0 in Atom::MOHitResponse!");
			RTEAssert(m_MOIDHit == m_LastHit.Body[HITEE]->GetID(), "g_MovableMan.GetMOFromID messed up in Atom::MOHitResponse!");
#endif

			// Get the roots for both bodies
			if (m_LastHit.Body[HITOR]) { m_LastHit.RootBody[HITOR] = m_LastHit.Body[HITOR]->GetRootParent(); }
			if (m_LastHit.Body[HITEE]) { m_LastHit.RootBody[HITEE] = m_LastHit.Body[HITEE]->GetRootParent(); }

			validHit = validHit && m_LastHit.Body[HITEE]->CollideAtPoint(m_LastHit);

			return validHit;
		}
		RTEAbort("Atom not supposed to do MO hit response if it didn't hit anything!");
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	HitData & Atom::TerrHitResponse() {
		RTEAssert(m_OwnerMO, "Stepping an Atom without a parent MO!");

		if (m_TerrainMatHit) {
			MID hitMaterialID = g_SceneMan.GetTerrMatter(m_HitPos[X], m_HitPos[Y]);
			MID domMaterialID = g_MaterialAir;
			MID subMaterialID = g_MaterialAir;
			m_LastHit.HitMaterial[HITOR] = m_Material;
			Material const *hitMaterial = m_LastHit.HitMaterial[HITEE] = g_SceneMan.GetMaterialFromID(hitMaterialID);
			Material const *domMaterial = g_SceneMan.GetMaterialFromID(g_MaterialAir);
			Material const *subMaterial = g_SceneMan.GetMaterialFromID(g_MaterialAir);
			bool hit[2];
			hit[X] = hit[Y] = false;
			m_LastHit.BitmapNormal.Reset();
			Vector hitAcc = m_LastHit.HitVel[HITOR];

			// Check for and react upon a collision in the dominant direction of travel.
			if (m_Delta[m_Dom] && ((m_Dom == X && g_SceneMan.GetTerrMatter(m_HitPos[X], m_IntPos[Y])) || (m_Dom == Y && g_SceneMan.GetTerrMatter(m_IntPos[X], m_HitPos[Y])))) {
				hit[m_Dom] = true;
				domMaterialID = (m_Dom == X) ? g_SceneMan.GetTerrMatter(m_HitPos[X], m_IntPos[Y]) : g_SceneMan.GetTerrMatter(m_IntPos[X], m_HitPos[Y]);
				domMaterial = g_SceneMan.GetMaterialFromID(domMaterialID);

				// Edit the normal accordingly.
				m_LastHit.BitmapNormal[m_Dom] = -m_Increment[m_Dom];
				// Bounce according to the collision.
				hitAcc[m_Dom] = -hitAcc[m_Dom] - hitAcc[m_Dom] * m_Material->GetRestitution() * domMaterial->GetRestitution();
			}

			// Check for and react upon a collision in the submissive direction of travel.
			if (m_SubStepped && m_Delta[m_Sub] && ((m_Sub == X && g_SceneMan.GetTerrMatter(m_HitPos[X], m_IntPos[Y])) || (m_Sub == Y && g_SceneMan.GetTerrMatter(m_IntPos[X], m_HitPos[Y])))) {
				hit[m_Sub] = true;
				subMaterialID = (m_Sub == X) ? g_SceneMan.GetTerrMatter(m_HitPos[X], m_IntPos[Y]) : g_SceneMan.GetTerrMatter(m_IntPos[X], m_HitPos[Y]);
				subMaterial = g_SceneMan.GetMaterialFromID(subMaterialID);

				// Edit the normal accordingly.
				m_LastHit.BitmapNormal[m_Sub] = -m_Increment[m_Sub];
				// Bounce according to the collision.
				hitAcc[m_Sub] = -hitAcc[m_Sub] - hitAcc[m_Sub] * m_Material->GetRestitution() * subMaterial->GetRestitution();
			}

			// If hit right on the corner of a pixel, bounce straight back with no friction.
			if (!hit[m_Dom] && !hit[m_Sub]) {
				// Edit the normal accordingly.
				m_LastHit.BitmapNormal[m_Dom] = -m_Increment[m_Dom];
				m_LastHit.BitmapNormal[m_Sub] = -m_Increment[m_Sub];

				hit[m_Dom] = true;
				hitAcc[m_Dom] = -hitAcc[m_Dom] - hitAcc[m_Dom] * m_Material->GetRestitution() * hitMaterial->GetRestitution();
				hit[m_Sub] = true;
				hitAcc[m_Sub] = -hitAcc[m_Sub] - hitAcc[m_Sub] * m_Material->GetRestitution() * hitMaterial->GetRestitution();
			} else if (hit[m_Dom] && !hit[m_Sub]) {
				// Calculate the effects of friction.
				m_LastHit.BitmapNormal[m_Sub] = -m_Increment[m_Sub] * m_Material->GetFriction() * domMaterial->GetFriction();
				hitAcc[m_Sub] = -hitAcc[m_Sub] * m_Material->GetFriction() * domMaterial->GetFriction();
			} else if (hit[m_Sub] && !hit[m_Dom]) {
				m_LastHit.BitmapNormal[m_Dom] = -m_Increment[m_Dom] * m_Material->GetFriction() * domMaterial->GetFriction();
				hitAcc[m_Dom] = -hitAcc[m_Dom] * m_Material->GetFriction() * subMaterial->GetFriction();
			}
			m_LastHit.BitmapNormal.Normalize();

			// Calculate effects of moment of inertia will have on the impulse.
			float MIhandle = m_LastHit.HitRadius[HITOR].GetPerpendicular().Dot(m_LastHit.BitmapNormal);

			// Calculate the actual impulse force.
			m_LastHit.ResImpulse[HITOR] = hitAcc / ((1.0F / m_LastHit.TotalMass[HITOR]) + (MIhandle * MIhandle / m_LastHit.MomInertia[HITOR]));
			// Scale by the impulse factor.
			m_LastHit.ResImpulse[HITOR] *= m_LastHit.ImpulseFactor[HITOR];

			return m_LastHit;
		}
		RTEAbort("Atom not supposed to do Terrain hit response if it didn't hit anything!");
		return m_LastHit;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Atom::SetupPos(Vector startPos) {
		RTEAssert(m_OwnerMO, "Stepping an Atom without a parent MO!");

		// Only save the previous positions if they are in the scene
		if (m_IntPos[X] > 0 && m_IntPos[Y] > 0) {
			m_PrevIntPos[X] = m_IntPos[X];
			m_PrevIntPos[Y] = m_IntPos[Y];
			m_IntPos[X] = std::floor(startPos.m_X);
			m_IntPos[Y] = std::floor(startPos.m_Y);
		} else {
			m_IntPos[X] = m_PrevIntPos[X] = std::floor(startPos.m_X);
			m_IntPos[Y] = m_PrevIntPos[Y] = std::floor(startPos.m_Y);
		}

		if ((m_TerrainMatHit = g_SceneMan.GetTerrMatter(m_IntPos[X], m_IntPos[Y])) != g_MaterialAir) {
			m_OwnerMO->SetHitWhatTerrMaterial(m_TerrainMatHit);
			if (m_OwnerMO->IntersectionWarning()) { m_TerrainHitsDisabled = true; }
		} else {
			m_TerrainHitsDisabled = false;
		}
		/*
		if (m_OwnerMO->m_HitsMOs && (m_MOIDHit = g_SceneMan.GetMOIDPixel(m_IntPos[X], m_IntPos[Y])) != g_NoMOID) {
			m_MOHitsDisabled = true;
		} else {
			m_MOHitsDisabled = false;
		}
		*/
		return m_MOIDHit != g_NoMOID || m_TerrainMatHit != g_MaterialAir;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Atom::SetupSeg(Vector startPos, Vector trajectory, float stepRatio) {
		RTEAssert(m_OwnerMO, "Stepping an Atom without a parent MO!");
		m_TerrainMatHit = g_MaterialAir;
		m_MOIDHit = g_NoMOID;

		m_StepRatio = stepRatio;
		m_SegProgress = 0.0F;
		m_SegTraj = trajectory;

		// Bresenham's line drawing algorithm preparation
		m_Delta[X] = std::floor(startPos.m_X + trajectory.m_X) - std::floor(startPos.m_X);
		m_Delta[Y] = std::floor(startPos.m_Y + trajectory.m_Y) - std::floor(startPos.m_Y);
		m_DomSteps = 0;
		m_SubSteps = 0;
		m_SubStepped = false;

		if (m_Delta[X] < 0) {
			m_Increment[X] = -1;
			m_Delta[X] = -m_Delta[X];
		} else {
			m_Increment[X] = 1;
		}

		if (m_Delta[Y] < 0) {
			m_Increment[Y] = -1;
			m_Delta[Y] = -m_Delta[Y];
		} else {
			m_Increment[Y] = 1;
		}

		// Scale by 2, for better accuracy of the error at the first pixel
		m_Delta2[X] = m_Delta[X] << 1;
		m_Delta2[Y] = m_Delta[Y] << 1;

		// If X is dominant, Y is submissive, and vice versa.
		if (m_Delta[X] > m_Delta[Y]) {
			m_Dom = X;
			m_Sub = Y;
		} else {
			m_Dom = Y;
			m_Sub = X;
		}
		m_Error = m_Delta2[m_Sub] - m_Delta[m_Dom];

		m_DomSteps = 0;
		m_SubSteps = 0;
		m_SubStepped = false;
		m_StepWasTaken = false;

		// Return how many steps there are for this atom to take
		return m_Delta[m_Dom] - m_DomSteps;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Atom::StepForward(int numSteps) {
		RTEAssert(m_OwnerMO, "Stepping an Atom without a parent MO!");

		// Only take the step if the step ratio permits it
		float prevProgress = m_SegProgress;
		if (m_Delta[m_Dom] && (m_SegProgress += m_StepRatio) >= std::floor(prevProgress + 1.0F)) {
			m_StepWasTaken = true;
			m_MOIDHit = g_NoMOID;
			m_TerrainMatHit = g_MaterialAir;
			bool hitStep = false;

			if (m_DomSteps < m_Delta[m_Dom]) {
				++m_DomSteps;
				if (m_SubStepped) { ++m_SubSteps; }
				m_SubStepped = false;

				m_IntPos[m_Dom] += m_Increment[m_Dom];
				if (m_Error >= 0) {
					m_IntPos[m_Sub] += m_Increment[m_Sub];
					m_SubStepped = true;
					m_Error -= m_Delta2[m_Dom];
				}
				//if (m_ChangedDir){
					m_Error += m_Delta2[m_Sub];
				//} else {
					//m_Error = m_PrevError;
				//}

	// Scene wrapping, if necessary
				g_SceneMan.WrapPosition(m_IntPos[X], m_IntPos[Y]);

				// Detect terrain hits, if not disabled.
				if (g_MaterialAir != (m_TerrainMatHit = g_SceneMan.GetTerrMatter(m_IntPos[X], m_IntPos[Y]))) {
					// Check if we're temporarily disabled from hitting terrain
					if (!m_TerrainHitsDisabled) {
						m_OwnerMO->SetHitWhatTerrMaterial(m_TerrainMatHit);

						m_HitPos[X] = m_IntPos[X];
						m_HitPos[Y] = m_IntPos[Y];
						RTEAssert(m_TerrainMatHit != 0, "Atom returning step with positive hit but without ID stored!");
						hitStep = true;
					}
				} else {
					// Re-enable terrain hits if we are now out of the terrain again
					m_TerrainHitsDisabled = false;
				}

				// Detect hits with non-ignored MO's, if enabled.
				if (m_OwnerMO->m_HitsMOs) {
					m_MOIDHit = g_SceneMan.GetMOIDPixel(m_IntPos[X], m_IntPos[Y]);
					if (IsIgnoringMOID(m_MOIDHit)) { m_MOIDHit = g_NoMOID; }

					if (m_MOIDHit != g_NoMOID) {
						if (!m_MOHitsDisabled) {
							m_HitPos[X] = m_IntPos[X];
							m_HitPos[Y] = m_IntPos[Y];
							RTEAssert(m_MOIDHit != g_NoMOID, "Atom returning step with positive hit but without ID stored!");
							hitStep = true;
							m_OwnerMO->SetHitWhatMOID(m_MOIDHit);
						}
					} else {
						m_MOHitsDisabled = false;
					}
				}
				return hitStep;
			}
			RTEAssert(0, "Atom shouldn't be taking steps beyond the trajectory!");
			m_OwnerMO->SetToDelete();
		}
		m_StepWasTaken = false;
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Atom::StepBack() {
		RTEAssert(m_OwnerMO, "Stepping an Atom without a parent MO!");

		// Not complete undo because we lost what these were during last step.
		/*
		m_MOIDHit = g_NoMOID;
		m_TerrainMatHit = g_MaterialAir;
		m_ChangedDir = true;
		m_PrevError = m_Error;
		*/

		m_SegProgress -= m_StepRatio;
		if (m_StepWasTaken) {
			--m_DomSteps;
			m_IntPos[m_Dom] -= m_Increment[m_Dom];
			if (m_SubStepped) {
				--m_SubSteps;
				m_IntPos[m_Sub] -= m_Increment[m_Sub];
			}
			g_SceneMan.WrapPosition(m_IntPos[X], m_IntPos[Y]);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Atom::Travel(float travelTime, bool autoTravel, bool scenePreLocked) {
		if (!m_OwnerMO) {
			RTEAbort("Traveling an Atom without a parent MO!");
			return travelTime;
		}
		Vector &position = m_OwnerMO->m_Pos;
		Vector &velocity = m_OwnerMO->m_Vel;
		float mass = m_OwnerMO->GetMass();
		float sharpness = m_OwnerMO->GetSharpness();
		bool &didWrap = m_OwnerMO->m_DidWrap;
		m_LastHit.Reset();

		BITMAP *trailBitmap = 0;

		int hitCount = 0;
		int error = 0;
		int dom = 0;
		int sub = 0;
		int domSteps = 0;
		int subSteps = 0;

		int intPos[2];
		int hitPos[2];
		int delta[2];
		int delta2[2];
		int increment[2];

		float timeLeft = travelTime;
		float segProgress = 0.0F;
		float retardation;

		bool hit[2];
		bool sinkHit;
		bool subStepped;
		//bool endOfTraj = false;

		const Material *hitMaterial = 0; //g_SceneMan.GetMaterialFromID(g_MaterialAir);
		unsigned char hitMaterialID = 0;

		const Material *domMaterial = 0; //g_SceneMan.GetMaterialFromID(g_MaterialAir);
		unsigned char domMaterialID = 0;

		const Material *subMaterial = 0; //g_SceneMan.GetMaterialFromID(g_MaterialAir);
		unsigned char subMaterialID = 0;

		Vector segTraj;
		Vector hitAccel;

		std::vector<std::pair<int, int>> trailPoints;

		// This saves us a few ms because Atom::Travel does a lot of allocations and reallocations if you have a lot of particles.
		// 6 should be enough for most not so fast travels, everything above simply works as usual.
		trailPoints.reserve(6);
		didWrap = false;
		int removeOrphansRadius = m_OwnerMO->m_RemoveOrphanTerrainRadius;
		int removeOrphansMaxArea = m_OwnerMO->m_RemoveOrphanTerrainMaxArea;
		float removeOrphansRate = m_OwnerMO->m_RemoveOrphanTerrainRate;

		// Bake in the Atom offset.
		position += m_Offset;

		// Lock all bitmaps involved outside the loop.
		if (!scenePreLocked) { g_SceneMan.LockScene(); }

		// Loop for all the different straight segments (between bounces etc) that have to be traveled during the timeLeft.
		do {
			intPos[X] = std::floor(position.m_X);
			intPos[Y] = std::floor(position.m_Y);

			// Get trail bitmap and put first pixel.
			if (m_TrailLength) {
				trailBitmap = g_SceneMan.GetMOColorBitmap();
				trailPoints.push_back({ intPos[X], intPos[Y] });
			}
			// Compute and scale the actual on-screen travel trajectory for this segment, based on the velocity, the travel time and the pixels-per-meter constant.
			segTraj = velocity * timeLeft * c_PPM;

			delta[X] = std::floor(position.m_X + segTraj.m_X) - intPos[X];
			delta[Y] = std::floor(position.m_Y + segTraj.m_Y) - intPos[Y];

			//segProgress = 0.0F;
			//delta2[X] = 0;
			//delta2[Y] = 0;
			//increment[X] = 0;
			//increment[Y] = 0;
			hit[X] = false;
			hit[Y] = false;
			//domSteps = 0;
			subSteps = 0;
			subStepped = false;
			sinkHit = false;
			hitAccel.Reset();

			if (delta[X] == 0 && delta[Y] == 0) {
				break;
			}

			//HitMaterial->Reset();
			//domMaterial->Reset();
			//subMaterial->Reset();

			// Bresenham's line drawing algorithm preparation
			if (delta[X] < 0) {
				increment[X] = -1;
				delta[X] = -delta[X];
			} else {
				increment[X] = 1;
			}
			if (delta[Y] < 0) {
				increment[Y] = -1;
				delta[Y] = -delta[Y];
			} else {
				increment[Y] = 1;
			}
			// Scale by 2, for better accuracy of the error at the first pixel
			delta2[X] = delta[X] << 1;
			delta2[Y] = delta[Y] << 1;

			// If X is dominant, Y is submissive, and vice versa.
			if (delta[X] > delta[Y]) {
				dom = X;
				sub = Y;
			} else {
				dom = Y;
				sub = X;
			}

			error = m_ChangedDir ? delta2[sub] - delta[dom] : m_PrevError;

			// Bresenham's line drawing algorithm execution
			for (domSteps = 0; domSteps < delta[dom] && !(hit[X] || hit[Y]); ++domSteps) {
				// Check for the special case if the Atom is starting out embedded in terrain. This can happen if something large gets copied to the terrain and embeds some Atoms.
				if (domSteps == 0 && g_SceneMan.GetTerrMatter(intPos[X], intPos[Y]) != g_MaterialAir) {
					++hitCount;
					hit[X] = hit[Y] = true;
					if (g_SceneMan.TryPenetrate(intPos[X], intPos[Y], velocity * mass * sharpness, velocity, retardation, 0.5F, m_NumPenetrations, removeOrphansRadius, removeOrphansMaxArea, removeOrphansRate)) {
						//segProgress = 0.0F;
						velocity += velocity * retardation;
						continue;
					} else {
						//segProgress = 1.0F;
						velocity.SetXY(0, 0);
						timeLeft = 0.0F;
						break;
					}
				}

				if (subStepped) { ++subSteps; }
				subStepped = false;

				intPos[dom] += increment[dom];
				if (error >= 0) {
					intPos[sub] += increment[sub];
					subStepped = true;
					error -= delta2[dom];
				}
				error += delta2[sub];

				g_SceneMan.WrapPosition(intPos[X], intPos[Y]);

				///////////////////////////////////////////////////////////////////////////////////////////////////
				// Atom-MO collision detection and response.

				// Detect hits with non-ignored MO's, if enabled.
				m_MOIDHit = g_SceneMan.GetMOIDPixel(intPos[X], intPos[Y]);

				if (m_OwnerMO->m_HitsMOs && m_MOIDHit != g_NoMOID && !IsIgnoringMOID(m_MOIDHit)) {
					m_OwnerMO->SetHitWhatMOID(m_MOIDHit);

					++hitCount;
					hitPos[X] = intPos[X];
					hitPos[Y] = intPos[Y];

					// Back up so the Atom is not inside the MO.
					intPos[dom] -= increment[dom];
					if (subStepped) { intPos[sub] -= increment[sub]; }

					m_LastHit.Reset();
					m_LastHit.TotalMass[HITOR] = mass;
					// TODO: Is this right? Perhaps should be 0?")
					m_LastHit.MomInertia[HITOR] = 1.0F;
					m_LastHit.ImpulseFactor[HITOR] = 1.0F;
					m_LastHit.ImpulseFactor[HITEE] = 1.0F;
					//m_LastHit.HitPoint = Vector(hitPos[X], hitPos[Y]);
					m_LastHit.HitVel[HITOR] = velocity;

					MovableObject *MO = g_MovableMan.GetMOFromID(m_MOIDHit);

					if (MO) { MO->SetHitWhatParticleUniqueID(m_OwnerMO->GetUniqueID()); }

					m_LastHit.Body[HITOR] = m_OwnerMO;
					m_LastHit.Body[HITEE] = MO;

#ifndef RELEASE_BUILD
					RTEAssert(m_LastHit.Body[HITEE], "Hitee MO is 0 in Atom::Travel!");
					RTEAssert(m_MOIDHit == m_LastHit.Body[HITEE]->GetID(), "g_MovableMan.GetMOFromID messed up in Atom::MOHitResponse!");
#endif

					// Don't do this normal approximation based on object centers, it causes particles to 'slide into' sprite objects when they should be resting on them.
					// Orthogonal normals only, as the pixel boundaries themselves! See further down for the setting of this.
					//m_LastHit.BitmapNormal = m_LastHit.Body[HITOR]->GetPos() - m_LastHit.Body[HITEE]->GetPos();
					//m_LastHit.BitmapNormal.Normalize();

					// Gold special collection case!
					// TODO: Make material IDs more robust!")
					if (m_Material->GetIndex() == c_GoldMaterialID && g_MovableMan.IsOfActor(m_MOIDHit)) {
						Actor *pActor = dynamic_cast<Actor *>(g_MovableMan.GetMOFromID(m_LastHit.Body[HITEE]->GetRootID()));
						if (pActor) {
							pActor->AddGold(m_OwnerMO->GetMass() * g_SceneMan.GetOzPerKg() * removeOrphansRadius ? 1.25F : 1.0F);
							m_OwnerMO->SetToDelete(true);
							// This is to break out of the do-while and the function properly.
							m_LastHit.Terminate[HITOR] = hit[dom] = hit[sub] = true;
							break;
						}
					}

					// Check for the collision point in the dominant direction of travel.
					if (delta[dom] && ((dom == X && g_SceneMan.GetMOIDPixel(hitPos[X], intPos[Y]) != g_NoMOID) || (dom == Y && g_SceneMan.GetMOIDPixel(intPos[X], hitPos[Y]) != g_NoMOID))) {
						hit[dom] = true;
						m_LastHit.HitPoint = (dom == X) ? Vector(hitPos[X], intPos[Y]) : Vector(intPos[X], hitPos[Y]);
						m_LastHit.BitmapNormal[dom] = -increment[dom];
					}

					// Check for the collision point in the submissive direction of travel.
					if (subStepped && delta[sub] && ((sub == X && g_SceneMan.GetMOIDPixel(hitPos[X], intPos[Y]) != g_NoMOID) || (sub == Y && g_SceneMan.GetMOIDPixel(intPos[X], hitPos[Y]) != g_NoMOID))) {
						hit[sub] = true;
						if (m_LastHit.HitPoint.IsZero()) {
							m_LastHit.HitPoint = (sub == X) ? Vector(hitPos[X], intPos[Y]) : Vector(intPos[X], hitPos[Y]);
						} else {
							// We hit pixels in both sub and dom directions on the other MO, a corner hit.
							m_LastHit.HitPoint.SetXY(hitPos[X], hitPos[Y]);
							m_LastHit.BitmapNormal[sub] = -increment[sub];
						}
					}

					// If neither the direct dominant or sub directions yielded a collision point, then that means we hit right on the corner edge of a pixel, and that is the collision point.
					if (!hit[dom] && !hit[sub]) {
						hit[dom] = hit[sub] = true;
						m_LastHit.HitPoint.SetXY(hitPos[X], hitPos[Y]);
						m_LastHit.BitmapNormal.SetXY(-increment[X], -increment[Y]);
					}

					// Now normalize the normal in case it's diagonal due to hit in both directions
					m_LastHit.BitmapNormal.Normalize();

					// Make this Atom ignore hits with this MO for the rest of the frame, to avoid erroneous multiple hits because the hit MO doesn't move away until it itself

					// is updated and the impulses produced in this hit are taken into effect.
					AddMOIDToIgnore(m_MOIDHit);

					m_LastHit.Body[HITEE]->CollideAtPoint(m_LastHit);
					hitAccel = m_LastHit.ResImpulse[HITOR] / mass;

					// Report the hit to both MO's in collision
					m_LastHit.RootBody[HITOR] = m_LastHit.Body[HITOR]->GetRootParent();
					m_LastHit.RootBody[HITEE] = m_LastHit.Body[HITEE]->GetRootParent();
					m_LastHit.RootBody[HITOR]->OnMOHit(m_LastHit.RootBody[HITEE]);
					m_LastHit.RootBody[HITEE]->OnMOHit(m_LastHit.RootBody[HITOR]);
				}

				///////////////////////////////////////////////////////////////////////////////////////////////////
				// Atom-Terrain collision detection and response.

				// If there was no MO collision detected, then check for terrain hits.
				else if ((hitMaterialID = g_SceneMan.GetTerrMatter(intPos[X], intPos[Y])) && !m_OwnerMO->m_IgnoreTerrain) {
					if (hitMaterialID != g_MaterialAir) { m_OwnerMO->SetHitWhatTerrMaterial(hitMaterialID); }

					hitMaterial = g_SceneMan.GetMaterialFromID(hitMaterialID);
					hitPos[X] = intPos[X];
					hitPos[Y] = intPos[Y];
					++hitCount;

#ifdef DEBUG_BUILD
					if (m_TrailLength) { putpixel(trailBitmap, intPos[X], intPos[Y], 199); }
#endif
					// Try penetration of the terrain.
					if (hitMaterial->GetIndex() != g_MaterialOutOfBounds && g_SceneMan.TryPenetrate(intPos[X], intPos[Y], velocity * mass * sharpness, velocity, retardation, 0.65F, m_NumPenetrations, removeOrphansRadius, removeOrphansMaxArea, removeOrphansRate)) {
						hit[dom] = hit[sub] = sinkHit = true;
						++m_NumPenetrations;
						m_ChangedDir = false;
						m_PrevError = error;

						// Calculate the penetration/sink response effects.
						hitAccel = velocity * retardation;
					} else {
						// Penetration failed, bounce.
						m_NumPenetrations = 0;
						m_ChangedDir = true;
						m_PrevError = error;

						// Back up so the Atom is not inside the terrain.
						intPos[dom] -= increment[dom];
						if (subStepped) { intPos[sub] -= increment[sub]; }

						// Undo scene wrapping, if necessary
						g_SceneMan.WrapPosition(intPos[X], intPos[Y]);

						// TODO: improve sticky logic!
						// Check if particle is sticky and should adhere to where it collided
						if (m_Material->GetStickiness() >= RandomNum() && velocity.GetLargest() > 0.5F) {
							// SPLAT, so update position, apply to terrain and delete, and stop traveling
							m_OwnerMO->SetPos(Vector(intPos[X], intPos[Y]));
							g_SceneMan.GetTerrain()->ApplyMovableObject(m_OwnerMO);
							m_OwnerMO->SetToDelete(true);
							m_LastHit.Terminate[HITOR] = hit[dom] = hit[sub] = true;
							break;
						}

						// Check for and react upon a collision in the dominant direction of travel.
						if (delta[dom] && ((dom == X && g_SceneMan.GetTerrMatter(hitPos[X], intPos[Y])) || (dom == Y && g_SceneMan.GetTerrMatter(intPos[X], hitPos[Y])))) {
							hit[dom] = true;
							domMaterialID = (dom == X) ? g_SceneMan.GetTerrMatter(hitPos[X], intPos[Y]) : g_SceneMan.GetTerrMatter(intPos[X], hitPos[Y]);
							domMaterial = g_SceneMan.GetMaterialFromID(domMaterialID);

							// Bounce according to the collision.
							hitAccel[dom] = -velocity[dom] - velocity[dom] * m_Material->GetRestitution() * domMaterial->GetRestitution();
						}

						// Check for and react upon a collision in the submissive direction of travel.
						if (subStepped && delta[sub] && ((sub == X && g_SceneMan.GetTerrMatter(hitPos[X], intPos[Y])) || (sub == Y && g_SceneMan.GetTerrMatter(intPos[X], hitPos[Y])))) {
							hit[sub] = true;
							subMaterialID = (sub == X) ? g_SceneMan.GetTerrMatter(hitPos[X], intPos[Y]) : g_SceneMan.GetTerrMatter(intPos[X], hitPos[Y]);
							subMaterial = g_SceneMan.GetMaterialFromID(subMaterialID);

							// Bounce according to the collision.
							hitAccel[sub] = -velocity[sub] - velocity[sub] * m_Material->GetRestitution() * subMaterial->GetRestitution();
						}

						// If hit right on the corner of a pixel, bounce straight back with no friction.
						if (!hit[dom] && !hit[sub]) {
							hit[dom] = true;
							hitAccel[dom] = -velocity[dom] - velocity[dom] * m_Material->GetRestitution() *  hitMaterial->GetRestitution();
							hit[sub] = true;
							hitAccel[sub] = -velocity[sub] - velocity[sub] * m_Material->GetRestitution() * hitMaterial->GetRestitution();
						} else if (hit[dom] && !hit[sub]) {
							// Calculate the effects of friction.
							hitAccel[sub] -= velocity[sub] * m_Material->GetFriction() * domMaterial->GetFriction();
						} else if (hit[sub] && !hit[dom]) {
							hitAccel[dom] -= velocity[dom] * m_Material->GetFriction() * subMaterial->GetFriction();
						}
					}
				} else if (m_TrailLength) {
					trailPoints.push_back({ intPos[X], intPos[Y] });
				}

				///////////////////////////////////////////////////////////////////////////////////////////////////
				// Apply Collision Responses

				// If we hit anything, and are about to start a new segment instead of a step, apply the collision response effects to the owning MO.
				if ((hit[X] || hit[Y]) && !m_LastHit.Terminate[HITOR]) {
					// Calculate the progress made on this segment before hitting something.
					// We count the hitting step made if it resulted in a terrain sink, because the Atoms weren't stepped back out of intersection.
					//segProgress = static_cast<float>(domSteps + sinkHit) / static_cast<float>(delta[dom]);
					segProgress = (static_cast<float>(domSteps + static_cast<int>(sinkHit)) < delta[dom]) ? (static_cast<float>(domSteps + static_cast<int>(sinkHit)) / std::fabs(static_cast<float>(segTraj[dom]))) : 1.0F;

					// Now calculate the total time left to travel, according to the progress made.
					timeLeft -= timeLeft * segProgress;

					// Move position forward to the hit position.
					//position += segTraj * segProgress;
					// Only move the dom forward by int domSteps, so we don't cross into a pixel too far
					position[dom] += (domSteps + static_cast<int>(sinkHit)) * increment[dom];

					// Move the submissive direction forward by as many int steps, or the full float segTraj if all sub-steps are clear
					if ((subSteps + static_cast<int>(subStepped && sinkHit)) < delta[sub]) {
						position[sub] += (subSteps + static_cast<int>(subStepped && sinkHit)) * increment[sub];
					} else {
						position[sub] += segTraj[sub];
					}

					Vector testPos = position - m_Offset;

					didWrap = g_SceneMan.WrapPosition(testPos) || didWrap;

					// Apply the collision response acceleration to the linear velocity of the owner MO.
					velocity += hitAccel;
				}
			}
		} while ((hit[X] || hit[Y]) && /* !segTraj.GetFloored().IsZero() && */ hitCount < 100 && !m_LastHit.Terminate[HITOR]);

		//RTEAssert(hitCount < 100, "Atom travel resulted in more than 100 segments!!");

		// Draw the trail
		if (g_TimerMan.DrawnSimUpdate() && m_TrailLength) {
			int length = m_TrailLength /* + 3 * RandomNum()*/;
			for (int i = trailPoints.size() - std::min(length, static_cast<int>(trailPoints.size())); i < trailPoints.size(); ++i) {
				putpixel(trailBitmap, trailPoints[i].first, trailPoints[i].second, m_TrailColor.GetIndex());
			}
		}

		// Unlock all bitmaps involved.
		//if (m_TrailLength) { trailBitmap->UnLock(); }
		if (!scenePreLocked) { g_SceneMan.UnlockScene(); }

		// Extract Atom offset.
		position -= m_Offset;

		// Travel along the remaining segTraj.
		if (!(hit[X] || hit[Y]) && autoTravel) { position += segTraj; }

		didWrap = g_SceneMan.WrapPosition(position) || didWrap;

		ClearMOIDIgnoreList();

		return hitCount;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void HitData::Clear() {
		HitPoint.Reset();
		VelDiff.Reset();
		BitmapNormal.Reset();
		HitDenominator = 0;

		for (unsigned short i = 0; i < 2; ++i) {
			Body[i] = 0;
			RootBody[i] = 0;
			HitVel[i].Reset();
			TotalMass[i] = 0;
			MomInertia[i] = 0;
			HitRadius[i].Reset();
			HitMaterial[i] = 0;
			PreImpulse[i].Reset();
			ResImpulse[i].Reset();
			ImpulseFactor[i] = 0;
			SquaredMIHandle[i] = 0;
			Terminate[i] = false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	HitData & HitData::operator=(const HitData &rhs) {
		if (this == &rhs) {
			return *this;
		}
		Clear();

		HitPoint = rhs.HitPoint;
		VelDiff = rhs.VelDiff;
		BitmapNormal = rhs.BitmapNormal;
		HitDenominator = rhs.HitDenominator;

		for (unsigned short i = 0; i < 2; ++i) {
			Body[i] = rhs.Body[i];
			RootBody[i] = rhs.RootBody[i];
			HitVel[i] = rhs.HitVel[i];
			TotalMass[i] = rhs.TotalMass[i];
			MomInertia[i] = rhs.MomInertia[i];
			HitRadius[i] = rhs.HitRadius[i];
			HitMaterial[i] = rhs.HitMaterial[i];
			PreImpulse[i] = rhs.PreImpulse[i];
			ResImpulse[i] = rhs.ResImpulse[i];
			ImpulseFactor[i] = rhs.ImpulseFactor[i];
			SquaredMIHandle[i] = rhs.SquaredMIHandle[i];
			Terminate[i] = rhs.Terminate[i];
		}
		return *this;
	}
}