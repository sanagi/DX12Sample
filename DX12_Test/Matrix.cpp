#include "Matrix.h"

#pragma region コンストラクタ系

Matrix::Matrix() {
	_mappedSceneData = nullptr;
	_mappedTransformData = nullptr;
}

Matrix::~Matrix() {
	_mappedSceneData = nullptr;
	_mappedTransformData = nullptr;
}

#pragma endregion


//ビュープロジェクション用ビューの生成
HRESULT Matrix::CreateSceneView(ComPtr<ID3D12Device> device, int width, int height) {
	ComPtr<ID3D12Resource> sceneConstBuff;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneData) + 0xff) & ~0xff);
	//定数バッファ作成
	auto result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(sceneConstBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	_mappedSceneData = nullptr;//マップ先を示すポインタ
	result = sceneConstBuff->Map(0, nullptr, (void**)&_mappedSceneData);//マップ

	XMFLOAT3 eye(0, 10, -15);
	XMFLOAT3 target(0, 10, 0);
	XMFLOAT3 up(0, 1, 0);
	_mappedSceneData->view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	_mappedSceneData->proj = XMMatrixPerspectiveFovLH(XM_PIDIV4,//画角は45°
		static_cast<float>(width) / static_cast<float>(height),//アス比
		0.1f,//近い方
		1000.0f//遠い方
	);
	_mappedSceneData->eye = eye;

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
	descHeapDesc.NodeMask = 0;//マスクは0
	descHeapDesc.NumDescriptors = 1;//
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//デスクリプタヒープ種別
	result = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(_sceneDescHeap.ReleaseAndGetAddressOf()));//生成

	////デスクリプタの先頭ハンドルを取得しておく
	auto heapHandle = _sceneDescHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = sceneConstBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = sceneConstBuff->GetDesc().Width;
	//定数バッファビューの作成
	device->CreateConstantBufferView(&cbvDesc, heapHandle);
	return result;
}

HRESULT Matrix::CreateTransformView(ComPtr<ID3D12Device> device) {
	ComPtr<ID3D12Resource> transformBuff;
	//GPUバッファ作成
	auto buffSize = sizeof(Transform);
	buffSize = (buffSize + 0xff) & ~0xff;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(buffSize);

	auto result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(transformBuff.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	//マップとコピー
	result = transformBuff->Map(0, nullptr, (void**)&_mappedTransformData);
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	//ビューの作成


	D3D12_DESCRIPTOR_HEAP_DESC transformDescHeapDesc = {};
	transformDescHeapDesc.NumDescriptors = 1;//とりあえずワールドひとつ
	transformDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	transformDescHeapDesc.NodeMask = 0;

	transformDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//デスクリプタヒープ種別
	result = device->CreateDescriptorHeap(&transformDescHeapDesc, IID_PPV_ARGS(_transformDescHeap.ReleaseAndGetAddressOf()));//生成
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = transformBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = buffSize;
	device->CreateConstantBufferView(&cbvDesc, _transformDescHeap->GetCPUDescriptorHandleForHeapStart());

	return S_OK;
}


void Matrix::Rotate(float angle) {
	_mappedTransformData->world = XMMatrixRotationY(angle);
}

#pragma region ゲッター

Matrix::SceneData* Matrix::GetMappedSceneData() {
	return _mappedSceneData;
}

ComPtr<ID3D12DescriptorHeap> Matrix::GetSceneDescHeap() {
	return _sceneDescHeap;
}

Matrix::Transform* Matrix::GetTransformData() {
	return _mappedTransformData;
}
ComPtr<ID3D12DescriptorHeap> Matrix::GetTransformHeap() {
	return _transformDescHeap;
}

#pragma endregion
