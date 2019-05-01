#pragma once

#include "Object.h"
#include "Camera.h"

class CShader;
class CUI : public CGameObject
{
public:
	CUI() {}
	CUI(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, float nWidth, float nLength, XMFLOAT3 xmfPosition);
	~CUI();
	void SetCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle) { m_d3dCbvGPUDescriptorHandle = d3dCbvGPUDescriptorHandle; }
	void SetCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { m_d3dCbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }

	void FollowCamera();

public:
	float							m_nWidth;
	float							m_nLength;
	int								m_iHP = 100;
	CPlayer*						m_pPlayer = NULL;
	CCamera*						m_pCamera;

	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorHandle;

};
