//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"
#include "Object.h"
#include "CUI.h"
#include "SoundMgr.h"
#include "Scene.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer


//#define _SAVENAV_MODE_


CPlayer::CPlayer()
{
	m_pCamera = NULL;

	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;


#ifdef _SAVENAV_MODE_
	return;
#else
	LoadNavigation();
#endif

}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) delete m_pCamera;
	if (m_pShadow)	delete m_pShadow;
	if (m_NavGuide) delete m_NavGuide;
}

void CPlayer::SetOriginMatrix()
{
	m_originmat = m_xmf4x4World;
}

void CPlayer::Reset()
{
	m_xmf4x4World = m_originmat;
	m_iHP = 100;
	m_iSkillGage = 0;
	m_navIndex = 0;
	m_navProcess = 0;
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
}

void CPlayer::SetNav(CGameObject * nav)
{
	m_NavGuide = nav;
	m_NavGuide->SetPosition(GetPosition());
}

void CPlayer::SetDE(CUI* nav)
{
	m_DashEffect = nav;
}

void CPlayer::NextRoad(float fTime)
{
	if (m_xmNavigationVector.empty())
		return;

	XMFLOAT3 now = GetPosition();
	XMFLOAT3 next = m_xmNavigationVector.at(m_navIndex);
	float length = Vector3::Length(Vector3::Subtract(PointingPos, now));
	

	if (length < 80.f )
	{
		if (next.x != 1)
		{
			PointingPos = next;
			m_navIndex++;
		}
		if(m_navProcess < m_navListSize)
		{
			m_navProcess++;
			if(m_ProgressUI)
				dynamic_cast<CProgressUI*> (m_ProgressUI)->Progressing();
		}
	}
	now.y += 7.f;
	if (m_NavGuide)
	{
		m_NavGuide->SetPosition(now);

		XMFLOAT3 xmf3Look = m_NavGuide->GetLook();
		XMFLOAT3 xmf3ToTarget = Vector3::Subtract(PointingPos, now,true);
		float fDotProduct = Vector3::DotProduct(xmf3Look, xmf3ToTarget);
		float fAngle = ::IsEqual(fDotProduct, 1.0f) ? 0.0f : ((fDotProduct > 0.0f) ? XMConvertToDegrees(acos(fDotProduct)) : 90.0f);
		XMFLOAT3 xmf3CrossProduct = Vector3::CrossProduct(xmf3Look, xmf3ToTarget);
		//	if (fAngle != 0.0f) Rotate(0.0f, fAngle * fElapsedTime * ((xmf3CrossProduct.y > 0.0f) ? 1.0f : -1.0f), 0.0f);
		m_NavGuide->Rotate(0.0f, fAngle * fTime * ((xmf3CrossProduct.y > 0.0f) ? 1.0f : -1.0f), 0.0f);
	}
}

void CPlayer::LoadNavigation()
{
	fstream in("NavData.txt", ios::in | ios::binary);
	while (in)
	{
		XMFLOAT3 dat;
		in >> dat.x;
		in >> dat.y;
		in >> dat.z;
		m_xmNavigationVector.emplace_back(dat);
	}
	PointingPos = m_xmNavigationVector.front();
	m_navListSize = m_xmNavigationVector.size();
	m_navProcess = 0;
}

void CPlayer::Attack()
{
	if (m_moveState == STATE_GROUND)
	{
		CSoundMgr::GetInstacne()->PlaySkillSound(_T("PlayerAtt"));
		SetAnimationSet(4);
		m_moveState = STATE_ATTACK;
	}

}

void CPlayer::PlusNavigationList()
{
	if (m_NavGuide == NULL) return;
	XMFLOAT3 now = GetPosition();
	if (m_xmNavigationVector.empty())
	{
		m_xmNavigationVector.emplace_back(now);
		return;
	}
	XMFLOAT3 next = m_xmNavigationVector.back();
	if (Vector3::Length(Vector3::Subtract(next, now))> 160.f)
	{
		m_xmNavigationVector.emplace_back(now);
	}
}

void CPlayer::SetLookVector(XMFLOAT3 xmf3Look)
{
	m_xmf3Look = xmf3Look;
}

void CPlayer::SetUpVector(XMFLOAT3 xmf3Up)
{
	m_xmf3Up = xmf3Up;
}

void CPlayer::SetRightVector(XMFLOAT3 xmf3Right)
{
	m_xmf3Right = xmf3Right;
}

void CPlayer::SetCheatMode()
{

	m_moveState = STATE_CHEAT;
}

void CPlayer::UseSkill()
{
	if (m_eSkillState != SKILL_FULL) return;
	m_eSkillState = SKILL_USING;
	m_iSkillGage = 0;
}

void CPlayer::Damage(int d)
{
	m_iHP -= d;
	if (m_iHP < 0) m_iHP = 0;
	else if (m_iHP > 100) m_iHP = 100;
}

void CPlayer::PlusSkillGage(int d)
{
	if (m_eSkillState != SKILL_CHARGING) return;

	m_iSkillGage += d; 
	if (m_iSkillGage >= 100)
	{
		m_iSkillGage = 100;
		m_eSkillState = SKILL_FULL;
	}
}

void CPlayer::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{

	if (dwDirection && !m_bPop)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, m_fSpeed);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -m_fSpeed);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, m_fSpeed);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -m_fSpeed);
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, m_fSpeed);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -m_fSpeed);

		Move(xmf3Shift, bUpdateVelocity);
	}
}


void CPlayer::CollideSide()
{
	m_CollideState = COLLIDEY;
}

bool CPlayer::CheckInWater(XMFLOAT3 pos, CHeightMapTerrain *pTerrain)
{
	if (m_moveState == STATE_JUMPING || m_moveState == STATE_ONOBJECTS) return false;

	for (int i = 0; i < m_nWater; i++)
	{
		XMFLOAT3 waterspos = m_ppWaters[i]->GetPosition();
		int halfwidth = m_ppWaters[i]->m_nWidth / 2;
		int halflength = m_ppWaters[i]->m_nLength / 2;

		float x = pos.x;
		float y = pos.y;

		if ((x < waterspos.x + halfwidth) && (x > waterspos.x - halfwidth) &&
			(y < waterspos.y + halflength) && (y > waterspos.y - halflength))
		{
			float waterheight = m_ppWaters[i]->GetPosition().y;
			float terrainheihgt = pTerrain->GetHeight(pos.x, pos.z, true) + 0.0f;
			if ((waterheight - terrainheihgt) > 3 && m_xmf3Position.y + 3 < waterheight)
			{
				m_xmf3Position.y = waterheight - 3;
				m_moveState = STATE_GROUND;
				m_iJumpnum = 0;
				return true;
			}
		}
	}
	return false;
}

void CPlayer::Move( XMFLOAT3 xmf3Shift, bool bUpdateVelocity)
{

	if (m_moveState == STATE_STUN || m_moveState == STATE_ATTACK) 
		return;
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pPlayerUpdatedContext;
		if (!pTerrain) return;

		XMFLOAT3 xmf3Scale = pTerrain->GetScale();
		m_predictedPos = Vector3::Add(m_xmf3Position, xmf3Shift);
		int z = (int)(m_predictedPos.z / xmf3Scale.z);
		bool bReverseQuad = ((z % 2) != 0);
		float fHeight = pTerrain->GetHeight(m_predictedPos.x, m_predictedPos.z, bReverseQuad) + 0.0f;


		if (m_moveState == STATE_CHEAT)
		{
			m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
			m_pCamera->Move(xmf3Shift);
		}

		if (pTerrain && m_fPreHeight != 0)
		{
			
			if (xmf3Shift.y > 0.1f) {
				int a = 2;
			}
			float minus = fHeight - m_fPreHeight;
			float degree;
			if (minus == 0) degree = 0;
			else
				degree = minus / (xmf3Shift.x * xmf3Shift.x + minus * minus);


		if (m_moveState == STATE_GROUND)
		{
			if (m_CollideState == COLLIDEN && degree < 0.3f) 
				m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		}
		else
		{
			if (m_CollideState == COLLIDEY)
			{
				if (m_moveState == STATE_FALLING)
				{
						xmf3Shift = XMFLOAT3(0, xmf3Shift.y, 0);
				}	
			}
				m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		}

		}
		else
			m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		m_pCamera->Move(xmf3Shift);

	}
}

void CPlayer::Dash(float fDistance)
{
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Look, fDistance);
	m_bDash = true;
	m_fMaxVelocityXZ = 70;
}

void CPlayer::Pop(float fDistance)
{
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Look, -fDistance);
	m_bPop = true;
	Jump();
	m_fMaxVelocityXZ = 70;
}
void CPlayer::Rotate(float x, float y, float z)
{
	if (m_moveState == STATE_STUN) return;
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA) || (nCurrentCameraMode == THIRD_PERSON_CAMERA))
	{
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		m_pCamera->Rotate(x, y, z);

		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(z));
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);



}

void CPlayer::SetStun()
{
	m_moveState = STATE_STUN;
	m_bDamaging = true;
}

void CPlayer::OnObject(float fy)
{
	m_moveState = STATE_ONOBJECTS;
	m_ObjectHeight = fy;
	m_iJumpnum = 0;
}

void CPlayer::Update(float fTimeElapsed)
{

	float fDistance = 0;
	if (m_moveState != STATE_CHEAT && m_moveState != STATE_GROUND && m_moveState != STATE_ONOBJECTS && m_moveState != STATE_STUN && m_moveState != STATE_ATTACK )
	{
		m_fTime += fTimeElapsed;
		fDistance = 30.f - 90.f * m_fTime ;
		if (fDistance < 0)
			m_moveState = STATE_FALLING;

		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, m_xmf3Up, fDistance);
	}

	//if (fDistance >= 0)
	XMFLOAT3 gravity = XMFLOAT3(0, m_xmf3Gravity.y * fTimeElapsed, 0.f);
	if(m_moveState!=STATE_FALLING)	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, gravity);

	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ;
	if (m_bDash|| m_bPop)
	{
		if (m_bDash)
		{
			m_fMaxVelocityXZ -= fTimeElapsed * 20;
			if (m_fMaxVelocityXZ <= 50.f)
			{
				m_fMaxVelocityXZ = 30.f;
				m_bDash = false;
			}
		}
		if (m_bPop)
		{

			m_fMaxVelocityXZ -= fTimeElapsed * 5;	
			if (m_moveState == STATE_GROUND)
			{
				m_fMaxVelocityXZ = 30.f;
				m_bPop = false;
			}
		}
		

	}

	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	float fMaxVelocityY = m_fMaxVelocityY;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);

	Move(xmf3Velocity, false);
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
	
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	XMFLOAT3 pos = m_xmf3Position;
	pos.y += 7.f;
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(pos);
	m_pCamera->RegenerateViewMatrix();

	if (m_pAnimationController) m_pAnimationController->SetLoop(true);

	if (m_moveState == STATE_GROUND)
	{
		if(m_bInWater) SetAnimationSet(3);
		else if (Vector3::IsZero(m_xmf3Velocity))
			SetAnimationSet(0);
		else
			SetAnimationSet(1);
	}
	else if (m_moveState == STATE_ATTACK)
	{
		if (m_pChild->m_pAnimationController->m_bAnimationEnd)
			m_moveState = STATE_GROUND;
	}
	
	if (m_bDamaging)
	{
		m_fDamagingTime += fTimeElapsed;
		if (m_moveState !=STATE_STUN && m_fDamagingTime > 0.8f)
		{
			m_bDamaging = false;
			m_fDamagingTime = 0.f;
			
		}
		else if (m_moveState == STATE_STUN && m_fDamagingTime > 1.f)
		{
			m_moveState = STATE_GROUND;
			m_bDamaging = false;
			m_fDamagingTime = 0.f;
			m_pScene->SetBloodScreenState(false);
			
		}
		if (m_iHP < 0)
			int a = 0;
	}
	if (m_eSkillState == SKILL_USING)
	{
		m_fSkillTime += fTimeElapsed;
		if (m_fSkillTime > 5)
		{
			m_eSkillState = SKILL_CHARGING;
			m_fSkillTime = 0;
		}
	}
	if(m_DashEffect)
		m_DashEffect->Update(fTimeElapsed);

#ifdef _SAVENAV_MODE_
	PlusNavigationList();
#else
	NextRoad(fTimeElapsed);
#endif

}

CCamera *CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera *pNewCamera = NULL;
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			pNewCamera = new CFirstPersonCamera(m_pCamera);
			break;
		case THIRD_PERSON_CAMERA:
			pNewCamera = new CThirdPersonCamera(m_pCamera);
			break;
		case SPACESHIP_CAMERA:
			pNewCamera = new CSpaceShipCamera(m_pCamera);
			break;
	}
	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xmf3Right = Vector3::Normalize(XMFLOAT3(m_xmf3Right.x, 0.0f, m_xmf3Right.z));
		m_xmf3Up = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_xmf3Look = Vector3::Normalize(XMFLOAT3(m_xmf3Look.x, 0.0f, m_xmf3Look.z));

		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = Vector3::Angle(XMFLOAT3(0.0f, 0.0f, 1.0f), m_xmf3Look);
		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}

	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->SetPlayer(this);
	}

	if (m_pCamera) delete m_pCamera;

	return(pNewCamera);
}

void CPlayer::Jump()
{

	if (m_iJumpnum < 2)
	{
		m_fTime = 0;
		m_moveState = STATE_JUMPING;

		m_iJumpnum++;
		SetAnimationSet(STATE_JUMPING);
		if(!m_bPop)CSoundMgr::GetInstacne()->PlayEffectSound(_T("Jump"));
	}

}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4ToParent._11 = m_xmf3Right.x; m_xmf4x4ToParent._12 = m_xmf3Right.y; m_xmf4x4ToParent._13 = m_xmf3Right.z;
	m_xmf4x4ToParent._21 = m_xmf3Up.x; m_xmf4x4ToParent._22 = m_xmf3Up.y; m_xmf4x4ToParent._23 = m_xmf3Up.z;
	m_xmf4x4ToParent._31 = m_xmf3Look.x; m_xmf4x4ToParent._32 = m_xmf3Look.y; m_xmf4x4ToParent._33 = m_xmf3Look.z;
	m_xmf4x4ToParent._41 = m_xmf3Position.x; m_xmf4x4ToParent._42 = m_xmf3Position.y; m_xmf4x4ToParent._43 = m_xmf3Position.z;

	m_xmf4x4ToParent = Matrix4x4::Multiply(XMMatrixScaling(m_xmf3Scale.x, m_xmf3Scale.y, m_xmf3Scale.z), m_xmf4x4ToParent);

	UpdateTransform(NULL);

}

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	if (nCameraMode == THIRD_PERSON_CAMERA) CGameObject::Render(pd3dCommandList, pCamera);
	if (m_bDash)
		m_DashEffect->bRender = true;
	else
		m_DashEffect->bRender = false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CAirplanePlayer::CAirplanePlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext)
{
	m_pCamera = ChangeCamera(/*SPACESHIP_CAMERA*/THIRD_PERSON_CAMERA, 0.0f);

	CGameObject *pGameObject = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, "Model/Mi24.bin", NULL, false);
	SetChild(pGameObject);

	OnPrepareAnimate();

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CAirplanePlayer::~CAirplanePlayer()
{
}

void CAirplanePlayer::OnPrepareAnimate()
{
	m_pMainRotorFrame = FindFrame("Top_Rotor");
	m_pTailRotorFrame = FindFrame("Tail_Rotor");
}

void CAirplanePlayer::Animate(float fTimeElapsed)
{
	if (m_pMainRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 2.0f) * fTimeElapsed);
		m_pMainRotorFrame->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_pMainRotorFrame->m_xmf4x4ToParent);
	}
	if (m_pTailRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 4.0f) * fTimeElapsed);
		m_pTailRotorFrame->m_xmf4x4ToParent = Matrix4x4::Multiply(xmmtxRotate, m_pTailRotorFrame->m_xmf4x4ToParent);
	}

	CPlayer::Animate(fTimeElapsed);
}

void CAirplanePlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();
}

CCamera *CAirplanePlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			SetFriction(1.f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(2.5f);
			SetMaxVelocityY(10.0f);
			m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case SPACESHIP_CAMERA:
			SetFriction(1.f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(40.0f);
			SetMaxVelocityY(10.0f);
			m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 200.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case THIRD_PERSON_CAMERA:
			SetFriction(1.f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(3.5f);
			SetMaxVelocityY(10.0f);
			m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.25f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 15.0f, -30.0f));
			m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		default:
			break;
	}
	Update(fTimeElapsed);

	return(m_pCamera);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CTerrainPlayer::CTerrainPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, char* name, int kind, bool banimation, void *pContext)
{
	m_playerKind = kind;
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	CGameObject *pGameObject = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, name, NULL, banimation);
	

	pGameObject->m_pAnimationController->m_pAnimationSets[4].m_fSpeed = 0.6f;
		pGameObject->m_pAnimationController->m_pAnimationSets[3].m_fSpeed = 0.8f;
		pGameObject->m_pAnimationController->m_pAnimationSets[1].m_fSpeed = 0.8f;

	pGameObject->m_pAnimationController->SetKind(kind);
	SetChild(pGameObject);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	SetPlayerUpdatedContext(pContext);
	SetCameraUpdatedContext(pContext);

	m_pShadow = new CShadow(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, 3, 3, GetPosition());
	
}

CTerrainPlayer::~CTerrainPlayer()
{
}

CCamera *CTerrainPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
		case FIRST_PERSON_CAMERA:
			SetFriction(250.0f);
			SetGravity(XMFLOAT3(0.0f, -0.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case SPACESHIP_CAMERA:
			SetFriction(125.0f);
			SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
			SetMaxVelocityXZ(300.0f);
			SetMaxVelocityY(400.0f);
			m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.0f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
			m_pCamera->GenerateProjectionMatrix(1.01f, 100.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			break;
		case THIRD_PERSON_CAMERA:
			SetFriction(100.0f);
			SetGravity(XMFLOAT3(0.0f, -200.f, 0.0f));
			SetMaxVelocityXZ(30.0f);
			SetMaxVelocityY(25.0f);
			m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
			m_pCamera->SetTimeLag(0.25f);
			m_pCamera->SetOffset(XMFLOAT3(0.0f, 10.0f, -25.0f));
			m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
			m_pCamera->GenerateProjectionMatrix(1.01f, 1000.0f, ASPECT_RATIO, 60.0f);
			m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
			m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
			
			break;
		default:
			break;
	}
	Update(fTimeElapsed);

	return(m_pCamera);
}

void CTerrainPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pPlayerUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);

	m_pShadow->SetPosition(xmf3PlayerPosition.x, pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.y) + 1, xmf3PlayerPosition.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight;
	if (m_moveState != STATE_ONOBJECTS)
		fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad) + 0.0f;
	else
		fHeight = m_ObjectHeight;
	if (xmf3PlayerPosition.y < fHeight)
	{
		m_fTime = 0;
		if (m_moveState != STATE_GROUND && m_moveState != STATE_STUN && m_moveState != STATE_CHEAT
			&& m_moveState != STATE_ATTACK)
		{
			m_iJumpnum = 0;
			m_moveState = STATE_GROUND;
		}

		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		
		SetVelocity(xmf3PlayerVelocity);
		xmf3PlayerPosition.y = fHeight;

		SetPosition(xmf3PlayerPosition);
	}
	if (m_moveState == STATE_GROUND || m_moveState == STATE_FALLING )
	{
		m_bInWater = CheckInWater(xmf3PlayerPosition, pTerrain);
	}
	m_fPreHeight = fHeight;

}

void CTerrainPlayer::OnCameraUpdateCallback(float fTimeElapsed)
{
	XMFLOAT3 pos = GetPosition();
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pCameraUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	int z = (int)(xmf3CameraPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);

	float fHeight;
	if (m_bInWater)
	{
		fHeight = m_xmf3Position.y + 10.0f;
	}
	else
		fHeight = pTerrain->GetHeight(xmf3CameraPosition.x, xmf3CameraPosition.z, bReverseQuad) + 5.0f;
	if (xmf3CameraPosition.y <= fHeight)
	{
		xmf3CameraPosition.y = fHeight;
		m_pCamera->SetPosition(xmf3CameraPosition);
		if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			CThirdPersonCamera *p3rdPersonCamera = (CThirdPersonCamera *)m_pCamera;
				p3rdPersonCamera->SetLookAt(pos);
		}
	}
	if (m_bCheatmode) // 치트시 카메라 이동
		m_pCamera->SetPosition(XMFLOAT3(xmf3CameraPosition.x, 1500, xmf3CameraPosition.z));
}
