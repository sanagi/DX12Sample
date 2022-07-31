#include "PMXModel.h"

#pragma region コンストラクタ系

PMXModel::PMXModel() {}

PMXModel::~PMXModel() {}

#pragma endregion

/// <summary>
/// リソース作成
/// </summary>
/// <param name="device"></param>
/// <param name="vertNum"></param>
void PMXModel::CreateResource(ComPtr<ID3D12Device> device) {
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(Vertices.size() * sizeof(PMXVertex));
	//UPLOAD(確保は可能)
	auto result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_vertexBuffer));

	//頂点のコピー
	PMXVertex* vertMap = nullptr;
	result = _vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	std::copy(Vertices.begin(), Vertices.end(), vertMap);
	_vertexBuffer->Unmap(0, nullptr);

	//バッファビュー
	_vbView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress(); //バッファの仮想アドレス
	_vbView.SizeInBytes = static_cast<UINT>(Vertices.size() * sizeof(PMXVertex));//全バイト数
	_vbView.StrideInBytes = sizeof(PMXVertex);//1頂点あたりのバイト数


	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	resDesc = CD3DX12_RESOURCE_DESC::Buffer(Indices.size() * sizeof(Indices[0]));

	//設定は、バッファのサイズ以外頂点バッファの設定を使いまわして
	//OKだと思います。
	result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_indexBuffer));

	//作ったバッファにインデックスデータをコピー
	unsigned short* mappedIdx = nullptr;
	_indexBuffer->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(Indices.begin(), Indices.end(), mappedIdx);
	_indexBuffer->Unmap(0, nullptr);

	//インデックスバッファビューを作成
	_indexBufferView = {};
	_indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	_indexBufferView.SizeInBytes = static_cast<UINT>(Indices.size() * sizeof(Indices[0]));

}


#pragma endregion

void PMXModel::SetRenderBuffer(ComPtr<ID3D12GraphicsCommandList> command_list) {
	//頂点情報のセット
	command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //トポロジ指定
	//バッファビューの指定
	command_list->IASetVertexBuffers(0, 1, &_vbView);
	//インデックスバッファビューの指定
	command_list->IASetIndexBuffer(&_indexBufferView);
}