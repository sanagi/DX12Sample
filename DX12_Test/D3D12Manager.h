#include "BaseInclude.h"
const int RTV_NUM = 2; //レンダーターゲットの枚数
const FLOAT CLEAR_COLOR[4] = { 1.0f, 0.0f, 0.20f, 1.0f }; // クリアする色

class D3D12Manager
{
public:
	
	D3D12Manager(HWND hwnd, int width, int height, LPCWSTR vertexShaderName, LPCWSTR pixelShaderName);
	~D3D12Manager();

	HRESULT Render(std::function<void(ID3D12GraphicsCommandList*)> draw);

private:

	//サイズ
	int _width;
	int _height;

	//ウィンドウのハンドル
	HWND _windowHandle;

	ComPtr<ID3D12Device> _dev = nullptr;
	ComPtr<IDXGIFactory4> _dxgiFactory = nullptr;
	ComPtr<IDXGISwapChain4> _swapChain = nullptr;

	//コマンドリスト
	ComPtr<ID3D12GraphicsCommandList> _commandList;
	ComPtr<ID3D12CommandAllocator>	_commandAllocator;
	//コマンドキュー
	ComPtr<ID3D12CommandQueue>	_commandQueue;

	//RTV用ヒープ
	ComPtr<ID3D12DescriptorHeap> _rtvHeaps = nullptr;
	//RTV用リソース
	ComPtr<ID3D12Resource> _rtvBuffers[RTV_NUM];

	//Depthステンシル用ヒープ
	ComPtr<ID3D12DescriptorHeap> _dsvHeaps = nullptr;

	//Descriptorのインクリメントサイズ
	UINT _descHandleIncSize;

	//フェンス
	ComPtr<ID3D12Fence> _queueFence;

	//ルートシグネチャ
	ComPtr<ID3D12RootSignature>	_rootSignature;

	//パイプラインステート
	ComPtr<ID3D12PipelineState> _pipelineState;

	//関数

	//生成
	HRESULT CreateDevice();
	HRESULT CreateFactory();
	HRESULT CreateCommandList();
	HRESULT CreateCommandQueue();
	HRESULT CreateFence();
	HRESULT CreateSwapChain();
	HRESULT CreateRenderTargetView();
	HRESULT CreateDepthStencilBuffer();
	HRESULT CreateRootSignature();
	HRESULT CreatePipelineStateObject(LPCWSTR vertexShaderName, LPCWSTR pixelShaderName);

	//描画
	HRESULT StackDrawCommandList(std::function<void(ID3D12GraphicsCommandList*)> draw);
	HRESULT WaitForPreviousFrame();

	//void DrawCallback(std::function<void(ID3D12GraphicsCommandList*)> draw);
};

