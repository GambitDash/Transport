//	Sound.cpp
//
//	Functions to help with DirectSound

#include <math.h>

#include "SDL.h"
#include "SDL_mixer.h"

#include "portage.h"
#include "CReadBlock.h"
#include "CArray.h"
#include "Kernel.h"

#include "CSoundMgr.h"

const int MAX_VOLUME = 0;
const int MIN_VOLUME = -10000;
const int MAX_VOLUME_LEVEL = 10;
const int VOLUME_STEP = 250;

const int MIX_STEREO = 2;

CSoundMgr::CSoundMgr (void) : m_Music(NULL), m_iSoundVolume(SDL_MIX_MAXVOLUME),
										m_Active(false)

//	CSoundMgr constructor

	{
	}

CSoundMgr::~CSoundMgr (void)

//	CSoundMgr destructor

	{
	CleanUp();
	}

void CSoundMgr::CleanUp (void)

//	CleanUp
//
//	Clean up sound manager

	{
	if (!m_Active)
		return;

	StopMusic();
	Mix_HaltChannel(-1);

	for (unsigned int i = 0; i < m_SoundChunk.size(); i++)
		{
		Mix_FreeChunk((Mix_Chunk *)m_SoundChunk[i]);
		m_SoundChunk[i] = NULL;
		}
	m_Active = FALSE;
	}

ALERROR CSoundMgr::Init ()

//	Init
//
//	Initialize sound manager

	{
	int ret;
	int audio_rate, audio_channels, bits;
	WORD audio_format;

	if (m_Active)
		return NOERROR;

	kernelDebugLogMessage("Opening audio device...");

	ret = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_STEREO,
			1024);
	if (ret < 0)
		{
		kernelDebugLogMessage("Failed to open audio: %s", Mix_GetError());
		m_Active = FALSE;
		return NOERROR;
		}
	Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
	bits = audio_format & 0xFF;

	kernelDebugLogMessage("Audio: %d Hz %d bit %s, %d bytes audio buffer\n",
			audio_rate, bits, audio_channels > 1 ? "stereo" : "mono",
			512);

	m_Active = TRUE;
	return NOERROR;
	}

ALERROR CSoundMgr::LoadWaveFile (const CString &sFilename, int *retiChannel)

//	LoadWaveFile
//
//	Creates a sound buffer from a WAV file

	{
	Mix_Chunk *sample;

	if (!m_Active)
		return NOERROR;

	sample = Mix_LoadWAV(sFilename.GetPointer());
	if (sample == NULL)
		{
		kernelDebugLogMessage("Failed to load sample %s: %s",
				sFilename.GetPointer(), Mix_GetError());
		return ERR_NOTFOUND;
		}
	kernelDebugLogMessage("Loaded sample %s into channel %d",
			sFilename.GetPointer(), m_SoundChunk.size());

	m_SoundChunk.push_back(sample);
	if (retiChannel)
		{
		*retiChannel = m_SoundChunk.size() - 1;
		}

	return NOERROR;
	}

ALERROR CSoundMgr::LoadWaveFromBuffer (IReadBlock &Data, int *retiChannel)

//	LoadWaveFromBuffer
//
//	Loads a wave file from a buffer

	{
	kernelDebugLogMessage("XXX Not Supported - LoadWaveFromBuffer");
	return NOERROR;
	}

void CSoundMgr::Play (int iChannel, int iVolume, int iPan)

//	Play
//
//	Plays a channel.
//
//	iVolume = 0 for maximum volume.
//	iVolume = -10,000 for minimum volume.

	{
	int channel;
	int ret;

	if (!m_Active)
		return;

	channel = Mix_PlayChannel(-1, (Mix_Chunk *)m_SoundChunk[iChannel], 0);
	if (channel >= 0)
		{
		iVolume += 10000;
		iVolume = floor(((float)MIX_MAX_VOLUME) * ((float)iVolume / 10000.0));
		ret = Mix_Volume(channel, iVolume);
		if (ret < 0)
			kernelDebugLogMessage("Failed to set volume %d: %s", iVolume,
					Mix_GetError());
		iPan += 127;
		ret = Mix_SetPanning(channel, iPan, 254 - iPan);
		if (ret < 0)
			kernelDebugLogMessage("Failed to pan %d: %s", iPan, Mix_GetError());
		}
	else
		{
		kernelDebugLogMessage("Failed to play chunk %d: %s", iChannel,
				Mix_GetError());
		}
	}

void CSoundMgr::PlayMusic (const CString &sFilename)

//	PlayMusic
//
//	Starts playing the given MP3 file

	{
	Mix_Music *music;

	if (!m_Active)
		return;

	/* Free any previous bit of music. */
	if (m_Music)
		{
		Mix_HaltMusic();
		Mix_FreeMusic((Mix_Music *)m_Music);
		m_Music = NULL;
		}

	music = Mix_LoadMUS(sFilename.GetPointer());
	if (music == NULL)
		{
		kernelDebugLogMessage("Unable to load music file %s: %s",
				sFilename.GetPointer(), Mix_GetError());
		return;
		}

	if(Mix_PlayMusic(music, 1) < 0)
		{
		kernelDebugLogMessage("Failed to play music %s: %s",
				sFilename.GetPointer(), Mix_GetError());
		Mix_FreeMusic(music);
		return;
		}

	m_Music = music;
	}

void CSoundMgr::StopMusic (void)

//	StopMusic
//
//	Stops the music playback (if it is still playing)

	{
	if (!m_Active)
		return;

	Mix_HaltMusic();
	Mix_FreeMusic((Mix_Music *)m_Music);
	m_Music = NULL;
	}

