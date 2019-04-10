#include "stdafx.h"
#include "CUI.h"
#include "Shader.h"
#include "Scene.h"

CUI::CUI(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12RootSignature * pd3dGraphicsRootSignature, int nWidth, int nLength, XMFLOAT3 xmfPosition)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_nMaterials = 2;
	m_ppMaterials = new CMaterial*[m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++)
		m_ppMaterials[i] = NULL;

	CMesh *pMesh = new CUIMesh(pd3dDevice, pd3dCommandList, nWidth, nLength);
	SetMesh(pMesh);

	CTexture *pWaterTex = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pWaterTex->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Model/Textures/waternormal.tiff", 0, false);

	CUIShader *pShader = new CUIShader();
	pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//SetShader(pShader);

	CMaterial *pMaterial = new CMaterial(1);
	pMaterial->m_xmf4AlbedoColor = XMFLOAT4(1, 1, 1, 1);
	pMaterial->SetTexture(pWaterTex, 0);
	pMaterial->SetMaterialType(MATERIAL_NORMAL_MAP);
	pMaterial->SetShader(pShader);

	SetMaterial(0, pMaterial);
	CScene::CreateShaderResourceViews(pd3dDevice, pWaterTex, 8, false);

	pWaterTex = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	pWaterTex->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Model/Textures/blue.tiff", 0, false);

	pMaterial = new CMaterial(1);
	pMaterial->m_xmf4AlbedoColor = XMFLOAT4(1, 1, 1, 1);
	pMaterial->SetTexture(pWaterTex, 0);
	pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	pMaterial->SetShader(pShader);


	SetMaterial(1, pMaterial);
	CScene::CreateShaderResourceViews(pd3dDevice, pWaterTex, 9, false);


	SetPosition(xmfPosition);
}

CUI::~CUI()
{
}

void CUI::FollowCamera()
{
}
