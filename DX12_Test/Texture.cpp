#include "Texture.h"

///アライメントに揃えたサイズを返す
///@param size 元のサイズ
///@param alignment アライメントサイズ
///@return アライメントをそろえたサイズ
size_t
AlignmentedSize(size_t size, size_t alignment) {
	return size + alignment - size % alignment;
}

Texture::Texture(ComPtr<ID3D12Device> device, D3D12_CPU_DESCRIPTOR_HANDLE resourceHeapHandle){
	Initialize(device, resourceHeapHandle);
}

void Texture::Initialize(ComPtr<ID3D12Device> device, D3D12_CPU_DESCRIPTOR_HANDLE resourceHeapHandle) {
	//WICテクスチャのロード
	CoInitializeEx(0, COINIT_MULTITHREADED);
	ScratchImage scratchImg = {};
	HRESULT hr = LoadFromWICFile(L"img/textest.png", WIC_FLAGS_NONE, &_metaData, scratchImg);
	auto img = scratchImg.GetImage(0, 0, 0);//生データ抽出

	//中間バッファとしてUploadヒープ設定
	D3D12_HEAP_PROPERTIES uploadHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;//単なるバッファとして
	auto pixelsize = scratchImg.GetPixelsSize();
	resDesc.Width = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * img->height;//データサイズ
	resDesc.Height = 1;//
	resDesc.DepthOrArraySize = 1;//
	resDesc.MipLevels = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;//連続したデータですよ
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//とくにフラグなし
	resDesc.SampleDesc.Count = 1;//通常テクスチャなのでアンチェリしない
	resDesc.SampleDesc.Quality = 0;//

	//中間バッファ作成
	ID3D12Resource* uploadbuff = nullptr;
	hr = device->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,//特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,//CPUから書き込み可能
		nullptr,
		IID_PPV_ARGS(&uploadbuff)
	);

	//テクスチャのヒープ設定
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	texHeapProp.CreationNodeMask = 0;//単一アダプタのため0
	texHeapProp.VisibleNodeMask = 0;//単一アダプタのため0

	//テクスチャを制御するディスクリプタヒープ
	resDesc.Format = _metaData.format;
	resDesc.Width = static_cast<UINT>(_metaData.width);//幅
	resDesc.Height = static_cast<UINT>(_metaData.height);//高さ
	resDesc.DepthOrArraySize = static_cast<UINT16>(_metaData.arraySize);//2Dで配列でもないので１
	resDesc.MipLevels = static_cast<UINT16>(_metaData.mipLevels);//ミップマップしないのでミップ数は１つ
	resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(_metaData.dimension);//2Dテクスチャ用
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//レイアウトについては決定しない

	//リソース作成
	hr = device->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,//コピー用
		nullptr,
		IID_PPV_ARGS(&TexBuffer)
	);
	if (FAILED(hr)) {
		return;
	}

	uint8_t* mapforImg = nullptr;//と同じ型にする
	hr = uploadbuff->Map(0, nullptr, (void**)&mapforImg);//マップ
	auto srcAddress = img->pixels;
	auto rowPitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	for (int y = 0; y < img->height; ++y) {
		std::copy_n(srcAddress,
			rowPitch,
			mapforImg);//コピー
		//1行ごとの辻褄を合わせてやる
		srcAddress += img->rowPitch;
		mapforImg += rowPitch;
	}
	uploadbuff->Unmap(0, nullptr);//アンマップ

	src = new D3D12_TEXTURE_COPY_LOCATION();
	dst = new D3D12_TEXTURE_COPY_LOCATION();

	dst->pResource = TexBuffer.Get();
	dst->Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst->SubresourceIndex = 0;

	src->pResource = uploadbuff;//中間バッファ
	src->Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;//フットプリント(メモリの占有領域)指定
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
	UINT nrow;
	UINT64 rowsize, size;
	auto desc = TexBuffer->GetDesc();
	device->GetCopyableFootprints(&desc, 0, 1, 0, &footprint, &nrow, &rowsize, &size);
	src->PlacedFootprint = footprint;
	src->PlacedFootprint.Offset = 0;
	src->PlacedFootprint.Footprint.Width = static_cast<UINT>(_metaData.width);
	src->PlacedFootprint.Footprint.Height = static_cast<UINT>(_metaData.height);
	src->PlacedFootprint.Footprint.Depth = static_cast<UINT>(_metaData.depth);
	src->PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
	src->PlacedFootprint.Footprint.Format = img->format;

	//テクスチャビュー
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = _metaData.format;//DXGI_FORMAT_R8G8B8A8_UNORM;//RGBA(0.0f～1.0fに正規化)
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//後述
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;//ミップマップは使用しないので1

	device->CreateShaderResourceView(TexBuffer.Get(), //ビューと関連付けるバッファ
		&srvDesc, //先ほど設定したテクスチャ設定情報
		resourceHeapHandle//ヒープのどこに割り当てるか
	);
}