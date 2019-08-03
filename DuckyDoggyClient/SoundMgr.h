#pragma once

#include "../Headers/Enum.h"

#define SOUNDMGR CSoundMgr::GetInstance()

class CStringCmp
{
public:
	CStringCmp(const TCHAR* pKey)
		: m_pKey(pKey) {}

public:
	template <typename T>
	bool operator()(T& dst)
	{
		return !_tcscmp(dst.first, m_pKey);
	}

private:
	const TCHAR* m_pKey;
};

class CSoundMgr
{
private:
	CSoundMgr(const CSoundMgr&);
	CSoundMgr& operator = (const CSoundMgr&);
private:
	static CSoundMgr* m_pInstance;
public:
	static CSoundMgr* GetInstance() {
		if (NULL == m_pInstance)
			m_pInstance = new CSoundMgr;
		return m_pInstance;
	}
	void DestroyInstance() {
		if (m_pInstance) {
			delete m_pInstance;
			m_pInstance = NULL;
		}
	}

private:
	FMOD_SYSTEM* m_pSystem;
	FMOD_CHANNEL* m_pChannel[CHANNEL_END];

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
