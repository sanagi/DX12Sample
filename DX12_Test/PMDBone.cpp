#include "PMDBone.h"

#pragma region �R���X�g���N�^

PMDBone::PMDBone(ComPtr<ID3D12Device> device, FILE* fp) {
	LoadBone(device, fp);
	CreateResource(device);
}

PMDBone::~PMDBone() {

}

#pragma endregion

#pragma region �ǂݍ���

void PMDBone::LoadBone(ComPtr<ID3D12Device> device, FILE* fp) {
	unsigned short boneNum = 0;
	fread(&boneNum, sizeof(boneNum), 1, fp);

	vector<Bone> pmdBones(boneNum);
	fread(pmdBones.data(), sizeof(Bone), boneNum, fp);
	fclose(fp);

	//�C���f�b�N�X�Ɩ��O�̑Ή��֌W�\�z�̂��߂Ɍ�Ŏg��
	vector<string> boneNames(pmdBones.size());
	//�{�[���m�[�h�}�b�v�����
	for (int idx = 0; idx < pmdBones.size(); ++idx) {
		auto& pb = pmdBones[idx];
		boneNames[idx] = pb.boneName;
		auto& node = _boneNodeTable[pb.boneName];
		node.boneIdx = idx;
		node.startPos = pb.pos;
	}
	//�e�q�֌W���\�z����
	for (auto& pb : pmdBones) {
		//�e�C���f�b�N�X���`�F�b�N(���蓾�Ȃ��ԍ��Ȃ��΂�)
		if (pb.parentNo >= pmdBones.size()) {
			continue;
		}
		auto parentName = boneNames[pb.parentNo];
		_boneNodeTable[parentName].children.emplace_back(&_boneNodeTable[pb.boneName]);
	}
	_boneMatrices.resize(pmdBones.size());
}

HRESULT PMDBone::CreateResource(ComPtr<ID3D12Device> device) {
	HRESULT hr{};

	//�A�b�v���[�h�p�̃q�[�v�ƃ��\�[�X�p�f�B�X�N���v�^
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(XMMATRIX) * _boneMatrices.size() + 0xff) & ~0xff);
	//�萔�o�b�t�@�쐬
	hr = device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_boneBuffer)
	);

	//�}�b�v�ƃR�s�[
	hr = _boneBuffer->Map(0, nullptr, (void**)&_boneMappedMatrix);
	if (FAILED(hr)) {
		assert(SUCCEEDED(hr));
		return hr;
	}
	std::copy(_boneMatrices.begin(), _boneMatrices.end(), _boneMappedMatrix);

	//���\�[�X�p�̃q�[�v
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//�V�F�[�_���猩����悤��
	descHeapDesc.NodeMask = 0;//�}�X�N��0
	descHeapDesc.NumDescriptors = 1;//
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//�f�X�N���v�^�q�[�v���
	device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(_boneMatrixDescHeap.ReleaseAndGetAddressOf()));//����

	//�o�b�t�@�r���[�p�f�B�X�N���v�^
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _boneBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(_boneBuffer->GetDesc().Width);

	D3D12_CPU_DESCRIPTOR_HANDLE resourceHeapHandle = _boneMatrixDescHeap->GetCPUDescriptorHandleForHeapStart();

	//�萔�o�b�t�@�r���[�̍쐬
	device->CreateConstantBufferView(&cbvDesc, resourceHeapHandle);

	return hr;
}

#pragma endregion 

#pragma region �{�[���ݒ�

void PMDBone::InitializeBone() {
	//�{�[�������ׂď���������B
	std::fill(_boneMatrices.begin(), _boneMatrices.end(), XMMatrixIdentity());

	auto armNode = _boneNodeTable["���r"];
	auto& armPos = armNode.startPos;
	auto armMat = XMMatrixTranslation(-armPos.x, -armPos.y, -armPos.z) * XMMatrixRotationZ(XM_PIDIV2) * XMMatrixTranslation(armPos.x, armPos.y, armPos.z);
	_boneMatrices[armNode.boneIdx] = armMat;

	auto elbowNode = _boneNodeTable["���Ђ�"];
	auto& elbowPos = elbowNode.startPos;
	auto elbowMat = XMMatrixTranslation(-elbowPos.x, -elbowPos.y, -elbowPos.z) * XMMatrixRotationZ(-XM_PIDIV2) * XMMatrixTranslation(elbowPos.x, elbowPos.y, elbowPos.z);
	_boneMatrices[elbowNode.boneIdx] = elbowMat;

	RecursiveMatrixMultiply(&_boneNodeTable["�Z���^�["], XMMatrixIdentity());

	std::copy(_boneMatrices.begin(), _boneMatrices.end(), _boneMappedMatrix);
}

/// <summary>
/// �{�[���̉�]�𖈉�s��
/// </summary>
/// <param name="command_list"></param>
void PMDBone::SettingBone(ComPtr<ID3D12GraphicsCommandList> command_list) {
	ID3D12DescriptorHeap* sceneheaps[] = { _boneMatrixDescHeap.Get() };
	command_list->SetDescriptorHeaps(1, sceneheaps);
	command_list->SetGraphicsRootDescriptorTable(2, _boneMatrixDescHeap->GetGPUDescriptorHandleForHeapStart());
}

/// <summary>
/// �ċA�I�Ƀ{�[���̉�]
/// </summary>
/// <param name="node"></param>
/// <param name="mat"></param>
void PMDBone::RecursiveMatrixMultiply(BoneNode* node, const XMMATRIX& mat) {
	_boneMatrices[node->boneIdx] *= mat;
	for (auto& cnode : node->children) {
		RecursiveMatrixMultiply(cnode, _boneMatrices[node->boneIdx]);
	}
}

/// <summary>
/// �N�I�[�^�j�I����]
/// </summary>
void PMDBone::SetQuaternion(std::unordered_map<std::string, std::vector<VMDMotion::KeyFrame>> motionMap) {
	for (auto& bonemotion : motionMap) {
		auto node = _boneNodeTable[bonemotion.first]; //�ŏ��̃m�[�h�擾
		auto& pos = node.startPos;

		auto mat = XMMatrixTranslation(-pos.x, -pos.y, -pos.z) *
			XMMatrixRotationQuaternion(bonemotion.second[0].quaternion) *
			XMMatrixTranslation(pos.x, pos.y, pos.z);
		_boneMatrices[node.boneIdx] = mat;
	}

	RecursiveMatrixMultiply(&_boneNodeTable["�Z���^�["], XMMatrixIdentity());
	copy(_boneMatrices.begin(), _boneMatrices.end(), _boneMappedMatrix);
}

#pragma endregion