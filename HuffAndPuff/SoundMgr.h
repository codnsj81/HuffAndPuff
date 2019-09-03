#pragma once

#include "stdafx.h"
enum eSOundTpe
{
	asdsasd
	,asda
	,asd,asdaas,asdasd
};

class CStringCMP
{
private:
	const TCHAR* m_pString;

public:
	explicit CStringCMP(const TCHAR* pKey)
		:m_pString(pKey)
	{}
	~CStringCMP() {}

public:
	template<class T>
	bool operator () (T data)
	{
		return !lstrcmp(data.first, m_pString);
	}
};

class CSoundMgr
{
	//사운드 매니저의 기능을 부여하자.

private:
	FMOD::System*	m_pSystem;		//fmod 시스템을 관리할 포인터.
	FMOD::Channel*	m_pEffect;		//효과음 채널.
	FMOD::Channel*	m_pMainBGM;		//배경음 채널.
	FMOD::Channel*	m_pSkill;		//스킬 채널.
	FMOD::Channel*	m_pWater;		//물 채널.


	FMOD_RESULT		m_Result;		//Fmod 에러 체크.

	unsigned int m_iVersion;		
	
	map<TCHAR*, FMOD::Sound*>	m_mapSound;


public:
	//실제 인터페이스를 정의.
	void Initialize(void);
	void LoadSoundFile(void);
	
	void PlayEffectSound(TCHAR* pSoundKey);
	void PlayBGMSound(TCHAR* pSoundKey);
	void PlaySkillSound(TCHAR* pSoundKey);
	void PlayWaterSound(TCHAR* pSoundKey);

	void StopBGM(void);
	void StopWater(void);
	void StopALL(void);

public:
	map<TCHAR*, FMOD::Sound*>*	GetSoundMap(void)
	{
		return &m_mapSound;
	}

private:
	void ErrorCheck(FMOD_RESULT _result);

private:
	static CSoundMgr* m_pInstance;

public:
	static CSoundMgr* GetInstacne(void)
	{
		if(m_pInstance == NULL)
			m_pInstance = new CSoundMgr;

		return m_pInstance;
	}

	void Update(void) {
		m_pSystem->update();
	}

	void Destroy(void)
	{
		if(m_pInstance)
		{
			delete m_pInstance;
			m_pInstance = NULL;
		}
	}
private:
	CSoundMgr(void);
public:
	~CSoundMgr(void);
};
