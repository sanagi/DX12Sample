#include "PMDActor.h"

#pragma region コンストラクタ系

PMDActor::PMDActor(ComPtr<ID3D12Device> device, const char* filepath, const char* motionpath, PMDRenderer renderer, bool useWhite) : _angle(0.0f)
{
	FILE* fp = nullptr;
	auto error = fopen_s(&fp, filepath, "rb");
	//auto error = fopen_s(&fp, "Model/巡音ルカ.pmd", "rb");
	if (fp == nullptr) {
		char strerr[256];
		strerror_s(strerr, 256, error);
	}
	//モデル生成
	_model = new PMDModel(fp, device, "rb");

	//マテリアル作成
	_material = new PMDMaterial(device, fp, filepath, PMDRenderer::TOON_MATERIAL_DESC_SIZE, renderer, useWhite);

	//ボーン関連
	_bone = new PMDBone(device, fp);

	//モーション関連
	_motion = new VMDMotion(motionpath);
	_motion->SetPMDBone(_bone);

	fclose(fp);

	//アニメーション開始
	_motion->PlayAnimation();
}


PMDActor::~PMDActor()
{
}

#pragma endregion

#pragma region 描画ループ

void PMDActor::Update(ComPtr<ID3D12GraphicsCommandList> command_list, std::shared_ptr<Matrix> matrix) {
	_motion->UpdateMotion();
	_angle += 0.01f;
	matrix->Rotate(_angle);
	_bone->SettingBone(command_list);
}

void PMDActor::Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list) {

	//モデル描画
	_model->SetRenderBuffer(command_list);
	//マテリアル描画
	_material->Draw(device, command_list, PMDRenderer::TOON_MATERIAL_DESC_SIZE);
}

#pragma endregion
