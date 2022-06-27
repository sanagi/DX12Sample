#include "Vertices.h"

//頂点
XMFLOAT3 vertices[] = {
	{-0.4f,-0.7f,0.0f} ,//左下
	{-0.4f,0.7f,0.0f} ,//左上
	{0.4f,-0.7f,0.0f} ,//右下
	{0.4f,0.7f,0.0f} ,//右上
};

//インデックス設定
unsigned short indices[] = { 
	0,1,2, 
	2,1,3
};

Vertices::Vertices() : _vertexBuffer{}, _indexBuffer{}{}

HRESULT Vertices::Initialize(ID3D12Device* device) {
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
	XMFLOAT3* vertMap = nullptr;
	hr = _vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	if (FAILED(hr)) {
		return hr;
	}
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	_vertexBuffer->Unmap(0, nullptr);

	//頂点バッファビューの作成
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();//バッファの仮想アドレス
	vbView.SizeInBytes = sizeof(vertices);//全バイト数
	vbView.StrideInBytes = sizeof(vertices[0]);//1頂点あたりのバイト数

	resdesc.Width = sizeof(indices);
	hr = device->CreateCommittedResource(&heapprop,	D3D12_HEAP_FLAG_NONE, &resdesc,	D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,	IID_PPV_ARGS(&_indexBuffer));

	//作ったバッファにインデックスデータをコピー
	unsigned short* mappedIndex = nullptr;
	_indexBuffer->Map(0, nullptr, (void**)&mappedIndex);
	std::copy(std::begin(indices), std::end(indices), mappedIndex);
	_indexBuffer->Unmap(0, nullptr);

	//インデックスバッファビューを作成
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ibView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);

	return hr;
}