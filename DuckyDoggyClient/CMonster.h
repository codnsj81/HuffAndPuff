#pragma once
#include "Object.h"

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
	void Damage(int dam);

	void setRecognitionMode(bool b) { m_bRecognition = b; }
	bool getRecognitionMode() { return m_bRecognition; }

	XMFLOAT3 FollowingPosition;

	bool GetDeathState() { return m_bDeath; }

protected:
	int m_iType;
	int m_iHp = 100;
	float m_fAggroDistance;
	int m_iAttack;
	bool m_bDeath = false;

	bool m_bRecognition = false;
};


class CSnake : public CMonster
{
public:
	CSnake();
	~CSnake() {}

	virtual void Animate(float fTimeElapsed);
	virtual int getCollision(CPlayer * player);
private:
};