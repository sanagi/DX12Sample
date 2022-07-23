#pragma once
#include "Matrix.h"

const int RTV_NUM = 2; //レンダーターゲットの枚数
const FLOAT CLEAR_COLOR[4] = { 1.0f, 1.0f, 1.00f, 1.0f }; // クリアする色

class D3D12Manager
{
public:

	D3D12Manager(HWND hwnd);
	~D3D12Manager();

	ComPtr<ID3D12Device> GetDevice();
	ComPtr<ID3D12GraphicsCommandList> GetCommandList();

	//描画
	void BeginDraw();
	void SetRenderer(ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootSignature);
	void SetScene();
	void EndDraw();

private:

	//サイズ
	SIZE _winSize;

	//ウィンドウのハンドル
	HWND _windowHandle;

	ComPtr<IDXGIFactory4> _dxgiFactory = nullptr;
	ComPtr<IDXGISwapChain4> _swapChain = nullptr;

	//デバイス
	ComPtr<ID3D12Device> _device = nullptr;
	//アロケータ
	ComPtr<ID3D12CommandAllocator>	_commandAllocator;
	//コマンドキュー
	ComPtr<ID3D12CommandQueue>	_commandQueue;
	//コマンドリスト
	ComPtr<ID3D12GraphicsCommandList> _commandList;

	//RTV用ヒープ
	ComPtr<ID3D12DescriptorHeap> _rtvHeaps = nullptr;
	//Depthステンシル用ヒープ
	ComPtr<ID3D12DescriptorHeap> _dsvHeaps = nullptr;
	//RTV用リソース
	std::vector<ID3D12Resource*> _backBuffers;

	//シザー矩形
	std::unique_ptr<D3D12_RECT> _scissorRect;
	//ビューポート
	std::unique_ptr<D3D12_VIEWPORT> _viewPort;

	//シーンを構成する行列
	Matrix* _sceneMatrix;
	//Matrix::SceneData* _mappedSceneData;
	//ComPtr<ID3D12DescriptorHeap> _sceneDescHeap = nullptr;

	//シーンを構成するバッファまわり
	ComPtr<ID3D12Resource> _sceneConstBuff = nullptr;

	struct SceneData {
		DirectX::XMMATRIX view;//ビュー行列
		DirectX::XMMATRIX proj;//プロジェクション行列
		DirectX::XMFLOAT3 eye;//視点座標
	};
	SceneData* _mappedSceneData;
	ComPtr<ID3D12DescriptorHeap> _sceneDescHeap = nullptr;

	//フェンス
	ComPtr<ID3D12Fence> _queueFence;
	UINT64 _fenceValue = 0;

	//関数
	//初期化周り
	HRESULT CreateDevice(); //デバイス生成
	HRESULT CreateFactory(); //Factory生成

	//コマンド系
	HRESULT CreateCommandList(); //コマンドリスト作成
	HRESULT CreateCommandQueue();//コマンドキュー作成
	
	//フェンス、スワップチェイン
	HRESULT CreateFence(); //フェンス
	HRESULT CreateSwapChain(); //スワップチェイン

	//シーン作成
	void CreateScene(); //シーンごと作成
	HRESULT CreateSceneView();

	//レンダーターゲット系
	HRESULT CreateFinalRenderTargetView();
	HRESULT CreateDepthStencilBuffer();

	//コマンド待ち
	HRESULT WaitForPreviousFrame();
	//リセット
	HRESULT ResetCommand();
};

