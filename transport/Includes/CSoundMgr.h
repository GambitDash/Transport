#ifndef INCL_CSOUNDMGR
#define INCL_CSOUNDMGR

#include <vector>

class CSoundMgr
	{
	public:
		CSoundMgr (void);
		~CSoundMgr ();

		ALERROR Init ();
		void CleanUp (void);

		ALERROR LoadWaveFile (const CString &sFilename, int *retiChannel);
		ALERROR LoadWaveFromBuffer (IReadBlock &Data, int *retiChannel);
		void Play (int iChannel, int iVolume, int iPan);
		inline void SetWaveVolume (int iVolumeLevel) { m_iSoundVolume = iVolumeLevel; }

		void PlayMusic (const CString &sFilename);
		void StopMusic (void);

	private:
		/* void * instead of Mix_Chunk. */
		std::vector<void *> m_SoundChunk;
		void *m_Music; 		/* Really Mix_Music. */

		unsigned char m_iSoundVolume;
		BOOL m_Active; 		/* Sound enabled? */
	};


#endif

