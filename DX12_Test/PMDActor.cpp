#include "PMDActor.h"

#pragma region �R���X�g���N�^�n

PMDActor::PMDActor(ComPtr<ID3D12Device> device, const char* filepath, const char* motionpath, PMDRenderer renderer, bool useWhite) : _angle(0.0f)
{
	FILE* fp = nullptr;
	auto error = fopen_s(&fp, filepath, "rb");
	//auto error = fopen_s(&fp, "Model/�������J.pmd", "rb");
	if (fp == nullptr) {
		char strerr[256];
		strerror_s(strerr, 256, error);
	}
	//���f������
	_model = new PMDModel(fp, device, "rb");

	//�}�e���A���쐬
	_material = new PMDMaterial(device, fp, filepath, PMDRenderer::TOON_MATERIAL_DESC_SIZE, renderer, useWhite);

	//�{�[���֘A
	_bone = new PMDBone(device, fp);

	//���[�V�����֘A
	_motion = new VMDMotion(motionpath);
	_motion->SetPMDBone(_bone);

	fclose(fp);

	//�A�j���[�V�����J�n
	_motion->PlayAnimation();
}


PMDActor::~PMDActor()
{
}

#pragma endregion

#pragma region �`�惋�[�v

void PMDActor::Update(ComPtr<ID3D12GraphicsCommandList> command_list, std::shared_ptr<Matrix> matrix) {
	_motion->UpdateMotion();
	_angle += 0.01f;
	matrix->Rotate(_angle);
	_bone->SettingBone(command_list);
}

void PMDActor::Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list) {

	//���f���`��
	_model->SetRenderBuffer(command_list);
	//�}�e���A���`��
	_material->Draw(device, command_list, PMDRenderer::TOON_MATERIAL_DESC_SIZE);
}

#pragma endregion
