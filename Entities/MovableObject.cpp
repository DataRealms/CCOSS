//////////////////////////////////////////////////////////////////////////////////////////
// File:            MovableObject.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the MovableObject class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "MovableObject.h"

#include "ActivityMan.h"
#include "PresetMan.h"
#include "SceneMan.h"
#include "ConsoleMan.h"
#include "SettingsMan.h"
#include "LuaMan.h"
#include "Atom.h"
#include "Actor.h"
#include "SLTerrain.h"

namespace RTE {

AbstractClassInfo(MovableObject, SceneObject);

unsigned long int MovableObject::m_UniqueIDCounter = 1;

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MovableObject, effectively
//                  resetting the members of this abstraction level only.

void MovableObject::Clear()
{
    m_MOType = TypeGeneric;
    m_Mass = 0;
    m_Vel.Reset();
    m_PrevPos.Reset();
    m_PrevVel.Reset();
	m_DistanceTravelled = 0;
    m_Scale = 1.0;
    m_GlobalAccScalar = 1.0;
    m_AirResistance = 0;
    m_AirThreshold = 5;
    m_PinStrength = 0;
    m_RestThreshold = 500;
    m_Forces.clear();
    m_ImpulseForces.clear();
    m_AgeTimer.Reset();
    m_RestTimer.Reset();
    m_Lifetime = 0;
    m_Sharpness = 1.0;
//    m_MaterialId = 0;
    m_CheckTerrIntersection = false;
    m_HitsMOs = false;
    m_pMOToNotHit = 0;
    m_MOIgnoreTimer.Reset();
    m_GetsHitByMOs = false;
    m_IgnoresTeamHits = false;
    m_IgnoresAtomGroupHits = false;
    m_IgnoresAGHitsWhenSlowerThan = -1;
    m_MissionCritical = false;
    m_CanBeSquished = true;
    m_IsUpdated = false;
    m_WrapDoubleDraw = true;
    m_DidWrap = false;
    m_MOID = g_NoMOID;
    m_RootMOID = g_NoMOID;
    m_HasEverBeenAddedToMovableMan = false;
	m_ExistsInMovableMan = false;
    m_MOIDFootprint = 0;
    m_AlreadyHitBy.clear();
	m_VelOscillations = 0;
    m_ToSettle = false;
    m_ToDelete = false;
    m_HUDVisible = true;
    m_IsTraveling = false;
    m_AllLoadedScripts.clear();
    m_FunctionsAndScripts.clear();
    m_ThreadedLuaState = nullptr;
    m_HasSinglethreadedScripts = false;
    m_ScriptObjectName.clear();
    m_ScreenEffectFile.Reset();
    m_pScreenEffect = 0;
	m_EffectRotAngle = 0;
	m_InheritEffectRotAngle = false;
	m_RandomizeEffectRotAngle = false;
	m_RandomizeEffectRotAngleEveryFrame = false;
	m_ScreenEffectHash = 0;
    m_EffectStartTime = 0;
    m_EffectStopTime = 0;
    m_EffectStartStrength = 128;
    m_EffectStopStrength = 128;
    m_EffectAlwaysShows = false;

    m_UniqueID = 0;

	m_RemoveOrphanTerrainRadius = 0;
	m_RemoveOrphanTerrainMaxArea = 0;
	m_RemoveOrphanTerrainRate = 0.0;
	m_DamageOnCollision = 0.0;
	m_DamageOnPenetration = 0.0;
	m_WoundDamageMultiplier = 1.0;
    m_ApplyWoundDamageOnCollision = false;
    m_ApplyWoundBurstDamageOnCollision = false;
	m_IgnoreTerrain = false;

	m_MOIDHit = g_NoMOID;
	m_TerrainMatHit = g_MaterialAir;
	m_ParticleUniqueIDHit = 0;

    m_LastCollisionSimFrameNumber = 0;

	m_SimUpdatesBetweenScriptedUpdates = 1;
    m_SimUpdatesSinceLastScriptedUpdate = 0;
}

LuaStateWrapper & MovableObject::GetAndLockStateForScript(const std::string &scriptPath) {
    // Initialize our threaded state if required
    if (g_LuaMan.IsScriptThreadSafe(scriptPath)) {
        if (m_ThreadedLuaState == nullptr) {
            m_ThreadedLuaState = g_LuaMan.GetAndLockFreeScriptState();
        } else {
            m_ThreadedLuaState->GetMutex().lock();
        }
        return *m_ThreadedLuaState;
    }

    m_HasSinglethreadedScripts = true;
    g_LuaMan.GetMasterScriptState().GetMutex().lock();
    return g_LuaMan.GetMasterScriptState();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MovableObject object ready for use.

int MovableObject::Create()
{
    if (SceneObject::Create() < 0)
        return -1;

    m_AgeTimer.Reset();
    m_RestTimer.Reset();

    // If the stop time hasn't been assigned, just make the same as the life time.
    if (m_EffectStopTime <= 0)
        m_EffectStopTime = m_Lifetime;

	m_UniqueID = MovableObject::GetNextUniqueID();

	m_MOIDHit = g_NoMOID;
	m_TerrainMatHit = g_MaterialAir;
	m_ParticleUniqueIDHit = 0;

    g_MovableMan.RegisterObject(this);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MovableObject object ready for use.

int MovableObject::Create(const float mass,
                          const Vector &position,
                          const Vector &velocity,
                          float rotAngle,
                          float angleVel,
                          unsigned long lifetime,
                          bool hitMOs,
                          bool getHitByMOs)
{
    m_Mass = mass;
    m_Pos = position;
    m_Vel = velocity;
    m_AgeTimer.Reset();
    m_RestTimer.Reset();
    m_Lifetime = lifetime;
//    m_MaterialId = matId;
    m_HitsMOs = hitMOs;
    m_GetsHitByMOs = getHitByMOs;

	m_UniqueID = MovableObject::GetNextUniqueID();

	m_MOIDHit = g_NoMOID;
	m_TerrainMatHit = g_MaterialAir;
	m_ParticleUniqueIDHit = 0;

	g_MovableMan.RegisterObject(this);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a MovableObject to be identical to another, by deep copy.

int MovableObject::Create(const MovableObject &reference)
{
    SceneObject::Create(reference);

    m_MOType = reference.m_MOType;
    m_Mass = reference.m_Mass;
    m_Pos = reference.m_Pos;
    m_Vel = reference.m_Vel;
    m_Scale = reference.m_Scale;
    m_GlobalAccScalar = reference.m_GlobalAccScalar;
    m_AirResistance = reference.m_AirResistance;
    m_AirThreshold = reference.m_AirThreshold;
    m_PinStrength = reference.m_PinStrength;
    m_RestThreshold = reference.m_RestThreshold;
//    m_Force = reference.m_Force;
//    m_ImpulseForce = reference.m_ImpulseForce;
    // Should reset age instead??
//    m_AgeTimer = reference.m_AgeTimer;
    m_AgeTimer.Reset();
    m_RestTimer.Reset();
    m_Lifetime = reference.m_Lifetime;
    m_Sharpness = reference.m_Sharpness;
//    m_MaterialId = reference.m_MaterialId;
    m_CheckTerrIntersection = reference.m_CheckTerrIntersection;
    m_HitsMOs = reference.m_HitsMOs;
    m_GetsHitByMOs = reference.m_GetsHitByMOs;
    m_IgnoresTeamHits = reference.m_IgnoresTeamHits;
    m_IgnoresAtomGroupHits = reference.m_IgnoresAtomGroupHits;
    m_IgnoresAGHitsWhenSlowerThan = reference.m_IgnoresAGHitsWhenSlowerThan;
    m_pMOToNotHit = reference.m_pMOToNotHit;
    m_MOIgnoreTimer = reference.m_MOIgnoreTimer;
    m_MissionCritical = reference.m_MissionCritical;
    m_CanBeSquished = reference.m_CanBeSquished;
    m_HUDVisible = reference.m_HUDVisible;

    for (auto &[scriptPath, scriptEnabled] : reference.m_AllLoadedScripts) {
        LoadScript(scriptPath, scriptEnabled);
    }

    if (reference.m_pScreenEffect)
    {
        m_ScreenEffectFile = reference.m_ScreenEffectFile;
        m_pScreenEffect = m_ScreenEffectFile.GetAsBitmap();

    }
	m_EffectRotAngle = reference.m_EffectRotAngle;
	m_InheritEffectRotAngle = reference.m_InheritEffectRotAngle;
	m_RandomizeEffectRotAngle = reference.m_RandomizeEffectRotAngle;
	m_RandomizeEffectRotAngleEveryFrame = reference.m_RandomizeEffectRotAngleEveryFrame;

	if (m_RandomizeEffectRotAngle)
		m_EffectRotAngle = c_PI * RandomNum(-2.0F, 2.0F);

	m_ScreenEffectHash = reference.m_ScreenEffectHash;
    m_EffectStartTime = reference.m_EffectStartTime;
    m_EffectStopTime = reference.m_EffectStopTime;
    m_EffectStartStrength = reference.m_EffectStartStrength;
    m_EffectStopStrength = reference.m_EffectStopStrength;
    m_EffectAlwaysShows = reference.m_EffectAlwaysShows;
	m_RemoveOrphanTerrainRadius = reference.m_RemoveOrphanTerrainRadius;
	m_RemoveOrphanTerrainMaxArea = reference.m_RemoveOrphanTerrainMaxArea;
	m_RemoveOrphanTerrainRate = reference.m_RemoveOrphanTerrainRate;
	m_DamageOnCollision = reference.m_DamageOnCollision;
	m_DamageOnPenetration = reference.m_DamageOnPenetration;
	m_WoundDamageMultiplier = reference.m_WoundDamageMultiplier;
	m_IgnoreTerrain = reference.m_IgnoreTerrain;

	m_MOIDHit = reference.m_MOIDHit;
	m_TerrainMatHit = reference.m_TerrainMatHit;
	m_ParticleUniqueIDHit = reference.m_ParticleUniqueIDHit;

	m_SimUpdatesBetweenScriptedUpdates = reference.m_SimUpdatesBetweenScriptedUpdates;
	m_SimUpdatesSinceLastScriptedUpdate = reference.m_SimUpdatesSinceLastScriptedUpdate;

	m_UniqueID = MovableObject::GetNextUniqueID();
	g_MovableMan.RegisterObject(this);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int MovableObject::ReadProperty(const std::string_view &propName, Reader &reader)
{
	if (propName == "Mass") {
		reader >> m_Mass;
	} else if (propName == "Velocity")
		reader >> m_Vel;
	else if (propName == "Scale")
		reader >> m_Scale;
	else if (propName == "GlobalAccScalar")
		reader >> m_GlobalAccScalar;
	else if (propName == "AirResistance")
	{
		reader >> m_AirResistance;
		// Backwards compatibility after we made this value scaled over time
		m_AirResistance /= 0.01666F;
	}
	else if (propName == "AirThreshold")
		reader >> m_AirThreshold;
	else if (propName == "PinStrength")
		reader >> m_PinStrength;
	else if (propName == "RestThreshold")
		reader >> m_RestThreshold;
	else if (propName == "LifeTime")
		reader >> m_Lifetime;
	else if (propName == "Age") {
		double age;
		reader >> age;
		m_AgeTimer.SetElapsedSimTimeMS(age);
	} else if (propName == "Sharpness")
		reader >> m_Sharpness;
	else if (propName == "HitsMOs")
		reader >> m_HitsMOs;
	else if (propName == "GetsHitByMOs")
		reader >> m_GetsHitByMOs;
	else if (propName == "IgnoresTeamHits")
		reader >> m_IgnoresTeamHits;
	else if (propName == "IgnoresAtomGroupHits")
		reader >> m_IgnoresAtomGroupHits;
	else if (propName == "IgnoresAGHitsWhenSlowerThan")
		reader >> m_IgnoresAGHitsWhenSlowerThan;
	else if (propName == "RemoveOrphanTerrainRadius")
	{
		reader >> m_RemoveOrphanTerrainRadius;
		if (m_RemoveOrphanTerrainRadius > MAXORPHANRADIUS)
			m_RemoveOrphanTerrainRadius = MAXORPHANRADIUS;
	}
	else if (propName == "RemoveOrphanTerrainMaxArea")
	{
		reader >> m_RemoveOrphanTerrainMaxArea;
		if (m_RemoveOrphanTerrainMaxArea > MAXORPHANRADIUS * MAXORPHANRADIUS)
			m_RemoveOrphanTerrainMaxArea = MAXORPHANRADIUS * MAXORPHANRADIUS;
	}
	else if (propName == "RemoveOrphanTerrainRate")
		reader >> m_RemoveOrphanTerrainRate;
	else if (propName == "MissionCritical")
		reader >> m_MissionCritical;
	else if (propName == "CanBeSquished")
		reader >> m_CanBeSquished;
	else if (propName == "HUDVisible")
		reader >> m_HUDVisible;
	else if (propName == "ScriptPath") {
		std::string scriptPath = g_PresetMan.GetFullModulePath(reader.ReadPropValue());
        switch (LoadScript(scriptPath)) {
            case 0:
                break;
            case -1:
                reader.ReportError("The script path " + scriptPath + " was empty.");
                break;
            case -2:
                reader.ReportError("The script path " + scriptPath + "  did not point to a valid file.");
                break;
            case -3:
                reader.ReportError("The script path " + scriptPath + " is already loaded onto this object.");
                break;
            case -4:
                reader.ReportError("Failed to do necessary setup to add scripts while attempting to add the script with path " + scriptPath + ". This has nothing to do with your script, please report it to a developer.");
                break;
            case -5:
                // Error in lua file, this'll pop up in the console so no need to report an error through the reader.
                break;
            default:
                RTEAbort("Reached default case while adding script in INI. This should never happen!");
                break;
        }
	} else if (propName == "ScreenEffect") {
        reader >> m_ScreenEffectFile;
        m_pScreenEffect = m_ScreenEffectFile.GetAsBitmap();
		m_ScreenEffectHash = m_ScreenEffectFile.GetHash();
    }
    else if (propName == "EffectStartTime")
        reader >> m_EffectStartTime;
	else if (propName == "EffectRotAngle")
		reader >> m_EffectRotAngle;
	else if (propName == "InheritEffectRotAngle")
		reader >> m_InheritEffectRotAngle;
	else if (propName == "RandomizeEffectRotAngle")
		reader >> m_RandomizeEffectRotAngle;
	else if (propName == "RandomizeEffectRotAngleEveryFrame")
		reader >> m_RandomizeEffectRotAngleEveryFrame;
	else if (propName == "EffectStopTime")
        reader >> m_EffectStopTime;
    else if (propName == "EffectStartStrength")
    {
        float strength;
        reader >> strength;
        m_EffectStartStrength = std::floor((float)255 * strength);
    }
    else if (propName == "EffectStopStrength")
    {
        float strength;
        reader >> strength;
        m_EffectStopStrength = std::floor((float)255 * strength);
    }
    else if (propName == "EffectAlwaysShows")
        reader >> m_EffectAlwaysShows;
	else if (propName == "DamageOnCollision")
		reader >> m_DamageOnCollision;
	else if (propName == "DamageOnPenetration")
		reader >> m_DamageOnPenetration;
	else if (propName == "WoundDamageMultiplier")
		reader >> m_WoundDamageMultiplier;
    else if (propName == "ApplyWoundDamageOnCollision")
        reader >> m_ApplyWoundDamageOnCollision;
    else if (propName == "ApplyWoundBurstDamageOnCollision")
        reader >> m_ApplyWoundBurstDamageOnCollision;
	else if (propName == "IgnoreTerrain")
		reader >> m_IgnoreTerrain;
    else if (propName == "SimUpdatesBetweenScriptedUpdates")
        reader >> m_SimUpdatesBetweenScriptedUpdates;
	else
        return SceneObject::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this MovableObject to an output stream for
//                  later recreation with Create(istream &stream);

int MovableObject::Save(Writer &writer) const
{
    SceneObject::Save(writer);
// TODO: Make proper save system that knows not to save redundant data!
// Note - this function isn't even called when saving a scene. Turns out that scene special-cases this stuff, see Scene::Save()
// In future, perhaps we ought to not do that. Who knows?

    writer.NewProperty("Mass");
    writer << m_Mass;
    writer.NewProperty("Velocity");
    writer << m_Vel;
    writer.NewProperty("Scale");
    writer << m_Scale;
    writer.NewProperty("GlobalAccScalar");
    writer << m_GlobalAccScalar;
    writer.NewProperty("AirResistance");
    writer << (m_AirResistance * 0.01666F); // Backwards compatibility after we made this value scaled over time
    writer.NewProperty("AirThreshold");
    writer << m_AirThreshold;
    writer.NewProperty("PinStrength");
    writer << m_PinStrength;
    writer.NewProperty("RestThreshold");
    writer << m_RestThreshold;
    writer.NewProperty("LifeTime");
    writer << m_Lifetime;
    writer.NewProperty("Sharpness");
    writer << m_Sharpness;
    writer.NewProperty("HitsMOs");
    writer << m_HitsMOs;
    writer.NewProperty("GetsHitByMOs");
    writer << m_GetsHitByMOs;
    writer.NewProperty("IgnoresTeamHits");
    writer << m_IgnoresTeamHits;
    writer.NewProperty("IgnoresAtomGroupHits");
    writer << m_IgnoresAtomGroupHits;
    writer.NewProperty("IgnoresAGHitsWhenSlowerThan");
    writer << m_IgnoresAGHitsWhenSlowerThan;
    writer.NewProperty("MissionCritical");
    writer << m_MissionCritical;
    writer.NewProperty("CanBeSquished");
    writer << m_CanBeSquished;
    writer.NewProperty("HUDVisible");
    writer << m_HUDVisible;
    for (const auto &[scriptPath, scriptEnabled] : m_AllLoadedScripts) {
        if (!scriptPath.empty()) {
            writer.NewProperty("ScriptPath");
            writer << scriptPath;
        }
    }
    writer.NewProperty("ScreenEffect");
    writer << m_ScreenEffectFile;
    writer.NewProperty("EffectStartTime");
    writer << m_EffectStartTime;
    writer.NewProperty("EffectStopTime");
    writer << m_EffectStopTime;
    writer.NewProperty("EffectStartStrength");
    writer << (float)m_EffectStartStrength / 255.0f;
    writer.NewProperty("EffectStopStrength");
    writer << (float)m_EffectStopStrength / 255.0f;
    writer.NewProperty("EffectAlwaysShows");
    writer << m_EffectAlwaysShows;
    writer.NewProperty("DamageOnCollision");
    writer << m_DamageOnCollision;
    writer.NewProperty("DamageOnPenetration");
    writer << m_DamageOnPenetration;
    writer.NewProperty("WoundDamageMultiplier");
    writer << m_WoundDamageMultiplier;
    writer.NewProperty("ApplyWoundDamageOnCollision");
    writer << m_ApplyWoundDamageOnCollision;
    writer.NewProperty("ApplyWoundBurstDamageOnCollision");
    writer << m_ApplyWoundBurstDamageOnCollision;
    writer.NewProperty("IgnoreTerrain");
    writer << m_IgnoreTerrain;
    writer.NewProperty("SimUpdatesBetweenScriptedUpdates");
    writer << m_SimUpdatesBetweenScriptedUpdates;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MovableObject::Destroy(bool notInherited) {
    if (ObjectScriptsInitialized()) {
        RunScriptedFunctionInAppropriateScripts("Destroy");

        if (m_ThreadedLuaState) {
            std::lock_guard<std::recursive_mutex> lock(m_ThreadedLuaState->GetMutex());
            m_ThreadedLuaState->RunScriptString(m_ScriptObjectName + " = nil;");
        }

        if (m_HasSinglethreadedScripts) {
            // Shouldn't ever happen (destruction is delayed in movableman to happen in singlethreaded manner), but lets check just in case
            if (g_LuaMan.GetThreadLuaStateOverride() || !g_LuaMan.GetMasterScriptState().GetMutex().try_lock()) {
                RTEAbort("Failed to destroy object scripts for " + GetModuleAndPresetName() + ". Please report this to a developer.");
            } else {
                std::lock_guard<std::recursive_mutex> lock(g_LuaMan.GetMasterScriptState().GetMutex(), std::adopt_lock);
                g_LuaMan.GetMasterScriptState().RunScriptString(m_ScriptObjectName + " = nil;");
            }
        }
    }
	g_MovableMan.UnregisterObject(this);
    if (!notInherited) { SceneObject::Destroy(); }
    Clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MovableObject::LoadScript(const std::string &scriptPath, bool loadAsEnabledScript) {
	if (scriptPath.empty()) {
		return -1;
	} else if (!System::PathExistsCaseSensitive(scriptPath)) {
		return -2;
	} else if (HasScript(scriptPath)) {
		return -3;
	}

    LuaStateWrapper& usedState = GetAndLockStateForScript(scriptPath);
    std::lock_guard<std::recursive_mutex> lock(usedState.GetMutex(), std::adopt_lock);

	std::string luaClearSupportedFunctionsString;
	luaClearSupportedFunctionsString.reserve(160);
	for (const std::string &functionName : GetSupportedScriptFunctionNames()) {
		luaClearSupportedFunctionsString += functionName + " = nil;";
	}
	if (usedState.RunScriptString(luaClearSupportedFunctionsString) < 0) {
		return -4;
	}

	for (const std::string &functionName : GetSupportedScriptFunctionNames()) {
		if (m_FunctionsAndScripts.find(functionName) == m_FunctionsAndScripts.end()) {
			m_FunctionsAndScripts.try_emplace(functionName);
		}
	}

	m_AllLoadedScripts.try_emplace(scriptPath, loadAsEnabledScript);

	std::unordered_map<std::string, LuabindObjectWrapper *> scriptFileFunctions;
	if (usedState.RunScriptFileAndRetrieveFunctions(scriptPath, GetSupportedScriptFunctionNames(), scriptFileFunctions) < 0) {
		return -5;
	}
	for (const auto &[functionName, functionObject] : scriptFileFunctions) {
		m_FunctionsAndScripts.at(functionName).emplace_back(std::unique_ptr<LuabindObjectWrapper>(functionObject));
	}

	usedState.RunScriptString(luaClearSupportedFunctionsString);

	if (ObjectScriptsInitialized()) {
		RunFunctionOfScript(scriptPath, "Create");
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MovableObject::ReloadScripts() {
    if (m_AllLoadedScripts.empty()) {
        return 0;
    }

	int status = 0;

	//TODO consider getting rid of this const_cast. It would require either code duplication or creating some non-const methods (specifically of PresetMan::GetEntityPreset, which may be unsafe. Could be this gross exceptional handling is the best way to go.
	MovableObject *movableObjectPreset = const_cast<MovableObject *>(dynamic_cast<const MovableObject *>(g_PresetMan.GetEntityPreset(GetClassName(), GetPresetName(), GetModuleID())));
	if (this != movableObjectPreset) {
		movableObjectPreset->ReloadScripts();
	}

	std::unordered_map<std::string, bool> loadedScriptsCopy = m_AllLoadedScripts;
	m_AllLoadedScripts.clear();
	m_FunctionsAndScripts.clear();
	for (const auto &[scriptPath, scriptEnabled] : loadedScriptsCopy) {
		status = LoadScript(scriptPath, scriptEnabled);
		// If the script fails to load because of an error in its Lua, we need to manually add the script path so it's not lost forever.
		if (status == -5) {
			m_AllLoadedScripts.try_emplace(scriptPath, scriptEnabled);
		} else if (status < 0) {
			break;
		}
	}

    return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MovableObject::InitializeObjectScripts() {
    auto createScriptedObjectInState = [&](LuaStateWrapper &luaState) {
        luaState.SetTempEntity(this);
        m_ScriptObjectName = "_ScriptedObjects[\"" + std::to_string(m_UniqueID) + "\"]";
        if (luaState.RunScriptString("_ScriptedObjects = _ScriptedObjects or {}; " + m_ScriptObjectName + " = To" + GetClassName() + "(LuaMan.TempEntity); ") < 0) {
            RTEAbort("Failed to initialize object scripts for " + GetModuleAndPresetName() + ". Please report this to a developer.");
        }
    };
    
    if (m_ThreadedLuaState) {
        std::lock_guard<std::recursive_mutex> lock(m_ThreadedLuaState->GetMutex());
        createScriptedObjectInState(*m_ThreadedLuaState);
    }

    if (m_HasSinglethreadedScripts) {
        std::lock_guard<std::recursive_mutex> lock(g_LuaMan.GetMasterScriptState().GetMutex());
        createScriptedObjectInState(g_LuaMan.GetMasterScriptState());
    }

	if (!m_FunctionsAndScripts.at("Create").empty() && RunScriptedFunctionInAppropriateScripts("Create", false, true) < 0) {
		m_ScriptObjectName = "ERROR";
		return -1;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MovableObject::EnableOrDisableScript(const std::string &scriptPath, bool enableScript) {
    if (m_AllLoadedScripts.empty()) {
        return false;
    }

	if (auto scriptEntryIterator = m_AllLoadedScripts.find(scriptPath); scriptEntryIterator != m_AllLoadedScripts.end() && scriptEntryIterator->second == !enableScript) {
		if (ObjectScriptsInitialized() && RunFunctionOfScript(scriptPath, enableScript ? "OnScriptEnable" : "OnScriptDisable") < 0) {
			return false;
		}
        scriptEntryIterator->second = enableScript;
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MovableObject::EnableOrDisableAllScripts(bool enableScripts) {
    for (const auto &[scriptPath, scriptIsEnabled] : m_AllLoadedScripts) {
		if (enableScripts != scriptIsEnabled) {
			EnableOrDisableScript(scriptPath, enableScripts);
		}
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MovableObject::RunScriptedFunctionInAppropriateScripts(const std::string &functionName, bool runOnDisabledScripts, bool stopOnError, const std::vector<const Entity *> &functionEntityArguments, const std::vector<std::string_view> &functionLiteralArguments, ThreadScriptsToRun scriptsToRun) {
    int status = 0;

    auto itr = m_FunctionsAndScripts.find(functionName);
    if (itr == m_FunctionsAndScripts.end() || itr->second.empty()) {
        return -1;
    } 
    
    if (!ObjectScriptsInitialized()) {
        status = InitializeObjectScripts();
    }

    if (status >= 0) {
        for (const std::unique_ptr<LuabindObjectWrapper> &functionObjectWrapper : itr->second) {
            bool scriptIsThreadSafe = g_LuaMan.IsScriptThreadSafe(functionObjectWrapper->GetFilePath());
            bool scriptIsSuitableForThread = scriptsToRun == ThreadScriptsToRun::SingleThreaded ? !scriptIsThreadSafe :
                                             scriptsToRun == ThreadScriptsToRun::MultiThreaded  ? scriptIsThreadSafe :
                                                                                                  true;
            if (!scriptIsSuitableForThread) {
                continue;
            }

            if (runOnDisabledScripts || m_AllLoadedScripts.at(functionObjectWrapper->GetFilePath())) {
                LuaStateWrapper& usedState = GetAndLockStateForScript(functionObjectWrapper->GetFilePath());
                std::lock_guard<std::recursive_mutex> lock(usedState.GetMutex(), std::adopt_lock);   
				status = usedState.RunScriptFunctionObject(functionObjectWrapper.get(), "_ScriptedObjects", std::to_string(m_UniqueID), functionEntityArguments, functionLiteralArguments);
                if (status < 0 && stopOnError) {
                    return status;
                }
            }
        }
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MovableObject::RunFunctionOfScript(const std::string &scriptPath, const std::string &functionName, const std::vector<const Entity *> &functionEntityArguments, const std::vector<std::string_view> &functionLiteralArguments) {    
    if (m_AllLoadedScripts.empty() || !ObjectScriptsInitialized()) {
		return -1;
	}

    LuaStateWrapper& usedState = GetAndLockStateForScript(scriptPath);
    std::lock_guard<std::recursive_mutex> lock(usedState.GetMutex(), std::adopt_lock);

	for (const std::unique_ptr<LuabindObjectWrapper> &functionObjectWrapper : m_FunctionsAndScripts.at(functionName)) {
		if (scriptPath == functionObjectWrapper->GetFilePath() && usedState.RunScriptFunctionObject(functionObjectWrapper.get(), "_ScriptedObjects", std::to_string(m_UniqueID), functionEntityArguments, functionLiteralArguments) < 0) {
			if (m_AllLoadedScripts.size() > 1) {
				g_ConsoleMan.PrintString("ERROR: An error occured while trying to run the " + functionName + " function for script at path " + scriptPath);
			}
			return -2;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     MovableObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a MovableObject object
//                  identical to an already existing one.

MovableObject::MovableObject(const MovableObject &reference):
    m_Mass(reference.GetMass()),
    m_Pos(reference.GetPos()),
    m_Vel(reference.GetVel()),
    m_AgeTimer(reference.GetAge()),
    m_Lifetime(reference.GetLifetime())
{

}
*/

void MovableObject::SetTeam(int team) {
	SceneObject::SetTeam(team);
	if (Activity *activity = g_ActivityMan.GetActivity()) { activity->ForceSetTeamAsActive(team); }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAltitude
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the altitide of this' pos (or appropriate low point) over the
//                  terrain, in pixels.

float MovableObject::GetAltitude(int max, int accuracy)
{
    return g_SceneMan.FindAltitude(m_Pos, max, accuracy);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MovableObject::AddAbsForce(const Vector &force, const Vector &absPos)
{
    m_Forces.push_back(std::make_pair(force, g_SceneMan.ShortestDistance(m_Pos, absPos) * c_MPP));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MovableObject::AddAbsImpulseForce(const Vector &impulse, const Vector &absPos)
{
#ifndef RELEASE_BUILD
		RTEAssert(impulse.GetLargest() < 500000, "HUEG IMPULSE FORCE");
#endif

		m_ImpulseForces.push_back(std::make_pair(impulse, g_SceneMan.ShortestDistance(m_Pos, absPos) * c_MPP));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MovableObject::RestDetection() {
	// Translational settling detection.
	if (m_Vel.Dot(m_PrevVel) < 0) {
		++m_VelOscillations;
	} else {
		m_VelOscillations = 0;
	}
	if ((m_Pos - m_PrevPos).MagnitudeIsGreaterThan(1.0F)) { m_RestTimer.Reset(); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MovableObject::IsAtRest() {
	if (m_RestThreshold < 0 || m_PinStrength) {
		return false;
	} else {
		if (m_VelOscillations > 2) {
			return true;
		}
		return m_RestTimer.IsPastSimMS(m_RestThreshold);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  OnMOHit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits another MO.
//                  This is called by the owned Atom/AtomGroup of this MovableObject during
//                  travel.

bool MovableObject::OnMOHit(HitData &hd)
{
    if (hd.RootBody[HITOR] != hd.RootBody[HITEE] && (hd.Body[HITOR] == this || hd.Body[HITEE] == this)) {
        RunScriptedFunctionInAppropriateScripts("OnCollideWithMO", false, false, {hd.Body[hd.Body[HITOR] == this ? HITEE : HITOR], hd.RootBody[hd.Body[HITOR] == this ? HITEE : HITOR]});
    }
	return hd.Terminate[hd.RootBody[HITOR] == this ? HITOR : HITEE] = false;
}

unsigned char MovableObject::HitWhatTerrMaterial() const
{
    return m_LastCollisionSimFrameNumber == g_MovableMan.GetSimUpdateFrameNumber() ? m_TerrainMatHit : g_MaterialAir;
}

void MovableObject::SetHitWhatTerrMaterial(unsigned char matID)
{
    m_TerrainMatHit = matID;
    m_LastCollisionSimFrameNumber = g_MovableMan.GetSimUpdateFrameNumber();
    RunScriptedFunctionInAppropriateScripts("OnCollideWithTerrain", false, false, {}, {std::to_string(m_TerrainMatHit)});
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Vector MovableObject::GetTotalForce() {
	Vector totalForceVector;
	for (const auto &[force, forceOffset] : m_Forces) {
		totalForceVector += force;
	}
	return totalForceVector;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ApplyForces
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gathers and applies the global and accumulated forces. Then it clears
//                  out the force list.Note that this does NOT apply the accumulated
//                  impulses (impulse forces)!

void MovableObject::ApplyForces()
{
    // Don't apply forces to pinned objects
    if (m_PinStrength > 0)
    {
        m_Forces.clear();
        return;
    }

    float deltaTime = g_TimerMan.GetDeltaTimeSecs();

//// TODO: remove this!$@#$%#@%#@%#@^#@^#@^@#^@#")
//    if (m_PresetName != "Test Player")
    // Apply global acceleration (gravity), scaled by the scalar we have that can even be negative.
    m_Vel += g_SceneMan.GetGlobalAcc() * m_GlobalAccScalar * deltaTime;

    // Calculate air resistance effects, only when something flies faster than a threshold
    if (m_AirResistance > 0 && m_Vel.GetLargest() >= m_AirThreshold)
        m_Vel *= 1.0 - (m_AirResistance * deltaTime);

	// Apply the translational effects of all the forces accumulated during the Update().
	if (m_Forces.size() > 0) {
		// Continuous force application to transformational velocity (F = m * a -> a = F / m).
		m_Vel += GetTotalForce() / (GetMass() != 0 ? GetMass() : 0.0001F) * deltaTime;
	}

    // Clear out the forces list
    m_Forces.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ApplyImpulses
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gathers and applies the accumulated impulse forces. Then it clears
//                  out the impulse list.Note that this does NOT apply the accumulated
//                  regular forces (non-impulse forces)!

void MovableObject::ApplyImpulses()
{
    // Don't apply forces to pinned objects
    if (m_PinStrength > 0)
    {
        m_ImpulseForces.clear();
        return;
    }

//    float totalImpulses.

    // Apply the translational effects of all the impulses accumulated during the Update()
    for (auto iItr = m_ImpulseForces.begin(); iItr != m_ImpulseForces.end(); ++iItr) {
        // Impulse force application to the transformational velocity of this MO.
        // Don't timescale these because they're already in kg * m/s (as opposed to kg * m/s^2).
        m_Vel += (((*iItr).first / (GetMass() != 0 ? GetMass() : 0.0001F)));
    }

    // Clear out the impulses list
    m_ImpulseForces.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PreTravel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does stuff that needs to be done before Travel(). Always call before
//                  calling Travel.

void MovableObject::PreTravel()
{
	// Temporarily remove the representation of this from the scene MO sampler
	if (m_GetsHitByMOs) {
        m_IsTraveling = true;
#ifdef DRAW_MOID_LAYER
		if (!g_SettingsMan.SimplifiedCollisionDetection()) {
			Draw(g_SceneMan.GetMOIDBitmap(), Vector(), DrawMode::g_DrawNoMOID, true);
		}
#endif
	}

    // Save previous position and velocities before moving
    m_PrevPos = m_Pos;
    m_PrevVel = m_Vel;

	m_MOIDHit = g_NoMOID;
	m_TerrainMatHit = g_MaterialAir;
	m_ParticleUniqueIDHit = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Travel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Travels this MovableObject, using its physical representation.

void MovableObject::Travel()
{

}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PostTravel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does stuff that needs to be done after Travel(). Always call after
//                  calling Travel.

void MovableObject::PostTravel()
{
    // Toggle whether this gets hit by other AtomGroup MOs depending on whether it's going slower than a set threshold
    if (m_IgnoresAGHitsWhenSlowerThan > 0) {
        m_IgnoresAtomGroupHits = m_Vel.MagnitudeIsLessThan(m_IgnoresAGHitsWhenSlowerThan);
    }

	if (m_GetsHitByMOs) {
        if (!GetParent()) {
            m_IsTraveling = false;
#ifdef DRAW_MOID_LAYER
			if (!g_SettingsMan.SimplifiedCollisionDetection()) {
                Draw(g_SceneMan.GetMOIDBitmap(), Vector(), DrawMode::g_DrawMOID, true);
			}
#endif
		}
		m_AlreadyHitBy.clear();
	}
	m_IsUpdated = true;

    // Check for age expiration
    if (m_Lifetime && m_AgeTimer.GetElapsedSimTimeMS() > m_Lifetime) {
        m_ToDelete = true;
    }

    // Check for stupid positions
    if (!GetParent() && !g_SceneMan.IsWithinBounds(m_Pos.m_X, m_Pos.m_Y, 1000)) {
        m_ToDelete = true;
    }

    // Fix speeds that are too high
    FixTooFast();

    // Never let mission critical stuff settle or delete
    if (m_MissionCritical) {
        m_ToSettle = false;
    }

    // Reset the terrain intersection warning
    m_CheckTerrIntersection = false;

	m_DistanceTravelled += m_Vel.GetMagnitude() * c_PPM * g_TimerMan.GetDeltaTimeSecs();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MovableObject::Update() {
	if (m_RandomizeEffectRotAngleEveryFrame) { m_EffectRotAngle = c_PI * 2.0F * RandomNormalNum(); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MovableObject::Draw(BITMAP* targetBitmap, const Vector& targetPos, DrawMode mode, bool onlyPhysical) const {
    if (mode == g_DrawMOID && m_MOID == g_NoMOID) {
        return;
    }

    g_SceneMan.RegisterDrawing(targetBitmap, mode == g_DrawNoMOID ? g_NoMOID : m_MOID, m_Pos - targetPos, 1.0F);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MovableObject::UpdateScripts(ThreadScriptsToRun scriptsToRun) {
	m_SimUpdatesSinceLastScriptedUpdate++;

	if (m_AllLoadedScripts.empty()) {
		return -1;
	}

	int status = 0;
	if (!ObjectScriptsInitialized()) {
		status = InitializeObjectScripts();
	}

	if (m_SimUpdatesSinceLastScriptedUpdate < m_SimUpdatesBetweenScriptedUpdates) {
		return 1;
	}

	m_SimUpdatesSinceLastScriptedUpdate = 0;

	if (status >= 0) {
		status = RunScriptedFunctionInAppropriateScripts("Update", false, true, {}, {}, scriptsToRun);

        // Perform our synced updates to let multithreaded scripts do anything that interacts with stuff in a non-const way
        // This is identical to non-multithreaded script's Update()
        // TODO - in future, enforce that everything in MultiThreaded Update() is const, so non-const actions must be performed in SyncedUpdate
        // This would require a bunch of Lua binding fuckery, but eventually maybe it'd be possible.
        // I wonder if we can do some SFINAE magic to make the luabindings automagically do a no-op with const objects, to avoid writing the bindings twice
        if (status >= 0 && scriptsToRun == ThreadScriptsToRun::SingleThreaded) {
            // If we're in a SingleThreaded context, we run the MultiThreaded scripts synced updates:
            status = RunScriptedFunctionInAppropriateScripts("SyncedUpdate", false, true, {}, {}, ThreadScriptsToRun::MultiThreaded); // This isn't a typo!
        }
	}

	return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MovableObject::WhilePieMenuOpenListener(const PieMenu *pieMenu) {
	return RunScriptedFunctionInAppropriateScripts("WhilePieMenuOpen", false, false, { pieMenu });
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateMOID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this' and its childrens MOID status. Supposed to be done every frame.

void MovableObject::UpdateMOID(std::vector<MovableObject *> &MOIDIndex, MOID rootMOID, bool makeNewMOID)
{
    // Register the own MOID
    RegMOID(MOIDIndex, rootMOID, makeNewMOID);

    // Register all the attachaed children of this, going through the class hierarchy
    UpdateChildMOIDs(MOIDIndex, rootMOID, makeNewMOID);

    // Figure out the total MOID footstep of this and all its children combined
    m_MOIDFootprint = MOIDIndex.size() - m_MOID;
}



//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Puts all MOIDs associated with this MO and all it's descendants into MOIDs vector

void MovableObject::GetMOIDs(std::vector<MOID> &MOIDs) const
{
	if (m_MOID != g_NoMOID) {
		MOIDs.push_back(m_MOID);
    }
}

MOID MovableObject::HitWhatMOID() const
{
    return m_LastCollisionSimFrameNumber == g_MovableMan.GetSimUpdateFrameNumber() ? m_MOIDHit : g_NoMOID;
}

void MovableObject::SetHitWhatMOID(MOID id)
{
    m_MOIDHit = id;
    m_LastCollisionSimFrameNumber = g_MovableMan.GetSimUpdateFrameNumber();
}

long int MovableObject::HitWhatParticleUniqueID() const
{
    return m_LastCollisionSimFrameNumber == g_MovableMan.GetSimUpdateFrameNumber() ? m_ParticleUniqueIDHit : 0;
}

void MovableObject::SetHitWhatParticleUniqueID(long int id)
{
    m_ParticleUniqueIDHit = id;
    m_LastCollisionSimFrameNumber = g_MovableMan.GetSimUpdateFrameNumber();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RegMOID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this MO register itself in the MOID register and get ID:s for
//                  itself and its children for this frame.
//                  BITMAP of choice.

void MovableObject::RegMOID(std::vector<MovableObject *> &MOIDIndex, MOID rootMOID, bool makeNewMOID) {
    if (!makeNewMOID && GetParent()) {
        m_MOID = GetParent()->GetID();
    } else {
        if (MOIDIndex.size() == g_NoMOID) { MOIDIndex.push_back(0); }

		m_MOID = MOIDIndex.size();
		MOIDIndex.push_back(this);
    }

    m_RootMOID = rootMOID == g_NoMOID ? m_MOID : rootMOID;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MovableObject::DrawToTerrain(SLTerrain *terrain) {
	if (!terrain) {
		return false;
	}
	if (dynamic_cast<MOSprite *>(this)) {
		auto wrappedMaskedBlit = [](BITMAP *sourceBitmap, BITMAP *destinationBitmap, const Vector &bitmapPos, bool swapSourceWithDestination) {
			std::array<BITMAP *, 2> bitmaps = { sourceBitmap, destinationBitmap };
			std::array<Vector, 5> srcPos = {
				Vector(bitmapPos.GetX(), bitmapPos.GetY()),
				Vector(bitmapPos.GetX() + static_cast<float>(g_SceneMan.GetSceneWidth()), bitmapPos.GetY()),
				Vector(bitmapPos.GetX() - static_cast<float>(g_SceneMan.GetSceneWidth()), bitmapPos.GetY()),
				Vector(bitmapPos.GetX(), bitmapPos.GetY() + static_cast<float>(g_SceneMan.GetSceneHeight())),
				Vector(bitmapPos.GetX(), bitmapPos.GetY() - static_cast<float>(g_SceneMan.GetSceneHeight()))
			};
			std::array<Vector, 5> destPos;
			destPos.fill(Vector());

			if (swapSourceWithDestination) {
				std::swap(bitmaps[0], bitmaps[1]);
				std::swap(srcPos, destPos);
			}
			masked_blit(bitmaps[0], bitmaps[1], srcPos[0].GetFloorIntX(), srcPos[0].GetFloorIntY(), destPos[0].GetFloorIntX(), destPos[0].GetFloorIntY(), destinationBitmap->w, destinationBitmap->h);
			if (g_SceneMan.SceneWrapsX()) {
				if (bitmapPos.GetFloorIntX() < 0) {
					masked_blit(bitmaps[0], bitmaps[1], srcPos[1].GetFloorIntX(), srcPos[1].GetFloorIntY(), destPos[1].GetFloorIntX(), destPos[1].GetFloorIntY(), destinationBitmap->w, destinationBitmap->h);
				} else if (bitmapPos.GetFloorIntX() + destinationBitmap->w > g_SceneMan.GetSceneWidth()) {
					masked_blit(bitmaps[0], bitmaps[1], srcPos[2].GetFloorIntX(), srcPos[2].GetFloorIntY(), destPos[2].GetFloorIntX(), destPos[2].GetFloorIntY(), destinationBitmap->w, destinationBitmap->h);
				}
			}
			if (g_SceneMan.SceneWrapsY()) {
				if (bitmapPos.GetFloorIntY() < 0) {
					masked_blit(bitmaps[0], bitmaps[1], srcPos[3].GetFloorIntX(), srcPos[3].GetFloorIntY(), destPos[3].GetFloorIntX(), destPos[3].GetFloorIntY(), destinationBitmap->w, destinationBitmap->h);
				} else if (bitmapPos.GetFloorIntY() + destinationBitmap->h > g_SceneMan.GetSceneHeight()) {
					masked_blit(bitmaps[0], bitmaps[1], srcPos[4].GetFloorIntX(), srcPos[4].GetFloorIntY(), destPos[4].GetFloorIntX(), destPos[4].GetFloorIntY(), destinationBitmap->w, destinationBitmap->h);
				}
			}
		};
		BITMAP *tempBitmap = g_SceneMan.GetIntermediateBitmapForSettlingIntoTerrain(static_cast<int>(GetDiameter()));
		Vector tempBitmapPos = m_Pos.GetFloored() - Vector(static_cast<float>(tempBitmap->w / 2), static_cast<float>(tempBitmap->w / 2));

		clear_bitmap(tempBitmap);
		// Draw the object to the temp bitmap, then draw the foreground layer on top of it, then draw it to the foreground layer.
		Draw(tempBitmap, tempBitmapPos, DrawMode::g_DrawColor, true);
		wrappedMaskedBlit(terrain->GetFGColorBitmap(), tempBitmap, tempBitmapPos, false);
		wrappedMaskedBlit(terrain->GetFGColorBitmap(), tempBitmap, tempBitmapPos, true);

		clear_bitmap(tempBitmap);
		// Draw the object to the temp bitmap, then draw the material layer on top of it, then draw it to the material layer.
		Draw(tempBitmap, tempBitmapPos, DrawMode::g_DrawMaterial, true);
		wrappedMaskedBlit(terrain->GetMaterialBitmap(), tempBitmap, tempBitmapPos, false);
		wrappedMaskedBlit(terrain->GetMaterialBitmap(), tempBitmap, tempBitmapPos, true);

		terrain->AddUpdatedMaterialArea(Box(tempBitmapPos, static_cast<float>(tempBitmap->w), static_cast<float>(tempBitmap->h)));
		g_SceneMan.RegisterTerrainChange(tempBitmapPos.GetFloorIntX(), tempBitmapPos.GetFloorIntY(), tempBitmap->w, tempBitmap->h, ColorKeys::g_MaskColor, false);
	} else {
		Draw(terrain->GetFGColorBitmap(), Vector(), DrawMode::g_DrawColor, true);
		Material const *terrMat = g_SceneMan.GetMaterialFromID(g_SceneMan.GetTerrain()->GetMaterialPixel(m_Pos.GetFloorIntX(), m_Pos.GetFloorIntY()));
		if (GetMaterial()->GetPriority() > terrMat->GetPriority()) {
			Draw(terrain->GetMaterialBitmap(), Vector(), DrawMode::g_DrawMaterial, true);
		}
		g_SceneMan.RegisterTerrainChange(m_Pos.GetFloorIntX(), m_Pos.GetFloorIntY(), 1, 1, DrawMode::g_DrawColor, false);
	}
	return true;
}

} // namespace RTE