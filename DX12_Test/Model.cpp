#include "Model.h"

Model::Model(ComPtr<ID3D12Device> device, const char* modelName, const char* mode) {
	Open(device, modelName, mode);
}

Model::~Model() {
}

void Model::Open(ComPtr<ID3D12Device> device, const char* modelName, const char* mode) {
	HRESULT hr{};
	char signature[3] = {};
	PMDHeader pmdheader = {};
	FILE* fp;

	auto error = fopen_s(&fp, "Model/初音ミク.pmd", "rb");
	if (fp == nullptr) {
		char strerr[256];
		strerror_s(strerr, 256, error);
	}

	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdheader, sizeof(pmdheader), 1, fp);

	unsigned int vertNum;//頂点数
	auto re = fread(&vertNum, sizeof(vertNum), 1, fp);

	constexpr unsigned int pmdvertex_size = 38;//頂点1つあたりのサイズ
	std::vector<PMDVertex> vertices(vertNum);//バッファ確保
	for (auto i = 0; i < vertNum; i++)
	{
		fread(&vertices[i], pmdvertex_size, 1, fp);
	}

	//インデックス数読み込み
	fread(&_indicesNum, sizeof(_indicesNum), 1, fp);

	//インデックス読み込み
	std::vector<unsigned short> indices(_indicesNum);
	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);

	//頂点情報とインデックス元にリソース作成
	CreateResource(device, vertices, indices);

	fclose(fp);
}

/// <summary>
/// リソース作成
/// </summary>
/// <param name="device"></param>
/// <param name="vertNum"></param>
void Model::CreateResource(ComPtr<ID3D12Device> device, std::vector<PMDVertex> vertices, std::vector<unsigned short> indices) {
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(vertices.size() * sizeof(PMDVertex));
	//UPLOAD(確保は可能)
	auto result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_vertexBuffer));

	//頂点のコピー
	PMDVertex* vertMap = nullptr;
	result = _vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	std::copy(vertices.begin(), vertices.end(), vertMap);
	_vertexBuffer->Unmap(0, nullptr);

	//バッファビュー
	_vbView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress(); //バッファの仮想アドレス
	_vbView.SizeInBytes = static_cast<UINT>(vertices.size() * sizeof(PMDVertex));//全バイト数
	_vbView.StrideInBytes = sizeof(PMDVertex);//1頂点あたりのバイト数


	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	resDesc = CD3DX12_RESOURCE_DESC::Buffer(indices.size() * sizeof(indices[0]));

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
	std::copy(indices.begin(), indices.end(), mappedIdx);
	_indexBuffer->Unmap(0, nullptr);

	//インデックスバッファビューを作成
	_indexBufferView = {};
	_indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	_indexBufferView.SizeInBytes = static_cast<UINT>(indices.size() * sizeof(indices[0]));

}

void Model::Draw(ComPtr<ID3D12GraphicsCommandList> command_list) {
	//バッファビューの指定
	command_list->IASetVertexBuffers(0, 1, &_vbView);
	//インデックスバッファビューの指定
	command_list->IASetIndexBuffer(&_indexBufferView);
	//描画
	command_list->DrawIndexedInstanced(_indicesNum, 1, 0, 0, 0);
}