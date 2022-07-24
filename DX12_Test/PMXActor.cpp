#include "PMXActor.h"

#pragma region コンストラクタ系

PMXActor::PMXActor(ComPtr<ID3D12Device> device, const char* filepath, PMXRenderer renderer) : _angle(0.0f)
{
	FILE* fp = nullptr;
	auto error = fopen_s(&fp, filepath, "rb");
	//auto error = fopen_s(&fp, "Model/巡音ルカ.pmd", "rb");
	if (fp == nullptr) {
		char strerr[256];
		strerror_s(strerr, 256, error);
	}
	//モデル生成
	_model = new PMXModel(fp, device, filepath);

	//マテリアル作成
	_material = new PMXMaterial(device, fp, filepath, PMXRenderer::TOON_MATERIAL_DESC_SIZE, renderer);

	fclose(fp);
}


PMXActor::~PMXActor()
{
}

#pragma endregion

#pragma region 描画ループ

void PMXActor::Update(std::shared_ptr<Matrix> matrix) {
	_angle += 0.01f;
	//matrix->Rotate(_angle);
}

void PMXActor::Draw(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> command_list) {

	//モデル描画
	_model->SetRenderBuffer(command_list);
	//マテリアル描画
	_material->Draw(device, command_list, PMXRenderer::TOON_MATERIAL_DESC_SIZE);
}

#pragma endregion
