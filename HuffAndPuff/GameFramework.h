#pragma once

#define FRAME_BUFFER_WIDTH		1000
#define FRAME_BUFFER_HEIGHT		750

#define SCENE_MAIN			0
#define SCENE_MANUAL		1
#define SCENE_STAGE1		2

#include "../Headers/Include.h"
#include "Timer.h"
#include "Player.h"
#include "Scene.h"
#include "CUI.h"

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();

	void CreateRtvAndDsvDescriptorHeaps();
	
	void CreateRenderTargetViews();
	void CreateDepthStencilView();

	void ChangeSwapChainState();

	void BuildPlayers();
    void BuildObjects();
    void ReleaseObjects();
	void CheckInWater();


    void ProcessInput();
    void AnimateObjects();
    void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void MouseClickInManual(POINT pos);
	void MouseClickInMain(POINT pos);

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	 CPlayer* GetPlayer() const { if(m_pPlayer!=nullptr) return m_pPlayer; }
	 CScene* GetScene() { return m_pScene; }

	 int	GetFlowState() { return m_FLOWSTATE; }
	 list<CUI*>* GetUIList() { return m_UIList; }
	 void BuildUI();
	 void SetbReverseControlMode();

private:

	float						m_fReverseTime = 0;
	bool						m_bReverseControl = false;
	int							m_FLOWSTATE = SCENE_MAIN;
	HINSTANCE					m_hInstance;
	HWND						m_hWnd; 

	list<CUI*>					*m_UIList;
	int							m_nWndClientWidth;
	int							m_nWndClientHeight;
       
	IDXGIFactory4				*m_pdxgiFactory = NULL;
	IDXGISwapChain3				*m_pdxgiSwapChain = NULL;
	ID3D12Device				*m_pd3dDevice = NULL;

	bool						m_bMsaa4xEnable = false;
	UINT						m_nMsaa4xQualityLevels = 0;

	static const UINT			m_nSwapChainBuffers = 4;
	UINT						m_nSwapChainBufferIndex;

	ID3D12Resource				*m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap		*m_pd3dRtvDescriptorHeap = NULL;
	UINT						m_nRtvDescriptorIncrementSize;

	ID3D12Resource				*m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap		*m_pd3dDsvDescriptorHeap = NULL;
	UINT						m_nDsvDescriptorIncrementSize;

	ID3D12CommandAllocator		*m_pd3dCommandAllocator = NULL;
	ID3D12CommandQueue			*m_pd3dCommandQueue = NULL;
	ID3D12GraphicsCommandList	*m_pd3dCommandList = NULL;

	ID3D12Fence					*m_pd3dFence = NULL;
	UINT64						m_nFenceValues[m_nSwapChainBuffers];
	HANDLE						m_hFenceEvent;

#if defined(_DEBUG)
	ID3D12Debug					*m_pd3dDebugController;
#endif

	CGameTimer					m_GameTimer;

	CScene						*m_pScene = NULL;
	CPlayer						*m_pPlayer = NULL;
	CCamera						*m_pCamera = NULL;

	CBackgroundUI**						m_pBackUIArr;
	

	POINT						m_ptOldCursorPos;

	_TCHAR						m_pszFrameRate[70];


	bool								m_bPlaying = true;
	float								m_overCountDown = 0;

	CStartUI*							m_pOverUI = NULL;
};

