#include "BaseInclude.h"
#include "Vertices.h"
#include "Texture.h"
#include "Matrix.h"
#include "Model.h"

const int RTV_NUM = 2; //レンダーターゲットの枚数
const FLOAT CLEAR_COLOR[4] = { 1.0f, 1.0f, 1.00f, 1.0f }; // クリアする色

class D3D12Manager
{
public:

	D3D12Manager(HWND hwnd, int width, int height, LPCWSTR vertexShaderName, LPCWSTR pixelShaderName);
	~D3D12Manager();

	HRESULT Render();

	ComPtr<ID3D12Device> Dev = nullptr;
	//コマンドリスト
	ComPtr<ID3D12GraphicsCommandList> CommandList;

private:

	//サイズ
	int _width;
	int _height;

	//ウィンドウのハンドル
	HWND _windowHandle;

	ComPtr<IDXGIFactory4> _dxgiFactory = nullptr;
	ComPtr<IDXGISwapChain4> _swapChain = nullptr;

	ComPtr<ID3D12CommandAllocator>	_commandAllocator;

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
	UINT64 _fenceValue = 0;

	//ルートシグネチャ
	ComPtr<ID3D12RootSignature>	_rootSignature;

	//パイプラインステート
	ComPtr<ID3D12PipelineState> _pipelineState;

	//コマンドキュー
	ComPtr<ID3D12CommandQueue>	_commandQueue;

	//リソース用ヒープ
	ComPtr<ID3D12DescriptorHeap> _resourceHeaps = nullptr;

	//シザー矩形
	D3D12_RECT _scissorRect;
	//ビューポート
	D3D12_VIEWPORT _viewPort;

	//関数

	//生成
	HRESULT CreateDevice();
	HRESULT CreateFactory();
	HRESULT CreateCommandList();
	HRESULT CreateCommandQueue();
	HRESULT CreateFence();
	HRESULT CreateSwapChain();
	HRESULT CreateRenderTargetView();
	HRESULT CreateResourceHeap();
	HRESULT CreateDepthStencilBuffer();
	HRESULT CreateRootSignature();
	HRESULT CreatePipelineStateObject(LPCWSTR vertexShaderName, LPCWSTR pixelShaderName);
	void CreateViewPortScissorRect();

	//GPUへの転送(テクスチャ)
	void CopyTexture();

	//描画
	HRESULT StackDrawCommandList();
	//コマンド待ち
	HRESULT WaitForPreviousFrame();
	//リセット
	HRESULT ResetCommand(ComPtr<ID3D12PipelineState> pipelineState);

	//描画するオブジェクト達
	//出来ればmain側でコールバックとかでやりたい
	Model* _model;
	Texture* _tex;
	Matrix* _matrix;

	//void DrawCallback(std::function<void(ID3D12GraphicsCommandList*)> draw);
};