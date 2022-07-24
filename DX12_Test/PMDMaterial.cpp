#include "PMDMaterial.h"

PMDMaterial::PMDMaterial(ComPtr<ID3D12Device> device, FILE* fp, std::string modelPath, int sizeNum, PMDRenderer renderer) : _renderer(renderer) {
	Load(device, fp, modelPath);
	CreateResource(device, sizeNum);
}

PMDMaterial::~PMDMaterial() {

}

void PMDMaterial::Load(ComPtr<ID3D12Device> device, FILE* fp, std::string modelPath) {
	fread(&_materialNum, sizeof(_materialNum), 1, fp); //�}�e���A������ǂ�

	_materialVector = std::vector<MaterialData>(_materialNum);
	_pmdMaterialVector = std::vector<PMDMaterialData>(_materialNum);
	_textureVector = std::vector<ComPtr<ID3D12Resource>>(_materialNum);
	_sphTexVector = std::vector<ComPtr<ID3D12Resource>>(_materialNum);
	_spaTexVector = std::vector<ComPtr<ID3D12Resource>>(_materialNum);
	_toonTexVector = std::vector<ComPtr<ID3D12Resource>>(_materialNum);

	auto tmp = _pmdMaterialVector.size() * sizeof(PMDMaterialData);

	fread(_pmdMaterialVector.data(), _pmdMaterialVector.size() * sizeof(PMDMaterialData), 1, fp); //�S���ǂ�

	//���g��GPU�p�ɃR�s�[
	for (int i = 0; i < _pmdMaterialVector.size(); ++i) {
		_materialVector[i].indicesNum = _pmdMaterialVector[i].indicesNum;
		_materialVector[i].material.diffuse = _pmdMaterialVector[i].diffuse;
		_materialVector[i].material.alpha = _pmdMaterialVector[i].alpha;
		_materialVector[i].material.specular = _pmdMaterialVector[i].specular;
		_materialVector[i].material.specularity = _pmdMaterialVector[i].specularity;
		_materialVector[i].material.ambient = _pmdMaterialVector[i].ambient;
		_materialVector[i].additionarl.toonIdx = _pmdMaterialVector[i].toonIdx;
	}

	//�e�N�X�`����ǂݍ���Ńe�N�X�`�����\�[�X�o�b�t�@�̔z��𓾂�
	for (int i = 0; i < _pmdMaterialVector.size(); i++) {

		//�g�D�[�����\�[�X�̓ǂݍ���
		char toonFilePath[32];
		sprintf_s(toonFilePath, "toon%02d.bmp", _pmdMaterialVector[i].toonIdx + 1);
		auto texFilePath = Texture::GetTexturePathFromModelAndTexPath(modelPath, toonFilePath);
		_toonTexVector[i] = Texture::LoadTextureFromFile(device, texFilePath, _resourceTable);

		if (strlen(_pmdMaterialVector[i].texFilePath) == 0) {
			_textureVector[i] = nullptr;
		}
		else
		{
			string texFileName = _pmdMaterialVector[i].texFilePath;
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
				if (Texture::GetExtension(_pmdMaterialVector[i].texFilePath) == "sph") {
					sphFileName = _pmdMaterialVector[i].texFilePath;
					texFileName = "";
				}
				else if (Texture::GetExtension(_pmdMaterialVector[i].texFilePath) == "spa") {
					spaFileName = _pmdMaterialVector[i].texFilePath;
					texFileName = "";
				}
				else {
					texFileName = _pmdMaterialVector[i].texFilePath;
				}
			}
			if (texFileName != "") {
				auto texFilePath = Texture::GetTexturePathFromModelAndTexPath(modelPath, texFileName.c_str());
				_textureVector[i] = Texture::LoadTextureFromFile(device, texFilePath, _resourceTable);
			}
			if (sphFileName != "") {
				auto sphFilePath = Texture::GetTexturePathFromModelAndTexPath(modelPath, sphFileName.c_str());
				_sphTexVector[i] = Texture::LoadTextureFromFile(device, sphFilePath, _resourceTable);
			}
			if (spaFileName != "") {
				auto spaFilePath = Texture::GetTexturePathFromModelAndTexPath(modelPath, spaFileName.c_str());
				_spaTexVector[i] = Texture::LoadTextureFromFile(device, spaFilePath, _resourceTable);
			}
		}

	}
}

void PMDMaterial::CreateResource(ComPtr<ID3D12Device> device, int sizeNum) {
	//TODO:�}�e���A���z�u�̖��ʂȋ󂫂����Ȃ����@�́H

	//�}�e���A���o�b�t�@���쐬
	auto materialBuffSize = sizeof(MaterialForhlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize * _materialNum);//�ܑ̂Ȃ����ǎd���Ȃ��ł���
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

	//�}�b�v�}�e���A���ɃR�s�[
	char* mapMaterial = nullptr;
	result = materialBuff->Map(0, nullptr, (void**)&mapMaterial);
	for (auto& m : _materialVector) {
		*((MaterialForhlsl*)mapMaterial) = m.material;//�f�[�^�R�s�[
		mapMaterial += materialBuffSize;//���̃A���C�����g�ʒu�܂Ői�߂�
	}
	materialBuff->Unmap(0, nullptr);

	D3D12_DESCRIPTOR_HEAP_DESC materialDescHeapDesc = {};
	materialDescHeapDesc.NumDescriptors = _materialNum * sizeNum;
	materialDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	materialDescHeapDesc.NodeMask = 0;

	materialDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//�f�X�N���v�^�q�[�v���
	result = device->CreateDescriptorHeap(&materialDescHeapDesc, IID_PPV_ARGS(&_descHeap));//����

	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
	matCBVDesc.BufferLocation = materialBuff->GetGPUVirtualAddress(); //�o�b�t�@�[�̃A�h���X
	matCBVDesc.SizeInBytes = static_cast<UINT>(materialBuffSize); // �}�e���A����256�A���C�����g�T�C�Y

	////�ʏ�e�N�X�`���r���[�쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//��q
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = 1;//�~�b�v�}�b�v�͎g�p���Ȃ��̂�1

	auto matDescHeapH = _descHeap->GetCPUDescriptorHandleForHeapStart();
	auto incSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//���ꂼ��̃r���[�쐬
	for (size_t i = 0; i < _materialNum; ++i) {
		device->CreateConstantBufferView(&matCBVDesc, matDescHeapH);
		matDescHeapH.ptr += incSize;
		matCBVDesc.BufferLocation += materialBuffSize;

		//�e�N�X�`���p�̃��\�[�X�r���[
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
/// �f�B�X�N���v�^�̃Z�b�g
/// </summary>
/// <param name="command_list"></param>
void PMDMaterial::Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list, int sizeNum) {
	//�q�[�v�̃Z�b�g
	command_list->SetDescriptorHeaps(1, &_descHeap);

	//�n���h���̃Z�b�g
	auto materialHandle = _descHeap->GetGPUDescriptorHandleForHeapStart();

	unsigned int indexOffset = 0;
	int i = 0;
	auto cbvsrvIncSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * sizeNum;
	for (auto& m : _materialVector) {
		command_list->SetGraphicsRootDescriptorTable(1, materialHandle);
		command_list->DrawIndexedInstanced(m.indicesNum, 1, indexOffset, 0, 0);
		materialHandle.ptr += cbvsrvIncSize;
		indexOffset += m.indicesNum;
	}
	//materialHandle.ptr = 0;
}

