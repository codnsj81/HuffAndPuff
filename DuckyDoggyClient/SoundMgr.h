#pragma once

#include "Include.h"

#define SOUNDMGR CSoundMgr::GetInstance()

class CSoundMgr
{
	DECLARE_SINGLETON(CSoundMgr)

private:
	FMOD_SYSTEM*				m_pSystem;
	FMOD_CHANNEL*				m_pChannel[CHANNEL_END];

	map<TCHAR*, FMOD_SOUND*>	m_MapSound;

private:
	CSoundMgr(void);
	~CSoundMgr(void);

public:
	void Initialize(void);
	void LoadSoundFile(void);
	void PlaySound(TCHAR* pSoundKey, CHANNEL_ID eChannel);
	void PlaySound(TCHAR* pSoundKey, CHANNEL_ID eChannel, float fVolume);
	void OncePlaySound(TCHAR* pSoundKey, CHANNEL_ID eChannel, float fVolume);
	void PlayBGM(TCHAR* pSoundKey, CHANNEL_ID eChannel, float fVolume);
	void UpdateSound(void);
	void StopSound(CHANNEL_ID eChannel);
	void StopSoundAll(void);
	void Release(void);
};
