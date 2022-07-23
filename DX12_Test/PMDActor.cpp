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
	_model = new Model(fp, device, "rb");

	//�}�e���A���쐬
	_material = new Material(device, fp, filepath, MATERIAL_DESC_SIZE, renderer);

	fclose(fp);
}


PMDActor::~PMDActor()
{
}

#pragma endregion

#pragma region �`�惋�[�v

void PMDActor::Update(std::shared_ptr<Matrix> matrix) {
	_angle += 0.01f;
	matrix->Rotate(_angle);
}

void PMDActor::Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list) {

	//���f���`��
	_model->SetRenderBuffer(command_list);

	//�}�e���A���`��
	_material->Draw(device, command_list, MATERIAL_DESC_SIZE);
	
}

#pragma endregion
