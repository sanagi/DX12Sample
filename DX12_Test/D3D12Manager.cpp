#include "D3D12Manager.h"

//コンストラクタ
D3D12Manager::D3D12Manager(HWND hwnd, int width, int height, LPCWSTR vertexShaderName, LPCWSTR pixelShaderName) : _windowHandle(hwnd), _width(width), _height(height){
	
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
	CreateRenderTargetView();
	//深度バッファ作成
	CreateDepthStencilBuffer();

	//描画オブジェクト作る
	//どっか別でやりたい
	_model = new Model(Dev, "Model/初音ミク.pmd", "rb");

	//ルートシグネチャ
	CreateRootSignature();
	//パイプラインステート
	CreatePipelineStateObject(vertexShaderName, pixelShaderName);

	//ビューポートとシザー矩形
	CreateViewPortScissorRect();

	//リソース用ヒープ
	CreateResourceHeap();

	auto basicHeapHandle = _resourceHeaps->GetCPUDescriptorHandleForHeapStart();
	//テクスチャ
	//_tex = new Texture(Dev, _resourceHeaps->GetCPUDescriptorHandleForHeapStart());
	//CopyTexture();

	//basicHeapHandle.ptr += Dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//行列
	_matrix = new Matrix(Dev, _width, _height, basicHeapHandle);
}

D3D12Manager::~D3D12Manager() {}

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
		if (D3D12CreateDevice(warpAdapter, lv, IID_PPV_ARGS(&Dev)) == S_OK) {
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

//コマンドリスト作成
HRESULT D3D12Manager::CreateCommandList() {
	HRESULT hr;

	//コマンドアロケータの作成
	hr = Dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator));
	if (FAILED(hr)) {
		return hr;
	}

	//コマンドアロケータとバインドしてコマンドリストを作成する
	hr = Dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator.Get(), nullptr, IID_PPV_ARGS(&CommandList));

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
	
	hr = Dev->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(_commandQueue.GetAddressOf()));
	if (FAILED(hr)) {
		return hr;
	}

	return hr;
}

//コマンドキューに利用するフェンス作成
HRESULT D3D12Manager::CreateFence() {
	HRESULT hr{};

	//コマンドキュー用のフェンスの生成
	hr = Dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_queueFence.GetAddressOf()));

	return hr;
}

//スワップチェインの作成
HRESULT D3D12Manager::CreateSwapChain() {
	HRESULT hr{};
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	swapChainDesc.Width = _width;
	swapChainDesc.Height = _height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
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

/// <summary>
/// リソース(定数バッファ、テクスチャ)用ヒープ作成
/// </summary>
/// <returns></returns>
HRESULT D3D12Manager::CreateResourceHeap() {
	HRESULT hr{};

	D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
	descHeapDesc.NodeMask = 0;//マスクは0
	descHeapDesc.NumDescriptors = 2;//SRV1つとCBV1つ
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//デスクリプタヒープ種別
	hr = Dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(_resourceHeaps.GetAddressOf()));//生成

	return hr;
}

//レンダーターゲットビューを作る
HRESULT D3D12Manager::CreateRenderTargetView() {
	HRESULT hr{};
	D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};

	//RTV用デスクリプタヒープの作成
	heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heap_desc.NumDescriptors = RTV_NUM;
	heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heap_desc.NodeMask = 0;

	hr = Dev->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(_rtvHeaps.GetAddressOf()));
	if (FAILED(hr)) {
		return hr;
	}

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	hr = _swapChain->GetDesc(&swcDesc);

	//ハンドルのインクリメントサイズ取得
	_descHandleIncSize = Dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//ディスクリプタヒープの先頭を取得
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	//SRGBレンダーターゲットビュー設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (UINT i = 0; i < swcDesc.BufferCount; ++i)
	{
		hr = _swapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&_rtvBuffers[i]));

		//レンダ―ターゲットビューの作成
		Dev ->CreateRenderTargetView(_rtvBuffers[i].Get(), &rtvDesc, rtvHandle);

		//先頭からポインタを進める
		rtvHandle.ptr += _descHandleIncSize;
	}

	return hr;
}

//深度バッファ作成
HRESULT D3D12Manager::CreateDepthStencilBuffer() {
	HRESULT hr;

	//深度バッファの作成
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Width = _width;
	resourceDesc.Height = _height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//深度値ヒーププロパティ
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//クリアバリュー
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT; // 32ビットfloat値でクリア
	clearValue.DepthStencil.Depth = 1.0f; // 深さ1.0fでクリア
	
	ID3D12Resource* depthBuffer = nullptr;
	hr = Dev->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&depthBuffer));

	//深度バッファ用のデスクリプタヒープの作成
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.NumDescriptors = 1;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = Dev->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&_dsvHeaps));
	if (FAILED(hr)) {
		return hr;
	}

	//深度バッファのビューの作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; //深度に32ビット
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	//ディスクリプタヒープの先頭を取得
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = _dsvHeaps->GetCPUDescriptorHandleForHeapStart();

	Dev->CreateDepthStencilView(depthBuffer, &dsvDesc, dsvHandle);

	return hr;
}

//ルートシグネチャ(ディスクリプターテーブルを管理するもの)を作る
HRESULT D3D12Manager::CreateRootSignature() {
	HRESULT hr{};

	ComPtr<ID3DBlob> errorBlob = nullptr;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; //頂点情報の列挙がある事を伝える

	//ディスクリプタレンジ
	D3D12_DESCRIPTOR_RANGE descTblRange[3] = {}; //テクスチャと定数の2つ

	descTblRange[0].NumDescriptors = 1;//定数ひとつ
	descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;//種別は定数
	descTblRange[0].BaseShaderRegister = 0;//0番スロットから
	descTblRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//マテリアル用
	descTblRange[1].NumDescriptors = 1;//テクスチャひとつ
	descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;//種別は定数
	descTblRange[1].BaseShaderRegister = 1;//1番スロットから
	descTblRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//テクスチャ1つ目
	descTblRange[2].NumDescriptors = 1;
	descTblRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//種別はテクスチャ
	descTblRange[2].BaseShaderRegister = 0;
	descTblRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//ルートパラメーター
	D3D12_ROOT_PARAMETER rootparam[2] = {};
	rootparam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[0].DescriptorTable.pDescriptorRanges = &descTblRange[0];//デスクリプタレンジのアドレス
	rootparam[0].DescriptorTable.NumDescriptorRanges = 1;//デスクリプタレンジ数
	rootparam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;//全シェーダから見える

	rootparam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[1].DescriptorTable.pDescriptorRanges = &descTblRange[1];//デスクリプタレンジのアドレス
	rootparam[1].DescriptorTable.NumDescriptorRanges = 2;//デスクリプタレンジ数
	rootparam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダから見える

	rootSignatureDesc.pParameters = rootparam;//ルートパラメータの先頭アドレス
	rootSignatureDesc.NumParameters = 2;//ルートパラメータ数

	//サンプラの指定
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//横繰り返し
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//縦繰り返し
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//奥行繰り返し
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//ボーダーの時は黒
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//補間しない(ニアレストネイバー)
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;//ミップマップ最大値
	samplerDesc.MinLOD = 0.0f;//ミップマップ最小値
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//オーバーサンプリングの際リサンプリングしない？
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダからのみ可視

	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	ID3DBlob* rootSigBlob = nullptr;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	hr = Dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature));
	rootSigBlob->Release();

	return hr;
}

void D3D12Manager::CopyTexture() {
	//GPUに転送
	CommandList->CopyTextureRegion(_tex->dst, 0, 0, 0, _tex->src, nullptr);

	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = _tex->TexBuffer.Get();
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	CommandList->ResourceBarrier(1, &BarrierDesc);
	CommandList->Close();

	//コマンドリストの実行
	ID3D12CommandList* cmdlists[] = { CommandList.Get() };
	_commandQueue->ExecuteCommandLists(1, cmdlists);
	WaitForPreviousFrame();
	//リセット
	ResetCommand(nullptr);
}

//パイプラインステートオブジェクト
//重要。後にリファクタするならここを色々分けたい
HRESULT D3D12Manager::CreatePipelineStateObject(LPCWSTR vertexShaderName, LPCWSTR pixelShaderName) {
	HRESULT hr{};

	ComPtr<ID3DBlob> vertexShader{};
	ComPtr<ID3DBlob> pixelShader{};
	ComPtr<ID3DBlob> errorBlob = nullptr;

#if defined(_DEBUG)
	UINT compile_flag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compile_flag = 0;
#endif

	//頂点シェーダーのコンパイル
	hr = D3DCompileFromFile(
		vertexShaderName, //シェーダー名
		nullptr, //マクロオブジェクト
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //includeのオブジェクト
		"VSMain", //シェーダーのメイン関数
		"vs_5_0", //シェーダーの種類
		compile_flag, //コンパイル設定
		0, //受け取り用のポインタアドレス(シェーダーは0)
		vertexShader.GetAddressOf(), //vsシェーダーのアドレス
		errorBlob.GetAddressOf() //エラー用アドレス
	);
	if (FAILED(hr)) {
		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("ファイルが見当たりません");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		return hr;
	}

	//ピクセルシェーダーのコンパイル
	hr = D3DCompileFromFile(
		pixelShaderName,
		nullptr, 
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"PSMain",
		"ps_5_0",
		compile_flag,
		0,
		pixelShader.GetAddressOf(),
		errorBlob.GetAddressOf()
	);
	if (FAILED(hr)) {
		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("ファイルが見当たりません");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		return hr;
	}

	//パイプラインステート
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	
	//ルートシグネチャ
	gpipeline.pRootSignature = _rootSignature.Get();
	
	//シェーダー
	gpipeline.VS.pShaderBytecode = vertexShader->GetBufferPointer();
	gpipeline.VS.BytecodeLength = vertexShader->GetBufferSize();
	gpipeline.PS.pShaderBytecode = pixelShader->GetBufferPointer();
	gpipeline.PS.BytecodeLength = pixelShader->GetBufferSize();

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//中身は0xffffffff
	gpipeline.BlendState.AlphaToCoverageEnable = false; //αテストしない
	gpipeline.BlendState.IndependentBlendEnable = false; //レンダーターゲットのBlendStateの割り当てを統一

	//ブレンドの指定 ブレンドするならここで設定しておく
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	//ひとまず加算や乗算やαブレンディングは使用しない
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	//ひとまず論理演算は使用しない
	renderTargetBlendDesc.LogicOpEnable = false;
	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	//ラスタライザ設定
	gpipeline.RasterizerState.MultisampleEnable = false;//まだアンチェリは使わない
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//カリングしない
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//中身を塗りつぶす
	gpipeline.RasterizerState.DepthClipEnable = true;//深度方向のクリッピングは有効に
	gpipeline.RasterizerState.FrontCounterClockwise = false;
	gpipeline.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	gpipeline.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	gpipeline.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	gpipeline.RasterizerState.AntialiasedLineEnable = false;
	gpipeline.RasterizerState.ForcedSampleCount = 0;
	gpipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	//デプスステンシルの指定
	gpipeline.DepthStencilState.DepthEnable = true; //デプステストあり
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	gpipeline.DepthStencilState.StencilEnable = false; //ステンシルテスト無し
	gpipeline.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	gpipeline.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

	//インプットレイアウトの設定
	gpipeline.InputLayout.pInputElementDescs = _model->InputLayout; //レイアウト先頭アドレス
	gpipeline.InputLayout.NumElements = _countof(_model->InputLayout); //レイアウト配列数
	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//ストリップ時のカットなし
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//三角形で構成

	//レンダーターゲットの設定
	gpipeline.NumRenderTargets = 1;//今は１つのみ
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//0〜1に正規化されたRGBA

	//サンプル設定
	gpipeline.SampleDesc.Count = 1;//サンプリングは1ピクセルにつき１
	gpipeline.SampleDesc.Quality = 0;//クオリティは最低

	//パイプライン作成
	hr = Dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelineState));

	return hr;
}

//ビューポートとシザー矩形
void D3D12Manager::CreateViewPortScissorRect() {
	_viewPort = {};
	_viewPort.Width = _width;//出力先の幅(ピクセル数)
	_viewPort.Height = _height;//出力先の高さ(ピクセル数)
	_viewPort.TopLeftX = 0;//出力先の左上座標X
	_viewPort.TopLeftY = 0;//出力先の左上座標Y
	_viewPort.MaxDepth = 1.0f;//深度最大値
	_viewPort.MinDepth = 0.0f;//深度最小値


	_scissorRect = {};
	_scissorRect.top = 0;//切り抜き上座標
	_scissorRect.left = 0;//切り抜き左座標
	_scissorRect.right = _scissorRect.left + _width;//切り抜き右座標
	_scissorRect.bottom = _scissorRect.top + _height;//切り抜き下座標
}

//描画命令を積む
HRESULT D3D12Manager::StackDrawCommandList() {
	HRESULT hr;

	// バックバッファのインデックス取得
	auto rtvBufferIndex = _swapChain->GetCurrentBackBufferIndex();

	//リソースバリアの指定(Present→Render)
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(_rtvBuffers[rtvBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandList->ResourceBarrier(1, &barrier);

	//パイプラインセット
	CommandList->SetPipelineState(_pipelineState.Get());

	//レンダ―ターゲットの設定
	auto rtvH = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	auto dsvH = _dsvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += rtvBufferIndex * _descHandleIncSize;
	CommandList->OMSetRenderTargets(1, &rtvH, TRUE, &dsvH);

	//深度バッファとレンダーターゲットのクリア
	CommandList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	CommandList->ClearRenderTargetView(rtvH, CLEAR_COLOR, 0, nullptr);

	//ルートシグネチャセット
	CommandList->SetGraphicsRootSignature(_rootSignature.Get());

	//ViewPort、シザー矩形
	CommandList->RSSetViewports(1, &_viewPort);
	CommandList->RSSetScissorRects(1, &_scissorRect);

	//リソース関連の紐づけ
	CommandList->SetGraphicsRootSignature(_rootSignature.Get());

	//定数系(WMP行列,Manager側で作ったHeapを使いまわしたもの)
	CommandList->SetDescriptorHeaps(1, _resourceHeaps.GetAddressOf());
	CommandList->SetGraphicsRootDescriptorTable(0, _resourceHeaps.Get()->GetGPUDescriptorHandleForHeapStart());

	//描画コマンド
	_model->Draw(Dev, CommandList);
	//CommandList->DrawIndexedInstanced(_model->VertNum, 1, 0, 0, 0);

	//リソースバリアの指定(Render→Present)
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(_rtvBuffers[rtvBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	CommandList->ResourceBarrier(1, &barrier);

	//コマンドリストのクローズ
	hr = CommandList->Close();

	return hr;
}

HRESULT D3D12Manager::ResetCommand(ComPtr<ID3D12PipelineState> pipelineState) {
	HRESULT hr;
	//アロケーターのリセット
	hr = _commandAllocator->Reset();
	if (FAILED(hr)) {
		return hr;
	}
	//コマンドリストのリセット
	if (pipelineState != nullptr) {
		hr = CommandList->Reset(_commandAllocator.Get(), pipelineState.Get());
	}
	else
	{
		hr = CommandList->Reset(_commandAllocator.Get(), nullptr);
	}
	//hr = _commandList->Reset(_commandAllocator, pipeline_state_.Get());
	if (FAILED(hr)) {
		return hr;
	}
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

//描画メイン
HRESULT D3D12Manager::Render() {
	HRESULT hr;

	//行列の回転アニメーション
	_matrix->Rotate();

	//コマンドリスト準備
	StackDrawCommandList();

	//コマンドリスト実行
	ID3D12CommandList* commandLists = CommandList.Get();
	_commandQueue->ExecuteCommandLists(1, &commandLists);

	//実行したらリセットしていい(Populateでも一応リセットしてるが)
	//_commandAllocator->Reset();
	//_commandList->Reset(_commandAllocator.Get(), nullptr);

	//実行したコマンドの終了待ち
	WaitForPreviousFrame();

	//フリップ処理
	hr = _swapChain->Present(1, 0);

	hr = ResetCommand(_pipelineState);

	if (FAILED(hr)) {
		return hr;
	}

	//カレントのバックバッファのインデックスを取得する
	//rtv_index_ = swap_chain_->GetCurrentBackBufferIndex();

	//同期処理
	/*command_queue_->Signal(queue_fence_.Get(), frames_);
	while (queue_fence_->GetCompletedValue() < frames_);
	frames_++;
	*/
	return S_OK;
}