//-----------------------------------------------------------------------------
// File: Shader.h
//-----------------------------------------------------------------------------

#pragma once

#include "Object.h"
#include "Camera.h"
#include "Scene.h"
class CScene;
class CFish;
class CFishtrap;
class CItemBox;

class CShader
{
public:
	CShader();
	virtual ~CShader();

private:
	int									m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob);
	D3D12_SHADER_BYTECODE ReadCompiledShaderFromFile(WCHAR *pszFileName, ID3DBlob **ppd3dShaderBlob=NULL);

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) { }
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) { }
	virtual void ReleaseShaderVariables() { }

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World) { }

	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList, int n=0);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	virtual void ReleaseUploadBuffers() { }

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext = NULL) { }
	virtual void AnimateObjects(float fTimeElapsed) { }
	virtual void ReleaseObjects() { }
	ID3D12PipelineState* GetPipelineState() { return m_pd3dPipelineState; }
	void SetPipelineState(ID3D12PipelineState* P) { m_pd3dPipelineState = P; }

protected:
	ID3DBlob							*m_pd3dVertexShaderBlob = NULL;
	ID3DBlob							*m_pd3dPixelShaderBlob = NULL;

	ID3D12PipelineState					*m_pd3dPipelineState = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC	m_d3dPipelineStateDesc;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CTerrainShader : public CShader
{
public:
	CTerrainShader();
	virtual ~CTerrainShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
	virtual D3D12_BLEND_DESC CreateBlendState();
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBoxShader : public CShader
{
public:
	CSkyBoxShader();
	virtual ~CSkyBoxShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CStandardShader : public CShader
{
public:
	CStandardShader();
	virtual ~CStandardShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
};


class CGrasshader : public CStandardShader
{
public:
	CGrasshader();
	virtual ~CGrasshader();

	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual D3D12_BLEND_DESC CreateBlendState();
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CObjectsShader : public CStandardShader
{
public:
	CObjectsShader();
	virtual ~CObjectsShader();

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext = NULL);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void ReleaseObjects();

	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

protected:
	CGameObject						**m_ppObjects = 0;
	int								m_nObjects = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CPlayerShader : public CStandardShader
{
public:
	CPlayerShader();
	virtual ~CPlayerShader();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkinnedAnimationShader : public CStandardShader
{
public:
	CSkinnedAnimationShader();
	virtual ~CSkinnedAnimationShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CMirrorShader : public CStandardShader
{
private:
	int m_nPipelineStates = 4;
	ID3D12PipelineState** m_ppd3dPipelineState;
	CWater** m_ppWater;
	CScene* m_pScene;
	XMVECTOR xmvMirrorPlane;
	XMMATRIX xmmtxReflect;

public:
	list<CFishtrap*>* m_FishTrapList = NULL;
	list<CFish*>* m_FishList = NULL;
	list<CItemBox*>* m_ItemBoxList = NULL;
	CGameObject** Boats = NULL;

	void Initialize();
	CMirrorShader();
	void GetWater(CWater** waters) { m_ppWater = waters; }
	virtual ~CMirrorShader();
	void GetScene(CScene* p) { m_pScene = p; }
	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);


	//virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();/*
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();*/
	virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList, int nPipelineState = 0);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle, CCamera* pCamera);

};

class CWaterShader : public CShader
{
public:
	CWaterShader();
	virtual ~CWaterShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
};

class CUIShader : public CShader
{
public:
	CUIShader();
	virtual ~CUIShader();
	
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
};

class CFontShader :  public CUIShader
{
public:
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
};

class CShadowShader : public CShader
{
public:
	CShadowShader();
	virtual ~CShadowShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_BLEND_DESC CreateBlendState();
};

class CEffectShader : public CStandardShader {
public:
	CEffectShader() {}
	~CEffectShader() {}


	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

};