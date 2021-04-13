#include "AtomGroup.h"
#include "SLTerrain.h"
#include "MOSRotating.h"
#include "LimbPath.h"
#include "ConsoleMan.h"

namespace RTE {

	ConcreteClassInfo(AtomGroup, Entity, 500)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AtomGroup::Clear() {
		m_Atoms.clear();
		m_SubGroups.clear();
		m_OwnerMOSR = nullptr;
		m_Material = g_SceneMan.GetMaterialFromID(g_MaterialAir);
		m_AutoGenerate = false;
		m_Resolution = 1;
		m_Depth = 0;
		m_JointOffset.Reset();
		m_LimbPos.Reset();
		m_MomentOfInertia = 0;
		m_IgnoreMOIDs.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int AtomGroup::Create() {
		if (Entity::Create() < 0) {
			return -1;
		}
		if (!m_AutoGenerate) {
			// Needs to be set manually by the new MO owner.
			m_OwnerMOSR = nullptr;
			m_Resolution = 0;
		}
		if (m_Atoms.empty()) {
			Atom *atom = new Atom(Vector(), m_Material, m_OwnerMOSR);
			m_Atoms.push_back(atom);
		} else if (m_Material->GetIndex() != m_Atoms.front()->GetMaterial()->GetIndex()) {
			m_Material = m_Atoms.front()->GetMaterial();
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int AtomGroup::Create(const AtomGroup &reference, bool onlyCopyOwnerAtoms) {
		Entity::Create(reference);

		m_OwnerMOSR = nullptr; // Needs to be set manually by the new MO owner.
		m_Material = reference.m_Material;
		m_AutoGenerate = false; /*reference.m_AutoGenerate;*/ // TODO: Investigate if AutoGenerating here can screw something up when copy-creating over an existing AtomGroup.
		m_Resolution = reference.m_Resolution;
		m_Depth = reference.m_Depth;
		m_JointOffset = reference.m_JointOffset;

		m_Atoms.clear();
		m_SubGroups.clear();

		for (const Atom *atom : reference.m_Atoms) {
			if (!onlyCopyOwnerAtoms || atom->GetSubID() == 0) {
				Atom *atomCopy = new Atom(*atom);
				atomCopy->SetIgnoreMOIDsByGroup(&m_IgnoreMOIDs);
				m_Atoms.push_back(atomCopy);

				long subgroupID = atomCopy->GetSubID();
				if (subgroupID != 0) {
					if (m_SubGroups.find(subgroupID) == m_SubGroups.end()) { m_SubGroups.insert({ subgroupID, std::list<Atom *>() }); }

					m_SubGroups.find(subgroupID)->second.push_back(atomCopy);
				}
			}
		}

		m_IgnoreMOIDs.clear();

		for (const MOID moidToIgnore : reference.m_IgnoreMOIDs) {
			m_IgnoreMOIDs.push_back(moidToIgnore);
		}

		if (!reference.m_Atoms.empty()) { m_Material = reference.m_Atoms.front()->GetMaterial(); }

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int AtomGroup::Create(MOSRotating *ownerMOSRotating, Material const *material, int resolution, int depth) {
		RTEAssert(ownerMOSRotating, "Trying to generate an AtomGroup for a MOSRotating without a sprite!");

		m_OwnerMOSR = ownerMOSRotating;
		m_Material = material;
		m_AutoGenerate = true;
		m_Resolution = (resolution > 0) ? resolution : c_DefaultAtomGroupResolution;
		m_Depth = depth;

		GenerateAtomGroup(m_OwnerMOSR);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int AtomGroup::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "Material") {
			Material mat;
			mat.Reset();
			reader >> mat;
			m_Material = mat.GetIndex() ? g_SceneMan.GetMaterialFromID(mat.GetIndex()) : g_SceneMan.GetMaterial(mat.GetPresetName());

			if (!m_Material) {
				g_ConsoleMan.PrintString("ERROR: Failed to find matching Material preset \"" + mat.GetPresetName() + "\" " + GetFormattedReaderPosition() + ". Was it defined with AddMaterial?");
				m_Material = g_SceneMan.GetMaterialFromID(g_MaterialAir);
				RTEAssert(m_Material, "Failed to find matching Material preset \"" + mat.GetPresetName() + "\" or even fall back to \"Air\" " + GetFormattedReaderPosition() + ".\nAborting!");
			}
		} else if (propName == "AutoGenerate") {
			reader >> m_AutoGenerate;
		} else if (propName == "Resolution") {
			reader >> m_Resolution;
		} else if (propName == "Depth") {
			reader >> m_Depth;
		} else if (propName == "AddAtom") {
			Atom *atom = new Atom;
			reader >> *atom;
			m_Atoms.push_back(atom);
		} else if (propName == "JointOffset") {
			reader >> m_JointOffset;
		} else {
			return Entity::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int AtomGroup::Save(Writer &writer) const {
		Entity::Save(writer);

		writer.NewProperty("Material");
		writer << m_Material;
		writer.NewProperty("AutoGenerate");
		writer << m_AutoGenerate;
		writer.NewProperty("Resolution");
		writer << m_Resolution;
		writer.NewProperty("Depth");
		writer << m_Depth;

		// Only write out Atoms if they were manually specified
		if (!m_AutoGenerate) {
			for (const Atom *atom : m_Atoms) {
				writer.NewProperty("AddAtom");
				writer << *atom;
			}
		}

		writer.NewProperty("JointOffset");
		writer << m_JointOffset;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AtomGroup::Destroy(bool notInherited) {
		for (const Atom *atom : m_Atoms) {
			delete atom;
		}
		if (!notInherited) { Entity::Destroy(); }
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float AtomGroup::CalculateMaxRadius() const {
		float magnitude;
		float longest = 0.0F;

		for (const Atom *atom : m_Atoms) {
			magnitude = atom->GetOffset().GetMagnitude();
			if (magnitude > longest) { longest = magnitude; }
		}
		return longest;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AtomGroup::SetOwner(MOSRotating *newOwner) {
		m_OwnerMOSR = newOwner;
		for (Atom *atom : m_Atoms) {
			atom->SetOwner(m_OwnerMOSR);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector AtomGroup::GetAdjustedAtomOffset(const Atom *atom) const {
		return atom->GetOffset().GetXFlipped(m_OwnerMOSR->m_HFlipped) * m_OwnerMOSR->GetRotMatrix();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float AtomGroup::GetMomentOfInertia() {
		float currentOwnerMass = (m_OwnerMOSR->GetMass() != 0 ? m_OwnerMOSR->GetMass() : 0.0001F);
		if (m_MomentOfInertia == 0.0F || std::abs(m_StoredOwnerMass - currentOwnerMass) >= (m_StoredOwnerMass / 10.0F)) {
			RTEAssert(m_OwnerMOSR, "Tried to calculate moment of inertia for an AtomGroup with no parent!");

			m_StoredOwnerMass = currentOwnerMass;
			float distMass = m_StoredOwnerMass / static_cast<float>(m_Atoms.size());
			float radius = 0.0F;
			for (const Atom *atom : m_Atoms) {
				radius = atom->GetOffset().GetMagnitude() * c_MPP;
				m_MomentOfInertia += distMass * radius * radius;
			}
		}
		// Avoid zero (if radius is nonexistent, for example), will cause divide by zero problems otherwise.
		if (m_MomentOfInertia == 0.0F) { m_MomentOfInertia = 0.000001F; }

		return m_MomentOfInertia;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AtomGroup::AddAtoms(const std::list<Atom *> &atomList, long subgroupID, const Vector &offset, const Matrix &offsetRotation) {
		if (m_SubGroups.count(subgroupID) == 0) { m_SubGroups.insert({ subgroupID, std::list<Atom *>() }); }

		Atom *atomToAdd;
		for (const Atom * atom : atomList) {
			atomToAdd = new Atom(*atom);
			atomToAdd->SetSubID(subgroupID);
			atomToAdd->SetOffset(offset + (atomToAdd->GetOriginalOffset() * offsetRotation));
			atomToAdd->SetOwner(m_OwnerMOSR);
			atomToAdd->SetIgnoreMOIDsByGroup(&m_IgnoreMOIDs);
			m_Atoms.push_back(atomToAdd);
			m_SubGroups.at(subgroupID).push_back(atomToAdd);
		}
		if (!atomList.empty()) { m_MomentOfInertia = 0.0F; }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AtomGroup::RemoveAtoms(long removeID) {
		bool removedAny = false;
		std::list<Atom *>::iterator eraseItr;

		// TODO: Look into using remove_if.
		for (std::list<Atom *>::iterator atomItr = m_Atoms.begin(); atomItr != m_Atoms.end();) {
			if ((*atomItr)->GetSubID() == removeID) {
				delete (*atomItr);
				eraseItr = atomItr;
				atomItr++;
				m_Atoms.erase(eraseItr);
				removedAny = true;
			} else {
				atomItr++;
			}
		}
		m_SubGroups.erase(removeID);
		if (removedAny) { m_MomentOfInertia = 0.0F; }

		return removedAny;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AtomGroup::UpdateSubAtoms(long subgroupID, const Vector &newOffset, const Matrix &newOffsetRotation) {
		if (m_SubGroups.empty() || m_SubGroups.count(subgroupID) == 0) {
			return false;
		}
		RTEAssert(!m_SubGroups.at(subgroupID).empty(), "Found an empty subgroup list in AtomGroup!?");

		for (Atom *subGroupAtom : m_SubGroups.at(subgroupID)) {
			subGroupAtom->SetOffset(newOffset + (subGroupAtom->GetOriginalOffset() * newOffsetRotation));
		}
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float AtomGroup::Travel(float travelTime, bool callOnBounce, bool callOnSink, bool scenePreLocked) {
		return Travel(m_OwnerMOSR->m_Pos, m_OwnerMOSR->m_Vel, m_OwnerMOSR->m_Rotation, m_OwnerMOSR->m_AngularVel, m_OwnerMOSR->m_DidWrap, m_OwnerMOSR->m_TravelImpulse, m_OwnerMOSR->GetMass(), travelTime, callOnBounce, callOnSink, scenePreLocked);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// TODO: Break down and rework this trainwreck.
	float AtomGroup::Travel(Vector &position, Vector &velocity, Matrix &rotation, float &angularVel, bool &didWrap, Vector &totalImpulse, float mass, float travelTime, bool callOnBounce, bool callOnSink, bool scenePreLocked) {
		RTEAssert(m_OwnerMOSR, "Tried to travel an AtomGroup that has no parent!");

		m_MomentOfInertia = GetMomentOfInertia();

		didWrap = false;

		int segCount = 0;
		int stepCount = 0;
		int stepsOnSeg = 0;
		int hitCount = 0;
		float timeLeft = travelTime;
		float segProgress = 0;
		float segRatio = 0;
		float retardation = 0;

		// TODO: Make this dependent on AtomGroup radius!, not hardcoded
		const float segRotLimit = c_PI / 6.0F;

		bool hitsMOs = m_OwnerMOSR->m_HitsMOs;
		bool hitStep = false;
		bool halted = false;

		// TODO: Look into pre-hit stuff and how it can be used to improve collision.
		//Vector preHitPos;
		//float preHitRot;

		Vector linSegTraj; // The planned travel trajectory of this AtomGroup's origin in pixels.

		HitData hitData;

		std::map<MOID, std::list<Atom *>> hitMOAtoms;
		std::list<Atom *> hitTerrAtoms;
		std::list<Atom *> penetratingAtoms;
		std::list<Atom *> hitResponseAtoms;

		// Lock all bitmaps involved outside the loop - only relevant for video bitmaps so disabled at the moment.
		//if (!scenePreLocked) { g_SceneMan.LockScene(); }

		// Loop for all the different straight segments (between bounces etc) that have to be traveled during the travelTime.
		do {
			// First see what Atoms are inside either the terrain or another MO, and cause collisions responses before even starting the segment
			for (Atom *atom : m_Atoms) {
				const Vector startOff = m_OwnerMOSR->RotateOffset(atom->GetOffset());

				if (atom->SetupPos(position + startOff)) {
					hitData.Reset();
					if (atom->IsIgnoringTerrain()) {
						// Calculate and store the accurate hit radius of the Atom in relation to the CoM.
						hitData.HitRadius[HITOR] = startOff * c_MPP;
						// Figure out the pre-collision velocity of the hitting Atom due to body translation and rotation.
						hitData.HitVel[HITOR] = velocity + hitData.HitRadius[HITOR].GetPerpendicular() * angularVel;

						/*
						radMag = hitData.HitRadius[HITOR].GetMagnitude();
						// These are set temporarily here, will be re-set later when the normal of the hit terrain bitmap (ortho pixel side) is known.
						hitData.HitDenominator = (1.0 / massDistribution) + ((radMag * radMag) / momentInertiaDistribution);
						hitData.PreImpulse[HITOR] = hitData.HitVel[HITOR] / hitData.HitDenominator;
						// Set the Atom with the hitData with all the info we have so far.
						atom->SetHitData(hitData);

						hitFactor = 1.0; // / (float)hitTerrAtoms.size();
						atom->GetHitData().mass[HITOR] = mass;
						atom->GetHitData().MomInertia[HITOR] = m_MomentOfInertia;
						atom->GetHitData().ImpulseFactor[HITOR] = hitFactor;

						// Call the call-on-bounce function, if requested.
						if (m_OwnerMOSR && callOnBounce) { halted = halted || m_OwnerMOSR->OnBounce((atom->GetHitData()); }

						// Compute and store this Atom's collision response impulse force.
						// Calculate effects of moment of inertia will have on the impulse.
						float MIhandle = m_LastHit.HitRadius[HITOR].GetPerpendicular().Dot(m_LastHit.BitmapNormal);
						*/

						if (!(atom->GetNormal().IsZero())) {
							hitData.ResImpulse[HITOR] = m_OwnerMOSR->RotateOffset(atom->GetNormal());
							hitData.ResImpulse[HITOR] = -hitData.ResImpulse[HITOR];
							hitData.ResImpulse[HITOR].SetMagnitude(hitData.HitVel[HITOR].GetMagnitude());

							// Apply terrain conflict response
							velocity += hitData.ResImpulse[HITOR] / mass;
							angularVel += hitData.HitRadius[HITOR].GetPerpendicular().Dot(hitData.ResImpulse[HITOR]) / m_MomentOfInertia;
							// Accumulate all the impulse forces so the MO can determine if it took damaged as a result
							totalImpulse += hitData.ResImpulse[HITOR];
						}
					}
				}
#ifdef DEBUG_BUILD
				// TODO: Remove this once AtomGroup drawing in Material layer draw mode is implemented.
				// Draw the positions of the Atoms at the start of each segment, for visual debugging.
				//putpixel(g_SceneMan.GetMOColorBitmap(), atom->GetCurrentPos().GetFloorIntX(), atom->GetCurrentPos().GetFloorIntY(), 122);
#endif
			}

			linSegTraj = velocity * timeLeft * c_PPM;

			// The amount of rotation to be achieved during the time slot, in radians.
			float rotDelta = angularVel * timeLeft;

			// Cap the segment if the rotation is too severe. This will chunk the segment into several in order to more closely approximate the arc an Atom on a rotating body will trace.
			if (std::fabs(rotDelta) > segRotLimit) {
				segRatio = segRotLimit / std::fabs(rotDelta);
				rotDelta = (rotDelta > 0) ? segRotLimit : -segRotLimit;
				linSegTraj *= segRatio;
			} else {
				segRatio = 1.0F;
			}
			segProgress = 0.0F;
		
			if (linSegTraj.IsZero() && rotDelta == 0) {
				break;
			}

			for (Atom *atom : m_Atoms) {
				// Calculate the segment trajectory for each individual Atom, with rotations considered.
				const Vector startOff = m_OwnerMOSR->RotateOffset(atom->GetOffset());
				const Vector trajFromAngularTravel = Vector(startOff).RadRotate(rotDelta) - startOff;

				// Set up the initial rasterized step for each Atom and save the longest trajectory.
				if (atom->SetupSeg(position + startOff, linSegTraj + trajFromAngularTravel) > stepsOnSeg) { stepsOnSeg = atom->GetStepsLeft(); }
			}

			for (Atom *atom : m_Atoms) {
				atom->SetStepRatio(static_cast<float>(atom->GetStepsLeft()) / static_cast<float>(stepsOnSeg));
			}

			// STEP LOOP ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			hitStep = false;

			for (stepCount = 0; !hitStep && stepsOnSeg != 0 && stepCount < stepsOnSeg; ++stepCount) {

				// SCENE COLLISION DETECTION ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				hitData.Reset();
				hitMOAtoms.clear();
				hitTerrAtoms.clear();
				penetratingAtoms.clear();
				hitResponseAtoms.clear();
				
				int atomsHitMOsCount = 0;

				for (Atom *atom : m_Atoms) {
					if (atom->StepForward()) {
						// If something was hit, first check for terrain hit.
						if (atom->HitWhatTerrMaterial()) {
							m_OwnerMOSR->SetHitWhatTerrMaterial(atom->HitWhatTerrMaterial());
							hitTerrAtoms.push_back(atom);
						}
						if (hitsMOs) {
							const MOID tempMOID = atom->HitWhatMOID();
							if (tempMOID != g_NoMOID) {
								m_OwnerMOSR->m_MOIDHit = tempMOID;
								MovableObject *moCollidedWith = g_MovableMan.GetMOFromID(tempMOID);
								if (moCollidedWith && moCollidedWith->HitWhatMOID() == g_NoMOID) { moCollidedWith->SetHitWhatMOID(m_OwnerMOSR->m_MOID); }

								// See if we already have another Atom hitting this MO in this step. If not, then create a new list unique for that MO's ID and insert into the map of MO-hitting Atoms.
								if (!(hitMOAtoms.count(tempMOID))) {
									std::list<Atom *> newList;
									newList.push_back(atom);
									hitMOAtoms.insert({ tempMOID, newList });
								} else {
									// If another Atom of this group has already hit this same MO during this step, go ahead and add the new Atom to the corresponding map for that MOID.
									hitMOAtoms.at(tempMOID).push_back(atom);
								}

								// Add the hit MO to the ignore list of ignored MOIDs
								//AddMOIDToIgnore(tempMOID);

								// Count the number of Atoms of this group that hit MOs this step. Used to properly distribute the mass of the owner MO in later collision responses during this step.
								atomsHitMOsCount++;
							}
						}
#ifdef DEBUG_BUILD
						// TODO: Remove this once AtomGroup drawing in Material layer draw mode is implemented.
						Vector tPos = atom->GetCurrentPos();
						Vector tNorm = m_OwnerMOSR->RotateOffset(atom->GetNormal()) * 7;
						line(g_SceneMan.GetMOColorBitmap(), tPos.GetFloorIntX(), tPos.GetFloorIntY(), tPos.GetFloorIntX() + tNorm.GetFloorIntX(), tPos.GetFloorIntY() + tNorm.GetFloorIntY(), 244);
						// Draw the positions of the hit points on screen for easy debugging.
						//putpixel(g_SceneMan.GetMOColorBitmap(), tPos.GetFloorIntX(), tPos.GetFloorIntY(), 5);
#endif
					}
				}

				// If no collisions, continue on to the next step.
				if (hitTerrAtoms.empty() && hitMOAtoms.empty()) {
					continue;
				}

				// There are colliding Atoms, therefore the group hit something.
				hitStep = true;
				++hitCount;

				// Calculate the progress made on this segment before hitting something. Special case of being at rest.
				if (stepCount == 0 && stepsOnSeg == 1) {
					halted = true;
				}
				segProgress = static_cast<float>(stepCount) / static_cast<float>(stepsOnSeg);

				/*
				preHitPos = position;
				preHitRot = rotation.GetRadAngle();
				*/

				// Move position forward to the hit position.
				position += linSegTraj * segProgress;
				didWrap = g_SceneMan.WrapPosition(position) || didWrap;

				// Move rotation forward according to the progress made on the segment.
				rotation += rotDelta * segProgress;

				// TERRAIN COLLISION RESPONSE ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				bool somethingPenetrated;

				do {
					somethingPenetrated = false;

					const float massDistribution = mass / static_cast<float>(hitTerrAtoms.size() * (m_Resolution ? m_Resolution : 1));
					const float momentInertiaDistribution = m_MomentOfInertia / static_cast<float>(hitTerrAtoms.size() * (m_Resolution ? m_Resolution : 1));

					// Determine which of the colliding Atoms will penetrate the terrain.
					for (std::list<Atom*>::iterator atomItr = hitTerrAtoms.begin(); atomItr != hitTerrAtoms.end(); ) {
						// Calculate and store the accurate hit radius of the Atom in relation to the CoM
						hitData.HitRadius[HITOR] = m_OwnerMOSR->RotateOffset((*atomItr)->GetOffset()) * c_MPP;
						// Figure out the pre-collision velocity of the hitting Atom due to body translation and rotation.
						hitData.HitVel[HITOR] = velocity + hitData.HitRadius[HITOR].GetPerpendicular() * angularVel;

						const float radMag = hitData.HitRadius[HITOR].GetMagnitude();
						// These are set temporarily here, will be re-set later when the normal of the hit terrain bitmap (ortho pixel side) is known.
						hitData.HitDenominator = (1.0F / massDistribution) + ((radMag * radMag) / momentInertiaDistribution);
						hitData.PreImpulse[HITOR] = hitData.HitVel[HITOR] / hitData.HitDenominator;
						// Set the Atom with the HitData with all the info we have so far.
						(*atomItr)->SetHitData(hitData);

						if (g_SceneMan.WillPenetrate((*atomItr)->GetCurrentPos().GetFloorIntX(), (*atomItr)->GetCurrentPos().GetFloorIntY(), hitData.PreImpulse[HITOR])) {
							// Move the penetrating Atom to the penetrating list from the collision list.
							penetratingAtoms.push_back(*atomItr);
							atomItr = hitTerrAtoms.erase(atomItr);
							somethingPenetrated = true;
						} else {
							++atomItr;
						}
					}
				} while (!hitTerrAtoms.empty() && somethingPenetrated);

				// TERRAIN BOUNCE ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				// If some Atoms could not penetrate even though all the impulse was on them, gather the bounce results and apply them to the owner.
				if (!hitTerrAtoms.empty()) {
					// Step back all Atoms that previously took one during this step iteration. This is so we aren't intersecting the hit MO anymore.
					for (Atom *hitTerrAtom : hitTerrAtoms) {
						hitTerrAtom->StepBack();
					}

					// Calculate the distributed mass that each bouncing Atom has.
					//massDistribution = mass /*/ (hitTerrAtoms.size() * (m_Resolution ? m_Resolution : 1))*/;
					//momentInertiaDistribution = m_MomentOfInertia/* / (hitTerrAtoms.size() * (m_Resolution ? m_Resolution : 1))*/;

					const float hitFactor = 1.0F / static_cast<float>(hitTerrAtoms.size());

					// Gather the collision response effects so that the impulse force can be calculated.
					for (Atom *hitTerrAtom : hitTerrAtoms) {
						hitTerrAtom->GetHitData().TotalMass[HITOR] = mass;
						hitTerrAtom->GetHitData().MomInertia[HITOR] = m_MomentOfInertia;
						hitTerrAtom->GetHitData().ImpulseFactor[HITOR] = hitFactor;

						// Get the HitData so far gathered for this Atom.
						//hitData = hitTerrAtom->GetHitData();

						// Call the call-on-bounce function, if requested.
						if (m_OwnerMOSR && callOnBounce) { halted = halted || m_OwnerMOSR->OnBounce(hitTerrAtom->GetHitData()); }

						// Copy back the new HitData with all the info we have so far.
						//hitTerrAtom->SetHitData(hitData);

						// Compute and store this Atom's collision response impulse force.
						hitTerrAtom->TerrHitResponse();
						hitResponseAtoms.push_back(hitTerrAtom);
					}
				}

				// TERRAIN SINK //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				// Handle terrain penetration effects.
				if (!penetratingAtoms.empty()) {
					const float hitFactor = 1.0F / static_cast<float>(penetratingAtoms.size());

					// Calculate and store the collision response effects.
					for (Atom *penetratingAtom : penetratingAtoms) {
						/*
						// This gets re-set later according to the ortho pixel edges hit.
						hitData.BitmapNormal = -(hitData.HitVel[HITOR].GetNormalized());
						hitData.SquaredMIHandle[HITOR] = hitData.HitRadius[HITOR].GetPerpendicular(); //.Dot(hitData.BitmapNormal);
						hitData.SquaredMIHandle[HITOR] *= hitData.SquaredMIHandle[HITOR];
						hitData.HitDenominator = (1.0 / massDistribution) + (hitData.SquaredMIHandle[HITOR] / momentInertiaDistribution);
						hitData.PreImpulse[HITOR] = hitData.HitVel[HITOR] / hitData.HitDenominator;
						*/

						// Get the HitData so far gathered for this Atom.
						hitData = penetratingAtom->GetHitData();

						if (g_SceneMan.TryPenetrate(penetratingAtom->GetCurrentPos().GetFloorIntX(), penetratingAtom->GetCurrentPos().GetFloorIntY(), hitData.PreImpulse[HITOR], hitData.HitVel[HITOR], retardation, 1.0F, 1/*(*penetratingAtom)->GetNumPenetrations()*/)) {
							// Recalculate these here without the distributed mass and MI.
							const float radMag = hitData.HitRadius[HITOR].GetMagnitude();
							hitData.HitDenominator = (1.0F / mass) + ((radMag * radMag) / m_MomentOfInertia);
							hitData.PreImpulse[HITOR] = hitData.HitVel[HITOR] / hitData.HitDenominator;
							hitData.TotalMass[HITOR] = mass;
							hitData.MomInertia[HITOR] = m_MomentOfInertia;
							hitData.ImpulseFactor[HITOR] = hitFactor;
							// Finally calculate the hit response impulse.
							hitData.ResImpulse[HITOR] = ((hitData.HitVel[HITOR] * retardation) / hitData.HitDenominator) * hitFactor;

							// Call the call-on-sink function, if requested.
							if (m_OwnerMOSR && callOnSink) { halted = halted || m_OwnerMOSR->OnSink(hitData); }

							// Copy back the new HitData with all the info we have so far.
							penetratingAtom->SetHitData(hitData);
							// Save the Atom for later application of its HitData to the body.
							hitResponseAtoms.push_back(penetratingAtom);
						}
					}
				}

				// MOVABLEOBJECT COLLISION RESPONSE /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				if (hitsMOs && !hitMOAtoms.empty()) {
					// Set the mass and other data pertaining to the hitor, aka this AtomGroup's owner MO.
					hitData.TotalMass[HITOR] = mass;
					hitData.MomInertia[HITOR] = m_MomentOfInertia;
					hitData.ImpulseFactor[HITOR] = 1.0F / static_cast<float>(atomsHitMOsCount);

					for (const map<MOID, std::list<Atom *>>::value_type &MOAtomMapEntry : hitMOAtoms) {
						// The denominator that the MovableObject being hit should divide its mass with for each Atom of this AtomGroup that is colliding with it during this step.
						hitData.ImpulseFactor[HITEE] = 1.0F / static_cast<float>(MOAtomMapEntry.second.size());

						for (Atom *hitMOAtom : MOAtomMapEntry.second) {
							// Step back all Atoms that hit MOs during this step iteration. This is so we aren't intersecting the hit MO anymore.
							hitMOAtom->StepBack();
							//hitData.HitPoint = hitMOAtom->GetCurrentPos();

							// Calculate and store the accurate hit radius of the Atom in relation to the CoM
							hitData.HitRadius[HITOR] = m_OwnerMOSR->RotateOffset(hitMOAtom->GetOffset()) * c_MPP;
							// Figure out the pre-collision velocity of the hitting Atom due to body translation and rotation.
							hitData.HitVel[HITOR] = velocity + hitData.HitRadius[HITOR].GetPerpendicular() * angularVel;
							// Set the Atom with the HitData with all the info we have so far.
							hitMOAtom->SetHitData(hitData);
							// Let the Atom calculate the impulse force resulting from the collision, and only add it if collision is valid
							if (hitMOAtom->MOHitResponse()) {
								// Report the hit to both MO's in collision
								HitData &hd = hitMOAtom->GetHitData();
								// Don't count collision if either says they got terminated
								if (!hd.RootBody[HITOR]->OnMOHit(hd) && !hd.RootBody[HITEE]->OnMOHit(hd)) {
									// Save the filled out Atom in the list for later application in this step.
									hitResponseAtoms.push_back(hitMOAtom);
								}
							}
						}
					}
				}
			}
			++segCount;

			// APPLY COLLISION RESPONSES /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			// If we hit anything, during this almost completed segment, and are about to start a new one, apply the calculated response effects to the owning MO.
			if (hitStep) {
				// Apply all the collision response impulse forces to the linear and angular velocities of the owner MO.
				for (Atom *hitResponseAtom : hitResponseAtoms) {
					// TODO: Investigate damping!
					hitData = hitResponseAtom->GetHitData();
					velocity += hitData.ResImpulse[HITOR] / mass;
					angularVel += hitData.HitRadius[HITOR].GetPerpendicular().Dot(hitData.ResImpulse[HITOR]) / m_MomentOfInertia;
					// Accumulate all the impulse forces so the MO can determine if it took damaged as a result
					totalImpulse += hitData.ResImpulse[HITOR];
				}

				// Make sub-pixel progress if there was a hit on the very first step.
				//if (segProgress == 0) { segProgress = 0.1 / (float)stepsOnSeg; }                 

				// Now calculate the total time left to travel, according to the progress made.
				timeLeft -= timeLeft * (segProgress * segRatio);
			} else {
				// If last completed segment didn't result in a hit, move things forward to reflect the progress made.
				segProgress = 1.0F;
				// Move position forward to the end segment position.
				position += linSegTraj * segProgress;
				didWrap = g_SceneMan.WrapPosition(position) || didWrap;
				// Move rotation forward according to the progress made on the segment.
				rotation += rotDelta * segProgress;
				// Now calculate the total time left to travel, according to the progress made.
				timeLeft -= timeLeft * (segProgress * segRatio);
			}

			if (hitCount > 10) {
				// RTEAbort("AtomGroup travel resulted in more than 1000 segments!!");
				break;
			}
		} while (segRatio != 1.0F || hitStep && /*!linSegTraj.GetFloored().IsZero() &&*/ !halted);

		ResolveMOSIntersection(position);

		//if (!scenePreLocked) { g_SceneMan.UnlockScene(); }

		ClearMOIDIgnoreList();

		// If too many Atoms are ignoring terrain, make a hole for the body so they won't
		int ignoreCount = 0;
		int maxIgnore = m_Atoms.size() / 2;

		for (const Atom *atom : m_Atoms) {
			if (atom->IsIgnoringTerrain()) {
				++ignoreCount;

				if (ignoreCount >= maxIgnore) {
					m_OwnerMOSR->ForceDeepCheck();
					break;
				}
			}
		}

		// Travel along the remaining trajectory if we didn't hit anything on the last segment and weren't told to halt.
		if (!hitStep && !halted) {
			/*
			// Move position forward the whole way.
			position += linSegTraj;
			// Wrap, if necessary.
			didWrap = g_SceneMan.WrapPosition(position) || didWrap;
			// Move rotation forward the whole way.
			rotation += rotDelta;
			*/
			return 0;
		}
		return timeLeft;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// TODO: Break down and rework this dumpsterfire.
	Vector AtomGroup::PushTravel(Vector &position, const Vector &velocity, float pushForce, bool &didWrap, float travelTime, bool callOnBounce, bool callOnSink, bool scenePreLocked) {
		RTEAssert(m_OwnerMOSR, "Tried to push-travel an AtomGroup that has no parent!");

		didWrap = false;

		int intPos[2];
		int hitPos[2];
		int delta[2];
		int delta2[2];
		int increment[2];
		bool hit[2];

		int legCount = 0;
		int stepCount = 0;
		int hitCount = 0;
		float timeLeft = travelTime;
		float mass = (m_OwnerMOSR->GetMass() != 0 ? m_OwnerMOSR->GetMass() : 0.0001F);
		float retardation;
		bool halted = false;

		// TODO: Fix HitMOs issue!!
		bool hitMOs = false	/*m_OwnerMOSR->m_HitsMOs*/;

		const Material *hitMaterial = nullptr;
		const Material *domMaterial = nullptr;
		const Material *subMaterial = nullptr;

		Vector legProgress;
		Vector forceVel;
		Vector returnPush;
		Vector ownerVel = velocity;
		Vector trajectory = ownerVel * travelTime * c_PPM; // Trajectory length in pixels.

		HitData hitData;

		std::map<MOID, std::set<Atom *>> MOIgnoreMap;
		std::map<MOID, std::deque<std::pair<Atom *, Vector>>> hitMOAtoms;
		std::deque<std::pair<Atom *, Vector>> hitTerrAtoms;
		std::deque<std::pair<Atom *, Vector>> penetratingAtoms;
		std::deque<std::pair<Vector, Vector>> impulseForces; // First Vector is the impulse force in kg * m/s, the second is force point, or its offset from the origin of the AtomGroup.

		// Lock all bitmaps involved outside the loop - only relevant for video bitmaps so disabled at the moment.
		//if (!scenePreLocked) { g_SceneMan.LockScene(); }

		// Before the very first step of the first leg of this travel, we find that we're already intersecting with another MO, then we completely ignore collisions with that MO for this entire travel.
		// This is to prevent MO's from getting stuck in each other.
		if (hitMOs) {
			intPos[X] = position.GetFloorIntX();
			intPos[Y] = position.GetFloorIntY();

			for (Atom *atom : m_Atoms) {
				const Vector flippedOffset = atom->GetOffset().GetXFlipped(m_OwnerMOSR->m_HFlipped);
				// See if the Atom is starting out on top of another MO
				MOID tempMOID = g_SceneMan.GetMOIDPixel(intPos[X] + flippedOffset.GetFloorIntX(), intPos[Y] + flippedOffset.GetFloorIntY());

				if (tempMOID != g_NoMOID) {
					// Make the appropriate entry in the MO-Atom interaction ignore map
					if (MOIgnoreMap.count(tempMOID) != 0) {
						// Found an entry for this MOID, so add the Atom entry to it
						MOIgnoreMap.at(tempMOID).insert(atom);
					} else {
						// There wasn't already an entry for this MOID, so create one and add the Atom to it.
						std::set<Atom *> newSet;
						newSet.insert(atom);
						MOIgnoreMap.insert({ tempMOID, newSet });
					}
				}
			}
		}

		// Loop for all the different straight legs (between bounces etc) that have to be traveled during the travelTime.
		do {
			intPos[X] = position.GetFloorIntX();
			intPos[Y] = position.GetFloorIntY();

			float prevTrajMag = trajectory.GetMagnitude();
			trajectory = ownerVel * timeLeft * c_PPM;

			const Vector nextPosition = position + trajectory;
			delta[X] = nextPosition.GetFloorIntX() - intPos[X];
			delta[Y] = nextPosition.GetFloorIntY() - intPos[Y];

			hit[X] = false;
			hit[Y] = false;

			if (delta[X] == 0 && delta[Y] == 0) {
				break;
			}

			hitTerrAtoms.clear();
			penetratingAtoms.clear();
			impulseForces.clear();

			int dom = 0;
			int sub = 0;
			int subSteps = 0;
			bool subStepped = false;

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
			delta2[X] = delta[X] * 2;
			delta2[Y] = delta[Y] * 2;

			// If X is dominant, Y is submissive, and vice versa.
			if (delta[X] > delta[Y]) {
				dom = X;
				sub = Y;
			} else {
				dom = Y;
				sub = X;
			}

			int error = delta2[sub] - delta[dom];

			if (delta[X] > 1000) { delta[X] = 1000; }
			if (delta[Y] > 1000) { delta[Y] = 1000; }

			// Bresenham's line drawing algorithm execution
			for (int domSteps = 0; domSteps < delta[dom] && !(hit[X] || hit[Y]); ++domSteps) {
				if (subStepped) { ++subSteps; }
				subStepped = false;

				// Take one step forward along the leg.
				intPos[dom] += increment[dom];
				if (error >= 0) {
					intPos[sub] += increment[sub];
					subStepped = true;
					error -= delta2[dom];
				}
				error += delta2[sub];

				didWrap = g_SceneMan.WrapPosition(intPos[X], intPos[Y]) || didWrap;

				// SCENE COLLISION DETECTION /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				int atomsHitMOsCount = 0;

				hitMOAtoms.clear();
				hitTerrAtoms.clear();

				for (Atom *atom : m_Atoms) {
					const Vector flippedOffset = atom->GetOffset().GetXFlipped(m_OwnerMOSR->m_HFlipped);
					MOID tempMOID = g_NoMOID;

					// First check if we hit any MO's, if applicable.
					bool ignoreHit = false;
					if (hitMOs) {
						tempMOID = g_SceneMan.GetMOIDPixel(intPos[X] + flippedOffset.GetFloorIntX(), intPos[Y] + flippedOffset.GetFloorIntY());
						// Check the ignore map for Atoms that should ignore hits against certain MOs.
						if (tempMOID != g_NoMOID && (MOIgnoreMap.count(tempMOID) != 0)) { ignoreHit = MOIgnoreMap.at(tempMOID).count(atom) != 0; }
					}

					if (hitMOs && tempMOID && !ignoreHit) {
						// See if we already have another Atom hitting this MO in this step. If not, then create a new deque unique for that MO's ID and insert into the map of MO-hitting Atoms.
						if (hitMOAtoms.count(tempMOID) == 0) {
							std::deque<std::pair<Atom *, Vector>> newDeque;
							newDeque.push_back({ atom, flippedOffset });
							hitMOAtoms.insert({ tempMOID, newDeque });
						} else {
							// If another Atom of this group has already hit this same MO during this step, go ahead and add the new Atom to the corresponding deque for that MOID.
							hitMOAtoms.at(tempMOID).push_back({ atom, flippedOffset });
						}
						// Count the number of Atoms of this group that hit MOs this step. Used to properly distribute the mass of the owner MO in later collision responses during this step.
						atomsHitMOsCount++;
					// If no MO has ever been hit yet during this step, then keep checking for terrain hits.
					} else if (atomsHitMOsCount == 0 && g_SceneMan.GetTerrMatter(intPos[X] + flippedOffset.GetFloorIntX(), intPos[Y] + flippedOffset.GetFloorIntY())) {
						hitTerrAtoms.push_back({ atom, flippedOffset });
					}

#ifdef DEBUG_BUILD
					// TODO: Remove this once AtomGroup drawing in Material layer draw mode is implemented.
					// Draw the positions of the hit points on screen for easy debugging.
					//putpixel(g_SceneMan.GetMOColorBitmap(), std::floor(position.GetFloorIntX() + flippedOffset.GetFloorIntX()), std::floor(position.GetFloorIntY() + flippedOffset.GetFloorIntY()), 122);
#endif
				}

				// If no collisions, continue on to the next step.
				if (hitTerrAtoms.empty() && hitMOAtoms.empty()) {
					continue;
				}

				// There are colliding Atoms, therefore the group hit something.
				hitPos[X] = intPos[X];
				hitPos[Y] = intPos[Y];
				++hitCount;

				// Calculate the progress made on this leg before hitting something.
				legProgress[dom] = static_cast<float>(domSteps * increment[dom]);
				legProgress[sub] = static_cast<float>(subSteps * increment[sub]);

				// Now calculate the total time left to travel, according to the progress made.
				timeLeft *= (trajectory.GetMagnitude() - legProgress.GetMagnitude()) / prevTrajMag;

				// The capped velocity used for the push calculations. a = F / m
				forceVel = Vector(ownerVel).CapMagnitude((pushForce * timeLeft) / mass);

				// MOVABLEOBJECT COLLISION RESPONSE /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				if (hitMOs && !hitMOAtoms.empty()) {
					// Back up one step so that we're not intersecting the other MO(s) anymore
					intPos[dom] -= increment[dom];
					if (subStepped) { intPos[sub] -= increment[sub]; }
					// Undo wrap, if necessary.
					didWrap = !g_SceneMan.WrapPosition(intPos[X], intPos[Y]) && didWrap;

					// Set the mass and other data pertaining to the hitor, aka this AtomGroup's owner MO.
					hitData.TotalMass[HITOR] = mass;
					hitData.MomInertia[HITOR] = 1.0F;
					hitData.ImpulseFactor[HITOR] = 1.0F / static_cast<float>(atomsHitMOsCount);
					// Figure out the pre-collision velocity of the hitting Atoms due to the max push force allowed.
					hitData.HitVel[HITOR] = forceVel;

					// The distributed mass of one hitting Atom of the hitting (this AtomGroup's owner) MovableObject.
					//float hitorMass = mass / ((atomsHitMOsCount/* + hitTerrAtoms.size()*/) * (m_Resolution ? m_Resolution : 1));
					//float hiteeMassDenom = 0;

					for (const std::map<MOID, std::deque<std::pair<Atom *, Vector>>>::value_type &MOAtomMapEntry : hitMOAtoms) {
						// The denominator that the MovableObject being hit should divide its mass with for each Atom of this AtomGroup that is colliding with it during this step.
						hitData.ImpulseFactor[HITEE] = 1.0F / static_cast<float>(MOAtomMapEntry.second.size());

						for (const std::pair<Atom *, Vector> &hitMOAtomEntry : MOAtomMapEntry.second) {
							// Bake in current Atom's offset into the int positions.
							const Vector &atomOffset = hitMOAtomEntry.second;
							intPos[X] += atomOffset.GetFloorIntX();
							intPos[Y] += atomOffset.GetFloorIntY();
							hitPos[X] += atomOffset.GetFloorIntX();
							hitPos[Y] += atomOffset.GetFloorIntY();

							// Calculate and store the accurate hit radius of the Atom in relation to the CoM
							hitData.HitRadius[HITOR] = atomOffset * c_MPP;
							hitData.HitPoint.Reset();
							hitData.BitmapNormal.Reset();

							// Check for the collision point in the dominant direction of travel.
							if (delta[dom] && ((dom == X && g_SceneMan.GetMOIDPixel(hitPos[X], intPos[Y]) != g_NoMOID) || (dom == Y && g_SceneMan.GetMOIDPixel(intPos[X], hitPos[Y]) != g_NoMOID))) {
								hit[dom] = true;
								hitData.HitPoint = (dom == X) ? Vector(static_cast<float>(hitPos[X]), static_cast<float>(intPos[Y])) : Vector(static_cast<float>(intPos[X]), static_cast<float>(hitPos[Y]));
								hitData.BitmapNormal[dom] = static_cast<float>(-increment[dom]);
							}

							// Check for the collision point in the submissive direction of travel.
							if (subStepped && delta[sub] && ((sub == X && g_SceneMan.GetMOIDPixel(hitPos[X], intPos[Y]) != g_NoMOID) || (sub == Y && g_SceneMan.GetMOIDPixel(intPos[X], hitPos[Y]) != g_NoMOID))) {
								hit[sub] = true;

								//if (hitData.HitPoint.IsZero()) {
									// NOTE: THis can actually be wrong since there may not in fact be a corner pixel, but two pixels hit on X and Y directions
									hitData.HitPoint = (sub == X) ? Vector(static_cast<float>(hitPos[X]), static_cast<float>(intPos[Y])) : Vector(static_cast<float>(intPos[X]), static_cast<float>(hitPos[Y]));
								/*
								// We hit pixels in both sub and dom directions on the other MO, a corner hit.
								} else {
									hitData.HitPoint.SetXY(hitPos[X], hitPos[Y]);
								}
								*/
								hitData.BitmapNormal[sub] = static_cast<float>(-increment[sub]);
							}

							// If neither the direct dominant or sub directions yielded a collision point, then that means we hit right on the corner of a pixel, and that is the collision point.
							if (!hit[dom] && !hit[sub]) {
								hit[dom] = hit[sub] = true;
								hitData.HitPoint.SetXY(static_cast<float>(hitPos[X]), static_cast<float>(hitPos[Y]));
								hitData.BitmapNormal.SetXY(static_cast<float>(-increment[X]), static_cast<float>(-increment[Y]));
							}
							hitData.BitmapNormal.Normalize();

							// Extract the current Atom's offset from the int positions.
							intPos[X] -= atomOffset.GetFloorIntX();
							intPos[Y] -= atomOffset.GetFloorIntY();
							hitPos[X] -= atomOffset.GetFloorIntX();
							hitPos[Y] -= atomOffset.GetFloorIntY();

							MOID hitMOID = g_SceneMan.GetMOIDPixel(hitData.HitPoint.GetFloorIntX(), hitData.HitPoint.GetFloorIntY());

							if (hitMOID != g_NoMOID) {
								hitData.Body[HITOR] = m_OwnerMOSR;
								hitData.Body[HITEE] = g_MovableMan.GetMOFromID(hitMOID);

#ifndef RELEASE_BUILD
								RTEAssert(hitData.Body[HITEE], "Hitee MO is 0 in AtomGroup::PushTravel!");
#endif

								hitData.Body[HITEE]->CollideAtPoint(hitData);

								// Save the impulse force resulting from the MO collision response calculation.
								impulseForces.push_back(make_pair(hitData.ResImpulse[HITOR], atomOffset));
							}
						}
					}
					// If any MOs were hit, continue on to the next leg without doing terrain stuff now.
					// Any terrain collisions will be taken care of on the next leg, when the effects of these MO hits have been applied.
					hitTerrAtoms.clear();
				}

				// TERRAIN COLLISION RESPONSE /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				bool somethingPenetrated = false;
				float massDist = 0;

				// Determine which of the colliding Atoms will penetrate the terrain.
				do {
					somethingPenetrated = false;

					massDist = mass / static_cast<float>(hitTerrAtoms.size() * (m_Resolution ? m_Resolution : 1));

					for (std::deque<std::pair<Atom *, Vector>>::iterator atomItr = hitTerrAtoms.begin(); atomItr != hitTerrAtoms.end(); ) {
						if (g_SceneMan.WillPenetrate(intPos[X] + (*atomItr).second.GetFloorIntX(), intPos[Y] + (*atomItr).second.GetFloorIntY(), forceVel, massDist)) {
							// Move the penetrating Atom to the penetrating list from the collision list.
							penetratingAtoms.push_back({ (*atomItr).first, (*atomItr).second });
							atomItr = hitTerrAtoms.erase(atomItr);
							somethingPenetrated = true;
						} else {
							++atomItr;
						}
					}
				} while (!hitTerrAtoms.empty() && somethingPenetrated);

				// TERRAIN BOUNCE /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				// If some Atoms could not penetrate even though all the mass was on them, gather the bounce results and apply them to the owner.
				if (!hitTerrAtoms.empty()) {
					// Back up one step so that we're not intersecting the terrain anymore
					intPos[dom] -= increment[dom];
					if (subStepped) { intPos[sub] -= increment[sub]; }

					// Undo wrap, if necessary.
					didWrap = !g_SceneMan.WrapPosition(intPos[X], intPos[Y]) && didWrap;

					// Call the call-on-bounce function, if requested.
					//if (m_OwnerMOSR && callOnBounce) { halted = m_OwnerMOSR->OnBounce(position); }

					// Calculate the distributed mass that each bouncing Atom has.
					massDist = mass / static_cast<float>((hitTerrAtoms.size()/* + atomsHitMOsCount*/) * (m_Resolution ? m_Resolution : 1));

					// Gather the collision response effects so that the impulse force can be calculated.
					for (const std::pair<Atom *, Vector> &hitTerrAtomsEntry : hitTerrAtoms) {
						// Bake in current Atom's offset into the int positions.
						const Vector &atomOffset = hitTerrAtomsEntry.second;
						intPos[X] += atomOffset.GetFloorIntX();
						intPos[Y] += atomOffset.GetFloorIntY();
						hitPos[X] += atomOffset.GetFloorIntX();
						hitPos[Y] += atomOffset.GetFloorIntY();

						Vector newVel = forceVel;

						unsigned char hitMaterialID = g_SceneMan.GetTerrMatter(hitPos[X], hitPos[Y]);
						hitMaterial = g_SceneMan.GetMaterialFromID(hitMaterialID);

						// Check for and react upon a collision in the dominant direction of travel.
						if (delta[dom] && ((dom == X && g_SceneMan.GetTerrMatter(hitPos[X], intPos[Y])) || (dom == Y && g_SceneMan.GetTerrMatter(intPos[X], hitPos[Y])))) {
							hit[dom] = true;
							unsigned char domMaterialID = (dom == X) ? g_SceneMan.GetTerrMatter(hitPos[X], intPos[Y]) : g_SceneMan.GetTerrMatter(intPos[X], hitPos[Y]);
							domMaterial = g_SceneMan.GetMaterialFromID(domMaterialID);

							// Bounce according to the collision.
							newVel[dom] = -newVel[dom] * hitTerrAtomsEntry.first->GetMaterial()->GetRestitution() * domMaterial->GetRestitution();
						}

						// Check for and react upon a collision in the submissive direction of travel.
						if (subStepped && delta[sub] && ((sub == X && g_SceneMan.GetTerrMatter(hitPos[X], intPos[Y])) || (sub == Y && g_SceneMan.GetTerrMatter(intPos[X], hitPos[Y])))) {
							hit[sub] = true;
							unsigned char subMaterialID = (sub == X) ? g_SceneMan.GetTerrMatter(hitPos[X], intPos[Y]) : g_SceneMan.GetTerrMatter(intPos[X], hitPos[Y]);
							subMaterial = g_SceneMan.GetMaterialFromID(subMaterialID);

							// Bounce according to the collision.
							newVel[sub] = -newVel[sub] * hitTerrAtomsEntry.first->GetMaterial()->GetRestitution() * subMaterial->GetRestitution();
						}

						// If hit right on the corner of a pixel, bounce straight back with no friction.
						if (!hit[dom] && !hit[sub]) {
							hit[dom] = true;
							newVel[dom] = -newVel[dom] * hitTerrAtomsEntry.first->GetMaterial()->GetRestitution() * hitMaterial->GetRestitution();
							hit[sub] = true;
							newVel[sub] = -newVel[sub] * hitTerrAtomsEntry.first->GetMaterial()->GetRestitution() * hitMaterial->GetRestitution();
						} else if (hit[dom] && !hit[sub]) {
							newVel[sub] -= newVel[sub] * hitTerrAtomsEntry.first->GetMaterial()->GetFriction() * domMaterial->GetFriction();
						} else if (hit[sub] && !hit[dom]) {
							newVel[dom] -= newVel[dom] * hitTerrAtomsEntry.first->GetMaterial()->GetFriction() * subMaterial->GetFriction();
						}

						// Compute and store this Atom's collision response impulse force.
						impulseForces.push_back({ (newVel - forceVel) * massDist, atomOffset });

						// Extract the current Atom's offset from the int positions.
						intPos[X] -= atomOffset.GetFloorIntX();
						intPos[Y] -= atomOffset.GetFloorIntY();
						hitPos[X] -= atomOffset.GetFloorIntX();
						hitPos[Y] -= atomOffset.GetFloorIntY();
					}
				}

				// TERRAIN SINK /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				// All Atoms must have penetrated and therefore the entire group has sunken into the terrain. Get the penetration resistance results and apply them to the owner.
				else if (!penetratingAtoms.empty()) {
					//bool sinkHit = true;
					hit[dom] = true;
					hit[sub] = true;

					// Call the call-on-sink function, if requested.
					//if (m_OwnerMOSR && callOnSink) { halted = m_OwnerMOSR->OnSink(position); }

					massDist = mass / static_cast<float>(penetratingAtoms.size() * (m_Resolution ? m_Resolution : 1));

					// Apply the collision response effects.
					for (const std::pair<Atom *, Vector> &penetratingAtomsEntry : penetratingAtoms) {
						if (g_SceneMan.TryPenetrate(intPos[X] + penetratingAtomsEntry.second.GetFloorIntX(), intPos[Y] + penetratingAtomsEntry.second.GetFloorIntY(), forceVel * massDist, forceVel, retardation, 1.0F, penetratingAtomsEntry.first->GetNumPenetrations())) {
							impulseForces.push_back({ forceVel * massDist * retardation, penetratingAtomsEntry.second });
						}
					}
				}

				// APPLY COLLISION RESPONSES /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				// If we hit anything, and are about to start a new leg instead of a step, apply the averaged collision response effects to the owning MO.
				if (hit[X] || hit[Y]) {
					// Move position forward to the hit position.
					position += legProgress;
					didWrap = didWrap || g_SceneMan.WrapPosition(position);

					// Apply velocity averages to the final resulting velocity for this leg.
					for (const std::pair<Vector, Vector> &impulseForcesEntry : impulseForces) {
						// Cap the impulse to what the max push force is
						//impulseForcesEntry.first.CapMagnitude(pushForce * (travelTime/* - timeLeft*/));
						ownerVel += impulseForcesEntry.first / mass;
						returnPush += impulseForcesEntry.first;
					}
					// Stunt travel time if there is no more velocity
					if (ownerVel.IsZero()) { timeLeft = 0; }
				}
				++stepCount;
			}
			++legCount;
		} while ((hit[X] || hit[Y]) && timeLeft > 0.0F && /*!trajectory.GetFloored().IsZero() &&*/ !halted && hitCount < 3);

		//if (!scenePreLocked) { g_SceneMan.UnlockScene(); }

		// Travel along the remaining trajectory.
		if (!(hit[X] || hit[Y]) && !halted) {
			position += trajectory;
			didWrap = g_SceneMan.WrapPosition(position) || didWrap;
			return returnPush;
		}
		return returnPush;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AtomGroup::PushAsLimb(const Vector &jointPos, const Vector &velocity, const Matrix &rotation, LimbPath &limbPath, const float travelTime, bool *restarted, bool affectRotation) {
		RTEAssert(m_OwnerMOSR, "Tried to push-as-limb an AtomGroup that has no parent!");

		bool didWrap = false;
		Vector pushImpulse;

		limbPath.SetJointPos(jointPos);
		limbPath.SetJointVel(velocity);
		limbPath.SetRotation(rotation);
		limbPath.SetFrameTime(travelTime);

		const Vector limbDist = g_SceneMan.ShortestDistance(jointPos, m_LimbPos);

		// Restart the path if the limb strayed off the path.
		if (limbDist.GetMagnitude() > m_OwnerMOSR->GetDiameter()) { limbPath.Terminate(); }

		// TODO: Change this to a regular while loop if possible.
		do {
			if (limbPath.PathEnded()) {
				if (restarted) { *restarted = true; }
				if (!limbPath.RestartFree(m_LimbPos, m_OwnerMOSR->GetRootID(), m_OwnerMOSR->IgnoresWhichTeam())) {
					return false;
				}
			}
			pushImpulse = PushTravel(m_LimbPos, limbPath.GetCurrentVel(m_LimbPos), limbPath.GetPushForce(), didWrap, limbPath.GetNextTimeChunk(m_LimbPos), false, false);
			limbPath.ReportProgress(m_LimbPos);
		} while (!limbPath.FrameDone() && !limbPath.PathEnded());

		if (pushImpulse.GetLargest() > 10000.0F) { pushImpulse.Reset(); }

		m_OwnerMOSR->AddImpulseForce(pushImpulse, affectRotation ? (g_SceneMan.ShortestDistance(m_OwnerMOSR->GetPos(), jointPos) * c_MPP) : Vector());

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AtomGroup::FlailAsLimb(const Vector &ownerPos, const Vector &jointOffset, const float limbRadius, const Vector &velocity, const float angularVel, const float mass, const float travelTime) {
		RTEAssert(m_OwnerMOSR, "Tried to flail an AtomGroup that has no parent!");

		bool didWrap = false;
		Vector jointPos = ownerPos + jointOffset;
		Vector centrifugalVel = jointOffset * std::fabs(angularVel);

		Vector pushImpulse = PushTravel(m_LimbPos, velocity + centrifugalVel, 100, didWrap, travelTime, false, false, false);

		Vector limbRange = m_LimbPos - jointPos;

		if (limbRange.GetMagnitude() > limbRadius) {
			limbRange.SetMagnitude(limbRadius);
			m_LimbPos = jointPos + limbRange;
		}
		return;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AtomGroup::InTerrain() const {
		RTEAssert(m_OwnerMOSR, "Tried to check overlap with terrain for an AtomGroup that has no parent!");

		// Only relevant for video bitmaps so disabled at the moment.
		//if (!g_SceneMan.SceneIsLocked()) { g_SceneMan.LockScene(); }

		bool penetrates = false;
		Vector atomPos;

		for (const Atom *atom : m_Atoms) {
			atomPos = m_OwnerMOSR->GetPos() + GetAdjustedAtomOffset(atom);
			if (g_SceneMan.GetTerrMatter(atomPos.GetFloorIntX(), atomPos.GetFloorIntY()) != g_MaterialAir) {
				penetrates = true;
				break;
			}
		}

		//if (g_SceneMan.SceneIsLocked()) { g_SceneMan.UnlockScene(); }

		return penetrates;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float AtomGroup::RatioInTerrain() const {
		RTEAssert(m_OwnerMOSR, "Tried to check ratio in terrain for an AtomGroup that has no parent!");

		// Only relevant for video bitmaps so disabled at the moment.
		//if (!g_SceneMan.SceneIsLocked()) { g_SceneMan.LockScene(); }

		Vector atomPos;
		int inTerrain = 0;

		for (const Atom *atom : m_Atoms) {
			atomPos = m_OwnerMOSR->GetPos() + GetAdjustedAtomOffset(atom);
			if (g_SceneMan.GetTerrMatter(atomPos.GetFloorIntX(), atomPos.GetFloorIntY()) != g_MaterialAir) { inTerrain++; }
		}

		//if (g_SceneMan.SceneIsLocked()) { g_SceneMan.UnlockScene(); }

		return static_cast<float>(inTerrain) / static_cast<float>(m_Atoms.size());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// TODO: Look into breaking this into smaller methods.
	bool AtomGroup::ResolveTerrainIntersection(Vector &position, unsigned char strongerThan) const {
		std::list<Atom *> intersectingAtoms;
		MOID hitMaterial = g_MaterialAir;

		float strengthThreshold = (strongerThan != g_MaterialAir) ? g_SceneMan.GetMaterialFromID(strongerThan)->GetIntegrity() : 0.0F;

		Vector atomOffset = Vector();
		Vector atomPos = Vector();

		// First go through all Atoms to find the first intersection and get the intersected MO
		for (Atom *atom : m_Atoms) {
			atomOffset = m_OwnerMOSR->RotateOffset(atom->GetOffset());
			atom->SetupPos(position + atomOffset);
			atomPos = atom->GetCurrentPos();
			hitMaterial = g_SceneMan.GetTerrain()->GetPixel(atomPos.GetFloorIntX(), atomPos.GetFloorIntY());
			if (hitMaterial != g_MaterialAir && strengthThreshold > 0.0F && g_SceneMan.GetMaterialFromID(hitMaterial)->GetIntegrity() > strengthThreshold) {
				intersectingAtoms.push_back(atom);
			}
		}
		if (intersectingAtoms.empty()) {
			return true;
		}
		if (intersectingAtoms.size() >= m_Atoms.size()) {
			return false;
		}

		Vector exitDirection = Vector();

		// Go through all intersecting Atoms and find their average inverse normal
		for (const Atom *intersectingAtom : intersectingAtoms) {
			exitDirection += m_OwnerMOSR->RotateOffset(intersectingAtom->GetNormal());
		}

		// TODO: Maybe use previous position to create an exit direction instead of quitting.
		if (exitDirection.IsZero()) {
			return false;
		}

		exitDirection = -exitDirection;
		exitDirection.SetMagnitude(m_OwnerMOSR->GetDiameter());

		// See which of the intersecting Atoms has the longest to travel along the exit direction before it clears
		float longestDistance = 0.0F;

		Vector clearPos = Vector();
		Vector atomExitVector = Vector();
		Vector totalExitVector = Vector();

		for (const Atom *intersectingAtom : intersectingAtoms) {
			bool rayHit = false;
			atomPos = intersectingAtom->GetCurrentPos();

			if (strengthThreshold <= 0.0F) {
				rayHit = g_SceneMan.CastMaterialRay(atomPos, exitDirection, g_MaterialAir, clearPos, 0, false);
			} else {
				rayHit = g_SceneMan.CastWeaknessRay(atomPos, exitDirection, strengthThreshold, clearPos, 0, false);
			}

			if (rayHit) {
				atomExitVector = clearPos - atomPos;
				if (atomExitVector.GetMagnitude() > longestDistance) {
					// We found the Atom with the longest to travel along the exit direction to clear, so that's the distance to move the whole object to clear all its Atoms.
					longestDistance = atomExitVector.GetMagnitude();
					totalExitVector = atomExitVector;
				}
			}
		}

		// If the exit vector is too large, then avoid the jarring jump and report that we didn't make it out
		if (totalExitVector.GetMagnitude() > m_OwnerMOSR->GetIndividualRadius()) {
			return false;
		}

		position += totalExitVector;

		// TODO: Figure out if a check for clearness after moving the position is actually needed and add one so this return is accurate.
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// TODO: Look into breaking this into smaller methods.
	bool AtomGroup::ResolveMOSIntersection(Vector &position) {
		if (!m_OwnerMOSR->m_HitsMOs) {
			return true;
		}

		MovableObject *intersectedMO = nullptr;
		MOID currentMOID = g_NoMOID;
		MOID hitMOID = g_NoMOID;

		Vector atomOffset = Vector();
		Vector atomPos = Vector();

		// First go through all Atoms to find the first intersection and get the intersected MO
		for (Atom *atom : m_Atoms) {
			atomOffset = m_OwnerMOSR->RotateOffset(atom->GetOffset());
			atom->SetupPos(position + atomOffset);
			atomPos = atom->GetCurrentPos();
			hitMOID = g_SceneMan.GetMOIDPixel(atomPos.GetFloorIntX(), atomPos.GetFloorIntY());

			if (hitMOID != g_NoMOID && !atom->IsIgnoringMOID(hitMOID)) {
				// Save the correct MOID to search for other atom intersections with
				currentMOID = hitMOID;

				// Get the MO we seem to be intersecting
				MovableObject *tempMO = g_MovableMan.GetMOFromID(hitMOID);
				RTEAssert(tempMO, "Intersected MOID couldn't be translated to a real MO!");
				tempMO = tempMO->GetRootParent();

				if (tempMO->GetsHitByMOs()) {
					// Make that MO draw itself again in the MOID layer so we can find its true edges
					intersectedMO = tempMO;
					intersectedMO->Draw(g_SceneMan.GetMOIDBitmap(), Vector(), g_DrawMOID, true);
					break;
				}
			}
		}
		if (!intersectedMO) {
			return false;
		}
		if (m_OwnerMOSR->OnMOHit(intersectedMO)) {
			return false;
		}
		if (intersectedMO->OnMOHit(m_OwnerMOSR->GetRootParent())) {
			return false;
		}

		std::list<Atom *> intersectingAtoms;

		// Restart and go through all Atoms to find all intersecting the specific intersected MO
		for (Atom *atom : m_Atoms) {
			atomPos = atom->GetCurrentPos();
			if (g_SceneMan.GetMOIDPixel(atomPos.GetFloorIntX(), atomPos.GetFloorIntY()) == currentMOID) {
				// Add atom to list of intersecting ones
				intersectingAtoms.push_back(atom);
			}
		}

		Vector exitDirection = Vector();
		Vector atomExitVector = Vector();
		Vector totalExitVector = Vector();

		// Go through all intersecting Atoms and find their average inverse normal
		for (const Atom *intersectingAtom : intersectingAtoms) {
			exitDirection += m_OwnerMOSR->RotateOffset(intersectingAtom->GetNormal());
		}

		// TODO: Maybe use previous position to create an exit direction instead of quitting.
		if (exitDirection.IsZero()) {
			return false;
		}

		exitDirection = -exitDirection;
		exitDirection.SetMagnitude(m_OwnerMOSR->GetDiameter());

		Vector clearPos = Vector();

		// See which of the intersecting Atoms has the longest to travel along the exit direction before it clears
		float longestDistance = 0.0F;
		for (const Atom *intersectingAtom : intersectingAtoms) {
			atomPos = intersectingAtom->GetCurrentPos();
			if (g_SceneMan.CastFindMORay(atomPos, exitDirection, g_NoMOID, clearPos, 0, true, 0)) {
				atomExitVector = clearPos - atomPos.GetFloored();
				if (atomExitVector.GetMagnitude() > longestDistance) {
					// We found the Atom with the longest to travel along the exit direction to clear, so that's the distance to move the whole object to clear all its Atoms.
					longestDistance = atomExitVector.GetMagnitude();
					totalExitVector = atomExitVector;
				}
			}
		}

		Vector thisExit;
		Vector intersectedExit;

		// If the other object is pinned, then only move this. Otherwise, apply the object exit vector to both this owner object, and the one it intersected, proportional to their masses.
		if (intersectedMO->GetPinStrength() > 0.0F) {
			thisExit = totalExitVector;
		} else {
			float massA = (m_OwnerMOSR->GetMass() != 0 ? m_OwnerMOSR->GetMass() : 0.0001F);
			float massB = (intersectedMO->GetMass() != 0 ? intersectedMO->GetMass() : 0.0001F);
			float invMassA = 1.0F / massA;
			float invMassB = 1.0F / massB;
			float normMassA = invMassA / (invMassA + invMassB);
			float normMassB = invMassB / (invMassA + invMassB);

			//TODO investigate whether we should apply some (relatively small) amount of movement to the object even if it's a lot heavier, for more realistic physics
			// If the intersected is much larger than this' MO, then only move this. Otherwise, apply the movements to both this and the intersected MO's, proportional to their respective masses.
			if (normMassB < 0.33F) {
				thisExit = totalExitVector;
			} else {
				thisExit = totalExitVector * normMassA;
				intersectedExit = -totalExitVector * normMassB;
			}
		}

		// Now actually apply the exit vectors to both, but only if the jump isn't too jarring
		if (thisExit.GetMagnitude() < m_OwnerMOSR->GetIndividualRadius()) { position += thisExit; }
		if (!intersectedExit.IsZero() && intersectedExit.GetMagnitude() < intersectedMO->GetRadius()) { intersectedMO->SetPos(intersectedMO->GetPos() + intersectedExit); }

		if (m_OwnerMOSR->CanBeSquished() && RatioInTerrain() > 0.75F) /* && totalExitVector.GetMagnitude() > m_OwnerMOSR->GetDiameter()) */ {
			// Move back before gibbing so gibs don't end up inside terrain
			position -= thisExit;
			m_OwnerMOSR->GibThis(-totalExitVector);
		}

		MOSRotating *intersectedMOS = dynamic_cast<MOSRotating *>(intersectedMO);

		if (intersectedMOS && intersectedMOS->CanBeSquished() && intersectedMOS->GetAtomGroup()->RatioInTerrain() > 0.75F) /* && totalExitVector.GetMagnitude() > intersectedMO->GetDiameter()) */ {
			// Move back before gibbing so gibs don't end up inside terrain
			intersectedMO->SetPos(intersectedMO->GetPos() - intersectedExit);
			intersectedMOS->GibThis(totalExitVector);
		}

		// TODO: Figure out if a check for clearness after moving the position is actually needed and add one so this return is accurate.
		return intersectingAtoms.empty();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AtomGroup::Draw(BITMAP *targetBitmap, const Vector &targetPos, bool useLimbPos, unsigned char color) const {
		Vector atomPos;
		Vector normal;

		acquire_bitmap(targetBitmap);

		for (const Atom *atom : m_Atoms) {
			if (!useLimbPos) {
				atomPos = m_OwnerMOSR->GetPos() + GetAdjustedAtomOffset(atom);
			} else {
				atomPos = m_LimbPos + GetAdjustedAtomOffset(atom);
			}
			if (!atom->GetNormal().IsZero()) {
				normal = atom->GetNormal().GetXFlipped(m_OwnerMOSR->m_HFlipped) * 5;
				line(targetBitmap, atomPos.GetFloorIntX() - targetPos.GetFloorIntX(), atomPos.GetFloorIntY() - targetPos.GetFloorIntY(), atomPos.GetFloorIntX() - targetPos.GetFloorIntX(), atomPos.GetFloorIntY() - targetPos.GetFloorIntY(), 244);
			}
			putpixel(targetBitmap, atomPos.GetFloorIntX() - targetPos.GetFloorIntX(), atomPos.GetFloorIntY() - targetPos.GetFloorIntY(), color);
		}
		release_bitmap(targetBitmap);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// TODO: dan pls.
	void AtomGroup::GenerateAtomGroup(MOSRotating *ownerMOSRotating) {
		BITMAP *refSprite = ownerMOSRotating->GetSpriteFrame();
		const Vector spriteOffset = ownerMOSRotating->GetSpriteOffset();
		const int spriteWidth = refSprite->w * static_cast<int>(m_OwnerMOSR->GetScale());
		const int spriteHeight = refSprite->h * static_cast<int>(m_OwnerMOSR->GetScale());

		// Only try to generate AtomGroup if scaled width and height are > 0 as we're playing with fire trying to create 0x0 bitmap. 
		if (spriteWidth > 0 && spriteHeight > 0) {
			int x;
			int y;
			bool inside;

			BITMAP *checkBitmap = create_bitmap_ex(8, spriteWidth, spriteHeight);
			clear_to_color(checkBitmap, g_MaskColor);

			acquire_bitmap(refSprite);
			acquire_bitmap(checkBitmap);

			// If Atoms are to be placed right at (below) the bitmap of the sprite.
			if (m_Depth <= 0) {
				// First scan horizontally, and place Atoms on outer silhouette edges.
				for (y = 0; y < spriteHeight; y += m_Resolution) {
					// Scan LEFT to RIGHT, placing one Atom on each first encountered silhouette edge
					for (x = 0; x < spriteWidth; ++x) {
						// Detect if we are crossing a silhouette boundary.
						if (getpixel(refSprite, x, y) != g_MaskColor) {
							// Mark that an Atom has been put in this location, to avoid duplicate Atoms
							putpixel(checkBitmap, x, y, 99);
							AddAtomToGroup(ownerMOSRotating, spriteOffset, x, y, true);
							break;
						}
					}
					// Scan RIGHT to LEFT, placing one Atom on each first encountered silhouette edge
					for (x = spriteWidth - 1; x >= 0; --x) {
						// Detect if we are crossing a silhouette boundary.
						if (getpixel(refSprite, x, y) != g_MaskColor && getpixel(checkBitmap, x, y) == g_MaskColor) {
							// Mark that an Atom has been put in this location, to avoid duplicate Atoms
							putpixel(checkBitmap, x, y, 99);
							AddAtomToGroup(ownerMOSRotating, spriteOffset, x, y, true);
							break;
						}
					}
				}

				// Then scan vertically, and place Atoms on silhouette edge, but avoiding duplicates.
				for (x = 0; x < spriteWidth; x += m_Resolution) {
					// Scan TOP to BOTTOM, placing one Atom on each first encountered silhouette edge
					for (y = 0; y < spriteHeight; ++y) {
						// Detect if we are crossing a silhouette boundary, but make sure Atom wasn't already placed during the horizontal scans.
						if (getpixel(refSprite, x, y) != g_MaskColor && getpixel(checkBitmap, x, y) == g_MaskColor) {
							putpixel(checkBitmap, x, y, 99);
							AddAtomToGroup(ownerMOSRotating, spriteOffset, x, y, true);
							break;
						}
					}
					// Scan BOTTOM to TOP, placing one Atom on each first encountered silhouette edge
					for (y = spriteHeight - 1; y >= 0; --y) {
						// Detect if we are crossing a silhouette boundary, but make sure Atom wasn't already placed during the horizontal scans.
						if (getpixel(refSprite, x, y) != g_MaskColor && getpixel(checkBitmap, x, y) == g_MaskColor) {
							AddAtomToGroup(ownerMOSRotating, spriteOffset, x, y, true);
							break;
						}
					}
				}
			} else {
				// Atoms are to be placed at a depth into the bitmap
				int depthCount = 0;
				bool clear = true;

				// First scan HORIZONTALLY from LEFT to RIGHT and place Atoms in depth beyond the silhouette edge.
				for (y = 0; y < spriteHeight; y += m_Resolution) {
					inside = false;
					for (x = 0; x < spriteWidth; ++x) {
						// Detect if we are crossing a silhouette boundary.
						if ((getpixel(refSprite, x, y) != g_MaskColor && !inside) || (getpixel(refSprite, x, y) == g_MaskColor && inside)) {
							// Reset the depth counter
							depthCount = 0;
							inside = !inside;
						} else if (inside) {
							// Increment the depth counter if inside silhouette and propose making Atom if specified depth is reached.
							++depthCount;
							if (depthCount == m_Depth) {
								clear = true;
								// Check whether depth is sufficient in the other cardinal directions.
								for (int i = 1; i <= m_Depth && clear; ++i) {
									if (x + i >= refSprite->w || y + i >= refSprite->h || y - i < 0 || getpixel(refSprite, x + i, y) == g_MaskColor || getpixel(refSprite, x, y + i) == g_MaskColor || getpixel(refSprite, x, y - i) == g_MaskColor) {
										clear = false;
									}
								}
								if (clear && getpixel(checkBitmap, x, y) == g_MaskColor) {
									// Mark that an Atom has been put in this location, to avoid duplicate Atoms.
									putpixel(checkBitmap, x, y, 99);
									AddAtomToGroup(ownerMOSRotating, spriteOffset, x, y, true);
								}
							}
						}
					}
				}

				// Scan HORIZONTALLY from RIGHT to LEFT and place Atoms in depth beyond the silhouette edge. 
				for (y = 0; y < spriteHeight; y += m_Resolution) {
					inside = false;
					for (x = spriteWidth - 1; x >= 0; --x) {
						// Detect if we are crossing a silhouette boundary.
						if ((getpixel(refSprite, x, y) != g_MaskColor && !inside) || (getpixel(refSprite, x, y) == g_MaskColor && inside)) {
							// Reset the depth counter
							depthCount = 0;
							inside = !inside;
						} else if (inside) {
							// Increment the depth counter if inside silhouette and propose making Atom if specified depth is reached.
							++depthCount;
							if (depthCount == m_Depth) {
								clear = true;
								// Check whether depth is sufficient in the other cardinal directions.
								for (int i = 1; i <= m_Depth && clear; ++i) {
									if (x - i < 0 || y + i >= refSprite->h || y - i < 0 || getpixel(refSprite, x - i, y) == g_MaskColor || getpixel(refSprite, x, y + i) == g_MaskColor || getpixel(refSprite, x, y - i) == g_MaskColor) {
										clear = false;
									}
								}
								if (clear && getpixel(checkBitmap, x, y) == g_MaskColor) {
									// Mark that an Atom has been put in this location, to avoid duplicate Atoms
									putpixel(checkBitmap, x, y, 99);
									AddAtomToGroup(ownerMOSRotating, spriteOffset, x, y, true);
								}
							}
						}
					}
				}

				// Scan VERTICALLY from TOP to BOTTOM and place Atoms in depth beyond the silhouette edge.
				for (x = 0; x < spriteWidth; x += m_Resolution) {
					inside = false;
					for (y = 0; y < spriteHeight; ++y) {
						// Detect if we are crossing a silhouette boundary.
						if ((getpixel(refSprite, x, y) != g_MaskColor && !inside) || (getpixel(refSprite, x, y) == g_MaskColor && inside)) {
							// Reset the depth counter
							depthCount = 0;
							inside = !inside;
						} else if (inside) {
							// Increment the depth counter if inside silhouette and propose making Atom if specified depth is reached.
							++depthCount;
							if (depthCount == m_Depth) {
								clear = true;
								// Check whether depth is sufficient in the other cardinal directions.
								for (int i = 1; i <= m_Depth && clear; ++i) {
									if (x + i >= refSprite->w || x - i < 0 || y + i >= refSprite->h || getpixel(refSprite, x + i, y) == g_MaskColor || getpixel(refSprite, x - i, y) == g_MaskColor || getpixel(refSprite, x, y + i) == g_MaskColor) {
										clear = false;
									}
								}
								// Depth is cleared in all directions, so go ahead and place Atom.
								if (clear && getpixel(checkBitmap, x, y) == g_MaskColor) {
									// Mark that an Atom has been put in this location, to avoid duplicate Atoms.
									putpixel(checkBitmap, x, y, 99);
									AddAtomToGroup(ownerMOSRotating, spriteOffset, x, y, true);
								}
							}
						}
					}
				}

				// Scan VERTICALLY from BOTTOM to TOP and place Atoms in depth beyond the silhouette edge.
				for (x = 0; x < spriteWidth; x += m_Resolution) {
					inside = false;
					for (y = spriteHeight - 1; y >= 0; --y) {
						// Detect if we are crossing a silhouette boundary.
						if ((getpixel(refSprite, x, y) != g_MaskColor && !inside) || (getpixel(refSprite, x, y) == g_MaskColor && inside)) {
							// Reset the depth counter
							depthCount = 0;
							inside = !inside;
						} else if (inside) {
							// Increment the depth counter if inside silhouette and propose making Atom if specified depth is reached.
							++depthCount;
							if (depthCount == m_Depth) {
								clear = true;
								// Check whether depth is sufficient in the other cardinal directions.
								for (int i = 1; i <= m_Depth && clear; ++i) {
									if (x + i >= refSprite->w || x - i < 0 || y - i < 0 || getpixel(refSprite, x + i, y) == g_MaskColor || getpixel(refSprite, x - i, y) == g_MaskColor || getpixel(refSprite, x, y - i) == g_MaskColor) {
										clear = false;
									}
								}
								// Depth is cleared in all directions, so go ahead and place Atom.
								if (clear && getpixel(checkBitmap, x, y) == g_MaskColor) {
									// Mark that an Atom has been put in this location, to avoid duplicate Atoms.
									putpixel(checkBitmap, x, y, 99);
									AddAtomToGroup(ownerMOSRotating, spriteOffset, x, y, true);
								}
							}
						}
					}
				}
			}
			release_bitmap(refSprite);
			release_bitmap(checkBitmap);

			destroy_bitmap(checkBitmap);
			checkBitmap = nullptr;
		}

		// If no Atoms were made, just place a default one in the middle
		if (m_Atoms.empty()) { AddAtomToGroup(ownerMOSRotating, spriteOffset, spriteWidth / 2, spriteHeight / 2, false); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AtomGroup::AddAtomToGroup(MOSRotating *ownerMOSRotating, const Vector &spriteOffset, int x, int y, bool calcNormal) {
		Atom *atomToAdd = new Atom(Vector(static_cast<float>(x) + spriteOffset.GetFloorIntX(), static_cast<float>(y) + spriteOffset.GetFloorIntY()), m_Material, ownerMOSRotating);
		if (calcNormal) { atomToAdd->CalculateNormal(ownerMOSRotating->GetSpriteFrame(), -ownerMOSRotating->GetSpriteOffset()); }
		atomToAdd->SetIgnoreMOIDsByGroup(&m_IgnoreMOIDs);
		m_Atoms.push_back(atomToAdd);
	}
}
