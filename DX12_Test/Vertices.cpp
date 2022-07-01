#include "Vertices.h"

//頂点
Vertices::VertexInfo vertices[] = {
		{{-0.5f,-0.9f,0.0f},{0.0f,1.0f} },//左下
		{{-0.5f,0.9f,0.0f} ,{0.0f,0.0f}},//左上
		{{0.5f,-0.9f,0.0f} ,{1.0f,1.0f}},//右下
		{{0.5f,0.9f,0.0f} ,{1.0f,0.0f}},//右上
};

//インデックス設定
unsigned short indices[] = { 
	0,1,2, 
	2,1,3
};

Vertices::Vertices(ComPtr<ID3D12Device> device) : _vertexBuffer{}, _indexBuffer{}{
	Initialize(device);
}

HRESULT Vertices::Initialize(ComPtr<ID3D12Device> device) {
	HRESULT hr;
	//ヒープ設定
	D3D12_HEAP_PROPERTIES heapprop = {};
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD; //ヒープの種類
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; //CPUページング設定
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; //メモリプールの場所

	//リソース設定
	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //バッファの指定
	resdesc.Width = sizeof(vertices); //幅のみでまかなうので全頂点
	resdesc.Height = 1; //1
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1; //アンチエイリアシングの設定
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE; 
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; //メモリの最初から最後まで繋がってる時の指定

	//頂点バッファの生成
	hr = device->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&_vertexBuffer));
	if (FAILED(hr)) {
		return hr;
	}

	//頂点バッファを渡す
	VertexInfo* vertMap = nullptr;
	hr = _vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	if (FAILED(hr)) {
		return hr;
	}
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	_vertexBuffer->Unmap(0, nullptr);

	//頂点バッファビューの作成
	_vbView = {};
	_vbView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();//バッファの仮想アドレス
	_vbView.SizeInBytes = sizeof(vertices);//全バイト数
	_vbView.StrideInBytes = sizeof(vertices[0]);//1頂点あたりのバイト数

	resdesc.Width = sizeof(indices);
	hr = device->CreateCommittedResource(&heapprop,	D3D12_HEAP_FLAG_NONE, &resdesc,	D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,	IID_PPV_ARGS(&_indexBuffer));

	//作ったバッファにインデックスデータをコピー
	unsigned short* mappedIndex = nullptr;
	_indexBuffer->Map(0, nullptr, (void**)&mappedIndex);
	std::copy(std::begin(indices), std::end(indices), mappedIndex);
	_indexBuffer->Unmap(0, nullptr);

	//インデックスバッファビューを作成
	_indexBufferView = {};
	_indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	_indexBufferView.SizeInBytes = sizeof(indices);

	return hr;
}

void Vertices::Draw(ComPtr<ID3D12GraphicsCommandList> command_list) {

	//バッファビューの指定
	command_list->IASetVertexBuffers(0, 1, &_vbView);
	//インデックスバッファビューの指定
	command_list->IASetIndexBuffer(&_indexBufferView);

	//描画コマンド
	//command_list->DrawInstanced(6, 1, 0, 0);
}