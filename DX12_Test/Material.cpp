#include "Material.h"

Material::Material(ComPtr<ID3D12Device> device, FILE* fp, std::string modelPath) {
	_whiteTex = Texture::CreateWhiteTexture(device);

	Load(device, fp, modelPath);
	CreateResource(device);
}

Material::~Material() {

}

void Material::Load(ComPtr<ID3D12Device> device, FILE* fp, std::string modelPath) {
	fread(&_materialNum, sizeof(_materialNum), 1, fp); //�}�e���A������ǂ�

	MaterialVector = std::vector<MaterialData>(_materialNum);
	PmdMaterialVector = std::vector<PMDMaterial>(_materialNum);
	TextureVector = std::vector<ComPtr<ID3D12Resource>>(_materialNum);
	
	auto tmp = PmdMaterialVector.size() * sizeof(PMDMaterial);

	fread(PmdMaterialVector.data(), PmdMaterialVector.size() * sizeof(PMDMaterial), 1, fp); //�S���ǂ�

	//���g��GPU�p�ɃR�s�[
	for (int i = 0; i < PmdMaterialVector.size(); ++i) {
		MaterialVector[i].indicesNum = PmdMaterialVector[i].indicesNum;
		MaterialVector[i].material.diffuse = PmdMaterialVector[i].diffuse;
		MaterialVector[i].material.alpha = PmdMaterialVector[i].alpha;
		MaterialVector[i].material.specular = PmdMaterialVector[i].specular;
		MaterialVector[i].material.specularity = PmdMaterialVector[i].specularity;
		MaterialVector[i].material.ambient = PmdMaterialVector[i].ambient;
		MaterialVector[i].additionarl.toonIdx = PmdMaterialVector[i].toonIdx;
	}

	//�e�N�X�`����ǂݍ���Ńe�N�X�`�����\�[�X�o�b�t�@�̔z��𓾂�
	for (int i = 0; i < PmdMaterialVector.size(); i++) {
		if (strlen(PmdMaterialVector[i].texFilePath) == 0) {
			TextureVector[i] = nullptr;
		}
		else
		{
			auto texFilePath = Texture::GetTexturePathFromModelAndTexPath(modelPath, PmdMaterialVector[i].texFilePath);
			TextureVector[i] = Texture::LoadTextureFromFile(device, texFilePath);
		}

	}
}

void Material::CreateResource(ComPtr<ID3D12Device> device) {
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
	for (auto& m : MaterialVector) {
		*((MaterialForhlsl*)mapMaterial) = m.material;//�f�[�^�R�s�[
		mapMaterial += materialBuffSize;//���̃A���C�����g�ʒu�܂Ői�߂�
	}
	materialBuff->Unmap(0, nullptr);

	D3D12_DESCRIPTOR_HEAP_DESC materialDescHeapDesc = {};
	materialDescHeapDesc.NumDescriptors = _materialNum * 2;//�}�e���A���ƃe�N�X�`��
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
		if (TextureVector[i].Get() == nullptr) {
			srvDesc.Format = _whiteTex->GetDesc().Format;
			device->CreateShaderResourceView(_whiteTex.Get(), &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = TextureVector[i]->GetDesc().Format;
			device->CreateShaderResourceView(TextureVector[i].Get(), &srvDesc, matDescHeapH);
		}

		
		matDescHeapH.ptr += incSize;
	}
}

/// <summary>
/// �f�B�X�N���v�^�̃Z�b�g
/// </summary>
/// <param name="command_list"></param>
void Material::Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list){
	//�q�[�v�̃Z�b�g
	command_list->SetDescriptorHeaps(1, &_descHeap);

	//�n���h���̃Z�b�g
	auto materialHandle = _descHeap->GetGPUDescriptorHandleForHeapStart();

	unsigned int indexOffset = 0;
	int i = 0;
	auto cbvsrvIncSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 2;
	for (auto& m : MaterialVector) {
		command_list->SetGraphicsRootDescriptorTable(1, materialHandle);
		command_list->DrawIndexedInstanced(m.indicesNum, 1, indexOffset, 0, 0);
		materialHandle.ptr += cbvsrvIncSize;
		indexOffset += m.indicesNum;
	}
	//materialHandle.ptr = 0;
}

