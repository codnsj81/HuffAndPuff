//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"
#include "Player.h"

#define MAX_LIGHTS						16 

#define POINT_LIGHT						1
#define SPOT_LIGHT						2
#define DIRECTIONAL_LIGHT				3

class CMonster;
class CStartUI;
class CDamageUI;
class CTrap;
struct StoneInfo
{
	int							m_iType;
	XMFLOAT3					m_pos;
	XMFLOAT3					m_size;
};

struct DashInfo
{
	XMFLOAT3		m_pos;
	XMFLOAT3		m_rot;
};
struct LIGHT
{
	XMFLOAT4							m_xmf4Ambient;
	XMFLOAT4							m_xmf4Diffuse;
	XMFLOAT4							m_xmf4Specular;
	XMFLOAT3							m_xmf3Position;
	float 								m_fFalloff;
	XMFLOAT3							m_xmf3Direction;
	float 								m_fTheta; //cos(m_fTheta)
	XMFLOAT3							m_xmf3Attenuation;
	float								m_fPhi; //cos(m_fPhi)
	bool								m_bEnable;
	int									m_nType;
	float								m_fRange;
	float								padding;
};										
										
struct LIGHTS							
{										
	LIGHT								m_pLights[MAX_LIGHTS];
	XMFLOAT4							m_xmf4GlobalAmbient;
	int									m_nLights;
};

class CScene
{
public:
    CScene();
    ~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	void BuildDefaultLightsAndMaterials();
	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseObjects();
	void Update();
	void SetDuckyNDoggy(CPlayer* ducky, CPlayer* doggy, CPlayer* player);

	void PlayerAttack();


	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }
	
	void LoadStone(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void LoadTrap(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);

	void LoadGrass(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void LoadTree(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	bool ProcessInput(UCHAR *pKeysBuffer);
    void AnimateObjects(float fTimeElapsed);
    void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera=NULL);
	void ObjectsCollides();
	void ReleaseUploadBuffers();
	CWater** GetWaters() { return m_ppWaters; }

	CPlayer								*m_pPlayer = NULL;
	CPlayer								*m_pDoggy;
	CPlayer								*m_pDucky;
	CGameObject*						m_pSnakeObject = NULL;
	ID3D12Device*						m_pd3dDevice = NULL;
	ID3D12GraphicsCommandList*			m_pd3dCommandList = NULL;
	void ResetObjects();
	void PlusTreeData();
	void SaveTreeData();


	void PlusTrapData();
	void SaveTrapData();

	void PlusGrassData();
	void SaveGrassData();
	void LoadDash(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList);

	void PlusMushroomData();
	void SaveMushroomData();
	void BuildMushroomData(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList);

	void PlusStoneData();
	void SaveStoneData();


	void PlusDashData();
	void SaveDashData();

	void PlusMonsterData();
	void SaveMonsterData();

	void BuildMonsterList(ID3D12Device * pd3dDevice, ID3D12GraphicsCommandList * pd3dCommandList);

	list<CMonster*>* GetMonsterList() {
		return &M_MonsterObjectslist;
	}

protected:
	ID3D12RootSignature					*m_pd3dGraphicsRootSignature = NULL;

	static ID3D12DescriptorHeap			*m_pd3dCbvSrvDescriptorHeap;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorStartHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorStartHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorStartHandle;

	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dCbvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorNextHandle;
	static D3D12_CPU_DESCRIPTOR_HANDLE	m_d3dSrvCPUDescriptorNextHandle;
	static D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dSrvGPUDescriptorNextHandle;
	
	void CreateDamageUI(CPlayer* pPlayer);


public:
	static void CreateCbvSrvDescriptorHeaps(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, int nShaderResourceViews);

	static D3D12_GPU_DESCRIPTOR_HANDLE CreateConstantBufferViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride);
	static D3D12_GPU_DESCRIPTOR_HANDLE CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture, UINT nRootParameter, bool bAutoIncrement);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorNextHandle() { return(m_d3dCbvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorNextHandle() { return(m_d3dCbvGPUDescriptorNextHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorNextHandle() { return(m_d3dSrvCPUDescriptorNextHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorNeLxtHandle() { return(m_d3dSrvGPUDescriptorNextHandle); }

	int									m_nGameObjects = 0;
	
	CTexture*							m_DamageUITex = NULL;
	CGameObject* HoneyComb = NULL;
	
	CGameObject							**m_ppGameObjects = NULL;
	list<CTree*>						m_TreeObjectslist;
	list<CGameObject*>					m_StoneObjectslist;
	list<CMonster*>						M_MonsterObjectslist;
	list<CGameObject*>					m_GrassObjectlist;
	list<CHoneyComb*>					m_HoneyComblist;
	list<CMushroom*>					m_Mushroomlist;
	list<CDash*>						m_DashList;
	list<CTrap*>						m_TrapList;
	list<CDamageUI*>					m_DamageUIList;

	int									m_nWaters = 0;
	CWater								**m_ppWaters = NULL;


	float								m_fElapsedTime = 0.0f;

	int									m_nShaders = 0;
	CShader								**m_ppShaders = NULL;

	CSkyBox								*m_pSkyBox = NULL;
	CHeightMapTerrain					*m_pTerrain = NULL;

	LIGHT								*m_pLights = NULL;
	int									m_nLights = 0;

	XMFLOAT4							m_xmf4GlobalAmbient;

	ID3D12Resource						*m_pd3dcbLights = NULL;
	LIGHTS								*m_pcbMappedLights = NULL;

	list<XMFLOAT2> TrapDatalist;
	list<XMFLOAT2> MushroomDatalist;
	list<XMFLOAT2> TreeDatalist;
	list<StoneInfo>	StoneDataList;
	list<XMFLOAT2> GrassDataList;
	list<StoneInfo>	MonsterDataList;
	list<DashInfo> DashDataList;
};
