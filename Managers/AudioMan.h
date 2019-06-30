#ifndef _RTEAUDIOMAN_
#define _RTEAUDIOMAN_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            AudioMan.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the AudioMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include <list>
#include <string>

#include "DDTTools.h"
#include "Singleton.h"
#define g_AudioMan AudioMan::Instance()

#include "Entity.h"
#include "Timer.h"

#define MAX_CLIENTS 4

#ifdef __USE_SOUND_FMOD
struct FSOUND_SAMPLE;
struct FSOUND_STREAM;
#elif __USE_SOUND_SDLMIXER
//#include "SDL.h"
//#include "SDL_mixer.h"
struct Mix_Chunk;
typedef struct _Mix_Music Mix_Music;
#endif

namespace RTE
{

class Sound;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           AudioMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The singleton manager of the WAV sound effects and OGG music playback.
// Parent(s):       Singleton, Serializable.
// Class history:   10/09/2004 AudioMan created.
//                  8/18/2006 AudioMan changed to use SDL and SDL_mixer.

class AudioMan:
    public Singleton<AudioMan>
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

// Playback priorities
enum PlaybackPriority
{
    PRIORITY_LOW = 0,
    PRIORITY_HIGH = 100,
    PRIORITY_NOATTENUATION,
    PRIORITY_COUNT
};


enum NetworkSoundState
{
	SOUND_PLAY = 0,
	SOUND_STOP,
	SOUND_SET_PITCH,
	SOUND_SET_ATTENUATION
};

enum NetworkMusicState
{
	MUSIC_PLAY = 0,
	MUSIC_STOP,
	MUSIC_SILENCE,
	MUSIC_SET_PITCH
};

struct SoundNetworkData
{
	unsigned char State;
	unsigned char AffectedByPitch;
	size_t SoundHash;
	short int Distance;
	short int Channel;
	short int Loops;
	float Pitch;
};

struct MusicNetworkData
{
	unsigned char State;
	char Path[256];
	int Loops;
	double Position;
	float Pitch;
};


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     AudioMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a AudioMan object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    AudioMan() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~AudioMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a AudioMan object before deletion
//                  from system memory.
// Arguments:       None.

    ~AudioMan() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the AudioMan object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this AudioMan to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the AudioMan will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire AudioMan, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the AudioMan object.
// Arguments:       None.
// Return value:    None.

    void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_ClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSoundsVolume
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the volume of all sounds to a specific volume.
//                  Does not affect music.
// Arguments:       The desired volume scalar. 0.0-1.0.
// Return value:    None.

    void SetSoundsVolume(double volume = 1.0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMusicVolume
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the music to a specific volume.
// Arguments:       The desired volume scalar. 0.0-1.0.
// Return value:    None.

    void SetMusicVolume(double volume = 1.0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTempMusicVolume
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the music to a specific volume, but it will onyl last until a new
//                  song is palyed. Useful for fading etc.
// Arguments:       The desired volume scalar. 0.0-1.0.
// Return value:    None.

    void SetTempMusicVolume(double volume = 1.0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSoundsVolume
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the volume of all sounds to a specific volume.
//                  Does not affect music.
// Arguments:       None.
// Return value:    The desired volume scalar. 0.0-1.0.

    double GetSoundsVolume() const { return m_SoundsVolume; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetGlobalPitch
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the global pitch multiplier for all sounds, optionally the music
//                  too.
// Arguments:       The desired pitch multiplier. Keep it > 0
//                  Whether to exclude the music from pitch modification
// Return value:    None.

    void SetGlobalPitch(double pitch = 1.0, bool excludeMusic = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGlobalPitch
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the volume of all sounds to a specific volume. Does not affect
//                  music.
// Arguments:       None.
// Return value:    The current pitch scalar. Will be > 0

    double GetGlobalPitch() const { return m_GlobalPitch; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMusicVolume
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the music to a specific volume.
// Arguments:       The desired volume scalar. 0.0-1.0.
// Return value:    None.

    double GetMusicVolume() const { return m_MusicVolume; }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlayModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Starts playing a certain music module
// Arguments:       The path to the IT/MOD//S3M/XM file to play
// Return value:    None.

    void PlayModule(const char *modfilepath);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlayMusic
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Starts playing a certain WAVE, MOD, MIDI, OGG, MP3 file in the music
//                  channel.
// Arguments:       The path to the music file to play.
//                  The number of times to loop the song. 0 means play once. -1 means
//                  play infinitely until stopped.
//                  The volume override for music for this song only. < 0 means no override.
// Return value:    None.

    void PlayMusic(const char *filepath, int loops = -1, double volumeOverride = -1.0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          QueueMusicStream
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Queues up another path to a stream that will be played after the current
//                  one is done. Can be done several times to queue up many tracks. The
//                  last track in the list will be looped infinitely.
// Arguments:       The path to the music file to play after the current one.
// Return value:    None.

    void QueueMusicStream(const char *filepath);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          QueueSilence
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Queues up a period of silence in teh music stream playlist.
// Arguments:       The number of secs to wait before going to the next stream.
// Return value:    None.

    void QueueSilence(int seconds);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearMusicQueue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the music queue.
// Arguments:       None.
// Return value:    None.

    void ClearMusicQueue() { m_MusicPlayList.clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlayNextStream
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Plays the next music stream in the queue, if any is queued.
// Arguments:       None.
// Return value:    None.

    void PlayNextStream();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlaySound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Starts playing a certain sound sample
// Arguments:       The path to the .wav file to play
// Return value:    None.

    void PlaySound(const char *wavefile);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlaySound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Starts playing the next sample of a certain Sound.
// Arguments:       Pointer to the Sound to start playing. Ownership is NOT transferred!
//                  The priority of this sound; higher gives it a higher likelyhood of
//                  getting mixed compared to lower-priority samples.
//                  Distance attenuation scalar: 0 = full volume, 1.0 = max distant, but not silent.
//                  The pitch modifier for this sound. 1.0 yields unmodified frequency.
// Return value:    Whether or not playback of the Sound was successful.

    bool PlaySound(Sound *pSound, int priority = PRIORITY_LOW, float distance = 0.0, double pitch = 1.0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlaySound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Starts playing the next sample of a certain Sound.
// Arguments:       Pointer to the Sound to start playing. Ownership is NOT transferred!
//                  The priority of this sound; higher gives it a higher likelyhood of
//                  getting mixed compared to lower-priority samples.
//                  Distance attenuation scalar: 0 = full volume, 1.0 = max distant, but not silent.
//                  The pitch modifier for this sound. 1.0 yields unmodified frequency.
// Return value:    Whether or not playback of the Sound was successful.

	bool PlaySound(int player, Sound *pSound, int priority = PRIORITY_LOW, float distance = 0.0, double pitch = 1.0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlaySound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Starts playing a certain WAVE sound file.
// Arguments:       The path to the music file to play.
//                  Normalized distance from 0 to 1 to play the sound with.
//                  Wheher the sound should be affected by pitch.
//					For which network player to play the sound for, -1 for all.
// Return value:    Returns the new sound object being played. OWNERSHIP IS TRANSFERRED!!!

	Sound * PlaySound(const char *filepath, float distance, bool loop, bool affectedByPitch, int player);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSoundAttenuation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets/updates the distance attenuation for a specific sound. Will only
//                  have an effect if the sound is currently being played.
// Arguments:       A pointer to a Sound object. Ownership IS NOT transferred!
//                  Distance attenuation scalar: 0 = full volume, 1.0 = max distant, but not silent.
// Return value:    Whetehr a smaple of the Sound is currently being played by any of the
//                  channels, and the attenuation was successfully set.

    bool SetSoundAttenuation(Sound *pSound, float distance = 0.0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSoundPitch
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets/updates the frequency/pitch for a specific sound. Will only
//                  have an effect if the sound is currently being played.
// Arguments:       A pointer to a Sound object. Ownership IS NOT transferred!
//                  New pitch, a multiplier of the original normal frequency. Keep it > 0
// Return value:    Whether a sample of the Sound is currently being played by any of the
//                  channels, and the pitch was successfully set.

    bool SetSoundPitch(Sound *pSound, float pitch);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMusicPitch
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets/updates the frequency/pitch for the music channel.
// Arguments:       New pitch, a multiplier of the original normal frequency. Keep it > 0
// Return value:    Whether a sample of the Sound is currently being played by any of the
//                  channels, and the pitch was successfully set.

    bool SetMusicPitch(float pitch);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsPlaying
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whetehr a certain Sound's last played sample is being played
//                  currently.
// Arguments:       A pointer to a Sound object. Ownership IS NOT transferred!
// Return value:    Whether the LAST sample that was played of the Sound is currently being
//                  played by any of the channels.

    bool IsPlaying(Sound *pSound);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsMusicPlaying
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whetehr any music stream is currently playing.
// Arguments:       None.
// Return value:    Whether any music stream is currently playing.

    bool IsMusicPlaying() { 
#ifdef __USE_SOUND_FMOD
		return m_AudioEnabled && m_pMusic; 
#elif __USE_SOUND_SDLMIXER
		return m_AudioEnabled && m_pMusic;
#else
		return m_AudioEnabled;
#endif
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsAudioEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether audio is enabled.
// Arguments:       None.
// Return value:    Whether audio is enabled.

	bool IsAudioEnabled() {
		return m_AudioEnabled;
	}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StopSound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Stops playing a certain sound sample
// Arguments:       The path to the .wav file to stop playing
// Return value:    None.

    void StopSound(const char *wavefile);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StopSound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Stops playing a certain Sound's samples.
// Arguments:       Pointer to the Sound to stop playing. Ownership is NOT transferred!
// Return value:    Whether playback of the sound was successfully stopped, or even found.

    bool StopSound(Sound *pSound);


	bool StopSound(int player, Sound *pSound);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FadeOutSound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fades out playback of a specific sound
// Arguments:       Pointer to the Sound to fade out playing. Ownership is NOT transferred!
//                  The amount of time, in ms, to fade out over.
// Return value:    None.

    void FadeOutSound(Sound *pSound, int fadeOutTime = 1000);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StopMusic
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Stops playing a the music channel.
// Arguments:       None.
// Return value:    None.

    void StopMusic();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMusicPosition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the music to a specific position in the song.
// Arguments:       The desired position from the start, in seconds.
// Return value:    None.

    void SetMusicPosition(double position);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMusicPosition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the position of palyback of the current music stream, in seconds. 
// Arguments:       None.
// Return value:    THe current position of the curtrent stream playing, in seconds.

    double GetMusicPosition();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMusicPath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the path of the last played music stream.
// Arguments:       None.
// Return value:    The file path of the last played music stream.

    std::string GetMusicPath() const { return m_MusicPath; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StopAll
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Stops all playback.
// Arguments:       None.
// Return value:    None.

    void StopAll();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this AudioMan. Supposed to be done every frame
//                  before drawing.
// Arguments:       None.
// Return value:    None.

    void Update();


	bool IsInMultiplayerMode() { return m_IsInMultiplayerMode; }

	void SetMultiplayerMode(bool value) { m_IsInMultiplayerMode = value; }

	void GetSoundEvents(int player, std::list<SoundNetworkData> & list);

	void RegisterSoundEvent(int player, unsigned char state, size_t hash, short int distance, short int channel, short int loops, float pitch, bool affectedByPitch);

	void GetMusicEvents(int player, std::list<MusicNetworkData> & list);

	void RegisterMusicEvent(int player, unsigned char state, const char *filepath, int loops, double position, float pitch);

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static const std::string m_ClassName;

	bool m_AudioEnabled;

#ifdef __USE_SOUND_FMOD
	FSOUND_STREAM *m_pMusic;
#elif __USE_SOUND_SDLMIXER
	Mix_Music *m_pMusic;
#endif

    int m_MusicChannel;
    // The path to the last played music stream
    std::string m_MusicPath;
    double m_SoundsVolume;
    double m_MusicVolume;
    // Global pitch multipler
    double m_GlobalPitch;
    // The 'normal' unpitched frequency of each channel handle we have
    std::vector<int> m_NormalFrequencies;
    // How each channel's pitch is modified individually
    std::vector<double> m_PitchModifiers;
    // Playlist of paths to music to play after the current nonlooping one is done
    std::list<std::string> m_MusicPlayList;
    // Timer for measuring silences between songs
    Timer m_SilenceTimer;

	bool m_IsInMultiplayerMode;

	std::list<SoundNetworkData> m_SoundEvents[MAX_CLIENTS];

	std::list<MusicNetworkData> m_MusicEvents[MAX_CLIENTS];


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this AudioMan, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    AudioMan(const AudioMan &reference);
    AudioMan & operator=(const AudioMan &rhs);

};

} // namespace RTE

#endif // File