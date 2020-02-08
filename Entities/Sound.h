#ifndef _RTESOUND_
#define _RTESOUND_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Sound.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Sound class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Entity.h"
#include "ContentFile.h"

#ifdef __USE_SOUND_FMOD
struct FSOUND_SAMPLE;
#elif __USE_SOUND_SDLMIXER
struct Mix_Chunk;
#elif __USE_SOUND_SDLMIXER
struct ga_Handle;
#endif

namespace RTE
{   

//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Sound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     One or more sound samples that all represent a specific sound effect.
// Parent(s):       Entity.
// Class history:   08/18/2006 Sound created.

class Sound:
    public Entity
{
    friend class AudioMan;


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
ENTITYALLOCATION(Sound)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Sound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Sound object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    Sound() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Sound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a Sound object
//                  identical to an already existing one.
// Arguments:       A Sound object which is passed in by reference.

    Sound(const Sound &reference) { Clear(); Create(reference); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Sound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Sound object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~Sound() { Destroy(true); }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Sound object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Sound to be identical to another, by deep copy.
// Arguments:       A reference to the Sound to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const Sound &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Sound and give it a path to its first sample/
// Arguments:       A path to the sound sample for this sound to have.
//                  Whether this sound's frequency will be pitched at all.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(std::string samplePath, bool pitched = true, int loops = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a Reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the Reader's position is untouched.
// Arguments:       The name of the property to be read.
//                  A Reader lined up to the value of the property to be read.
// Return value:    An error return value signaling whether the property was successfully
//                  read or not. 0 means it was read successfully, and any nonzero indicates
//                  that a property of that name could not be found in this or base classes.

    virtual int ReadProperty(std::string propName, Reader &reader);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire Sound, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); Entity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Sound to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the Sound will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Sound object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    virtual void Destroy(bool notInherited = false);

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Sound addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for Sound:s which adds together the
//                  Atom:s of two Groups and merges them into one.
// Arguments:       An Sound reference as the left hand side operand..
//                  An Sound reference as the right hand side operand.
// Return value:    The resulting Sound.

    friend Sound operator+(const Sound &lhs, const Sound &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Sound pointer addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for Sound:s pointer which adds together
//                  the Atom:s of two Groups pointed to and merges them into one.
//                  Ownership of the returned dallocated AG IS TRANSFERRED!
// Arguments:       An Sound pointer as the left hand side operand.
//                  An Sound pointer as the right hand side operand.
// Return value:    The resulting dynamically allocated Sound. OWNERSHIP IS TRANSFERRED!

    friend Sound * operator+(const Sound *lhs, const Sound *rhs);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ClassInfo instance of this Entity.
// Arguments:       None.
// Return value:    A reference to the ClassInfo of this' class.

    virtual const Entity::ClassInfo & GetClass() const { return m_sClass; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_sClass.GetName(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSampleList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current list of samples in the Sound.
// Arguments:       None.
// Return value:    A reference to the list.

#ifdef __USE_SOUND_FMOD
	const std::vector<std::pair<ContentFile, FSOUND_SAMPLE *> > & GetSampleList() const { return m_Samples; }
#elif __USE_SOUND_SDLMIXER
	const std::vector<std::pair<ContentFile, Mix_Chunk *> > & GetSampleList() const { return m_Samples; }
#elif __USE_SOUND_GORILLA
	const std::vector<std::pair<ContentFile, ga_Sound *> > & GetSampleList() const { return m_Samples; }
#endif // __USE_SOUND_FMOD


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasAnySamples
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this Sound has been initialized at all yet and loaded
//                  with any samples.
// Arguments:       None.
// Return value:    Whether this Sound has any samples.

    bool HasAnySamples() const { 
#ifdef __USE_SOUND_FMOD
		return !m_Samples.empty(); 
#elif __USE_SOUND_SDLMIXER
		return !m_Samples.empty();
#elif __USE_SOUND_GORILLA
		return !m_Samples.empty();
#else
		return false;
#endif
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSampleCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current number of Samples that make up the Sound.
// Arguments:       None.
// Return value:    An int with the count.

    int GetSampleCount() const {
#ifdef __USE_SOUND_FMOD
		return m_Samples.size(); 
#elif __USE_SOUND_SDLMIXER
		return m_Samples.size();
#elif __USE_SOUND_GORILLA
		return m_Samples.size();
#else
		return 0;
#endif
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLoopSetting
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the looping setting of this Sound. 0 means the sound is set to
//                  only play once. -1 means it loops indefinitely.
// Arguments:       None.
// Return value:    An int with the loop count.

    int GetLoopSetting() const { return m_Loops; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLoopSetting
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the looping setting of this Sound. 0 means the sound is set to
//                  only play once. -1 means it loops indefinitely.
// Arguments:       An int with the loop count.
// Return value:    None,

	void SetLoopSetting(int loops) { m_Loops = loops; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPriority
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current playback priority.. higher the more likely the sound
//                  is to make it into the mixing.
// Arguments:       None.
// Return value:    The playback priority.

    int GetPriority() const { return m_Priority; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCurrentChannel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets current channel that one of the samples of this Sound is being
//                  played on. 
// Arguments:       None.
// Return value:    The current channel. -1 if none or not being played.

    int GetCurrentChannel() const { return m_LastChannel; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCurrentSample
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the currently played sample, if any.
// Arguments:       None.
// Return value:    A pointer to the currently played sample, or 0 if no sample of this
//                  Sound is currently being played. Ownership is NOT transferred.

#ifdef __USE_SOUND_FMOD
	FSOUND_SAMPLE * GetCurrentSample();
#elif __USE_SOUND_SDLMIXER
	Mix_Chunk * GetCurrentSample();
#elif __USE_SOUND_GORILLA
	ga_Sound * GetCurrentSample();
#endif // __USE_SOUND_FMOD


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StartNextSample
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets and sets the next sample of this Sound to be played. It's a random
//                  selection of all the samples of this Sound. Note that this will change
//                  each time this method is called.
// Arguments:       None.
// Return value:    A pointer to the sample to be played. Ownership is NOT transferred.

#ifdef __USE_SOUND_FMOD
	FSOUND_SAMPLE * StartNextSample();
#elif __USE_SOUND_SDLMIXER
	Mix_Chunk * StartNextSample();
#elif __USE_SOUND_GORILLA
	ga_Sound * StartNextSample();
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetChannel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current channel that is being played on by this Sound.
// Arguments:       A number of the new channel. -1 for no channel.
// Return value:    None.

    void SetChannel(int channel) { m_LastChannel = channel; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPriority
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current playback priority. Higher value will make this more
//                  likely to make it into mixing on playback.
// Arguments:       The new priority. See AudioMan::PRIORITY_* enum
// Return value:    None.

    void SetPriority(int priority) { m_Priority = priority; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetAffectedByPitch
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this is affected by (global) pitch changes or not.
// Arguments:       Whether this should be affected by pitch or not.
// Return value:    None.

    void SetAffectedByPitch(bool pitched = true) { m_AffectedByPitch = pitched; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateAttenuation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the distance attenuation of the sound while it's playing.
// Arguments:       How much distance attenuation to apply to the sound. 0 = full volume
//                  1.0 = max distant, but still won't be completely silent.
// Return value:    Whether this Sound's attenuation setting was successful.

    bool UpdateAttenuation(float distance = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Play
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Plays the next sample of this Sound. 
// Arguments:       How much distance attenuation to apply to the sound. 0 = full volume
//                  1.0 = max distant, but still won't be completely silent.
// Return value:    Whether this Sound successfully started playing on a channel.

    bool Play(float distance = 0, int player = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Stop
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Stops playback of this Sound.
// Arguments:       None.
// Return value:    Whether this Sound successfully stopped playing.

    bool Stop(int player = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FadeOut
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fades out playback of the currently played sound.
// Arguments:       The time, in ms, of how long to fade out for.
// Return value:    None.

    void FadeOut(int fadeOutTime = 1000);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsBeingPlayed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this Sound is currently being played.
// Arguments:       None.
// Return value:    Whether this Sound is being played on a channel.

    bool IsBeingPlayed();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddSample
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a new sample to this Sound's collection, loaded from a file.
// Arguments:       A path to the new sample to add. This will be handled through PresetMan.
// Return value:    None.

    void AddSample(std::string samplePath);


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Sound, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    static Entity::ClassInfo m_sClass;

	// All the SDL samples of this Sound, they are NOT owned by this, but owned by the Contentfile static maps
#ifdef __USE_SOUND_FMOD    
	std::vector<std::pair<ContentFile, FSOUND_SAMPLE *> > m_Samples;
#elif __USE_SOUND_SDLMIXER
	std::vector<std::pair<ContentFile, Mix_Chunk *> > m_Samples;
#elif __USE_SOUND_GORILLA
	std::vector<std::pair<ContentFile, ga_Sound *> > m_Samples;
#endif // __USE_SOUND_FMOD
    // Index of the current (or last, if nothing is being played) sample being played
    int m_CurrentSample;
    // Current channel the current Sample of this Sound is being played on.
    int m_LastChannel;
    // Number of loops (repeats) the sound should play when played. 0 means it plays once, -1 means it plays until stopped 
    int m_Loops;
    // The mixing priority of this, the higher the more likely it's to be mixed and heard
    int m_Priority;
    // Whether this should be altered by pitch
    bool m_AffectedByPitch;

	size_t m_Hash;
};

} // namespace RTE

#endif // File