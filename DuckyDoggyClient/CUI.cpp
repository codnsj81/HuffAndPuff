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

	CMesh *pMesh = new CUIMesh(pd3dDevice, pd3dCommandList, nWidth, nLength,1,1);
	SetMesh(pMesh);

	CUIShader *pShader = new CUIShader();
	pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//SetShader(pShader);

	CMaterial *pMaterial = new CMaterial(1);
	pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	pMaterial->SetShader(pShader);

	SetMaterial(0, pMaterial);
	SetPosition(xmfPosition);
}

CHP::~CHP()
{
}

void CHP::SetHpScale()
{
	float ratio = (m_pPlayer->GetHp() / 100.f);
	SetScale(ratio, 1, 1);
}

void CHP::Update(float elapsed)
{
	float ratio = (m_pPlayer->GetHp() / 100.f);
	SetScale(ratio, 1, 1);
}

CUI::~CUI()
{
}

void CUI::SetTexture(CTexture * tex)
{
	m_ppMaterials[0]->SetTexture(tex, 0);
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

	CMesh *pMesh = new CUIMesh(pd3dDevice, pd3dCommandList, nWidth, nLength,1,1);
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
			bEx = true;
		}
	}
}

CEndUI::CEndUI(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12RootSignature * pd3dGraphicsRootSignature, float nWidth, float nLength, XMFLOAT3 xmfPosition, wchar_t * pFilename)
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

	EndPoint = xmfPosition;
	//pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	
}

CEndUI::~CEndUI()
{
}

void CEndUI::Update(float elapsed)
{
	if (!Trigger && !bEx)
	{
		XMFLOAT3 playerpos = m_pPlayer->GetPosition();
		float distance = Vector3::Length(Vector3::Subtract(playerpos, EndPoint));
		if (distance < 20)
		{
			bRender = true;
			Trigger = true;
		}

	}
	else
	CStartUI::Update(elapsed);
}

CImageUI::CImageUI(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12RootSignature * pd3dGraphicsRootSignature, float nWidth, float nLength, XMFLOAT3 xmfPosition, wchar_t * pFilename)
{
	bRender = false;
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_nMaterials = 1;
	m_ppMaterials = new CMaterial*[m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++)
		m_ppMaterials[i] = NULL;

	CMesh *pMesh = new CUIMesh(pd3dDevice, pd3dCommandList, nWidth, nLength, 1, 1);
	SetMesh(pMesh);

	CShader *pShader = new CUIShader();
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
	bRender = true;
}

CImageUI::~CImageUI()
{
}

void CImageUI::Update(float elapsed)
{
}

CMP::CMP(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12RootSignature * pd3dGraphicsRootSignature, float nWidth, float nLength, XMFLOAT3 xmfPosition)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_nMaterials = 2;
	m_ppMaterials = new CMaterial*[m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++)
		m_ppMaterials[i] = NULL;

	CMesh *pMesh = new CUIMesh(pd3dDevice, pd3dCommandList, nWidth, nLength, 1, 1);
	SetMesh(pMesh);

	CUIShader *pShader = new CUIShader();
	pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//SetShader(pShader);

	CMaterial *pMaterial = new CMaterial(1);
	pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	pMaterial->SetShader(pShader);

	SetMaterial(0, pMaterial);
	bRender = true;

	SetPosition(xmfPosition);
}

CMP::~CMP()
{
}

void CMP::Update(float elapsed)
{
	float ratio = (m_pPlayer->GetSkillGage() / 100.f);
	SetScale(ratio, 1, 1);
}

CDamageUI::CDamageUI(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12RootSignature * pd3dGraphicsRootSignature, float nWidth, float nLength, int num, wchar_t * pFilename)
{
	bRender = true;
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_nMaterials = 1;
	m_ppMaterials = new CMaterial*[m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++)
		m_ppMaterials[i] = NULL;

	CMesh *pMesh = new CDamageUIMesh(pd3dDevice, pd3dCommandList, num, nWidth, nLength);
	SetMesh(pMesh);

	CShader *pShader = new CUIShader();
	pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);


	CMaterial *pMaterial = new CMaterial(1);
	pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	pMaterial->SetShader(pShader);

	SetMaterial(0, pMaterial);

	//pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
}

CDamageUI::~CDamageUI()
{
}

void CDamageUI::SetTexture(CTexture * tex)
{
	m_ppMaterials[0]->SetTexture(tex,0);
}

void CDamageUI::Update(float elapsed)
{
	SetPosition(Vector3::Add(GetPosition(), Vector3::ScalarProduct(GetLook(),-elapsed)));
	TimeElapsed += elapsed;
	if (TimeElapsed > 1.f) bRender = false;

}

CProgressUI::CProgressUI(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList, ID3D12RootSignature * pd3dGraphicsRootSignature, float nWidth, float nLength, float winPosx, float winPosy, wchar_t * pFilename)
{
	bRender = false;
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_nMaterials = 1;
	m_ppMaterials = new CMaterial*[m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++)
		m_ppMaterials[i] = NULL;

	CMesh *pMesh = new CUIMesh(pd3dDevice, pd3dCommandList, nWidth, nLength, 1, 1);
	SetMesh(pMesh);

	CShader *pShader = new CUIShader();
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
	SetWinpos(winPosx, winPosy);
	m_fWinposz = 10.f;
	//pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	bRender = true;
}

void CProgressUI::Progressing()
{
	m_fWinposx += ProgressWidth;
}

CClockUI::CClockUI(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float nWidth, float nLength)
{
	bRender = false;
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_nMaterials = 1;
	m_ppMaterials = new CMaterial * [m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++)
		m_ppMaterials[i] = NULL;

	CMesh* pMesh = new CFontMesh(pd3dDevice, pd3dCommandList, nWidth, nLength);
	SetMesh(pMesh);

	CShader* pShader = new CUIShader();
	pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CMaterial* pMaterial = new CMaterial(1);
	pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	pMaterial->SetShader(pShader);

	SetMaterial(0, pMaterial);

	//pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
	bRender = true;
}

CClockUI::~CClockUI()
{
}

void CClockUI::Update(float elapsed)
{
	dynamic_cast<CFontMesh*> (m_pMesh)->SetNumber(m_iNum);
	
}
