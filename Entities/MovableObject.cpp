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
#include "PresetMan.h"
#include "SceneMan.h"
#include "SettingsMan.h"
#include "LuaMan.h"
#include "Atom.h"


using namespace std;

namespace RTE
{

ABSTRACTCLASSINFO(MovableObject, SceneObject)

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
    m_MOIDFootprint = 0;
    m_AlreadyHitBy.clear();
    m_VelOscillations = 0;
    m_ToSettle = false;
    m_ToDelete = false;
    m_HUDVisible = true;
    m_ScriptPath.clear();
    m_ScriptPresetName.clear();
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
	m_RemoveOrphanTerrainRadius = 0;
	m_RemoveOrphanTerrainMaxArea = 0;
	m_RemoveOrphanTerrainRate = 0.0;
	m_DamageOnCollision = 0.0;
	m_DamageOnPenetration = 0.0;
	m_WoundDamageMultiplier = 1.0;
	m_IgnoreTerrain = false;

	m_MOIDHit = g_NoMOID;
	m_TerrainMatHit = g_MaterialAir;
	m_ParticleUniqueIDHit = 0;

	m_ProvidesPieMenuContext = false;
	m_pPieMenuActor = 0;
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
    m_ScriptPath = reference.m_ScriptPath;
    m_ScriptPresetName = reference.m_ScriptPresetName;
    // Should be unique to the object, will be created lazily upon first UpdateScript
//    m_ScriptObjectName
    if (reference.m_pScreenEffect)
    {
        m_ScreenEffectFile = reference.m_ScreenEffectFile;
        m_pScreenEffect = reference.m_pScreenEffect;

    }
	m_EffectRotAngle = reference.m_EffectRotAngle;
	m_InheritEffectRotAngle = reference.m_InheritEffectRotAngle;
	m_RandomizeEffectRotAngle = reference.m_RandomizeEffectRotAngle;
	m_RandomizeEffectRotAngleEveryFrame = reference.m_RandomizeEffectRotAngleEveryFrame;

	if (m_RandomizeEffectRotAngle)
		m_EffectRotAngle = PI * 2 * NormalRand();

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

	m_UniqueID = MovableObject::GetNextUniqueID();
	g_MovableMan.RegisterObject(this);

	m_ProvidesPieMenuContext = reference.m_ProvidesPieMenuContext;
	m_pPieMenuActor = reference.m_pPieMenuActor;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int MovableObject::ReadProperty(std::string propName, Reader &reader)
{
	if (propName == "Mass")
	{
		reader >> m_Mass;
		if (m_Mass == 0)
			m_Mass = 0.0001;
	}
	else if (propName == "Velocity")
		reader >> m_Vel;
	else if (propName == "Scale")
		reader >> m_Scale;
	else if (propName == "GlobalAccScalar")
		reader >> m_GlobalAccScalar;
	else if (propName == "AirResistance")
	{
		reader >> m_AirResistance;
		// Backwards compatibility after we made this value scaled over time
		m_AirResistance /= 0.01666;
	}
	else if (propName == "AirThreshold")
		reader >> m_AirThreshold;
	else if (propName == "PinStrength")
		reader >> m_PinStrength;
	else if (propName == "RestThreshold")
		reader >> m_RestThreshold;
	else if (propName == "LifeTime")
		reader >> m_Lifetime;
	else if (propName == "Sharpness")
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
	else if (propName == "ProvidesPieMenuContext")
		reader >> m_ProvidesPieMenuContext;
	else if (propName == "AddPieSlice")
	{
		PieMenuGUI::Slice newSlice;
		reader >> newSlice;
		PieMenuGUI::AddAvailableSlice(newSlice);
	}
	else if (propName == "ScriptPath")
    {
        reader >> m_ScriptPath;
        // Read in the Lua script function definitions for this preset
        LoadScripts(m_ScriptPath);
    }
    else if (propName == "ScreenEffect")
    {
        reader >> m_ScreenEffectFile;
        m_pScreenEffect = m_ScreenEffectFile.GetAsBitmap();
		m_ScreenEffectHash = m_ScreenEffectFile.GetHash();

		//char buf[128];
		//sprintf(buf, "%s %lu", m_ScreenEffectFile.GetDataPath().c_str(), m_ScreenEffectHash);
		//g_ConsoleMan.PrintString(buf);
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
        m_EffectStartStrength = floorf((float)255 * strength);
    }
    else if (propName == "EffectStopStrength")
    {
        float strength;
        reader >> strength;
        m_EffectStopStrength = floorf((float)255 * strength);
    }
    else if (propName == "EffectAlwaysShows")
        reader >> m_EffectAlwaysShows;
	else if (propName == "DamageOnCollision")
		reader >> m_DamageOnCollision;
	else if (propName == "DamageOnPenetration")
		reader >> m_DamageOnPenetration;
	else if (propName == "WoundDamageMultiplier")
		reader >> m_WoundDamageMultiplier;
	else if (propName == "IgnoreTerrain")
		reader >> m_IgnoreTerrain;
	else
        // See if the base class(es) can find a match instead
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
/*
    writer.NewProperty("Mass");
    writer << m_Mass;
    writer.NewProperty("Velocity");
    writer << m_Vel;
    writer.NewProperty("Scale");
    writer << m_Scale;
    writer.NewProperty("GlobalAccScalar");
    writer << m_GlobalAccScalar;
    writer.NewProperty("AirResistance");
    writer << m_AirResistance;
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
    if (!m_ScriptPath.empty())
    {
        writer.NewProperty("ScriptPath");
        writer << m_ScriptPath;
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
*/
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the MovableObject object.

void MovableObject::Destroy(bool notInherited)
{
    // Clean up the existence of this in the script state
    if (!m_ScriptObjectName.empty())
    {
        // Call the scripted destruction function, but only after first checking if it and this instance's Lua representation really exists
        g_LuaMan.RunScriptString("if " + m_ScriptPresetName + " and " + m_ScriptPresetName + ".Destroy and " + m_ScriptObjectName + " then " + m_ScriptPresetName + ".Destroy(" + m_ScriptObjectName + "); end");
        // Assign nil to the variable that held this' representation in Lua
        g_LuaMan.RunScriptString("if " + m_ScriptObjectName + " then " + m_ScriptObjectName + " = nil; end");
    }

    if (!notInherited)
        SceneObject::Destroy();
    Clear();

	g_MovableMan.UnregisterObject(this);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LoadScripts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads the preset scripts of this object, from a specified path.

int MovableObject::LoadScripts(string scriptPath)
{
    if (scriptPath.empty())
        return -1;

    // Read in the Lua script function definitions for this preset
    int error = 0;

    // Save the script path
    m_ScriptPath = scriptPath;

    // Clear the temporary variable names that will hold the functions read in from the file
    if ((error = g_LuaMan.RunScriptString("Create = nil; Destroy = nil; Update = nil;")) < 0)
        return error;

    // Run the file that specifies the Lua functions for this' operating logic
    if ((error = g_LuaMan.RunScriptFile(m_ScriptPath)) < 0)
        return error;

    // Create a new table for all presets and object instances of this class, to organize things a bit
    if ((error = g_LuaMan.RunScriptString("if not " + GetClassName() + "s then " + GetClassName() + "s = {}; end")) < 0)
        return error;
// TODO WAIT A MINUTE.. is this an original preset????!! .. does it matter? A: not really
    // Get a new ID for this original preset so we can assign the read-in function definitions to it
    m_ScriptPresetName = GetClassName() + "s." + g_LuaMan.GetNewPresetID();

    // Clear out the instance object name so it gets created in the state upon first UpdateScript
    m_ScriptObjectName.clear();

    // Under the class' table, create a new table for all functions of this specific preset and its unique ID
    if ((error = g_LuaMan.RunScriptString(m_ScriptPresetName + " = {};")) < 0)
        return error;

    // Now assign the different functions read in from the script file to the permanent locations of this original preset's table
    if ((error = g_LuaMan.RunScriptString("if Create then " + m_ScriptPresetName + ".Create = Create; end;")) < 0)
        return error;
    if ((error = g_LuaMan.RunScriptString("if Destroy then " + m_ScriptPresetName + ".Destroy = Destroy; end;")) < 0)
        return error;
    if ((error = g_LuaMan.RunScriptString("if Update then " + m_ScriptPresetName + ".Update = Update; end;")) < 0)
        return error;
	if ((error = g_LuaMan.RunScriptString("if OnPieMenu then " + m_ScriptPresetName + ".OnPieMenu = OnPieMenu; end;")) < 0)
		return error;

    return error;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReloadScripts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reloads the preset scripts of this object, from the same script file
//                  path as was originally defined. This will also update the original
//                  preset in the PresetMan with the updated scripts so future objects
//                  spawned will use the new scripts.

int MovableObject::ReloadScripts()
{
    int error = 0;

    // Read in the Lua script function definitions for this preset
    if (!m_ScriptPath.empty())
    {
        // Reload the preset funcitons of this instance
        if ((error = LoadScripts(m_ScriptPath)) < 0)
            return error;
        // Now also reload the ones of the original preset of this so all future objects will use the new scripts
        MovableObject *pPreset = const_cast<MovableObject *>(dynamic_cast<const MovableObject *>(g_PresetMan.GetEntityPreset(GetClassName(), GetPresetName(), GetModuleID())));
        if (pPreset && pPreset != this)
        {
            if ((error = pPreset->LoadScripts(m_ScriptPath)) < 0)
            return error;
        }
    }

    return error;
}

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

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAltitude
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the altitide of this' pos (or appropriate low point) over the
//                  terrain, in pixels.

float MovableObject::GetAltitude(int max, int accuracy)
{
    return g_SceneMan.FindAltitude(m_Pos, max, accuracy);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RestDetection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does the calculations necessary to detect whether this MO appears to
//                  have has settled in the world and is at rest or not. IsAtRest()
//                  retreves the answer.

void MovableObject::RestDetection()
{
    if (m_PinStrength)
        return;

    // Translational settling detection
    if ((m_Vel.Dot(m_PrevVel) < 0)) {
        if (m_VelOscillations >= 2 && m_RestThreshold >= 0)
            m_ToSettle = true;
        else
            ++m_VelOscillations;
    }
    else
        m_VelOscillations = 0;

//    if (fabs(m_Vel.m_X) >= 0.25 || fabs(m_Vel.m_Y) >= 0.25)
//        m_RestTimer.Reset();

    if (fabs(m_Pos.m_X - m_PrevPos.m_X) >= 1.0f || fabs(m_Pos.m_Y - m_PrevPos.m_Y) >= 1.0f)
        m_RestTimer.Reset();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsAtRest
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates wheter the MovableObject has been at rest (no velocity) for
//                  more than one (1) second.

bool MovableObject::IsAtRest()
{
    if (m_PinStrength)
        return false;

    if (m_RestThreshold < 0)
        return false;
    else
        return m_RestTimer.IsPastSimMS(m_RestThreshold);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  OnMOHit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits another MO.
//                  This is called by the owned Atom/AtomGroup of this MovableObject during
//                  travel.

bool MovableObject::OnMOHit(HitData &hd)
{
    return hd.terminate[hd.pRootBody[HITOR] == this ? HITOR : HITEE] = OnMOHit(hd.pRootBody[hd.pRootBody[HITOR] == this ? HITEE : HITOR]);
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

    // Apply the translational effects of all the forces accumulated during the Update()
    for (deque<pair<Vector, Vector> >::iterator fItr = m_Forces.begin(); fItr != m_Forces.end(); ++fItr)
    {
        // Continuous force application to transformational velocity.
        // (F = m * a -> a = F / m).
        m_Vel += ((*fItr).first / GetMass()) * deltaTime;
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
    for (deque<pair<Vector, Vector> >::iterator iItr = m_ImpulseForces.begin(); iItr != m_ImpulseForces.end(); ++iItr) {
        // Impulse force application to the transformational velocity of this MO.
        // Don't timescale these because they're already in kg * m/s (as opposed to kg * m/s^2).
        m_Vel += (*iItr).first / GetMass();
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
    if (m_GetsHitByMOs)
    {
		if (g_SettingsMan.PreciseCollisions())
		{
			// Temporarily remove the representation of this from the scene MO layers
			Draw(g_SceneMan.GetMOIDBitmap(), Vector(), g_DrawNoMOID, true);
		}
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
    if (m_IgnoresAGHitsWhenSlowerThan > 0)
        m_IgnoresAtomGroupHits = m_Vel.GetLargest() < m_IgnoresAGHitsWhenSlowerThan;

    if (m_GetsHitByMOs)
    {
		if (g_SettingsMan.PreciseCollisions())
		{
			// Replace updated MOID representation to scene after Update
			Draw(g_SceneMan.GetMOIDBitmap(), Vector(), g_DrawMOID, true);
		}
        m_AlreadyHitBy.clear();
    }
    m_IsUpdated = true;

    // Check for age expiration
    if (m_Lifetime && m_AgeTimer.GetElapsedSimTimeMS() > m_Lifetime)
        m_ToDelete = true;

    // Check for stupid positions and velocities, but critical stuff can't go too fast
    if (!g_SceneMan.IsWithinBounds(m_Pos.m_X, m_Pos.m_Y, 100))
        m_ToDelete = true;

    // Fix speeds that are too high
    FixTooFast();

    // Never let mission critical stuff settle or delete
    if (m_MissionCritical)
        m_ToSettle = false;

    // Reset the terrain intersection warning
    m_CheckTerrIntersection = false;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Pure v. method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MovableObject. Supposed to be done every frame. This also
//                  applies and clear the accumulated impulse forces (impulses), and the
//                  transferred forces of MOs attached to this.

void MovableObject::Update()
{
    return;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateScript
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MovableObject's Lua script. Supposed to be done every
//                  frame after the rest of the hardcoded C++ update is done.

int MovableObject::UpdateScript()
{
    // This preset doesn't seem to have any script file defined, so then just return
    if (m_ScriptPath.empty() || m_ScriptPresetName.empty())
        return -1;

    int error = 0;

    // Check to make sure the preset of this is still defined in the Lua state. If not, re-create it and recover gracefully
    if (!g_LuaMan.ExpressionIsTrue(m_ScriptPresetName, false))
        ReloadScripts();

    // First see if we even have a representation stored in the Lua state, and if not, create one
    if (m_ScriptObjectName.empty())
    {
        // Get the unique object identifier for this object and construct the object isntance name in Lua that points to this object so we can pass it into the preset functions
        m_ScriptObjectName = GetClassName() + "s." + g_LuaMan.GetNewObjectID();

        // Give access to this in the Lua state
        g_MovableMan.SetScriptedEntity(this);
        // Create the Lua variable which will hold the object instance of this instance for as long as it exists
        if ((error = g_LuaMan.RunScriptString(m_ScriptObjectName + " = To" + GetClassName() + "(MovableMan.ScriptedEntity);")) < 0)
            return error;

        // Call the scripted creation function, but only after first checking if it and this instance's Lua representation really exists
        if ((error = g_LuaMan.RunScriptString("if " + m_ScriptPresetName + ".Create and " + m_ScriptObjectName + " then " + m_ScriptPresetName + ".Create(" + m_ScriptObjectName + "); end")) < 0)
            return error;
    }

    // Call the defined function, but only after first checking if it and this instance's Lua representation exists
    if ((error = g_LuaMan.RunScriptString("if " + m_ScriptPresetName + ".Update and " + m_ScriptObjectName + " then " + m_ScriptPresetName + ".Update(" + m_ScriptObjectName + "); end")) < 0)
        return error;

    return error;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  OnPieMenu
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Executes the Lua-defined OnPieMenu event handler.

int MovableObject::OnPieMenu(Actor * pActor)
{
	if (!pActor)
		return -1;

	// This preset doesn't seem to have any script file defined, so then just return
	if (m_ScriptPath.empty() || m_ScriptPresetName.empty())
		return -1;

	if (m_ScriptObjectName.empty())
		return -1;

	m_pPieMenuActor = pActor;

	int error = 0;

	if ((error = g_LuaMan.RunScriptString("if " + m_ScriptPresetName + ".OnPieMenu and " + m_ScriptObjectName + " then " + m_ScriptPresetName + ".OnPieMenu(" + m_ScriptObjectName + "); end")) < 0)
		return error;

	return error;
}

void MovableObject::Update()
{
	if (m_RandomizeEffectRotAngleEveryFrame)
		m_EffectRotAngle = PI * 2 * NormalRand();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateMOID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this' and its childrens MOID status. Supposed to be done every frame.

void MovableObject::UpdateMOID(vector<MovableObject *> &MOIDIndex,
                               int rootMOID,
                               bool makeNewMOID)
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
	if (m_MOID != g_NoMOID)
		MOIDs.push_back(m_MOID);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RegMOID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this MO register itself in the MOID register and get ID:s for
//                  itself and its children for this frame.
//                  BITMAP of choice.

void MovableObject::RegMOID(vector<MovableObject *> &MOIDIndex,
                            MOID rootMOID,
                            bool makeNewMOID)
{
    // Make a new MOID for itself
    if (makeNewMOID)
    {
		// Skip g_NoMOID item
		if (MOIDIndex.size() == g_NoMOID)
			MOIDIndex.push_back(0);

		m_MOID = MOIDIndex.size();
		MOIDIndex.push_back(this);
    }
    // Use the parent's MOID instead (the two are considered the same MO)
    else
        m_MOID = MOIDIndex.size() - 1;

    // Assign the root MOID
    m_RootMOID = (rootMOID == g_NoMOID ? m_MOID : rootMOID);

}

} // namespace RTE