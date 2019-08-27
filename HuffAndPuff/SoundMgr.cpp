#include "stdafx.h"
#include "SoundMgr.h"

CSoundMgr* CSoundMgr::m_pInstance = NULL;

CSoundMgr::CSoundMgr(void)
{
	m_pSystem	= NULL;

	m_pEffect	= NULL;	
	m_pMainBGM	= NULL;
	m_pSkill	= NULL;	
	m_pMonster	= NULL;

	m_iVersion = 0;

	m_Result = FMOD_OK;

	Initialize();
}

CSoundMgr::~CSoundMgr(void)
{
	map<TCHAR*, FMOD::Sound*>::iterator iter     = m_mapSound.begin();
	map<TCHAR*, FMOD::Sound*>::iterator iter_end = m_mapSound.end();
	
	for(iter; iter != iter_end; )
	{
		m_Result = iter->second->release();
		ErrorCheck(m_Result);

		delete [] (iter->first);

		iter = m_mapSound.erase(iter);
		iter_end = m_mapSound.end();

		if(iter == iter_end)
			break;
	}

	m_Result = m_pSystem->close();
	ErrorCheck(m_Result);

	m_Result = m_pSystem->release();
	ErrorCheck(m_Result);
}

void CSoundMgr::Initialize(void)
{

	m_Result = FMOD::System_Create(&m_pSystem);
	ErrorCheck(m_Result);

	m_Result = m_pSystem->getVersion(&m_iVersion);
	ErrorCheck(m_Result);

	m_pSystem->init(32, FMOD_INIT_NORMAL, NULL);
	ErrorCheck(m_Result);
	m_pMainBGM->setVolume(0.7f);
}

void CSoundMgr::LoadSoundFile(void)
{


	FMOD::Sound* pSound;
	m_Result = m_pSystem->createSound("SoundFile/LogoBGM.wav", FMOD_LOOP_NORMAL, 0, &pSound);
	TCHAR* pName = _T("LogoBGM");
	if (m_Result == FMOD_OK)
	{
		m_mapSound.insert(make_pair(pName, pSound));
	}

	m_Result = m_pSystem->createSound("SoundFile/InGame.flac", FMOD_DEFAULT, 0, &pSound);
	pName = _T("InGame");
	if (m_Result == FMOD_OK)
	{
		m_mapSound.insert(make_pair(pName, pSound));
	}

	m_Result = m_pSystem->createSound("SoundFile/Trap.ogg", FMOD_DEFAULT, 0, &pSound);
	pName = _T("Trap");
	if (m_Result == FMOD_OK)
	{
		m_mapSound.insert(make_pair(pName, pSound));
	}

	m_Result = m_pSystem->createSound("SoundFile/Gulp.mp3", FMOD_DEFAULT, 0, &pSound);
	pName = _T("Gulp");
	if (m_Result == FMOD_OK)
	{
		m_mapSound.insert(make_pair(pName, pSound));
	}

	m_Result = m_pSystem->createSound("SoundFile/Jump.mp3", FMOD_DEFAULT, 0, &pSound);
	pName = _T("Jump");
	if (m_Result == FMOD_OK)
	{
		m_mapSound.insert(make_pair(pName, pSound));
	}
	m_Result = m_pSystem->createSound("SoundFile/Button.mp3", FMOD_DEFAULT, 0, &pSound);
	pName = _T("Button");
	if (m_Result == FMOD_OK)
	{
		m_mapSound.insert(make_pair(pName, pSound));
	}
	
	m_Result = m_pSystem->createSound("SoundFile/dash.wav", FMOD_DEFAULT, 0, &pSound);
	pName = _T("dash");
	if (m_Result == FMOD_OK)
	{
		m_mapSound.insert(make_pair(pName, pSound));
	}

	m_Result = m_pSystem->createSound("SoundFile/MonAtt.ogg", FMOD_DEFAULT, 0, &pSound);
	pName = _T("MonAtt");
	if (m_Result == FMOD_OK)
	{
		m_mapSound.insert(make_pair(pName, pSound));
	}

	m_Result = m_pSystem->createSound("SoundFile/PlayerAtt.ogg", FMOD_DEFAULT, 0, &pSound);
	pName = _T("PlayerAtt");
	if (m_Result == FMOD_OK)
	{
		m_mapSound.insert(make_pair(pName, pSound));
	}

	m_Result = m_pSystem->createSound("SoundFile/Mushroom.mp3", FMOD_DEFAULT, 0, &pSound);
	pName = _T("Mushroom");
	if (m_Result == FMOD_OK)
	{
		m_mapSound.insert(make_pair(pName, pSound));
	}


	m_Result = m_pSystem->createSound("SoundFile/Snake.ogg", FMOD_DEFAULT, 0, &pSound);
	pName = _T("Snake");
	if (m_Result == FMOD_OK)
	{
		m_mapSound.insert(make_pair(pName, pSound));
	}

	m_Result = m_pSystem->createSound("SoundFile/Falling.ogg", FMOD_DEFAULT, 0, &pSound);
	pName = _T("Falling");
	if (m_Result == FMOD_OK)
	{
		m_mapSound.insert(make_pair(pName, pSound));
	}

	m_Result = m_pSystem->update();
	ErrorCheck(m_Result);
}

void CSoundMgr::PlayEffectSound(TCHAR* pSoundKey)
{
	map<TCHAR*, FMOD::Sound*>::iterator iter;

	iter = find_if(m_mapSound.begin(), m_mapSound.end(), CStringCMP(pSoundKey));

	if(iter == m_mapSound.end())
		return;

	m_Result = m_pSystem->playSound(iter->second, 0, false, &m_pEffect);
	ErrorCheck(m_Result);
}

void CSoundMgr::PlayBGMSound(TCHAR* pSoundKey)
{
	map<TCHAR*, FMOD::Sound*>::iterator iter;

	iter = find_if(m_mapSound.begin(), m_mapSound.end(), CStringCMP(pSoundKey));

	if(iter == m_mapSound.end())
		return;

	iter->second->setMode(FMOD_LOOP_NORMAL);

	m_Result = m_pSystem->playSound((*iter).second, 0, false, &m_pMainBGM);
	ErrorCheck(m_Result);
}

void CSoundMgr::PlaySkillSound(TCHAR* pSoundKey)
{
	map<TCHAR*, FMOD::Sound*>::iterator iter;

	iter = find_if(m_mapSound.begin(), m_mapSound.end(), CStringCMP(pSoundKey));

	if(iter == m_mapSound.end())
		return;

	m_Result = m_pSystem->playSound((*iter).second, 0, false, &m_pSkill);
	ErrorCheck(m_Result);
}

void CSoundMgr::StopBGM(void)
{
	m_Result = m_pMainBGM->stop();
	ErrorCheck(m_Result);
}

void CSoundMgr::StopALL(void)
{
	m_Result = m_pMainBGM->stop();
	ErrorCheck(m_Result);

	m_Result = m_pEffect->stop();
	ErrorCheck(m_Result);

	m_Result = m_pSkill->stop();
	ErrorCheck(m_Result);

	m_Result = m_pMonster->stop();
	ErrorCheck(m_Result);
}


void CSoundMgr::ErrorCheck(FMOD_RESULT _result)
{
	if(_result != FMOD_OK)
	{
		//cout << "¿À·ù : " << _result << endl;
		//system("pause");
		return;
	}
}
