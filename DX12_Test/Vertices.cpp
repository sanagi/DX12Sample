#include "Vertices.h"

//���_
Vertices::VertexInfo vertices[] = {
		{{-0.5f,-0.9f,0.0f},{0.0f,1.0f} },//����
		{{-0.5f,0.9f,0.0f} ,{0.0f,0.0f}},//����
		{{0.5f,-0.9f,0.0f} ,{1.0f,1.0f}},//�E��
		{{0.5f,0.9f,0.0f} ,{1.0f,0.0f}},//�E��
};

//�C���f�b�N�X�ݒ�
unsigned short indices[] = { 
	0,1,2, 
	2,1,3
};

Vertices::Vertices(ComPtr<ID3D12Device> device) : _vertexBuffer{}, _indexBuffer{}{
	Initialize(device);
}

HRESULT Vertices::Initialize(ComPtr<ID3D12Device> device) {
	HRESULT hr;
	//�q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES heapprop = {};
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD; //�q�[�v�̎��
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; //CPU�y�[�W���O�ݒ�
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; //�������v�[���̏ꏊ

	//���\�[�X�ݒ�
	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; //�o�b�t�@�̎w��
	resdesc.Width = sizeof(vertices); //���݂̂ł܂��Ȃ��̂őS���_
	resdesc.Height = 1; //1
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1; //�A���`�G�C���A�V���O�̐ݒ�
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE; 
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; //�������̍ŏ�����Ō�܂Ōq�����Ă鎞�̎w��

	//���_�o�b�t�@�̐���
	hr = device->CreateCommittedResource(&heapprop, D3D12_HEAP_FLAG_NONE, &resdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&_vertexBuffer));
	if (FAILED(hr)) {
		return hr;
	}

	//���_�o�b�t�@��n��
	VertexInfo* vertMap = nullptr;
	hr = _vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	if (FAILED(hr)) {
		return hr;
	}
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	_vertexBuffer->Unmap(0, nullptr);

	//���_�o�b�t�@�r���[�̍쐬
	_vbView = {};
	_vbView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();//�o�b�t�@�̉��z�A�h���X
	_vbView.SizeInBytes = sizeof(vertices);//�S�o�C�g��
	_vbView.StrideInBytes = sizeof(vertices[0]);//1���_������̃o�C�g��

	resdesc.Width = sizeof(indices);
	hr = device->CreateCommittedResource(&heapprop,	D3D12_HEAP_FLAG_NONE, &resdesc,	D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,	IID_PPV_ARGS(&_indexBuffer));

	//������o�b�t�@�ɃC���f�b�N�X�f�[�^���R�s�[
	unsigned short* mappedIndex = nullptr;
	_indexBuffer->Map(0, nullptr, (void**)&mappedIndex);
	std::copy(std::begin(indices), std::end(indices), mappedIndex);
	_indexBuffer->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@�r���[���쐬
	_indexBufferView = {};
	_indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	_indexBufferView.SizeInBytes = sizeof(indices);

	return hr;
}

void Vertices::Draw(ComPtr<ID3D12GraphicsCommandList> command_list) {

	//�o�b�t�@�r���[�̎w��
	command_list->IASetVertexBuffers(0, 1, &_vbView);
	//�C���f�b�N�X�o�b�t�@�r���[�̎w��
	command_list->IASetIndexBuffer(&_indexBufferView);

	//�`��R�}���h
	//command_list->DrawInstanced(6, 1, 0, 0);
}