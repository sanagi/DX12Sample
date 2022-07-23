#include "Matrix.h"

#pragma region 初期化

Matrix::Matrix(ComPtr<ID3D12Device> device, int width, int height) {
	Initialize(device, width, height);
}

void Matrix::Initialize(ComPtr<ID3D12Device> device, int width, int height) {
	HRESULT hr{};

	//使用する行列作成
	_worldMat = XMMatrixIdentity();
	XMFLOAT3 eye(0, 10, -15);
	XMFLOAT3 target(0, 10, 0);
	XMFLOAT3 up(0, 1, 0);
	_viewMat = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	_projMat = XMMatrixPerspectiveFovLH(XM_PIDIV2,//画角は90°
		static_cast<float>(width) / static_cast<float>(height),//アス比
		1.0f,//近い方
		100.0f//遠い方
	);

	//アップロード用のヒープとリソース用ディスクリプタ
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(MatricesData) + 0xff) & ~0xff);
	//定数バッファ作成
	hr = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_constBuffer)
	);

	hr = _constBuffer->Map(0, nullptr, (void**)&_mapMatrix);//マップ
	_mapMatrix->world = _worldMat;
	_mapMatrix->view = _viewMat;
	_mapMatrix->proj = _projMat;

	//リソース用のヒープ
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
	descHeapDesc.NodeMask = 0;//マスクは0
	descHeapDesc.NumDescriptors = 1;//
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//デスクリプタヒープ種別
	device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(_matrixDescHeap.ReleaseAndGetAddressOf()));//生成

	//バッファビュー用ディスクリプタ
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _constBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(_constBuffer->GetDesc().Width);

	D3D12_CPU_DESCRIPTOR_HANDLE resourceHeapHandle = _matrixDescHeap->GetCPUDescriptorHandleForHeapStart();

	//定数バッファビューの作成
	device->CreateConstantBufferView(&cbvDesc, resourceHeapHandle);
}

#pragma endregion

#pragma region ループ時

void Matrix::Rotate(float angle) {
	_worldMat = XMMatrixRotationY(angle);
	_mapMatrix->world = _worldMat;
}

#pragma endregion

#pragma region ゲッター

ComPtr<ID3D12DescriptorHeap> Matrix::GetDescHeap() {
	return _matrixDescHeap;
}

#pragma endregion