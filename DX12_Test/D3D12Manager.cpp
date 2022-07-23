#include "D3D12Manager.h"
#include"Application.h"

#pragma region �R���X�g���N�^�E�f�X�g���N�^

/// <summary>
/// �R���X�g���N�^
/// </summary>
/// <param name="hwnd"></param>
D3D12Manager::D3D12Manager(HWND hwnd) : _windowHandle(hwnd) {

	auto& app = Application::Instance();
	_winSize = app.GetWindowSize();

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
	CreateFinalRenderTargetView();
	//�[�x�o�b�t�@�쐬
	CreateDepthStencilBuffer();

	//���\�[�X�p�q�[�v
	CreateScene();
}

D3D12Manager::~D3D12Manager() {}

#pragma endregion

#pragma region ������

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
		if (D3D12CreateDevice(warpAdapter, lv, IID_PPV_ARGS(&_device)) == S_OK) {
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

#pragma endregion

#pragma region �R�}���h���X�g

//�R�}���h���X�g�쐬
HRESULT D3D12Manager::CreateCommandList() {
	HRESULT hr;

	//�R�}���h�A���P�[�^�̍쐬
	hr = _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator));
	if (FAILED(hr)) {
		return hr;
	}

	//�R�}���h�A���P�[�^�ƃo�C���h���ăR�}���h���X�g���쐬����
	hr = _device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator.Get(), nullptr, IID_PPV_ARGS(&_commandList));

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

	hr = _device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(_commandQueue.GetAddressOf()));
	if (FAILED(hr)) {
		return hr;
	}

	return hr;
}

#pragma endregion


#pragma region �t�F���X

//�R�}���h�L���[�ɗ��p����t�F���X�쐬
HRESULT D3D12Manager::CreateFence() {
	HRESULT hr{};

	//�R�}���h�L���[�p�̃t�F���X�̐���
	hr = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_queueFence.GetAddressOf()));

	return hr;
}

//�R�}���h�҂�
HRESULT D3D12Manager::WaitForPreviousFrame() {
	HRESULT hr;

	//CPU����fence�l�̃C���N�������g
	hr = _commandQueue->Signal(_queueFence.Get(), ++_fenceValue);
	if (FAILED(hr)) {
		return -1;
	}

	//GPU�������I������Ƃ��ɍX�V�����l��CPU���ŃC���N�������g�����l�łȂ��Ȃ�҂��C�x���g���J��Ԃ�
	if (_queueFence->GetCompletedValue() != _fenceValue) {
		HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		//fenceValue�ɂȂ�����C�x���g�𔭐�������
		hr = _queueFence->SetEventOnCompletion(_fenceValue, fenceEvent);
		if (FAILED(hr)) {
			return -1;
		}

		//�C�x���g����������܂ő҂�
		WaitForSingleObject(fenceEvent, INFINITE);

		CloseHandle(fenceEvent);
	}
	return S_OK;
}

#pragma endregion

#pragma region �X���b�v�`�F�C��

//�X���b�v�`�F�C���̍쐬
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

#pragma endregion

#pragma region �V�[���쐬

void D3D12Manager::CreateScene() {
	//_sceneMatrix = new Matrix();
	//_sceneMatrix->CreateSceneView(_device, _winSize.cx, _winSize.cy);
	//return Matrix::CreateSceneView(_device, _winSize.cx, _winSize.cy, _mappedSceneData, _sceneDescHeap);
	CreateSceneView();
}

//�r���[�v���W�F�N�V�����p�r���[�̐���
HRESULT D3D12Manager::CreateSceneView() {
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	auto result = _swapChain->GetDesc1(&desc);
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneData) + 0xff) & ~0xff);
	//�萔�o�b�t�@�쐬
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

	_mappedSceneData = nullptr;//�}�b�v��������|�C���^
	result = _sceneConstBuff->Map(0, nullptr, (void**)&_mappedSceneData);//�}�b�v

	XMFLOAT3 eye(0, 15, -15);
	XMFLOAT3 target(0, 15, 0);
	XMFLOAT3 up(0, 1, 0);
	_mappedSceneData->view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	_mappedSceneData->proj = XMMatrixPerspectiveFovLH(XM_PIDIV4,//��p��45��
		static_cast<float>(desc.Width) / static_cast<float>(desc.Height),//�A�X��
		0.1f,//�߂���
		1000.0f//������
	);
	_mappedSceneData->eye = eye;

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//�V�F�[�_���猩����悤��
	descHeapDesc.NodeMask = 0;//�}�X�N��0
	descHeapDesc.NumDescriptors = 1;//
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//�f�X�N���v�^�q�[�v���
	result = _device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(_sceneDescHeap.ReleaseAndGetAddressOf()));//����

	////�f�X�N���v�^�̐擪�n���h�����擾���Ă���
	auto heapHandle = _sceneDescHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _sceneConstBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = _sceneConstBuff->GetDesc().Width;
	//�萔�o�b�t�@�r���[�̍쐬
	_device->CreateConstantBufferView(&cbvDesc, heapHandle);
	return result;

}

#pragma endregion

#pragma region �����_�[�^�[�Q�b�g

//�����_�[�^�[�Q�b�g�r���[�����
HRESULT D3D12Manager::CreateFinalRenderTargetView() {
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	auto result = _swapChain->GetDesc1(&desc);

	HRESULT hr{};
	D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};

	//RTV�p�f�X�N���v�^�q�[�v�̍쐬
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

	//�f�B�X�N���v�^�q�[�v�̐擪���擾
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	//SRGB�����_�[�^�[�Q�b�g�r���[�ݒ�
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (UINT i = 0; i < swcDesc.BufferCount; ++i)
	{
		hr = _swapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&_backBuffers[i]));
		
		rtvDesc.Format = _backBuffers[i]->GetDesc().Format;
		
		//�����_�\�^�[�Q�b�g�r���[�̍쐬
		_device->CreateRenderTargetView(_backBuffers[i], &rtvDesc, rtvHandle);

		//�擪����|�C���^��i�߂�
		rtvHandle.ptr += _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	//�r���[�|�[�g��Rect�쐬
	_viewPort.reset(new CD3DX12_VIEWPORT(_backBuffers[0]));
	_scissorRect.reset(new CD3DX12_RECT(0, 0, desc.Width, desc.Height));

	return hr;
}

//�[�x�o�b�t�@�쐬
HRESULT D3D12Manager::CreateDepthStencilBuffer() {
	HRESULT hr;

	DXGI_SWAP_CHAIN_DESC1 desc = {};
	auto result = _swapChain->GetDesc1(&desc);

	//�[�x�o�b�t�@�̍쐬
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

	//�[�x�l�q�[�v�v���p�e�B
	auto depthHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	//�N���A�o�����[
	CD3DX12_CLEAR_VALUE depthClearValue(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);


	ID3D12Resource* depthBuffer = nullptr;
	hr = _device->CreateCommittedResource(&depthHeapProp, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&depthBuffer));

	//�[�x�o�b�t�@�p�̃f�X�N���v�^�q�[�v�̍쐬
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.NumDescriptors = 1;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = _device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&_dsvHeaps));
	if (FAILED(hr)) {
		return hr;
	}

	//�[�x�o�b�t�@�̃r���[�̍쐬
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; //�[�x��32�r�b�g
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	_device->CreateDepthStencilView(depthBuffer, &dsvDesc, _dsvHeaps->GetCPUDescriptorHandleForHeapStart());

	return hr;
}

#pragma endregion

#pragma region �`��

/// <summary>
/// �`��O�̏���
/// </summary>
void D3D12Manager::BeginDraw() {
	//DirectX����
	//�o�b�N�o�b�t�@�̃C���f�b�N�X���擾
	auto backBufferIndex = _swapChain->GetCurrentBackBufferIndex();
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(_backBuffers[backBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	_commandList->ResourceBarrier(1, &barrier);


	//�����_�[�^�[�Q�b�g���w��
	auto rtvH = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += backBufferIndex * _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//�[�x���w��
	auto dsvH = _dsvHeaps->GetCPUDescriptorHandleForHeapStart();
	_commandList->OMSetRenderTargets(1, &rtvH, false, &dsvH);
	_commandList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


	//��ʃN���A
	float clearColor[] = { 1.0f,1.0f,1.0f,1.0f };//���F
	_commandList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	//�r���[�|�[�g�A�V�U�[��`�̃Z�b�g
	_commandList->RSSetViewports(1, _viewPort.get());
	_commandList->RSSetScissorRects(1, _scissorRect.get());
}

/// <summary>
/// Render����҂̎w��
/// </summary>
/// <param name="pipelineState"></param>
/// <param name="rootSignature"></param>
/// <param name="primitivTopology"></param>
void D3D12Manager::SetRenderer(ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootSignature) {
	//PMD�p�̕`��p�C�v���C���ɍ��킹��
	_commandList->SetPipelineState(pipelineState);
	//���[�g�V�O�l�`����PMD�p�ɍ��킹��
	_commandList->SetGraphicsRootSignature(rootSignature);
}

/// <summary>
/// ���݂̃V�[��(�r���[�v���W�F�N�V����)���Z�b�g
/// </summary>
void D3D12Manager::SetScene() {
	//ID3D12DescriptorHeap* sceneheaps[] = { _sceneMatrix->GetSceneDescHeap().Get() };
	//_commandList->SetDescriptorHeaps(1, sceneheaps);
	//_commandList->SetGraphicsRootDescriptorTable(0, _sceneMatrix->GetSceneDescHeap()->GetGPUDescriptorHandleForHeapStart());
	//���݂̃V�[��(�r���[�v���W�F�N�V����)���Z�b�g
	ID3D12DescriptorHeap* sceneheaps[] = { _sceneDescHeap.Get() };
	_commandList->SetDescriptorHeaps(1, sceneheaps);
	_commandList->SetGraphicsRootDescriptorTable(0, _sceneDescHeap->GetGPUDescriptorHandleForHeapStart());
}

/// <summary>
/// Draw�̌㏈��
/// </summary>
void D3D12Manager::EndDraw() {
	auto backBufferIndex = _swapChain->GetCurrentBackBufferIndex();
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(_backBuffers[backBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	_commandList->ResourceBarrier(1, &barrier);

	//���߂̃N���[�Y
	_commandList->Close();

	//�R�}���h���X�g�̎��s
	ID3D12CommandList* cmdlists[] = { _commandList.Get() };
	_commandQueue->ExecuteCommandLists(1, cmdlists);
	//�҂�
	WaitForPreviousFrame();

	//���Z�b�g
	ResetCommand();

	//�t���b�v����
	_swapChain->Present(1, 0);
}

/// <summary>
/// ���Z�b�g
/// </summary>
/// <returns></returns>
HRESULT D3D12Manager::ResetCommand() {
	HRESULT hr;
	//�A���P�[�^�[�̃��Z�b�g
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

#pragma region �Q�b�^�[

ComPtr<ID3D12Device> D3D12Manager::GetDevice() {
	return _device;
}

ComPtr<ID3D12GraphicsCommandList> D3D12Manager::GetCommandList() {
	return _commandList;
}

#pragma endregion
