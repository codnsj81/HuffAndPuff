#include "stdafx.h"
#include "CUI.h"
#include "Shader.h"
#include "Scene.h"

CUI::CUI(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12RootSignature * pd3dGraphicsRootSignature, float nWidth, float nLength, XMFLOAT3 xmfPosition)
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

CUI::~CUI()
{
}

void CUI::FollowCamera()
{
}
