//////////////////////////////////////////////////////////////////////////////////////////
// File:            Sound.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Sound class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#ifdef __USE_SOUND_FMOD
#include "fmod.h"
#include "fmod_errors.h"
#elif __USE_SOUND_SDLMIXER
#include "SDL.h"
#include "SDL_mixer.h"
#elif __USE_SOUND_GORILLA
#include "gorilla/ga.h"
#include "gorilla/gau.h"
#endif

#include "Sound.h"
#include "DDTTools.h"
#include "AudioMan.h"

using namespace std;

namespace RTE
{

CONCRETECLASSINFO(Sound, Entity, 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Sound, effectively
//                  resetting the members of this abstraction level only.

void Sound::Clear()
{
#ifdef __USE_SOUND_FMOD    
	m_Samples.clear();
#elif __USE_SOUND_SDLMIXER
	m_Samples.clear();
#elif __USE_SOUND_GORILLA
	m_Samples.clear();
#endif
    m_CurrentSample = 0;
    m_LastChannel = -1;
    m_Loops = 0;
    m_Priority = AudioMan::PRIORITY_LOW;
    m_AffectedByPitch = true;
	m_Hash = 0;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Sound object ready for use.

int Sound::Create()
{
    if (Entity::Create() < 0)
        return -1;

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Sound to be identical to another, by deep copy.

int Sound::Create(const Sound &reference)
{
    Entity::Create(reference);

#ifdef __USE_SOUND_FMOD
	for (vector<pair<ContentFile, FSOUND_SAMPLE *> >::const_iterator itr = reference.m_Samples.begin(); itr != reference.m_Samples.end(); ++itr)
        m_Samples.push_back(*itr);
#elif __USE_SOUND_SDLMIXER
	for (vector<pair<ContentFile, Mix_Chunk *> >::const_iterator itr = reference.m_Samples.begin(); itr != reference.m_Samples.end(); ++itr)
		m_Samples.push_back(*itr);
#elif __USE_SOUND_GORILLA
	for (vector<pair<ContentFile, ga_Sound *> >::const_iterator itr = reference.m_Samples.begin(); itr != reference.m_Samples.end(); ++itr)
		m_Samples.push_back(*itr);
#endif // __USE_SOUND_FMOD

    m_CurrentSample = reference.m_CurrentSample;
    m_LastChannel = -1;
    m_Loops = reference.m_Loops;
    m_Priority = reference.m_Priority;
    m_AffectedByPitch = reference.m_AffectedByPitch;
	m_Hash = reference.m_Hash;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Sound and give it a path to its first sample/

int Sound::Create(std::string samplePath, bool pitched, int loops)
{
    AddSample(samplePath);

    m_AffectedByPitch = pitched;
	m_Loops = loops;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Sound::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "AddSample")
    {
        ContentFile newFile;
        reader >> newFile;
		m_Hash = newFile.GetHash();
#ifdef __USE_SOUND_FMOD
		FSOUND_SAMPLE *pNewSample = newFile.GetAsSample();
        if (!pNewSample)
        {
            reader.ReportError( string( "Failed to load the sample from the file, error: " ) + FMOD_ErrorString( FSOUND_GetError() ) );
        }
        m_Samples.push_back(pair<ContentFile, FSOUND_SAMPLE *>(newFile, pNewSample));
#elif __USE_SOUND_SDLMIXER
		if (g_AudioMan.IsAudioEnabled())
		{
			Mix_Chunk *pNewSample = newFile.GetAsSample();
			if (!pNewSample)
			{
				reader.ReportError(string("Failed to load the sample from the file, error: ") + Mix_GetError());
			}
			m_Samples.push_back(pair<ContentFile, Mix_Chunk *>(newFile, pNewSample));
		}
#elif __USE_SOUND_GORILLA
		if (g_AudioMan.IsAudioEnabled())
		{
			ga_Sound *pNewSample = newFile.GetAsSample();
			if (!pNewSample)
			{
				reader.ReportError(string("Failed to load the sample from the file, error: "));
			}
			m_Samples.push_back(pair<ContentFile, ga_Sound *>(newFile, pNewSample));
		}
#endif // __USE_SOUND_FMOD

    }
    else if (propName == "LoopSetting")
        reader >> m_Loops;
    else if (propName == "Priority")
        reader >> m_Priority;
    else if (propName == "AffectedByPitch")
        reader >> m_AffectedByPitch;
    else
        // See if the base class(es) can find a match instead
        return Entity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Sound with a Writer for
//                  later recreation with Create(Reader &reader);

int Sound::Save(Writer &writer) const
{
    Entity::Save(writer);

#ifdef __USE_SOUND_FMOD
	for (vector<pair<ContentFile, FSOUND_SAMPLE *> >::const_iterator itr = m_Samples.begin(); itr != m_Samples.end(); ++itr)
    {
        writer.NewProperty("AddSample");
        writer << (*itr).first;
    }
#elif __USE_SOUND_SDLMIXER
	for (vector<pair<ContentFile, Mix_Chunk *> >::const_iterator itr = m_Samples.begin(); itr != m_Samples.end(); ++itr)
	{
		writer.NewProperty("AddSample");
		writer << (*itr).first;
	}
#endif // __USE_SOUND_FMOD

    writer.NewProperty("LoopSetting");
    writer << m_Loops;
    writer.NewProperty("Priority");
    writer << m_Priority;
    writer.NewProperty("AffectedByPitch");
    writer << m_AffectedByPitch;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Sound object.

void Sound::Destroy(bool notInherited)
{
    // Don't delete samples since they are owned in the CoententFile static maps

    if (!notInherited)
        Entity::Destroy();
    Clear();
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Sound addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for Sound:s which adds together the
//                  Atom:s of two Groups and merges them into one.

Sound operator+(const Sound &lhs, const Sound &rhs)
{
    Sound returnAG(lhs);
    returnAG.SetOwner(lhs.GetOwner());
    returnAG.AddAtoms(rhs.GetAtomList());
    return returnAG;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Sound pointer addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for Sound:s pointer which adds together
//                  the Atom:s of two Groups pointed to and merges them into one.
//                  Ownership of the returned dallocated AG IS TRANSFERRED!

Sound * operator+(const Sound *lhs, const Sound *rhs)
{
    Sound *pReturnAG = new Sound(*lhs);
    pReturnAG->SetOwner(lhs->GetOwner());
    pReturnAG->AddAtoms(rhs->GetAtomList());
    return pReturnAG;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCurrentSample
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the currently played sample, if any.

#ifdef __USE_SOUND_FMOD
FSOUND_SAMPLE * Sound::GetCurrentSample()
{
    DAssert(m_CurrentSample >= 0 && m_CurrentSample < m_Samples.size(), "Sample index is out of bounds!");
    return m_Samples[m_CurrentSample].second;
}
#elif __USE_SOUND_SDLMIXER
Mix_Chunk * Sound::GetCurrentSample()
{
	DAssert(m_CurrentSample >= 0 && m_CurrentSample < m_Samples.size(), "Sample index is out of bounds!");
	return m_Samples[m_CurrentSample].second;
}
#elif __USE_SOUND_GORILLA
ga_Sound * Sound::GetCurrentSample()
{
	DAssert(m_CurrentSample >= 0 && m_CurrentSample < m_Samples.size(), "Sample index is out of bounds!");
	return m_Samples[m_CurrentSample].second;
}
#endif // __USE_SOUND_FMOD

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StartNextSample
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the next sample of this Sound to be played. It's a random
//                  selection of all the samples of this Sound. Note that this will change
//                  each time this method is called.

#ifdef __USE_SOUND_FMOD
FSOUND_SAMPLE * Sound::StartNextSample()
{
    // Only two samples; alternate
    if (m_Samples.size() == 2)
    {
        m_CurrentSample = m_CurrentSample == 0 ? 1 : 0;
    }
    // More than two, select randomly
    else if (m_Samples.size() > 2)
    {
        int lastSample = m_CurrentSample;
        m_CurrentSample = floorf((float)m_Samples.size() * PosRand());
        // Mix it up again if we got the same sound twice
        if (m_CurrentSample == lastSample)
            m_CurrentSample = floorf((float)m_Samples.size() * PosRand());
    }
    DAssert(m_CurrentSample >= 0 && m_CurrentSample < m_Samples.size(), "Sample index is out of bounds!");
    return m_Samples[m_CurrentSample].second;
}
#elif __USE_SOUND_SDLMIXER
Mix_Chunk * Sound::StartNextSample()
{
	// Only two samples; alternate
	if (m_Samples.size() == 2)
	{
		m_CurrentSample = m_CurrentSample == 0 ? 1 : 0;
	}
	// More than two, select randomly
	else if (m_Samples.size() > 2)
	{
		int lastSample = m_CurrentSample;
		m_CurrentSample = floorf((float)m_Samples.size() * PosRand());
		// Mix it up again if we got the same sound twice
		if (m_CurrentSample == lastSample)
			m_CurrentSample = floorf((float)m_Samples.size() * PosRand());
	}
	DAssert(m_CurrentSample >= 0 && m_CurrentSample < m_Samples.size(), "Sample index is out of bounds!");
	return m_Samples[m_CurrentSample].second;
}
#elif __USE_SOUND_GORILLA
	ga_Sound * Sound::StartNextSample()
	{
		// Only two samples; alternate
		if (m_Samples.size() == 2)
		{
			m_CurrentSample = m_CurrentSample == 0 ? 1 : 0;
		}
		// More than two, select randomly
		else if (m_Samples.size() > 2)
		{
			int lastSample = m_CurrentSample;
			m_CurrentSample = floorf((float)m_Samples.size() * PosRand());
			// Mix it up again if we got the same sound twice
			if (m_CurrentSample == lastSample)
				m_CurrentSample = floorf((float)m_Samples.size() * PosRand());
		}
		DAssert(m_CurrentSample >= 0 && m_CurrentSample < m_Samples.size(), "Sample index is out of bounds!");
		return m_Samples[m_CurrentSample].second;
	}
#endif // __USE_SOUND_FMOD


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateAttenuation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the distance attenuation of the sound while it's playing.

bool Sound::UpdateAttenuation(float distance)
{
    return g_AudioMan.SetSoundAttenuation(this, distance);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Play
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Plays the next sample of this Sound.

bool Sound::Play(float distance, int player)
{
#ifdef __USE_SOUND_FMOD
	if (!m_Samples.empty())
    {
        return g_AudioMan.PlaySound(player, this, m_Priority, distance);
    }
    else
        return false;
#elif __USE_SOUND_SDLMIXER
	if (!m_Samples.empty())
	{
		return g_AudioMan.PlaySound(player, this, m_Priority, distance);
	}
	else
		return false;
#elif __USE_SOUND_GORILLA
	if (!m_Samples.empty())
	{
		return g_AudioMan.PlaySound(player, this, m_Priority, distance);
	}
	else
		return false;
#else
		return false;
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Stop
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Stops playback of this Sound.

bool Sound::Stop(int player)
{
#ifdef	__USE_SOUND_FMOD
	if (!m_Samples.empty())
    {
        return g_AudioMan.StopSound(player, this);
    }
    else return false;
#elif __USE_SOUND_SDLMIXER
	if (!m_Samples.empty())
	{
		return g_AudioMan.StopSound(player, this);
	}
	else return false;
#elif __USE_SOUND_GORILLA
	if (!m_Samples.empty())
	{
		return g_AudioMan.StopSound(player, this);
	}
	else return false;
#else
	return false;
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FadeOut
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fades out playback of the currently played sound.

void Sound::FadeOut(int fadeOutTime)
{
    
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsBeingPlayed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this Sound is currently being played.

bool Sound::IsBeingPlayed()
{
    return g_AudioMan.IsPlaying(this);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddSample
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a new sample to this Sound's collection, loaded from a file.

void Sound::AddSample(string samplePath)
{
    ContentFile newFile(samplePath.c_str());
	m_Hash = newFile.GetHash();
#ifdef __USE_SOUND_FMOD
	FSOUND_SAMPLE *pNewSample = newFile.GetAsSample();
    AAssert(pNewSample, "Failed to load the sample from the file");
	m_Samples.push_back(pair<ContentFile, FSOUND_SAMPLE *>(newFile, pNewSample));
#elif __USE_SOUND_SDLMIXER
	if (g_AudioMan.IsAudioEnabled())
	{
		Mix_Chunk *pNewSample = newFile.GetAsSample();
		AAssert(pNewSample, "Failed to load the sample from the file");
		m_Samples.push_back(pair<ContentFile, Mix_Chunk *>(newFile, pNewSample));
	}
#elif __USE_SOUND_GORILLA
	if (g_AudioMan.IsAudioEnabled())
	{
		ga_Sound *pNewSample = newFile.GetAsSample();
		AAssert(pNewSample, "Failed to load the sample from the file");
		m_Samples.push_back(pair<ContentFile, ga_Sound *>(newFile, pNewSample));
	}
#endif // __USE_SOUND_FMOD
}

} // namespace RTE
