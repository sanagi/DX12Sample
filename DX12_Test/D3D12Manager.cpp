#include "D3D12Manager.h"
#include"Application.h"

#pragma region コンストラクタ・デストラクタ

/// <summary>
/// コンストラクタ
/// </summary>
/// <param name="hwnd"></param>
D3D12Manager::D3D12Manager(HWND hwnd) : _windowHandle(hwnd) {

	auto& app = Application::Instance();
	_winSize = app.GetWindowSize();

	//デバイスの初期化
	CreateFactory();
	CreateDevice();

	//コマンドリスト
	CreateCommandList();
	//コマンドキュー
	CreateCommandQueue();
	//フェンス
	CreateFence();
	//スワップチェイン
	CreateSwapChain();
	//レンダ―ターゲットビュー
	CreateFinalRenderTargetView();
	//深度バッファ作成
	CreateDepthStencilBuffer();

	//リソース用ヒープ
	CreateScene();
}

D3D12Manager::~D3D12Manager() {}

#pragma endregion

#pragma region 初期化

//デバイスの生成
HRESULT D3D12Manager::CreateDevice() {

	//どのバージョンか
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	IDXGIAdapter* warpAdapter;
	_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));


	//デバイス作成
	HRESULT hr{};
	//D3D_FEATURE_LEVEL featureLevel;
	for (auto lv : levels) {
		if (D3D12CreateDevice(warpAdapter, lv, IID_PPV_ARGS(&_device)) == S_OK) {
			//featureLevel = lv;
			return S_OK;
		}

	}

	//デバイスを作ったらアダプタ要らない
	warpAdapter->Release();

	return S_FALSE;
}

//ファクトリの作成
HRESULT D3D12Manager::CreateFactory() {
	HRESULT hr{};
	UINT flag{};

	//デバッグモードの場合はデバッグレイヤーを有効にする
#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugLayer;
		hr = D3D12GetDebugInterface(IID_PPV_ARGS(debugLayer.GetAddressOf()));
		if (FAILED(hr)) {
			return hr;
		}
		debugLayer->EnableDebugLayer();
	}

	flag |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	//ファクトリの作成
	HRESULT factoryResult = CreateDXGIFactory2(flag, IID_PPV_ARGS(_dxgiFactory.GetAddressOf()));

	return hr;
}

#pragma endregion

#pragma region コマンドリスト

//コマンドリスト作成
HRESULT D3D12Manager::CreateCommandList() {
	HRESULT hr;

	//コマンドアロケータの作成
	hr = _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator));
	if (FAILED(hr)) {
		return hr;
	}

	//コマンドアロケータとバインドしてコマンドリストを作成する
	hr = _device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator.Get(), nullptr, IID_PPV_ARGS(&_commandList));

	return hr;
}

//コマンドキューを生成
HRESULT D3D12Manager::CreateCommandQueue() {
	HRESULT hr{};
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

	//タイムアウトなし
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	//アダプターを使わない時は0
	commandQueueDesc.NodeMask = 0;
	//コマンドリストと合わせる
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//優先度0
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

	hr = _device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(_commandQueue.GetAddressOf()));
	if (FAILED(hr)) {
		return hr;
	}

	return hr;
}

#pragma endregion


#pragma region フェンス

//コマンドキューに利用するフェンス作成
HRESULT D3D12Manager::CreateFence() {
	HRESULT hr{};

	//コマンドキュー用のフェンスの生成
	hr = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_queueFence.GetAddressOf()));

	return hr;
}

//コマンド待ち
HRESULT D3D12Manager::WaitForPreviousFrame() {
	HRESULT hr;

	//CPU側でfence値のインクリメント
	hr = _commandQueue->Signal(_queueFence.Get(), ++_fenceValue);
	if (FAILED(hr)) {
		return -1;
	}

	//GPU処理が終わったときに更新される値がCPU側でインクリメントした値でないなら待ちイベントを繰り返す
	if (_queueFence->GetCompletedValue() != _fenceValue) {
		HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		//fenceValueになったらイベントを発生させる
		hr = _queueFence->SetEventOnCompletion(_fenceValue, fenceEvent);
		if (FAILED(hr)) {
			return -1;
		}

		//イベントが発生するまで待つ
		WaitForSingleObject(fenceEvent, INFINITE);

		CloseHandle(fenceEvent);
	}
	return S_OK;
}

#pragma endregion

#pragma region スワップチェイン

//スワップチェインの作成
HRESULT D3D12Manager::CreateSwapChain() {
	HRESULT hr{};
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	swapChainDesc.Width = _winSize.cx;
	swapChainDesc.Height = _winSize.cy;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = RTV_NUM;//裏と表で２枚作りたいので2を指定する
	//スケーリング可能
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	//フリップ後即破棄
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	//指定なし
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	//切り替え可能
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	hr = _dxgiFactory->CreateSwapChainForHwnd(_commandQueue.Get(), _windowHandle, &swapChainDesc, NULL, NULL, (IDXGISwapChain1**)_swapChain.GetAddressOf());
	if (FAILED(hr)) {
		return hr;
	}

	return S_OK;
}

#pragma endregion

#pragma region シーン作成

void D3D12Manager::CreateScene() {
	//_sceneMatrix = new Matrix();
	//_sceneMatrix->CreateSceneView(_device, _winSize.cx, _winSize.cy);
	//return Matrix::CreateSceneView(_device, _winSize.cx, _winSize.cy, _mappedSceneData, _sceneDescHeap);
	CreateSceneView();
}

//ビュープロジェクション用ビューの生成
HRESULT D3D12Manager::CreateSceneView() {
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	auto result = _swapChain->GetDesc1(&desc);
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneData) + 0xff) & ~0xff);
	//定数バッファ作成
	result = _device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_sceneConstBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	_mappedSceneData = nullptr;//マップ先を示すポインタ
	result = _sceneConstBuff->Map(0, nullptr, (void**)&_mappedSceneData);//マップ

	XMFLOAT3 eye(0, 15, -15);
	XMFLOAT3 target(0, 15, 0);
	XMFLOAT3 up(0, 1, 0);
	_mappedSceneData->view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	_mappedSceneData->proj = XMMatrixPerspectiveFovLH(XM_PIDIV4,//画角は45°
		static_cast<float>(desc.Width) / static_cast<float>(desc.Height),//アス比
		0.1f,//近い方
		1000.0f//遠い方
	);
	_mappedSceneData->eye = eye;

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
	descHeapDesc.NodeMask = 0;//マスクは0
	descHeapDesc.NumDescriptors = 1;//
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//デスクリプタヒープ種別
	result = _device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(_sceneDescHeap.ReleaseAndGetAddressOf()));//生成

	////デスクリプタの先頭ハンドルを取得しておく
	auto heapHandle = _sceneDescHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _sceneConstBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = _sceneConstBuff->GetDesc().Width;
	//定数バッファビューの作成
	_device->CreateConstantBufferView(&cbvDesc, heapHandle);
	return result;

}

#pragma endregion

#pragma region レンダーターゲット

//レンダーターゲットビューを作る
HRESULT D3D12Manager::CreateFinalRenderTargetView() {
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	auto result = _swapChain->GetDesc1(&desc);

	HRESULT hr{};
	D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};

	//RTV用デスクリプタヒープの作成
	heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heap_desc.NumDescriptors = RTV_NUM;
	heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heap_desc.NodeMask = 0;

	hr = _device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(_rtvHeaps.GetAddressOf()));
	if (FAILED(hr)) {
		return hr;
	}

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	hr = _swapChain->GetDesc(&swcDesc);
	_backBuffers.resize(swcDesc.BufferCount);

	//ディスクリプタヒープの先頭を取得
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	//SRGBレンダーターゲットビュー設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (UINT i = 0; i < swcDesc.BufferCount; ++i)
	{
		hr = _swapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&_backBuffers[i]));
		
		rtvDesc.Format = _backBuffers[i]->GetDesc().Format;
		
		//レンダ―ターゲットビューの作成
		_device->CreateRenderTargetView(_backBuffers[i], &rtvDesc, rtvHandle);

		//先頭からポインタを進める
		rtvHandle.ptr += _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	//ビューポートとRect作成
	_viewPort.reset(new CD3DX12_VIEWPORT(_backBuffers[0]));
	_scissorRect.reset(new CD3DX12_RECT(0, 0, desc.Width, desc.Height));

	return hr;
}

//深度バッファ作成
HRESULT D3D12Manager::CreateDepthStencilBuffer() {
	HRESULT hr;

	DXGI_SWAP_CHAIN_DESC1 desc = {};
	auto result = _swapChain->GetDesc1(&desc);

	//深度バッファの作成
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Width = desc.Width;
	resourceDesc.Height = desc.Height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.MipLevels = 1;
	resourceDesc.Alignment = 0;

	//深度値ヒーププロパティ
	auto depthHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	//クリアバリュー
	CD3DX12_CLEAR_VALUE depthClearValue(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);


	ID3D12Resource* depthBuffer = nullptr;
	hr = _device->CreateCommittedResource(&depthHeapProp, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&depthBuffer));

	//深度バッファ用のデスクリプタヒープの作成
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.NumDescriptors = 1;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = _device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&_dsvHeaps));
	if (FAILED(hr)) {
		return hr;
	}

	//深度バッファのビューの作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; //深度に32ビット
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	_device->CreateDepthStencilView(depthBuffer, &dsvDesc, _dsvHeaps->GetCPUDescriptorHandleForHeapStart());

	return hr;
}

#pragma endregion

#pragma region 描画

/// <summary>
/// 描画前の処理
/// </summary>
void D3D12Manager::BeginDraw() {
	//DirectX処理
	//バックバッファのインデックスを取得
	auto backBufferIndex = _swapChain->GetCurrentBackBufferIndex();
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(_backBuffers[backBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	_commandList->ResourceBarrier(1, &barrier);


	//レンダーターゲットを指定
	auto rtvH = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += backBufferIndex * _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//深度を指定
	auto dsvH = _dsvHeaps->GetCPUDescriptorHandleForHeapStart();
	_commandList->OMSetRenderTargets(1, &rtvH, false, &dsvH);
	_commandList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


	//画面クリア
	float clearColor[] = { 1.0f,1.0f,1.0f,1.0f };//白色
	_commandList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	//ビューポート、シザー矩形のセット
	_commandList->RSSetViewports(1, _viewPort.get());
	_commandList->RSSetScissorRects(1, _scissorRect.get());
}

/// <summary>
/// Renderする者の指定
/// </summary>
/// <param name="pipelineState"></param>
/// <param name="rootSignature"></param>
/// <param name="primitivTopology"></param>
void D3D12Manager::SetRenderer(ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootSignature) {
	//PMD用の描画パイプラインに合わせる
	_commandList->SetPipelineState(pipelineState);
	//ルートシグネチャもPMD用に合わせる
	_commandList->SetGraphicsRootSignature(rootSignature);
}

/// <summary>
/// 現在のシーン(ビュープロジェクション)をセット
/// </summary>
void D3D12Manager::SetScene() {
	//ID3D12DescriptorHeap* sceneheaps[] = { _sceneMatrix->GetSceneDescHeap().Get() };
	//_commandList->SetDescriptorHeaps(1, sceneheaps);
	//_commandList->SetGraphicsRootDescriptorTable(0, _sceneMatrix->GetSceneDescHeap()->GetGPUDescriptorHandleForHeapStart());
	//現在のシーン(ビュープロジェクション)をセット
	ID3D12DescriptorHeap* sceneheaps[] = { _sceneDescHeap.Get() };
	_commandList->SetDescriptorHeaps(1, sceneheaps);
	_commandList->SetGraphicsRootDescriptorTable(0, _sceneDescHeap->GetGPUDescriptorHandleForHeapStart());
}

/// <summary>
/// Drawの後処理
/// </summary>
void D3D12Manager::EndDraw() {
	auto backBufferIndex = _swapChain->GetCurrentBackBufferIndex();
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(_backBuffers[backBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	_commandList->ResourceBarrier(1, &barrier);

	//命令のクローズ
	_commandList->Close();

	//コマンドリストの実行
	ID3D12CommandList* cmdlists[] = { _commandList.Get() };
	_commandQueue->ExecuteCommandLists(1, cmdlists);
	//待ち
	WaitForPreviousFrame();

	//リセット
	ResetCommand();

	//フリップ処理
	_swapChain->Present(1, 0);
}

/// <summary>
/// リセット
/// </summary>
/// <returns></returns>
HRESULT D3D12Manager::ResetCommand() {
	HRESULT hr;
	//アロケーターのリセット
	hr = _commandAllocator->Reset();
	if (FAILED(hr)) {
		return hr;
	}

	hr = _commandList->Reset(_commandAllocator.Get(), nullptr);
	if (FAILED(hr)) {
		return hr;
	}

	return hr;
}

#pragma endregion

#pragma region ゲッター

ComPtr<ID3D12Device> D3D12Manager::GetDevice() {
	return _device;
}

ComPtr<ID3D12GraphicsCommandList> D3D12Manager::GetCommandList() {
	return _commandList;
}

#pragma endregion
