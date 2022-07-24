#include "PMXActor.h"

#pragma region �R���X�g���N�^�n

PMXActor::PMXActor(ComPtr<ID3D12Device> device, const char* filepath, PMXRenderer renderer) : _angle(0.0f)
{
	FILE* fp = nullptr;
	auto error = fopen_s(&fp, filepath, "rb");
	//auto error = fopen_s(&fp, "Model/�������J.pmd", "rb");
	if (fp == nullptr) {
		char strerr[256];
		strerror_s(strerr, 256, error);
	}
	//���f������
	_model = new PMXModel(fp, device, filepath);

	//�}�e���A���쐬
	_material = new PMXMaterial(device, fp, filepath, PMXRenderer::TOON_MATERIAL_DESC_SIZE, renderer);

	fclose(fp);
}


PMXActor::~PMXActor()
{
}

#pragma endregion

#pragma region �`�惋�[�v

void PMXActor::Update(std::shared_ptr<Matrix> matrix) {
	_angle += 0.01f;
	//matrix->Rotate(_angle);
}

void PMXActor::Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list) {

	//���f���`��
	_model->SetRenderBuffer(command_list);
	//�}�e���A���`��
	_material->Draw(device, command_list, PMXRenderer::TOON_MATERIAL_DESC_SIZE);
}

#pragma endregion
