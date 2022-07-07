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

	auto error = fopen_s(&fp, "Model/�����~�N.pmd", "rb");
	if (fp == nullptr) {
		char strerr[256];
		strerror_s(strerr, 256, error);
	}

	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdheader, sizeof(pmdheader), 1, fp);

	unsigned int vertNum;//���_��
	auto re = fread(&vertNum, sizeof(vertNum), 1, fp);

	constexpr unsigned int pmdvertex_size = 38;//���_1������̃T�C�Y
	std::vector<PMDVertex> vertices(vertNum);//�o�b�t�@�m��
	for (auto i = 0; i < vertNum; i++)
	{
		fread(&vertices[i], pmdvertex_size, 1, fp);
	}

	//�C���f�b�N�X���ǂݍ���
	fread(&_indicesNum, sizeof(_indicesNum), 1, fp);

	//�C���f�b�N�X�ǂݍ���
	std::vector<unsigned short> indices(_indicesNum);
	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);

	//���_���ƃC���f�b�N�X���Ƀ��\�[�X�쐬
	CreateResource(device, vertices, indices);

	fclose(fp);
}

/// <summary>
/// ���\�[�X�쐬
/// </summary>
/// <param name="device"></param>
/// <param name="vertNum"></param>
void Model::CreateResource(ComPtr<ID3D12Device> device, std::vector<PMDVertex> vertices, std::vector<unsigned short> indices) {
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(vertices.size() * sizeof(PMDVertex));
	//UPLOAD(�m�ۂ͉\)
	auto result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_vertexBuffer));

	//���_�̃R�s�[
	PMDVertex* vertMap = nullptr;
	result = _vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	std::copy(vertices.begin(), vertices.end(), vertMap);
	_vertexBuffer->Unmap(0, nullptr);

	//�o�b�t�@�r���[
	_vbView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress(); //�o�b�t�@�̉��z�A�h���X
	_vbView.SizeInBytes = static_cast<UINT>(vertices.size() * sizeof(PMDVertex));//�S�o�C�g��
	_vbView.StrideInBytes = sizeof(PMDVertex);//1���_������̃o�C�g��


	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	resDesc = CD3DX12_RESOURCE_DESC::Buffer(indices.size() * sizeof(indices[0]));

	//�ݒ�́A�o�b�t�@�̃T�C�Y�ȊO���_�o�b�t�@�̐ݒ���g���܂킵��
	//OK���Ǝv���܂��B
	result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_indexBuffer));

	//������o�b�t�@�ɃC���f�b�N�X�f�[�^���R�s�[
	unsigned short* mappedIdx = nullptr;
	_indexBuffer->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(indices.begin(), indices.end(), mappedIdx);
	_indexBuffer->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@�r���[���쐬
	_indexBufferView = {};
	_indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	_indexBufferView.SizeInBytes = static_cast<UINT>(indices.size() * sizeof(indices[0]));

}

void Model::Draw(ComPtr<ID3D12GraphicsCommandList> command_list) {
	//�o�b�t�@�r���[�̎w��
	command_list->IASetVertexBuffers(0, 1, &_vbView);
	//�C���f�b�N�X�o�b�t�@�r���[�̎w��
	command_list->IASetIndexBuffer(&_indexBufferView);
	//�`��
	command_list->DrawIndexedInstanced(_indicesNum, 1, 0, 0, 0);
}