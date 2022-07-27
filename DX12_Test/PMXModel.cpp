#include "PMXModel.h"

#pragma region �R���X�g���N�^�n

PMXModel::PMXModel() {}

PMXModel::~PMXModel() {}

#pragma endregion

/// <summary>
/// ���\�[�X�쐬
/// </summary>
/// <param name="device"></param>
/// <param name="vertNum"></param>
void PMXModel::CreateResource(ComPtr<ID3D12Device> device) {
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(Vertices.size() * sizeof(PMXVertex));
	//UPLOAD(�m�ۂ͉\)
	auto result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_vertexBuffer));

	//���_�̃R�s�[
	PMXVertex* vertMap = nullptr;
	result = _vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	std::copy(Vertices.begin(), Vertices.end(), vertMap);
	_vertexBuffer->Unmap(0, nullptr);

	//�o�b�t�@�r���[
	_vbView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress(); //�o�b�t�@�̉��z�A�h���X
	_vbView.SizeInBytes = static_cast<UINT>(Vertices.size() * sizeof(PMXVertex));//�S�o�C�g��
	_vbView.StrideInBytes = sizeof(PMXVertex);//1���_������̃o�C�g��


	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	resDesc = CD3DX12_RESOURCE_DESC::Buffer(Indices.size() * sizeof(Indices[0]));

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
	std::copy(Indices.begin(), Indices.end(), mappedIdx);
	_indexBuffer->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@�r���[���쐬
	_indexBufferView = {};
	_indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	_indexBufferView.SizeInBytes = static_cast<UINT>(Indices.size() * sizeof(Indices[0]));

}


#pragma endregion

void PMXModel::SetRenderBuffer(ComPtr<ID3D12GraphicsCommandList> command_list) {
	//���_���̃Z�b�g
	command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //�g�|���W�w��
	//�o�b�t�@�r���[�̎w��
	command_list->IASetVertexBuffers(0, 1, &_vbView);
	//�C���f�b�N�X�o�b�t�@�r���[�̎w��
	command_list->IASetIndexBuffer(&_indexBufferView);
}