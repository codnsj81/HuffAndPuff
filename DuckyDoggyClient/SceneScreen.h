#pragma once
#include "Object.h"
#include "Camera.h"

class CShader;
class CSceneScreen : public CGameObject 
{
public:
	CSceneScreen() {}
	CSceneScreen(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float nWidth, float nLength, XMFLOAT4X4 cameramat, wchar_t* pFilename);
	~CSceneScreen();
public:
	void SetTexture(CTexture* tex);
	void SetWinpos(float x, float y);
	void Update(float elapsed);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void SetCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle) { m_d3dCbvGPUDescriptorHandle = d3dCbvGPUDescriptorHandle; }
	void SetCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { m_d3dCbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }
private:
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorHandle;
private:
	float							m_nWidth;
	float							m_nLength;
	float							m_fWinposx;
	float							m_fWinposy;
	float							m_fWinposz;
	bool							bRender = true;
};