#include "Matrix.h"

#pragma region �R���X�g���N�^�n

Matrix::Matrix() {
	_mappedSceneData = nullptr;
	_mappedTransformData = nullptr;
}

Matrix::~Matrix() {
	_mappedSceneData = nullptr;
	_mappedTransformData = nullptr;
}

#pragma endregion


//�r���[�v���W�F�N�V�����p�r���[�̐���
HRESULT Matrix::CreateSceneView(ComPtr<ID3D12Device> device, int width, int height) {
	ComPtr<ID3D12Resource> sceneConstBuff;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneData) + 0xff) & ~0xff);
	//�萔�o�b�t�@�쐬
	auto result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(sceneConstBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	_mappedSceneData = nullptr;//�}�b�v��������|�C���^
	result = sceneConstBuff->Map(0, nullptr, (void**)&_mappedSceneData);//�}�b�v

	XMFLOAT3 eye(0, 10, -15);
	XMFLOAT3 target(0, 10, 0);
	XMFLOAT3 up(0, 1, 0);
	_mappedSceneData->view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	_mappedSceneData->proj = XMMatrixPerspectiveFovLH(XM_PIDIV4,//��p��45��
		static_cast<float>(width) / static_cast<float>(height),//�A�X��
		0.1f,//�߂���
		1000.0f//������
	);
	_mappedSceneData->eye = eye;

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//�V�F�[�_���猩����悤��
	descHeapDesc.NodeMask = 0;//�}�X�N��0
	descHeapDesc.NumDescriptors = 1;//
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//�f�X�N���v�^�q�[�v���
	result = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(_sceneDescHeap.ReleaseAndGetAddressOf()));//����

	////�f�X�N���v�^�̐擪�n���h�����擾���Ă���
	auto heapHandle = _sceneDescHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = sceneConstBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = sceneConstBuff->GetDesc().Width;
	//�萔�o�b�t�@�r���[�̍쐬
	device->CreateConstantBufferView(&cbvDesc, heapHandle);
	return result;
}

HRESULT Matrix::CreateTransformView(ComPtr<ID3D12Device> device) {
	ComPtr<ID3D12Resource> transformBuff;
	//GPU�o�b�t�@�쐬
	auto buffSize = sizeof(Transform);
	buffSize = (buffSize + 0xff) & ~0xff;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(buffSize);

	auto result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(transformBuff.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	//�}�b�v�ƃR�s�[
	result = transformBuff->Map(0, nullptr, (void**)&_mappedTransformData);
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	//�r���[�̍쐬


	D3D12_DESCRIPTOR_HEAP_DESC transformDescHeapDesc = {};
	transformDescHeapDesc.NumDescriptors = 1;//�Ƃ肠�������[���h�ЂƂ�
	transformDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	transformDescHeapDesc.NodeMask = 0;

	transformDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//�f�X�N���v�^�q�[�v���
	result = device->CreateDescriptorHeap(&transformDescHeapDesc, IID_PPV_ARGS(_transformDescHeap.ReleaseAndGetAddressOf()));//����
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = transformBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = buffSize;
	device->CreateConstantBufferView(&cbvDesc, _transformDescHeap->GetCPUDescriptorHandleForHeapStart());

	return S_OK;
}


void Matrix::Rotate(float angle) {
	_mappedTransformData->world = XMMatrixRotationY(angle);
}

#pragma region �Q�b�^�[

Matrix::SceneData* Matrix::GetMappedSceneData() {
	return _mappedSceneData;
}

ComPtr<ID3D12DescriptorHeap> Matrix::GetSceneDescHeap() {
	return _sceneDescHeap;
}

Matrix::Transform* Matrix::GetTransformData() {
	return _mappedTransformData;
}
ComPtr<ID3D12DescriptorHeap> Matrix::GetTransformHeap() {
	return _transformDescHeap;
}

#pragma endregion
