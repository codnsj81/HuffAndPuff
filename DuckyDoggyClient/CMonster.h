#pragma once
#include "Object.h"
class CMonster :
	public CGameObject
{
public:
	CMonster();
	virtual ~CMonster();
	virtual int getCollision(CPlayer * player);
	virtual void Update();

private:
	int m_iHp = 100;
	int m_iAttack = 10;
};

