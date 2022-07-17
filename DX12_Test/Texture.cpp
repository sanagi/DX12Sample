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
	//Initialize(device, resourceHeapHandle);
	//TexBuffer = LoadTextureFromFile(device, );
}

/// <summary>
/// ファイル名から拡張子を取得
/// </summary>
/// <param name="path"></param>
/// <returns></returns>
std::string Texture::GetExtension(const std::string& path) {
	int index = path.rfind('.');
	return path.substr(index + 1, path.length() - index - 1);
}

/// <summary>
/// テクスチャのパスをセパレーター文字で分離
/// </summary>
/// <param name="path"></param>
/// <param name="splitter"></param>
/// <returns></returns>
std::pair<std::string, std::string> Texture::SplitFileName(const std::string& path, const char splitter) {
	int index = path.find(splitter);
	pair<std::string, std::string> ret;
	ret.first = path.substr(0, index);
	ret.second = path.substr(index + 1, path.length() - index - 1);
	return ret;
}

///モデルのパスとテクスチャのパスから合成パスを得る
///@param modelPath アプリケーションから見たpmdモデルのパス
///@param texPath PMDモデルから見たテクスチャのパス
///@return アプリケーションから見たテクスチャのパス
std::string Texture::GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath) {
	//ファイルのフォルダ区切りは\と/の二種類が使用される可能性があり
	//ともかく末尾の\か/を得られればいいので、双方のrfindをとり比較する
	//int型に代入しているのは見つからなかった場合はrfindがepos(-1→0xffffffff)を返すため
	auto pathIndex = modelPath.rfind('/');
	auto folderPath = modelPath.substr(0, pathIndex + 1);
	return folderPath + texPath;
}

///string(マルチバイト文字列)からwstring(ワイド文字列)を得る
///@param str マルチバイト文字列
///@return 変換されたワイド文字列
std::wstring Texture::GetWideStringFromString(const std::string& str) {
	//呼び出し1回目(文字列数を得る)
	auto num1 = MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), -1, nullptr, 0);

	std::wstring wstr;//stringのwchar_t版
	wstr.resize(num1);//得られた文字列数でリサイズ

	//呼び出し2回目(確保済みのwstrに変換文字列をコピー)
	auto num2 = MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), -1, &wstr[0], num1);

	assert(num1 == num2);//一応チェック
	return wstr;
}

/// <summary>
/// テクスチャを読んでリソースバッファを得る
/// </summary>
/// <param name="texPath"></param>
/// <returns></returns>
ID3D12Resource* Texture::LoadTextureFromFile(ComPtr<ID3D12Device> device, std::string& texPath) {
	
	//WICテクスチャのロード
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};

	auto result = CoInitializeEx(0, COINIT_MULTITHREADED);

	result = LoadFromWICFile(GetWideStringFromString(texPath).c_str(), WIC_FLAGS_NONE, &metadata, scratchImg);
	if (FAILED(result)) {
		return nullptr;
	}

	/*auto wtexpath = GetWideStringFromString(texPath);//テクスチャのファイルパス
	auto ext = GetExtension(texPath);//拡張子を取得
	auto result = loadLambdaTable[ext](wtexpath,
		&metadata,
		scratchImg);
	*/
	
	auto img = scratchImg.GetImage(0, 0, 0);//生データ抽出
	
											
	//中間バッファとしてUploadヒープ設定
	/*D3D12_HEAP_PROPERTIES uploadHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

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
	result = device->CreateCommittedResource(
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
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	texHeapProp.CreationNodeMask = 0;//単一アダプタのため0
	texHeapProp.VisibleNodeMask = 0;//単一アダプタのため0

	//テクスチャを制御するディスクリプタヒープ
	resDesc.Format = metadata.format;
	resDesc.Width = static_cast<UINT>(metadata.width);//幅
	resDesc.Height = static_cast<UINT>(metadata.height);//高さ
	resDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);//2Dで配列でもないので１
	resDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels);//ミップマップしないのでミップ数は１つ
	resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);//2Dテクスチャ用
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//レイアウトについては決定しない

	ID3D12Resource* texBuffer = nullptr;
	//リソース作成
	result = device->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,//コピー用
		nullptr,
		IID_PPV_ARGS(&texBuffer)
	);
	if (FAILED(result)) {
		return nullptr;
	}

	uint8_t* mapforImg = nullptr;//と同じ型にする
	result = uploadbuff->Map(0, nullptr, (void**)&mapforImg);//マップ
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

	return texBuffer;*/

	//WriteToSubresourceで転送する用のヒープ設定
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//特殊な設定なのでdefaultでもuploadでもなく
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//ライトバックで
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//転送がL0つまりCPU側から直で
	texHeapProp.CreationNodeMask = 0;//単一アダプタのため0
	texHeapProp.VisibleNodeMask = 0;//単一アダプタのため0

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = metadata.format;
	resDesc.Width = static_cast<UINT>(metadata.width);//幅
	resDesc.Height = static_cast<UINT>(metadata.height);//高さ
	resDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);
	resDesc.SampleDesc.Count = 1;//通常テクスチャなのでアンチェリしない
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels);//ミップマップしないのでミップ数は１つ
	resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//レイアウトについては決定しない
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//とくにフラグなし

	ID3D12Resource* texbuff = nullptr;
	result = device->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&texbuff)
	);

	if (FAILED(result)) {
		return nullptr;
	}
	result = texbuff->WriteToSubresource(0,
		nullptr,//全領域へコピー
		img->pixels,//元データアドレス
		static_cast<UINT>(img->rowPitch),//1ラインサイズ
		static_cast<UINT>(img->slicePitch)//全サイズ
	);
	if (FAILED(result)) {
		return nullptr;
	}

	return texbuff;
}

/// <summary>
/// 白テクスチャ
/// </summary>
/// <param name="device"></param>
/// <returns></returns>
ID3D12Resource* Texture::CreateWhiteTexture(ComPtr<ID3D12Device> device) {
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//特殊な設定なのでdefaultでもuploadでもなく
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//ライトバックで
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//転送がL0つまりCPU側から直で
	texHeapProp.CreationNodeMask = 0;//単一アダプタのため0
	texHeapProp.VisibleNodeMask = 0;//単一アダプタのため0

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;//幅
	resDesc.Height = 4;//高さ
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = 1;//
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//レイアウトについては決定しない
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//とくにフラグなし

	ID3D12Resource* whiteBuff = nullptr;
	auto result = device->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&whiteBuff)
	);
	if (FAILED(result)) {
		return nullptr;
	}
	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0xff);

	result = whiteBuff->WriteToSubresource(0, nullptr, data.data(), 4 * 4, static_cast<UINT>(data.size()));
	return whiteBuff;
}

/// <summary>
/// 黒テクスチャ
/// </summary>
/// <param name="device"></param>
/// <returns></returns>
ID3D12Resource* Texture::CreateBlackTexture(ComPtr<ID3D12Device> device) {
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//特殊な設定なのでdefaultでもuploadでもなく
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//ライトバックで
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//転送がL0つまりCPU側から直で
	texHeapProp.CreationNodeMask = 0;//単一アダプタのため0
	texHeapProp.VisibleNodeMask = 0;//単一アダプタのため0

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;//幅
	resDesc.Height = 4;//高さ
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = 1;//
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//レイアウトについては決定しない
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//とくにフラグなし

	ID3D12Resource* blackBuff = nullptr;
	auto result = device->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&blackBuff)
	);
	if (FAILED(result)) {
		return nullptr;
	}
	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0x00);

	result = blackBuff->WriteToSubresource(0, nullptr, data.data(), 4 * 4, static_cast<UINT>(data.size()));
	return blackBuff;
}

/*void Texture::Initialize(ComPtr<ID3D12Device> device, D3D12_CPU_DESCRIPTOR_HANDLE resourceHeapHandle) {
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
}*/