#pragma once

#include "Object.h"
#include "Camera.h"

class CShader;
class CScreen : public CGameObject
{
public:
	CScreen() {}
	~CScreen();
public:
	virtual void Update(float elapsed);
	void SetPos(float x, float y) {
		m_fPosX = x; m_fPosY = y;
	}
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorHandle;

private:
	float							m_nWidth;
	float							m_nLength;
	float							m_fPosX;
	float							m_fPosY;
	bool							m_bAdmitRender;
};

