#pragma once

#include "Object.h"
#include "Camera.h"

class CShader;

class CUI : public CGameObject
{
public:
	CUI() {}
	~CUI();
public:

	CPlayer*						m_pPlayer = NULL;
	float							m_nWidth;
	float							m_nLength;
	float							m_fWinposx = 10.f;
	float							m_fWinposy = 9.f;
	bool bRender = true;
	bool bEx = false;

	void SetWinpos(float x, float y);
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorHandle;
};

class CHP : public CUI
{
public:
	CHP() {}
	CHP(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, float nWidth, float nLength, XMFLOAT3 xmfPosition);
	~CHP();
	void SetCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle) { m_d3dCbvGPUDescriptorHandle = d3dCbvGPUDescriptorHandle; }
	void SetCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { m_d3dCbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }
	virtual void Update();
	void FollowCamera();
	void SetHpScale();

public:
	int								m_iHP = 100;

	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorHandle;

};

class CStartUI : public CUI
{
public :
	CStartUI() {}
	CStartUI(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, float nWidth, float nLength, XMFLOAT3 xmfPosition, wchar_t* pFilename);
	~CStartUI();
	virtual void Update(float elapsed);
	bool Trigger = false;
	float TimeElapsed = 0.f;
};

class CEndUI : public CStartUI
{
public:

	CEndUI(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, float nWidth, float nLength, XMFLOAT3 xmfPosition, wchar_t* pFilename);
	~CEndUI();

	virtual void Update(float elapsed);
private:
	XMFLOAT3 EndPoint;
};