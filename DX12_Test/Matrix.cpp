#include "Matrix.h"

#pragma region ������

Matrix::Matrix(ComPtr<ID3D12Device> device, int width, int height) {
	Initialize(device, width, height);
}

void Matrix::Initialize(ComPtr<ID3D12Device> device, int width, int height) {
	HRESULT hr{};

	//�g�p����s��쐬
	_worldMat = XMMatrixIdentity();
	XMFLOAT3 eye(0, 10, -15);
	XMFLOAT3 target(0, 10, 0);
	XMFLOAT3 up(0, 1, 0);
	_viewMat = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	_projMat = XMMatrixPerspectiveFovLH(XM_PIDIV2,//��p��90��
		static_cast<float>(width) / static_cast<float>(height),//�A�X��
		1.0f,//�߂���
		100.0f//������
	);

	//�A�b�v���[�h�p�̃q�[�v�ƃ��\�[�X�p�f�B�X�N���v�^
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(MatricesData) + 0xff) & ~0xff);
	//�萔�o�b�t�@�쐬
	hr = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_constBuffer)
	);

	hr = _constBuffer->Map(0, nullptr, (void**)&_mapMatrix);//�}�b�v
	_mapMatrix->world = _worldMat;
	_mapMatrix->view = _viewMat;
	_mapMatrix->proj = _projMat;

	//���\�[�X�p�̃q�[�v
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//�V�F�[�_���猩����悤��
	descHeapDesc.NodeMask = 0;//�}�X�N��0
	descHeapDesc.NumDescriptors = 1;//
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//�f�X�N���v�^�q�[�v���
	device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(_matrixDescHeap.ReleaseAndGetAddressOf()));//����

	//�o�b�t�@�r���[�p�f�B�X�N���v�^
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _constBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(_constBuffer->GetDesc().Width);

	D3D12_CPU_DESCRIPTOR_HANDLE resourceHeapHandle = _matrixDescHeap->GetCPUDescriptorHandleForHeapStart();

	//�萔�o�b�t�@�r���[�̍쐬
	device->CreateConstantBufferView(&cbvDesc, resourceHeapHandle);
}

#pragma endregion

#pragma region ���[�v��

void Matrix::Rotate(float angle) {
	_worldMat = XMMatrixRotationY(angle);
	_mapMatrix->world = _worldMat;
}

#pragma endregion

#pragma region �Q�b�^�[

ComPtr<ID3D12DescriptorHeap> Matrix::GetDescHeap() {
	return _matrixDescHeap;
}

#pragma endregion