#include "PMXMaterial.h"

PMXMaterial::PMXMaterial(PMXRenderer renderer) : _renderer(renderer) {
	MaterialNum = 0;
}

PMXMaterial::~PMXMaterial() {

}

//GPUに渡すデータを作る
void PMXMaterial::CreateMaterialDataForGPU(ComPtr<ID3D12Device> device, std::string modelPath) {
	//Vectorのサイズがおかしい
	if (PmxMaterialVector.size() <= 0) {
		return;
	}

	//中身をGPU用にコピー
	for (int i = 0; i < PmxMaterialVector.size(); ++i) {
		MaterialVector[i].indicesNum = PmxMaterialVector[i].indicesNum;
		MaterialVector[i].material.diffuse = PmxMaterialVector[i].diffuse;
		MaterialVector[i].material.specular = PmxMaterialVector[i].specular;
		MaterialVector[i].material.specularity = PmxMaterialVector[i].specularity;
		MaterialVector[i].material.ambient = PmxMaterialVector[i].ambient;
		MaterialVector[i].additionarl.toonIdx = PmxMaterialVector[i].toonCommonTextureIndex;
	}

	_textureVector = std::vector<ComPtr<ID3D12Resource>>(MaterialNum);
	_sphTexVector = std::vector<ComPtr<ID3D12Resource>>(MaterialNum);
	_spaTexVector = std::vector<ComPtr<ID3D12Resource>>(MaterialNum);
	_toonTexVector = std::vector<ComPtr<ID3D12Resource>>(MaterialNum);

	//テクスチャをテーブルから読み込んでテクスチャリソースバッファの配列を得る
	for (int i = 0; i < PmxMaterialVector.size(); i++) {

		//最初はトゥーンリソースの読み込み
		char toonFilePath[32];
		sprintf_s(toonFilePath, "toon%02d.bmp", PmxMaterialVector[i].toonCommonTextureIndex + 1);
		auto texFilePath = Texture::GetTexturePathFromModelAndTexPath(modelPath, toonFilePath);
		_toonTexVector[i] = Texture::LoadTextureFromFile(device, texFilePath, _resourceTable);

		_textureVector[i] = _sphTexVector[i] = _spaTexVector[i] = nullptr;


		string texFileName;
		int colorMapIndex = PmxMaterialVector[i].colorMapTextureIndex;
		if (0 <= colorMapIndex && colorMapIndex < TexturePathVector.size()) {
			texFileName = TexturePathVector[PmxMaterialVector[i].colorMapTextureIndex];
		}

		int sphereMapIndex = PmxMaterialVector[i].sphereMapTextureIndex;
		string sphereFileName;
		if (0 <= sphereMapIndex && sphereMapIndex < TexturePathVector.size()) {
			sphereFileName = TexturePathVector[PmxMaterialVector[i].sphereMapTextureIndex];
		}
	

		if (texFileName != "") {
			auto texFilePath = Texture::GetTexturePathFromModelAndTexPath(modelPath, texFileName.c_str());
			_textureVector[i] = Texture::LoadTextureFromFile(device, texFilePath, _resourceTable);
		}
		if (sphereFileName != "") {
			std::string sphFilePath = "";
			std::string spaFilePath = "";
			switch (PmxMaterialVector[i].sphereMode)
			{
				//スフィアモード 0:無効 1:乗算(sph) 2:加算(spa) 3:サブテクスチャ(追加UV1のx,yをUV参照して通常テクスチャ描画を行う)
				//3はまだ実装中
			case 1:
				sphFilePath = Texture::GetTexturePathFromModelAndTexPath(modelPath, sphereFileName.c_str());
				_sphTexVector[i] = Texture::LoadTextureFromFile(device, sphFilePath, _resourceTable);
				break;
			case 2:
				spaFilePath = Texture::GetTexturePathFromModelAndTexPath(modelPath, sphereFileName.c_str());
				_spaTexVector[i] = Texture::LoadTextureFromFile(device, spaFilePath, _resourceTable);
				break;
			case 0:
			case 3:
			default:
				break;
			}
		}
	}
}

void PMXMaterial::CreateResource(ComPtr<ID3D12Device> device, int sizeNum) {
	//TODO:マテリアル配置の無駄な空きを作らない方法は？

	//マテリアルバッファを作成
	auto materialBuffSize = sizeof(MaterialForhlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize * MaterialNum);//勿体ないけど仕方ないですね
	ID3D12Resource* materialBuff = nullptr;
	auto result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&materialBuff)
	);
	if (FAILED(result)) {
		return;
	}

	//マップマテリアルにコピー
	char* mapMaterial = nullptr;
	result = materialBuff->Map(0, nullptr, (void**)&mapMaterial);
	for (auto& m : MaterialVector) {
		*((MaterialForhlsl*)mapMaterial) = m.material;//データコピー
		mapMaterial += materialBuffSize;//次のアライメント位置まで進める
	}
	materialBuff->Unmap(0, nullptr);

	D3D12_DESCRIPTOR_HEAP_DESC materialDescHeapDesc = {};
	materialDescHeapDesc.NumDescriptors = MaterialNum * sizeNum;
	materialDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	materialDescHeapDesc.NodeMask = 0;

	materialDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//デスクリプタヒープ種別
	result = device->CreateDescriptorHeap(&materialDescHeapDesc, IID_PPV_ARGS(&_descHeap));//生成

	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
	matCBVDesc.BufferLocation = materialBuff->GetGPUVirtualAddress(); //バッファーのアドレス
	matCBVDesc.SizeInBytes = static_cast<UINT>(materialBuffSize); // マテリアルの256アライメントサイズ

	////通常テクスチャビュー作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//後述
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;//ミップマップは使用しないので1

	auto matDescHeapH = _descHeap->GetCPUDescriptorHandleForHeapStart();
	auto incSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//それぞれのビュー作成
	for (size_t i = 0; i < MaterialNum; ++i) {
		device->CreateConstantBufferView(&matCBVDesc, matDescHeapH);
		matDescHeapH.ptr += incSize;
		matCBVDesc.BufferLocation += materialBuffSize;

		//テクスチャ用のリソースビュー
		if (_textureVector[i].Get() == nullptr) {
			srvDesc.Format = _renderer.WhiteTex->GetDesc().Format;
			device->CreateShaderResourceView(_renderer.WhiteTex.Get(), &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = _textureVector[i]->GetDesc().Format;
			device->CreateShaderResourceView(_textureVector[i].Get(), &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;

		if (_sphTexVector[i] == nullptr) {
			srvDesc.Format = _renderer.WhiteTex->GetDesc().Format;
			device->CreateShaderResourceView(_renderer.WhiteTex.Get(), &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = _sphTexVector[i]->GetDesc().Format;
			device->CreateShaderResourceView(_sphTexVector[i].Get(), &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;

		if (_spaTexVector[i] == nullptr) {
			srvDesc.Format = _renderer.BlackTex->GetDesc().Format;
			device->CreateShaderResourceView(_renderer.BlackTex.Get(), &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = _spaTexVector[i]->GetDesc().Format;
			device->CreateShaderResourceView(_spaTexVector[i].Get(), &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;

		if (_toonTexVector[i] == nullptr) {
			srvDesc.Format = _renderer.GradTex->GetDesc().Format;
			device->CreateShaderResourceView(_renderer.GradTex.Get(), &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = _toonTexVector[i]->GetDesc().Format;
			device->CreateShaderResourceView(_toonTexVector[i].Get(), &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;
	}
}

/// <summary>
/// ディスクリプタのセット
/// </summary>
/// <param name="command_list"></param>
void PMXMaterial::Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, int sizeNum) {
	//ヒープのセット
	command_list->SetDescriptorHeaps(1, &_descHeap);

	//ハンドルのセット
	auto materialHandle = _descHeap->GetGPUDescriptorHandleForHeapStart();

	unsigned int indexOffset = 0;
	int i = 0;
	auto cbvsrvIncSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * sizeNum;
	for (auto& m : MaterialVector) {
		command_list->SetGraphicsRootDescriptorTable(1, materialHandle);
		command_list->DrawIndexedInstanced(m.indicesNum, 1, indexOffset, 0, 0);
		materialHandle.ptr += cbvsrvIncSize;
		indexOffset += m.indicesNum;
	}
	//materialHandle.ptr = 0;
}

