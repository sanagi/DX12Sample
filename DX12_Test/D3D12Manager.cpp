#include "D3D12Manager.h"

//�R���X�g���N�^
D3D12Manager::D3D12Manager(HWND hwnd, int width, int height, LPCWSTR vertexShaderName, LPCWSTR pixelShaderName) : _windowHandle(hwnd), _width(width), _height(height){
	
	//�f�o�C�X�̏�����
	CreateFactory();
	CreateDevice();

	//�R�}���h���X�g
	CreateCommandList();
	//�R�}���h�L���[
	CreateCommandQueue();
	//�t�F���X
	CreateFence();
	//�X���b�v�`�F�C��
	CreateSwapChain();
	//�����_�\�^�[�Q�b�g�r���[
	CreateRenderTargetView();
	//�[�x�o�b�t�@�쐬
	CreateDepthStencilBuffer();

	//���[�g�V�O�l�`��
	CreateRootSignature();
	//�p�C�v���C���X�e�[�g
	CreatePipelineStateObject(vertexShaderName, pixelShaderName);

	//�r���[�|�[�g�ƃV�U�[��`
	CreateViewPortScissorRect();

	//�`��I�u�W�F�N�g���
	//�ǂ����ʂł�肽��
	_vert = new Vertices(Dev);
}

D3D12Manager::~D3D12Manager() {}

//�f�o�C�X�̐���
HRESULT D3D12Manager::CreateDevice() {
	
	//�ǂ̃o�[�W������
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	IDXGIAdapter* warpAdapter;
	_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));


	//�f�o�C�X�쐬
	HRESULT hr{};
	//D3D_FEATURE_LEVEL featureLevel;
	for (auto lv : levels) {
		if (D3D12CreateDevice(warpAdapter, lv, IID_PPV_ARGS(&Dev)) == S_OK) {
			//featureLevel = lv;
			return S_OK;
		}
		
	}

	//�f�o�C�X���������A�_�v�^�v��Ȃ�
	warpAdapter->Release();

	return S_FALSE;
}

//�t�@�N�g���̍쐬
HRESULT D3D12Manager::CreateFactory() {
	HRESULT hr{};
	UINT flag{};

	//�f�o�b�O���[�h�̏ꍇ�̓f�o�b�O���C���[��L���ɂ���
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

	//�t�@�N�g���̍쐬
	HRESULT factoryResult = CreateDXGIFactory2(flag, IID_PPV_ARGS(_dxgiFactory.GetAddressOf()));

	return hr;
}

//�R�}���h���X�g�쐬
HRESULT D3D12Manager::CreateCommandList() {
	HRESULT hr;

	//�R�}���h�A���P�[�^�̍쐬
	hr = Dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator));
	if (FAILED(hr)) {
		return hr;
	}

	//�R�}���h�A���P�[�^�ƃo�C���h���ăR�}���h���X�g���쐬����
	hr = Dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator.Get(), nullptr, IID_PPV_ARGS(&CommandList));
	CommandList->Close();

	return hr;
}

//�R�}���h�L���[�𐶐�
HRESULT D3D12Manager::CreateCommandQueue() {
	HRESULT hr{};
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

	//�^�C���A�E�g�Ȃ�
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	//�A�_�v�^�[���g��Ȃ�����0
	commandQueueDesc.NodeMask = 0;
	//�R�}���h���X�g�ƍ��킹��
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//�D��x0
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	
	hr = Dev->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(_commandQueue.GetAddressOf()));
	if (FAILED(hr)) {
		return hr;
	}

	return hr;
}

//�R�}���h�L���[�ɗ��p����t�F���X�쐬
HRESULT D3D12Manager::CreateFence() {
	HRESULT hr{};

	//�R�}���h�L���[�p�̃t�F���X�̐���
	hr = Dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_queueFence.GetAddressOf()));

	return hr;
}

//�X���b�v�`�F�C���̍쐬
HRESULT D3D12Manager::CreateSwapChain() {
	HRESULT hr{};
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	swapChainDesc.Width = _width;
	swapChainDesc.Height = _height;
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = RTV_NUM;//���ƕ\�łQ����肽���̂�2���w�肷��

	//�X�P�[�����O�\
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;

	//�t���b�v�㑦�j��
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	//�w��Ȃ�
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	//�؂�ւ��\
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	hr = _dxgiFactory->CreateSwapChainForHwnd(_commandQueue.Get(), _windowHandle, &swapChainDesc, NULL, NULL, (IDXGISwapChain1**)_swapChain.GetAddressOf());
	if (FAILED(hr)) {
		return hr;
	}

	return S_OK;
}

//�����_�[�^�[�Q�b�g�r���[�����
HRESULT D3D12Manager::CreateRenderTargetView() {
	HRESULT hr{};
	D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};

	//RTV�p�f�X�N���v�^�q�[�v�̍쐬
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

	//�n���h���̃C���N�������g�T�C�Y�擾
	_descHandleIncSize = Dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//�f�B�X�N���v�^�q�[�v�̐擪���擾
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < swcDesc.BufferCount; ++i)
	{
		hr = _swapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&_rtvBuffers[i]));

		//�����_�\�^�[�Q�b�g�r���[�̍쐬
		Dev ->CreateRenderTargetView(_rtvBuffers[i].Get(), NULL, rtvHandle);

		//�擪����|�C���^��i�߂�
		rtvHandle.ptr += _descHandleIncSize;
	}

	return hr;
}

//�[�x�o�b�t�@�쐬
HRESULT D3D12Manager::CreateDepthStencilBuffer() {
	HRESULT hr;

	//�[�x�o�b�t�@�̍쐬
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Width = _width;
	resourceDesc.Height = _height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//�[�x�l�q�[�v�v���p�e�B
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//�N���A�o�����[
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT; // 32�r�b�gfloat�l�ŃN���A
	clearValue.DepthStencil.Depth = 1.0f; // �[��1.0f�ŃN���A
	
	ID3D12Resource* depthBuffer = nullptr;
	hr = Dev->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&depthBuffer));

	//�[�x�o�b�t�@�p�̃f�X�N���v�^�q�[�v�̍쐬
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.NumDescriptors = 1;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = Dev->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&_dsvHeaps));
	if (FAILED(hr)) {
		return hr;
	}

	//�[�x�o�b�t�@�̃r���[�̍쐬
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; //�[�x��32�r�b�g
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	//�f�B�X�N���v�^�q�[�v�̐擪���擾
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = _dsvHeaps->GetCPUDescriptorHandleForHeapStart();

	Dev->CreateDepthStencilView(depthBuffer, &dsvDesc, dsvHandle);

	return hr;
}

//���[�g�V�O�l�`��(�f�B�X�N���v�^�[�e�[�u�����Ǘ��������)�����
HRESULT D3D12Manager::CreateRootSignature() {
	HRESULT hr{};

	ComPtr<ID3DBlob> errorBlob = nullptr;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; //���_���̗񋓂����鎖��`����

	ID3DBlob* rootSigBlob = nullptr;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	hr = Dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature));
	rootSigBlob->Release();

	return hr;
}

//�p�C�v���C���X�e�[�g�I�u�W�F�N�g
//�d�v�B��Ƀ��t�@�N�^����Ȃ炱����F�X��������
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

	//���_�V�F�[�_�[�̃R���p�C��
	hr = D3DCompileFromFile(
		vertexShaderName, //�V�F�[�_�[��
		nullptr, //�}�N���I�u�W�F�N�g
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //include�̃I�u�W�F�N�g
		"VSMain", //�V�F�[�_�[�̃��C���֐�
		"vs_5_0", //�V�F�[�_�[�̎��
		compile_flag, //�R���p�C���ݒ�
		0, //�󂯎��p�̃|�C���^�A�h���X(�V�F�[�_�[��0)
		vertexShader.GetAddressOf(), //vs�V�F�[�_�[�̃A�h���X
		errorBlob.GetAddressOf() //�G���[�p�A�h���X
	);
	if (FAILED(hr)) {
		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("�t�@�C������������܂���");
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

	//�s�N�Z���V�F�[�_�[�̃R���p�C��
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
			::OutputDebugStringA("�t�@�C������������܂���");
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

	//���_�V�F�[�_�[�ɕ`����Ă�f�[�^�̎w��
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ 
			"POSITION", //�Z�}���e�B�N�X
			0, //�Z�}���e�B�N�X�̃C���f�b�N�X
			DXGI_FORMAT_R32G32B32_FLOAT, //�t�H�[�}�b�g�w��
			0, //���̓X���b�g�C���f�b�N�X
			D3D12_APPEND_ALIGNED_ELEMENT, //�f�[�^�̃I�t�Z�b�g�ʒu
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, //InputSlotClass
			0 //��x�ɕ`�悷��C���X�^���X��
		},
		{ 
			"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,
			0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 
		},
	};

	//�p�C�v���C���X�e�[�g
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	
	//���[�g�V�O�l�`��
	gpipeline.pRootSignature = _rootSignature.Get();
	
	//�V�F�[�_�[
	gpipeline.VS.pShaderBytecode = vertexShader->GetBufferPointer();
	gpipeline.VS.BytecodeLength = vertexShader->GetBufferSize();
	gpipeline.PS.pShaderBytecode = pixelShader->GetBufferPointer();
	gpipeline.PS.BytecodeLength = pixelShader->GetBufferSize();

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//���g��0xffffffff
	gpipeline.BlendState.AlphaToCoverageEnable = false; //���e�X�g���Ȃ�
	gpipeline.BlendState.IndependentBlendEnable = false; //�����_�[�^�[�Q�b�g��BlendState�̊��蓖�Ă𓝈�

	//�u�����h�̎w�� �u�����h����Ȃ炱���Őݒ肵�Ă���
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	//�ЂƂ܂����Z���Z�⃿�u�����f�B���O�͎g�p���Ȃ�
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	//�ЂƂ܂��_�����Z�͎g�p���Ȃ�
	renderTargetBlendDesc.LogicOpEnable = false;
	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	//���X�^���C�U�ݒ�
	gpipeline.RasterizerState.MultisampleEnable = false;//�܂��A���`�F���͎g��Ȃ�
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//�J�����O���Ȃ�
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//���g��h��Ԃ�
	gpipeline.RasterizerState.DepthClipEnable = true;//�[�x�����̃N���b�s���O�͗L����
	gpipeline.RasterizerState.FrontCounterClockwise = false;
	gpipeline.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	gpipeline.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	gpipeline.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	gpipeline.RasterizerState.AntialiasedLineEnable = false;
	gpipeline.RasterizerState.ForcedSampleCount = 0;
	gpipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	//�f�v�X�X�e���V���̎w��
	gpipeline.DepthStencilState.DepthEnable = true; //�f�v�X�e�X�g����
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpipeline.DepthStencilState.StencilEnable = false; //�X�e���V���e�X�g����
	gpipeline.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	gpipeline.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

	//�C���v�b�g���C�A�E�g�̐ݒ�
	gpipeline.InputLayout.pInputElementDescs = inputLayout; //���C�A�E�g�擪�A�h���X
	gpipeline.InputLayout.NumElements = _countof(inputLayout); //���C�A�E�g�z��
	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//�X�g���b�v���̃J�b�g�Ȃ�
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//�O�p�`�ō\��

	//�����_�[�^�[�Q�b�g�̐ݒ�
	gpipeline.NumRenderTargets = 1;//���͂P�̂�
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//0�`1�ɐ��K�����ꂽRGBA

	//�T���v���ݒ�
	gpipeline.SampleDesc.Count = 1;//�T���v�����O��1�s�N�Z���ɂ��P
	gpipeline.SampleDesc.Quality = 0;//�N�I���e�B�͍Œ�

	//�p�C�v���C���쐬
	hr = Dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelineState));

	return hr;
}

//�r���[�|�[�g�ƃV�U�[��`
void D3D12Manager::CreateViewPortScissorRect() {
	_viewPort = {};
	_viewPort.Width = _width;//�o�͐�̕�(�s�N�Z����)
	_viewPort.Height = _height;//�o�͐�̍���(�s�N�Z����)
	_viewPort.TopLeftX = 0;//�o�͐�̍�����WX
	_viewPort.TopLeftY = 0;//�o�͐�̍�����WY
	_viewPort.MaxDepth = 1.0f;//�[�x�ő�l
	_viewPort.MinDepth = 0.0f;//�[�x�ŏ��l


	_scissorRect = {};
	_scissorRect.top = 0;//�؂蔲������W
	_scissorRect.left = 0;//�؂蔲�������W
	_scissorRect.right = _scissorRect.left + _width;//�؂蔲���E���W
	_scissorRect.bottom = _scissorRect.top + _height;//�؂蔲�������W
}

//�`�施�߂�ς�
HRESULT D3D12Manager::StackDrawCommandList() {
	HRESULT hr;

	// �o�b�N�o�b�t�@�̃C���f�b�N�X�擾
	auto rtvBufferIndex = _swapChain->GetCurrentBackBufferIndex();

	//���\�[�X�o���A�쐬
	D3D12_RESOURCE_BARRIER barrierDesc = {};
	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierDesc.Transition.pResource = _rtvBuffers[rtvBufferIndex].Get();
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	//���\�[�X�o���A�̎w��(Present��Render)
	CommandList->ResourceBarrier(1, &barrierDesc);

	//�p�C�v���C���Z�b�g
	CommandList->SetPipelineState(_pipelineState.Get());

	//�����_�\�^�[�Q�b�g�̐ݒ�
	auto rtvH = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	auto dsvH = _dsvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += rtvBufferIndex * _descHandleIncSize;
	CommandList->OMSetRenderTargets(1, &rtvH, TRUE, &dsvH);

	//�[�x�o�b�t�@�ƃ����_�[�^�[�Q�b�g�̃N���A
	CommandList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	CommandList->ClearRenderTargetView(rtvH, CLEAR_COLOR, 0, nullptr);

	//���[�g�V�O�l�`���Z�b�g
	CommandList->SetGraphicsRootSignature(_rootSignature.Get());

	//ViewPort�A�V�U�[��`
	CommandList->RSSetViewports(1, &_viewPort);
	CommandList->RSSetScissorRects(1, &_scissorRect);

	//���_���̃Z�b�g
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP); //�g�|���W�w��

	//�`��R�}���h
	_vert->Draw(CommandList);
	//draw();

	//���\�[�X�o���A�̎w��(Render��Present)
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	CommandList->ResourceBarrier(1, &barrierDesc);

	//�R�}���h���X�g�̃N���[�Y
	hr = CommandList->Close();

	return hr;
}

//�R�}���h�҂�
HRESULT D3D12Manager::WaitForPreviousFrame() {
	HRESULT hr;
	UINT64 fenceValue = 0;

	//CPU����fence�l�̃C���N�������g
	hr = _commandQueue->Signal(_queueFence.Get(), ++fenceValue);
	if (FAILED(hr)) {
		return -1;
	}

	//GPU�������I������Ƃ��ɍX�V�����l��CPU���ŃC���N�������g�����l�łȂ��Ȃ�҂��C�x���g���J��Ԃ�
	if (_queueFence->GetCompletedValue() != fenceValue) {
		HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		//fenceValue�ɂȂ�����C�x���g�𔭐�������
		hr = _queueFence->SetEventOnCompletion(fenceValue, fenceEvent);
		if (FAILED(hr)) {
			return -1;
		}

		//�C�x���g����������܂ő҂�
		WaitForSingleObject(fenceEvent, INFINITE);

		CloseHandle(fenceEvent);
	}
	return S_OK;
}

//�`�惁�C��
HRESULT D3D12Manager::Render() {
	HRESULT hr;

	//�A���P�[�^�[�̃��Z�b�g
	hr = _commandAllocator->Reset();
	if (FAILED(hr)) {
		return hr;
	}
	//�R�}���h���X�g�̃��Z�b�g
	hr = CommandList->Reset(_commandAllocator.Get(), nullptr);
	//hr = _commandList->Reset(_commandAllocator, pipeline_state_.Get());
	if (FAILED(hr)) {
		return hr;
	}

	//�R�}���h���X�g����
	StackDrawCommandList();

	//�R�}���h���X�g���s
	ID3D12CommandList* commandLists = CommandList.Get();
	_commandQueue->ExecuteCommandLists(1, &commandLists);

	//���s�����烊�Z�b�g���Ă���(Populate�ł��ꉞ���Z�b�g���Ă邪)
	//_commandAllocator->Reset();
	//_commandList->Reset(_commandAllocator.Get(), nullptr);

	//���s�����R�}���h�̏I���҂�
	WaitForPreviousFrame();

	//�t���b�v����
	hr = _swapChain->Present(1, 0);
	if (FAILED(hr)) {
		return hr;
	}

	//�J�����g�̃o�b�N�o�b�t�@�̃C���f�b�N�X���擾����
	//rtv_index_ = swap_chain_->GetCurrentBackBufferIndex();

	//��������
	/*command_queue_->Signal(queue_fence_.Get(), frames_);
	while (queue_fence_->GetCompletedValue() < frames_);
	frames_++;
	*/
	return S_OK;
}

/*/�`��Callback
void D3D12Manager::DrawCallback(std::function<void(ID3D12GraphicsCommandList*)> draw) {
	draw(_commandList.Get());
}*/