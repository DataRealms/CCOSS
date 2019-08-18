//////////////////////////////////////////////////////////////////////////////////////////
// File:            AudioMan.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the AudioMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include <mutex>

#ifdef __USE_SOUND_FMOD
#include "fmod.h"
#elif __USE_SOUND_SDLMIXER
#include "SDL.h"
#include "SDL_mixer.h"
#elif __USE_SOUND_GORILLA
#include "gorilla/ga.h"
#include "gorilla/gau.h"
#endif

#include "AudioMan.h"
#include "ConsoleMan.h"
#include "SettingsMan.h"
#include "Sound.h"
#include "DDTError.h"

#include <vector>
#include <string>


// Allegro defines those via define in astdlib.h and Boost with stdlib go crazy about those so we need to undefine them manually.
#undef int8_t
#undef uint8_t
#undef int16_t
#undef uint16_t
#undef int32_t
#undef uint32_t
#undef intptr_t
#undef uintptr_t

#undef LONG_LONG
#undef int64_t
#undef uint64_t

#ifdef __USE_SOUND_FMOD
#define MAX_VOLUME 255
#elif __USE_SOUND_SDLMIXER
#define MAX_VOLUME MIX_MAX_VOLUME
#elif __USE_SOUND_GORILLA
#define MAX_VOLUME 1.0f
#endif

namespace RTE
{

const string AudioMan::m_ClassName = "AudioMan";

#ifdef __USE_SOUND_FMOD
// Callback for catching music streams that end
signed char F_CALLBACKAPI PlayNextCallback(FSOUND_STREAM *stream, void *buff, int len, void *userdata)
{
    g_AudioMan.PlayNextStream();
    return 0;
}
#elif __USE_SOUND_SDLMIXER
// Callback for catching music streams that end
void PlayNextCallback()
{
	g_AudioMan.PlayNextStream();
}
#elif __USE_SOUND_GORILLA
// Callback for catching music streams that end
void PlayNextCallback(ga_Handle *in_finishedHandle, void *in_context)
{
	g_AudioMan.PlayNextStream();
}
#endif // __USE_SOUND_FMOD


// I know this is a crime, but if I include it in FrameMan.h the whole thing will collapse due to int redefinitions in Allegro
std::mutex g_SoundEventsListMutex[MAX_CLIENTS];


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this AudioMan, effectively
//                  resetting the members of this abstraction level only.

void AudioMan::Clear()
{
	m_AudioEnabled = false;
#ifdef __USE_SOUND_FMOD
    m_pMusic = 0;
#elif __USE_SOUND_SDLMIXER
	m_pMusic = 0;
#elif __USE_SOUND_GORILLA
	m_pManager = 0;
	m_pMixer = 0;
	m_pStreamManager = 0;

	m_pMusic = 0;

	m_SoundChannels.clear();
	m_SoundInstances.clear();
	//m_MaxChannels = 0;
#endif
    m_MusicChannel = -1;
    m_MusicPath.clear();
    m_SoundsVolume = 1.0;
    m_MusicVolume = 1.0;
    m_GlobalPitch = 1.0;
    m_NormalFrequencies.clear();
    m_PitchModifiers.clear();
    m_MusicPlayList.clear();
    m_SilenceTimer.Reset();
    m_SilenceTimer.SetRealTimeLimitS(-1);
	m_IsInMultiplayerMode = false;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		m_SoundEvents[i].clear();
		m_MusicEvents[i].clear();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the AudioMan object ready for use.

int AudioMan::Create()
{
#ifdef __USE_SOUND_FMOD
    printf( "GRAN GRAN THE AUDIO MAN\n" );
    // Basic params for the audio
    int audioBitrate = 44100;
    int maxChannels = g_SettingsMan.GetAudioChannels();

	#define __USE_ALSA
	
	#ifdef _WIN32
	#undef __USE_ALSA
	#endif

	#ifdef __APPLE__
	#undef __USE_ALSA
	#endif
    // This is where we open up our audio device. Mix_OpenAudio takes as its parameters the audio format we'd /like/ to have.
	#ifdef __USE_ALSA
    FSOUND_SetOutput( FSOUND_OUTPUT_ALSA );
	#endif

    if (!FSOUND_Init(audioBitrate, maxChannels, 0))
	{
		// Audio failed to init, so just disable it
        m_AudioEnabled = false;
		return -1;
	}
	else
		m_AudioEnabled = true;

    // Set the global sound channels' volume
    FSOUND_SetSFXMasterVolume(MAX_VOLUME * m_SoundsVolume);
    // And Music volume
    SetMusicVolume(m_MusicVolume);

    // Init up the array of normal frequencies
    int channelCount = FSOUND_GetMaxChannels();
    for (int channel = 0; channel < channelCount; ++channel)
    {
        m_NormalFrequencies.push_back(FSOUND_GetFrequency(channel));
        m_PitchModifiers.push_back(1.0);
    }

    // Init the global pitch
    SetGlobalPitch(m_GlobalPitch);
#elif __USE_SOUND_SDLMIXER
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		// Audio failed to init, so just disable it
		m_AudioEnabled = false;
		return -1;
	}

	int audioBitrate = 44100;
	Uint16 audioFormat = AUDIO_F32;
	int maxChannels = 32;
	int maxAudioChannels = 2;

	// Open the audio device
	if (Mix_OpenAudio(audioBitrate, audioFormat, maxAudioChannels, 4096) < 0) {
		// Audio failed to init, so just disable it
		m_AudioEnabled = false;
		return -1;
	} else {
		Mix_QuerySpec(&audioBitrate, &audioFormat, &maxAudioChannels);
	}

	Mix_HookMusicFinished(PlayNextCallback);
	Mix_AllocateChannels(maxChannels);

	m_AudioEnabled = true;

	// Set the global sound channels' volume
	SetSoundsVolume(m_SoundsVolume);
	// And Music volume
	SetMusicVolume(m_MusicVolume);

	// Init up the array of normal frequencies
	int channelCount = maxChannels;
	for (int channel = 0; channel < channelCount; ++channel)
	{
		//m_NormalFrequencies.push_back(FSOUND_GetFrequency(channel));
		m_PitchModifiers.push_back(1.0);
	}

	// Init the global pitch
	SetGlobalPitch(m_GlobalPitch);
#elif __USE_SOUND_GORILLA
	if (gc_initialize(0) != GC_SUCCESS)
	{
		// Audio failed to init, so just disable it
		m_AudioEnabled = false;
		return -1;
	}

	int maxChannels = g_SettingsMan.GetAudioChannels();

	maxChannels = 16;

	m_pManager = gau_manager_create();
	//m_pManager = gau_manager_create_custom(GA_DEVICE_TYPE_DEFAULT, GAU_THREAD_POLICY_SINGLE, maxChannels, 1024);
	if (!m_pManager)
	{
		// Audio failed to init, so just disable it
		m_AudioEnabled = false;
		return -1;
	}

	m_pMixer = gau_manager_mixer(m_pManager);
	m_pStreamManager = gau_manager_streamManager(m_pManager);

	// Init up the array of normal frequencies
	m_MusicChannel = 0;
	for (int channel = 0; channel < maxChannels; ++channel)
	{
		m_SoundChannels.push_back(0);
		m_SoundInstances.push_back(0);
		m_PitchModifiers.push_back(1.0);
	}

	m_AudioEnabled = true;

	// Set the global sound channels' volume
	SetSoundsVolume(m_SoundsVolume);
	// And Music volume
	SetMusicVolume(m_MusicVolume);
	// Init the global pitch
	SetGlobalPitch(m_GlobalPitch);

#endif

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this AudioMan with a Writer for
//                  later recreation with Create(Reader &reader);

int AudioMan::Save(Writer &writer) const
{

// TODO: "Do this!")

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the AudioMan object.

void AudioMan::Destroy()
{
	if (!m_AudioEnabled)
		return;

    StopAll();

    // Close FMOD
#ifdef __USE_SOUND_FMOD
    FSOUND_Close();
#elif __USE_SOUND_SDLMIXER
	Mix_CloseAudio();
	SDL_Quit();
#elif __USE_SOUND_GORILLA
	gau_manager_destroy(m_pManager);
	gc_shutdown();
#endif

    Clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSoundsVolume
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the volume of all sounds to a specific volume.
//                  Does not affect music.

void AudioMan::SetSoundsVolume(double volume)
{
    m_SoundsVolume = volume;

	if (!m_AudioEnabled)
		return;

#ifdef __USE_SOUND_FMOD
	FSOUND_SetSFXMasterVolume(MAX_VOLUME * m_SoundsVolume);
#elif __USE_SOUND_SDLMIXER
	Mix_Volume(-1, (MAX_VOLUME * m_SoundsVolume));
#elif __USE_SOUND_GORILLA
	for (int i = 1; i < m_SoundChannels.size(); i++)
		if (m_SoundChannels[i])
			ga_handle_setParamf(m_SoundChannels[i], GA_HANDLE_PARAM_GAIN, MAX_VOLUME * m_SoundsVolume);
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMusicVolume
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the music to a specific volume.
// Arguments:       The desired volume scalar. 0.0-1.0.
// Return value:    None.

void AudioMan::SetMusicVolume(double volume)
{
    m_MusicVolume = volume;

	if (!m_AudioEnabled)
		return;

#ifdef __USE_SOUND_FMOD
	if (m_pMusic >= 0)
        FSOUND_SetVolumeAbsolute(m_MusicChannel, MAX_VOLUME * m_MusicVolume);
#elif __USE_SOUND_SDLMIXER
	if (m_pMusic >= 0)
		Mix_VolumeMusic(MAX_VOLUME * m_MusicVolume);
#elif __USE_SOUND_GORILLA
	if (m_SoundChannels.size() > m_MusicChannel && m_SoundChannels[m_MusicChannel])
		ga_handle_setParamf(m_SoundChannels[m_MusicChannel], GA_HANDLE_PARAM_GAIN, MAX_VOLUME * m_MusicVolume);
#endif __USE_SOUND_FMOD

}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTempMusicVolume
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the music to a specific volume, but it will onyl last until a new
//                  song is palyed. Useful for fading etc.
// Arguments:       The desired volume scalar. 0.0-1.0.
// Return value:    None.

void AudioMan::SetTempMusicVolume(double volume)
{
	if (!m_AudioEnabled)
		return;

#ifdef __USE_SOUND_FMOD
	if (m_pMusic >= 0)
        FSOUND_SetVolumeAbsolute(m_MusicChannel, MAX_VOLUME * volume);
#elif __USE_SOUND_SDLMIXER
	if (m_pMusic >= 0)
		Mix_VolumeMusic(MAX_VOLUME * volume);
#elif __USE_SOUND_GORILLA
	for (int i = 1; i < m_SoundChannels.size(); i++)
		if (m_SoundChannels[i])
			ga_handle_setParamf(m_SoundChannels[i], GA_HANDLE_PARAM_GAIN, MAX_VOLUME * volume);
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetGlobalPitch
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the global pitch multiplier for all sounds, including the music.

void AudioMan::SetGlobalPitch(double pitch, bool excludeMusic)
{
	if (!m_AudioEnabled)
		return;

	if (m_IsInMultiplayerMode)
		RegisterSoundEvent(-1, SOUND_SET_PITCH, 0, 0, 0, 0, pitch, excludeMusic);

#ifdef __USE_SOUND_FMOD
	// Keep the pitch value sane
    m_GlobalPitch = pitch > 0.1 ? pitch : (pitch < 16.0 ? pitch : 16.0);
    // The channel index is stored in the lower 12 bits of the channel handle
    int musicChannelIndex = excludeMusic ? m_MusicChannel & 0x00000FFF : -1;

    // Go through all active channels and set the pitch on each, except for the music one
    int channelCount = FSOUND_GetMaxChannels();
    for (int channel = 0; channel < channelCount; ++channel)
    {
        // It works to pass the channel index to IsPlaying and SetFreq because the lwoer 12 bits are enough to show which channel we're talking about
// Adjust the music too, it works better for gameplay
        if (channel != musicChannelIndex && FSOUND_IsPlaying(channel))
        {
            // Only set the frequency of those whose normal frequency values are normal (not set to <= 0 because they shouldn't be pitched)
            if (m_NormalFrequencies[channel] > 0)
                FSOUND_SetFrequency(channel, m_NormalFrequencies[channel] * m_PitchModifiers[channel] * m_GlobalPitch);
        }
    }
#elif __USE_SOUND_SDLMIXER
	// Looks like it does not support pitching
#elif __USE_SOUND_GORILLA
	// Keep the pitch value sane
	m_GlobalPitch = pitch > 0.1 ? pitch : (pitch < 16.0 ? pitch : 16.0);

	// Go through all active channels and set the pitch on each, except for the music one
	for (int channel = 1; channel < m_SoundChannels.size(); ++channel)
	{
		if (m_SoundChannels[channel] && !ga_handle_finished(m_SoundChannels[channel]))
		{
			ga_handle_setParamf(m_SoundChannels[channel], GA_HANDLE_PARAM_PITCH, m_PitchModifiers[channel] * m_GlobalPitch);
		}
	}
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlayMusic
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Starts playing a certain WAVE, MOD, MIDI, OGG, MP3 file in the music
//                  channel

void AudioMan::PlayMusic(const char *filepath, int loops, double volumeOverride)
{
	if (!m_AudioEnabled)
		return;
	if (m_IsInMultiplayerMode)
		RegisterMusicEvent(-1, MUSIC_PLAY, filepath, loops, 0.0, 1.0);

#ifdef __USE_SOUND_FMOD
	// If music is already playing, first stop it
    if (m_pMusic)
    {
        FSOUND_Stream_Stop(m_pMusic);
        FSOUND_Stream_Close(m_pMusic);
        m_pMusic = 0;
    }

	// Open the stream
    m_pMusic = FSOUND_Stream_Open(filepath, 0, 0, 0);
    if (!m_pMusic)
    {
        g_ConsoleMan.PrintString("ERROR: Could not open and play music file:" + string(filepath));
        return;
    }
    // Start playing the stream
    m_MusicChannel = FSOUND_Stream_Play(FSOUND_FREE, m_pMusic);
    if (m_MusicChannel < 0)
    {
        g_ConsoleMan.PrintString("ERROR: Could not open and play music file:" + string(filepath));
        return;
    }
    // Save the path of the last played music stream
    m_MusicPath = filepath;
    // Set the callback so that it will switch to next song when current one is done.
    FSOUND_Stream_SetEndCallback(m_pMusic, PlayNextCallback, 0);
    // Set loop mode
    if (loops != 0)
        FSOUND_Stream_SetMode(m_pMusic, FSOUND_LOOP_NORMAL);
    // Set loop count
    FSOUND_Stream_SetLoopCount(m_pMusic, loops);
    // Set the volume of the music stream's channel, and override if asked to, but not if the normal music volume is muted
    if (volumeOverride >= 0 && m_MusicVolume > 0)
        FSOUND_SetVolumeAbsolute(m_MusicChannel, MAX_VOLUME * volumeOverride);
    else
        FSOUND_SetVolumeAbsolute(m_MusicChannel, MAX_VOLUME * m_MusicVolume);

    // The channel index is stored in the lower 12 bits of the channel handle
    int channelIndex = m_MusicChannel & 0x00000FFF;

    // Save the 'normal' frequency so we can pitch it later
    if (channelIndex < m_NormalFrequencies.size())
        m_NormalFrequencies[channelIndex] = FSOUND_GetFrequency(m_MusicChannel);
    m_PitchModifiers[channelIndex] = 1.0;
#elif __USE_SOUND_SDLMIXER
	// If music is already playing, first stop it
	if (m_pMusic)
	{
		Mix_HaltMusic();
		Mix_FreeMusic(m_pMusic);
		m_pMusic = 0;
	}

	// Open the stream
	m_pMusic = Mix_LoadMUS(filepath);
	if (!m_pMusic)
	{
		g_ConsoleMan.PrintString("ERROR: Could not open and play music file:" + string(filepath));
		g_ConsoleMan.PrintString("MIX: " + string(Mix_GetError()));
		g_ConsoleMan.PrintString("Available codecs");

		int i, max = Mix_GetNumMusicDecoders();
		for (i = 0; i < max; ++i)
			g_ConsoleMan.PrintString(Mix_GetMusicDecoder(i));
		return;
	}
	// Start playing the stream
	if (Mix_PlayMusic(m_pMusic, -1))
	if (m_MusicChannel < 0)
	{
		g_ConsoleMan.PrintString("ERROR: Could not open and play music file:" + string(filepath));
		g_ConsoleMan.PrintString("MIX: " + string(Mix_GetError()));
		return;
	}
	// Save the path of the last played music stream
	m_MusicPath = filepath;




	// Set the callback so that it will switch to next song when current one is done.
	//FSOUND_Stream_SetEndCallback(m_pMusic, PlayNextCallback, 0);
	// Set loop mode
	//if (loops != 0)
	//	FSOUND_Stream_SetMode(m_pMusic, FSOUND_LOOP_NORMAL);
	// Set loop count
	//FSOUND_Stream_SetLoopCount(m_pMusic, loops);
	// Set the volume of the music stream's channel, and override if asked to, but not if the normal music volume is muted
	//if (volumeOverride >= 0 && m_MusicVolume > 0)
	//	FSOUND_SetVolumeAbsolute(m_MusicChannel, MAX_VOLUME * volumeOverride);
	//else
	//	FSOUND_SetVolumeAbsolute(m_MusicChannel, MAX_VOLUME * m_MusicVolume);

	// The channel index is stored in the lower 12 bits of the channel handle
	int channelIndex = m_MusicChannel & 0x00000FFF;

	// Save the 'normal' frequency so we can pitch it later
	//if (channelIndex < m_NormalFrequencies.size())
	//	m_NormalFrequencies[channelIndex] = FSOUND_GetFrequency(m_MusicChannel);
	m_PitchModifiers[channelIndex] = 1.0;
#elif __USE_SOUND_GORILLA
	// If music is already playing, first stop it
	if (m_pMusic)
	{
		ga_handle_stop(m_pMusic);
		ga_handle_destroy(m_pMusic);
		m_pMusic = 0;
	}

	// Look for file extension
	char format[16];
	strcpy(format, "");

	int dotPos = -1;

	for (int i = strlen(filepath); i >= 0; i--)
	{
		if (filepath[i] == '.')
		{
			dotPos = i;
			break;
		}
	}

	if (dotPos == -1)
		return;

	strcpy(format, &filepath[dotPos + 1]);
	
	// Open the stream
	m_pMusic = gau_create_handle_buffered_file(m_pMixer, m_pStreamManager, filepath, format, PlayNextCallback, 0, 0);
	if (!m_pMusic)
	{
		g_ConsoleMan.PrintString("ERROR: Could not open and play music file:" + string(filepath));
		return;
	}
	// Save the path of the last played music stream
	m_MusicPath = filepath;

	m_SoundChannels[m_MusicChannel] = m_pMusic;
	ga_handle_play(m_pMusic);

	// Set the volume of the music stream's channel, and override if asked to, but not if the normal music volume is muted
	if (volumeOverride >= 0 && m_MusicVolume > 0)
		ga_handle_setParamf(m_pMusic, GA_HANDLE_PARAM_GAIN, MAX_VOLUME * volumeOverride);
	else
		ga_handle_setParamf(m_pMusic, GA_HANDLE_PARAM_GAIN, MAX_VOLUME * m_MusicVolume);

	m_PitchModifiers[m_MusicChannel] = 1.0;
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          QueueMusicStream
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Queues up another path to a stream that will be played after the current
//                  one is done. Can be done several times to queue up many tracks. The
//                  last track in the list will be looped infinitely.

void AudioMan::QueueMusicStream(const char *filepath)
{
#ifdef __USE_SOUND_FMOD
    if (!m_pMusic)
        PlayMusic(filepath);
    else
    {
        m_MusicPlayList.push_back(string(filepath));
        // Set the callback so that it will switch to next song when current one is done.
        FSOUND_Stream_SetEndCallback(m_pMusic, PlayNextCallback, 0);
    }
#elif __USE_SOUND_SDLMIXER
	if (!m_pMusic)
		PlayMusic(filepath);
	else
		m_MusicPlayList.push_back(string(filepath));
#elif __USE_SOUND_GORILLA
	if (!m_pMusic)
		PlayMusic(filepath);
	else
		m_MusicPlayList.push_back(string(filepath));
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          QueueSilence
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Queues up a period of silence in teh music stream playlist.

void AudioMan::QueueSilence(int seconds)
{
    if (seconds > 0)
    {
        // Encode the silence as number of secs preceded by '@'
        char str[256];
        sprintf(str, "@%i", seconds);
        m_MusicPlayList.push_back(string(str));
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlayNextStream
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Plays the next music stream in the queue, if any is queued.
// Arguments:       None.
// Return value:    None.

void AudioMan::PlayNextStream()
{
    if (!m_MusicPlayList.empty())
    {
        string nextString = m_MusicPlayList.front();
        m_MusicPlayList.pop_front();

        // Look for special encoding if we are supposed to have a silence between tracks
        if (nextString.c_str()[0] == '@')
        {
            // Decipher the number of secs we're supposed to wait
            int seconds = 0;
            sscanf(nextString.c_str(), "@%i", &seconds);
            m_SilenceTimer.SetRealTimeLimitS(seconds > 0 ? seconds : 0);
            m_SilenceTimer.Reset();

#ifdef __USE_SOUND_FMOD
            // Stop music playback
            if (m_pMusic)
            {
				RegisterMusicEvent(-1, MUSIC_SILENCE, 0, seconds, 0.0, 1.0);

                FSOUND_Stream_Stop(m_pMusic);
                FSOUND_Stream_Close(m_pMusic);
                m_pMusic = 0;
            }
#elif __USE_SOUND_SDLMIXER
			// Stop music playback
			if (m_pMusic)
			{
				RegisterMusicEvent(-1, MUSIC_SILENCE, 0, seconds, 0.0, 1.0);

				Mix_HaltMusic();
				Mix_FreeMusic(m_pMusic);
				m_pMusic = 0;
			}
#elif __USE_SOUND_GORILLA
			if (m_pMusic)
			{
				RegisterMusicEvent(-1, MUSIC_SILENCE, 0, seconds, 0.0, 1.0);

				ga_handle_stop(m_pMusic);
				ga_handle_destroy(m_pMusic);
				m_pMusic = 0;
			}
#endif
        }
        else
        {
            // Only loop the last one, otherwise play playlist entries only once
            PlayMusic(nextString.c_str(), m_MusicPlayList.empty() ? -1 : 0);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlaySound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Starts playing a certain sample

void AudioMan::PlaySound(const char *wavefilepath)
{
	Sound snd;
	snd.Create(wavefilepath, false);
	snd.Play();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlaySound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Starts playing a certain WAVE sound file.

Sound * AudioMan::PlaySound(const char *filepath, float distance, bool loop, bool affectedByPitch, int player)
{
	if (!filepath)
	{
		g_ConsoleMan.PrintString("Error: Null filepath passed to AudioMan::PlaySound!");
		return 0;
	}

	Sound * snd = new Sound();
	snd->Create(filepath, affectedByPitch, loop ? 1 : 0);
	PlaySound(player, snd, PRIORITY_LOW, distance, affectedByPitch ? m_GlobalPitch : 1.0);

	return snd;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSoundEvents
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fills the list with sound events happened for the specified network player

void AudioMan::GetSoundEvents(int player, std::list<SoundNetworkData> & list)
{
	if (player < 0 || player >= MAX_CLIENTS)
		return;

	list.clear();
	g_SoundEventsListMutex[player].lock();

	for (std::list<SoundNetworkData>::iterator eItr = m_SoundEvents[player].begin(); eItr != m_SoundEvents[player].end(); ++eItr)
		list.push_back((*eItr));

	m_SoundEvents[player].clear();

	g_SoundEventsListMutex[player].unlock();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RegisterSoundEvent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds the sound event to internal list of sound events for the specified player.

void AudioMan::RegisterSoundEvent(int player, unsigned char state, size_t hash, short int distance, short int channel, short int loops, float pitch, bool affectedByPitch)
{
	// Send event to all players
	if (player == -1)
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
			RegisterSoundEvent(i, state, hash, distance, channel, loops, pitch, affectedByPitch);
	} 
	else
	{
		if (player >= 0 && player < MAX_CLIENTS)
		{
			SoundNetworkData d;
			d.State = state;
			d.Distance = distance;
			d.SoundHash = hash;
			d.Channel = channel;
			d.Loops = loops;
			d.Pitch = pitch;
			d.AffectedByPitch = affectedByPitch ? 1 : 0;

			g_SoundEventsListMutex[player].lock();
			m_SoundEvents[player].push_back(d);
			g_SoundEventsListMutex[player].unlock();
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMusicEvents
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fills the list with music events happened for the specified network player

void AudioMan::GetMusicEvents(int player, std::list<MusicNetworkData> & list)
{
	if (player < 0 || player >= MAX_CLIENTS)
		return;

	list.clear();
	g_SoundEventsListMutex[player].lock();

	for (std::list<MusicNetworkData>::iterator eItr = m_MusicEvents[player].begin(); eItr != m_MusicEvents[player].end(); ++eItr)
		list.push_back((*eItr));

	m_MusicEvents[player].clear();

	g_SoundEventsListMutex[player].unlock();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RegisterMusicEvent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds the sound event to internal list of sound events for the specified player.

void AudioMan::RegisterMusicEvent(int player, unsigned char state, const char *filepath, int loops, double position, float pitch)
{
	if (player == -1)
	{
		// Send event to all players
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			RegisterMusicEvent(i, state, filepath, loops, position, pitch);
		}
	}
	else
	{
		MusicNetworkData d;
		d.State = state;
		d.Loops = loops;
		d.Pitch = pitch;
		d.Position = position;
		if (filepath)
			strncpy(d.Path, filepath, 255);
		else
			memset(d.Path, 0, 255);

		g_SoundEventsListMutex[player].lock();
		m_MusicEvents[player].push_back(d);
		g_SoundEventsListMutex[player].unlock();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlaySound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Starts playing the next sample of a certain Sound.

bool AudioMan::PlaySound(int player, Sound *pSound, int priority, float distance, double pitch)
{
	// We need to play sound before registering a event because it will assign channel number to the sound being played
	// clients then uses this number to identify the sound being played
	bool ret = PlaySound(pSound, priority, distance, pitch);

	if (m_IsInMultiplayerMode)
	{
		if (pSound)
		{
			RegisterSoundEvent(player, SOUND_PLAY, pSound->m_Hash, distance, pSound->m_LastChannel, pSound->m_Loops, pitch, pSound->m_AffectedByPitch);
		}
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlaySound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Starts playing the next sample of a certain Sound.

bool AudioMan::PlaySound(Sound *pSound, int priority, float distance, double pitch)
{
#ifdef __USE_SOUND_FMOD
    if (!m_AudioEnabled || !pSound)
        return false;

    pSound->m_LastChannel = FSOUND_PlaySound(FSOUND_FREE, pSound->StartNextSample());

    if (pSound->m_LastChannel == -1)
    {
        g_ConsoleMan.PrintString("ERROR: Could not play a sound sample!");
        return false;
    }

    // Set sample's channel looping setting
    if (pSound->m_Loops == 0)
        FSOUND_SetLoopMode(pSound->m_LastChannel, FSOUND_LOOP_OFF);
    else
        FSOUND_SetLoopMode(pSound->m_LastChannel, FSOUND_LOOP_NORMAL);

    // Set sample's channel priority setting
    // Sound is high enough to be 'reserved', can't be overridden at all
// Don't, have to un-reserve every time it ends
//    if (pSound->m_Priority > 255)
//        FSOUND_SetReserved(pSound->m_LastChannel, true);
//    else if (pSound->m_Priority > 0)
    if (pSound->m_Priority > 0)
        FSOUND_SetPriority(pSound->m_LastChannel, pSound->m_Priority);

// TODO: Use the fmod 3D attenuation method for this isntead
    // Set the distance attenuation effect of the just started sound
    if (distance >= 0)
    {
        if (distance > 1.0f)
            distance = 1.0f;
        // Multiply by 0.9 because we don't want to to go completely quiet if max distance
        distance *= 0.9f;
        FSOUND_SetVolume(pSound->m_LastChannel, MAX_VOLUME * (1.0f - distance));
    }

    // The channel index is stored in the lower 12 bits of the channel handle
    int channelIndex = pSound->m_LastChannel & 0x00000FFF;

    // Save the 'normal' frequency so we can pitch it later
    DAssert(channelIndex < m_NormalFrequencies.size(), "Channel index higher than normal freq array?");
    if (channelIndex < m_NormalFrequencies.size())
    {
        // If this sound isn't supposed to be pitched, then set a <= 0 normal frequency to indicate it
        if (!pSound->m_AffectedByPitch)
            m_NormalFrequencies[channelIndex] = 0;
        else
            m_NormalFrequencies[channelIndex] = FSOUND_GetFrequency(pSound->m_LastChannel);
    }

    // Store the individual pitch modifier
    m_PitchModifiers[channelIndex] = pitch;
#elif __USE_SOUND_SDLMIXER
	if (!m_AudioEnabled || !pSound)
		return false;

	pSound->m_LastChannel = Mix_PlayChannel(-1, pSound->StartNextSample(), pSound->m_Loops);

	if (pSound->m_LastChannel == -1)
	{
		g_ConsoleMan.PrintString("ERROR: Could not play a sound sample!");
		return false;
	}

	// Set sample's channel priority setting
	// Sound is high enough to be 'reserved', can't be overridden at all
	// Don't, have to un-reserve every time it ends
	//    if (pSound->m_Priority > 255)
	//        FSOUND_SetReserved(pSound->m_LastChannel, true);
	//    else if (pSound->m_Priority > 0)
	//if (pSound->m_Priority > 0)
	//	FSOUND_SetPriority(pSound->m_LastChannel, pSound->m_Priority);

	// TODO: Use the fmod 3D attenuation method for this isntead
	// Set the distance attenuation effect of the just started sound
	if (distance >= 0)
	{
		if (distance > 1.0f)
			distance = 1.0f;
		// Multiply by 0.9 because we don't want to to go completely quiet if max distance
		distance *= 0.9f;
		//Mix_Volume(pSound->m_LastChannel, (MAX_VOLUME * (1.0f - distance)));
		Mix_SetDistance(pSound->m_LastChannel, (255 * distance));
	}

	// The channel index is stored in the lower 12 bits of the channel handle
	//int channelIndex = pSound->m_LastChannel & 0x00000FFF;

	// Save the 'normal' frequency so we can pitch it later
	//DAssert(channelIndex < m_NormalFrequencies.size(), "Channel index higher than normal freq array?");
	//if (channelIndex < m_NormalFrequencies.size())
	//{
		// If this sound isn't supposed to be pitched, then set a <= 0 normal frequency to indicate it
	//	if (!pSound->m_AffectedByPitch)
	//		m_NormalFrequencies[channelIndex] = 0;
	//	else
	//		m_NormalFrequencies[channelIndex] = FSOUND_GetFrequency(pSound->m_LastChannel);
	//}

	// Store the individual pitch modifier
	//m_PitchModifiers[channelIndex] = pitch;
#elif __USE_SOUND_GORILLA
	if (!m_AudioEnabled || !pSound)
	return false;

	int channel = -1;
	// Find a free channel
	for (int i = 1; i < m_SoundChannels.size(); i++)
	{
		if (m_SoundChannels[i] == 0 || (m_SoundChannels[i] != 0 && !ga_handle_playing(m_SoundChannels[i])))
		{
			channel = i;
			break;
		}
	}

	// Add new channels when there's no space
	if (channel == -1)
	{
		m_SoundChannels.push_back(0);
		m_SoundInstances.push_back(0);
		m_PitchModifiers.push_back(1.0f);

		channel = m_SoundChannels.size() - 1;
	}

	if (m_SoundChannels[channel] != 0)
	{
		ga_handle_destroy(m_SoundChannels[channel]);
		m_SoundChannels[channel] = 0;
	}

	pSound->m_LastChannel = channel;

	if (pSound->m_LastChannel == -1)
	{
		g_ConsoleMan.PrintString("ERROR: Could not play a sound sample!");
		return false;
	}

	// Set sample's channel looping setting
	ga_Handle * handle;
	if (pSound->m_Loops > 0)
	{
		gau_SampleSourceLoop* loopSrc; 
		handle = gau_create_handle_sound(m_pMixer, pSound->GetCurrentSample(), 0, 0, &loopSrc);
		gau_sample_source_loop_set(loopSrc, -1, 0);
	}
	else 
	{
		handle = gau_create_handle_sound(m_pMixer, pSound->GetCurrentSample(), 0, 0, 0);
	}
	m_SoundChannels[channel] = handle;
	m_SoundInstances[channel] = pSound->GetCurrentSample();

	ga_handle_play(handle);

	// Set the distance attenuation effect of the just started sound
	SetSoundAttenuation(pSound, distance);

	// Store the individual pitch modifier
	SetSoundPitch(pSound, pitch);
#endif

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSoundAttenuation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets/updates the distance attenuation for a specific sound. Will only
//                  have an effect if the sound is currently being played.

bool AudioMan::SetSoundAttenuation(Sound *pSound, float distance)
{
	if (!m_AudioEnabled || !pSound)
		return false;

// TODO: Use the fmod 3D attenuation method for this instead
    // Set the distance attenuation effect of the just started sound
    if (distance > 0)
    {
        if (distance > 1.0f)
            distance = 1.0f;
        // Multiply by 0.9 because we don't want to to go completely quiet if max distance
        distance *= 0.95f;
#ifdef __USE_SOUND_FMOD
		FSOUND_SetVolume(pSound->m_LastChannel, MAX_VOLUME * (1.0f - distance));
#elif __USE_SOUND_SDLMIXER
		//Mix_Volume(pSound->m_LastChannel, ((double)MIX_MAX_VOLUME * (1.0f - distance)));
		Mix_SetDistance(pSound->m_LastChannel, (255 * distance));
#elif __USE_SOUND_GORILLA
		if (pSound->m_LastChannel >= 0)
			ga_handle_setParamf(m_SoundChannels[pSound->m_LastChannel], GA_HANDLE_PARAM_GAIN, MAX_VOLUME * (1.0f - distance));
#endif
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSoundPitch
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets/updates the frequency/pitch for a specific sound. Will only
//                  have an effect if the sound is currently being played.

bool AudioMan::SetSoundPitch(Sound *pSound, float pitch)
{
    if (!m_AudioEnabled || !pSound || !IsPlaying(pSound))
        return false;

	if (IsInMultiplayerMode() && pSound)
		RegisterSoundEvent(-1, SOUND_SET_PITCH, pSound->m_Hash, 0, pSound->m_LastChannel, pSound->m_Loops, pitch, pSound->m_AffectedByPitch);

    // Keep teh pitch value sane
    if (pitch < 0.1f)
        pitch = 0.1f;
    if (pitch > 16.0f)
        pitch = 16.0f;

    // Only set the frequency of those whose normal frequency values are normal (not set to <= 0 because they shouldn't be pitched)
#ifdef __USE_SOUND_FMOD
	// The channel index is stored in the lower 12 bits of the channel handle
	int channelIndex = pSound->m_LastChannel & 0x00000FFF;

	// Update the individual pitch modifier
	m_PitchModifiers[channelIndex] = pitch;

    if (m_NormalFrequencies[channelIndex] > 0)
        FSOUND_SetFrequency(pSound->m_LastChannel, m_NormalFrequencies[channelIndex] * m_PitchModifiers[channelIndex] * m_GlobalPitch);
#elif __USE_SOUND_SDLMIXER
	// SDL seems to not support pitch changes
#elif __USE_SOUND_GORILLA
	if (pSound->m_LastChannel >= 0)
	{
		m_PitchModifiers[pSound->m_LastChannel] = pitch;
		ga_handle_setParamf(m_SoundChannels[pSound->m_LastChannel], GA_HANDLE_PARAM_PITCH, m_PitchModifiers[pSound->m_LastChannel] * m_GlobalPitch);
	}
#endif

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMusicPitch
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets/updates the frequency/pitch for the music channel.

bool AudioMan::SetMusicPitch(float pitch)
{
	if (m_IsInMultiplayerMode)
		RegisterMusicEvent(-1, MUSIC_SET_PITCH, 0, 0, 0.0, pitch);

#ifdef __USE_SOUND_FMOD
	if (!m_AudioEnabled || !m_pMusic)
        return false;
#elif __USE_SOUND_SDLMIXER
	if (!m_AudioEnabled || !m_pMusic)
		return false;
#elif __USE_SOUND_GORILLA
	if (!m_AudioEnabled || !m_pMusic)
		return false;
#else
	return false;
#endif

    // Keep teh pitch value sane
    if (pitch < 0.1f)
        pitch = 0.1f;
    if (pitch > 16.0f)
        pitch = 16.0f;

    // The channel index is stored in the lower 12 bits of the channel handle
    int channelIndex = m_MusicChannel & 0x00000FFF;

    // Update the individual pitch modifier
    m_PitchModifiers[channelIndex] = pitch;

    // Only set the frequency of those whose normal frequency values are normal (not set to <= 0 because they shouldn't be pitched)
#ifdef __USE_SOUND_FMOD
	if (m_NormalFrequencies[channelIndex] > 0)
        FSOUND_SetFrequency(m_MusicChannel, m_NormalFrequencies[channelIndex] * m_PitchModifiers[channelIndex] * m_GlobalPitch);
#elif __USE_SOUND_SDLMIXER

#elif __USE_SOUND_GORILLA
	ga_handle_setParamf(m_SoundChannels[m_MusicChannel], GA_HANDLE_PARAM_PITCH, m_PitchModifiers[m_MusicChannel] * m_GlobalPitch);
#endif

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsPlaying
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whetehr a certain Sound's last played sample is being played
//                  currently.

bool AudioMan::IsPlaying(Sound *pSound)
{
    if (!m_AudioEnabled || !pSound)
        return false;

    if (pSound->GetSampleCount() <= 0 || pSound->m_LastChannel < 0)
        return false;

// TODO: This doesn't check all the samples of the Sound?!
#ifdef __USE_SOUND_FMOD
    if (FSOUND_GetCurrentSample(pSound->m_LastChannel) != pSound->GetCurrentSample())
        return false;

	return FSOUND_IsPlaying(pSound->m_LastChannel);
#elif __USE_SOUND_SDLMIXER
	if (Mix_GetChunk(pSound->m_LastChannel) != pSound->GetCurrentSample())
			return false;

	int playing = Mix_Playing(pSound->m_LastChannel);
	return playing;
#elif __USE_SOUND_GORILLA
	if (m_SoundInstances[pSound->m_LastChannel] != pSound->GetCurrentSample())
		return false;

	int playing = ga_handle_playing(m_SoundChannels[pSound->m_LastChannel]);
	return playing;
#else
	return false;
#endif

}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StopSound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Stops playing a certain sound sample

void AudioMan::StopSound(const char *wavefilepath)
{
    
}
*/
bool AudioMan::StopSound(int player, Sound *pSound)
{
	if (m_IsInMultiplayerMode)
	{
		if (pSound)
		{
			RegisterSoundEvent(player, SOUND_STOP, pSound->m_Hash, 0, pSound->m_LastChannel, pSound->m_Loops, 1.0, pSound->m_AffectedByPitch);
		}
	}
	return StopSound(pSound);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StopSound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Stops playing a certain Sound's samples.

bool AudioMan::StopSound(Sound *pSound)
{
	if (!m_AudioEnabled || !pSound)
		return false;

    if (IsPlaying(pSound))
    {
#ifdef __USE_SOUND_FMOD
		FSOUND_StopSound(pSound->m_LastChannel);
#elif __USE_SOUND_SDLMIXER
		Mix_HaltChannel(pSound->m_LastChannel);
#elif __USE_SOUND_GORILLA
		ga_handle_stop(m_SoundChannels[pSound->m_LastChannel]);
#endif
        pSound->m_LastChannel = -1;
        return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FadeOutSound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fades out playback of a specific sound

void AudioMan::FadeOutSound(Sound *pSound, int fadeOutTime)
{
	if (!m_AudioEnabled || !pSound)
		return;
/* TODO: This
    if (IsPlaying(pSound))
    {
        Mix_FadeOutChannel(pSound->m_LastChannel, fadeOutTime);
    }
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StopMusic
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Stops playing a the music channel.

void AudioMan::StopMusic()
{

	if (m_IsInMultiplayerMode)
		RegisterMusicEvent(-1, MUSIC_STOP, 0, 0, 0.0, 0.0);

#ifdef __USE_SOUND_FMOD
	if (!m_AudioEnabled || !m_pMusic)
		return;

	FSOUND_Stream_Stop(m_pMusic);
    FSOUND_Stream_Close(m_pMusic);
    m_pMusic = 0;
#elif __USE_SOUND_SDLMIXER
	if (!m_AudioEnabled || !m_pMusic)
		return;

	Mix_HaltMusic();
	Mix_FreeMusic(m_pMusic);
	m_pMusic = 0;
#elif __USE_SOUND_GORILLA
	if (!m_AudioEnabled || !m_pMusic)
		return;

	ga_handle_stop(m_pMusic);
	ga_handle_destroy(m_pMusic);
	m_pMusic = 0;
#endif 

    // Clear out playlist, it doesn't apply anymore
    m_MusicPlayList.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMusicPosition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the music to a specific position in the song.

void AudioMan::SetMusicPosition(double position)
{
#ifdef __USE_SOUND_FMOD
	if (!m_AudioEnabled || !m_pMusic)
		return;

	FSOUND_Stream_SetTime(m_pMusic, position * 1000);
#elif __USE_SOUND_SDLMIXER
	if (!m_AudioEnabled || !m_pMusic)
		return;

	Mix_RewindMusic();
	Mix_SetMusicPosition(position);
#elif __USE_SOUND_GORILLA
	if (!m_AudioEnabled || !m_pMusic)
		return;

	ga_handle_seek(m_pMusic, position);
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMusicPosition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the music to a specific position in the song.

double AudioMan::GetMusicPosition()
{
#ifdef __USE_SOUND_FMOD
	if (!m_AudioEnabled || !m_pMusic)
		return 0;

	return ((double)FSOUND_Stream_GetTime(m_pMusic)) / 1000.0;
#elif __USE_SOUND_SDLMIXER
	if (!m_AudioEnabled || !m_pMusic)
		return 0;

	return 0;
#elif __USE_SOUND_GORILLA
	if (!m_AudioEnabled || !m_pMusic)
		return 0;

	return ga_handle_tell(m_pMusic, GA_TELL_PARAM_CURRENT);
#else
	return 0;
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StopAll
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Stops all audio playback.

void AudioMan::StopAll()
{
	if (!m_AudioEnabled)
		return;

    // Halt sound playback on all channels
#ifdef __USE_SOUND_FMOD
	FSOUND_StopSound(FSOUND_ALL);

    // If music is playing, stop it
    if (m_pMusic)
    {
        FSOUND_Stream_Stop(m_pMusic);
        FSOUND_Stream_Close(m_pMusic);
        m_pMusic = 0;
    }
#elif __USE_SOUND_SDLMIXER
	Mix_HaltChannel(-1);

	// If music is playing, stop it
	if (m_pMusic)
	{
		Mix_HaltMusic();
		Mix_FreeMusic(m_pMusic);
		m_pMusic = 0;
	}
#elif __USE_SOUND_GORILLA
	for (int i = 1; i < m_SoundChannels.size(); i++)
	{
		if (m_SoundChannels[i] && ga_handle_playing(m_SoundChannels[i]))
			ga_handle_stop(m_SoundChannels[i]);
	}

	// If music is playing, stop it
	if (m_pMusic)
	{
		ga_handle_stop(m_pMusic);
		ga_handle_destroy(m_pMusic);
		m_pMusic = 0;
	}
#endif

    // Clear out playlist, it doesn't apply anymore
    m_MusicPlayList.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this AudioMan. Supposed to be done every frame
//                  before drawing.

void AudioMan::Update()
{
	if (!m_AudioEnabled)
		return;

#ifdef __USE_SOUND_FMOD
    // Done waiting for silence
    if (!m_pMusic && m_SilenceTimer.IsPastRealTimeLimit())
        PlayNextStream();
#elif __USE_SOUND_SDLMIXER
	// Done waiting for silence
	if (!m_pMusic && m_SilenceTimer.IsPastRealTimeLimit())
		PlayNextStream();
#elif __USE_SOUND_GORILLA
	// Done waiting for silence
	if (!m_pMusic && m_SilenceTimer.IsPastRealTimeLimit())
		PlayNextStream();
	gau_manager_update(m_pManager);
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayingChannelCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the number of channels currently used.

int AudioMan::GetPlayingChannelCount()
{
#ifdef __USE_SOUND_FMOD
	return FSOUND_GetChannelsPlaying();
#elif __USE_SOUND_SDLMIXER
#elif __USE_SOUND_GORILLA
	int count = 0;

	for (int i = 0; i < m_SoundChannels.size(); i++)
	{
		if (m_SoundChannels[i] && ga_handle_playing(m_SoundChannels[i]))
			count++;
	}
	return count;
#endif
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalChannelCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the number of channels available.

int AudioMan::GetTotalChannelCount()
{
#ifdef __USE_SOUND_FMOD
	return FSOUND_GetMaxChannels();
#elif __USE_SOUND_SDLMIXER
#elif __USE_SOUND_GORILLA
	return m_SoundChannels.size();
#endif
	return 0;
}

} // namespace RTE
