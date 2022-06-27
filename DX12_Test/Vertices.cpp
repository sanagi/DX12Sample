#include "Vertices.h"

//���_
XMFLOAT3 vertices[] = {
	{-0.4f,-0.7f,0.0f} ,//����
	{-0.4f,0.7f,0.0f} ,//����
	{0.4f,-0.7f,0.0f} ,//�E��
	{0.4f,0.7f,0.0f} ,//�E��
};

//�C���f�b�N�X�ݒ�
unsigned short indices[] = { 
	0,1,2, 
	2,1,3
};

Vertices::Vertices() : _vertexBuffer{}, _indexBuffer{}{}

HRESULT Vertices::Initialize(ID3D12Device* device) {
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
	XMFLOAT3* vertMap = nullptr;
	hr = _vertexBuffer->Map(0, nullptr, (void**)&vertMap);
	if (FAILED(hr)) {
		return hr;
	}
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	_vertexBuffer->Unmap(0, nullptr);

	//���_�o�b�t�@�r���[�̍쐬
	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();//�o�b�t�@�̉��z�A�h���X
	vbView.SizeInBytes = sizeof(vertices);//�S�o�C�g��
	vbView.StrideInBytes = sizeof(vertices[0]);//1���_������̃o�C�g��

	resdesc.Width = sizeof(indices);
	hr = device->CreateCommittedResource(&heapprop,	D3D12_HEAP_FLAG_NONE, &resdesc,	D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,	IID_PPV_ARGS(&_indexBuffer));

	//������o�b�t�@�ɃC���f�b�N�X�f�[�^���R�s�[
	unsigned short* mappedIndex = nullptr;
	_indexBuffer->Map(0, nullptr, (void**)&mappedIndex);
	std::copy(std::begin(indices), std::end(indices), mappedIndex);
	_indexBuffer->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@�r���[���쐬
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ibView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);

	return hr;
}