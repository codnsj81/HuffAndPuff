#pragma once

#include "Object.h"
#include "Camera.h"

class CShader;
class CGameFramework;
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
	float							m_fWinposz = 20.f;
	bool bRender = true;
	bool bEx = false;

	void SetWinpos(float x, float y);
	virtual void Update(float elapsed) {}
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
	virtual void Update(float elapsed);
	void SetHpScale();

public:
	int								m_iHP = 100;

	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorHandle;

};
class CMP : public CHP
{
public :
	CMP(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, float nWidth, float nLength, XMFLOAT3 xmfPosition);
	~CMP();
	virtual void Update(float elapsed);
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
	CEndUI(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, float nWidth, float nLength, XMFLOAT3 xmfPosition, wchar_t* pFilename, CGameFramework* framework);
	~CEndUI();

	virtual void Update(float elapsed);
private:
	CGameFramework* m_pFramework;
	XMFLOAT3 EndPoint;
};

class CImageUI : public CUI
{
public:
	CImageUI() {}
	CImageUI(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, float nWidth, float nLength, XMFLOAT3 xmfPosition, wchar_t* pFilename);
	~CImageUI();

	virtual void Update(float elapsed);
};

class CDamageUI : public CStartUI
{
public:
	CDamageUI() {}
	CDamageUI(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, float nWidth, float nLength, int num, wchar_t* pFilename);
	~CDamageUI();

	virtual void Update(float elapsed);
};


class CProgressUI : public CImageUI
{
private:
	int m_iProgress = 0;
	float ProgressWidth = 16.5f;
	float OriginPosx;
	
public:
	CProgressUI(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, float nWidth, float nLength, float winPosx, float winPosy, wchar_t* pFilename);
	~CProgressUI() {} 
	void Progressing();
	void SetProgressWidth(float width) { ProgressWidth = width; }
};

class CClockUI : public CImageUI
{
public :
	CClockUI() {}
	CClockUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float nWidth, float nLength);
	~CClockUI();

	void SetNum(int n) { m_iNum = n; }
	virtual void Update(float elapsed);
	
	

private:
	CGameObject* m_members;
	int m_iNum = 0;
};


class CBackgroundUI : public CUI
{
public:
	CBackgroundUI() {}
	CBackgroundUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float nWidth, float nLength, XMFLOAT4X4 cameramat, wchar_t* pFilename);
	~CBackgroundUI();

public:
	void MoveToCamera(CCamera* cameramat);
	virtual void Update(float elapsed);
};

class CEffectUI : public CDamageUI
{
public:
	CEffectUI() {}
	CEffectUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float nWidth, float nLength);
	~CEffectUI() {}

	virtual void Update(float elapsed);

protected:
	float m_fTime = 1;
};

enum cloudstate {CLOUD_BIGGER, CLOUD_BLINDING, CLOUD_SMALLER, CLOUD_NONE};
class CCloud : public CStartUI
{
public:
	CCloud() {}
	CCloud(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float nWidth, float nLength, XMFLOAT3 xmfPosition, wchar_t* pFilename);
	~CCloud() {}
	void CloudSwitch();
	virtual void Update(float elapsed);
private:
	cloudstate m_cloudstate = CLOUD_NONE;
	float m_fTime = 0.f;
	float m_fSize = 0.f;
};



class CExplosion : public CDamageUI
{
public:
	CExplosion() {}
	CExplosion(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, float nWidth, float nLength);
	~CExplosion() {}

	virtual void Update(float elapsed);

private:
	float m_fTime = 0;
	int m_iNum = 1;
};
