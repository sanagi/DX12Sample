#include "Material.h"

Material::Material(ComPtr<ID3D12Device> device, FILE* fp, std::string modelPath) {
	_whiteTex = Texture::CreateWhiteTexture(device);
	_blackTex = Texture::CreateBlackTexture(device);

	Load(device, fp, modelPath);
	CreateResource(device);
}

Material::~Material() {

}

void Material::Load(ComPtr<ID3D12Device> device, FILE* fp, std::string modelPath) {
	fread(&_materialNum, sizeof(_materialNum), 1, fp); //マテリアル数を読む

	MaterialVector = std::vector<MaterialData>(_materialNum);
	PmdMaterialVector = std::vector<PMDMaterial>(_materialNum);
	TextureVector = std::vector<ComPtr<ID3D12Resource>>(_materialNum);
	sphTexVector = std::vector<ComPtr<ID3D12Resource>>(_materialNum);
	spaTexVector = std::vector<ComPtr<ID3D12Resource>>(_materialNum);
	
	auto tmp = PmdMaterialVector.size() * sizeof(PMDMaterial);

	fread(PmdMaterialVector.data(), PmdMaterialVector.size() * sizeof(PMDMaterial), 1, fp); //全部読む

	//中身をGPU用にコピー
	for (int i = 0; i < PmdMaterialVector.size(); ++i) {
		MaterialVector[i].indicesNum = PmdMaterialVector[i].indicesNum;
		MaterialVector[i].material.diffuse = PmdMaterialVector[i].diffuse;
		MaterialVector[i].material.alpha = PmdMaterialVector[i].alpha;
		MaterialVector[i].material.specular = PmdMaterialVector[i].specular;
		MaterialVector[i].material.specularity = PmdMaterialVector[i].specularity;
		MaterialVector[i].material.ambient = PmdMaterialVector[i].ambient;
		MaterialVector[i].additionarl.toonIdx = PmdMaterialVector[i].toonIdx;
	}

	//テクスチャを読み込んでテクスチャリソースバッファの配列を得る
	for (int i = 0; i < PmdMaterialVector.size(); i++) {
		if (strlen(PmdMaterialVector[i].texFilePath) == 0) {
			TextureVector[i] = nullptr;
		}
		else
		{
			string texFileName = PmdMaterialVector[i].texFilePath;
			string sphFileName = "";
			string spaFileName = "";
			auto namepair = Texture::SplitFileName(texFileName);
			if (std::count(texFileName.begin(), texFileName.end(), '*') > 0) {
				auto namepair = Texture::SplitFileName(texFileName);
				if (Texture::GetExtension(namepair.first) == "sph") {
					texFileName = namepair.second;
					sphFileName = namepair.first;
				}
				else if (Texture::GetExtension(namepair.first) == "spa") {
					texFileName = namepair.second;
					spaFileName = namepair.first;
				}
				else {
					texFileName = namepair.first;
					if (Texture::GetExtension(namepair.second) == "sph") {
						sphFileName = namepair.second;
					}
					else if (Texture::GetExtension(namepair.first) == "spa") {
						spaFileName = namepair.second;
					}
				}
			}
			else {
				if (Texture::GetExtension(PmdMaterialVector[i].texFilePath) == "sph") {
					sphFileName = PmdMaterialVector[i].texFilePath;
					texFileName = "";
				}
				else if (Texture::GetExtension(PmdMaterialVector[i].texFilePath) == "spa") {
					spaFileName = PmdMaterialVector[i].texFilePath;
					texFileName = "";
				}
				else {
					texFileName = PmdMaterialVector[i].texFilePath;
				}
			}
			if (texFileName != "") {
				auto texFilePath = Texture::GetTexturePathFromModelAndTexPath(modelPath, texFileName.c_str());
				TextureVector[i] = Texture::LoadTextureFromFile(device, texFilePath);
			}
			if (sphFileName != "") {
				auto sphFilePath = Texture::GetTexturePathFromModelAndTexPath(modelPath, sphFileName.c_str());
				sphTexVector[i] = Texture::LoadTextureFromFile(device, sphFilePath);
			}
			if (spaFileName != "") {
				auto spaFilePath = Texture::GetTexturePathFromModelAndTexPath(modelPath, spaFileName.c_str());
				spaTexVector[i] = Texture::LoadTextureFromFile(device, spaFilePath);
			}
		}

	}
}

void Material::CreateResource(ComPtr<ID3D12Device> device) {
	//TODO:マテリアル配置の無駄な空きを作らない方法は？

	//マテリアルバッファを作成
	auto materialBuffSize = sizeof(MaterialForhlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize * _materialNum);//勿体ないけど仕方ないですね
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
	materialDescHeapDesc.NumDescriptors = _materialNum * 4;//マテリアルとテクスチャ2つ(普通のとスフィア)
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
	for (size_t i = 0; i < _materialNum; ++i) {
		device->CreateConstantBufferView(&matCBVDesc, matDescHeapH);
		matDescHeapH.ptr += incSize;
		matCBVDesc.BufferLocation += materialBuffSize;

		//テクスチャ用のリソースビュー
		if (TextureVector[i].Get() == nullptr) {
			srvDesc.Format = _whiteTex->GetDesc().Format;
			device->CreateShaderResourceView(_whiteTex.Get(), &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = TextureVector[i]->GetDesc().Format;
			device->CreateShaderResourceView(TextureVector[i].Get(), &srvDesc, matDescHeapH);
		}		
		matDescHeapH.ptr += incSize;

		if (sphTexVector[i] == nullptr) {
			srvDesc.Format = _whiteTex->GetDesc().Format;
			device->CreateShaderResourceView(_whiteTex.Get(), &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = sphTexVector[i]->GetDesc().Format;
			device->CreateShaderResourceView(sphTexVector[i].Get(), &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;

		if (spaTexVector[i] == nullptr) {
			srvDesc.Format = _blackTex->GetDesc().Format;
			device->CreateShaderResourceView(_blackTex.Get(), &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = spaTexVector[i]->GetDesc().Format;
			device->CreateShaderResourceView(spaTexVector[i].Get(), &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;
	}
}

/// <summary>
/// ディスクリプタのセット
/// </summary>
/// <param name="command_list"></param>
void Material::Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list){
	//ヒープのセット
	command_list->SetDescriptorHeaps(1, &_descHeap);

	//ハンドルのセット
	auto materialHandle = _descHeap->GetGPUDescriptorHandleForHeapStart();

	unsigned int indexOffset = 0;
	int i = 0;
	auto cbvsrvIncSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 4;
	for (auto& m : MaterialVector) {
		command_list->SetGraphicsRootDescriptorTable(1, materialHandle);
		command_list->DrawIndexedInstanced(m.indicesNum, 1, indexOffset, 0, 0);
		materialHandle.ptr += cbvsrvIncSize;
		indexOffset += m.indicesNum;
	}
	//materialHandle.ptr = 0;
}

