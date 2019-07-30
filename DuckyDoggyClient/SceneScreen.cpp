#include "stdafx.h"
#include "SceneScreen.h"
#include "Shader.h"
#include "Scene.h"

CSceneScreen::CSceneScreen(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float nWidth, float nLength, XMFLOAT3 xmfPosition)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_nMaterials = 2;
	m_ppMaterials = new CMaterial * [m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++)
		m_ppMaterials[i] = NULL;

	// UIMesh를 그대로 갖다써도 될진 모르겠지만,, 문제 생기면 CScreenMesh 를 직접 만들어 보는 걸로.
	CMesh* pMesh = new CUIMesh(pd3dDevice, pd3dCommandList, nWidth, nLength, 1, 1);
	SetMesh(pMesh);

	CUIShader* pShader = new CUIShader();
	pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CMaterial* pMaterial = new CMaterial(1);
	pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	pMaterial->SetShader(pShader);

	SetMaterial(0, pMaterial);
	SetPosition(xmfPosition);
}

CSceneScreen::~CSceneScreen()
{
}

void CSceneScreen::SetTexture(CTexture* tex)
{
	m_ppMaterials[0]->SetTexture(tex, 0);
}

void CSceneScreen::SetWinpos(float x, float y)
{
	m_fWinposx = x;
	m_fWinposy = y;
}

void CSceneScreen::Update(float elapsed)
{
	// 다음 씬으로 넘어가기.
	if(0/*조건*/)
		bRender = false;
}
void CSceneScreen::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	//XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
	//SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);

	CGameObject::Render(pd3dCommandList, pCamera);
}