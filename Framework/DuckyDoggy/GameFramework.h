#pragma once
class CGameFramework // 게임의 뼈대 입니다.
{
	// Direct3D 디바이스 생성, 관리
	// 화면 출력을 위한 여러가지 처리
	//		게임 객체의 생성, 관리, 사용자 입력, 에니메이션 등
private:
	HINSTANCE m_hInstance;
	HWND m_hwnd;

	int m_nWndClientWidth;
	int m_nWndClientHeight;

	IDXGIFactory4 *m_pdxgiFactory; // DXGI 팩토리 인터페이스에 대한 포인터
	IDXGISwapChain3 *m_pdxgiSwapChain;//스왑 체인 인터페이스에 대한 포인터(디스플레이 제어)
	ID3D12Device *m_pd3dDevice;
	//Direct3D 디바이스 인터페이스에 대하 포인터, 리소스 생성

	bool m_bMsaa4xEnable = false;
	UINT m_nMsaa4xQualityLevels = 0;
	//MSAA 다중 샘플링 활성화, 다중 샘플링 레벨 설정

	static const UINT m_nSwapChainBuffers = 2;	//스왑 체인 후면 버퍼 개수
	UINT m_nSwapChainBufferIndex;

	ID3D12Resource *m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];// 렌더 타겟 버퍼,
	ID3D12DescriptorHeap *m_pd3dRtvDescriptorHeap;//서술자 힙 인터페이스 포인터,
	UINT m_nRtvDescriptorIncrementSize;  //렌더 타겟 서술자 원소

	ID3D12Resource *m_pd3dDepthStencilBuffer;// 깊이-스텐실 버퍼
	ID3D12DescriptorHeap *m_pd3dDsvDescriptorHeap;//서술자 힙 인터페이스 포인터
	UINT  m_nDsvDescriptorIncrementSize;//깊이 - 스텐실 서술자 원소 크기

	ID3D12CommandQueue *m_pd3dCommandQueue; // 명령큐
	ID3D12CommandAllocator *m_pd3dCommandAllocator;// 명령 할당자
	ID3D12GraphicsCommandList *m_pd3dCommandList;// 명령 리스트 인터페이스 포인터

	ID3D12PipelineState *m_pd3dPipelineState;//그래픽스 파이프라인 상태 객체에 대한 인터페이스 포인터
	
	ID3D12Fence *m_pd3dFence;// 펜스 인터페이스 포인터
	UINT64 m_nFenceValue;//펜스 값
	HANDLE m_hFenceEvent;//이벤트 핸들

#if defined(_DEBUG)
	ID3D12Debug *m_pd3dDebugController;
#endif
	D3D12_VIEWPORT m_d3dViewport;//뷰포트
	D3D12_RECT m_d3dScissorRect;//시저 사각형

public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	//프레임워크를 초기화하는 함수이다(주 윈도우가 생성되면 호출된다).

	void OnDestroy(); 
	void CreateSwapChain();//스왑 체인
	void CreateDirect3DDevice(); //디바이스
	void CreateRtvAndDsvDescriptorHeaps();// 서술자 힙
	void CreateCommandQueueAndList();// 명령 큐/할당자/리스트

	void BuildObjects();
	void ReleaseObjects();
	//렌더링할 메쉬와 게임 객체를 생성하고 소멸

	//프레임워크의 핵심(사용자 입력, 애니메이션, 렌더링)을 구성
	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	void WaitForGpuComplete(); //CPU와 GPU 동기화


	//윈도우의 메시지(키보드, 마우스 입력)를 처리
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM
		lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM
		lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam,
		LPARAM lParam);

	void CreateRenderTargetView();
	void CreateDepthStencilView();


};

