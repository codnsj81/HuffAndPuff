//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include <fstream>
#include <cstdlib>
#include "Scene.h"
#include "CMonster.h"
#include "CUI.h"
#include "GameFramework.h"
#include "SoundMgr.h"

ID3D12DescriptorHeap *CScene::m_pd3dCbvSrvDescriptorHeap = NULL;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorStartHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorStartHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorStartHandle;

D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dCbvGPUDescriptorNextHandle;
D3D12_CPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvCPUDescriptorNextHandle;
D3D12_GPU_DESCRIPTOR_HANDLE	CScene::m_d3dSrvGPUDescriptorNextHandle;

CScene::CScene()
{
}

CScene::~CScene()
{
}

void CScene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 1;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);

	m_pLights[0].m_bEnable = true;
	m_pLights[0].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.4f, 0.0f);
	m_pLights[0].m_xmf3Direction = XMFLOAT3(0.5f, 0.5f, 0.0f);
}

void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
		m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
		CreateCbvSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, 60); //SuperCobra(17), Gunship(2), Player:Mi24(1), Angrybot()
		CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
		BuildDefaultLightsAndMaterials();

		XMFLOAT3 xmf3Scale(5.0f, 3.0f, 5.0f); // -> 나중에 크기 6,3,6으로 바꾸기
		XMFLOAT4 xmf4Color(0.3f, 0.3f, 0.3f, 0.0f);
		
		m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, _T("Terrain/terrain.raw"), 257, 257, xmf3Scale, xmf4Color);
		m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);


		m_nWaters = 2;
		m_ppWaters = new CWater * [m_nWaters];
		m_ppWaters[0] = new CWater(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 400, 200, XMFLOAT3(955, 30.f, 515));
		//m_ppWaters[0]->Rotate(0, 10.f, 0);

		m_ppWaters[1] = new CWater(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 500, 350, XMFLOAT3(714, 30.f, 803.f));
		Potion = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/potion.bin", NULL, false);

		HoneyComb = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Honey.bin", NULL, false);
		BuildMonsterList(pd3dDevice, pd3dCommandList);

		m_nGameObjects = 0;
		m_ppGameObjects = new CGameObject * [m_nGameObjects];


		m_DamageUITex = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_DamageUITex->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Model/Textures/number.tiff", 0, false);
		CScene::CreateShaderResourceViews(pd3dDevice, m_DamageUITex, 3, false);


		m_HitAttackEffectTex = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_HitAttackEffectTex->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Model/Textures/effect13.tiff", 0, false);
		CScene::CreateShaderResourceViews(pd3dDevice, m_HitAttackEffectTex, 3, false);


		m_DamageUITexYellow = new CTexture(1, RESOURCE_TEXTURE2D, 0);
		m_DamageUITexYellow->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Model/Textures/number3.tiff", 0, false);
		CScene::CreateShaderResourceViews(pd3dDevice, m_DamageUITexYellow, 3, false);

		LoadStone(pd3dDevice, pd3dCommandList);
		LoadTree(pd3dDevice, pd3dCommandList);
		LoadGrass(pd3dDevice, pd3dCommandList);
		LoadTrap(pd3dDevice, pd3dCommandList);
		LoadDash(pd3dDevice, pd3dCommandList);
		BuildMushroomData(pd3dDevice, pd3dCommandList);
		CreateShaderVariables(pd3dDevice, pd3dCommandList);

}

void CScene::BuildClock(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{

	CTexture* m_ClockTex = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_ClockTex->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Model/Textures/number2.tiff", 0, false);
	CScene::CreateShaderResourceViews(pd3dDevice, m_ClockTex, 3, false);
	 
	m_iClockMin = new CClockUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 1,1);
	m_iClockMin->SetWinpos(-1.7, 10);
	m_iClockMin->SetTexture(m_ClockTex);
	
	m_UIList->emplace_back(m_iClockMin);

	m_iClockSec1 = new CClockUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 1, 1);
	m_iClockSec1->SetWinpos(0.7, 10);
	m_iClockSec1->SetTexture(m_ClockTex);
	m_UIList->emplace_back(m_iClockSec1);

	m_iClockSec2 = new CClockUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 1, 1);
	m_iClockSec2->SetWinpos(2.2, 10);
	m_iClockSec2->SetTexture(m_ClockTex);
	m_UIList ->emplace_back(m_iClockSec2);

	CUI* pUI = new CClockUI(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 1, 1);
	dynamic_cast<CClockUI*> (pUI)->SetNum(10);
	pUI->SetWinpos(0, 10);
	pUI->SetTexture(m_ClockTex);
	m_UIList->emplace_back(pUI);
}

void CScene::ReleaseObjects()
{

		if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();
		if (m_pd3dCbvSrvDescriptorHeap) m_pd3dCbvSrvDescriptorHeap->Release();

		if (m_pTerrain) delete m_pTerrain;
		if (m_pSkyBox) delete m_pSkyBox;
		if (Potion)
		{
			Potion->Release();
		}
		if (m_ppGameObjects)
		{
			for (int i = 0; i < m_nGameObjects; i++) if (m_ppGameObjects[i]) m_ppGameObjects[i]->Release();
			delete[] m_ppGameObjects;
		}
		if (m_TreeObjectslist.size() != 0)
		{
			for (auto n : m_TreeObjectslist) {
				n->Release();
			}
		}
		if (m_StoneObjectslist.size() != 0)
		{
			for (auto n : m_StoneObjectslist) {
				n->Release();
			}
		}
		if (m_HoneyComblist.size() != 0)
		{
			for (auto n : m_HoneyComblist) {
				n->Release();
			}
		}

		if (m_Mushroomlist.size() != 0)
		{
			for (auto n : m_Mushroomlist) {
				n->Release();
			}
		}
		if (m_ppWaters)
		{
			for (int i = 0; i < m_nWaters; i++) if (m_ppWaters[i]) m_ppWaters[i]->Release();
			delete[] m_ppWaters;
		}
	
		if (m_pSnakeObject) m_pSnakeObject->Release();
		if (HoneyComb) HoneyComb->Release();
		if (m_DamageUITex) m_DamageUITex->Release();
		if (m_DamageUITexYellow) m_DamageUITexYellow->Release();

		ReleaseShaderVariables();

		if (m_pLights) delete[] m_pLights;
}

void CScene::Update(float fTime)
{

		ObjectsCollides();
		for (auto a : *m_UIList)
		{
			(a)->Update(fTime);
		}

		for (int i = 0; i < 2; i++)
		{
			if (m_ppWaters[i])
			{
				m_ppWaters[i]->Animate(fTime);

			}
		}

		if (bCreatePDUI)
		{
			CreateDamageDEF(monDUIPos);
			CreateDamageUIP(monDUIPos);
			bCreatePDUI = false;
		}
		TimeCount(fTime);
}

void CScene::PlayerAttack()
{
	m_pPlayer->Attack();
	if (m_pSnakeObject)
	{
		XMFLOAT3 pos1 = m_pSnakeObject->GetPosition();
		XMFLOAT3 pos2 = m_pPlayer->GetPosition();
		float distance = Vector3::Length(Vector3::Subtract(pos1, pos2));
		if (distance < 10)
		{
			m_pSnakeObject->Damage(m_pPlayer->GetAtt());
			bCreatePDUI = true; 
			monDUIPos = pos1;
		}
	}
}

ID3D12RootSignature *CScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[10];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 6; //t6: gtxtAlbedoTexture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 7; //t7: gtxtSpecularTexture
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 8; //t8: gtxtNormalTexture
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 9; //t9: gtxtMetallicTexture
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 10; //t10: gtxtEmissionTexture
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 11; //t11: gtxtEmissionTexture
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 12; //t12: gtxtEmissionTexture
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[7].NumDescriptors = 1;
	pd3dDescriptorRanges[7].BaseShaderRegister = 13; //t13: gtxtSkyBoxTexture
	pd3dDescriptorRanges[7].RegisterSpace = 0;
	pd3dDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[8].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[8].NumDescriptors = 1;
	pd3dDescriptorRanges[8].BaseShaderRegister = 1; //t1: gtxtTerrainBaseTexture
	pd3dDescriptorRanges[8].RegisterSpace = 0;
	pd3dDescriptorRanges[8].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[9].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[9].NumDescriptors = 1;
	pd3dDescriptorRanges[9].BaseShaderRegister = 2; //t2: gtxtTerrainDetailTexture
	pd3dDescriptorRanges[9].RegisterSpace = 0;
	pd3dDescriptorRanges[9].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[15];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 33;
	pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Lights
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[10].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[7]);
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[11].Descriptor.ShaderRegister = 7; //Skinned Bone Offsets
	pd3dRootParameters[11].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[12].Descriptor.ShaderRegister = 8; //Skinned Bone Transforms
	pd3dRootParameters[12].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[8]);
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[14].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[14].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[9]);
	pd3dRootParameters[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[2];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void **)&m_pcbMappedLights);
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));
}

void CScene::ReleaseShaderVariables()
{
	if (m_pd3dcbLights)
	{
		m_pd3dcbLights->Unmap(0, NULL);
		m_pd3dcbLights->Release();
	}
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();
	if (m_pSkyBox) m_pSkyBox->ReleaseUploadBuffers();

	for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nWaters; i++) m_ppWaters[i]->ReleaseUploadBuffers();

	for (auto n : m_TreeObjectslist)
	{
		n->ReleaseUploadBuffers();
	}
	for (auto n : m_StoneObjectslist)
	{
		n->ReleaseUploadBuffers();
	}
	if(m_pSnakeObject)
		m_pSnakeObject->ReleaseShaderVariables();

	for (auto n : m_TrapList)
		n->ReleaseShaderVariables();

	for (auto n : *m_UIList)
		n->ReleaseShaderVariables();
}

void CScene::TimeCount(float time)
{
	if (!m_iClockMin && !m_iClockSec1 && !m_iClockSec2) return;
	m_fStageTime += time;
	if (m_fStageTime > 1)
	{
		m_iStageTime++;

		int Min = int( m_iStageTime / 60);
		int Sec = m_iStageTime % 60;
		int Sec1 = Sec / 10;
		int Sec2 = Sec % 10;
		dynamic_cast<CClockUI*>(m_iClockMin)->SetNum(Min);
		dynamic_cast<CClockUI*>(m_iClockSec1)->SetNum(Sec1);
		dynamic_cast<CClockUI*>(m_iClockSec2)->SetNum(Sec2);
		m_fStageTime = 0;
	}
}

void CScene::SetBloodScreenState(bool b)
{
	m_BloodScreen->bRender = b;
}

void CScene::ResetObjects()
{
}

void CScene::PlusTreeData()
{
	XMFLOAT2 playerXZpos;
	playerXZpos.x = m_pPlayer->GetPosition().x;
	playerXZpos.y = m_pPlayer->GetPosition().z;
	TreeDatalist.push_back(playerXZpos);
}

void CScene::SaveTreeData()
{
	fstream out("TreeData.txt", ios::out | ios::binary);

	for (auto n : TreeDatalist)
	{
		out << n.x << " " << n.y << "\n";
	}
}

void CScene::PlusTrapData()
{

	XMFLOAT2 playerXZpos;
	playerXZpos.x = m_pPlayer->GetPosition().x;
	playerXZpos.y = m_pPlayer->GetPosition().z;
	TrapDatalist.push_back(playerXZpos);
}

void CScene::SaveTrapData()
{
	fstream out("TrapData.txt", ios::out | ios::binary);

	for (auto n : TrapDatalist)
	{
		out << n.x << " " << n.y << "\n";
	}
}

void CScene::PlusGrassData()
{
	XMFLOAT2 playerXZpos;
	playerXZpos.x = m_pPlayer->GetPosition().x;
	playerXZpos.y = m_pPlayer->GetPosition().z;
	GrassDataList.push_back(playerXZpos);
}

void CScene::SaveGrassData()
{
	fstream out("GrassData.txt", ios::out | ios::binary);

	for (auto n : GrassDataList)
	{
		out << n.x << " " << n.y << "\n";
	}
}

void CScene::LoadDash(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	CGameObject* pOBJ = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Dash.bin", NULL, false);

	fstream in("DashData.txt", ios::in | ios::binary);
	while (in)
	{
		DashInfo dat;
		in >> dat.m_pos.x;
		in >> dat.m_pos.y;
		in >> dat.m_pos.z;
		in >> dat.m_rot;

		DashDataList.emplace_back(dat);
	}

	list<DashInfo>::iterator iter = DashDataList.begin();
	list<DashInfo>::iterator end = DashDataList.end();

	for (iter; iter != end; iter++)
	{
		CDash* obj = new CDash();
		obj->SetChild(pOBJ, true);
		obj->SetHitBox(XMFLOAT3(4,4,4));
		obj->SetPosition(iter->m_pos.x, iter->m_pos.y, iter->m_pos.z);
		//Vector3::Normalize(iter->m_rot);
		obj->Rotate(0, iter->m_rot, 0);
		m_DashList.push_back(obj);
	}
}

void CScene::PlusMushroomData()
{
	MushroomDatalist.push_back(XMFLOAT2(m_pPlayer->GetPosition().x, m_pPlayer->GetPosition().z));
}

void CScene::SaveMushroomData()
{
	fstream out("MushroomData.txt", ios::out | ios::binary);
	for (auto n : MushroomDatalist)
	{
		out << n.x <<" " << n.y<< "\n";
	}
}

void CScene::BuildMushroomData(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	CGameObject* Mushroom = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/mushroom01.bin", NULL, false);
	CGameObject* Mushroom2 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/mushroom02.bin", NULL, false);

	fstream in("MushroomData.txt", ios::in | ios::binary);
	while (in)
	{
		XMFLOAT2 dat;
		in >> dat.x;
		in >> dat.y;
		MushroomDatalist.emplace_back(dat);
	}

	list<XMFLOAT2>::iterator iter = MushroomDatalist.begin();
	list<XMFLOAT2>::iterator end = MushroomDatalist.end();

	for (iter; iter != end; iter++)
	{
		float RandomRotate = rand() % 360;
		CMushroom* obj = new CMushroom();
		int treerand = rand() % 2;
		if (treerand == 0)
			obj->SetChild(Mushroom, true);
		else if (treerand == 1)
			obj->SetChild(Mushroom2, true);


		obj->SetHitBox(XMFLOAT3(5,5,5));
		obj->SetPosition(iter->x, m_pTerrain->GetHeight(iter->x, iter->y), iter->y);
	//	obj->Rotate(0, RandomRotate, 0);
		m_Mushroomlist.push_back(obj);
	}
}

void CScene::PlusStoneData()
{
	
	StoneInfo playerPos;
	playerPos.m_pos.x = m_pPlayer->GetPosition().x;
	playerPos.m_pos.y = m_pPlayer->GetPosition().y ;
	playerPos.m_pos.z = m_pPlayer->GetPosition().z;
	playerPos.m_size = XMFLOAT3(3.f, 3.f, 3.f);
	playerPos.m_iType = 0;

	StoneDataList.push_back(playerPos);

}

void CScene::SaveStoneData()
{
	fstream out("StoneData.txt", ios::out | ios::binary);
	for (auto n : StoneDataList)
	{
		out <<n.m_iType << " " << n.m_pos.x << " " << n.m_pos.y  <<" " << n.m_pos.z << " "
			<< n.m_size.x << " "<< n.m_size.y<<" " << n.m_size.z  << endl;
	}
}

void CScene::PlusDashData()
{
	DashInfo playerPos;

	playerPos.m_pos.x = m_pPlayer->GetPosition().x;
	playerPos.m_pos.y = m_pPlayer->GetPosition().y;
	playerPos.m_pos.z = m_pPlayer->GetPosition().z;
	XMFLOAT3 look = Vector3::Normalize( m_pPlayer->GetLookVector());
	
	playerPos.m_rot = 10;
	DashDataList.push_back(playerPos);
}

void CScene::SaveDashData()
{
	fstream out("DashData.txt", ios::out | ios::binary);
	for (auto n : DashDataList)
	{
		out << n.m_pos.x << " " << n.m_pos.y + 0.1f << " " << n.m_pos.z << " "
			<< n.m_rot<< endl;
	}
}

void CScene::PlusMonsterData()
{

	StoneInfo playerPos;
	playerPos.m_pos.x = m_pPlayer->GetPosition().x;
	playerPos.m_pos.y = m_pPlayer->GetPosition().y;
	playerPos.m_pos.z = m_pPlayer->GetPosition().z;
	playerPos.m_size = XMFLOAT3(3,3,3);
	playerPos.m_iType = MONTYPE_SNAKE;

	MonsterDataList.push_back(playerPos);
}

void CScene::SaveMonsterData()
{
	fstream out("MonsterData.txt", ios::out | ios::binary);
	for (auto n : MonsterDataList)
	{
		out << n.m_iType << " " << n.m_pos.x << " " << n.m_pos.y << " " << n.m_pos.z << " "
			<< n.m_size.x << " " << n.m_size.y << " " << n.m_size.z << endl;
	}
}

void CScene::BuildMonsterList(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	StoneInfo dat;
	int type = 1;
	fstream in("MonsterData.txt", ios::in | ios::binary);
	while (in)
	{
		in >> dat.m_iType;
		in >> dat.m_pos.x;
		in >> dat.m_pos.y;
		in >> dat.m_pos.z;


		in >> dat.m_size.x;
		in >> dat.m_size.y;
		in >> dat.m_size.z;
		MonsterDataList.emplace_back(dat);
	}

	vector<StoneInfo>::iterator iter = MonsterDataList.begin();

	CGameObject* pSnake = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/snake.bin", NULL, true);
	pSnake->SetAnimationSpeed(0.5f);

	float RandomRotate = rand() % 360;\
	m_pSnakeObject = new CSnake();
	m_pSnakeObject->SetChild(pSnake, true);
	m_pSnakeObject->SetScene(this);
	m_pSnakeObject->SetPosition(iter->m_pos.x, iter->m_pos.y, iter->m_pos.z);
	m_pSnakeObject->Rotate(0, RandomRotate, 0);
	m_pSnakeObject->SetScale(iter->m_size.x, iter->m_size.y, iter->m_size.z);
	m_pSnakeObject->SetHitBox(XMFLOAT3(4.f, 3.f, 9.f));
}

void CScene::RenderStage1(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pSkyBox) m_pSkyBox->Render(pd3dCommandList, pCamera);
	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);

	for (auto p : m_TreeObjectslist)
	{
		p->UpdateTransform(NULL);
		p->Render(pd3dCommandList, pCamera);
	}
	for (auto p : m_StoneObjectslist)
	{
		p->UpdateTransform(NULL);
		p->Render(pd3dCommandList, pCamera);
	}

	if (m_pSnakeObject) m_pSnakeObject->Render(pd3dCommandList, pCamera);

	for (auto p : m_GrassObjectlist)
	{
		p->UpdateTransform(NULL);
		p->Render(pd3dCommandList, pCamera);
	}
	for (auto p : m_HoneyComblist)
	{
		p->UpdateTransform(NULL);
		p->Render(pd3dCommandList, pCamera);
	}
	for (auto p : m_Mushroomlist)
	{
		p->UpdateTransform(NULL);
		p->Render(pd3dCommandList, pCamera);
	}
	for (auto p : m_TrapList)
	{
		p->UpdateTransform(NULL);
		p->Render(pd3dCommandList, pCamera);
	}
	for (auto p : m_DashList)
	{
		p->UpdateTransform(NULL);
		p->Render(pd3dCommandList, pCamera);
	}
	for (auto p : m_DamageUIList)
	{
		if (p->bRender)
		{
			p->UpdateTransform(NULL);
			p->Render(pd3dCommandList, pCamera);
		}
	}

	if (m_pPlayer) m_pPlayer->Render(m_pd3dCommandList, pCamera);

	for (int i = 0; i < 2; i++)
	{
		if (m_ppWaters[i])
		{
			m_ppWaters[i]->Render(m_pd3dCommandList, pCamera);

		}
	}

}

void CScene::CreateDamageUI(int dam)
{
	CDamageUI* DUI = new CDamageUI(m_pd3dDevice, m_pd3dCommandList, m_pd3dGraphicsRootSignature, 4, 4, dam, NULL);
	DUI->SetTexture(m_DamageUITex);
	m_pPlayer->GetCamera()->RotateUI(DUI);
	DUI->SetPosition(m_pPlayer->GetPosition().x, m_pPlayer->GetPosition().y + 7, m_pPlayer->GetPosition().z);
	m_DamageUIList.emplace_back(DUI);
}



void CScene::CreateDamageUIP(const XMFLOAT3 & pos)
{
	CDamageUI* DUI = new CDamageUI(m_pd3dDevice, m_pd3dCommandList, m_pd3dGraphicsRootSignature, 4, 4, 8, NULL);
	DUI->SetTexture(m_DamageUITexYellow);
	m_pPlayer->GetCamera()->RotateUI(DUI);
	DUI->SetPosition(pos.x, pos.y + 7, pos.z);
	m_DamageUIList.emplace_back(DUI);
}

void CScene::CreateDamageDEF(const XMFLOAT3& pos)
{
	CEffectUI* DUI = new CEffectUI(m_pd3dDevice, m_pd3dCommandList, m_pd3dGraphicsRootSignature, 12, 10);
	DUI->SetTexture(m_HitAttackEffectTex);
	m_pPlayer->GetCamera()->RotateUI(DUI);

	DUI->SetPosition(pos.x, pos.y, pos.z);
	m_DamageUIList.emplace_back(DUI);
}


void CScene::CreateCbvSrvDescriptorHeaps(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews + nShaderResourceViews; //CBVs + SRVs 
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dCbvSrvDescriptorHeap);

	m_d3dCbvCPUDescriptorNextHandle = m_d3dCbvCPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dCbvGPUDescriptorNextHandle = m_d3dCbvGPUDescriptorStartHandle = m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dSrvCPUDescriptorNextHandle.ptr = m_d3dSrvCPUDescriptorStartHandle.ptr = m_d3dCbvCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
	m_d3dSrvGPUDescriptorNextHandle.ptr = m_d3dSrvGPUDescriptorStartHandle.ptr = m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nConstantBufferViews);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateConstantBufferViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle = m_d3dCbvGPUDescriptorNextHandle;
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		m_d3dCbvCPUDescriptorNextHandle.ptr = m_d3dCbvCPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, m_d3dCbvCPUDescriptorNextHandle);
		m_d3dCbvGPUDescriptorNextHandle.ptr = m_d3dCbvGPUDescriptorNextHandle.ptr + ::gnCbvSrvDescriptorIncrementSize;
	}
	return(d3dCbvGPUDescriptorHandle);
}

D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	switch (nTextureType)
	{
		case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
		case RESOURCE_TEXTURE2D_ARRAY:
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
			d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
			d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
			d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			break;
		case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
			d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
			d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
			d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
			d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
			d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
			break;
		case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			d3dShaderResourceViewDesc.TextureCube.MipLevels = -1;
			d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
			d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			break;
		case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
			d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
			d3dShaderResourceViewDesc.Buffer.NumElements = 0;
			d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
			d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			break;
	}
	return(d3dShaderResourceViewDesc);
}

D3D12_GPU_DESCRIPTOR_HANDLE CScene::CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture, UINT nRootParameter, bool bAutoIncrement)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorNextHandle;
	if (pTexture)
	{
		int nTextures = pTexture->GetTextures();
		int nTextureType = pTexture->GetTextureType();
		for (int i = 0; i < nTextures; i++)
		{
			ID3D12Resource *pShaderResource = pTexture->GetTexture(i);
			D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();
			D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nTextureType);
			pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, m_d3dSrvCPUDescriptorNextHandle);
			m_d3dSrvCPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;

			pTexture->SetRootArgument(i, (bAutoIncrement) ? (nRootParameter + i) : nRootParameter, m_d3dSrvGPUDescriptorNextHandle);
			m_d3dSrvGPUDescriptorNextHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		}
	}
	return(d3dSrvGPUDescriptorHandle);
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	default:
		break;
	}
	return(false);
}

void CScene::LoadStone(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{

	CGameObject *pStone = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Rock.bin", NULL, false);
	CGameObject *pStone2 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/rock4.bin", NULL, false);
	CGameObject *pStone3 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/rock3.bin", NULL, false);
	CGameObject *pStone4 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/rock5.bin", NULL, false);

	StoneInfo dat;
	int type = 1;
	fstream in("StoneData.txt", ios::in | ios::binary);
	while (in)
	{
		in >> dat.m_iType;
		in >> dat.m_pos.x;
		in >> dat.m_pos.y;
		in >> dat.m_pos.z;

		
		in >> dat.m_size.x;
		if (dat.m_size.x > 4) dat.m_size.x = 5;
		in >> dat.m_size.y;
		if (dat.m_size.y > 4) dat.m_size.y = 4;
		in >> dat.m_size.z;
		if (dat.m_size.z > 4) dat.m_size.z = 5;
		StoneDataList.emplace_back(dat);
	}

	list<StoneInfo>::iterator iter = StoneDataList.begin();
	list<StoneInfo>::iterator end = StoneDataList.end();

	for (iter; iter != end; iter++)
	{
		float RandomRotate = rand() % 360;
		CGameObject* obj = new CGameObject();
		
		obj->SetPosition(iter->m_pos.x, iter->m_pos.y+3, iter->m_pos.z);
		obj->SetScale(iter->m_size.x, iter->m_size.y, iter->m_size.z);
		switch (iter->m_iType)
		{
		case 0:
			obj->SetChild(pStone, true);
			obj->SetScale(1,1,1);
			obj->SetHitBox(XMFLOAT3(3.f * iter->m_size.x, 0.8f * iter->m_size.y, 3.f * iter->m_size.z));
			break;
		case 1:
			obj->SetChild(pStone2, true);;
			obj->Rotate(0, RandomRotate, 0);
			obj->SetHitBox(XMFLOAT3(6 * iter->m_size.x,4 * iter->m_size.y,6 * iter->m_size.z));
			break;
		case 2:
			obj->SetChild(pStone3, true);;
			obj->Rotate(0, RandomRotate, 0);
			obj->SetHitBox(XMFLOAT3(5 * iter->m_size.x,5 * iter->m_size.y, 5 * iter->m_size.z));
			break;
		case 3:
			obj->SetChild(pStone4, true);;
			obj->Rotate(0, RandomRotate, 0);
			obj->SetHitBox(XMFLOAT3(8 * iter->m_size.x, 3 * iter->m_size.y, 8 * iter->m_size.z));
			break;
		}

		m_StoneObjectslist.push_back(obj);
	}
}

void CScene::LoadTrap(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	CGameObject *pTrap = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/trap.bin", NULL, false);

	fstream in("TrapData.txt", ios::in | ios::binary);
	while (in)
	{
		XMFLOAT2 dat;
		in >> dat.x;
		in >> dat.y;
		TrapDatalist.emplace_back(dat);
	}

	list<XMFLOAT2>::iterator iter = TrapDatalist.begin();
	list<XMFLOAT2>::iterator end = TrapDatalist.end();

	for (iter; iter != end; iter++)
	{
		CTrap* obj = new CTrap();
		obj->SetChild(pTrap, true);
		obj->SetHitBox(XMFLOAT3(1.f, 1.f, 1.f));
		obj->SetPosition(iter->x, m_pTerrain->GetHeight(iter->x, iter->y), iter->y);
		m_TrapList.push_back(obj);
	}
}

void CScene::LoadGrass(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList)
{
	CGrasshader* pShader = new CGrasshader();

	pShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CGameObject *pGrass = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/grass.bin", pShader, false);

	fstream in("GrassData.txt", ios::in | ios::binary);
	while (in)
	{
		XMFLOAT2 dat;
		in >> dat.x;
		in >> dat.y;
		GrassDataList.emplace_back(dat);
	}

	list<XMFLOAT2>::iterator iter = GrassDataList.begin();
	list<XMFLOAT2>::iterator end = GrassDataList.end();

	for (iter; iter != end; iter++)
	{
		float RandomRotate = rand() % 360;
		CGameObject* obj = new CGameObject();
			obj->SetChild(pGrass, true);
		obj->SetPosition(iter->x, m_pTerrain->GetHeight(iter->x, iter->y), iter->y);
		obj->Rotate(0, RandomRotate, 0);
		m_GrassObjectlist.push_back(obj);
	}

}


void CScene::SaveNavigation()
{
	fstream out("NavData.txt", ios::out | ios::binary);
	list<XMFLOAT3>* list = m_pPlayer->GetNavigationList();
	for (auto t : *list)
	{
		out << t.x << " " << t.y << " " << t.z << " " << endl;
	}
}

void CScene::LoadTree(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	
	CGameObject *pTree = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Tree.bin", NULL, false);
	CGameObject *pTree2 = CGameObject::LoadGeometryAndAnimationFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Tree2.bin", NULL, false);

	fstream in("TreeData.txt", ios::in | ios::binary);
	while (in)
	{
		XMFLOAT2 dat;
		in >> dat.x;
		in >> dat.y;
		TreeDatalist.emplace_back(dat);
	}

	list<XMFLOAT2>::iterator iter = TreeDatalist.begin();
	list<XMFLOAT2>::iterator end = TreeDatalist.end();

	for (iter; iter != end; iter++)
	{
		float RandomRotate = rand() % 360;
		CTree* obj = new CTree();
		int treerand = rand() % 2;
		if (treerand == 0)
			obj->SetChild(pTree, true);
		else if (treerand == 1)
			obj->SetChild(pTree2, true);
		obj->SetPosition(iter->x, m_pTerrain->GetHeight(iter->x, iter->y), iter->y);
		obj->SetHitBox(XMFLOAT3(6.f, 20.f, 6.f));
		obj->Rotate(0, RandomRotate, 0);
		m_TreeObjectslist.push_back(obj);
	}

}

bool CScene::ProcessInput(UCHAR *pKeysBuffer)
{
	return(false);
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	m_fElapsedTime = fTimeElapsed;

	if (m_pSnakeObject)
	{
		if (m_pSnakeObject->GetDeathState())
		{
			m_pPlayer->PlusSkillGage(100);
			m_pSnakeObject->Next();
			if (m_pSnakeObject->GetIndex() == MonsterDataList.size())
				m_pSnakeObject->Release();
			else
				m_pSnakeObject->ResetToNext(MonsterDataList.at(m_pSnakeObject->GetIndex()).m_pos);
		}
		else
		{

			m_pSnakeObject->Animate(m_fElapsedTime);
			m_pSnakeObject->UpdateTransform(NULL);

			XMFLOAT3 pos1 = m_pSnakeObject->GetPosition();
			XMFLOAT3 pos2 = m_pPlayer->GetPosition();
			float distance = Vector3::Length(Vector3::Subtract(pos1, pos2));
			if (distance < m_pSnakeObject->GetAggroDistance())
			{
				if(!m_pSnakeObject->getRecognitionMode())
					CSoundMgr::GetInstacne()->PlaySkillSound(_T("Snake"));
				m_pSnakeObject->setRecognitionMode(true);
				m_pSnakeObject->FollowingPosition = pos2;
			}
			else
				m_pSnakeObject->setRecognitionMode(false);
		}
	}
			

	for (auto h : m_HoneyComblist)
	{
		h->Animate(fTimeElapsed);
	}
	for (auto h : m_Mushroomlist)
	{
		h->Animate(fTimeElapsed);
	}
	for (auto h : m_TrapList)
	{
		h->Animate(fTimeElapsed);
	}

	for (auto h : m_DamageUIList)
	{
		h->Update(fTimeElapsed);
	}
	
}

void CScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{

		if (m_pd3dGraphicsRootSignature) pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
		if (m_pd3dCbvSrvDescriptorHeap) pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

		pCamera->SetViewportsAndScissorRects(pd3dCommandList);
		pCamera->UpdateShaderVariables(pd3dCommandList);

		UpdateShaderVariables(pd3dCommandList);

		D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights


		switch (m_MainFramework->GetFlowState())
		{
		case SCENE_STAGE1:

			list<CUI*>::iterator iter = m_UIList->begin();
			list<CUI*>::iterator iter_end = m_UIList->end();

			RenderStage1(pd3dCommandList, pCamera);
			for (int i = 0; i < 2; i++)
			{
				m_ppWaters[i]->UpdateTransform(NULL);
				m_ppWaters[i]->Render(m_pd3dCommandList, pCamera);
			}
			for (iter; iter!= iter_end ; iter++)
			{
				if ((*iter)->bRender)
				{
					(*iter)->UpdateTransform(NULL);
					(*iter)->Render(m_pd3dCommandList, pCamera);
				}
			}
			
			m_pPlayer->GetNavGuide()->Render(m_pd3dCommandList, pCamera);
		}

}

void CScene::ObjectsCollides()
{
	if (m_pPlayer->GetMoveState() == STATE_CHEAT) return;

	m_pPlayer->m_CollideState = 1;
	if (m_ppGameObjects)
	{
		for (int i = 0; i < m_nGameObjects; i++)
		{
			m_ppGameObjects[i]->getCollision(m_pPlayer);
		}
	}
	for (auto n : m_TreeObjectslist) {
		if (n->getCollision(m_pPlayer) != COLLIDE_NONE)
		{

			if (!n->GetHoneyDrop())
			{
				int prob = rand() % 10;
				if (prob < 4)
				{
					XMFLOAT3 pos = m_pPlayer->GetPosition();
					CHoneyComb* temp = new CHoneyComb();
					temp->SetChild(HoneyComb);
					temp->SetPosition(pos.x, pos.y + 20, pos.z);
					temp->SetFloorHeight(m_pTerrain->GetHeight(n->GetPosition().x, n->GetPosition().z));
					temp->Rotate(rand() % 360, rand() % 360, rand() % 360);
					m_HoneyComblist.push_back(temp);
					CSoundMgr::GetInstacne()->PlayEffectSound(_T("Falling"));
				}

				else if (prob < 8)
				{

					XMFLOAT3 pos = m_pPlayer->GetPosition();
					CPotion* temp = new CPotion();
					temp->SetChild(Potion);
					temp->SetPosition(pos.x, pos.y + 20, pos.z);
					temp->SetFloorHeight(m_pTerrain->GetHeight(n->GetPosition().x, n->GetPosition().z));
					m_HoneyComblist.push_back(temp);
					CSoundMgr::GetInstacne()->PlayEffectSound(_T("Falling"));
				}
			}

			n->SetHoneyDrop();
		}

	}

	for (auto n : m_StoneObjectslist)
	{
		n->getCollision(m_pPlayer);
	}
	m_pSnakeObject->getCollision(m_pPlayer);

	for (auto n : m_Mushroomlist)
	{
		if (n->getCollision(m_pPlayer, false) != COLLIDE_NONE)
		{
			if (!n->GetCollided())
			{
				CreateDamageUI( 4);
				m_pPlayer->Damage(4);
				m_pPlayer->Pop(100);
				CSoundMgr::GetInstacne()->PlayEffectSound(_T("Mushroom"));
			}
			n->SetCollided(true);
		}
	}


	for (auto n : m_TrapList)
	{
		if (n->getCollision(m_pPlayer, false) != COLLIDE_NONE)
		{
			if (!n->GetCollided())
			{

				CreateDamageUI( 3);
				m_pPlayer->SetStun();
				m_pPlayer->Damage(3);
				CSoundMgr::GetInstacne()->PlayEffectSound(_T("Trap"));
				n->SetCollided(true);
				m_BloodScreen->bRender = true;
			}
		}

	}
	for (auto n : m_DashList)
	{
		if (n->getCollision(m_pPlayer, false) != COLLIDE_NONE)
		{
			CSoundMgr::GetInstacne()->PlayEffectSound(_T("dash"));
			m_pPlayer->Dash(m_fElapsedTime * 30);
		}

	}


	list<CHoneyComb*> ::iterator honeyiter = m_HoneyComblist.begin();
	list<CHoneyComb*> ::iterator honeyend = m_HoneyComblist.end();
	for (honeyiter; honeyiter != honeyend; honeyiter++)
	{
		if ((*honeyiter)->getCollision(m_pPlayer) )
		{
			if ((*honeyiter)->GetDamage() == 6)
				CreateDamageUI(6);
			else
				CSoundMgr::GetInstacne()->PlayEffectSound(_T("Gulp"));
		}
		if ((*honeyiter)->GetbDie())
		{
			honeyiter = m_HoneyComblist.erase(honeyiter);
			if (honeyiter == honeyend) break;
		}
	}
}

