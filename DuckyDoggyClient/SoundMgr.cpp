#include "StdAfx.h"
#include "SoundMgr.h"

CSoundMgr* CSoundMgr::m_pInstance = NULL;

CSoundMgr::CSoundMgr(void)
	: m_pSystem(NULL)
{
	Initialize();
}

CSoundMgr::~CSoundMgr(void)
{
	Release();
}

void CSoundMgr::Release(void)
{
	map<TCHAR*, FMOD_SOUND*>::iterator iter_begin = m_MapSound.begin();
	map<TCHAR*, FMOD_SOUND*>::iterator iter_end = m_MapSound.end();

	for (; iter_begin != iter_end; ++iter_begin)
	{
		delete[] iter_begin->first;
		FMOD_Sound_Release(iter_begin->second);
	}

	m_MapSound.clear();

	FMOD_System_Close(m_pSystem);
	FMOD_System_Release(m_pSystem);
}

void CSoundMgr::Initialize(void)
{
	//unsigned int niVersion = NULL;//
	FMOD_System_Create(&m_pSystem);
	//FMOD_System_GetVersion(m_pSystem, &niVersion);//
	FMOD_System_Init(m_pSystem, CHANNEL_END, FMOD_INIT_NORMAL, NULL);

	LoadSoundFile();
}

void CSoundMgr::LoadSoundFile(void)
{
	FMOD_RESULT FResult;
	FMOD_SOUND* pSound = nullptr;
	for (int i = 0; i < 11; i++) {
		wchar_t* pwstring = new wchar_t[100];
		wsprintf(pwstring, L"Sound/Sound%d.mp3", i);
		// wchar_t to char
		char* pStr;
		int strSize = WideCharToMultiByte(CP_ACP, 0, pwstring, -1, NULL, 0, NULL, NULL);
		pStr = new char[strSize];
		WideCharToMultiByte(CP_ACP, 0, pwstring, -1, pStr, strSize, 0, 0);
		//
		if(i == 6 || i ==9)
			 FResult = FMOD_System_CreateSound(m_pSystem, pStr, FMOD_DEFAULT, NULL, &pSound);
		else
			FResult = FMOD_System_CreateSound(m_pSystem, pStr, FMOD_LOOP_NORMAL, NULL, &pSound);
		if (FMOD_OK == FResult)
			m_MapSound.insert(pair<TCHAR*, FMOD_SOUND*>(pwstring, pSound));
	}
}

void CSoundMgr::PlaySound(TCHAR* pSoundKey, CHANNEL_ID eChannel)
{
	// FMOD_System_Update(m_pSystem);

	map<TCHAR*, FMOD_SOUND*>::iterator iter = find_if(m_MapSound.begin(),
		m_MapSound.end(), CStringCmp(pSoundKey));

	if (iter == m_MapSound.end())
		return;

	FMOD_System_PlaySound(m_pSystem, iter->second, NULL, 0, &(m_pChannel[eChannel]));


}

void CSoundMgr::PlaySound(TCHAR* pSoundKey, CHANNEL_ID eChannel, float fVolume)
{
	//FMOD_System_Update(m_pSystem);

	map<TCHAR*, FMOD_SOUND*>::iterator iter = find_if(m_MapSound.begin(),
		m_MapSound.end(), CStringCmp(pSoundKey));

	if (iter == m_MapSound.end())
		return;

	FMOD_System_PlaySound(m_pSystem, iter->second, NULL, 0, &(m_pChannel[eChannel]));
	FMOD_Channel_SetVolume(m_pChannel[eChannel], fVolume);

}


void CSoundMgr::OncePlaySound(TCHAR* pSoundKey, CHANNEL_ID eChannel, float fVolume)
{
	map<TCHAR*, FMOD_SOUND*>::iterator iter = find_if(m_MapSound.begin(),
		m_MapSound.end(), CStringCmp(pSoundKey));

	if (iter == m_MapSound.end())
		return;

	FMOD_BOOL isPlaying;
	FMOD_Channel_IsPlaying(m_pChannel[eChannel], &isPlaying);

	if (!isPlaying)
	{
		FMOD_System_PlaySound(m_pSystem, iter->second, NULL, 0, &(m_pChannel[eChannel]));
		FMOD_Channel_SetVolume(m_pChannel[eChannel], fVolume);
	}
}


void CSoundMgr::PlayBGM(TCHAR* pSoundKey, CHANNEL_ID eChannel, float fVolume)
{
	map<TCHAR*, FMOD_SOUND*>::iterator iter = find_if(m_MapSound.begin(),
		m_MapSound.end(), CStringCmp(pSoundKey));

	if (iter == m_MapSound.end())
		return;

	//	FMOD_Sound_SetLoopCount(iter->second, -1);
	//	FMOD_Sound_SetMode(iter->second, FMOD_LOOP_NORMAL);
	FMOD_System_PlaySound(m_pSystem, iter->second, NULL, 0, &(m_pChannel[eChannel]));
}

void CSoundMgr::UpdateSound(void)
{
	FMOD_System_Update(m_pSystem);
}

void CSoundMgr::StopSound(CHANNEL_ID eChannel)
{
	FMOD_Channel_Stop(m_pChannel[eChannel]);
}

void CSoundMgr::StopSoundAll(void)
{
	for (int i = 0; i < CHANNEL_END; ++i)
		FMOD_Channel_Stop(m_pChannel[i]);
}

