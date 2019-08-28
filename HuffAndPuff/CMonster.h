#pragma once
#include "Object.h"

class CScene;
class CPlayer;
class CMonster :
	public CGameObject
{
public:
	CMonster();
	virtual ~CMonster();
	virtual int getCollision(CPlayer * player);
	virtual void Animate(float fTimeElapsed);

	float GetAggroDistance() { return m_fAggroDistance; }
	void SetaggroDistance(float f) { m_fAggroDistance = f; }

	void setAP(int ap) { m_iAttack = ap; }
	virtual bool Damage(int dam);

	void setRecognitionMode(bool b) { m_bRecognition = b; }
	bool getRecognitionMode() { return m_bRecognition; }

	XMFLOAT3 FollowingPosition;

	CScene* m_pScene = NULL;
	void ResetToNext(XMFLOAT3 pos);
	void SetScene(CScene* scene) { m_pScene = scene; }
	bool GetDeathState() { return m_bDeath; }
	void SetLookVector(XMFLOAT3 xmf3Look)
	{
		m_xmf4x4World._31 = xmf3Look.x;
		m_xmf4x4World._32 = xmf3Look.y; 
		m_xmf4x4World._33 = xmf3Look.z;
	}

	void SetRightVector(XMFLOAT3 xmf3Right)
	{
		m_xmf4x4World._11 = xmf3Right.x;
		m_xmf4x4World._12 = xmf3Right.y;
		m_xmf4x4World._13 = xmf3Right.z;
	}
	int	GetIndex() { return m_index; }
	void Next() { m_index++; }
protected:
	int m_index = 0;
	int m_iType;
	int m_iHp = 100;
	float m_fAggroDistance;
	int m_iAttack;
	bool m_bDeath = false;
	bool m_bAttacking = false;
	bool m_bRecognition = false;
	bool m_bDeathING = false;
};


class CSnake : public CMonster
{
public:
	CSnake();
	~CSnake() {}

	virtual void Animate(float fTimeElapsed);
	virtual int getCollision(CPlayer * player);
	virtual bool Damage(int dam);


	void SetDeathING(bool bDeath) {
		m_bDeathING = bDeath;
		m_pChild->m_pAnimationController->SetLoop(false);
		SetAnimationSet(3);
	}
private:
};

class CFish : public CMonster
{
public:
	CFish() {}
	~CFish() {}
	virtual void Animate(float fTimeElapsed);
};