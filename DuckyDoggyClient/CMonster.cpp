#include "stdafx.h"
#include "CMonster.h"
#include "Player.h"
#include "Scene.h"

CMonster::CMonster()
{
	m_iType = MONTYPE_SNAKE;
	SetAnimationSet(0);
}


CMonster::~CMonster()
{
}

int CMonster::getCollision(CPlayer * player)
{
	float minX, maxX, minY, maxY, minZ, maxZ = 0.f; // i ifndex
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

	
	return result;
}

void CMonster::Animate(float fTimeElapsed)
{
	CGameObject::Animate(fTimeElapsed);
	if (m_pAnimationController) m_pAnimationController->SetLoop(true);

}

bool CMonster::Damage(int dam)
{
	m_iHp -= dam;
	if (m_iHp < 0)
	{
		return true;
	}
	return false;
}

CSnake::CSnake()
{
	CMonster::CMonster();
	setAP(10);
	SetaggroDistance(50.f);
}

void CSnake::Animate(float fTimeElapsed)
{
	CMonster::Animate(fTimeElapsed);

	if (m_bDeathING)
	{
		if (m_pChild->m_pAnimationController->m_bAnimationEnd == true)
		{
			m_bDeath = true;
			m_bDeathING = false;
		}
		return;
	}

	// 다른 클라이언트의 snake 정보 위주여야 한다면 패턴 X
	if (m_bIsMain == false)
		return;

	if (m_bRecognition)
	{
		XMFLOAT3 xmf3Position = GetPosition();

		XMFLOAT3 xmf3Look = GetLook();
		XMFLOAT3 xmf3ToTarget = Vector3::Subtract(FollowingPosition, xmf3Position, true);
		float fDotProduct = Vector3::DotProduct(xmf3Look, xmf3ToTarget);
		float fAngle = ::IsEqual(fDotProduct, 1.0f) ? 0.0f : ((fDotProduct > 0.0f) ? XMConvertToDegrees(acos(fDotProduct)) : 90.0f);
		XMFLOAT3 xmf3CrossProduct = Vector3::CrossProduct(xmf3Look, xmf3ToTarget);
		//	if (fAngle != 0.0f) Rotate(0.0f, fAngle * fElapsedTime * ((xmf3CrossProduct.y > 0.0f) ? 1.0f : -1.0f), 0.0f);
		Rotate(0.0f, fAngle * fTimeElapsed * ((xmf3CrossProduct.y > 0.0f) ? 1.0f : -1.0f), 0.0f);
		SetPosition(Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Look, 10 * fTimeElapsed)));
		if (m_bAttacking)
		{
			if (m_pChild->m_pAnimationController->m_bAnimationEnd == true)
				m_bAttacking = false;
		}
		else
		{
			m_pChild->m_pAnimationController->SetLoop(true);
			SetAnimationSet(1);
		}

		// snake의 위치, 애니메이션 상태 서버에 전송
		m_UpdateCnt++;
		if (g_myinfo.connected == true && m_UpdateCnt>=3) {
			snake_info s_info;
			s_info.id = m_iID;
			s_info.x = m_xmf4x4ToParent._41; s_info.y = m_xmf4x4ToParent._42; s_info.z = m_xmf4x4ToParent._43;
			s_info.l_x = m_xmf4x4ToParent._31; s_info.l_y = m_xmf4x4ToParent._32; s_info.l_z = m_xmf4x4ToParent._33;
			s_info.r_x = m_xmf4x4ToParent._11; s_info.r_y = m_xmf4x4ToParent._12; s_info.l_z = m_xmf4x4ToParent._13;
			s_info.animationSet = m_pChild->m_pAnimationController->GetAnimationSet();
			int retval;
			/// 고정
			packet_info packetinfo;
			packetinfo.type = cs_notify_snakeinfo;
			packetinfo.size = sizeof(snake_info);
			packetinfo.id = g_myinfo.id;
			char buf[BUFSIZE];
			memcpy(buf, &packetinfo, sizeof(packetinfo));
			/// 가변 (고정 데이터에 가변 데이터 붙이는 형식으로)
			memcpy(buf + sizeof(packetinfo), &s_info, sizeof(snake_info));
			retval = send(g_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				MessageBoxW(g_hWnd, L"send()", L"send() - cs_notify_monsterinfo", MB_OK);
				exit(1);
			}
			cout << "send() -cs_notify_snakeinfo " << endl;
			m_UpdateCnt = 0;
		}
	}

	else
	{
		SetAnimationSet(0);
	}

}

int CSnake::getCollision(CPlayer * player)
{
	if (m_bDeathING) return 0;
	int result = CMonster::getCollision(player);
	if (result != COLLIDE_NONE)
	{
		if (!player->m_bDamaging && !m_bDeathING)
		{
			player->Damage(m_iAttack);
			m_pScene->CreateDamageUI(player, m_iAttack);
			player->m_bDamaging = true;
		}
		if (!m_bAttacking)
		{
			SetAnimationSet(2);
			m_bAttacking = true;
		}
	}
	return 0;
}

bool CSnake::Damage(int dam)
{
	if (m_bDeathING) return true;
	bool result = CMonster::Damage(dam);
	if (result)
	{
		m_bDeathING = true;
		m_pChild->m_pAnimationController->SetLoop(false);
		SetAnimationSet(3);

		// 몬스터가 죽었다는 패킷 전송
		if (g_myinfo.connected == true) {
			int id = m_iID;
			int retval;
			/// 고정
			packet_info packetinfo;
			packetinfo.type = cs_snake_is_dead;
			packetinfo.size = sizeof(int);
			packetinfo.id = g_myinfo.id;
			char buf[BUFSIZE];
			memcpy(buf, &packetinfo, sizeof(packetinfo));
			/// 가변 (고정 데이터에 가변 데이터 붙이는 형식으로)
			memcpy(buf + sizeof(packetinfo), &id, sizeof(int));
			retval = send(g_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				MessageBoxW(g_hWnd, L"send()", L"send() - cs_monster_is_dead", MB_OK);
				exit(1);
			}
		}
		cout << "send() - cs_monster_is_dead " << endl;
	}
}
