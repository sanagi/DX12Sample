#include "PMDActor.h"

const int MATERIAL_DESC_SIZE = 4; //�}�e���A���A��{�e�N�X�`���A�X�t�B�A2��

#pragma region �R���X�g���N�^�n

PMDActor::PMDActor(ComPtr<ID3D12Device> device, const char* filepath, PMDRenderer renderer) : _angle(0.0f)
{
	FILE* fp = nullptr;
	auto error = fopen_s(&fp, filepath, "rb");
	//auto error = fopen_s(&fp, "Model/�������J.pmd", "rb");
	if (fp == nullptr) {
		char strerr[256];
		strerror_s(strerr, 256, error);
	}
	//���f������
	//_model = new Model(fp, device, filepath);

	//�}�e���A���쐬
	_material = new Material(device, fp, filepath, MATERIAL_DESC_SIZE, renderer);

	//�g�����X�t�H�[������
	//_transformMatrix = new Matrix();
	//_transformMatrix->CreateTransformView(device);
	//Matrix::CreateTransformView(device, _mappedTransform, _transformHeap);
	//_transformMatrix->GetTransformData()->world = XMMatrixIdentity();

	//_transform.world = XMMatrixIdentity();
	//CreateTransformView(device);

	fclose(fp);
}


PMDActor::~PMDActor()
{
}

#pragma endregion

/*HRESULT PMDActor::CreateTransformView(ComPtr<ID3D12Device> device) {
	//GPU�o�b�t�@�쐬
	auto buffSize = sizeof(Matrix::Transform);
	buffSize = (buffSize + 0xff) & ~0xff;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(buffSize);

	auto result = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_transformBuff.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	//�}�b�v�ƃR�s�[
	result = _transformBuff->Map(0, nullptr, (void**)&_mappedTransform);
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}
	*_mappedTransform = _transform;

	//�r���[�̍쐬
	D3D12_DESCRIPTOR_HEAP_DESC transformDescHeapDesc = {};
	transformDescHeapDesc.NumDescriptors = 1;//�Ƃ肠�������[���h�ЂƂ�
	transformDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	transformDescHeapDesc.NodeMask = 0;

	transformDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//�f�X�N���v�^�q�[�v���
	result = device->CreateDescriptorHeap(&transformDescHeapDesc, IID_PPV_ARGS(_transformHeap.ReleaseAndGetAddressOf()));//����
	if (FAILED(result)) {
		assert(SUCCEEDED(result));
		return result;
	}

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _transformBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = buffSize;
	device->CreateConstantBufferView(&cbvDesc, _transformHeap->GetCPUDescriptorHandleForHeapStart());

	return S_OK;
}*/

#pragma region �`�惋�[�v

void PMDActor::Update() {
	//_angle += 0.03f;
	//_transformMatrix->GetTransformData()->world = XMMatrixRotationY(_angle);
	//_angle += 0.03f;
	//_mappedTransform->world = XMMatrixRotationY(_angle);
}

void PMDActor::Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list) {

	//���f���`��
	//_model->SetRenderBuffer(command_list);

	//�g�����X�t�H�[���p�̃q�[�v�w��
	//ID3D12DescriptorHeap* transheaps[] = {_transformMatrix->GetTransformHeap().Get()};
	//command_list->SetDescriptorHeaps(1, transheaps);
	//command_list->SetGraphicsRootDescriptorTable(1, _transformMatrix->GetTransformHeap()->GetGPUDescriptorHandleForHeapStart());
	
	//ID3D12DescriptorHeap* transheaps[] = { _transformHeap.Get() };
	//command_list->SetDescriptorHeaps(1, transheaps);
	//command_list->SetGraphicsRootDescriptorTable(1, _transformHeap->GetGPUDescriptorHandleForHeapStart());

	//�}�e���A���`��
	_material->Draw(device, command_list, MATERIAL_DESC_SIZE);
	
}

#pragma endregion
