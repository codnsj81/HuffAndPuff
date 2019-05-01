#include "stdafx.h"
#include "CMonster.h"
#include "Player.h"

CMonster::CMonster()
{
}


CMonster::~CMonster()
{
}

int CMonster::getCollision(CPlayer * player)
{
	float minX, maxX, minY, maxY, minZ, maxZ = 0.f; // i index
	float minX1, maxX1, minY1, maxY1, minZ1, maxZ1 = 0.f;// j index

	float pX, pY, sX, sY, sZ, pZ = 0.f;
	pX = m_xmf4x4World._41;
	pY = m_xmf4x4World._42;
	pZ = m_xmf4x4World._43;
	sX = m_Hitbox.x;
	sY = m_Hitbox.y;
	sZ = m_Hitbox.z;

	minX = pX - sX / 2.f; maxX = pX + sX / 2.f;
	minY = pY; maxY = pY + sY;
	minZ = pZ - sZ / 2.f; maxZ = pZ + sZ / 2.f;

	XMFLOAT3 pPos = player->GetPrecdictedPos();
	pX = pPos.x;
	pY = pPos.y;
	pZ = pPos.z;
	XMFLOAT3 pSize = player->GetHitBox();
	sX = pSize.x;
	sY = pSize.y;
	sZ = pSize.z;

	minX1 = pX - sX / 2.f; maxX1 = pX + sX / 2.f;
	minY1 = pY; maxY1 = pY + sY;
	minZ1 = pZ; maxZ1 = pZ + sZ;
	int result = CGameObject::BBCollision(minX, maxX, minY, maxY, minZ, maxZ, minX1, maxX1, minY1, maxY1, minZ1, maxZ1);

	if (result != COLLIDE_NONE && !player->m_bDamaging )
	{
		player->Damage(m_iAttack);
		player->m_bDamaging = true;
	}
	return 0;
}
