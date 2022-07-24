#include "PMDModel.h"

#pragma region �R���X�g���N�^�n

PMDModel::PMDModel(FILE* fp, ComPtr<ID3D12Device> device, const char* modelName) {

	//�t�@�C���ǂݍ���
	Open(fp, device, modelName, "rb");
	//���_���ƃC���f�b�N�X���Ƀ��\�[�X�쐬
	CreateResource(device, _vertices, _indices);
}

PMDModel::~PMDModel() {
}

#pragma endregion

#pragma region �t�@�C�����J��,���\�[�X����

void PMDModel::Open(FILE* fp, ComPtr<ID3D12Device> device, const char* modelName, const char* mode) {
	HRESULT hr{};
	char signature[3] = {};
	PMDHeader pmdheader = {};

	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdheader, sizeof(pmdheader), 1, fp);

	unsigned int vertNum;//���_��
	auto re = fread(&vertNum, sizeof(vertNum), 1, fp);

	constexpr unsigned int pmdvertex_size = 38;//���_1������̃T�C�Y
	_vertices = std::vector<PMDVertex>(vertNum);//�o�b�t�@�m��
	for (auto i = 0; i < vertNum; i++)
	{
		fread(&_vertices[i], pmdvertex_size, 1, fp);
	}

	//�C���f�b�N�X���ǂݍ���
	fread(&_indicesNum, sizeof(_indicesNum), 1, fp);

	//�C���f�b�N�X�ǂݍ���
	_indices = std::vector<unsigned short>(_indicesNum);
	fread(_indices.data(), _indices.size() * sizeof(_indices[0]), 1, fp);
}

/// <summary>
/// ���\�[�X�쐬
/// </summary>
/// <param name="device"></param>
/// <param name="vertNum"></param>
void PMDModel::CreateResource(ComPtr<ID3D12Device> device, std::vector<PMDVertex> vertices, std::vector<unsigned short> indices) {
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


#pragma endregion

void PMDModel::SetRenderBuffer(ComPtr<ID3D12GraphicsCommandList> command_list) {
	//���_���̃Z�b�g
	command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //�g�|���W�w��
	//�o�b�t�@�r���[�̎w��
	command_list->IASetVertexBuffers(0, 1, &_vbView);
	//�C���f�b�N�X�o�b�t�@�r���[�̎w��
	command_list->IASetIndexBuffer(&_indexBufferView);
}