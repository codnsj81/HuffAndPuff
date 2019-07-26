#pragma once

#include "Object.h"

class CShader;
class CScreen : public CGameObject
{
public:
	CScreen() {}
	~CScreen();
public:
	float							m_nWidth;
	float							m_nLength;
	float							m_fPosX;
	float							m_fPosY;
	bool							m_bAdmitRender;
	void SetPos(float x, float y) {
		m_fPosX = x; m_fPosY = y;
	}
};

