#pragma once

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

#define PIGGYBACK_NONE 0
#define PIGGYBACK_CARRY	1
#define PIGGYBACK_CRRIED 2



// 플레이어충돌
#define COLLIDEY	0
#define COLLIDEN	1


#include "Object.h"
#include "Camera.h"
#include "../Headers/Include.h"

class CPlayer : public CGameObject
{
protected:
	
	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3					m_xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	float           			m_fPitch = 0.0f;
	float           			m_fYaw = 0.0f;
	float           			m_fRoll = 0.0f;

	XMFLOAT3					m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3     				m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float           			m_fMaxVelocityXZ = 0.0f;
	float           			m_fMaxVelocityY = 0.0f;
	float           			m_fFriction = 0.0f;

	LPVOID						m_pPlayerUpdatedContext = NULL;
	LPVOID						m_pCameraUpdatedContext = NULL;

	CPlayer*					m_pParter = NULL;
	CCamera						*m_pCamera = NULL;
	CUI							*m_pHPUI = NULL;
	bool						m_bInWater = false;
	bool						m_bCheatmode = false;

	int							m_PiggybackState = PIGGYBACK_NONE;
	int							m_moveState = STATE_GROUND;
	float						m_fTime = 0.f;
	float						m_fPreHeight = 0;
	CWater						**m_ppWaters;
	int							m_nWater;
	int							m_playerKind;
	float						m_ObjectHeight;
	int							m_iJumpnum = 0 ;
	float						m_fSpeed = 3.25f;



	float							m_iHP = 100.f;
	XMFLOAT3					m_predictedPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	
	int						m_iAnimationSet = 0;
public:
	CPlayer();
	virtual ~CPlayer();

	XMFLOAT3 GetPrecdictedPos() { return m_predictedPos; }
	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }
	
	int GetMoveState() { return m_moveState; }
	void SetCheatMode();
	
	float	GetHp() { return m_iHP; }
	void Damage(int d) { m_iHP -= d; }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(const XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(const XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetPosition(const XMFLOAT3& xmf3Position) { Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false); }
	void SetParter(CPlayer* pPlayer) { m_pParter = pPlayer; }
	void SetScale(XMFLOAT3& xmf3Scale) { m_xmf3Scale = xmf3Scale; }
	void SetPiggyBackState(int state) { m_PiggybackState = state; }

	void GivePiggyBack();
	void CollideSide();

	const XMFLOAT3& GetVelocity() const { return(m_xmf3Velocity); }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	CCamera *GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }

	void SetWaters(CWater** waters) { m_ppWaters = waters; }
	void SetnWaters(int n) { m_nWater = n; }

	void SetState(int state) { m_moveState = state; }
	bool CheckInWater(XMFLOAT3 pos, CHeightMapTerrain *pTerrain);

	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	void Rotate(float x, float y, float z);

	void OnObject(float fy);
	void Update(float fTimeElapsed);

	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	CCamera *OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual	void Jump();
	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) { return(NULL); }
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);


	int							m_CollideState = COLLIDEN;

	void SetPosition_async(const XMFLOAT3& xmf3Position) { 
		m_xmf3Position = xmf3Position;
	}

	int GetAnimationSet() { return m_iAnimationSet; }
	//
	bool						m_bDamaging = false;
	float						m_fDamagingTime = 0.f;

};

class CAirplanePlayer : public CPlayer
{
public:
	CAirplanePlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext=NULL);
	virtual ~CAirplanePlayer();

	CGameObject					*m_pMainRotorFrame = NULL;
	CGameObject					*m_pTailRotorFrame = NULL;

private:
	virtual void OnPrepareAnimate();
	virtual void Animate(float fTimeElapsed);

public:
	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	virtual void OnPrepareRender();
};

class CTerrainPlayer : public CPlayer
{
public:
	CTerrainPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, char* name, int kind, bool banimation, void* pContext=NULL);
	virtual ~CTerrainPlayer();

public:
	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);

	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	virtual void OnCameraUpdateCallback(float fTimeElapsed);
};

