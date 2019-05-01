#include "stdafx.h"
#include "CUI.h"
#include "Shader.h"
#include "Scene.h"

CHP::CHP(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12RootSignature * pd3dGraphicsRootSignature, float nWidth, float nLength, XMFLOAT3 xmfPosition)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_nMaterials = 2;
	m_ppMaterials = new CMaterial*[m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++)
		m_ppMaterials[i] = NULL;

	CMesh *pMesh = new CUIMesh(pd3dDevice, pd3dCommandList, nWidth, nLength);
	SetMesh(pMesh);

	CUIShader *pShader = new CUIShader();
	pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//SetShader(pShader);

	CMaterial *pMaterial = new CMaterial(1);
	pMaterial->m_xmf4AlbedoColor = XMFLOAT4(1, 0, 0, 1);
	//pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	pMaterial->SetShader(pShader);

	SetMaterial(0, pMaterial);
	SetPosition(xmfPosition);
}

CHP::~CHP()
{
}

void CHP::Update()
{
	SetScale(m_iHP / 100.f, 1, 1);
}

void CHP::FollowCamera()
{
}

CUI::~CUI()
{
}

void CUI::SetWinpos(float x, float y)
{
	m_fWinposx = x;
	m_fWinposy = y;
}

CStartUI::CStartUI(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12RootSignature * pd3dGraphicsRootSignature, float nWidth, float nLength, XMFLOAT3 xmfPosition, wchar_t* pFilename)
{
	bRender = false;
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_nMaterials = 1;
	m_ppMaterials = new CMaterial*[m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++)
		m_ppMaterials[i] = NULL;

	CMesh *pMesh = new CUIMesh(pd3dDevice, pd3dCommandList, nWidth, nLength);
	SetMesh(pMesh);

	CShader *pShader = new CShader();
	pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	
	CTexture *Texture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	Texture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pFilename, 0, false);

	CScene::CreateShaderResourceViews(pd3dDevice, Texture, 3, false);

	CMaterial *pMaterial = new CMaterial(1);
	pMaterial->SetTexture(Texture, 0);
	pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	pMaterial->SetShader(pShader);

	SetMaterial(0, pMaterial);

	//pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	SetPosition(xmfPosition);

}

CStartUI::~CStartUI()
{
}

void CStartUI::Update(float elapsed)
{
	if (Trigger)
	{
		TimeElapsed += elapsed;
		if (TimeElapsed > 2.f)
		{
			Trigger = false;
			bRender = false;
		}
	}
}
