#include "stdafx.h"
#include "SceneScreen.h"
#include "Shader.h"
#include "Scene.h"

CSceneScreen::CSceneScreen(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float nWidth, float nLength, XMFLOAT4X4 cameramat, wchar_t* pFilename)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_nMaterials = 2;
	m_ppMaterials = new CMaterial * [m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++)
		m_ppMaterials[i] = NULL;

	// UIMesh를 그대로 갖다써도 될진 모르겠지만,, 문제 생기면 CScreenMesh 를 직접 만들어 보는 걸로.
	CMesh* pMesh = new CScreenMesh(pd3dDevice, pd3dCommandList, nWidth, nLength, 1, 1);
	SetMesh(pMesh);

	CUIShader* pShader = new CUIShader();
	pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* Texture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	Texture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pFilename, 0, false);
	CScene::CreateShaderResourceViews(pd3dDevice, Texture, 3, false);

	CMaterial* pMaterial = new CMaterial(1);
	pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	pMaterial->SetTexture(Texture);
	pMaterial->SetShader(pShader);
	SetMaterial(0, pMaterial);
	m_xmf4x4ToParent = cameramat;
	MoveForward(230);
	MoveUp(-15);
	Rotate(90, 0, 0);
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

	CGameObject::Render(pd3dCommandList, pCamera);
}